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
PRJ=..$/..$/..
BFPRJ=..$/..

PROJECTPCH=sd
PROJECTPCHSOURCE=$(BFPRJ)$/util$/sd_sd

PRJNAME=binfilter
TARGET=sd_core

#GEN_HID=TRUE
#GEN_HID_OTHER=TRUE
NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(BFPRJ)$/util$/makefile.pmk
INC+= -I$(PRJ)$/inc$/bf_sd
IENV+=-I..\ui\inc

# --- Files --------------------------------------------------------


SRS1NAME=$(TARGET)
SRC1FILES = sd_glob.src

SLOFILES = $(SLO)$/sd_stlsheet.obj  \
           $(SLO)$/sd_stlpool.obj	\
           $(SLO)$/sd_drawdoc.obj \
           $(SLO)$/sd_drawdoc2.obj \
           $(SLO)$/sd_drawdoc3.obj \
           $(SLO)$/sd_drawdoc4.obj \
           $(SLO)$/sd_sdpage.obj \
           $(SLO)$/sd_sdattr.obj \
           $(SLO)$/sd_sdobjfac.obj \
           $(SLO)$/sd_anminfo.obj	\
           $(SLO)$/sd_sdiocmpt.obj	\
           $(SLO)$/sd_sdpage2.obj	\
           $(SLO)$/sd_cusshow.obj

EXCEPTIONSFILES= \
           $(SLO)$/sd_drawdoc.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

