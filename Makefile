CC=gcc
CFLAGS=-g -Wall `pkg-config --cflags gtk+-2.0`
LDFLAGS=`pkg-config --libs gtk+-2.0` -lm

.PHONY: all clean

all: imagestamp

clean:
	-rm *.o imagestamp
