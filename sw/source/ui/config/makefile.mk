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
TARGET=config
LIBTARGET=NO
# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------
IMGLST_SRS=$(SRS)$/config.srs
BMP_IN=$(PRJ)$/win/imglst

SRS1NAME=$(TARGET)
SRC1FILES =	\
        mailconfigpage.src \
        optdlg.src \
        optload.src \
        optcomp.src \
        redlopt.src

EXCEPTIONSFILES = \
        $(SLO)$/mailconfigpage.obj \
        $(SLO)$/optcomp.obj \
        $(SLO)$/optload.obj

SLOFILES =	\
        $(EXCEPTIONSFILES) \
        $(SLO)$/barcfg.obj \
        $(SLO)$/caption.obj \
        $(SLO)$/cfgitems.obj \
        $(SLO)$/dbconfig.obj \
        $(SLO)$/fontcfg.obj \
        $(SLO)$/modcfg.obj \
        $(SLO)$/optpage.obj \
        $(SLO)$/prtopt.obj \
        $(SLO)$/uinums.obj \
        $(SLO)$/usrpref.obj \
        $(SLO)$/viewopt.obj

LIB1TARGET= $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
        $(SLO)$/barcfg.obj \
        $(SLO)$/caption.obj \
        $(SLO)$/cfgitems.obj \
        $(SLO)$/dbconfig.obj \
        $(SLO)$/fontcfg.obj \
        $(SLO)$/modcfg.obj \
        $(SLO)$/prtopt.obj \
        $(SLO)$/uinums.obj \
        $(SLO)$/usrpref.obj \
        $(SLO)$/viewopt.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk