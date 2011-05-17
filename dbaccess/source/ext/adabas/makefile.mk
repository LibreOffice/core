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

PRJNAME=dbaccess
TARGET=adabasui
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------
.INCLUDE :	settings.mk

# --- resource -----------------------------------------------------
SRS1NAME=$(TARGET)
SRC1FILES =	\
        AdabasNewDb.src

# --- defines ------------------------------------------------------
CDEFS+=-DCOMPMOD_NAMESPACE=adabasui

# --- Files -------------------------------------
SLOFILES=\
        $(SLO)$/ASqlNameEdit.obj		\
        $(SLO)$/AdabasNewDb.obj			\
        $(SLO)$/ANewDb.obj				\
        $(SLO)$/adabasuistrings.obj		\
        $(SLO)$/Aservices.obj			\
        $(SLO)$/Acomponentmodule.obj

# --- Library -----------------------------------
SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(UNOTOOLSLIB)				\
    $(SVTOOLLIB)				\
    $(TOOLSLIB)					\
    $(SVLLIB)					\
    $(VCLLIB)					\
    $(UCBHELPERLIB) 			\
    $(SFXLIB)					\
    $(TKLIB)					\
    $(COMPHELPERLIB)

SHL1IMPLIB=i$(SHL1TARGET)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(TARGET).dxp
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- .res file ----------------------------------------------------------
RES1FILELIST=\
    $(SRS)$/$(SRS1NAME).srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)


# --- Targets ----------------------------------

.INCLUDE : target.mk

ALLTAR : $(MISC)/adabasui.component

$(MISC)/adabasui.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        adabasui.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt adabasui.component
