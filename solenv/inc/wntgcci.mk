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

# mk file for Window Intel using GCC

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
CFLAGS=-fmessage-length=0 -c -nostdinc $(OLE2DEF)
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
CDEFS+=-DWIN32 -DWINVER=0x500 -D_WIN32_WINNT=0x500 -D_WIN32_IE=0x500 -D_M_IX86 -D_NATIVE_WCHAR_T_DEFINED -D_MSC_EXTENSIONS -D_FORCENAMELESSUNION
.IF  "$(DYNAMIC_CRT)"!=""
CDEFS+=-D_DLL
.ENDIF

# -Wshadow does not work for C with nested uses of pthread_cleanup_push:
CFLAGSWARNCC=-Wall -Wextra -Wendif-labels
CFLAGSWARNCXX=$(CFLAGSWARNCC) -Wshadow -Wno-ctor-dtor-privacy \
    -Wno-non-virtual-dtor
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
    starmath \
    sw \
    xmlsecurity

STATIC= -static
DYNAMIC= -dynamic

LINK*=$(CXX)
LINKC*=$(CC)

CYGLIB=$(LIB:s/;/ -L/)
LINKFLAGS=-nostdlib -Wl,--enable-stdcall-fixup,--enable-runtime-pseudo-reloc-v2 -L$(CYGLIB)
.IF "$(USE_MINGW)"=="cygwin"
MINGWLIBDIR=$(COMPATH)$/lib$/mingw
.ELSE
MINGWLIBDIR=$(COMPATH)$/lib
.ENDIF
MINGWSSTDOBJ=$(MINGW_CLIB_DIR)$/crtbegin.o
MINGWSSTDENDOBJ=$(MINGW_CLIB_DIR)$/crtend.o
LINKFLAGSAPPGUI=-mwindows $(MINGWLIBDIR)$/crt2.o
LINKFLAGSSHLGUI=--warn-once -mwindows -shared $(MINGWLIBDIR)$/dllcrt2.o
LINKFLAGSAPPCUI=-mconsole $(MINGWLIBDIR)$/crt2.o
LINKFLAGSSHLCUI=--warn-once -mconsole -shared $(MINGWLIBDIR)$/dllcrt2.o
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

.IF "$(MINGW_SHARED_GXXLIB)"=="YES" && "$(DYNAMIC_CRT)"!=""
STDLIBCPP=$(MINGW_SHARED_LIBSTDCPP)
.ELSE
STDLIBCPP=-lstdc++
.ENDIF
UWINAPILIB*=$(DYNAMIC) -luwinapi

.IF "$(MINGW_SHARED_GCCLIB)"=="YES" && "$(DYNAMIC_CRT)"!=""
MINGW_LIBGCC=-lgcc_s -lgcc
LINKFLAGS+=-shared-libgcc
.ELSE
.IF "$(MINGW_GCCLIB_EH)"=="YES"
MINGW_LIBGCC=-lgcc_eh -lgcc
.ELSE
MINGW_LIBGCC=-lgcc
.ENDIF
.ENDIF
STDOBJVCL=$(L)$/salmain.obj
STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=
STDLIBGUIMT=-Wl,--start-group CPPRUNTIME $(MINGW_LIBGCC)
STDLIBCUIMT=-Wl,--start-group CPPRUNTIME $(MINGW_LIBGCC)
STDSHLGUIMT=-Wl,--start-group CPPRUNTIME $(MINGW_LIBGCC)
STDSHLCUIMT=-Wl,--start-group CPPRUNTIME $(MINGW_LIBGCC)
.IF  "$(DYNAMIC_CRT)"!=""
STDLIBGUIMT+=-lmingwthrd
STDLIBCUIMT+=-lmingwthrd
STDSHLGUIMT+=-lmingwthrd
STDSHLCUIMT+=-lmingwthrd
.ENDIF
STDLIBGUIMT+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt
STDLIBCUIMT+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt
STDSHLGUIMT+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt
STDSHLCUIMT+=-lmingw32 -lmoldname -lmingwex -Wl,--end-group $(UWINAPILIB) -lm -lkernel32 -luser32 -lmsvcrt

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
WININETLIB=-lwininet
OLDNAMESLIB=-lmoldname
MSIMG32LIB=$(PSDK_HOME)$/lib$/msimg32.lib
PROPSYSLIB=-lpropsys
