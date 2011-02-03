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

PRJ=				..$/..$/..
PRJINC=				$(PRJ)$/source
PRJNAME=			chart2
TARGET=				chvaxes

ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/chartview.pmk

#.IF "$(GUI)" == "WNT"
#CFLAGS+=-GR
#.ENDIF

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES = \
    $(SLO)$/VAxisOrGridBase.obj \
    $(SLO)$/VAxisBase.obj \
    $(SLO)$/DateHelper.obj \
    $(SLO)$/DateScaling.obj \
    $(SLO)$/Tickmarks.obj \
    $(SLO)$/Tickmarks_Equidistant.obj \
    $(SLO)$/Tickmarks_Dates.obj \
    $(SLO)$/MinimumAndMaximumSupplier.obj \
    $(SLO)$/ScaleAutomatism.obj \
    $(SLO)$/VAxisProperties.obj \
    $(SLO)$/VCartesianAxis.obj \
    $(SLO)$/VCartesianGrid.obj \
    $(SLO)$/VCartesianCoordinateSystem.obj \
    $(SLO)$/VPolarAxis.obj \
    $(SLO)$/VPolarAngleAxis.obj \
    $(SLO)$/VPolarRadiusAxis.obj \
    $(SLO)$/VPolarGrid.obj \
    $(SLO)$/VPolarCoordinateSystem.obj \
    $(SLO)$/VCoordinateSystem.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
