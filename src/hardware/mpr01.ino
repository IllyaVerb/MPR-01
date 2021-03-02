#include <Servo.h>
#include <Ultrasonic.h>

#define BLYNK_PRINT Serial
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#define EspSerial Serial3
#define ESP8266_BAUD 9600

char auth[] = "";//token MPR-01

// Your WiFi credentials.
// Set password to "" for open networks.

char ssid[] = "";
char pass[] = "";

ESP8266 wifi(&EspSerial);

//Ultrasonic ultrasonic(3,4);//trig-3,echo-4
Servo servo;

const int item_eye[7] = {0,34,35,36,37,38},//датчик лежащего предмета
          left_eye = 22,    //левый инфракрасный
          right_eye = 23,   //правый инфракрасный
          fLed = 28,        //светодиод-фара
          infLed = 29,      //пищалка
          IN1 = 32,         // первый для мотора левый
          IN2 = 33,         // второй для мотора левый
          IN3 = 30,         // первый для мотора правый
          IN4 = 31,         // второй для мотора правый
          ENB1 = 6,         // скорость моторов левый
          ENB2 = 5;         // скорость моторов правый

int joystickX = 0, joystickY = 0,//Управление джойстиком
    t, l, mode=0, SPEED=255;

BLYNK_WRITE(V5){
  int a=param.asInt();
  SPEED=a;
}

BLYNK_WRITE(V2){
  double a = param[0].asInt();//X
  double b = param[1].asInt();//Y
  
  if(mode==0){
    double Speed=sqrt(a*a+b*b)/100;
    double enb1,
           enb2;
  if(Speed>1)Speed=1;
  if(b<0){
    digitalWrite (IN1, HIGH);
    digitalWrite (IN2, LOW);
    digitalWrite (IN3, LOW);
    digitalWrite (IN4, HIGH);
    b*=-1;
  } else {
    digitalWrite (IN1, LOW);
    digitalWrite (IN2, HIGH);
    digitalWrite (IN3, HIGH);
    digitalWrite (IN4, LOW); 
  }
  if(a>0){
    enb1=Speed*SPEED;
    enb2=(1-a/100)*Speed*SPEED;
  }
  if(a<0){
    enb1=(1+a/100)*Speed*SPEED;
    enb2=Speed*SPEED;
  }
  if(a==0){
    enb1=1;
    enb2=1;
  }

  if(enb1>255)enb1=255;
  if(enb2>255)enb2=255;
  
  analogWrite(ENB1, enb1);
  analogWrite(ENB2, enb2);
  //delay(50);
  }
  
}

BLYNK_WRITE(V1){
  int a = param.asInt();
  mode=a;
}
void fall(){
  
  if(digitalRead(item_eye[1])==LOW||
  digitalRead(item_eye[2])==LOW||
  digitalRead(item_eye[3])==LOW||
  digitalRead(item_eye[4])==LOW||
  digitalRead(item_eye[5])==LOW){
  digitalWrite(infLed, HIGH);
  Blynk.virtualWrite(V0, 255);
  stopik();
  //delay(10);
  //Blynk.notify("Look! MPR-02 is finding an item!");
  fall();
  }else {
    digitalWrite(infLed, LOW);
    Blynk.virtualWrite(V0, 0);
  }
  
}
int forward(){
  digitalWrite (IN1, LOW);
  digitalWrite (IN2, HIGH); 
  digitalWrite (IN3, HIGH);
  digitalWrite (IN4, LOW); 
  analogWrite(ENB1, SPEED);
  analogWrite(ENB2, SPEED);
}
int revers(){
  digitalWrite (IN1, HIGH);
  digitalWrite (IN2, LOW);
  digitalWrite (IN3, LOW);
  digitalWrite (IN4, HIGH);  
  analogWrite(ENB1, SPEED);
  analogWrite(ENB2, SPEED);
}
int stopik(){
  analogWrite(ENB1, 0);
  analogWrite(ENB2, 0);
}
int turnLeft(){
  digitalWrite (IN1, HIGH);
  digitalWrite (IN2, LOW);
  digitalWrite (IN3, HIGH);
  digitalWrite (IN4, LOW); 
  analogWrite(ENB1, SPEED*0.95);
  analogWrite(ENB2, SPEED*0.95);
  //myDelay(500);
}
int turnRight(){
  digitalWrite (IN1, LOW);
  digitalWrite (IN2, HIGH); 
  digitalWrite (IN3, LOW);
  digitalWrite (IN4, HIGH);
  analogWrite(ENB1, SPEED*0.95);
  analogWrite(ENB2, SPEED*0.95);
  //myDelay(500);
}

void myDelay(int count){
  for(int i=1;i<=count;i++){
  if(i%25==0)fall();
  delay(1);
  }
}

