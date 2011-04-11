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
PRJINC=..$/inc

PRJNAME=extensions
TARGET=dbp
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=dbp

# --- Files --------------------------------------------------------

CXXFILES=			dbptools.cxx	\
                    gridwizard.cxx	\
                    listcombowizard.cxx	\
                    optiongrouplayouter.cxx	\
                    commonpagesdbp.cxx	\
                    groupboxwiz.cxx	\
                    wizardservices.cxx	\
                    controlwizard.cxx	\
                    moduledbp.cxx	\
                    dbpservices.cxx

SLOFILES=			$(SLO)$/dbptools.obj	\
                    $(SLO)$/gridwizard.obj	\
                    $(SLO)$/listcombowizard.obj	\
                    $(SLO)$/optiongrouplayouter.obj	\
                    $(SLO)$/commonpagesdbp.obj	\
                    $(SLO)$/groupboxwiz.obj	\
                    $(SLO)$/wizardservices.obj	\
                    $(SLO)$/controlwizard.obj	\
                    $(SLO)$/moduledbp.obj	\
                    $(SLO)$/dbpservices.obj


SRS1NAME=$(TARGET)
SRC1FILES=			gridpages.src	\
                    commonpagesdbp.src	\
                    dbpilots.src	\
                    groupboxpages.src	\
                    listcombopages.src

RESLIB1NAME=dbp
RESLIB1SRSFILES= $(SRS)$/dbp.srs

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS= \
        $(SVTOOLLIB) \
        $(VCLLIB) \
        $(TOOLSLIB) \
        $(SVLLIB) \
        $(SFXLIB)		\
        $(DBTOOLSLIB) \
                $(UNOTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/dbp.component

$(MISC)/dbp.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dbp.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dbp.component
