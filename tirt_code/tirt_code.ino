#include <Wire.h>

int I2C_Address = 0xA7 >> 1; // ADXL345 的 I2C 地址

int X0, X1, Y0, Y1, Z1, Z0;
float X, Y, Z;

#define TRIGGER_PIN_L 10
#define ECHO_PIN_L 11
//NewPing sonar_L(TRIGGER_PIN_L, ECHO_PIN_L, MAX_DISTANCE); //左邊超聲波
#define TRIGGER_PIN_R 8
#define ECHO_PIN_R 9

#define L echopin_L()/58.2
#define R echopin_R()/58.2

#define left_a 5
#define left_b 6
#define left_pwm 7

#define right_a 3
#define right_b 2
#define right_pwm 4

#define left_sensor digitalRead(33)
#define right_sensor digitalRead(31)
#define back_sensor digitalRead(35)

#define mode1 digitalRead(43)
#define mode2 digitalRead(41)
#define mode3 digitalRead(39)
#define mode4 digitalRead(37)
//0沒碰到黑線//1碰到黑線
int sen0 = 0;
int sen1 = 1;
int mode_station;
int setmax = 35;
int setmax1 = 20;
int setmini = 0;

typedef struct motor {
  int a;
  int b;
  int pwm;
} motor;

motor MOTOR[] = {
  {left_a, left_b, left_pwm},
  {right_a, right_b, right_pwm},
};

enum Direction1
{
  left = 0,
  right = 1,


};

void setup()
{
  Serial.begin(9600);//開始與電腦連線，傳輸定額為9600
  pinMode(left_a, OUTPUT);
  pinMode(left_b, OUTPUT);
  pinMode(right_a, OUTPUT);
  pinMode(right_b, OUTPUT);

  pinMode(ECHO_PIN_L, INPUT);
  pinMode(TRIGGER_PIN_L, OUTPUT);
  pinMode(ECHO_PIN_L, INPUT);
  pinMode(TRIGGER_PIN_R, OUTPUT);

  pinMode(left_sensor, INPUT);
  pinMode(right_sensor, INPUT);
  pinMode(back_sensor, INPUT);

  pinMode(mode1, INPUT);
  pinMode(mode2, INPUT);
  pinMode(mode3, INPUT);
  pinMode(mode4, INPUT);
  Wire.begin();  //初始化 I2C
  setReg(0x2D, 0xA); // (打開電源, 設定輸出資料速度為 100 Hz)
}

