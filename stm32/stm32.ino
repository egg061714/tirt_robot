#include <Arduino_FreeRTOS.h>
#include <Wire.h>

// ------------------------- I2C (ADXL345)
int I2C_Address = 0xA7 >> 1;
int X0, X1, Y0, Y1, Z1, Z0;
volatile float ax, ay, az;

// ------------------------- 超音波
#define TRIGGER_PIN_L 10
#define ECHO_PIN_L 11
#define TRIGGER_PIN_R 8
#define ECHO_PIN_R 9
volatile int distanceL = 0;
volatile int distanceR = 0;

// ------------------------- 馬達控制
#define left_a 5
#define left_b 6
#define left_pwm 7
#define right_a 3
#define right_b 2
#define right_pwm 4

// ------------------------- 感測器與模式按鍵
#define left_sensor digitalRead(33)
#define right_sensor digitalRead(31)
#define back_sensor digitalRead(35)
#define mode1 digitalRead(43)
#define mode2 digitalRead(41)
#define mode3 digitalRead(39)
#define mode4 digitalRead(37)

int sen0 = 0;
int sen1 = 1;

// ------------------------- 馬達封裝
typedef struct motor {
  int a;
  int b;
  int pwm;
} motor;

motor MOTOR[] = {
  {left_a, left_b, left_pwm},
  {right_a, right_b, right_pwm},
};

enum Direction1 { left = 0, right = 1 };

// ------------------------- 模式控制
volatile int currentMode = 0;
volatile bool runFlag = false;

// ------------------------- 任務宣告
void TaskControl(void *);
void TaskUltrasonic(void *);
void TaskADXL345(void *);

// ------------------------- setup
void setup() {
  Serial.begin(9600);
  Wire.begin();
  setReg(0x2D, 0xA); // 啟用 ADXL345

  pinMode(TRIGGER_PIN_L, OUTPUT);
  pinMode(ECHO_PIN_L, INPUT);
  pinMode(TRIGGER_PIN_R, OUTPUT);
  pinMode(ECHO_PIN_R, INPUT);

  pinMode(left_a, OUTPUT);
  pinMode(left_b, OUTPUT);
  pinMode(right_a, OUTPUT);
  pinMode(right_b, OUTPUT);

  pinMode(33, INPUT);
  pinMode(31, INPUT);
  pinMode(35, INPUT);
  pinMode(43, INPUT);
  pinMode(41, INPUT);
  pinMode(39, INPUT);
  pinMode(37, INPUT);

  xTaskCreate(TaskControl, "Control", 256, NULL, 2, NULL);
  xTaskCreate(TaskUltrasonic, "Ultrasonic", 128, NULL, 1, NULL);
  xTaskCreate(TaskADXL345, "ADXL", 128, NULL, 1, NULL);
}

void loop() {}

// ------------------------- 控制任務
void TaskControl(void *pvParameters) {
  for (;;) {
    if (mode1 == HIGH) {
      currentMode = 1; runFlag = true;
    } else if (mode2 == HIGH) {
      currentMode = 2; runFlag = true;
    } else if (mode3 == HIGH) {
      currentMode = 3; runFlag = true;
    } else if (mode4 == HIGH) {
      runFlag = false; turn(3, 0);
    }

    if (runFlag) {
      switch (currentMode) {
        case 1:
          rule(); found(); one(200); runrunfound(); break;
        case 2:
          turn(1, 100); found(); one(200); break;
        case 3:
          turn(1, 100); found(); break;
      }
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// ------------------------- 超音波任務
void TaskUltrasonic(void *pvParameters) {
  for (;;) {
    distanceL = echopin_L() / 58.2;
    distanceR = echopin_R() / 58.2;
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

// ------------------------- ADXL345 任務
void TaskADXL345(void *pvParameters) {
  for (;;) {
    X0 = getData(0x32); X1 = getData(0x33);
    Y0 = getData(0x34); Y1 = getData(0x35);
    Z0 = getData(0x36); Z1 = getData(0x37);
    ax = ((X1 << 8) + X0) / 256.0;
    ay = ((Y1 << 8) + Y0) / 256.0;
    az = ((Z1 << 8) + Z0) / 256.0;
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

// ------------------------- 馬達與行為邏輯
void moterstate(int mode, int maxspeed, enum Direction1 d) {
  switch (mode) {
    case 1:
      digitalWrite(MOTOR[d].a , HIGH);
      digitalWrite(MOTOR[d].b , LOW);
      analogWrite(MOTOR[d].pwm , maxspeed); break;
    case 2:
      digitalWrite(MOTOR[d].a , LOW);
      digitalWrite(MOTOR[d].b , HIGH);
      analogWrite(MOTOR[d].pwm , maxspeed); break;
    case 3:
      digitalWrite(MOTOR[d].a , LOW);
      digitalWrite(MOTOR[d].b , LOW);
      analogWrite(MOTOR[d].pwm , maxspeed); break;
  }
}

void turn(int mode, int speed) {
  switch (mode) {
    case 1: moterstate(1, speed, left); moterstate(1, speed, right); break;
    case 2: moterstate(2, speed, left); moterstate(2, speed, right); break;
    case 3: moterstate(3, 0, left); moterstate(3, 0, right); break;
    case 4: moterstate(1, speed, left); moterstate(2, speed, right); break;
    case 5: moterstate(2, speed, left); moterstate(1, speed, right); break;
  }
}

void rule() {
  do { turn(2, 75); } while (back_sensor == sen0);
  turn(3, 0);
}

void one(int speed) {
  int i = 75;
  while (distanceL < 25 && distanceR < 25 && left_sensor == sen0 && right_sensor == sen0) {
    i += 60;
    turn(1, (i >= speed ? 255 : i));
  }
  turn(3, 0);
}

void runrunfound() {
  if (distanceL > 25 && distanceR > 25) {
    turn(1, 100);
  } else {
    while (distanceL <= 20 && distanceR >= 15) {
      turn(5, 100);
    }
    while (distanceR <= 20 && distanceL >= 15) {
      turn(4, 100);
    }
  }
}

void found() {
  if (left_sensor == sen1 && right_sensor == sen1) {
    sen();
  } else if (left_sensor == sen1 && right_sensor == sen0) {
    sensordo(4);
  } else if (left_sensor == sen0 && right_sensor == sen1) {
    sensordo(5);
  }
}

void sensordo(int mode) {
  turn(2, 75);
  delay(random(100, 500));
  turn(mode, random(50, 70));
  delay(random(50, 800));
}

void sen() {
  turn(2, 75);
  delay(random(300, 1001));
  turn(random(4, 6), random(100, 150));
  delay(random(50, 300));
}

int echopin_L() {
  digitalWrite(TRIGGER_PIN_L, LOW); delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN_L, HIGH); delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN_L, LOW);
  return pulseIn(ECHO_PIN_L, HIGH);
}

int echopin_R() {
  digitalWrite(TRIGGER_PIN_R, LOW); delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN_R, HIGH); delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN_R, LOW);
  return pulseIn(ECHO_PIN_R, HIGH);
}

void setReg(int reg, int data) {
  Wire.beginTransmission(I2C_Address);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

int getData(int reg) {
  Wire.beginTransmission(I2C_Address);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(I2C_Address, 1);
  return Wire.available() ? Wire.read() : 0;
}
