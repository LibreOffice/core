#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: unxlngp.mk,v $
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

# mak file for unxlngp
ASM=
AFLAGS=

# _PTHREADS is needed for the stl
CDEFS+=$(PTHREAD_CFLAGS) -D_PTHREADS -D_REENTRANT -DGLIBC=2
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
ARCH_FLAGS*=

CXX*=g++
CC*=gcc
CFLAGS=-nostdinc -c
CFLAGSCC= -pipe $(ARCH_FLAGS)
CFLAGSCXX= -pipe -fguiding-decls -fno-rtti -fno-exceptions $(ARCH_FLAGS)

# Exceptions increase the size of shared libraries by 50% !!
.IF "$(PRJNAME)"=="usr" || "$(PRJNAME)"=="uno" || "$(PRJNAME)"=="starone" || "$(PRJNAME)"=="schedule" || "$(PRJNAME)"=="one" || "$(SET_EXEPTIONS)"!=""
CFLAGSCXX += -fexceptions
.ENDIF
PICSWITCH:=-fPIC
#STDOBJVCL=$(L)/salmain.o
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=$(PICSWITCH)
CFLAGSSLOCUIMT=$(PICSWITCH)
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
CFLAGSOPT=-O2
CFLAGSNOOPT=
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
LINKFLAGS=-melf_i386 -z nodefs -dynamic-linker /lib/ld-linux.so.2 /nw386/dev/s/solenv/unxlngp/usr/lib/crti.o /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtbegin.o
LINKFLAGSAPPGUI=/usr/lib/crt1.o
LINKFLAGSSHLGUI=-noinhibit-exec -warn-once -G
LINKFLAGSAPPCUI=/usr/lib/crt1.o
LINKFLAGSSHLCUI=-noinhibit-exec -warn-once -G
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

.IF "$(NO_BSYMBOLIC)"==""
.IF "$(PRJNAME)" != "envtest"
LINKFLAGSSHLGUI+=-Bsymbolic
LINKFLAGSSHLCUI+=-Bsymbolic
.ENDIF
.ENDIF				# "$(NO_BSYMBOLIC)"==""

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

STDLIBCPP=-lstdc++

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBGUIMT=-Bdynamic -lcrypt -ldl -lpthread -lm -lgcc -lc /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtend.o /nw386/dev/s/solenv/unxlngp/usr/lib/crtn.o
STDLIBCUIMT=-Bdynamic -lcrypt -ldl -lpthread -lm -lgcc -lc /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtend.o /nw386/dev/s/solenv/unxlngp/usr/lib/crtn.o
STDSHLGUIMT=-Bdynamic -lX11 -lXext -lcrypt -ldl -lpthread -lm -lgcc -lc /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtend.o /nw386/dev/s/solenv/unxlngp/usr/lib/crtn.o
STDSHLCUIMT=-Bdynamic -lcrypt -ldl -lpthread -lm -lgcc -lc /nw386/dev/s/solenv/unxlngp/lib/gcc-lib/i586-pc-linux-gnu/egcs-2.90.29/crtend.o /nw386/dev/s/solenv/unxlngp/usr/lib/crtn.o

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

