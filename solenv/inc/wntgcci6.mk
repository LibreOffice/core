#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: wntgcci6.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:35:10 $
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

# mk file for wntgcci6

SOLAR_JAVA*=TRUE
FULL_DESK=TRUE
JAVAFLAGSDEBUG=-g

# SOLAR JAva Unterstuetzung nur fuer wntmsci

ASM=ml
AFLAGS=/c /Cp /coff

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=-march=pentium

CXX*=gcc
### Der gcc vertraegt kein Semikolon im Include-Pfad         RT
# old:
#CFLAGS=-c -Wall -I$(INCLUDE) $(OLE2DEF)
# new:
#CYGINC=$(INCLUDE:s/-I /-I/:+"  ":s/;/ -I/:s/-I  //:s/   / /)
CFLAGS=-fmessage-length=0 -c -nostdinc -fpcc-struct-return $(OLE2DEF)
###
CFLAGSCC=-pipe $(ARCH_FLAGS)
CFLAGSCXX=-pipe $(ARCH_FLAGS)
CFLAGSEXCEPTIONS=-fexceptions -fno-enforce-eh-specs
CFLAGS_NO_EXCEPTIONS=-fno-exceptions
PICSWITCH:=

CFLAGS_CREATE_PCH=-x c++-header -I$(INCPCH) -DPRECOMPILED_HEADERS
CFLAGS_USE_PCH=-I$(SLO)$/pch -DPRECOMPILED_HEADERS -Winvalid-pch
CFLAGS_USE_EXCEPTIONS_PCH=-I$(SLO)$/pch_ex -DPRECOMPILED_HEADERS -Winvalid-pch

CFLAGSOBJGUIST=
CFLAGSOBJCUIST=
CFLAGSOBJGUIMT=-D_MT
CFLAGSOBJCUIMT=-D_MT
CFLAGSSLOGUIMT=-D_MT $(PICSWITCH)
CFLAGSSLOCUIMT=-D_MT $(PICSWITCH)
CFLAGSPROF=
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=
.IF "$(PRODUCT)"!=""
CFLAGSOPT=-O2 -fno-strict-aliasing		# optimizing for products
.ELSE 	# "$(PRODUCT)"!=""
CFLAGSOPT=   							# no optimizing for non products
.ENDIF	# "$(PRODUCT)"!=""
# Compiler flags for disabling optimizations
CFLAGSNOOPT=-O0
# Compiler flags for describing the output path
CFLAGSOUTOBJ=-o
#plattform hart setzen
CDEFS+=-DWIN32 -DWINVER=0x400 -D_WIN32_IE=0x400 -D_DLL -D_M_IX86 -DSTLPORT_VERSION=450 -D_NATIVE_WCHAR_T_DEFINED

# -Wshadow does not work for C with nested uses of pthread_cleanup_push:
CFLAGSWARNCC=-Wall -Wextra -Wendif-labels
CFLAGSWARNCXX=$(CFLAGSWARNCC) -Wshadow -Wno-ctor-dtor-privacy \
    -Wno-non-virtual-dtor -Wno-uninitialized
CFLAGSWALLCC=$(CFLAGSWARNCC)
CFLAGSWALLCXX=$(CFLAGSWARNCXX)
CFLAGSWERRCC=-Werror
CFLAGSWERRCXX=-Werror

MODULES_WITH_WARNINGS := \
    b_server \
    chart2 \
    devtools \
    extensions \
    lingu \
    r_tools \
    soldep \
    starmath \
    sw \
    xmlsecurity

STATIC= -static
DYNAMIC= -dynamic

