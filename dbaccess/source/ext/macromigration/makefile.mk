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
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=dbmm
USE_DEFFILE=TRUE

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
        macromigration.src

# ... object files ............................
SLOFILES=   $(SLO)$/macromigrationwizard.obj \
            $(SLO)$/macromigrationdialog.obj \
            $(SLO)$/macromigrationpages.obj \
            $(SLO)$/dbmm_module.obj \
            $(SLO)$/dbmm_services.obj \
            $(SLO)$/migrationengine.obj \
            $(SLO)$/docinteraction.obj \
            $(SLO)$/progresscapture.obj \
            $(SLO)$/progressmixer.obj \
            $(SLO)$/migrationlog.obj \
            $(SLO)$/dbmm_types.obj \

# --- library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS= \
        $(CPPULIB)          \
        $(CPPUHELPERLIB)    \
        $(COMPHELPERLIB)    \
        $(UNOTOOLSLIB)      \
        $(TOOLSLIB)         \
        $(SALLIB)           \
        $(SVTOOLLIB)        \
        $(SVLLIB)           \
        $(VCLLIB)           \
        $(SVXCORELIB)           \
        $(SVXLIB)           \
        $(UCBHELPERLIB)     \
        $(XMLSCRIPTLIB)

SHL1LIBS=       $(SLB)$/$(TARGET).lib
SHL1IMPLIB=     i$(TARGET)
SHL1DEPN=       $(SHL1LIBS)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

DEF1NAME=       $(SHL1TARGET)

# --- .res files -------------------------------

RES1FILELIST=\
    $(SRS)$/$(TARGET).srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/dbmm.component

$(MISC)/dbmm.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        dbmm.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt dbmm.component
