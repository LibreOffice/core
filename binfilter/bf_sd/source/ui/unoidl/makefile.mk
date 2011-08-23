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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..$/..
BFPRJ=..$/..$/..

PROJECTPCH=sd
PROJECTPCHSOURCE=$(BFPRJ)$/util$/sd_sd

PRJNAME=binfilter
TARGET=sd_unoidl

#GEN_HID=TRUE
#GEN_HID_OTHER=TRUE
NO_HIDS=TRUE

ENABLE_EXCEPTIONS=TRUE
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_sd
# --- Files --------------------------------------------------------

.IF "$(COM)"=="GCC"
NOOPTFILES= $(SLO)$/sd_unowcntr.obj
.ENDIF

SLOFILES =      \
        $(SLO)$/sd_unopool.obj \
        $(SLO)$/sd_UnoDocumentSettings.obj \
        $(SLO)$/sd_facreg.obj \
        $(SLO)$/sd_unomodel.obj    \
        $(SLO)$/sd_unopage.obj     \
        $(SLO)$/sd_unolayer.obj    \
        $(SLO)$/sd_unopres.obj     \
        $(SLO)$/sd_unocpres.obj    \
        $(SLO)$/sd_unoobj.obj		\
        $(SLO)$/sd_unowcntr.obj	\
        $(SLO)$/sd_unogsfm.obj		\
        $(SLO)$/sd_unogstyl.obj	\
        $(SLO)$/sd_unopsfm.obj		\
        $(SLO)$/sd_unopstyl.obj	\
        $(SLO)$/sd_unokywds.obj	\
        $(SLO)$/sd_unostyls.obj	\
        $(SLO)$/sd_unopback.obj \
        $(SLO)$/sd_unodoc.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

