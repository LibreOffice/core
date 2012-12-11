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
TARGET=quickstarter
TARGET1=sdqsmsi
TARGET2=qslnkmsi

# --- Settings -----------------------------------------------------

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=TRUE
DYNAMIC_CRT=
USE_DEFFILE=TRUE

.INCLUDE : settings.mk

UWINAPILIB=

# --- Files --------------------------------------------------------

.IF "$(OS)"=="WNT" && "$(WINDOWS_SDK_HOME)"!=""

STDSHL += \
    $(ADVAPI32LIB)\
    $(SHELL32LIB)\
    $(MSILIB)

.IF "$(COM)"=="GCC"
STDSHL+=	\
    $(KERNEL32LIB)\
    -lmsvcrt
.ENDIF

SHL1OBJS =	$(SLO)$/shutdown_quickstart.obj \
            $(SLO)$/quickstarter.obj

SHL1TARGET = $(TARGET1)
SHL1IMPLIB = i$(TARGET1)

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
SHL1BASE = 0x1c000000
DEF1NAME=$(SHL1TARGET)
SHL1DEPN=$(SHL1OBJS)
DEF1EXPORTFILE=$(TARGET1).dxp

# --- Files --------------------------------------------------------

SHL2OBJS =	$(SLO)$/remove_quickstart_link.obj \
            $(SLO)$/quickstarter.obj

SHL2TARGET = $(TARGET2)
SHL2IMPLIB = i$(TARGET2)

SHL2DEF = $(MISC)$/$(SHL2TARGET).def
SHL2BASE = 0x1c000000
DEF2NAME=$(SHL2TARGET)
SHL2DEPN=$(SHL1OBJS)
DEF2EXPORTFILE=$(TARGET2).dxp

.ENDIF

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk

# -------------------------------------------------------------------------
