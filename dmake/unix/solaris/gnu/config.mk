# This is the Solaris gcc configuration file for DMAKE
#	It modifies the value of CC to be gcc
#

CC = gcc

# disable a gcc bug when compiling runargv.c
runargv.o ?= CFLAGS += -g
