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

SOLAR_JAVA*=
JAVAFLAGSDEBUG=-g

# SOLAR JAva Unterstuetzung nur fuer wntmsci

ASM=ml
AFLAGS=/c /Cp /coff

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=-march=pentium

CC*=i686-w64-mingw32-gcc
CXX*=i686-w64-mingw32-g++

CFLAGS+=-fmessage-length=0 -c

CFLAGSCC=-pipe $(ARCH_FLAGS)
CFLAGSCXX=-pipe $(ARCH_FLAGS)
.IF "$(HAVE_THREADSAFE_STATICS)" != "TRUE"
CFLAGSCXX += -fno-threadsafe-statics
.END

.IF "$(HAVE_CXX0X)" == "TRUE"
# FIXME still does not compile fully CFLAGSCXX+=-std=gnu++0x
.ENDIF

CFLAGSEXCEPTIONS=-fexceptions
CFLAGS_NO_EXCEPTIONS=-fno-exceptions
.IF "$(dbgutil)"==""
CFLAGSEXCEPTIONS+= -fno-enforce-eh-specs
.ENDIF
PICSWITCH:=

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
CFLAGSWARNBOTH=-Wall -Wextra -Wendif-labels
CFLAGSWARNCC=$(CFLAGSWARNBOTH) -Wdeclaration-after-statement
CFLAGSWARNCXX=$(CFLAGSWARNBOTH) -Wshadow -Wno-ctor-dtor-privacy \
    -Wno-non-virtual-dtor
CFLAGSWALLCC=$(CFLAGSWARNCC)
CFLAGSWALLCXX=$(CFLAGSWARNCXX)
CFLAGSWERRCC=-Werror -DLIBO_WERROR
CFLAGSWERRCXX=-Werror -DLIBO_WERROR

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
LINKFLAGS= -Wl,--enable-stdcall-fixup,--enable-runtime-pseudo-reloc-v2 -L$(CYGLIB)
MINGWLIBDIR=$(COMPATH)$/lib
MINGWSSTDOBJ=
MINGWSSTDENDOBJ=
LINKFLAGSAPPGUI=-mwindows 
LINKFLAGSSHLGUI=-Wl,--warn-once -mwindows -shared 
LINKFLAGSAPPCUI=-mconsole 
LINKFLAGSSHLCUI=-Wl,--warn-once -mconsole -shared
LINKFLAGSTACK=
LINKFLAGSPROF=
LINKFLAGSDEBUG=-g
LINKFLAGSOPT=

STDLIBCPP=-lstdc++
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

LIBMGR=$(AR)
LIBFLAGS=-rsu

MAPSYM=
MAPSYMFLAGS=

RC=$(WINDRES)
RCFLAGS=-D__MINGW32__ -DWIN32 -D_WIN32_IE=0x400 $(RCFILES)
RCFLAGSOUTRES=
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

ADVAPI32LIB=-ladvapi32
SHELL32LIB=-lshell32
GDI32LIB=-lgdi32
OLE32LIB=-lole32
OLEAUT32LIB=-loleaut32
UUIDLIB=-luuid
WINSPOOLLIB=-lwinspool
IMM32LIB=-limm32
VERSIONLIB=-lversion
WINMMLIB=-lwinmm
MPRLIB=-lmpr
WS2_32LIB=-lws2_32
KERNEL32LIB=-lkernel32
USER32LIB=-luser32
LIBCMT=-lmsvcrt
COMDLG32LIB=-lcomdlg32
COMCTL32LIB=-lcomctl32
CRYPT32LIB=-lcrypt32
DDRAWLIB=-lddraw
SHLWAPILIB=-lshlwapi
WININETLIB=-lwininet
OLDNAMESLIB=-lmoldname
MSIMG32LIB=-lmsimg32
PROPSYSLIB=-lpropsys

# Libraries for which we use an import library built
# from Wine's .def file
MSILIB=-lmsi
GDIPLUSLIB=-lgdiplus
URLMONLIB=-lurlmon

# Libraries that mingw-w64 has but mingw.org doesn't. At least the OBS MinGW
# cross-compiler is based on mingw-w64. When using MinGW natively on Windows
# (which as such I don't think we want to support) we  use the Windows SDK
# libraries.
.IF "$(CROSS_COMPILING)"=="YES"
DBGHELPLIB=-ldbghelp
.ELSE
DBGHELPLIB=$(WINDOWS_SDK_HOME)$/lib$/dbghelp.lib
.ENDIF
