#include <Servo.h>
#include <Stepper.h>

unsigned long times;
int fsink;
int m = 0;
const int hojdesteps = 200;
int klarstep = 1;
int pickupstage = 0;
Stepper hojde(hojdesteps, 23, 25, 27, 29);
Servo vinkel;
int pos = 0; //servoen der styrer vinklen, samt dens postion.
int ofset = 60;

void setup() {
  pinMode(7, INPUT); //sensor til nulstiling af højde
  pinMode(52, OUTPUT);//ralea til transportbånd
  pinMode(40, OUTPUT);//styring af elektromagnet
  digitalWrite(52, HIGH);
  hojde.setSpeed(100); //hastigheder på højde motoren.
  Serial.begin(9600);
  fsink = 1000; //hvor langt delay der er på
  times = millis() - fsink; //timestamp til delay
  vinkel.attach(9);
  vinkel.write(pos + ofset);
}

void loop() {
  if (digitalRead(7) == 1 && klarstep == 1) {
    klarstep = 2;;
  }
  if (digitalRead(7) == 0 && klarstep == 1) { //nulstiller armens højde.
    hojde.step(-1);
  } else if (klarstep == 2) {
    hojde.step(4 * hojdesteps);
    digitalWrite(52, LOW);
    klarstep++;
  } else {
    //her kører hovedkoden som tjekker om møtrikken står op eller ligger ned.
    if (analogRead(A0) < 900) {
      if (m == 0) {
        m = 1;
      }
      if (millis() - times > fsink) {
        for (int i = 0; i < 10000; i++) {
          if (analogRead(A1) < 900) {
            m = 2;
          }
        }
        times = millis();
      }
    }
    //hvis møtrikken ligger ned, stopper transportbåndet under armen.
    if (analogRead(A2) < 900 && m == 1 && pickupstage == 0) {
      digitalWrite(52, HIGH);
      pickupstage = 1;
    } else if (pickupstage == 3) {
      digitalWrite(52, LOW);
      delay(500);
      pickupstage = 0;
    } else if (analogRead(A2) < 900 && m == 2 && pickupstage == 0) {
      m = 0;
    }
    if (pickupstage == 1) { //her samles møtrikken op
      digitalWrite(40, HIGH);
      hojde.step(-3 * hojdesteps);
      delay(500);
      hojde.step(8 * hojdesteps);
      for (pos = 0 + ofset; pos <= 90 + ofset; pos += 1) {
        // in steps of 1 degree
        vinkel.write(pos);
        delay(15);
      }
      pickupstage = 3;
      hojde.step(-hojdesteps);
      digitalWrite(40, LOW);
      delay(500);
      for (pos = 90 + ofset; pos >= 0 + ofset; pos -= 1) {
        vinkel.write(pos);
        delay(15);
      }
      digitalWrite(52, LOW);
      hojde.step(-4 * hojdesteps);
    }
  }
}
