#*************************************************************************
#
#   $RCSfile: unxlngppc.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: hr $ $Date: 2003-04-28 16:47:14 $
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

# Makefile for linux-ppc 
# Christer Gustavsson <cg@nocrew.org> 
ASM= 
AFLAGS= 

SOLAR_JAVA=TRUE 
JAVAFLAGSDEBUG=-g 

# filter for supressing verbose messages from linker
#not needed at the moment
#LINKOUTPUT_FILTER=" |& $(SOLARENV)$/bin$/msg_filter"

# _PTHREADS is needed for the stl 
CDEFS+=-DGLIBC=2 -D_PTHREADS -D_REENTRANT -DNEW_SOLAR -D_USE_NAMESPACE=1 -DSTLPORT_VERSION=400 -DPOWERPC -DPPC 

# this is a platform with JAVA support
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

# source code is still not signed versus unsigned char clean 
CFLAGS=-fsigned-char -nostdinc -c $(INCLUDE) -I$(SOLARENV)/unxlngppc/usr/include 
CFLAGSCC=-fsigned-char

# Flags for enabling exception handling
CFLAGSEXCEPTIONS=-fexceptions 
# Flags for disabling exception handling
CFLAGS_NO_EXCEPTIONS=-fno-exceptions 

# -fpermissive should be removed as soon as possible 
CFLAGSCXX= -pipe -fno-for-scope -fpermissive -frtti


#Note: the build is not consistent in that it links static librtaries
# libraries into dynamic libraries in places, so use -fPIC throughout
# until fixed.

# Compiler flags for compiling static object in single threaded environment with graphical user interface
CFLAGSOBJGUIST=-fPIC 
# Compiler flags for compiling static object in single threaded environment with character user interface
CFLAGSOBJCUIST=-fPIC
# Compiler flags for compiling static object in multi threaded environment with graphical user interface
CFLAGSOBJGUIMT=-fPIC
# Compiler flags for compiling static object in multi threaded environment with character user interface
CFLAGSOBJCUIMT=-fPIC
# Compiler flags for compiling shared object in multi threaded environment with graphical user interface
CFLAGSSLOGUIMT=-fPIC
# Compiler flags for compiling shared object in multi threaded environment with character user interface
CFLAGSSLOCUIMT=-fPIC 

# Compiler flags for profiling
CFLAGSPROF= 

# Compiler flags for debugging
CFLAGSDEBUG=-g 
CFLAGSDBGUTIL= 

# Compiler flags for enabling optimazations
CFLAGSOPT=-O2  -fno-schedule-insns -fno-strict-aliasing -fno-schedule-insns2

# Compiler flags for disabling optimazations
CFLAGSNOOPT=-O0

# Compiler flags for discibing the output path
CFLAGSOUTOBJ=-o 

# switches for dynamic and static linking
STATIC = -Wl,-Bstatic 
DYNAMIC = -Wl,-Bdynamic 

# name of linker
LINK*=gcc 
# default linker flags
LINKFLAGS= 

# linker flags for linking applications
LINKFLAGSAPPGUI= -Wl,-export-dynamic 
LINKFLAGSAPPCUI= -Wl,-export-dynamic 
# linker flags for linking shared libraries
LINKFLAGSSHLGUI= -shared 
LINKFLAGSSHLCUI= -shared 

LINKFLAGSTACK= 
LINKFLAGSPROF= 
LINKFLAGSDEBUG=-g 
LINKFLAGSOPT= 

.IF "$(NO_BSYMBOLIC)"=="" 
.IF "$(PRJNAME)" != "envtest" 
LINKFLAGSSHLGUI+=-Wl,-Bsymbolic 
LINKFLAGSSHLCUI+=-Wl,-Bsymbolic 
.ENDIF 
.ENDIF # "$(NO_BSYMBOLIC)"=="" 

LINKVERSIONMAPFLAG=-Wl,--version-script 

SONAME_SWITCH=-Wl,-h

# Sequence of libs does matter !

STDLIBCPP=-lstdc++ 

# default objectfilenames to link
STDOBJGUI= 
STDSLOGUI= 
STDOBJCUI= 
STDSLOCUI= 

# libraries for linking applications
STDLIBCUIST=-ldl -lm
STDLIBGUIMT=-lX11 -ldl -lpthread -lm
STDLIBCUIMT=-ldl -lpthread -lm
STDLIBGUIST=-lX11 -ldl -lm
# libraries for linking shared libraries 
STDSHLGUIMT=-lX11 -lXext -ldl -lpthread -lm
STDSHLCUIMT=-ldl -lpthread -lm

LIBSTLPORT=$(DYNAMIC) -lstlport_gcc
LIBSTLPORTST=$(STATIC) -lstlport_gcc $(DYNAMIC)

# name of library manager
LIBMGR=ar 
LIBFLAGS=-r 

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
DLLPOSTFIX=lp
DLLPRE=lib 
DLLPOST=.so 
