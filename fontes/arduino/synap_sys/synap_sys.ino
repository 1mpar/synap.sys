#include "parameters.h"
#include "peers.h"
#include "pots.h"

String str;

void setup() {
  if (USE_LED13_NO_TIME)
    pinMode(13, OUTPUT);

  Serial.begin(9660);

  Serial.println("\n\n\n\n\n/init/begin");

  if (DEBUG_INIT) {
    Serial.println("/DEBUG/ENABLED");
  }

  if (DEBUG_INIT) {
    str = "/DEBUG/NUM_PINS/";
    str += NUM_PINS;
    Serial.println(str);
    str = "/DEBUG/NUM_POTS/";
    str += NUM_POTS;
    Serial.println(str);
  }


  Serial.println("\n/init/pins/check");
  for (byte b = 0; b < NUM_PINS; b++) {

    pinMode(PINS[b], OUTPUT);
    digitalWrite(PINS[b], LOW);
    pinMode(PINS[b], INPUT);
    if (DEBUG_INIT) {
      str = "/DEBUG/PIN/";
      str += b;
      str += "/INPUT/";
      str += digitalRead(PINS[b]) ? "ON" : "OFF";
      Serial.println(str);
    }
  }
  Serial.println("\n/init/pins/ok");

  if (DEBUG_INIT) {
    Serial.println("/DEBUG/STARTED\n");
  }

  str = "\n/init/delays/";
  str += DELAY_INPUT;
  str += "/";
  str += DELAY_OUTPUT;
  str += "/";
  str += DELAY_LOOP;
  Serial.println(str);

  resetOldPeers();
  resetPeers();

  Serial.println("\n/init/end");

}

byte c;
byte lastPin = 0;
long potsOldValues[NUM_POTS];
long lastShowTime, lastLoopPinsTime, lastLoopPotsTime;

void loop() {
  if (DEBUG) Serial.println("**********************");

  long showTime = millis();
  if ((showTime - lastShowTime) > (DELAY_SHOW_TIME)) {
    if (USE_LED13_NO_TIME) {
      digitalWrite(13, !digitalRead(13));
    } else {
      str = "/loop/time/";
      str += showTime;
      Serial.println(str);
    }
    lastShowTime = showTime;
  }

  long loopPinsTime = millis();
  if (DELAY_LOOP_PINS < (loopPinsTime - lastLoopPinsTime)) {
    loopPins();
    lastLoopPinsTime = loopPinsTime;
  }

  long loopPotsTime = millis();
  if (DELAY_LOOP_POTS < (loopPotsTime - lastLoopPotsTime)) {
    loopPots();
    lastLoopPotsTime = loopPotsTime;
  }

  delay(DELAY_LOOP);

}

byte lastPotRead = 0;
void loopPots() {
  byte p = lastPotRead++;
  if (lastPotRead >= NUM_POTS)
    lastPotRead = 0;
    
  signed long v = analogRead(POTS[p]);
  signed long old = potsOldValues[p];
  if(DEBUG_POTS){
    String str = "/DEBUG/POTS/";
    str += p;
    str += "/";
    str += old;
    str += "/";
    str += v;
    str += "/";
    str += abs(old - v);
    str += "/";
    str += POT_LAG;
    str += "/";
    str += abs(old - v) > POT_LAG;
  }
  
  if (abs(old - v) > POT_LAG) {
    String str = "/POT/";
    str += p;
    str += "/";
    str += v;
    Serial.println(str);
    potsOldValues[p] = v;
  }
}


void loopPins() {
  c = 0;

  for (byte p_out = 0; p_out < NUM_PINS && c < NUM_PEERS; p_out++) {
    bool nextOutput = false;

    // retorna o ultimo pino usado para o estado INP
    pinMode(PINS[lastPin], INPUT);
    lastPin = p_out;

    delay(DELAY_OUTPUT);

    pinMode(PINS[p_out], OUTPUT);
    digitalWrite(PINS[p_out], HIGH);


    if (DEBUG_OUTPUT) {
      str = "\n/DEBUG/PIN/OUTPUT/";
      str += p_out;
      Serial.println(str);
    } // DEBUG

    for (byte p_in = 0; p_in < NUM_PINS; p_in++) {
      if (p_in == p_out) continue;

      delay(DELAY_INPUT);
      bool pinState = digitalRead(PINS[p_in]);

      bool nextInput = false;

      if (DEBUG_INPUT) {
        str = "/DEBUG/PIN/";
        str += p_in;
        str += "/INPUT/";
        str += pinState ? "ON" : "OFF";
        Serial.println(str);
      } // DEBUG

      for (byte pair = 0; pair < NUM_PEERS  && c < NUM_PEERS; pair++) {
        if (DEBUG_PAIR2) {
          str = "/DEBUG/PAIR2/";
          str += pair;
          str += "/";
          str += pairPin1(pair);
          str += "/";
          str += pairPin2(pair);
          Serial.println(str);
        }// DEBUG

        if (usedPin(pair, p_in)) {
          nextInput = true;
          break;
        }
      }
      if (nextInput) continue;

      if (pinState) {
        if (DEBUG > 5) {
          str = "\n/DEBUG/PIN/ON/";
          str += p_out;
          str += "/WITH/";
          str += p_in;
          str += "\n";
          Serial.println(str);
        } // DEBUG

        setPair(c++, p_in, p_out);

        nextOutput = true; // vou para a proxima entrada, apenas um par por output;
        break;
      }
    } //FINDA FOR INPUT

    digitalWrite(PINS[p_out], LOW);
    if (nextOutput) continue;
  } // FINALIZA FOR OUTPUT

  showPeers();
  movePeers();
}
