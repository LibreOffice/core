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

PRJNAME=sw
TARGET=unoidl
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=no

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(DVO_XFORMS)" != ""
CDEFS+=-DDVO_XFORMS
.ENDIF

# --- Files --------------------------------------------------------

SLO1FILES =  \
        $(SLO)$/unodefaults.obj\
        $(SLO)$/unodispatch.obj\
        $(SLO)$/unotxdoc.obj\
        $(SLO)$/unoatxt.obj \
        $(SLO)$/unomailmerge.obj \
        $(SLO)$/unomod.obj \
        $(SLO)$/unotxvw.obj \
        $(SLO)$/dlelstnr.obj \
        $(SLO)$/unofreg.obj \
        $(SLO)$/SwXDocumentSettings.obj \
        $(SLO)$/SwXPrintPreviewSettings.obj \
        $(SLO)$/SwXFilterOptions.obj\
        $(SLO)$/RefreshListenerContainer.obj \
        $(SLO)$/unomodule.obj \
        $(SLO)$/unodoc.obj 

SLO2FILES = \
    $(SLO)$/swdetect.obj		\
    $(SLO)$/swdet2.obj		\
    $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO1FILES) \
    $(SLO2FILES)

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

