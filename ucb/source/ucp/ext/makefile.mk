#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
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

PRJ = ..$/..$/..
PRJNAME = ucb
TARGET = ucpext.uno
ENABLE_EXCEPTIONS = TRUE

.INCLUDE : settings.mk
DLLPRE =

SLOFILES= \
        $(SLO)$/ucpext_provider.obj \
        $(SLO)$/ucpext_content.obj \
        $(SLO)$/ucpext_services.obj \
        $(SLO)$/ucpext_resultset.obj \
        $(SLO)$/ucpext_datasupplier.obj

SHL1STDLIBS = \
        $(UCBHELPERLIB)     \
        $(COMPHELPERLIB)    \
        $(CPPUHELPERLIB)    \
        $(CPPULIB)          \
        $(SALLIB)           \
        $(SALHELPERLIB)

SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1TARGET = $(TARGET)

SHL1DEPN =
SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def

DEF1NAME = $(SHL1TARGET)

.INCLUDE : target.mk

ALLTAR : $(MISC)/ucpext.component

$(MISC)/ucpext.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpext.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpext.component
