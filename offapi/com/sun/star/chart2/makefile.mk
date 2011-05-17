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

PRJ=..$/..$/..$/..

TARGET=csschart2
PACKAGE=com$/sun$/star$/chart2

PRJNAME=offapi

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

#-------------------------------------------------------------------

IDLFILES= \
    AxisType.idl \
    AxisOrientation.idl \
    CoordinateSystemTypeID.idl \
    CurveStyle.idl \
    DataPointGeometry3D.idl \
    DataPointLabel.idl \
    FillBitmap.idl \
    IncrementData.idl \
    InterpretedData.idl \
    LegendPosition.idl \
    LightSource.idl \
    PieChartOffsetMode.idl \
    RelativePosition.idl \
    RelativeSize.idl \
    ScaleData.idl \
    StackingDirection.idl \
    SubIncrement.idl \
    Symbol.idl \
    SymbolStyle.idl \
    TickmarkStyle.idl \
    TransparencyStyle.idl \
    XAnyDescriptionAccess.idl\
    XAxis.idl \
    XCoordinateSystem.idl \
    XCoordinateSystemContainer.idl \
    XChartDocument.idl \
    XChartShape.idl \
    XChartShapeContainer.idl \
    XChartType.idl \
    XChartTypeContainer.idl \
    XChartTypeManager.idl \
    XChartTypeTemplate.idl \
    XColorScheme.idl \
    XDataInterpreter.idl \
    XDataSeries.idl \
    XDataSeriesContainer.idl \
    XDefaultSizeTransmitter.idl \
    XDiagram.idl \
    XDiagramProvider.idl \
    XFastPropertyState.idl \
    XFormattedString.idl \
    XInternalDataProvider.idl \
    XLabeled.idl \
    XLegend.idl \
    XRegressionCurve.idl \
    XRegressionCurveCalculator.idl \
    XRegressionCurveContainer.idl \
    XScaling.idl \
    XStyleSupplier.idl \
    XTarget.idl \
    XTitle.idl \
    XTitled.idl \
    XTransformation.idl \

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
