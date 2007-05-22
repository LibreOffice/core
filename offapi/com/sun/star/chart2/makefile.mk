#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: vg $ $Date: 2007-05-22 15:06:14 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
    AxisPosition.idl \
    Break.idl \
    CoordinateSystemTypeID.idl \
    CurveStyle.idl \
    DataPointGeometry3D.idl \
    DataPointLabel.idl \
    ErrorBarStyle.idl \
    ExplicitIncrementData.idl \
    ExplicitScaleData.idl \
    ExplicitSubIncrement.idl \
    FillBitmap.idl \
    IncrementData.idl \
    InterpretedData.idl \
    LegendExpansion.idl \
    LegendPosition.idl \
    LegendSymbolStyle.idl \
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
    ViewLegendEntry.idl \
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
    XDiagram.idl \
    XDiagramProvider.idl \
    XFastPropertyState.idl \
    XFormattedString.idl \
    XInternalDataProvider.idl \
    XLabeled.idl \
    XLegend.idl \
    XLegendEntry.idl \
    XLegendSymbolProvider.idl \
    XPlotter.idl \
    XRegressionCurve.idl \
    XRegressionCurveCalculator.idl \
    XRegressionCurveContainer.idl \
    XScaling.idl \
    XStyleSupplier.idl \
    XTarget.idl \
    XTitle.idl \
    XTitled.idl \
    XTransformation.idl

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
