#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: unxirxm3.mk,v $
#
# $Revision: 1.12 $
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


# Makefile for IRIX/mips
ASM=
AFLAGS=

SOLAR_JAVA*=TRUE
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

CC= cc
CXX= CC
CFLAGS= -c
CDEFS+= -DSTLPORT_VERSION=0x450 -D_USE_NAMESPACE=1 -DNEW_SOLAR
CFLAGSCC=$(ARCH_FLAGS)
CFLAGSCXX=      -LANG:ansi-for-init-scope=OFF -LANG:std=ON -LANG:libc_in_namespace_std=ON $(ARCH_FLAGS)

# Compiler flags for compiling static object in multi threaded environment with graphical user interface
CFLAGSOBJGUIMT=
# Compiler flags for compiling static object in multi threaded environment with character user interface
CFLAGSOBJCUIMT=
# Compiler flags for compiling shared object in multi threaded environment with graphical user interface
CFLAGSSLOGUIMT=-KPIC
# Compiler flags for compiling shared object in multi threaded environment with character user interface
CFLAGSSLOCUIMT=-KPIC
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
CFLAGSWALLCC=-fullwarn
CFLAGSWERRCC=

# exception flags
CFLAGSEXCEPTIONS=-LANG:exceptions=ON
CFLAGS_NO_EXCEPTIONS=-LANG:exceptions=OFF
#CFLAGS_NO_EXCEPTIONS=

STATIC=                 -Wl,-Bstatic
DYNAMIC=                -Wl,-Bdynamic

#
# To use the map files, you need to have a gcc_specs file which contains:
# *linker:
# /path/to/bin/ld.sh
# where ld.sh is a wrapper script that does some conversion of the
# map files on the fly.
#
LINK= CC
LINKFLAGS=      -L/usr/lib32 -Wl,-no_unresolved
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
STDLIBGUIMT=    $(THREADLIB) $(DYNAMIC) -lX11 -lm -lc
STDLIBCUIMT=    $(THREADLIB) $(DYNAMIC) -lc -lm
STDSHLGUIMT=    -L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lX11 -lm -lc
STDSHLCUIMT=    -L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lm -lc
THREADLIB=              -lpthread

#LIBSTLPORT=$(DYNAMIC) -lstlport_mipspro
LIBSTLPORT=$(DYNAMIC)
LIBSTLPORTST=
#LIBSTLPORTST= -lstlport_mipspro

LIBMGR=                 ar
LIBFLAGS=               -r

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=                             irc
RCFLAGS=                -fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=             im
DLLPRE=                 lib
DLLPOST=                .so


LDUMP=c++filt


