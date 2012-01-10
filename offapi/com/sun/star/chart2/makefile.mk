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
