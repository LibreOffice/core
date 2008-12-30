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
# $Revision: 1.10 $
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
TARGET=				chcitemsetwrapper

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES=   	$(SLO)$/ItemConverter.obj \
                $(SLO)$/AxisItemConverter.obj \
                $(SLO)$/MultipleItemConverter.obj \
                $(SLO)$/MultipleChartConverters.obj \
                $(SLO)$/CharacterPropertyItemConverter.obj \
                $(SLO)$/SeriesOptionsItemConverter.obj \
                $(SLO)$/DataPointItemConverter.obj \
                $(SLO)$/GraphicPropertyItemConverter.obj \
                $(SLO)$/LegendItemConverter.obj \
                $(SLO)$/StatisticsItemConverter.obj \
                $(SLO)$/TitleItemConverter.obj \
                $(SLO)$/RegressionCurveItemConverter.obj \
                $(SLO)$/RegressionEquationItemConverter.obj \
                $(SLO)$/ErrorBarItemConverter.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk

