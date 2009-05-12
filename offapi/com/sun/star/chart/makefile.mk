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
# $Revision: 1.13 $
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

PRJNAME=offapi

TARGET=csschart
PACKAGE=com$/sun$/star$/chart

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AccessibleChartElement.idl\
    AccessibleChartDocumentView.idl\
    AreaDiagram.idl\
    BarDiagram.idl\
    Chart3DBarProperties.idl\
    ChartArea.idl\
    ChartAxis.idl\
    ChartAxisArrangeOrderType.idl\
    ChartAxisAssign.idl\
    ChartAxisMarks.idl\
    ChartAxisPosition.idl\
    ChartAxisLabelPosition.idl\
    ChartAxisMarkPosition.idl\
    ChartAxisXSupplier.idl\
    ChartAxisYSupplier.idl\
    ChartAxisZSupplier.idl\
    ChartData.idl\
    ChartDataArray.idl\
    ChartDataCaption.idl\
    ChartDataChangeEvent.idl\
    ChartDataChangeType.idl\
    ChartDataPoint.idl\
    ChartDataPointProperties.idl\
    ChartDataRow.idl\
    ChartDataRowProperties.idl\
    ChartDataRowSource.idl\
    ChartDataValue.idl\
    ChartDocument.idl\
    ChartErrorCategory.idl\
    ChartErrorIndicatorType.idl\
    ChartGrid.idl\
    ChartLegend.idl\
    ChartLegendPosition.idl\
    ChartLine.idl\
    ChartPieSegmentProperties.idl\
    ChartRegressionCurveType.idl\
    ChartSeriesAddress.idl\
    ChartSolidType.idl\
    ChartStatistics.idl\
    ChartSymbolType.idl\
    ChartTableAddressSupplier.idl\
    ChartTitle.idl\
    ChartTwoAxisXSupplier.idl\
    ChartTwoAxisYSupplier.idl\
    DataLabelPlacement.idl\
    Diagram.idl\
    Dim3DDiagram.idl\
    DonutDiagram.idl\
    LineDiagram.idl\
    MissingValueTreatment.idl\
    NetDiagram.idl\
    PieDiagram.idl\
    StackableDiagram.idl\
    StockDiagram.idl\
    X3DDefaultSetter.idl\
    X3DDisplay.idl\
    XAxisXSupplier.idl\
    XAxisYSupplier.idl\
    XAxisZSupplier.idl\
    XChartData.idl\
    XChartDataArray.idl\
    XChartDataChangeEventListener.idl\
    XChartDocument.idl\
    XDiagram.idl\
    XStatisticDisplay.idl\
    XTwoAxisXSupplier.idl\
    XTwoAxisYSupplier.idl\
    XSecondAxisTitleSupplier.idl\
    XYDiagram.idl\
    ErrorBarStyle.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
