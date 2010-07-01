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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=unoidl
ENABLE_EXCEPTIONS=TRUE
AUTOSEG=true
LIBTARGET = NO
PRJINC=..$/slidesorter

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

.IF "$(COM)"=="GCC"
NOOPTFILES= $(SLO)$/unowcntr.obj
.ENDIF

SLO1FILES =      \
        $(SLO)$/DrawController.obj \
        $(SLO)$/SdUnoSlideView.obj\
        $(SLO)$/SdUnoOutlineView.obj\
        $(SLO)$/SdUnoDrawView.obj \
        $(SLO)$/unopool.obj \
        $(SLO)$/UnoDocumentSettings.obj \
        $(SLO)$/facreg.obj \
        $(SLO)$/unomodel.obj    \
        $(SLO)$/unopage.obj     \
        $(SLO)$/unolayer.obj    \
        $(SLO)$/unocpres.obj    \
        $(SLO)$/unoobj.obj		\
        $(SLO)$/unosrch.obj		\
        $(SLO)$/unowcntr.obj	\
        $(SLO)$/unokywds.obj	\
        $(SLO)$/unopback.obj	\
        $(SLO)$/unodoc.obj      \
        $(SLO)$/unomodule.obj	\
        $(SLO)$/randomnode.obj

SLO2FILES = \
    $(SLO)$/sddetect.obj		\
    $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO1FILES) \
    $(SLO2FILES)

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

