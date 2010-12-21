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

PRJNAME=extensions
TARGET=bib
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=			$(SLO)$/formcontrolcontainer.obj \
                    $(SLO)$/loadlisteneradapter.obj \
                    $(SLO)$/bibconfig.obj \
                    $(SLO)$/bibcont.obj \
                    $(SLO)$/bibload.obj \
                    $(SLO)$/bibmod.obj \
                    $(SLO)$/general.obj \
                    $(SLO)$/framectr.obj \
                    $(SLO)$/bibview.obj \
                    $(SLO)$/bibbeam.obj  \
                    $(SLO)$/toolbar.obj  \
                    $(SLO)$/datman.obj	\


SRS1NAME=$(TARGET)
SRC1FILES=			bib.src	 \
                    datman.src \
                    sections.src \
                    toolbar.src

RESLIB1NAME=bib
RESLIB1SRSFILES= $(SRS)$/bib.srs
RESLIB1DEPN= bib.src bib.hrc


SHL1STDLIBS= \
        $(SOTLIB)           \
        $(SVTOOLLIB) \
        $(TKLIB) \
        $(VCLLIB) \
        $(SVLLIB) \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB) \
        $(DBTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1LIBS=       $(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


ALLTAR : $(MISC)/bib.component

$(MISC)/bib.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        bib.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt bib.component