LINK*=$(CC)
CYGLIB=$(LIB:s/;/ -L/)
LINKFLAGS=-nostdlib -Wl,--enable-stdcall-fixup,--enable-runtime-pseudo-reloc -L$(CYGLIB)
.IF "$(USE_MINGW)"=="cygwin"
MINGWLIBDIR=$(COMPATH)$/lib$/mingw
.ELSE
MINGWLIBDIR=$(COMPATH)$/lib
.ENDIF
LINKFLAGSAPPGUI=-mwindows $(MINGWLIBDIR)$/crt2.o
LINKFLAGSSHLGUI=--warn-once -mwindows -shared $(MINGWLIBDIR)$/dllcrt2.o
LINKFLAGSAPPCUI=-mconsole $(MINGWLIBDIR)$/crt2.o
LINKFLAGSSHLCUI=--warn-once -mconsole -shared $(MINGWLIBDIR)$/dllcrt2.o
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

STDLIBCPP=-lstdc++
UWINAPILIB*=$(DYNAMIC) -luwinapi

STDOBJVCL=$(L)$/salmain.obj
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIST=-Wl,--start-group CPPRUNTIME -lgcc
STDLIBCUIST=-Wl,--start-group CPPRUNTIME -lgcc
STDLIBGUIMT=-Wl,--start-group CPPRUNTIME -lgcc
STDLIBCUIMT=-Wl,--start-group CPPRUNTIME -lgcc
STDSHLGUIST=-Wl,--start-group CPPRUNTIME -lgcc
STDSHLCUIST=-Wl,--start-group CPPRUNTIME -lgcc
STDSHLGUIMT=-Wl,--start-group CPPRUNTIME -lgcc
STDSHLCUIMT=-Wl,--start-group CPPRUNTIME -lgcc
STDLIBGUIMT+=-lmingwthrd
STDLIBCUIMT+=-lmingwthrd
STDSHLGUIMT+=-lmingwthrd
STDSHLCUIMT+=-lmingwthrd
STDLIBGUIST+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt
STDLIBCUIST+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt
STDLIBGUIMT+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt
STDLIBCUIMT+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt
STDSHLGUIST+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt
STDSHLCUIST+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt
STDSHLGUIMT+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt
STDSHLCUIMT+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt

LIBSTLPORT=-lstlport_gcc
LIBSTLPORTST=-lstlport_gcc_static

LIBMGR=ar
LIBFLAGS=-rsu

IMPLIB=ld
IMPLIBFLAGS=

MAPSYM=tmapsym
MAPSYMFLAGS=

RC=rc
RCFLAGS=-D__MINGW32__ -DWIN32 -D_WIN32_IE=0x400 -fo$@ $(RCFILES)
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

DLLPOSTFIX=gi
PCHPOST=.gch

ADVAPI32LIB=-ladvapi32
SHELL32LIB=-lshell32
GDI32LIB=-lgdi32
OLE32LIB=-lole32
OLEAUT32LIB=-loleaut32
UUIDLIB=$(PSDK_HOME)$/lib$/uuid.lib
WINSPOOLLIB=-lwinspool
IMM32LIB=-limm32
PSPLIB=-lpsp
VERSIONLIB=-lversion
WINMMLIB=-lwinmm
WSOCK32LIB=-lwsock32
MPRLIB=-lmpr
WS2_32LIB=-lws2_32
KERNEL32LIB=-lkernel32
USER32LIB=-luser32
LIBCMT=-lmsvcrt
COMDLG32LIB=-lcomdlg32
COMCTL32LIB=-lcomctl32
CRYPT32LIB=$(PSDK_HOME)$/lib$/crypt32.lib
GDIPLUSLIB=$(PSDK_HOME)$/lib$/gdiplus.lib
DBGHELPLIB=$(PSDK_HOME)$/lib$/dbghelp.lib
MSILIB=$(PSDK_HOME)$/lib$/msi.lib
DDRAWLIB=$(DIRECTXSDK_LIB)/ddraw.lib
SHLWAPILIB=$(PSDK_HOME)$/lib$/shlwapi.lib
URLMONLIB=$(PSDK_HOME)$/lib$/urlmon.lib
UNICOWSLIB=$(PSDK_HOME)$/lib$/unicows.lib
WININETLIB=-lwininet
OLDNAMESLIB=-lmoldname
MSIMG32LIB=$(PSDK_HOME)$/lib$/msimg32.lib
