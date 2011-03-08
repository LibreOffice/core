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
PRJNAME=filter
TARGET=xsltdlg

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
GEN_HID_OTHER=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =	xmlfiltersettingsdialog.src							\
            xmlfiltertabdialog.src								\
            xmlfiltertabpagebasic.src							\
            xmlfiltertabpagexslt.src							\
            xmlfiltertestdialog.src								\
            xmlfileview.src										\
            xmlfilterdialogstrings.src

SLOFILES=	$(SLO)$/typedetectionimport.obj						\
            $(SLO)$/typedetectionexport.obj						\
            $(SLO)$/xmlfilterjar.obj							\
            $(SLO)$/xmlfilterdialogcomponent.obj				\
            $(SLO)$/xmlfiltersettingsdialog.obj					\
            $(SLO)$/xmlfiltertabdialog.obj						\
            $(SLO)$/xmlfiltertabpagebasic.obj					\
            $(SLO)$/xmlfiltertabpagexslt.obj					\
            $(SLO)$/xmlfiltertestdialog.obj						\
            $(SLO)$/xmlfileview.obj

# --- Library -----------------------------------

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES= $(SRS)$/$(TARGET).srs

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
    
SHL1STDLIBS=\
    $(SFX2LIB)			\
    $(SVTOOLLIB)		\
    $(SVLLIB)			\
    $(TKLIB)			\
    $(CPPULIB)			\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(SALLIB)			\
    $(TOOLSLIB)			\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(ONELIB)

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk
