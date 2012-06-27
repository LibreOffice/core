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

PRJ = ..$/..
PRJNAME = cli_ure

TARGET = cli_uno
ENABLE_EXCEPTIONS = TRUE
USE_DEFFILE = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(COM)" == "MSC"
# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
CFLAGSCXX += -clr:oldSyntax -AI $(DLLDEST) -AI $(SOLARBINDIR) -wd4339

.IF "$(debug)" != ""
CFLAGS += -Ob0
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/cli_environment.obj	\
    $(SLO)$/cli_bridge.obj		\
    $(SLO)$/cli_data.obj		\
    $(SLO)$/cli_proxy.obj		\
    $(SLO)$/cli_uno.obj

SHL1OBJS = $(SLOFILES)

SHL1TARGET = $(TARGET)

SHL1STDLIBS = \
    $(CPPULIB)			\
    $(SALLIB)			\
    mscoree.lib

.IF "$(USE_DEBUG_RUNTIME)" == ""
SHL1STDLIBS += \
    msvcmrt.lib
.ELSE
SHL1STDLIBS += \
    msvcmrtd.lib
.ENDIF

SHL1VERSIONMAP = bridge_exports.map

SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
