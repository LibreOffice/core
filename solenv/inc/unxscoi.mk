#*************************************************************************
#
#   $RCSfile: unxscoi.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: hr $ $Date: 2003-03-27 11:48:20 $
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


# mak file fuer wnticci
ASM=
AFLAGS=

CDEFS+=-D_PTHREADS -D_REENTRANT 
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE -DX86 -DNEW_SOLAR
# kann c++ was c braucht??

CXX*=/nw386/dev/s/solenv/unxscoi/bin/g++
CC*=/nw386/dev/s/solenv/unxscoi/bin/gcc
CFLAGS=-w -c $(INCLUDE)
CFLAGSCC=-mpentium
CFLAGSCXX= -mpentium
CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions
CFLAGSCXX=-mpentium -fguiding-decls -frtti

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

STATIC		= -Wl,-Bstatic
DYNAMIC		= -Wl,-Bdynamic

THREADLIB=
LINK=/nw386/dev/s/solenv/unxscoi/bin/gcc
LINKFLAGS=
# SCO hat grosse Probleme mit fork/exec und einigen shared libraries
# rsc2 muss daher statisch gelinkt werden
.IF "$(PRJNAME)"=="rsc"
LINKFLAGSAPPGUI=-L/nw386/dev/s/solenv/unxscoi/lib $(STATIC) -lpthread_init $(DYNAMIC)
LINKFLAGSAPPCUI=-L/nw386/dev/s/solenv/unxscoi/lib $(STATIC) -lpthread_init $(DYNAMIC)
.ELSE
LINKFLAGSAPPGUI=-L/nw386/dev/s/solenv/unxscoi/lib -lpthread_init
LINKFLAGSAPPCUI=-L/nw386/dev/s/solenv/unxscoi/lib -lpthread_init
.ENDIF
LINKFLAGSSHLGUI=-G -W,l,-Bsymbolic
LINKFLAGSSHLCUI=-G -W,l,-Bsymbolic
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

# standard C++ Library
#
# das statische dazulinken der libstdc++ macht jede shared library um 50k
# (ungestrippt) oder so groesser, auch wenn sie ueberhaupt nicht gebraucht
# wird. Da muessen wir uns was besseres ueberlegen.
STDLIBCPP=-Wl,-Bstatic -lstdc++ -Wl,-Bdynamic

# reihenfolge der libs NICHT egal!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIST=-lXext -lX11 -ldl -lsocket -lm
STDLIBCUIST=-ldl -lsocket -lm
.IF "$(PRJNAME)"=="rsc"
STDLIBGUIMT=-lXext -lX11 $(STATIC) -lpthread $(DYNAMIC) -ldl -lsocket -lm
STDLIBCUIMT=$(STATIC) -lpthread $(DYNAMIC) -ldl -lsocket -lm 
.ELSE
STDLIBGUIMT=-lXext -lX11 -lpthread -ldl -lsocket -lm
STDLIBCUIMT=-lpthread -ldl -lsocket -lm 
.ENDIF
#STDSHLGUIMT=-lXext -lX11 -lpthread -ldl -lsocket -lm
#STDSHLCUIMT=-lpthread -ldl -lsocket -lm 

STDLIBCPP= -lstdc++
SHLLINKARCONLY=yes

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

DLLPOSTFIX=ci
DLLPRE=lib
DLLPOST=.so

LDUMP=cppfilt /b /n /o /p

.IF "$(WORK_STAMP)"!="MIX364"
DLLPOSTFIX=ci
.ELSE
DLLPOSTFIX=
.ENDIF
