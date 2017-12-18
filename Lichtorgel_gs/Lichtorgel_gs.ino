// Arduino LED Color Organ Kit
//https://www.youtube.com/user/greatscottlab/


#define analogPin A0      //connects to the Output of the MSGEQ7, Measures the voltage value for each frequency band (0V-5V) 
#define strobePin 4      //connects to the Strobe Pin of the MSGEQ7, controls the Multiplexer and thus switches between the frequency bands
#define resetPin 8       //connects to the Reset Pin of the MSGEQ7, controls the Multiplexer and let's it restart with the lowest frequency band
#define ledred 3
#define ledyellow 5                        //connects to the Gate of the BUZ11 MOSFET of the RED LED (Bass) and creates a PWM signal with variable duty cycle which depends of the peak value of the low frequencies to control the brightness of the RED LED 
#define ledgreen 6      //connects to the Gate of the BUZ11 MOSFET of the GREEN LED (Middle) and creates a PWM signal with variable duty cycle which depends of the peak value of the middle frequencies to control the brightness of the GREEN LED
#define ledblue 9       //connects to the Gate of the BUZ11 MOSFET of the BLUE LED (High) and creates a PWM signal with variable duty cycle which depends of the peak value of the high frequencies to control the brightness of the BLUE LED
#define filter 85*7        //There will always be some noises which the analogpin will receive. With this filter value we can ignore the very low peaks of the output of the MSGEQ7. Fell free to adjust this value to your liking
#define singlefilter 25
#define read_delay 5

#define buttonPin 2
#define button_onTime 50
#define button_offTime 600

#define progled 13

#define maxprog 3

int spectrumValue[7]; //Integer variable to store the 10bit values of the frequency bands
int tmp = 0;

int program = 0;

//#define seri
//#define pwm_cap

#ifdef pwm_cap
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
#endif
void setup() {
#ifdef seri
  Serial.begin(9600);
  Serial.println("Starting...");
  //needed to output the values of the frequencies bands on the serial monitor
#endif
  pinMode(analogPin, INPUT);  //defines analog pin A0 as an Input
  pinMode(strobePin, OUTPUT); //defines strobe pin 2 as Output
  pinMode(resetPin, OUTPUT);  //defines reset pin 3 as Output
  pinMode(ledred, OUTPUT);    //defines ledred pin 9 as Output
  pinMode(ledblue, OUTPUT);   //defines ledblue pin 10 as Output
  pinMode(ledgreen, OUTPUT);  //defines ledgreen pin 11 as Output
  pinMode(ledyellow, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(progled, OUTPUT);
  #ifdef pwm_cap
  setPwmFrequency(3, 64);
  setPwmFrequency(5, 256);
  setPwmFrequency(6, 256);
  setPwmFrequency(9, 64);
  setPwmFrequency(10, 64);
  setPwmFrequency(11, 64);
  #endif
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
}

void loop() {
  checkButton();
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);              //change from high to low starts the output of the mutliplexer from the beginning
  tmp = 0;
  for (int i = 0; i < 7; i++) {             //for loop goes through this cycle 7 times to get the values for each frequency band
    digitalWrite(strobePin, LOW);           //puts strobe pin low to output the frequency band
    delayMicroseconds(30);                  //wait until output value of MSGEQ7 can be measured (see timing diagram in the datasheet)
    spectrumValue[i] = analogRead(analogPin); //put analog DC value in the spectrumValue variable
    tmp += spectrumValue[i];
    if (spectrumValue[i] < singlefilter) {
      spectrumValue[i] = 0;
    }

    //if the received value is below the filter value it will get set to 0
    spectrumValue[i] = map(spectrumValue[i], 0, 1023, 0, 255); //transform the 10bit analog input value to a suitable 8bit PWM value
    //    Serial.print(spectrumValue[i]);         //outputs the PWM value on the serial monitor
    //    Serial.print(" ");
    digitalWrite(strobePin, HIGH);          //puts the strobe pin high to get ready for the next cycle

  }
  if (tmp < filter) {
    for (int i = 0; i < 7; i++) {
      spectrumValue[i] = 0;
    }
  }
  if (program == 0) {
    analogWrite(ledred, (spectrumValue[1] * 2 / 5 + spectrumValue[0] * 3 / 5));
    analogWrite(ledyellow, spectrumValue[2]);
    analogWrite(ledgreen, spectrumValue[3]);
    analogWrite(ledblue, spectrumValue[4] * 2 / 3 + spectrumValue[5] * 1 / 3);
  } else if (program == 1) {
    analogWrite(ledred, (spectrumValue[1] * 2 / 5 + spectrumValue[0] * 3 / 5));
    analogWrite(ledyellow, spectrumValue[2]);
    analogWrite(ledgreen, spectrumValue[3]);
    analogWrite(ledblue, spectrumValue[4]);
  } else if (program == 2) {
    analogWrite(ledred, (spectrumValue[1] * 2 / 5 + spectrumValue[0] * 3 / 5));
    analogWrite(ledyellow, spectrumValue[2] / 2 + spectrumValue[3] / 2);
    analogWrite(ledgreen, spectrumValue[4] * 5 / 6 + spectrumValue[3] / 6);
    analogWrite(ledblue, (spectrumValue[5] * 2 / 3 + spectrumValue[6] / 3));
  }
  delay(read_delay);
}

void checkButton() {
  if (digitalRead(buttonPin) == HIGH) {
    digitalWrite(ledred, LOW);
    digitalWrite(ledblue, LOW);
    digitalWrite(ledgreen, LOW);
    digitalWrite(ledyellow, LOW);
    program += buttonData();
    program %= maxprog;
#ifdef seri
    Serial.println(program);
#endif
    lightProg();
  }
}

void lightProg() {
  delay(300);
  for (int i = 0; i <= program; i++) {
    digitalWrite(progled, HIGH);
    delay(200);
    digitalWrite(progled, LOW);
    delay(200);
  }
  delay(600);
}

int buttonData() {
  int toreturn = 0;
  while (1000) {
    unsigned long onTime = millis();
    if (digitalRead(buttonPin) == HIGH) {
      while (digitalRead(buttonPin) == HIGH) {
      }
    } else {
      return toreturn;
    }
    onTime = millis() - onTime;
#ifdef seri
    Serial.println(onTime);
#endif
    if (onTime > button_onTime) {
      toreturn++;
    }
    unsigned long offtime = millis();
    while (digitalRead(buttonPin) == LOW) {
      if (millis() - offtime > button_offTime) {
        return toreturn;
      }
    }
  }
  return toreturn;
}


