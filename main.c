#include "config.h"

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "waveform.h"
#include "melody1.h"


const tMelody* melody;

typedef struct {
    uint16_t fNote;
    uint16_t sPos;
    uint16_t ePos;
} tPlayerState;

tPlayerState channelState[4];

// Обработчик прерывания по переполнению таймера 1
// частота срабатывания - 31,25 КГц
ISR(TIMER1_OVF_vect) {
    static uint16_t _tick = -2;
    static uint16_t _tickMax = 0xffff;
    int8_t sample = 0;

    if (melody) {
        for (uint8_t i = 0; i < 4; i++) {
            uint8_t _s_index = channelState[i].sPos >> 8;
            uint8_t _e_index = channelState[i].ePos >> 7;
            if ((channelState[i].sPos += channelState[i].fNote) >= 0x7900) {
                channelState[i].sPos -= 0x7900;
            }
            if (++channelState[i].ePos > 16256) {
                channelState[i].ePos = 16256;
            }
            uint8_t sineVal = pgm_read_byte(&(s_sineTable[_s_index]));
            uint8_t envelopeVal = pgm_read_byte(&(s_envelopeTable[_e_index]));
            int8_t _sample = (int8_t)((((int16_t)sineVal - 0x80) * envelopeVal) >> 8);
            sample += _sample;
        }

        OCR1AL = (uint8_t)((int16_t)sample + 0x80) >> 1;

        if (++_tick >= _tickMax) {
            _tick = 0;

            uint8_t channel = pgm_read_byte((uint8_t*)melody);
            uint8_t note = pgm_read_byte((uint8_t*)melody + 1);
            uint8_t delay = pgm_read_word((uint8_t*)melody + 2);

            if ( 0 == delay ) {
                OCR1AL = 0x80;
                _tick = -2;
                melody = NULL;
            } else {
                _tickMax = delay << 6;
                channelState[channel].ePos = 0;
                channelState[channel].sPos = 0;
                channelState[channel].fNote = pgm_read_word(&(notes[note]));
                melody++;
            }
        }
    }
}

// Инициализация устройства
void init() {

    // Установка режима работы портов
    DDRB  = 0b00000010;
    PORTB = 0b00000000;

    DDRC  = 0b00000000;
    PORTC = 0b00000000;

    DDRD  = 0b00000000;
    PORTD = 0b00000000;

    // Таймер 1 для организации ШИМ звукового генератора на выводе OCR1A
    TCCR1A = 0b10000001;                      // normal PWM mode for ORC1A
    TCCR1B = 0b00001001;                      // FAST 8 bit PWM, no prescaller
    OCR1AL = 0x80;
    TIMSK1 = 0b00000001;                      // enable interrupt on timer 1 overflow

    sei();

}

int main() {
    init();

    melody = sMelody;

    for (;;) {
        _delay_ms(125);
    }

    return 0;
}
