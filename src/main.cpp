#include <Arduino.h>

static int speaker_pin = D1;
static int straightKey_pin = D5;

static int cw_freq = 600;
static int wpm = 20;

static int volume = 5;

void playTone(int channel, int frequency);
void endTone(int channel);
void straightKeyToggled();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(speaker_pin, OUTPUT);
  analogWrite(speaker_pin, 0);

  pinMode(straightKey_pin, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.print("Starting loop");
}

static int key_state = 0;
void loop() {
  int new_state = 0;

  if (digitalRead(straightKey_pin) == 0) {
    new_state = 1;
  }

  if (new_state != key_state) {
    if (new_state == 1) {
      playTone(speaker_pin, cw_freq);
    } else {
      endTone(speaker_pin);
    }

    key_state = new_state;
  }

  if (key_state == 1) {
      digitalWrite(LED_BUILTIN, 0);
  } else {
      digitalWrite(LED_BUILTIN, 1);
  }
}

void playTone(int pin, int frequency) {
  analogWriteFreq(frequency);
  analogWrite(pin, volume);
}

void endTone(int pin) {
  analogWrite(pin, 0);
}