#*************************************************************************
#
#   $RCSfile: unxmacxp.mk,v $
#
#   $Revision: 1.44 $
#
#   last change: $Author: hjs $ $Date: 2003-08-18 14:49:54 $
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

##########################################################################
# Platform MAKEFILE for Mac OS X and Darwin
##########################################################################

# DARWIN_VERSION holds the Darwin version in the format: 000000. For example,
# if the Darwin version is 1.3.7, DARWIN_VERSION will be set to 010307.
DARWIN_VERSION=$(shell -/bin/sh -c "uname -r | sed 's/\./ /g' | xargs printf %2.2i%2.2i%2.2i")  

ASM=
AFLAGS=
LINKOUTPUT_FILTER=

# Definitions that we may need on the compile line.
# -D_PTHREADS and -D_REENTRANT are needed for STLport, and must be specified when
#  compiling STLport sources too, either internally or externally.
CDEFS+=-DGLIBC=2 -D_PTHREADS -D_REENTRANT -DNO_PTHREAD_PRIORITY -DPOWERPC -DPPC -DSTLPORT_VERSION=400 -D_USE_NAMESPACE=1

# Name of library where static data members are initialized
# STATICLIBNAME=static$(DLLPOSTFIX)
# STATICLIB=-l$(STATICLIBNAME)

# MacOS X specific Java compilation/link flags
.IF "$(SOLAR_JAVA)"!=""
    JAVADEF=-DSOLAR_JAVA
    JAVAFLAGSDEBUG=-g
    JAVA_RUNTIME=-framework JavaVM
.ENDIF

# Specify the compiler to use.  NOTE:  MacOS X should always specify
# c++ for C++ compilation as it does certain C++ specific things
# behind the scenes for us.
# CC = C++ compiler to use
# cc = C compiler to use
# objc = Objective C compiler to use
CXX*=g++
CC*=gcc
objc=cc

CFLAGS=-fsigned-char -fmessage-length=0 -malign-natural -c $(INCLUDE)

# ---------------------------------
#  Compilation flags
# ---------------------------------
# Normal C compilation flags
CFLAGSCC=-pipe -fsigned-char -malign-natural

# Normal Objective C compilation flags
OBJCFLAGS=-no-precomp

# Comp Flags for files that need exceptions enabled (C and C++)
CFLAGSEXCEPTIONS=-fexceptions -fno-enforce-eh-specs

# Comp Flags for files that do not need exceptions enabled (C and C++)
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

# Normal C++ compilation flags
CFLAGSCXX=-pipe -malign-natural -fsigned-char -fno-for-scope -fpermissive -Wno-long-double 

# Other flags
CFLAGSOBJGUIST=-fPIC -fno-common
CFLAGSOBJCUIST=-fPIC -fno-common
CFLAGSOBJGUIMT=-fPIC -fno-common
CFLAGSOBJCUIMT=-fPIC -fno-common
CFLAGSSLOGUIMT=-fPIC -fno-common
CFLAGSSLOCUIMT=-fPIC -fno-common
CFLAGSPROF=

# Flag for including debugging information in object files
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=

# Flag to specify output file to compiler/linker
CFLAGSOUTOBJ=-o

# ---------------------------------
#  Optimization flags
# ---------------------------------
CFLAGSOPT=-O2 -fno-strict-aliasing
CFLAGSNOOPT=-O0

# Currently, there is no nas support for OS X...
CDEFS+= -DNO_AUDIO

# ---------------------------------
#  STLport library names
# ---------------------------------
LIBSTLPORT=-lstlport_gcc -lstdc++
LIBSTLPORTST=$(SOLARVERSION)$/$(INPATH)$/lib/libstlport_gcc.a -lstdc++

# ---------------------------------
#  Link stage flags
# ---------------------------------
# always link with gcc since you may be linking c code and don't want -lstdc++ linked in!
LINK=gcc
LINKFLAGS=-Wl,-multiply_defined,suppress

# LINKFLAGS=-dynamic -framework System -Wl,-multiply_defined,suppress -lcc_dynamic -lstdc++ -L$(MISC)

# [ed] 5/14/02 If we're building for aqua, add in the objc runtime library into our link line
.IF "$(GUIBASE)" == "aqua"
    LINKFLAGS+=-lobjc
    # Sometimes we still use files that would be in a GUIBASE="unx" specific directory
    # because they really aren't GUIBASE specific, so we've got to account for that here.
    INCGUI+= -I$(PRJ)$/unx/inc
.ENDIF

# Random link flags dealing with different cases of linking

LINKFLAGSAPPGUI=-bind_at_load
.IF "$(UNIXVERSIONNAMES)"!=""
    LINKFLAGSSHLGUI=-dynamiclib -single_module -install_name '@executable_path$/$(@:f:b:b)'
.ELSE
    LINKFLAGSSHLGUI=-dynamiclib -single_module -install_name '@executable_path$/$(@:f)'
.ENDIF
LINKFLAGSAPPCUI=-bind_at_load -Wl,-u,__objcInit
.IF "$(UNIXVERSIONNAMES)"!=""
    LINKFLAGSSHLCUI=-dynamiclib -single_module -install_name '@executable_path$/$(@:f:b:b)'
.ELSE
    LINKFLAGSSHLCUI=-dynamiclib -single_module -install_name '@executable_path$/$(@:f)'
.ENDIF
LINKFLAGSTACK=
LINKFLAGSPROF=

# Flag to add debugging information to final products
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

# ---------------------------------
#  MacOS X shared library specifics
# ---------------------------------
# Tag to identify an output file as a MacOS X output file
DLLPOSTFIX=mxp
# Tag to identify an output file as a library
DLLPRE=lib
# File extension to identify dynamic shared libraries on MacOS X
DLLPOST=.dylib

# We don't use mapping on MacOS X
LINKVERSIONMAPFLAG=-Wl,--version-script

SONAME_SWITCH=-Wl,-h

STDLIBCPP=-lstdc++

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBGUIST=-lm
STDLIBCUIST=-lX11 -lpthread -lm
STDLIBGUIMT=-lpthread -lm
STDLIBCUIMT=-lX11 -lm
STDSHLGUIMT=-lX11 -lXext -lpthread -m
STDSHLCUIMT=-lpthread -lm

LIBMGR=ar
LIBFLAGS=-r

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=
