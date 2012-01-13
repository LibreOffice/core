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
TARGET=				chmodelmain

PRJINC=				$(PRJ)$/source

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- export library -------------------------------------------------

#Specifies object files to bind into linked libraries.
SLOFILES=	\
    $(SLO)$/Axis.obj \
    $(SLO)$/GridProperties.obj \
    $(SLO)$/BaseCoordinateSystem.obj \
    $(SLO)$/CartesianCoordinateSystem.obj \
    $(SLO)$/ChartModel.obj		\
    $(SLO)$/ChartModel_Persistence.obj		\
    $(SLO)$/DataPoint.obj \
    $(SLO)$/DataPointProperties.obj \
    $(SLO)$/DataSeries.obj \
    $(SLO)$/Diagram.obj \
    $(SLO)$/DataSeriesProperties.obj \
    $(SLO)$/FormattedString.obj \
    $(SLO)$/Legend.obj \
    $(SLO)$/PageBackground.obj \
    $(SLO)$/PolarCoordinateSystem.obj \
    $(SLO)$/StockBar.obj \
    $(SLO)$/Title.obj \
    $(SLO)$/Wall.obj \
    $(SLO)$/UndoManager.obj \
    $(SLO)$/_serviceregistration_model.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
