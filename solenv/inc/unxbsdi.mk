#*************************************************************************
#
#   $RCSfile: unxbsdi.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: hr $ $Date: 2003-04-28 16:46:12 $
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

# mak file fuer unxbsdi

ASM=
AFLAGS=

cc=gcc -c
CC=g++ -c
CDEFS+=-D_PTHREADS -D_REENTRANT
CDEFS+=-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CDEFS+=-DNO_INET_ON_DEMAND -DX86 -DNEW_SOLAR -DNCIfeature
CFLAGS+=-c $(INCLUDE)
CFLAGSCC=-pipe -mpentium
CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions
CFLAGSCXX=-pipe -mpentium -fguiding-decls -frtti

CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=	-fPIC
CFLAGSSLOCUIMT=	-fPIC
CFLAGSPROF=     -pg
CFLAGSDEBUG=	-g
CFLAGSDBGUTIL=
# die zusaetzlichen Optimierungsschalter schalten alle Optimierungen ein, die zwischen -O und -O2 liegen und
# per Schalter einschaltbar sind. Dennoch gibt es einen Unterschied: einige Files im Writer werden
# misoptimiert wenn -O2 eingeschaltet ist und waehrend die untenstehenden Schalter funktionieren.
CFLAGSOPT=-O -fcse-follow-jumps -fcse-skip-blocks -fexpensive-optimizations -fstrength-reduce -fforce-mem -fcaller-saves -fgcse -frerun-cse-after-loop -frerun-loop-opt -fschedule-insns2 -fregmove -foptimize-register-move
#CFLAGSOPT=-O2
CFLAGSNOOPT=-O
CFLAGSOUTOBJ=-o

STATIC=			-Bstatic
DYNAMIC=		-Bdynamic

THREADLIB=
LINK= gcc
LINKFLAGS=
.IF "$(PRJNAME)"=="osl" ||  "$(PRJNAME)"=="rtl" 
LINKFLAGSSHLGUI= -shared -nostdlib
LINKFLAGSSHLCUI= -shared -nostdlib
.ELSE
LINKFLAGSSHLGUI= -shared -nostdlib /usr/lib/c++rt0.o
LINKFLAGSSHLCUI= -shared -nostdlib /usr/lib/c++rt0.o
.ENDIF
LINKFLAGSAPPGUI= -L/nw386/dev/s/solenv/unxbsdi/lib -lpthread_init -lpthread
LINKFLAGSAPPCUI= -L/nw386/dev/s/solenv/unxbsdi/lib -lpthread_init -lpthread
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=

_SYSLIBS= -lpthread -lgcc -lc -lm 
_X11LIBS= -L/usr/X11R6/lib -lXext -lX11

STDLIBCPP= -lstdc++

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBGUIST=  ${_X11LIBS} ${_SYSLIBS}
STDLIBCUIST=              ${_SYSLIBS}
STDLIBGUIMT=  ${_X11LIBS} ${_SYSLIBS}
STDLIBCUIMT=              ${_SYSLIBS}
STDSHLGUIMT=
STDSHLCUIMT=

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

DLLPOSTFIX=		bi
DLLPRE=			lib
DLLPOST=		.so.1.0

LDUMP=

