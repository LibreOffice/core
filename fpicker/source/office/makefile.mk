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

PRJNAME=fpicker
TARGET=fps_office
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE
GEN_HID=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE=

# --- Files --------------------------------------------------------

SLOFILES=\
    $(SLO)$/asyncfilepicker.obj \
    $(SLO)$/commonpicker.obj \
    $(SLO)$/OfficeControlAccess.obj \
    $(SLO)$/OfficeFilePicker.obj \
    $(SLO)$/OfficeFolderPicker.obj \
    $(SLO)$/fpinteraction.obj \
    $(SLO)$/fpsmartcontent.obj \
    $(SLO)$/fps_office.obj \
    $(SLO)$/iodlg.obj \
    $(SLO)$/iodlgimp.obj

SHL1TARGET=	$(TARGET).uno
SHL1IMPLIB=	i$(TARGET)
SHL1OBJS=	$(SLOFILES)
SHL1STDLIBS=\
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(UCBHELPERLIB) \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)

SRS1NAME=	$(TARGET)
SRC1FILES=	\
    OfficeFilePicker.src \
    iodlg.src

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=\
    $(SRS)$/fps_office.srs

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/fps_office.component

$(MISC)/fps_office.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fps_office.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fps_office.component
