#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unxbsda.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: kz $ $Date: 2008-03-07 16:55:06 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

# mak file fuer unxbsda

ASM=
AFLAGS=

ARCH_FLAGS*=

CC=				gcc -c
CXX=			g++ -c
CDEFS+=			-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CDEFS+=			-D_REENTRANT 		# -D_PTHREADS -DPOSIX_PTHREAD_SEMANTICS
CDEFS+= 		-DPRINTER_DUMMY
CFLAGSCC=		-fsigned-char $(ARCH_FLAGS)
CFLAGSCXX=		-fsigned-char $(ARCH_FLAGS)
PICSWITCH:=-fPIC
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=$(PICSWITCH)
CFLAGSSLOCUIMT=$(PICSWITCH)
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

