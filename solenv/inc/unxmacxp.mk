#*************************************************************************
#
#   $RCSfile: unxmacxp.mk,v $
#
#   $Revision: 1.42 $
#
#   last change: $Author: hr $ $Date: 2003-03-27 11:48:19 $
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

# DARWIN_VERSION holds the Darwin version in the format: 000000. For example,
# if the Darwin version is 1.3.7, DARWIN_VERSION will be set to 010307.
DARWIN_VERSION=$(shell -/bin/sh -c "uname -r | sed 's/\./ /g' | xargs printf %2.2i%2.2i%2.2i")  

# mk file for unxmacxp
ASM=
AFLAGS=

LINKOUTPUT_FILTER=

# _PTHREADS is needed for the stl
CDEFS+=-DGLIBC=2 -D_PTHREADS -D_REENTRANT -DNO_PTHREAD_PRIORITY -DSTLPORT_VERSION=400 -D_USE_NAMESPACE=1

# Name of library where static data members are initialized
STATICLIBNAME=static$(DLLPOSTFIX)
STATICLIB=-l$(STATICLIBNAME)

.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
JAVAFLAGSDEBUG=-g
JAVA_RUNTIME=-framework JavaVM
.ENDIF

CXX*=cc
CC*=cc
objc=cc

# [ed] 6/15/02 Some .. bleep .. decided to make a directory within a module that has the same
# name as one of the STL headers.  This sucks since, with Apple's compilers, if a directory
# with a name appears in a directory specified with a -I directive, the compiler will flag
# an error and not search for a file with that name in a following -I directive on the
# command line.  Therefore...this ugly hack...put STL as the first include on the linkline.

.IF "$(STLPORT4)"!=""
.IF "$(STLPORT4)"=="NO_STLPORT4"

# [ed] We need to add in the STLport include directory included with the standard OOo sources,
# currently STLPORT 4.5.

CFLAGS=-c -I$(SOLARVERSION)$/$(GVERDIR)$/inc/stl $(INCLUDE)
.ELSE

# [ed] We've got stlport4!

CFLAGS=-c -I$(STLPORT4)$/stlport $(INCLUDE)

.ENDIF
.ELSE

# [ed] We need to add in the STLport include directory included with the standrad OOo sources,
# currently STLPORT 4.5

CFLAGS=-c -I$(SOLARVERSION)$/$(GVERDIR)$/inc/stl $(INCLUDE)

.ENDIF

CFLAGSCC=-pipe -traditional-cpp -fno-common

OBJCFLAGS=-no-precomp

CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

CFLAGSCXX=-pipe -fno-for-scope -fpermissive -fno-operator-names -fno-coalesce -fno-common

CFLAGSOBJGUIST=-fPIC
CFLAGSOBJCUIST=-fPIC
CFLAGSOBJGUIMT=-fPIC
CFLAGSOBJCUIMT=-fPIC
CFLAGSSLOGUIMT=-fPIC
CFLAGSSLOCUIMT=-fPIC
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=

# Workaround for the compiler's buggy opitmizer when exceptions are turned on.
# Note that we have to define NO_OPTIMIZE for all files as once you turn off
# optimizing for one file, we need to adjust our compiling of the remaining
# optimized files.
CDEFS+=-DNO_OPTIMIZE
.IF "$(ENABLE_EXCEPTIONS)"==""
CFLAGSOPT=-O2
CFLAGSNOOPT=-O
.ELSE
CFLAGSOPT=
CFLAGSNOOPT=
.ENDIF
CFLAGSOUTOBJ=-o

# Currently, there is no nas support for OS X...
CDEFS+=           -DNO_AUDIO

SOLARVERSHLLIBS=$(shell -/bin/sh -c "ls $(SOLARLIBDIR)$/*$(DLLPOST) $(SOLARLIBDIR)$/*$(DLLPOST).[0-9] $(LB)$/*$(DLLPOST) $(LB)$/*$(DLLPOST).[0-9] $(MISC)$/*$(DLLPOST) $(MISC)$/*$(DLLPOST).[0-9] 2>/dev/null | grep -E -v 'lib\w+static' | grep -v cppuhelper")
.IF "$(STLPORT4)"!=""
.IF "$(STLPORT4)"=="NO_STLPORT4"
  # Look for STLport 4.5 libraries in SOLARLIBDIR
  SOLARVERSHLLIBS+=$(shell -/bin/sh -c "ls $(SOLARLIBDIR)$/$(DLLPRE)$(LIBSTLPORT:s/-l//)$(DLLPOST).[0-9].[0-9] 2>/dev/null")
.ELSE
  # Look for STLport 4.0 libraries in the STLport4 home directory
  SOLARVERSHLLIBS+=$(shell -/bin/sh -c "ls $(STLPORT4)$/lib$/*$(DLLPOST) $(STLPORT4)$/lib$/*$(DLLPOST).[0-9] 2>/dev/null")
.ENDIF
.ENDIF

# GrP mega-hack! no cppuhelper for cppuhelper
.IF "$(PRJNAME)"!="cppuhelper"
SOLARVERSHLLIBS+=$(shell -/bin/sh -c "ls $(SOLARLIBDIR)$/*$(DLLPOST) $(SOLARLIBDIR)$/*$(DLLPOST).[0-9] $(LB)$/*$(DLLPOST) $(LB)$/*$(DLLPOST).[0-9] $(MISC)$/*$(DLLPOST) $(MISC)$/*$(DLLPOST).[0-9] 2>/dev/null | grep -E -v 'lib\w+static' | grep cppuhelper")
.ENDIF

# GrP remove -U options (can't use for two-level)
LINK=cc
LINKFLAGS=-dynamic -framework System -framework CoreFoundation -lcc_dynamic -lstdc++ \
  $(foreach,i,$(SOLARVERSHLLIBS) -dylib_file @executable_path$/$(i:f):$i) \
  -L$(MISC)

# [ed] 5/14/02 If we're building for aqua, add in the objc runtime library into our link line
.IF "$(GUIBASE)" == "aqua"
LINKFLAGS+=-lobjc
INCGUI+= -I$(PRJ)$/unx/inc
.ENDIF

LINKFLAGSAPPGUI=
.IF "$(UNIXVERSIONNAMES)"!=""
LINKFLAGSSHLGUI=-dynamiclib -install_name '@executable_path$/$(@:f:b:b)'
.ELSE
LINKFLAGSSHLGUI=-dynamiclib -install_name '@executable_path$/$(@:f)'
.ENDIF
LINKFLAGSAPPCUI=-Wl,-u,__objcInit
.IF "$(UNIXVERSIONNAMES)"!=""
LINKFLAGSSHLCUI=-dynamiclib -install_name '@executable_path$/$(@:f:b:b)'
.ELSE
LINKFLAGSSHLCUI=-dynamiclib -install_name '@executable_path$/$(@:f)'
.ENDIF
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

LINKVERSIONMAPFLAG=

# Reihenfolge der libs NICHT egal!

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBGUIST=
STDLIBCUIST=
STDLIBGUIMT=
STDLIBCUIMT=
STDSHLGUIMT=
STDSHLCUIMT=

LIBSTLPORT=-lstlport_gcc
.IF "$(STLPORT4)"!=""
LIBSTLPORTST=$(STLPORT4)$/lib/libstlport_gcc.a
.ELSE
LIBSTLPORTST=$(SOLARVERSION)$/$(INPATH)$/lib/libstlport_gcc.a
.ENDIF

LIBMGR=libtool
LIBFLAGS=-o

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=mxp
DLLPRE=lib
DLLPOST=.dylib
