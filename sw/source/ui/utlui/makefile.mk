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
TARGET=utlui

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------


SRS1NAME=$(TARGET)
SRC1FILES =  \
        initui.src \
        gloslst.src \
        navipi.src \
        poolfmt.src \
        attrdesc.src \
        unotools.src \
                utlui.src

EXCEPTIONSFILES= \
        $(SLO)$/bookctrl.obj \
        $(SLO)$/glbltree.obj \
        $(SLO)$/navipi.obj \
        $(SLO)$/unotools.obj    \
        $(SLO)$/content.obj \
        $(SLO)$/swrenamexnameddlg.obj

SLOFILES =  $(EXCEPTIONSFILES) \
        $(SLO)$/condedit.obj \
        $(SLO)$/gloslst.obj \
        $(SLO)$/initui.obj \
        $(SLO)$/navicfg.obj \
        $(SLO)$/numfmtlb.obj \
        $(SLO)$/prcntfld.obj \
        $(SLO)$/textcontrolcombo.obj \
        $(SLO)$/tmplctrl.obj \
        $(SLO)$/uitool.obj \
        $(SLO)$/uiitems.obj \
        $(SLO)$/attrdesc.obj \
        $(SLO)$/shdwcrsr.obj \
                $(SLO)$/zoomctrl.obj \
                $(SLO)$/viewlayoutctrl.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

