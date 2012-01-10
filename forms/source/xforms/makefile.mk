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



PRJ=..$/..

PRJNAME=forms
TARGET=xforms

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
        $(SLO)$/model.obj \
        $(SLO)$/model_ui.obj \
        $(SLO)$/binding.obj \
        $(SLO)$/xforms_services.obj \
        $(SLO)$/unohelper.obj \
        $(SLO)$/propertysetbase.obj \
        $(SLO)$/pathexpression.obj \
        $(SLO)$/computedexpression.obj \
        $(SLO)$/boolexpression.obj \
        $(SLO)$/mip.obj \
        $(SLO)$/submission.obj \
        $(SLO)$/datatyperepository.obj \
        $(SLO)$/datatypes.obj \
        $(SLO)$/convert.obj \
        $(SLO)$/enumeration.obj \
        $(SLO)$/resourcehelper.obj \
        $(SLO)$/xmlhelper.obj \
        $(SLO)$/xformsevent.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

