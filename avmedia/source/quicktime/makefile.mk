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
TARGET=avmediaQuickTime

.IF "$(GUIBASE)"=="aqua"

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

# --- Files ----------------------------------

CFLAGSCXX+=$(OBJCXXFLAGS)

SLOFILES= \
        $(SLO)$/quicktimeuno.obj  \
        $(SLO)$/manager.obj       \
        $(SLO)$/window.obj        \
        $(SLO)$/framegrabber.obj        \
        $(SLO)$/player.obj

EXCEPTIONSFILES= \
        $(SLO)$/quicktimeuno.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
             $(CPPULIB) \
             $(SALLIB)  \
             $(COMPHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(TOOLSLIB) \
             $(VCLLIB) 

SHL1STDLIBS+= \
             -framework Cocoa \
             -framework QTKit \
             -framework QuickTime

# build DLL
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

.ELSE
dummy:
    @echo " Nothing to build for GUIBASE=$(GUIBASE)"
.ENDIF

ALLTAR : $(MISC)/avmediaQuickTime.component

$(MISC)/avmediaQuickTime.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt avmediaQuickTime.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt avmediaQuickTime.component
