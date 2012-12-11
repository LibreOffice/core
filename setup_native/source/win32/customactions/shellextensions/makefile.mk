#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=..$/..$/..$/..
PRJNAME=setup_native
TARGET=shlxtmsi

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(OS)"=="WNT" && "$(WINDOWS_SDK_HOME)"!=""

UWINAPILIB=

SLOFILES = \
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
    $(SLO)$/copyeditiondata.obj \
    $(SLO)$/vistaspecial.obj \
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
.ELSE
STDSHL+=    \
    psapi.lib
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
