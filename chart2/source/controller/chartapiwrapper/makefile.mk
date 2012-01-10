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



PRJ=				..$/..$/..
PRJNAME=			chart2
TARGET=				chchartapiwrapper

PRJINC=				$(PRJ)$/source

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- export library -------------------------------------------------

#Specifies object files to bind into linked libraries.
SLOFILES=	\
    $(SLO)$/Chart2ModelContact.obj \
    $(SLO)$/AreaWrapper.obj \
    $(SLO)$/AxisWrapper.obj \
    $(SLO)$/ChartDataWrapper.obj \
    $(SLO)$/ChartDocumentWrapper.obj \
    $(SLO)$/DataSeriesPointWrapper.obj \
    $(SLO)$/DiagramWrapper.obj \
    $(SLO)$/GridWrapper.obj \
    $(SLO)$/LegendWrapper.obj \
    $(SLO)$/TitleWrapper.obj \
    $(SLO)$/MinMaxLineWrapper.obj \
    $(SLO)$/UpDownBarWrapper.obj \
    $(SLO)$/WallFloorWrapper.obj \
    $(SLO)$/WrappedAutomaticPositionProperties.obj \
    $(SLO)$/WrappedCharacterHeightProperty.obj \
    $(SLO)$/WrappedDataCaptionProperties.obj \
    $(SLO)$/WrappedTextRotationProperty.obj \
    $(SLO)$/WrappedGapwidthProperty.obj \
    $(SLO)$/WrappedScaleProperty.obj \
    $(SLO)$/WrappedSplineProperties.obj \
    $(SLO)$/WrappedStockProperties.obj \
    $(SLO)$/WrappedSymbolProperties.obj \
    $(SLO)$/WrappedAxisAndGridExistenceProperties.obj \
    $(SLO)$/WrappedNumberFormatProperty.obj \
    $(SLO)$/WrappedStatisticProperties.obj \
    $(SLO)$/WrappedSceneProperty.obj \
    $(SLO)$/WrappedSeriesAreaOrLineProperty.obj \
    $(SLO)$/WrappedAddInProperty.obj \
    $(SLO)$/WrappedScaleTextProperties.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
