#*************************************************************************
#
#   $RCSfile: unxaixp.mk,v $
#
#   $Revision: 1.3 $
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

# mak file fuer unxaixp
ASM=
AFLAGS=

CXX=				xlC_r
CC=				xlc_r
CFLAGS=			-c -qlanglvl=extended -qchars=signed 
CFLAGS+=		$(INCLUDE)
CDEFS+=			-D_PTHREADS
CDEFS+=			-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
CFLAGSCC=
CFLAGSCXX=
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
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
STDLIBGUIST=
STDLIBCUIST=
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

DLLPOSTFIX=		ap
.IF "$(WORK_STAMP)"=="MIX364"
DLLPOSTFIX=
.ENDIF
DLLPRE=			lib
DLLPOST=		.so


LDUMP=			cppfilt /b /n /o /p

