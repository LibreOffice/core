#*************************************************************************
#
#   $RCSfile: unxfbsdi.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: hr $ $Date: 2003-04-28 16:46:28 $
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

ASM=
AFLAGS=

SOLAR_JAVA=TRUE
JAVAFLAGSDEBUG=-g

LINKOUTPUT_FILTER=" |& $(SOLARENV)$/bin$/msg_filter"


CC=				gcc
CXX=			g++
CDEFS+=			-D_USE_NAMESPACE=1 -DX86 -DNEW_SOLAR -DSTLPORT_VERSION=400
CDEFS+=			-D_REENTRANT -D_PTHREADS -D_THREAD_SAFE
CFLAGS+= -c $(INCLUDE)
CFLAGS+= -I/usr/X11R6/include
CFLAGSCC= -pipe
CFLAGSCXX= -pipe -fno-for-scope -fpermissive
CFLAGSEXCEPTIONS= -fexceptions
CFLAGS_NO_EXCEPTIONS= -fno-exceptions
CFLAGSOBJGUIST= -fPIC
CFLAGSOBJCUIST= -fPIC
CFLAGSOBJGUIMT= -fPIC
CFLAGSOBJCUIMT= -fPIC
CFLAGSSLOGUIMT=	-fPIC
CFLAGSSLOCUIMT=	-fPIC
CFLAGSPROF=     -pg
CFLAGSDEBUG=	-g
CFLAGSDBGUTIL=
CFLAGSOPT= -O2
CFLAGSNOOPT= -O
CFLAGSOUTOBJ= -o

STATIC=	-Wl,-Bstatic
DYNAMIC= -Wl,-Bdynamic

#THREADLIB= -pthread
LINK= gcc
# -v -nostdlib
LINKFLAGS=
LINKFLAGSAPPGUI= -Wl,-export-dynamic
LINKFLAGSAPPCUI= -Wl,-export-dynamic
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

STDLIBCPP=-lstdc++

# _SYSLIBS= -L/usr/lib -lm
# _X11LIBS= -L/usr/X11R6/lib -lXext -lX11
# _CXXLIBS= -L/usr/lib -lstdc++ -L/usr/local/lib

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBCUIST=-lm
STDLIBGUIST=-lX11 -lm
STDLIBGUIMT=-lX11 -lXext -pthread -lm -lstlport_gcc
STDLIBCUIMT=-pthread -lm -lstlport_gcc
STDSHLGUIMT=-lX11 -lXext -pthread -lm -lstlport_gcc
STDSHLCUIMT=-pthread -lm -lstlport_gcc

LIBMGR=			ar
LIBFLAGS=		-r
LIBEXT=			.a

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=		-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=		fi
DLLPRE=			lib
DLLPOST=		.so
LDUMP=
