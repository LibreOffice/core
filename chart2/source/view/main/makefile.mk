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
