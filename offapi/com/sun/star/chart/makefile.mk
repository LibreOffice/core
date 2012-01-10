#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
    BubbleDiagram.idl\
    Chart3DBarProperties.idl\
    ChartArea.idl\
    ChartAxis.idl\
    ChartAxisArrangeOrderType.idl\
    ChartAxisAssign.idl\
    ChartAxisMarks.idl\
    ChartAxisPosition.idl\
    ChartAxisLabelPosition.idl\
    ChartAxisMarkPosition.idl\
    ChartAxisType.idl\
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
    ChartLegendExpansion.idl \
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
    TimeIncrement.idl \
    Diagram.idl\
    Dim3DDiagram.idl\
    DonutDiagram.idl\
    FilledNetDiagram.idl\
    LineDiagram.idl\
    MissingValueTreatment.idl\
    NetDiagram.idl\
    PieDiagram.idl\
    StackableDiagram.idl\
    StockDiagram.idl\
    TimeUnit.idl\
    TimeInterval.idl\
    X3DDefaultSetter.idl\
    X3DDisplay.idl\
    XAxis.idl\
    XAxisSupplier.idl\
    XAxisXSupplier.idl\
    XAxisYSupplier.idl\
    XAxisZSupplier.idl\
    XChartData.idl\
    XChartDataArray.idl\
    XChartDataChangeEventListener.idl\
    XChartDocument.idl\
    XComplexDescriptionAccess.idl\
    XDateCategories.idl\
    XDiagram.idl\
    XDiagramPositioning.idl\
    XStatisticDisplay.idl\
    XTwoAxisXSupplier.idl\
    XTwoAxisYSupplier.idl\
    XSecondAxisTitleSupplier.idl\
    XYDiagram.idl\
    ErrorBarStyle.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
