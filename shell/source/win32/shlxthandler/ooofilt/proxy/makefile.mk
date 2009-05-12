#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.7 $
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

PRJ=..$/..$/..$/..$/..
PRJNAME=shell
TARGET=ooofiltproxy
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
DYNAMIC_CRT=
UWINAPILIB=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+=-DWIN32_LEAN_AND_MEAN -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CDEFS+=-D_WIN32_IE=0x501

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/ooofiltproxy.obj
SHL1TARGET=$(TARGET)
SHL1STDLIBS=$(OLE32LIB)\
     $(ADVAPO32LIB)\
     $(COMCTL32LIB)\
     $(UUIDLIB)\
     $(SHELL32LIB)\
     $(KERNEL32LIB)

SHL1DEPN=
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.IF "$(BUILD_X64)"!=""
#------------- x64 stuff ----------------
USE_DEFFILE_X64=TRUE
CFLAGS_X64+=-DWIN32_LEAN_AND_MEAN -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CDEFS_X64+=-D_WIN32_IE=0x501

SLOFILES_X64=$(SLO_X64)$/ooofiltproxy.obj
SHL1TARGET_X64=$(TARGET)
SHL1STDLIBS_X64=$(OLE32LIB_X64)\
     $(ADVAPO32LIB_X64)\
     $(COMCTL32LIB_X64)\
     $(UUIDLIB_X64)\
     $(SHELL32LIB_X64)\
     $(KERNEL32LIB_X64)\
     $(MSVCRT_X64)

SHL1DEPN_X64=
SHL1OBJS_X64=$(SLO_X64)$/ooofiltproxy.obj
SHL1DEF_X64=$(MISC_X64)$/$(TARGET).def
DEF1NAME_X64=$(TARGET)
DEF1EXPORTFILE_X64=exports.dxp
.ENDIF # "$(BUILD_X64)"!=""

# --- Targets ------------------------------------------------------

.INCLUDE :	set_wntx64.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_wntx64.mk
