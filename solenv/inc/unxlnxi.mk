#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: unxlnxi.mk,v $
#
# $Revision: 1.13 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
# mak file fuer unxlnxi
ASM=
AFLAGS=

# _PTHREADS is needed for the stl
CDEFS+=$(PTHREAD_CFLAGS) -D_PTHREADS
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CDEFS+=-DNO_INET_ON_DEMAND -DX86 -DNEW_SOLAR

# kann c++ was c braucht??

.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
.IF "$(debug)"==""
JAVA_RUNTIME=-ljava
.ELSE
JAVA_RUNTIME=-ljava_g
.ENDIF
.ENDIF                                        

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=-mpentium

CXX*=g++
CC*=gcc
.IF "$(GLIBC)"=="2"
CFLAGS=-c -I. -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC)
.ELSE
CFLAGS=-c -I. -I/usr/solar/inc/pthread_provenzano -I$(INC) -I$(INCLOCAL) -I$(INCGUI) -I$(INCCOM) $(SOLARINC)
.ENDIF
CFLAGSCC=-pipe -fguiding-decls $(ARCH_FLAGS)
CFLAGSCXX=-pipe -fguiding-decls $(ARCH_FLAGS)
PICSWITCH:=-fpic
#STDOBJVCL=$(L)/salmain.o
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=$(PICSWITCH)
CFLAGSSLOCUIMT=$(PICSWITCH)
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-O2
CFLAGSNOOPT=-O2
CFLAGSOUTOBJ=-o

CFLAGSWARNCC=
CFLAGSWARNCXX=$(CFLAGSWARNCC) -Wno-ctor-dtor-privacy
# -Wshadow does not work for C with nested uses of pthread_cleanup_push:
CFLAGSWALLCC=-Wall -Wextra -Wendif-labels
CFLAGSWALLCXX=$(CFLAGSWALLCC) -Wshadow -Wno-ctor-dtor-privacy
CFLAGSWERRCC=-Werror

STATIC		= -Bstatic
DYNAMIC		= -Bdynamic

LINK=ld
.IF "$(GLIBC)"=="2"
LINKFLAGS=-melf_i386 -z nodefs -dynamic-linker /lib/ld-linux.so.2 /usr/lib/crti.o /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtbegin.o
.ELSE
LINKFLAGS=-melf_i386 -z nodefs -dynamic-linker /lib/ld-linux.so.1 /usr/lib/crti.o /usr/lib/crtbegin.o -lpthread_init
.ENDIF
LINKFLAGSAPPGUI=/usr/lib/crt1.o 
LINKFLAGSSHLGUI=-noinhibit-exec -warn-once -Bsymbolic -G
LINKFLAGSAPPCUI=/usr/lib/crt1.o 
LINKFLAGSSHLCUI=/usr/lib/crt1.o
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

SONAME_SWITCH=-Wl,-h

# reihenfolge der libs NICHT egal!

# standard C++ Library
#
# das statische dazulinken der libstdc++ macht jede shared library um 50k 
# (ungestrippt) oder so groesser, auch wenn sie ueberhaupt nicht gebraucht 
# wird. Da muessen wir uns was besseres ueberlegen.
#
# Da mit der neuen libc.so.6 (libc-2.0.5.so) sowieso eine System-Library
# ausgeliefert werden muss, linken wir auch die libstdc++.so dynamisch.

.IF "$(GLIBC)"=="2"
STDLIBCPP=-lstdc++
.ELSE
STDLIBCPP=-Bstatic -lstdc++ -Bdynamic
.ENDIF

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

.IF "$(WORK_STAMP)=="MIX364"
.IF "$(GLIBC)"=="2"
STDLIBGUIMT=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDLIBCUIMT=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDSHLGUIMT=-Bdynamic -lX11 -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDSHLCUIMT=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
.ELSE
STDLIBGUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDLIBCUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDSHLGUIMT=-Bdynamic -lX11 -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDSHLCUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
.ENDIF
.ELSE
.IF "$(GLIBC)"=="2"
STDLIBGUIMT=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDLIBCUIMT=-Bdynamic -lgcc -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDSHLGUIMT=-Bdynamic -lX11 -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
STDSHLCUIMT=-Bdynamic -lm -lc -ldl /usr/lib/gcc-lib/i586-pc-linux-gnu/2.8.1/crtend.o /usr/lib/crtn.o 
.ELSE
STDLIBGUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDLIBCUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDSHLGUIMT=-Bdynamic -lX11 -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
STDSHLCUIMT=-Bdynamic -lpthread -lgcc -lm -lc -ldl /usr/lib/crtend.o /usr/lib/crtn.o 
.ENDIF
.ENDIF

LIBMGR=ar
LIBFLAGS=-r
# LIBEXT=.so

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

.IF "$(WORK_STAMP)"!="LVM364"
.IF "$(WORK_STAMP)"!="MIX364"
DLLPOSTFIX=li
.ENDIF
.ENDIF
DLLPRE=lib
DLLPOST=.so

LDUMP=cppfilt /b /n /o /p

