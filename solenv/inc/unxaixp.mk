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

# mak file fuer unxaixp
ASM=
AFLAGS=

ARCH_FLAGS*=

CXX=				xlC_r
CC=				xlc_r
CFLAGS=			-c -qlanglvl=extended -qchars=signed 
CDEFS+=			-D_PTHREADS
CDEFS+=			-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CFLAGSCC=$(ARCH_FLAGS)
CFLAGSCXX=$(ARCH_FLAGS)
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT=
CFLAGSSLOCUIMT=
CFLAGSPROF=
CFLAGSDEBUG=
CFLAGSDBGUTIL=
CFLAGSOPT=		-O
CFLAGSNOOPT=
CFLAGSOUTOBJ=	-o

STATIC= 		# -Bstatic
DYNAMIC= 		# -Bdynamic

#
# Zu den Linkflags von shared Libraries:
#
# -G			Erzeugen eines Shared-Objects
# -brtl			aktiviert Runtime-Linking, wird von -G mitgesetzt
# -bsymbolc		loest Symbole innerhalb einer Library zur Linkzeit auf
# -p0			??? (ENOMANPAGESINSTALLABLEFROMCD)
# -bnoquiet		verbose
#
LINK=
LINKFLAGS=
LINKFLAGSAPPCUI=	xlC_r -qlanglvl=extended -qchars=signed -brtl -bnolibpath
LINKFLAGSAPPGUI=	xlC_r -qlanglvl=extended -qchars=signed -brtl -bnolibpath
LINKFLAGSSHLCUI=	makeC++SharedLib_r -G -bsymbolic -bdynamic -bnolibpath -p0
LINKFLAGSSHLGUI=	makeC++SharedLib_r -G -bsymbolic -bdynamic -bnolibpath -p0
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=
LINKFLAGSOPT=

# Reihenfolge der libs NICHT egal!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
# application
STDLIBGUIMT=	# -lX11 -ldl
STDLIBCUIMT=	# -lX11 -ldl
# shared library
STDSHLGUIMT=	-lX11 -ldl
STDSHLCUIMT=	-lX11 -ldl
THREADLIB=

LIBMGR=			ar
LIBFLAGS=		-r
# LIBEXT=		.so

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=irc
RCFLAGS=		-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPRE=			lib
DLLPOST=		.so


LDUMP=			cppfilt /b /n /o /p

