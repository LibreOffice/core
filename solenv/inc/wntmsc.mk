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

# mk file for $(OS)$(COM)$(CPU)$(COMEX) == WNTMSC[IX]{12,13}

JAVAFLAGSDEBUG=-g

.IF "$(BITNESS_OVERRIDE)" == ""
ASM=ml
AFLAGS=/c /Cp /coff /safeseh
.ELSE
ASM=ml64
AFLAGS=/c /Cp
.ENDIF

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
ARCH_FLAGS*=

CC*:=cl
.IF "$(bndchk)" != ""
CXX*=nmcl
.ELSE
.IF "$(truetime)" != ""
CXX*=nmcl /NMttOn
.ELSE
CXX*:=cl
.ENDIF
.ENDIF # "$(bndchk)" != ""

.IF "$(stoponerror)" != ""
CXX+= /NMstoponerror
.ENDIF

.IF "$(nmpass)" != ""
CXX+= /NMpass
.ENDIF

.IF "$(ttinlines)" != ""
CXX+= /NMttInlines
.ENDIF

.IF "$(ttnolines)" != ""
CXX+= /NMttNoLines
.ENDIF

.IF "$(VERBOSE)" != "TRUE"
NOLOGO*=-nologo
.ENDIF

.IF "$(VERBOSE)" != "TRUE"
COMPILE_ECHO_SWITCH=-n
COMPILE_ECHO_FILE=
.ENDIF

# Flags for COMEX == 11

# disable "warning C4675: resolved overload was found by argument-dependent
# lookup":
# -wd4251 -wd4275 -wd4290 -wd4675 -wd4786 -wd4800
CFLAGS+=-Zm500 -Zc:forScope,wchar_t- -GR

# Stack buffer overrun detection.
CFLAGS+=-GS

CFLAGS+=-c -nologo -Gs $(NOLOGO)

.IF "$(BITNESS_OVERRIDE)" == ""
CDEFS+= -D_X86_=1 -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_NON_CONFORMING_SWPRINTFS -D_SCL_SECURE_NO_WARNINGS
.ELSE
CDEFS+= -D_AMD64_=1 -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_NON_CONFORMING_SWPRINTFS -D_SCL_SECURE_NO_WARNINGS
.ENDIF

.IF "$(product)" != ""
CFLAGS+= -Gy
.ENDIF # "$(product)" != ""

# Flags to enable build with symbols; old comment said "required for crashdump feature"
# but unclear if that is true or not any more; just use the same as CFLAGSDEBUG for now.
# That is what we do in gbuild anyway...
CFLAGSENABLESYMBOLS=-Zi -Fd$(MISC)/$(@:b).pdb

.IF "$(bndchk)" != ""
.IF "$(debug)" == ""
CFLAGS+= -Z7
.ENDIF
.ENDIF

.IF "$(truetime)" != ""
.IF "$(debug)" == ""
CFLAGS+= -Z7
.ENDIF
.ENDIF

CFLAGSEXCEPTIONS=-EHa
CFLAGS_NO_EXCEPTIONS=

.IF "$(BITNESS_OVERRIDE)" == ""
# enable boost support for __cdecl (SAL_CALL) C++-UNO interface methods
CDEFS+=-DBOOST_MEM_FN_ENABLE_CDECL
.ENDIF

CFLAGSCALL=-Gd

CFLAGSCC=$(ARCH_FLAGS)
.IF "$(DYNAMIC_CRT)"!=""
CDEFSSLOMT+=-D_DLL
.IF "$(NO_DYNAMIC_OBJ)"==""
CDEFSOBJMT+=-D_DLL
.ENDIF # "$(NO_DYNAMIC_OBJ)"==""
.ENDIF # "$(DYNAMIC_CRT)"!=""

CFLAGSPROF=-Gh -Fd$(MISC)/$(@:b).pdb
CFLAGSDEBUG=-Zi -Fd$(MISC)/$(@:b).pdb
CFLAGSDBGUTIL=
.IF "$(VC_STANDARD)"==""
CFLAGSOPT=-Oxs -Oy-
CFLAGSNOOPT=-Od
.ELSE			#  "$(VC_STANDARD)"==""
CFLAGSOPT=
CFLAGSNOOPT=
.ENDIF			#  "$(VC_STANDARD)"==""
CFLAGSOUTOBJ=-Fo

