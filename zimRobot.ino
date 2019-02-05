#include <HCSR04.h>
#include <SoftwareSerial.h>
#include "pitches.h"

#define BT_TX 3
#define BT_RX 4

#define PIEZO 8

#define L293N_ENA 5
#define L293N_ENB 6
#define L293N_IN1 7
#define L293N_IN2 9
#define L293N_IN3 10
#define L293N_IN4 11

#define DISTANCE_THRESHOLD 20

long prevMillis_env = 0;
long interval_env = 2500;

long prevMillis_dist = 0;
long interval_dist = 1000;

int value = 0;
float voltage = 0;
float temp = 0;
float light = 0;
int distance = 0;

boolean BTConnect = false;
char inChar;
String inString;

// Obstacle detected sound
int obstacleDetectedNotes[] = {NOTE_D5, NOTE_E6};
int obstacleDetectedNoteDurations[] = {8, 12};

// For connect/disconnect tones
int btConnect[] = {NOTE_G5, NOTE_C6};
int btConnectNoteDurations[] = {12, 8};

int btDisconnect[] = {NOTE_C5, NOTE_G4};
int btDisconnectNoteDurations[] = {12, 8};

// For Bluetooth HC-05
SoftwareSerial BTSerial(BT_TX, BT_RX);

// For Ultrasonic sensor HC-SR04(trig, echo)
HCSR04 front_HC(12, 13);

void setup() {
  // put your setup code here, to run once:
  pinMode(L293N_ENA, OUTPUT);
  pinMode(L293N_ENB, OUTPUT);
  pinMode(L293N_IN1, OUTPUT);
  pinMode(L293N_IN2, OUTPUT);
  pinMode(L293N_IN3, OUTPUT);
  pinMode(L293N_IN4, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("Hello world!");

  BTSerial.begin(9600);
  BTSerial.println("Hello world!");

  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  // Sensors Reporting if interval is reached
  unsigned long curMillis = millis();
  if (curMillis - prevMillis_env > interval_env){
    prevMillis_env = curMillis;
    // Temperature
    value = analogRead(A0);
    voltage = value * 0.00488;
    temp = voltage * 100;
    //Serial.print("TEMP:");
    //Serial.println(temp);
    BTSerial.print("TEMP:");
    BTSerial.println(temp);
  
    // Light
    value = analogRead(A1);
    light = value;
    //Serial.print("LIGHT:");
    //Serial.println(light);
    BTSerial.print("LIGHT:");
    BTSerial.println(light);
  }
  if (curMillis - prevMillis_dist > interval_dist){
    prevMillis_dist = curMillis;
    // Distance (Front) - Ultrasonic
    distance = front_HC.dist();
    //Serial.print("DIST:");
    //Serial.println(distance);
    BTSerial.print("DIST:");
    BTSerial.println(distance);
  }
  
  // If there is incoming data
  if (BTSerial.available() > 0){
    // If a connection is made
    if (BTConnect == false) {
      BTConnect = true;
      playMelody(btConnect, btConnectNoteDurations, 2);
    }

    // Read incoming data and act accordingly
    inString = "";
    while (BTSerial.available() > 0) {
      inChar = BTSerial.read();
      inString = inString + inChar;
    }
    //Serial.println(inString);

    if (inString == "#b=0#") {
      robotStop();
      BTSerial.println("Stopping Robot");
    }
    else if (inString == "#b=9#" ||
             inString == "#b=19#" ||
             inString == "#b=29#" ||
             inString == "#b=39#" ||
             inString == "#b=49#") {
      robotBreak();
      BTSerial.println("Robot Braking...");
    }
    else if (inString == "#b=1#") {        
      if (distance < DISTANCE_THRESHOLD){
          robotStop();
          BTSerial.println("Robot encountered obstacle...");
      }
      else{
        robotForward(150);
        BTSerial.println("Robot Move Forward...");
      }
    }
    else if (inString == "#b=2#") {
      robotReverse(150);
      BTSerial.println("Robot Move Backward...");
    }
    else if (inString == "#b=3#") {
      robotTurnLeft(150);
      BTSerial.println("Robot Turn Left...");
    }
    else if (inString == "#b=4#") {
      robotTurnRight(150);
      BTSerial.println("Robot Turn Right...");
    }
    else if (inString.startsWith("+DISC")) {
      BTConnect = false;
      delay(1000);
      while (BTSerial.available() > 0) {
        BTSerial.read();
      }
      playMelody(btDisconnect, btDisconnectNoteDurations, 2);
    }
  }
}

void playMelody(int *melody, int *noteDurations, int notesLength)
{
  pinMode(PIEZO, OUTPUT);
  
  for (int thisNote = 0; thisNote < notesLength; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(PIEZO, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(PIEZO);
  }
}

void robotStop()
{
  digitalWrite(L293N_ENA, LOW);
  digitalWrite(L293N_IN1, LOW);
  digitalWrite(L293N_IN2, LOW);

  digitalWrite(L293N_ENB, LOW);
  digitalWrite(L293N_IN3, LOW);
  digitalWrite(L293N_IN4, LOW);
}

void robotBreak()
{
  digitalWrite(L293N_ENA, HIGH);
  digitalWrite(L293N_IN1, HIGH);
  digitalWrite(L293N_IN2, HIGH);

  digitalWrite(L293N_ENB, HIGH);
  digitalWrite(L293N_IN3, HIGH);
  digitalWrite(L293N_IN4, HIGH);
}

void robotForward(int motorSpeed)
{
  analogWrite(L293N_ENA, motorSpeed);
  digitalWrite(L293N_IN1, LOW);
  digitalWrite(L293N_IN2, HIGH);

  analogWrite(L293N_ENB, motorSpeed);
  digitalWrite(L293N_IN3, HIGH);
  digitalWrite(L293N_IN4, LOW);
}

void robotReverse(int motorSpeed)
{
  analogWrite(L293N_ENA, motorSpeed);
  digitalWrite(L293N_IN1, HIGH);
  digitalWrite(L293N_IN2, LOW);

  analogWrite(L293N_ENB, motorSpeed);
  digitalWrite(L293N_IN3, LOW);
  digitalWrite(L293N_IN4, HIGH);
}

void robotTurnRight(int motorSpeed)
{
  analogWrite(L293N_ENA, motorSpeed);
  digitalWrite(L293N_IN1, HIGH);
  digitalWrite(L293N_IN2, LOW);

  analogWrite(L293N_ENB, motorSpeed);
  digitalWrite(L293N_IN3, HIGH);
  digitalWrite(L293N_IN4, LOW);
}

void robotTurnLeft(int motorSpeed)
{
  analogWrite(L293N_ENA, motorSpeed);
  digitalWrite(L293N_IN1, LOW);
  digitalWrite(L293N_IN2, HIGH);

  analogWrite(L293N_ENB, motorSpeed);
  digitalWrite(L293N_IN3, LOW);
  digitalWrite(L293N_IN4, HIGH);
}
