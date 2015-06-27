#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

bool start = true;
byte mem1 = 0;
byte mem2 = 0;
byte n = 50;
double pwm = 50;
int error = 0;
int memerr = 0;
int sum = 0;
double kp = 0.1;
double ki = 0.05;
double kd = 1;
int destination = 0;
volatile int pos = 0;
int i = 0;
volatile byte k = 0;
volatile bool At = 0;
volatile bool Bt = 0;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(57600);
  lcd.clear();
  lcd.print("Initialization...");
  DDRD = B01110000;
  TCCR0A = B10000011;
  TCCR0B = B00000100;
  OCR0A = pwm;

  TCCR2A = B00000000;
  TCCR2B = B00000101;
  TIMSK2 = B00000001;

  EICRA |= (1 << ISC00) | (1 << ISC10);
  EIMSK |= (1 << INT0) | (1 << INT1);

  sei();
}

void loop()
{
  if (start) {
    byte diff = 100;
    int mem = 100;
    PORTD = B00100000;
    OCR0A = 100;
    while (diff>3) {
      delay(20);
      diff = pos-mem;
      mem = pos;
    }
    OCR0A = 0;
    pos = 0;
    start = false;
  }
  else {
    if (Serial.available() > 2) {
      mem1 = Serial.read();
      mem2 = Serial.read();
      n = Serial.read();
      destination = 100 * mem1 + mem2;
    }
    i++;
    if (i % 100 == 0) {
      destination += n - 50;
    }
    if (i == 10000) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(pwm);
      lcd.setCursor(6, 0);
      lcd.print(pos);
      lcd.setCursor(11, 0);
      lcd.print(destination);
      lcd.setCursor(0, 1);
      lcd.print(error);
      i = 0;
    }

    if (pos > 6700 || pos < 300) OCR0A = 0;
    else if (k >= 2) {
      k = 0;
      memerr = error;
      error = destination - pos;
      sum = (sum + error) / 2;
      if (error > 0) {
        pwm = error * kp + sum * ki - (memerr - error) * kd + 50;
        PORTD = B00010000;
        if (pwm < 0) pwm = 0;
        if (pwm >= 255)pwm = 255;
        OCR0A = pwm;
      } else if (error <= 0) {
        pwm = -(error * kp + sum * ki - (memerr - error) * kd) + 50;
        PORTD = B00100000;
        if (error == 0) pwm = 0;
        if (pwm < 0) pwm = 0;
        else if (pwm >= 255)pwm = 255;
        OCR0A = pwm;
      }
    }
  }
}

ISR (INT0_vect) {
  At = PIND & B00000100;
  if (At == Bt) pos++;
  else pos--;
}

ISR (INT1_vect) {
  Bt = PIND & B00001000;
  if (At != Bt) pos++;
  else pos--;
}

ISR (TIMER2_OVF_vect) {
  k++;
}
