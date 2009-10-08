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
# $Revision: 1.8.44.1 $
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
TARGET=				chmodelmain

PRJINC=				$(PRJ)$/source

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- export library -------------------------------------------------

#Specifies object files to bind into linked libraries.
SLOFILES=	\
    $(SLO)$/Axis.obj \
    $(SLO)$/GridProperties.obj \
    $(SLO)$/BaseCoordinateSystem.obj \
    $(SLO)$/CartesianCoordinateSystem.obj \
    $(SLO)$/ChartData.obj		\
    $(SLO)$/ChartModel.obj		\
    $(SLO)$/ChartModel_Persistence.obj		\
    $(SLO)$/DataPoint.obj \
    $(SLO)$/DataPointProperties.obj \
    $(SLO)$/DataSeries.obj \
    $(SLO)$/Diagram.obj \
    $(SLO)$/DataSeriesProperties.obj \
    $(SLO)$/FormattedString.obj \
    $(SLO)$/ImplChartModel.obj \
    $(SLO)$/Legend.obj \
    $(SLO)$/PageBackground.obj \
    $(SLO)$/PolarCoordinateSystem.obj \
    $(SLO)$/StockBar.obj \
    $(SLO)$/Title.obj \
    $(SLO)$/Wall.obj \
    $(SLO)$/_serviceregistration_model.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