void setup(){
  servo.attach(27);
  servo.write(75);
  
  Serial.begin(9600);
  delay(10);
  EspSerial.begin(ESP8266_BAUD);
  delay(10);
  Serial2.begin(9600);
  delay(10);
  Blynk.begin(auth, wifi, ssid, pass);

  pinMode(fLed, OUTPUT);
  pinMode(infLed, OUTPUT);
  
  pinMode (IN1, OUTPUT);
  pinMode (IN2, OUTPUT);
  pinMode (IN3, OUTPUT);
  pinMode (IN4, OUTPUT);
  pinMode(A11, INPUT);
  pinMode(item_eye[1], INPUT);
  pinMode(item_eye[2], INPUT);
  pinMode(item_eye[3], INPUT);
  pinMode(item_eye[4], INPUT);
  pinMode(item_eye[5], INPUT);
  pinMode(left_eye, INPUT);
  pinMode(right_eye, INPUT);
  
  pinMode(3, OUTPUT);//trig
  pinMode(4, INPUT);//echo
}

int dist(){
  int o=0;
  for(int i=0;i<1;i++){
    o += (int)sonic_read(3, 4);
  }
  return o;
}

long sonic_read(int trig_pin, int echo_pin){
  digitalWrite(trig_pin, LOW);
  delayMicroseconds(5);
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
 
  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echo_pin, INPUT);
  long duration = pulseIn(echo_pin, HIGH);
 
  // Convert the time into a distance
  long cm = (duration / 2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
  return cm;
}

int readIK(){
  int y=0;
  for(int i=0;i<10;i++){
    y += analogRead(A0);
  }
  float volts = y/10*0.0048828125;  // value from sensor * (5/1024)
  
  y = 13*pow(volts, -1); // worked out from datasheet graph
  
  Serial.print(y);
  Serial.println(" _IK     ");
  return y;
}



int automode_searching(int dist_c, int edge){
  //myDelay(5);
  //stopik();
      if(dist_c <= edge || digitalRead(left_eye)==LOW || digitalRead(right_eye)==LOW){
       stopik();
       delay(10);
       
       servo.write(170);
       delay(500);
       //myDelay(210);
       //fall();
       int left_dist=dist();
       //myDelay(10);
        
       servo.write(0);
       delay(500);
       //myDelay(260);
       //fall();
       int right_dist=dist();
       //myDelay(10);
       
       servo.write(75);
       delay(1000);
       //myDelay(210);
       if((right_dist<=60&&left_dist<=60)||(digitalRead(left_eye)==LOW && digitalRead(right_eye)==LOW)){
         revers();
         return 1000;
       }
       else{
         if((right_dist<left_dist)||(digitalRead(right_eye)==LOW && digitalRead(left_eye)==HIGH)){
           turnLeft();
           return 1000;
         }
         if((right_dist>=left_dist)||(digitalRead(right_eye)==HIGH && digitalRead(left_eye)==LOW)){
           turnRight();
           return 1000;
          }
        }
      }
      else
      {
       forward(); 
       return 500;
       //fall();
      }
}

void handcontrol(){
  digitalWrite(infLed, LOW);
  Blynk.virtualWrite(V0, 0);
  mode=0;
}

void faru(){
  if (analogRead(A11) < 300){ 
  digitalWrite(fLed, HIGH); 
  Blynk.virtualWrite(V3, 255);
  }
  else {
    digitalWrite(fLed, LOW);
    Blynk.virtualWrite(V3, 0);
  }
}

/* vars for timer: {next time to use, timeout} */
unsigned int timers[100][2] = { {0, 3000}, 
                                {0, 200}, 
                                {0, 10}, 
                                {0, 5}};
unsigned int timers_count = 4;

void loop(){   
  int now_time = millis(); // current time
  
  for (int i=0; i<timers_count; i++){
    if (millis() < timers[i][0])
      continue;
      
    timers[i][0] = now_time + timers[i][1];
    
    switch(i){
      case 0:{
        Blynk.run();
        break;
      }
      case 1:{
        int distance = dist();
        Blynk.virtualWrite(V4, distance);
        //Serial.println(mode);
        if(mode==1)
          timers[i][0] = millis() + automode_searching(distance, 50);
        else 
          handcontrol();

        //Serial.println(timers[i][0] - now_time);
        break;
      }
      case 2:{
        faru();
        break;
      }
      case 3:{
        fall();
        break;
      }
    }
    /*
    if( now_time >= next_time ){
        next_time = now_time + timeout;
        Blynk.run();
    }*/
  }
  
  /*if(mode==1)
    automode_searching();
  else 
    handcontrol();
  myDelay(10);*/
  delay(1);
}
  
