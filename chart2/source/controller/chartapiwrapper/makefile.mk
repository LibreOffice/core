#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.4.44.3 $
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

PRJ=				..$/..$/..
PRJNAME=			chart2
TARGET=				chchartapiwrapper

PRJINC=				$(PRJ)$/source

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- export library -------------------------------------------------

#Specifies object files to bind into linked libraries.
SLOFILES=	\
    $(SLO)$/Chart2ModelContact.obj \
    $(SLO)$/AreaWrapper.obj \
    $(SLO)$/AxisWrapper.obj \
    $(SLO)$/ChartDataWrapper.obj \
    $(SLO)$/ChartDocumentWrapper.obj \
    $(SLO)$/DataSeriesPointWrapper.obj \
    $(SLO)$/DiagramWrapper.obj \
    $(SLO)$/GridWrapper.obj \
    $(SLO)$/LegendWrapper.obj \
    $(SLO)$/TitleWrapper.obj \
    $(SLO)$/MinMaxLineWrapper.obj \
    $(SLO)$/UpDownBarWrapper.obj \
    $(SLO)$/WallFloorWrapper.obj \
    $(SLO)$/WrappedAutomaticPositionProperties.obj \
    $(SLO)$/WrappedCharacterHeightProperty.obj \
    $(SLO)$/WrappedDataCaptionProperties.obj \
    $(SLO)$/WrappedTextRotationProperty.obj \
    $(SLO)$/WrappedGapwidthProperty.obj \
    $(SLO)$/WrappedScaleProperty.obj \
    $(SLO)$/WrappedSplineProperties.obj \
    $(SLO)$/WrappedStockProperties.obj \
    $(SLO)$/WrappedSymbolProperties.obj \
    $(SLO)$/WrappedAxisAndGridExistenceProperties.obj \
    $(SLO)$/WrappedNumberFormatProperty.obj \
    $(SLO)$/WrappedStatisticProperties.obj \
    $(SLO)$/WrappedSceneProperty.obj \
    $(SLO)$/WrappedSeriesAreaOrLineProperty.obj \
    $(SLO)$/WrappedAddInProperty.obj \
    $(SLO)$/WrappedScaleTextProperties.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
