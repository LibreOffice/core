#*************************************************************************
#
#   $RCSfile: unxmacxp.mk,v $
#
#   $Revision: 1.30 $
#
#   last change: $Author: pluby $ $Date: 2001-03-09 07:52:37 $
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


# mk file for unxmacxp
ASM=
AFLAGS=

LINKOUTPUT_FILTER=

# _PTHREADS is needed for the stl
CDEFS+=-DGLIBC=2 -D_PTHREADS -D_REENTRANT -DNO_PTHREAD_PRIORITY -DSTLPORT_VERSION=400 -D_USE_NAMESPACE=1

# Workaround for Mac OS X duplicate symbol plugin bug
.IF "$(SYMBOLPREFIX)"==""
SYMBOLPREFIX=$(TARGET)
.ENDIF
.IF "$(SYMBOLPREFIX)"!=""
CDEFS+=-Dcomponent_getImplementationEnvironment=lib$(SYMBOLPREFIX)component_getImplementationEnvironment \
  -Dcomponent_writeInfo=lib$(SYMBOLPREFIX)component_writeInfo \
  -Dcomponent_getFactory=lib$(SYMBOLPREFIX)component_getFactory \
  -Dcomponent_getDescriptionFunc=lib$(SYMBOLPREFIX)component_getDescriptionFunc \
  -Duno_initEnvironment=lib$(SYMBOLPREFIX)uno_initEnvironment \
  -Duno_ext_getMapping=lib$(SYMBOLPREFIX)uno_ext_getMapping
.ENDIF
.IF "$(PRJNAME)"=="registry"
CDEFS+=-DinitRegistry_Api=lib$(REGLIB:s/-l//)initRegistry_Api \
  -DinitRegistryTypeReader_Api=lib$(REGLIB:s/-l//)initRegistryTypeReader_Api \
  -DinitRegistryTypeWriter_Api=lib$(REGLIB:s/-l//)initRegistryTypeWriter_Api
.ENDIF

# Name of library where static data members are initialized
STATICLIBNAME=static$(DLLPOSTFIX)
STATICLIB=-l$(STATICLIBNAME)

.IF "$(SOLAR_JAVA)"!=""
JAVADEF=-DSOLAR_JAVA
JAVAFLAGSDEBUG=-g
JAVA_RUNTIME=-framework JavaVM
.ENDIF

CC=cc
cc=cc
objc=cc
CFLAGS=-c $(INCLUDE)
CFLAGSCC=-pipe -traditional-cpp

OBJCFLAGS=-no-precomp

CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

CFLAGSCXX=-pipe -fno-for-scope -fpermissive -fno-operator-names

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

SOLARVERSHLLIBS=$(shell -/bin/sh -c "ls $(SOLARLIBDIR)$/*$(DLLPOST) $(LB)$/*$(DLLPOST) $(MISC)$/*$(DLLPOST) 2>/dev/null | grep -E -v 'lib\w+static'")
.IF "$(STLPORT4)"!=""
SOLARVERSHLLIBS+=$(shell -/bin/sh -c "ls $(STLPORT4)$/lib$/*$(DLLPOST) 2>/dev/null")
.ENDIF

LINK=cc
LINKFLAGS=-dynamic -framework System -framework Cocoa -lcc_dynamic -lstdc++ \
  $(foreach,i,$(SOLARVERSHLLIBS) -dylib_file @executable_path$/$(i:f):$i) \
  -L$(MISC)
LINKFLAGSAPPGUI=-Wl,-u,__objcInit
LINKFLAGSSHLGUI=-dynamiclib -install_name '@executable_path$/$(@:f)' \
  -Wl,-U,___progname -Wl,-U,_environ
LINKFLAGSAPPCUI=-Wl,-u,__objcInit
LINKFLAGSSHLCUI=-dynamiclib -install_name '@executable_path$/$(@:f)' \
  -Wl,-U,___progname -Wl,-U,_environ
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
