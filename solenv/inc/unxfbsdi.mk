#*************************************************************************
#
#   $RCSfile: unxfbsdi.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: svesik $ $Date: 2004-04-21 13:54:46 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

# mak file for unxfbsdi
ASM=$(CC)
AFLAGS=-x assembler-with-cpp -c $(CDEFS)

# filter for supressing verbose messages from linker
#not needed at the moment
#LINKOUTPUT_FILTER=" |& $(SOLARENV)$/bin$/msg_filter"

# this is a platform with JAVA support
SOLAR_JAVA*=TRUE
.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
.IF "$(debug)"==""
JAVA_RUNTIME=-ljava
.ELSE
JAVA_RUNTIME=-ljava_g
.ENDIF
.ENDIF

# name of C++ Compiler
CXX*=g++
# name of C Compiler
CC*=gcc

# filter for supressing verbose messages from linker
# not needed at the moment
LINKOUTPUT_FILTER=" |& $(SOLARENV)$/bin$/msg_filter"

# options for C and C++ Compiler
CDEFS+=	-D_USE_NAMESPACE=1 -DX86 -DNEW_SOLAR -DSTLPORT_VERSION=450 -DOSVERSION=$(OSVERSION)
CDEFS+= $(PTHREAD_CFLAGS) -D_REENTRANT

# flags for C and C++ Compile
CFLAGS+= -w -c $(INCLUDE)
CFLAGS+= -I/usr/X11R6/include

# flags for the C++ Compiler
CFLAGSCC= -pipe -fno-rtti
CFLAGSCXX= -pipe -fno-for-scope -fpermissive -fno-rtti

# Flags for enabling exception handling
CFLAGSEXCEPTIONS= -fexceptions
CFLAGS_NO_EXCEPTIONS= -fno-exceptions

# Compiler flags for compiling static object in single threaded
# environment with graphical user interface
CFLAGSOBJGUIST= -fPIC

# Compiler flags for compiling static object in single threaded
# environment with character user interface
CFLAGSOBJCUIST= -fPIC

# Compiler flags for compiling static object in multi threaded
# environment with graphical user interface
CFLAGSOBJGUIMT= -fPIC

# Compiler flags for compiling static object in multi threaded
# environment with character user interface
CFLAGSOBJCUIMT= -fPIC

# Compiler flags for compiling shared object in multi threaded
# environment with graphical user interface
CFLAGSSLOGUIMT=	-fPIC

# Compiler flags for compiling shared object in multi threaded
# environment with character user interface
CFLAGSSLOCUIMT=	-fPIC

# Compiler flags for profilin
CFLAGSPROF=     -pg

# Compiler flags for debugging
CFLAGSDEBUG=	-g
CFLAGSDBGUTIL=

# Compiler flags to enable optimizations
# -02 is broken for FreeBSD
CFLAGSOPT= -O

# Compiler flags to disable optimizations
# -0 is broken for STLport for FreeBSD
CFLAGSNOOPT= -O0

# Compiler flags for the output path
CFLAGSOUTOBJ= -o

# Enable all warnings
CFLAGSWALL=-Wall

# Set default warn level
CFLAGSDFLTWARN=-w

# switches for dynamic and static linking
STATIC=	-Wl,-Bstatic
DYNAMIC= -Wl,-Bdynamic

# name of linker
LINK=$(CC)

# default linker flags
# LINKFLAGSRUNPATH*=-Wl,-rpath\''$$ORIGIN'\'
LINKFLAGS=$(LINKFLAGSRUNPATH)

# linker flags for linking applications
LINKFLAGSAPPGUI= -Wl,--noinhibit-exec
LINKFLAGSAPPCUI= -Wl,--noinhibit-exec

# linker flags for linking shared libraries
LINKFLAGSSHLGUI= -shared
LINKFLAGSSHLCUI= -shared

LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

.IF "$(NO_BSYMBOLIC)"==""
.IF "$(PRJNAME)" != "envtest"
LINKFLAGSSHLGUI+= -Wl,-Bsymbolic
LINKFLAGSSHLCUI+= -Wl,-Bsymbolic
.ENDIF
.ENDIF

LINKVERSIONMAPFLAG=-Wl,--version-script

# Sequence of libs does matter !
STDLIBCPP=-lstdc++

# _SYSLIBS= -L/usr/lib -lm
# _X11LIBS= -L/usr/X11R6/lib -lXext -lX11
# _CXXLIBS= -L/usr/lib -lstdc++ -L/usr/local/lib

# default objectfilenames to link
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

# libraries for linking applications
STDLIBCUIST=-lm
STDLIBGUIST=-lXaw -lXt -lX11 -lm
STDLIBGUIMT=-lXaw -lXt -lX11 $(PTHREAD_LIBS) -lm
STDLIBCUIMT=$(PTHREAD_LIBS) -lm

# libraries for linking shared libraries
STDSHLGUIMT=-lXaw -lXt -lX11 -lXext $(PTHREAD_LIBS) -lm
STDSHLCUIMT=$(PTHREAD_LIBS) -lm

LIBSALCPPRT*=-Wl,--whole-archive -lsalcpprt -Wl,--no-whole-archive

# STLport always needs pthread.
LIBSTLPORT=$(DYNAMIC) -lstlport_gcc $(STDLIBCPP) $(PTHREAD_LIBS)
LIBSTLPORTST=$(STATIC) -lstlport_gcc $(DYNAMIC) $(PTHREAD_LIBS)

# name of library manager
LIBMGR=ar
LIBFLAGS=-r
LIBEXT=			.a

# tool for generating import libraries
IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

# platform specific identifier for shared libs
DLLPOSTFIX=fi
DLLPRE=lib
DLLPOST=.so
