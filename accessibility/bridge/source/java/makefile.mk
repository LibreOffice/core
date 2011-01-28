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

PRJ=..$/..$/..
PRJNAME=accessibility
TARGET=accessbridge
LIBTARGET=NO
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
VERSIONOBJ=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="WNT" && "$(SOLAR_JAVA)" != ""

SLOFILES= $(SLO)$/WindowsAccessBridgeAdapter.obj

SHL1TARGET=java_uno_accessbridge
SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=$(VCLLIB) $(TOOLSLIB) $(JVMACCESSLIB) $(CPPULIB) $(SALLIB)
SHL1OBJS=$(SLOFILES) 
SHL1VERSIONOBJ=

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

SHL1HEADER=$(OUT)$/inc$/WindowsAccessBridgeAdapter.h

.ENDIF			# "$(GUI)"=="WNT" && "$(SOLAR_JAVA)" != ""

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(GUI)"=="WNT" && "$(SOLAR_JAVA)" != ""

$(SLO)$/WindowsAccessBridgeAdapter.obj : $(SHL1HEADER)

$(SHL1HEADER) :
    javah -classpath $(OUT)$/class -o $(SHL1HEADER) org.openoffice.accessibility.WindowsAccessBridgeAdapter

.ENDIF			# "$(GUI)"=="WNT"  && "$(SOLAR_JAVA)" != ""
