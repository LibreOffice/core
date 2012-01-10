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

PRJNAME=oox
TARGET=chart
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =								\
    $(SLO)$/axiscontext.obj				\
    $(SLO)$/axisconverter.obj			\
    $(SLO)$/axismodel.obj				\
    $(SLO)$/chartcontextbase.obj		\
    $(SLO)$/chartconverter.obj			\
    $(SLO)$/chartdrawingfragment.obj	\
    $(SLO)$/chartspaceconverter.obj		\
    $(SLO)$/chartspacefragment.obj		\
    $(SLO)$/chartspacemodel.obj			\
    $(SLO)$/converterbase.obj			\
    $(SLO)$/datasourcecontext.obj		\
    $(SLO)$/datasourceconverter.obj		\
    $(SLO)$/datasourcemodel.obj			\
    $(SLO)$/modelbase.obj				\
    $(SLO)$/objectformatter.obj			\
    $(SLO)$/plotareacontext.obj			\
    $(SLO)$/plotareaconverter.obj		\
    $(SLO)$/plotareamodel.obj			\
    $(SLO)$/seriescontext.obj			\
    $(SLO)$/seriesconverter.obj			\
    $(SLO)$/seriesmodel.obj				\
    $(SLO)$/titlecontext.obj			\
    $(SLO)$/titleconverter.obj			\
    $(SLO)$/titlemodel.obj				\
    $(SLO)$/typegroupcontext.obj		\
    $(SLO)$/typegroupconverter.obj		\
    $(SLO)$/typegroupmodel.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk
