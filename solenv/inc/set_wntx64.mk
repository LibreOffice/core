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
###############################################
#        Windows 64 bit special
#	only used for 64 bit shell extension
#          ( Windows Vista )
# ---------------------------------------------
# THIS FILE WILL BE DELETED when a fully ported
# Windows 64 bit version is available
###############################################

.IF "$(BUILD_X64)"!=""
# ----------------- settings for x64 --------------------
CC_X64*:=$(WRAPCMD) $(CXX_X64_BINARY)
CXX_X64*:=$(WRAPCMD) $(CXX_X64_BINARY)
LINK_X64*:=$(WRAPCMD) $(LINK_X64_BINARY) $(NOLOGO) -MACHINE:X64
LIBMGR_X64=$(WRAPCMD) $(LIBMGR_X64_BINARY) $(NOLOGO)
IMPLIB_X64=$(WRAPCMD) $(LIBMGR_X64_BINARY)

USE_CFLAGS_X64=-c -nologo -Gs $(NOLOGO) -Zm500 -Zc:forScope,wchar_t- -GR

# Stack buffer overrun detection.
CFLAGS+=-GS

USE_CDEFS_X64+= -DWIN32 -D_AMD64_=1 -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_NON_CONFORMING_SWPRINTFS
.IF "$(debug)"!=""
USE_CFLAGS_X64+=-Zi -Fd$(MISC_X64)/$(@:b).pdb
USE_CDEFS_X64+=-DDEBUG
.ENDIF # "$(debug)"!=""
USE_CFLAGS_X64+=$(CFLAGS_X64)
USE_CDEFS_X64+=$(CDEFS_X64)
INCLUDE_X64=$(subst,/stl$(SPACECHAR),dont_use_stl$(SPACECHAR) $(INCLUDE))

.IF "$(product)" != ""
USE_CFLAGS_X64+=-Gy
.ENDIF # "$(product)" != ""

# inline expansion
USE_CFLAGS_X64+=-Ob1

.IF "$(DYNAMIC_CRT)"!=""
CDEFSSLOMT_X64+=-D_MT -D_DLL
CDEFSSLOMT_X64+=-D_MT -D_DLL
.IF "$(NO_DYNAMIC_OBJ)"==""
CDEFSOBJMT_X64+=-D_MT -D_DLL
CDEFSOBJMT_X64+=-D_MT -D_DLL
.ELSE
CDEFSOBJMT_X64+=-D_MT
CDEFSOBJMT_X64+=-D_MT
.ENDIF # "$(NO_DYNAMIC_OBJ)"==""
.ELSE
CDEFSSLOMT_X64+=-D_MT
CDEFSOBJMT_X64+=-D_MT
.ENDIF # "$(DYNAMIC_CRT)"!=""

.IF "$(PRODUCT)"!="full"
.ELSE
LINKFLAGS_X64=/MAP /OPT:NOREF
.ENDIF

# excetion handling protection
LINKFLAGS+=-safeseh

# enable DEP
LINKFLAGS+=-nxcompat

# enable ASLR
LINKFLAGS+=-dynamicbase

.IF "$(PRODUCT)"!="full"
LINKFLAGS_X64+= -MD -DEBUG
.ELSE # "$(PRODUCT)"!="full"
LINKFLAGS_X64+= -MD -RELEASE -DEBUG -INCREMENTAL:NO
.ENDIF # "$(PRODUCT)"!="full"
MAPFILE=-out:$$@

.IF "$(debug)" != ""
LINKFLAGS_X64+= $(LINKFLAGSDEBUG)
.ENDIF

LINKFLAGSSHLCUI_X64=/SUBSYSTEM:CONSOLE /DLL
LINKFLAGSSHL_X64=$(LINKFLAGSSHLCUI_X64)
CDEFSSLO_X64+=$(CDEFSMT_X64) $(CDEFSOBJMT_X64)
CFLAGSSLO_X64+=-MT

STDOBJGUI_X64=
STDSLOGUI_X64=
STDOBJCUI_X64=
STDSLOCUI_X64=

IMPLIBFLAGS_X64=-machine:X64

LIBPATH_X64=$(PSDK)/lib/x64
LIBPATH_VC_X64=$(COMPATH)/lib/amd64

ADVAPI32LIB_X64=$(LIBPATH_X64)/advapi32.lib
SHELL32LIB_X64=$(LIBPATH_X64)/shell32.lib
GDI32LIB_X64=$(LIBPATH_X64)/gdi32.lib
OLE32LIB_X64=$(LIBPATH_X64)/ole32.lib
OLEAUT32LIB_X64=$(LIBPATH_X64)/oleaut32.lib
UUIDLIB_X64=$(LIBPATH_X64)/uuid.lib
WINSPOOLLIB_X64=$(LIBPATH_X64)/winspool.lib
IMM32LIB_X64=$(LIBPATH_X64)/imm32.lib
VERSIONLIB_X64=$(LIBPATH_X64)/version.lib
WINMMLIB_X64=$(LIBPATH_X64)/winmm.lib
WSOCK32LIB_X64=$(LIBPATH_X64)/wsock32.lib
MPRLIB_X64=$(LIBPATH_X64)/mpr.lib
WS2_32LIB_X64=$(LIBPATH_X64)/ws2_32.lib
KERNEL32LIB_X64=$(LIBPATH_X64)/kernel32.lib
USER32LIB_X64=$(LIBPATH_X64)/user32.lib
COMDLG32LIB_X64=$(LIBPATH_X64)/comdlg32.lib
COMCTL32LIB_X64=$(LIBPATH_X64)/comctl32.lib
CRYPT32LIB_X64=$(LIBPATH_X64)/crypt32.lib
GDIPLUSLIB_X64=$(LIBPATH_X64)/gdiplus.lib
DBGHELPLIB_X64=$(LIBPATH_X64)/dbghelp.lib
PROPSYSLIB_X64=$(LIBPATH_X64)/propsys.lib
MSILIB_X64=$(LIBPATH_X64)/msi.lib
DDRAWLIB_X64=$(LIBPATH_X64)/ddraw.lib
SHLWAPILIB_X64=$(LIBPATH_X64)/shlwapi.lib
URLMONLIB_X64=$(LIBPATH_X64)/urlmon.lib
WININETLIB_X64=$(LIBPATH_X64)/wininet.lib
OLDNAMESLIB_X64=$(LIBPATH_VC_X64)/oldnames.lib
MSIMG32LIB_X64=$(LIBPATH_X64)/msimg32.lib
MSVCPRT_X64=$(LIBPATH_VC_X64)/msvcprt.lib
MSVCRT_X64=$(LIBPATH_VC_X64)/msvcrt.lib

MISC_X64=$(MISC)/x64
OBJ_X64=$(OBJ)/x64
SLO_X64=$(SLO)/x64
LB_X64=$(LB)/x64
SLB_X64=$(SLB)/x64
L_X64=$(SOLARLIBDIR_X64)
VERSIONOBJ_X64=$(SLO_X64)/_version.obj
BIN_X64=$(BIN)/x64
RES_X64=$(RES)/x64
SOLARLIBDIR_X64=$(SOLARVERSION)/$(INPATH)/lib$(UPDMINOREXT)/x64
LIB_X64:=$(LB_X64);$(SLB_X64);$(ILIB_X64)

.IF "$(LIBTARGET)"==""
.IF "$(OBJFILES_X64)$(IDLOBJFILES_X64)"!=""
OBJTARGET_X64=$(LB_X64)/$(TARGET).lib
.ENDIF # "$(OBJFILES_X64)$(IDLOBJFILES_X64)"!=""
.IF "$(SLOFILES_X64)$(IDLSLOFILES_x64)"!=""
SLOTARGET_X64=$(SLB_X64)/$(TARGET).lib
.ENDIF # "$(SLOFILES_X64)$(IDLSLOFILES_x64)"!=""
.ENDIF # "$(LIBTARGET)"==""

.IF "$(OBJFILES_X64)"!=""
.IF "$(LIBTARGET)" != ""
NOLIBOBJTARGET_X64=$(OBJFILES_X64)
.ENDIF
.ENDIF

.IF "$(SLOFILES_X64)"!=""
.IF "$(LIBTARGET)" != ""
NOLIBSLOTARGET_X64=$(SLOFILES_X64)
.ENDIF
.ENDIF

.IF "$(SHL1TARGET_X64)"!=""
SHL1TARGETN_X64=$(BIN_X64)/$(DLLPRE)$(SHL1TARGET_X64)$(DLLPOST)
.ENDIF
.IF "$(SHL2TARGET_X64)"!=""
SHL2TARGETN_X64=$(BIN_X64)/$(DLLPRE)$(SHL2TARGET_X64)$(DLLPOST)
.ENDIF
.IF "$(LIB1TARGET_X64)" != ""
LIB1TARGETN_X64=$(LIB1TARGET_X64)
.ENDIF
.IF "$(LIB2TARGET_X64)" != ""
LIB2TARGETN_X64=$(LIB2TARGET_X64)
.ENDIF
.IF "$(DEF1NAME_X64)"!=""
DEF1TARGETN_X64=$(MISC_X64)/$(DEF1NAME_X64).def
.ENDIF
.IF "$(DEF2NAME_X64)"!=""
DEF2TARGETN_X64=$(MISC_X64)/$(DEF2NAME_X64).def
.ENDIF

$(SLO_X64)/%.obj : $(MISC_X64)/%.c
    @echo ------------------------------
    @echo Making: $@
    -$(MKDIR) $(@:d)
    @-$(MKDIR) $(MISC_X64)
    @@-$(RM) $@
    @$(TYPE) $(mktmp $(CC_X64) $(USE_CFLAGS_X64) $(INCLUDE_C) $(CFLAGSCC_X64) $(CFLAGSSLO_X64) $(USE_CDEFS_X64) $(CDEFSSLO_X64) $(CFLAGSAPPEND_X64) $(CFLAGSOUTOBJ)$(SLO_X64)/$*.obj $(MISC_X64)/$*.c )
    @$(ECHONL)
    $(CC_X64) @$(mktmp $(USE_CFLAGS_X64) $(INCLUDE_C) $(CFLAGSCC_X64) $(CFLAGSSLO_X64) $(USE_CDEFS_X64) $(CDEFSSLO_X64) $(CFLAGSAPPEND_X64) $(CFLAGSOUTOBJ)$(SLO_X64)/$*.obj $(MISC_X64)/$*.c )

.ENDIF # "$(BUILD_X64)"!=""

