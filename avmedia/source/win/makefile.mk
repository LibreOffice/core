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

PRJ=..$/..
PRJNAME=avmedia
TARGET=avmediawin

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(ENABLE_DIRECTX)" != ""

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

# --- Files ----------------------------------

.IF "$(GUI)" == "WNT"

SLOFILES= \
        $(SLO)$/winuno.obj      \
        $(SLO)$/manager.obj     \
        $(SLO)$/window.obj      \
        $(SLO)$/player.obj      \
        $(SLO)$/framegrabber.obj

EXCEPTIONSFILES= \
        $(SLO)$/winuno.obj      \
        $(SLO)$/framegrabber.obj    

SHL1TARGET=$(TARGET)
SHL1STDLIBS= $(CPPULIB) $(SALLIB) $(COMPHELPERLIB) $(CPPUHELPERLIB) $(UNOTOOLSLIB) $(TOOLSLIB) $(VCLLIB) 
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

SHL1STDLIBS += $(OLE32LIB)
SHL1STDLIBS += $(OLEAUT32LIB)
SHL1STDLIBS += $(GDI32LIB)

.IF "$(COM)"=="GCC"
SHL1STDLIBS += $(PSDK_HOME)$/lib$/strmiids.lib
.ELSE
SHL1STDLIBS += strmiids.lib
SHL1STDLIBS += dxguid.lib
.ENDIF

.ENDIF
.ENDIF

.INCLUDE :  	target.mk
