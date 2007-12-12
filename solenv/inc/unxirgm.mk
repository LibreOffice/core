#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unxirgm.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: kz $ $Date: 2007-12-12 13:19:17 $
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


# Makefile for IRIX/mips using GNU compilers 2.95.2 for n32 ABI
ASM=
AFLAGS=

SOLAR_JAVA=TRUE
JAVADEF=-DSOLAR_JAVA
#JAVAFLAGSDEBUG=-g

.IF "$(debug)"==""
JAVA_RUNTIME=-ljava
.ELSE
JAVA_RUNTIME=-ljava_g
.ENDIF

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=

CC= gcc
CXX= g++
CFLAGS=-c
CDEFS+= -D_PTHREADS -DSTLPORT_VERSION=0x450 -D_USE_NAMESPACE=1 -DNEW_SOLAR
CFLAGSCC=$(ARCH_FLAGS)
CFLAGSCXX=$(ARCH_FLAGS)
PICSWITCH:=-fpic

# Compiler flags for compiling static object in multi threaded environment with graphical user interface
CFLAGSOBJGUIMT=
# Compiler flags for compiling static object in multi threaded environment with character user interface
CFLAGSOBJCUIMT=
# Compiler flags for compiling shared object in multi threaded environment with graphical user interface
CFLAGSSLOGUIMT=$(PICSWITCH)
# Compiler flags for compiling shared object in multi threaded environment with character user interface
CFLAGSSLOCUIMT=$(PICSWITCH)
# Compiler flags for profiling
CFLAGSPROF=
# Compiler flags for debugging
CFLAGSDEBUG=-g
CFLAGSDBGUTIL= 
# Compiler flags for enabling optimizations
CFLAGSOPT=-O2
# Compiler flags for disabling optimizations
CFLAGSNOOPT=-O0
# Compiler flags for describing the output path
CFLAGSOUTOBJ=-o

CFLAGSWARNCC=-w
CFLAGSWALLCC=-Wall
CFLAGSWERRCC=-Werror

# exception flags
CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

STATIC=			-Wl,-Bstatic
DYNAMIC=		-Wl,-Bdynamic

#
# To use the map files, you need to have a gcc_specs file which contains:
# *linker:
# /path/to/bin/ld.sh
# where ld.sh is a wrapper script that does some conversion of the
# map files on the fly.
#
LINK= g++
LINKFLAGS=	-L/usr/lib32 -Wl,-no_unresolved
LINKVERSIONMAPFLAG= -Wl,-exports_file

LINKFLAGSAPPGUI= $(THREADLIB)
LINKFLAGSAPPCUI= $(THREADLIB)
LINKFLAGSSHLGUI= $(THREADLIB)
LINKFLAGSSHLCUI= $(THREADLIB)

LINKFLAGSAPPGUI+= -Wl,-multigot
LINKFLAGSAPPCUI+= -Wl,-multigot
LINKFLAGSSHLGUI+= -shared 
LINKFLAGSSHLCUI+= -shared

LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG= -g
LINKFLAGSOPT=

LINKFLAGSSHLGUI += -Wl,-Bsymbolic
LINKFLAGSSHLCUI += -Wl,-Bsymbolic

APPLINKSTATIC=-Bstatic
APPLINKSHARED=-Bsymbolic

SONAME_SWITCH = -Wl,-soname -Wl,

# Sequence of libraries DOES matter!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIMT=	$(THREADLIB) $(DYNAMIC) -lX11 -lm -lgcc -lc
STDLIBCUIMT=	$(THREADLIB) $(DYNAMIC) -lgcc -lc -lm
STDSHLGUIMT=	-L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lX11 -lm -lgcc -lc
STDSHLCUIMT=	-L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lm -lgcc -lc
THREADLIB=		-lpthread

.IF "$(STLPORT_VER)" >= "500"
LIBSTLPORT=$(DYNAMIC) -lstlport
LIBSTLPORTST= -lstlport
.ELSE
LIBSTLPORT=$(DYNAMIC) -lstlport_gcc
LIBSTLPORTST= -lstlport_gcc
.ENDIF

LIBMGR=			ar
LIBFLAGS=		-r

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=				irc
RCFLAGS=		-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=		im
DLLPRE=			lib
DLLPOST=		.so


LDUMP=c++filt
