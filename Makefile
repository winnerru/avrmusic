MCU = atmega328p
PROGRAMMER = usbasp

TARG = firmware

CC = avr-g++
OBJCOPY = avr-objcopy
AVRSIZE = avr-size

SRCS = main.c

OBJS = $(SRCS:.c=.o)

CFLAGS = -g -mmcu=$(MCU) -Os -Wall -Werror -fpermissive # -mcall-prologues
LDFLAGS = -g -mmcu=$(MCU) -Os -Wall -Werror

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@.elf  $(OBJS) -lm
	$(AVRSIZE) -C --mcu=$(MCU) $@.elf
	$(OBJCOPY) -O binary -R .eeprom -R .nwram  $@.elf $@.bin
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram  $@.elf $@.hex
	$(OBJCOPY) -O ihex -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings $@.elf $@.eep

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

upload:
	avrdude -v -p $(MCU) -c $(PROGRAMMER) -U flash:w:$(TARG).hex
#	avrdude -v -p $(MCU) -c $(PROGRAMMER) -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m

clean:
	rm -fv *.elf *.bin *.hex  $(OBJS) *.map *.eep
