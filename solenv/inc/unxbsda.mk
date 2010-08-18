#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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

