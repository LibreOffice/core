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
PRJINC=				$(PRJ)$/source
PRJNAME=			chart2
TARGET=				chvaxes

ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/chartview.pmk

#.IF "$(GUI)" == "WNT"
#CFLAGS+=-GR
#.ENDIF

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES = \
    $(SLO)$/VAxisOrGridBase.obj \
    $(SLO)$/VAxisBase.obj \
    $(SLO)$/DateHelper.obj \
    $(SLO)$/DateScaling.obj \
    $(SLO)$/Tickmarks.obj \
    $(SLO)$/Tickmarks_Equidistant.obj \
    $(SLO)$/Tickmarks_Dates.obj \
    $(SLO)$/MinimumAndMaximumSupplier.obj \
    $(SLO)$/ScaleAutomatism.obj \
    $(SLO)$/VAxisProperties.obj \
    $(SLO)$/VCartesianAxis.obj \
    $(SLO)$/VCartesianGrid.obj \
    $(SLO)$/VCartesianCoordinateSystem.obj \
    $(SLO)$/VPolarAxis.obj \
    $(SLO)$/VPolarAngleAxis.obj \
    $(SLO)$/VPolarRadiusAxis.obj \
    $(SLO)$/VPolarGrid.obj \
    $(SLO)$/VPolarCoordinateSystem.obj \
    $(SLO)$/VCoordinateSystem.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
