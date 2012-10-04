#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

# mk file for unxsogs
ASM=/usr/ccs/bin/as
AFLAGS=-P

SOLAR_JAVA*=TRUE
JAVAFLAGSDEBUG=-g

# _PTHREADS is needed for the stl
CDEFS+=-D_PTHREADS -D_REENTRANT -DSYSV -DSUN -DSUN4 -D_POSIX_PTHREAD_SEMANTICS -D_USE_NAMESPACE=1 


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
CFLAGS=-c
CFLAGSCC= -pipe $(ARCH_FLAGS)

CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

CFLAGSCXX= -pipe $(ARCH_FLAGS)
.IF "$(HAVE_THREADSAFE_STATICS)" != "TRUE"
CFLAGSCXX += -fno-threadsafe-statics
.END

PICSWITCH:=-fPIC
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

STATIC		= -Wl,-Bstatic
DYNAMIC		= -Wl,-Bdynamic

LINK*=$(CXX)
LINKC*=$(CC)

LINKFLAGS=
#LINKFLAGSAPPGUI=-Wl,-export-dynamic
LINKFLAGSSHLGUI=-shared
#LINKFLAGSAPPCUI=-Wl,-export-dynamic
LINKFLAGSSHLCUI=-shared
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

LINKVERSIONMAPFLAG=-Wl,--version-script

# enable visibility define in "sal/types.h"
.IF "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CDEFS += -DHAVE_GCC_VISIBILITY_FEATURE
.ENDIF # "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"

.IF "$(HAVE_SFINAE_ANONYMOUS_BROKEN)" == "TRUE"
CDEFS += -DHAVE_SFINAE_ANONYMOUS_BROKEN
.ENDIF # "$(HAVE_SFINAE_ANONYMOUS_BROKEN)" == "TRUE"

# Reihenfolge der libs NICHT egal!

STDLIBCPP=-lstdc++

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBGUIMT=$(DYNAMIC) -lpthread -lthread -lm
STDLIBCUIMT=$(DYNAMIC) -lpthread -lthread -lm
# libraries for linking shared libraries
STDSHLGUIMT=$(DYNAMIC) -lpthread -lthread -lm
STDSHLCUIMT=$(DYNAMIC) -lpthread -lthread -lm

STDLIBGUIMT+=-lX11

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

DLLPRE=lib
DLLPOST=.so
