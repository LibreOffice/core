#
# (c) Copyright 1990 Conor P. Cahill (uunet!virtech!cpcahil).  
# You may copy, distribute, and use this software as long as this
# copyright statement is not removed.
#
#
# This is the Makefile for the malloc debugging library
#
# $Id: makefile,v 1.1.1.1 2000-09-22 15:33:26 hr Exp $
#
CC=cc
# for System V systems use this CFLAGS
#CFLAGS=-g -DSYS5
# else for BSD use:
#CFLAGS=-g
LINT=lint
SHARCMD=shar -o mallocshar -l50 -x -a -n Malloclib
SHELL=/bin/sh

LIB=libmalloc.a

SRCS=	malloc.c	\
	free.c		\
	realloc.c	\
	calloc.c	\
	string.c	\
	mlc_chk.c	\
	mlc_chn.c	\
	memory.c	\
	tostring.c	\
	m_perror.c	\
	m_init.c	\
	mallopt.c	\
	dump.c

OBJS=	malloc.o	\
	free.o		\
	realloc.o	\
	calloc.o	\
	string.o	\
	mlc_chk.o	\
	mlc_chn.o	\
	memory.o	\
	tostring.o	\
	m_perror.o	\
	m_init.o	\
	mallopt.o	\
	dump.o

TESTS=testmlc testmem

all:	$(LIB) $(TESTS)

clean:  
	rm -f $(TESTS) pgm $(LIB) *.o *.ln

sharfile:
	$(SHARCMD) Makefile README patchlevel *.[ch3] 
	
$(LIB): $(OBJS)
	ar ru $(LIB) $(OBJS)
	-if test -s /bin/ranlib; then /bin/ranlib $(LIB); else exit 0; fi
	-if test -s /usr/bin/ranlib; then /usr/bin/ranlib $(LIB); else exit 0; fi

testmlc:	$(LIB) testmlc.o
	$(CC) -o $@ testmlc.o $(LIB)

testmem:	$(LIB) testmem.o
	$(CC) -o $@ testmem.o $(LIB)

lint:	
	$(LINT) $(CFLAGS) $(SRCS) testmlc.c testmem.c


$(OBJS):	malloc.h

tostring.o malloc.o dump.o:	tostring.h
