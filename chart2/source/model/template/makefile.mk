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
TARGET=				chmodeltemplate

PRJINC=				$(PRJ)$/source

ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# i26518 the gcc-3.0.4 requires to enhance the template-depth
# this seems to be a compiler issue, so we recommend not to use 3.0.x anymore
.IF "$(COM)"=="GCC" 
    CFLAGS+=-ftemplate-depth-128
.ENDIF

# --- export library -------------------------------------------------

#Specifies object files to bind into linked libraries.
SLOFILES=	\
    $(SLO)$/AreaChartType.obj \
    $(SLO)$/AreaChartTypeTemplate.obj \
    $(SLO)$/BarChartType.obj \
    $(SLO)$/BarChartTypeTemplate.obj \
    $(SLO)$/CandleStickChartType.obj \
    $(SLO)$/ChartType.obj		\
    $(SLO)$/ChartTypeManager.obj		\
    $(SLO)$/ChartTypeTemplate.obj		\
    $(SLO)$/ColumnChartType.obj \
    $(SLO)$/ColumnLineChartTypeTemplate.obj \
    $(SLO)$/DataInterpreter.obj \
    $(SLO)$/FilledNetChartType.obj \
    $(SLO)$/LineChartType.obj \
    $(SLO)$/LineChartTypeTemplate.obj \
    $(SLO)$/NetChartType.obj \
    $(SLO)$/NetChartTypeTemplate.obj \
    $(SLO)$/PieChartType.obj \
    $(SLO)$/PieChartTypeTemplate.obj \
    $(SLO)$/ScatterChartType.obj \
    $(SLO)$/ScatterChartTypeTemplate.obj \
    $(SLO)$/BubbleChartType.obj \
    $(SLO)$/BubbleChartTypeTemplate.obj \
    $(SLO)$/BubbleDataInterpreter.obj \
    $(SLO)$/StockChartTypeTemplate.obj \
    $(SLO)$/StockDataInterpreter.obj \
    $(SLO)$/XYDataInterpreter.obj \
    $(SLO)$/ColumnLineDataInterpreter.obj \
    $(SLO)$/_serviceregistration_charttypes.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
