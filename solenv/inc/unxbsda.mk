#*************************************************************************
#
#   $RCSfile: unxbsda.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2003-03-27 11:48:15 $
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

# mak file fuer unxbsda

ASM=
AFLAGS=

CC=				gcc -c
CXX=			g++ -c
CDEFS+=			-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CDEFS+=			-D_REENTRANT 		# -D_PTHREADS -DPOSIX_PTHREAD_SEMANTICS
CDEFS+= 		-DNO_AUDIO -DPRINTER_DUMMY
CFLAGS+=		$(INCLUDE)
CFLAGSCC=		-fsigned-char
CFLAGSCXX=		-fsigned-char
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=	-fPIC
CFLAGSSLOCUIMT=	-fPIC
CFLAGSPROF=
CFLAGSDEBUG=	-g
CFLAGSDBGUTIL=
CFLAGSOPT=		-O2
CFLAGSNOOPT=
CFLAGSOUTOBJ=	-o

STATIC=			-Bstatic
DYNAMIC=		-Bdynamic

THREADLIB=		pthread
LINK=			ld -e start -dc -dp
LINKFLAGS=
LINKFLAGSAPPGUI=
LINKFLAGSSHLGUI=
LINKFLAGSAPPCUI=
LINKFLAGSSHLCUI=
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=

STDOBJGUI=		/usr/lib/crt0.o
STDSLOGUI=		/usr/lib/crt0.o
STDOBJCUI=		/usr/lib/crt0.o
STDSLOCUI=		/usr/lib/crt0.o
STDLIBGUIST=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDLIBCUIST=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDLIBGUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDLIBCUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDSHLGUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc
STDSHLCUIMT=	-Bdynamic -lc -Bstatic -lg++ -lstdc++ -Bdynamic -lm -lgcc -lc -lgcc

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

DLLPOSTFIX=		ba
DLLPRE=			lib
DLLPOST=		.so

LDUMP=

