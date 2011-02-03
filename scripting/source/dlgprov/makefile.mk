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

PRJNAME=scripting
TARGET=dlgprov
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

SLOFILES= \
        $(SLO)$/dlgprov.obj \
        $(SLO)$/DialogModelProvider.obj \
        $(SLO)$/dlgevtatt.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX).uno
SHL1IMPLIB=	i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS= \
        $(SFX2LIB) \
        $(VCLLIB) \
        $(TOOLSLIB) \
        $(XMLSCRIPTLIB) \
        $(CPPUHELPERLIB) \
        $(COMPHELPERLIB) \
        $(UCBHELPERLIB) \
        $(CPPULIB) \
        $(BASICLIB) \
        $(SALLIB)

SHL1DEPN=
SHL1LIBS=$(SLB)$/$(TARGET).lib

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

# ------------------------------------------------------------------

ALLTAR : \
        $(MISC)$/$(TARGET).don \

$(SLOFILES) : $(MISC)$/$(TARGET).don

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(INCCOM)$/$(TARGET) -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@
 

ALLTAR : $(MISC)/dlgprov.component

$(MISC)/dlgprov.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dlgprov.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dlgprov.component