# For C and C++, certain warnings are disabled globally, as they result in
# spurious warnings and are hard or impossible to workaround:
# - "warning C4061: enumerate in switch of enum is not explicitly handled by a
#   case label",
# - "warning C4127: conditional expression is constant",
# - "warning C4191: unsafe conversion from function type to function type",
# - "warning C4217: member template functions cannot be used for copy-assignment
#   or copy-construction",
# - "warning C4250: 'class1' : inherits 'class2::member' via dominance",
# - "warning C4355: 'this' used in base member initializer list",
# - "warning C4511: copy constructor could not be generated",
# - "warning C4512: assignment operator could not be generated",
# - "warning C4514: unreferenced inline function has been removed",
# - "warning C4611: interaction between '_setjmp' and C++ object destruction is
#   non-portable",
# - "warning C4625: copy constructor could not be generated because a base class
#   copy constructor is inaccessible",
# - "warning C4626: assignment operator could not be generated because a base
#   class assignment operator is inaccessible",
# - "warning C4675: resolved overload was found by argument-dependent lookup",
# - "warning C4710: function not inlined",
# - "warning C4711: function selected for automatic inline expansion",
# - "warning C4820: padding added after member".
# - "warning C4503: 'identifier' : decorated name length exceeded, name was truncated"
#   (http://msdn2.microsoft.com/en-us/library/074af4b6.aspx)
# - "warning C4180: qualifier applied to function type has no meaning; ignored"
#   (frequently seen with a recent boost)
# - "warning C4265: 'identifier' : class has virtual functions, but destructor is not
#   virtual"
# For C, certain warnings from system headers (stdlib.h etc.) have to be
# disabled globally (for C++, this is not necessary, as the system headers are
# wrapped by STLport):
# - "warning C4255: no function prototype given: converting
#   '()' to '(void)'".
# - "warning C4365: conversion from ... to ... signed/unsigned mismatch"

CFLAGSWARNCXX=-Wall -wd4061 -wd4127 -wd4191 -wd4217 -wd4242 -wd4244 -wd4245 -wd4250 -wd4251 -wd4275 \
    -wd4290 -wd4294 -wd4355 -wd4511 -wd4512 -wd4514 -wd4555 -wd4611 -wd4625 -wd4626 \
    -wd4640 -wd4675 -wd4686 -wd4706 -wd4710 -wd4711 -wd4786 -wd4800 -wd4820 -wd4503 -wd4619 \
    -wd4365 -wd4668 -wd4738 -wd4826 -wd4350 -wd4505 -wd4692 -wd4189 -wd4005 \
    -wd4180 -wd4265
CFLAGSWARNCC=$(CFLAGSWARNCXX) -wd4255
CFLAGSWALLCC=$(CFLAGSWARNCC)
CFLAGSWALLCXX=$(CFLAGSWARNCXX)
CFLAGSWERRCC=-WX -DLIBO_WERROR

# Once all modules on this platform compile without warnings, set
# COMPILER_WARN_ERRORS=TRUE here instead of setting MODULES_WITH_WARNINGS (see
# settings.mk):
MODULES_WITH_WARNINGS := binfilter

CDEFS+=-DWIN32 -D_MT -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0500
.IF "$(COMEX)" == "11"
_VC_MANIFEST_BASENAME=__VC80
.ELSE
_VC_MANIFEST_BASENAME=__VC90
.ENDIF

.IF "$(BITNESS_OVERRIDE)" == ""
LINK=link /MACHINE:IX86 /IGNORE:4102 /IGNORE:4197
.ELSE
LINK=link /MACHINE:X64
.ENDIF
    # do *not* add $(NOLOGO) to LINK or LINKFLAGS. Strangely, the wntmsci12 linker links fine then, but exits with
    # a return value 1, which makes dmake think it failed
.IF "$(PRODUCT)"!="full"
.ELSE
LINKFLAGS=/MAP /OPT:NOREF
.ENDIF

# excetion handling protection
.IF "$(BITNESS_OVERRIDE)" == ""
LINKFLAGS+=-safeseh
.ENDIF

# enable DEP
LINKFLAGS+=-nxcompat

# enable ASLR
LINKFLAGS+=-dynamicbase

.IF "$(linkinc)" != ""
LINKFLAGS+=-NODEFAULTLIB -INCREMENTAL:YES -DEBUG
MAPFILE=
_VC_MANIFEST_INC=1
.ELSE # "$(linkinc)" != ""
_VC_MANIFEST_INC=0
.IF "$(PRODUCT)"!="full"
LINKFLAGS+= -NODEFAULTLIB -DEBUG
.ELSE # "$(PRODUCT)"!="full"
LINKFLAGS+= -NODEFAULTLIB -RELEASE -DEBUG -INCREMENTAL:NO
.ENDIF # "$(PRODUCT)"!="full"
MAPFILE=-out:$$@
.ENDIF # "$(linkinc)" != ""

.IF "$(bndchk)" != ""
LINK=nmlink $(COMMENTFLAG) $(NOLOGO) /MACHINE:IX86
LINKFLAGS=-NODEFAULTLIB -DEBUG
.ENDIF

