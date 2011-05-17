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

PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=shlxtmsi

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE

.INCLUDE : settings.mk

#Disable precompiled header
CDEFS+=-Dnot_used_define_to_disable_pch

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="WNT"

UWINAPILIB=

SLOFILES = \
    $(SLO)$/shellextensions.obj \
    $(SLO)$/startmenuicon.obj \
    $(SLO)$/upgrade.obj \
    $(SLO)$/iconcache.obj \
    $(SLO)$/postuninstall.obj \
    $(SLO)$/migrateinstallpath.obj \
    $(SLO)$/completeinstallpath.obj \
    $(SLO)$/checkdirectory.obj \
    $(SLO)$/setadmininstall.obj \
    $(SLO)$/layerlinks.obj \
    $(SLO)$/dotnetcheck.obj \
    $(SLO)$/registerextensions.obj \
    $(SLO)$/copyeditiondata.obj \
    $(SLO)$/vistaspecial.obj \
    $(SLO)$/checkrunningoffice.obj \
    $(SLO)$/checkpatches.obj \
    $(SLO)$/copyextensiondata.obj

STDSHL += \
    $(ADVAPI32LIB)\
    $(MSILIB)\
        $(SHELL32LIB)								

.IF "$(COM)"=="GCC"
STDSHL+=	\
    $(KERNEL32LIB)\
    -lmsvcrt
.ENDIF

SHL1OBJS = $(SLOFILES) \
    $(SLO)$/seterror.obj

SHL1TARGET = $(TARGET)
SHL1IMPLIB = i$(TARGET)

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1DEPN = $(SLB)$/$(TARGET).lib
SHL1BASE = 0x1c000000
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

.ENDIF

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------


