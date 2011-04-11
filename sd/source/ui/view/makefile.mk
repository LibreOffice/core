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
TARGET=view

AUTOSEG=true
PRJINC=..$/slidesorter
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =	\
        DocumentRenderer.src

SLOFILES =	\
        $(SLO)$/sdview.obj	\
        $(SLO)$/sdview2.obj	\
        $(SLO)$/sdview3.obj	\
        $(SLO)$/sdview4.obj	\
        $(SLO)$/sdview5.obj	\
        $(SLO)$/viewshel.obj \
        $(SLO)$/viewshe2.obj \
        $(SLO)$/viewshe3.obj \
        $(SLO)$/sdwindow.obj \
        $(SLO)$/drviewsh.obj \
        $(SLO)$/drviews1.obj \
        $(SLO)$/drviews2.obj \
        $(SLO)$/drviews3.obj \
        $(SLO)$/drviews4.obj \
        $(SLO)$/drviews5.obj \
        $(SLO)$/drviews6.obj \
        $(SLO)$/drviews7.obj \
        $(SLO)$/drviews8.obj \
        $(SLO)$/drviews9.obj \
        $(SLO)$/drviewsa.obj \
        $(SLO)$/drviewsb.obj \
        $(SLO)$/drviewsc.obj \
        $(SLO)$/drviewsd.obj \
        $(SLO)$/drviewse.obj \
        $(SLO)$/drviewsf.obj \
        $(SLO)$/drviewsg.obj \
        $(SLO)$/drviewsi.obj \
        $(SLO)$/drviewsj.obj \
        $(SLO)$/drvwshrg.obj \
        $(SLO)$/drawview.obj \
        $(SLO)$/grviewsh.obj \
        $(SLO)$/outlnvsh.obj \
        $(SLO)$/outlnvs2.obj \
        $(SLO)$/presvish.obj \
        $(SLO)$/outlview.obj \
        $(SLO)$/tabcontr.obj \
        $(SLO)$/drbezob.obj	\
        $(SLO)$/drtxtob.obj	\
        $(SLO)$/drtxtob1.obj	\
        $(SLO)$/sdruler.obj	\
        $(SLO)$/frmview.obj	\
        $(SLO)$/clview.obj  \
        $(SLO)$/zoomlist.obj	\
        $(SLO)$/unmodpg.obj	\
        $(SLO)$/DocumentRenderer.obj			\
        $(SLO)$/FormShellManager.obj			\
        $(SLO)$/GraphicObjectBar.obj			\
        $(SLO)$/GraphicViewShellBase.obj		\
        $(SLO)$/ImpressViewShellBase.obj		\
        $(SLO)$/MediaObjectBar.obj				\
        $(SLO)$/Outliner.obj					\
        $(SLO)$/OutlinerIterator.obj			\
        $(SLO)$/OutlineViewShellBase.obj		\
        $(SLO)$/SlideSorterViewShellBase.obj	\
        $(SLO)$/PresentationViewShellBase.obj	\
        $(SLO)$/ToolBarManager.obj				\
        $(SLO)$/UpdateLockManager.obj			\
        $(SLO)$/ViewClipboard.obj				\
        $(SLO)$/ViewShellBase.obj				\
        $(SLO)$/ViewShellImplementation.obj		\
        $(SLO)$/ViewShellManager.obj			\
        $(SLO)$/ViewShellHint.obj				\
        $(SLO)$/ViewTabBar.obj					\
        $(SLO)$/WindowUpdater.obj				\
        $(SLO)$/viewoverlaymanager.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