.IF "$(truetime)" != ""
LINK=nmlink /NMttOn $(COMMENTFLAG) $(NOLOGO) /MACHINE:IX86
LINKFLAGS=-NODEFAULTLIB -DEBUG
.ENDIF

.IF "$(COMEX)" == "11"
LINKFLAGSAPPGUI=/SUBSYSTEM:WINDOWS,4.0
LINKFLAGSSHLGUI=/SUBSYSTEM:WINDOWS,4.0 /DLL
.ELSE
LINKFLAGSAPPGUI=/SUBSYSTEM:WINDOWS
LINKFLAGSSHLGUI=/SUBSYSTEM:WINDOWS /DLL
.ENDIF # "$(COMEX)" == "11"
LINKFLAGSAPPCUI=/SUBSYSTEM:CONSOLE /BASE:0x1b000000
LINKFLAGSSHLCUI=/SUBSYSTEM:CONSOLE /DLL
LINKFLAGSTACK=/STACK:
LINKFLAGSPROF=/DEBUG:mapped,partial /DEBUGTYPE:coff cap.lib
LINKFLAGSWST=/DEBUG:mapped,partial /DEBUGTYPE:coff wst.lib /NODEFAULTLIB
LINKFLAGSDEBUG=-DEBUG
LINKFLAGSOPT=

UWINAPILIB*=uwinapi.lib
.IF "$(DYNAMIC_CRT)"!=""
.IF "$(USE_DEBUG_RUNTIME)" != ""
LIBCMT=msvcrtd.lib
LIBCPMT=msvcprtd.lib
CDEFS+=-D_DEBUG
.ELSE  # "$(USE_DEBUG_RUNTIME)" != ""
LIBCMT=msvcrt.lib
LIBCPMT=msvcprt.lib
.ENDIF # "$(USE_DEBUG_RUNTIME)" != ""
.ELSE # "$(DYNAMIC_CRT)"!=""
.IF "$(USE_DEBUG_RUNTIME)" != ""
LIBCMT=libcmtd.lib
LIBCPMT=libcpmtd.lib
CDEFS+=-D_DEBUG
.ELSE  # "$(USE_DEBUG_RUNTIME)" != ""
LIBCMT=libcmt.lib
LIBCPMT=libcpmt.lib
.ENDIF # "$(USE_DEBUG_RUNTIME)" != ""
.ENDIF # "$(DYNAMIC_CRT)"!=""

STDOBJGUI=
STDSLOGUI=
STDOBJCUI=
STDSLOCUI=

STDLIBGUIMT=$(LIBCMT) $(LIBCPMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDLIBCUIMT=$(LIBCMT) $(LIBCPMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDSHLGUIMT=$(LIBCMT) $(LIBCPMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib
STDSHLCUIMT=$(LIBCMT) $(LIBCPMT) $(UWINAPILIB) kernel32.lib user32.lib oldnames.lib

LIBMGR=lib $(NOLOGO)
IMPLIB=lib
LIBFLAGS=

.IF "$(BITNESS_OVERRIDE)" == ""
IMPLIBFLAGS=-machine:IX86
.ELSE
IMPLIBFLAGS=-machine:X64
.ENDIF

MAPSYM=
MAPSYMFLAGS=

RC=rc
RCFLAGS=-r -DWIN32 $(RCFILES)
RCFLAGSOUTRES=-fo
RCLINK=rc
RCLINKFLAGS=
RCSETVERSION=

MT=mt.exe
MTFLAGS=$(NOLOGO)


CSC*=$(FLIPCMD) csc
VBC*=vbc

ADVAPI32LIB=advapi32.lib
SHELL32LIB=shell32.lib
GDI32LIB=gdi32.lib
OLE32LIB=ole32.lib
OLEAUT32LIB=oleaut32.lib
UUIDLIB=uuid.lib
WINSPOOLLIB=winspool.lib
IMM32LIB=imm32.lib
VERSIONLIB=version.lib
WINMMLIB=winmm.lib
MPRLIB=mpr.lib
WS2_32LIB=ws2_32.lib
KERNEL32LIB=kernel32.lib
USER32LIB=user32.lib
COMDLG32LIB=comdlg32.lib
COMCTL32LIB=comctl32.lib
CRYPT32LIB=crypt32.lib
GDIPLUSLIB=gdiplus.lib
DBGHELPLIB=dbghelp.lib
MSILIB=msi.lib
DDRAWLIB=ddraw.lib
SHLWAPILIB=shlwapi.lib
URLMONLIB=urlmon.lib
WININETLIB=wininet.lib
OLDNAMESLIB=oldnames.lib
MSIMG32LIB=msimg32.lib
PROPSYSLIB=propsys.lib
