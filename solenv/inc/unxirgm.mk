#*************************************************************************
#
#   $RCSfile: unxirgm.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: hr $ $Date: 2003-03-27 11:48:16 $
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


# Makefile for IRIX/mips using GNU compilers 2.95.2 for n32 ABI
ASM=
AFLAGS=

#
# Compiler flags:
#
# -ptused :	Instantiate templates while they're in
#		use. This switch was added because a static member function
#		couldn't be instantiated.
#		(ts&sdo/10.06.98)
#
#cc=			cc -KPIC -c -n32 -ansi
CC=			gcc
#CXX=			CC -KPIC -c -n32 -ptused -OPT:Olimit=20523 
CXX=			g++ -c
CFLAGS=		$(INCLUDE)
CDEFS+=	-D_STD_NO_NAMESPACE -D_VOS_NO_NAMESPACE -D_UNO_NO_NAMESPACE
# CFLAGS+=	-D__STL_NATIVE_INCLUDE_PATH=/usr/include -D__STL_NATIVE_C_INCLUDE_PATH=/usr/include
# CFLAGS+=	-D_STL_NATIVE_INCLUDE_PATH=/usr/include -D_STL_NATIVE_C_INCLUDE_PATH=/usr/include
CFLAGSCC=	 -c
CFLAGSCXX= 	-fno-for-scope -fpermissive
CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=
CFLAGSOBJCUIMT=
CFLAGSSLOGUIMT= -fpic
CFLAGSSLOCUIMT= -fpic
CFLAGSPROF=
CFLAGSDEBUG=	-g
CFLAGSDBGUTIL=
CFLAGSOPT=		-O2
CFLAGSNOOPT=		-O
CFLAGSOUTOBJ=	-o

STATIC=			-Wl,-Bstatic
DYNAMIC=		-Wl,-Bdynamic

#
# Link flags:
#
# -update_registry <file> :	Is needed to link several shared libraries
#				**VALID MECHANISM SOMETIMES MISSING FOR LOCAL STANDS**
#
LINK=			g++
LINKFLAGS=	-L/usr/lib32	
LINKFLAGSAPPGUI= -Wl,-multigot
LINKFLAGSAPPCUI= -Wl,-multigot

LINKFLAGSSHLGUI= -shared
LINKFLAGSSHLCUI= -shared

LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG= -g
LINKFLAGSOPT=

LINKFLAGSSHLGUI += -Wl,-Bsymbolic
LINKFLAGSSHLCUI += -Wl,-Bsymbolic

APPLINKSTATIC=-Bstatic
APPLINKSHARED=-Bsymbolic

# Sequence of libraries DOES matter!
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
# STDLIBGUIST=	$(DYNAMIC) -lX11 -lc -lm
STDLIBGUIST=	$(DYNAMIC) -lX11 -lc -lm
STDLIBCUIST=	$(DYNAMIC) -lc -lm
# STDLIBGUIMT=	$(THREADLIB) $(DYNAMIC) -lX11 -lc -lm
STDLIBGUIMT=	$(THREADLIB) $(DYNAMIC) -lX11 -lc -lm
STDLIBCUIMT=	$(THREADLIB) $(DYNAMIC) -lc -lm
# STDSHLGUIMT=	-L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lX11 -lc -lm
STDSHLGUIMT=	-L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lX11 -lc -lm
STDSHLCUIMT=	-L/usr/lib32 $(THREADLIB) $(DYNAMIC) -lc -lm
THREADLIB=		-lpthread

LIBMGR=			ar
LIBFLAGS=		-r
# LIBEXT=		.so

IMPLIB=
IMPLIBFLAGS=

MAPSYM=
MAPSYMFLAGS=

RC=				irc
RCFLAGS=		-fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=		im
.IF "$(WORK_STAMP)"=="MIX364"
DLLPOSTFIX=
.ENDIF
DLLPRE=			lib
DLLPOST=		.so


LDUMP=

STDLIBCPP = -lstdc++

# --------------------------
# FROM THE OLE ENVIRONMENT:
# --------------------------
#
# Linking of a static library:
#	ar -r ...
#
# Linking of a shared library:
#	CC -B symbolic -soname <...> -B dynamic -shared -n32 -multigot
#		-update_registry <...> ...
#
# Linking of an application with static libraries:
#	CC -B static ...
#
# Linking of an application with shared libraries:
#	CC -n32 -multigot ...
#

