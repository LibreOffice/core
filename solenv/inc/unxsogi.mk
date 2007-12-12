#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unxsogi.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: kz $ $Date: 2007-12-12 13:21:46 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

# mk file for unxsogi
ASM=/usr/ccs/bin/as
AFLAGS=-P

SOLAR_JAVA=TRUE
JAVAFLAGSDEBUG=-g

# _PTHREADS is needed for the stl
CDEFS+=-D_PTHREADS -D_REENTRANT -DSYSV -DSUN -DSUN4 -D_POSIX_PTHREAD_SEMANTICS -DSTLPORT_VERSION=$(STLPORT_VER) -D_USE_NAMESPACE=1 


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
LINKFLAGSAPPGUI=-Wl,-export-dynamic
LINKFLAGSSHLGUI=-shared
LINKFLAGSAPPCUI=-Wl,-export-dynamic
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

# Reihenfolge der libs NICHT egal!

STDLIBCPP=-lstdc++

STDOBJVCL=$(L)$/salmain.o
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

.IF "$(STLPORT_VER)" >= "500"
LIBSTLPORT=$(DYNAMIC) -lstlport
LIBSTLPORTST=$(STATIC) -lstlport $(DYNAMIC)
.ELSE
LIBSTLPORT=$(DYNAMIC) -lstlport_gcc
LIBSTLPORTST=$(STATIC) -lstlport_gcc $(DYNAMIC)
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

DLLPOSTFIX=sogi
DLLPRE=lib
DLLPOST=.so
