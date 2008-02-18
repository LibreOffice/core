#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: rt $ $Date: 2008-02-18 15:37:36 $
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
    NetDiagram.idl\
    PieDiagram.idl\
    StackableDiagram.idl\
    StockDiagram.idl\
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

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
