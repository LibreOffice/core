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

PRJ = ..$/..
PRJNAME = cli_ure

TARGET = cli_uno
NO_BSYMBOLIC = TRUE
ENABLE_EXCEPTIONS = TRUE
USE_DEFFILE = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk


.IF "$(COM)" == "MSC"
# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
.IF "$(CCNUMVER)" <= "001399999999"
CFLAGSCXX += -clr -AI $(DLLDEST) -AI $(SOLARBINDIR) -wd4339
.ELSE
CFLAGSCXX += -clr:oldSyntax -AI $(DLLDEST) -AI $(SOLARBINDIR) -wd4339
.ENDIF

.IF "$(debug)" != ""
CFLAGS += -Ob0
.ENDIF



.IF "$(CCNUMVER)" <= "001399999999"
#see  Microsoft Knowledge Base Article - 814472
LINKFLAGS += -NOENTRY -NODEFAULTLIB:nochkclr.obj -INCLUDE:__DllMainCRTStartup@12
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

.IF "$(CCNUMVER)" >= "001399999999"
SHL1STDLIBS += \
    msvcmrt.lib
.ENDIF

SHL1VERSIONMAP = bridge_exports.map

SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
