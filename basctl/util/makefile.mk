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

PRJ=..

PRJNAME=basctl
TARGET=basctl
#svx.hid generieren
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ----------------------------------------------------

# --- Basctl - DLL ----------

.IF "$(header)" == ""

HELPIDFILES=    ..$/inc$/helpid.hrc

SHL1TARGET= basctl$(DLLPOSTFIX)
SHL1IMPLIB= basctl
SHL1BASE  = 0x1d800000
SHL1STDLIBS= \
            $(EDITENGLIB) \
            $(SVXCORELIB) \
            $(SVXLIB) \
            $(SFX2LIB) \
            $(FWELIB) \
            $(BASICLIB) \
            $(SVTOOLLIB) \
            $(TKLIB) \
            $(VCLLIB) \
            $(SVLLIB) \
            $(SOTLIB) \
            $(UNOTOOLSLIB) \
            $(TOOLSLIB) \
            $(XMLSCRIPTLIB) \
            $(COMPHELPERLIB) \
            $(CPPUHELPERLIB) \
            $(UCBHELPERLIB) \
            $(CPPULIB) \
            $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(SHELLLIB)
.ENDIF # WNT


SHL1LIBS=       $(SLB)$/basctl.lib
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=basctl.map

DEF1NAME        =$(SHL1TARGET)

LIB1TARGET      =$(SLB)$/basctl.lib
LIB1FILES       = \
            $(LIBPRE) $(SLB)$/basicide.lib		\
            $(LIBPRE) $(SLB)$/dlged.lib			\
            $(LIBPRE) $(SLB)$/accessibility.lib

SRSFILELIST=\
                $(SRS)$/basicide.srs \
                $(SRS)$/dlged.srs

RESLIB1NAME=basctl
RESLIB1IMAGES =$(PRJ)$/res
RESLIB1SRSFILES= $(SRSFILELIST)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # "$(header)" == ""


ALLTAR : $(MISC)/basctl.component

$(MISC)/basctl.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        basctl.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt basctl.component
