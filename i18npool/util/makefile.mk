#*************************************************************************
#*
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
#************************************************************************

PRJ=..

PRJNAME=i18npool
TARGET=i18npool.uno

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
DLLPRE =
.INCLUDE :	$(PRJ)/version.mk

# --- Allgemein ----------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/defaultnumberingprovider.lib	\
        $(SLB)$/nativenumber.lib		\
        $(SLB)$/registerservices.lib		\
        $(SLB)$/numberformatcode.lib		\
        $(SLB)$/locale.lib			\
        $(SLB)$/indexentry.lib			\
        $(SLB)$/calendar.lib		\
        $(SLB)$/breakiterator.lib \
        $(SLB)$/transliterationImpl.lib \
        $(SLB)$/characterclassification.lib \
        $(SLB)$/collator.lib \
        $(SLB)$/inputchecker.lib \
        $(SLB)$/textconversion.lib \
        $(SLB)$/ordinalsuffix.lib

SHL1TARGET= $(TARGET)
# WNT needs implib name even if there is none
SHL1IMPLIB= i$(SHL1TARGET)

SHL1DEPN=	makefile.mk \
        $(SLB)/i18nisolang$(ISOLANG_MAJOR)$(COMID).lib
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1LIBS=$(LIB1TARGET)
SHL1STDLIBS= \
        $(I18NUTILLIB) \
        $(I18NISOLANGLIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
        $(ICUINLIB) \
        $(ICUUCLIB) \
        $(ICUDATALIB)

# --- Targets ------------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/i18npool.component

$(MISC)/i18npool.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        i18npool.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt i18npool.component