void loop()
{
  //    Serial.println(L);
  //  mode_station = 0;
  if (mode1 == 1)
  {
    mode_station = 1;
    rule();
  }
  else if (mode2 == 1)
  {
    mode_station = 2;
    rule();
  }
  else if (mode3 == 1)
  {
    mode_station = 3;
    rule();
  }
  else if (mode4 == 1)
  {
    mode_station = 4;
  }
  Serial.println(mode_station);
  switch (mode_station)
  {
    case 1:
      Serial.println("1");
      turn(1, 100);
      found();
      one(200);
      runrunfound();
      //      one(200);

      break;
    case 2:
      Serial.println("2");
      turn(1, 100);
      //      moterstate(2, 100, left);
      //            moterstate(1, 100, right);
      found();
      one(200);
      break;
    case 3:
      Serial.println("3");
      turn(1, 100);
      found();
      break;
    case 4:
      Serial.println("4");
      moterstate(3, 0, left);
      moterstate(3, 0, right);
      break;
      //    default:
      //      Serial.println("non");
      //      turn(1, 100);
      //      found();
      //      runrunfound();
      //      one(100);
      //      break;

  }

}
void rule()//輕量
{
  do
  {
    turn(2, 75 );
  } while (back_sensor == sen0);
  turn(3, 75 );
}
void one(int speeding)
{
  int i = 75;
  while (L < 25 && R < 25 && left_sensor == sen0 && right_sensor == sen0)
  {
    Serial.println("衝");
    int a;
    i += 60;
    a = i;

    turn(1, a);

    if (i >= speeding)
    {

      a = 255;
      turn(1, a);

    }
  }
  //  turn(3, 0);
}
void runrunfound()
{
  //  if (L <= setmax && L > setmini)
  //  {
  //    turn(5, 100);
  //
  //    Serial.println("左尋敵維修1");
  //
  //  }
  //  else if ( R <= setmax && R > setmini)
  //  {
  //    turn(4, 100);
  //
  //    Serial.println("右尋敵維修1");
  //
  //  }
  if (L > 25 && R > 25)
  {
    turn(1, 100);
    Serial.println("打發走人");
  }
  else
  {
    Serial.println("開幹");
    while (L <= setmax1 && L > setmini && R >= 15)
    {
      turn(5, 100);

      Serial.println("左面前");
    }
    //   turn(1, 100);
    while (R <= setmax1 && R > setmini && L >= 15)
    {
      turn(4, 100);

      Serial.println("右面前");
    }
  }

  //  turn(1, 100);

}
void found()
{
  if (left_sensor == sen1 && right_sensor == sen1)
  {
    sen();
    Serial.println("後衝");
  }
  if (left_sensor == sen1 && right_sensor == sen0)
  {
    sensordo(4);
    Serial.println("左");
  }
  if (left_sensor == sen0 && right_sensor == sen1)
  {
    sensordo(5);
    Serial.println("右");
  }
}
void sensordo(int mode)
{
  turn(2, 75 );
  delay(random(100, 500));
  turn(mode, random(50, 70));
  delay(random(50, 800));
}
void sen()
{
  turn(2, 75);
  delay(random(300, 1001));
  turn( random(4, 6), random(100, 150) );
  delay(random(50, 300));
}
void moterstate(int mode, int maxspeed, enum Direction1 direction1)
{
  switch (mode)
  {
    case 1:
      digitalWrite(MOTOR[direction1].a , HIGH);
      digitalWrite(MOTOR[direction1].b , LOW);
      analogWrite(MOTOR[direction1].pwm , maxspeed);
      break;
    case 2:
      digitalWrite(MOTOR[direction1].a , LOW);
      digitalWrite(MOTOR[direction1].b , HIGH);
      analogWrite(MOTOR[direction1].pwm , maxspeed);
      break;
    case 3:
      digitalWrite(MOTOR[direction1].a , LOW);
      digitalWrite(MOTOR[direction1].b , LOW);
      analogWrite(MOTOR[direction1].pwm , maxspeed);
      break;
  }
}
void turn(int mode, int speeding)
{
  switch (mode)
  {
    case 1://前進
      moterstate(1, speeding, left);
      moterstate(1, speeding, right);
      break;
    case 2://後退
      moterstate(2, speeding, left);
      moterstate(2, speeding, right);
      break;
    case 3://停
      moterstate(3, 0, left);
      moterstate(3, 0, right);
      break;
    case 4://左旋
      moterstate(1, speeding, left);
      moterstate(2, speeding, right);
      break;
    case 5://右璇
      moterstate(2, speeding, left);
      moterstate(1, speeding, right);
      break;
    //    case 6://等加速度
    //      Acc(speeding);
    //      break;
    case 7 ://左尋敵(維修乾)
      moterstate(1, 155, left);
      moterstate(1, 100, right);
      break;
    case 8://右尋敵(維修乾)
      moterstate(1, 100, left);
      moterstate(1, 155, right);
      break;
    case 9://左尋敵(維修乾)
      moterstate(1, 100, left);
      moterstate(1, 0, right);
      break;
    case 10://右尋敵(維修乾)
      moterstate(1, 0, left);
      moterstate(1, 100, right);
      break;
  }
}
int echopin_L()
{

  digitalWrite(TRIGGER_PIN_L, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN_L, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN_L, LOW);
  return pulseIn(ECHO_PIN_L, HIGH);
}
int echopin_R()
{
  digitalWrite(TRIGGER_PIN_R, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN_R, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN_R, LOW);
  return pulseIn(ECHO_PIN_R, HIGH);
}
void adxl345()
{
  X0 = getData(0x32); // 取得 X 軸 低位元資料
  X1 = getData(0x33); // 取得 X 軸 高位元資料
  X = ((X1 << 8)  + X0) / 256.0;

  Y0 = getData(0x34); // 取得 Y 軸 低位元資料
  Y1 = getData(0x35); // 取得 Y 軸 高位元資料
  Y = ((Y1 << 8)  + Y0) / 256.0;

  Z0 = getData(0x36); // 取得 Z 軸 低位元資料
  Z1 = getData(0x37); // 取得 Y 軸 高位元資料
  Z = ((Z1 << 8)  + Z0) / 256.0;

  Serial.print("X= ");
  Serial.print(X);
  Serial.print("    Y= ");
  Serial.print(Y);
  Serial.print("    Z= ");
  Serial.println(Z);

}
void setReg(int reg, int data) {
  Wire.beginTransmission(I2C_Address);
  Wire.write(reg); // 指定佔存器
  Wire.write(data); // 寫入資料
  Wire.endTransmission();
}

/* getData(reg)：取得佔存器裡的資料
   參數：reg → 佔存器位址
*/
int getData(int reg) {
  Wire.beginTransmission(I2C_Address);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(I2C_Address, 1);

  if (Wire.available() <= 1) {
    return Wire.read();
  }
}
