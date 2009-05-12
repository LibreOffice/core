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
TARGET=				chview

ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/chartview.pmk

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES =  \
            $(SLO)$/ChartItemPool.obj \
            $(SLO)$/DrawModelWrapper.obj \
            $(SLO)$/NumberFormatterWrapper.obj \
            $(SLO)$/PropertyMapper.obj \
            $(SLO)$/Stripe.obj \
            $(SLO)$/VLineProperties.obj \
            $(SLO)$/ShapeFactory.obj \
            $(SLO)$/VLegendSymbolFactory.obj \
            $(SLO)$/DataPointSymbolSupplier.obj \
            $(SLO)$/Linear3DTransformation.obj \
            $(SLO)$/VPolarTransformation.obj \
            $(SLO)$/Clipping.obj \
            $(SLO)$/PlottingPositionHelper.obj \
            $(SLO)$/LabelPositionHelper.obj \
            $(SLO)$/PolarLabelPositionHelper.obj \
            $(SLO)$/PlotterBase.obj \
            $(SLO)$/VDataSeries.obj \
            $(SLO)$/VLegend.obj \
            $(SLO)$/VTitle.obj \
            $(SLO)$/ChartView.obj \
            $(SLO)$/_serviceregistration_view.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
