#*************************************************************************
#
#   $RCSfile: unxmacxp.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: pluby $ $Date: 2000-10-30 17:37:48 $
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
CDEFS+=-DGLIBC=2 -D_PTHREADS -D_REENTRANT -DNO_PTHREAD_PRIORITY -DSTLPORT_VERSION=321 -D_USE_NAMESPACE=1

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
CFLAGSOPT=-O2
CFLAGSNOOPT=-O
CFLAGSOUTOBJ=-o

SOLARVERSHLLIBS=$(shell -$(FIND) $(SOLARVERSION)$/$(INPATH)$/lib -name $(DLLPRE)\*$(DLLPOST))
LINK=cc
LINKFLAGS=-dynamic -framework System -framework Cocoa -framework QD \
  -lcc_dynamic -lstdc++ \
  $(foreach,i,$(SOLARVERSHLLIBS) '-dylib_file @executable_path$/$(i:f):$i')
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
