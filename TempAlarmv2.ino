/*
 * Smart Object for 'Fablab Smart Objects' keuzevak
 * 
 * Thijs Dregmans
 * student 1024272
 * Periode 2, Ti1H
 * 
 */

#include <Arduino.h>
#include <Wire.h>
#include <arduinio.h>

#define I2C_TEMP_SENSOR_ADDRESS 0x4A

#define dataPin 2
#define clockPin 3
#define latchPin1 4
#define latchPin2 5

#define knop1 9
#define knop2 10

#define led1EnBuzzer 6
#define led2 7
#define led3 8
#define led4 11

// constanten voor 7-segment Display
const byte getallen[] = {
  B00111111, //0
  B00000110, //1
  B01011011, //2
  B01001111, //3
  B01100110, //4
  B01101101, //5
  B01111101, //6
  B00000111, //7
  B01111111, //8
  B01101111  //9
};

byte bits;

int grenswaarde;
bool alarmModusAan;
bool alarmAan;
bool alarmBijGroterDanGrens;

bool knop1in;
bool knop2in;

int modus = 0;
bool knipperCyclusEven = true;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();  
  Serial.begin(9600);

  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(latchPin1, OUTPUT);
  pinMode(latchPin2, OUTPUT);
  

  pinMode(led1EnBuzzer, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  pinMode(knop1, OUTPUT);
  pinMode(knop2, OUTPUT);

  ClearSevenSegment();
  alarmModusAan = false;
  digitalWrite(led2, HIGH);
  alarmBijGroterDanGrens = true;
  grenswaarde = 0;
}

int getTemp() {
  Wire.beginTransmission(I2C_TEMP_SENSOR_ADDRESS);  
  Wire.write(0x00);  
  Wire.endTransmission();
  Wire.requestFrom(I2C_TEMP_SENSOR_ADDRESS, 1);  
  int8_t temperture = Wire.read(); 
  return temperture; 
}

void SevenSegment(int getal) {
  if(getal>=0 || getal<100) {
    unsigned eenheden = (getal / 1U) % 10;
    unsigned tientallen = (getal / 10U) % 10;
    
    digitalWrite(latchPin1, LOW);
    bits = B11111111 ^ getallen[tientallen];
    shiftOut(dataPin, clockPin, MSBFIRST, bits);
    digitalWrite(latchPin1, HIGH);
    
    digitalWrite(latchPin2, LOW);
    bits = B11111111 ^ getallen[eenheden];
    shiftOut(dataPin, clockPin, MSBFIRST, bits);
    digitalWrite(latchPin2, HIGH);
  }
}
void ClearSevenSegment() {
  digitalWrite(latchPin1, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, B11111111);
  digitalWrite(latchPin1, HIGH);
  
  digitalWrite(latchPin2, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, B11111111);
  digitalWrite(latchPin2, HIGH);
}

void loop() {
  // huidige instellingen

  knop1in = digitalRead(knop1);
  knop2in = digitalRead(knop2);

  if(modus == 0) {
    Serial.println("modus 0");
    // meet temp en laat zien op sevenSegment
    SevenSegment(getTemp());
    Serial.println(getTemp());
    // bekijk grenswaarde en laat alarm afgaan
    if(alarmModusAan) {
      if((alarmBijGroterDanGrens && grenswaarde < getTemp()) || (!alarmBijGroterDanGrens && grenswaarde > getTemp())) {
        alarmAan = true;
      }
      else {
        alarmAan = false;
      }
    }
    if(alarmAan) {
      digitalWrite(led1EnBuzzer, HIGH);
      delay(200);
      digitalWrite(led1EnBuzzer, LOW);
      delay(200);
    }
    // knop 1 & 2: volgende menu
    if(knop1in && knop2in) {
      modus = 1;
    }
  }
  else if(modus == 1) {
    ClearSevenSegment();
    Serial.println("modus 1");
    if((knop1in && !knop2in) || (!knop1in && knop2in)) {
      alarmModusAan = !alarmModusAan;
      if(alarmModusAan) {
        digitalWrite(led4, HIGH);
      }
      else {
        digitalWrite(led4, LOW);
      }
    }
    // knop 1 & 2: volgende menu
    if(knop1in && knop2in) {
      modus = 2;
    }
  }
  else if(modus == 2) {
    Serial.println("modus 2");
    // laat grenswaarde zien op SevenSegment
    if(knipperCyclusEven) {
      ClearSevenSegment();
    }
    else {
      SevenSegment(grenswaarde);
    }
    knipperCyclusEven = !knipperCyclusEven;
    // knop 1: trek 1 af van grenswaarde
    if(knop1in && !knop2in) {
      grenswaarde = grenswaarde - 1;
      Serial.print(grenswaarde);
      Serial.println(": uitgevoerde bewerking: grenswaarde -1");
    }
    // knop 2: tel 1 op bij grenwaarde
    if(!knop1in && knop2in) {
      grenswaarde++;
      Serial.println(": uitgevoerde bewerking: grenswaarde +1");
    }

    // knop 1 & 2: volgende menu
    if(knop1in && knop2in) {
      modus = 3;
    }
  }
  else if(modus == 3) {
    Serial.println("modus 3");

    // laat grenswaarde zien op SevenSegment
    if(knipperCyclusEven) {
      ClearSevenSegment();
    }
    else {
      SevenSegment(grenswaarde);
    }
    
    if(alarmBijGroterDanGrens) {
      digitalWrite(led2, LOW);
      digitalWrite(led3, HIGH);
    }
    else {
      digitalWrite(led2, HIGH);
      digitalWrite(led3, LOW);
    }
    // knop 1: grenswaarde als ondergrens
    if(knop1in && !knop2in) {
      alarmBijGroterDanGrens = false;
    }
    // knop 2: grenswaarde als bovengrens
    if(!knop1in && knop2in) {
      alarmBijGroterDanGrens = true;
    }

    // knop 1 & 2: volgende menu
    if(knop1in && knop2in) {
      modus = 0;
    }
  }
  delay(100);
}
