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
PRJINC=..
PRJNAME=forms
TARGET=common

ENABLE_EXCEPTIONS=TRUE
#TARGETTYPE=GUI

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE: $(PRJ)$/makefile.pmk

# --- Types -------------------------------------

INCPRE+=$(SOLARINCDIR)$/offuh
INCPRE*=$(INCCOM)$/$(TARGET)

# --- Files -------------------------------------

SLOFILES=	$(SLO)$/limitedformats.obj \
            $(SLO)$/property.obj \
            $(SLO)$/services.obj \
            $(SLO)$/InterfaceContainer.obj \
            $(SLO)$/ids.obj	\
            $(SLO)$/frm_module.obj  \
            $(SLO)$/frm_strings.obj \
            $(SLO)$/listenercontainers.obj \
            $(SLO)$/componenttools.obj \

# --- Targets ----------------------------------

.INCLUDE : target.mk

ALLTAR : \
        $(MISC)$/$(TARGET).don \

$(SLOFILES) : $(MISC)$/$(TARGET).don

$(MISC)$/$(TARGET).don : $(SOLARBINDIR)$/oovbaapi.rdb
        +$(CPPUMAKER) -O$(INCCOM)$/$(TARGET) -BUCR $(SOLARBINDIR)$/oovbaapi.rdb -X$(SOLARBINDIR)$/types.rdb && echo > $@
        echo $@

