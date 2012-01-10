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



PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=reportdesign
TARGET=api

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/dll.pmk

# --- Files -------------------------------------
SLOFILES=	\
        $(SLO)$/FormattedField.obj		\
        $(SLO)$/Groups.obj				\
        $(SLO)$/Group.obj				\
        $(SLO)$/Section.obj				\
        $(SLO)$/services.obj			\
        $(SLO)$/FixedText.obj			\
        $(SLO)$/ImageControl.obj		\
        $(SLO)$/ReportDefinition.obj	\
        $(SLO)$/ReportComponent.obj		\
        $(SLO)$/ReportEngineJFree.obj	\
        $(SLO)$/Tools.obj				\
        $(SLO)$/Function.obj			\
        $(SLO)$/Functions.obj			\
        $(SLO)$/ReportControlModel.obj	\
        $(SLO)$/FormatCondition.obj		\
        $(SLO)$/ReportVisitor.obj		\
        $(SLO)$/Shape.obj				\
        $(SLO)$/FixedLine.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

