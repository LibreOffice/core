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
PRJNAME=ucb
TARGET=ucphier
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# Version
UCPHIER_MAJOR=1

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk

# --- General -----------------------------------------------------
.IF "$(L10N_framework)"==""

SLOFILES=\
    $(SLO)$/hierarchyservices.obj		\
    $(SLO)$/hierarchydata.obj           \
    $(SLO)$/hierarchyprovider.obj		\
    $(SLO)$/hierarchycontent.obj		\
    $(SLO)$/hierarchycontentcaps.obj	\
    $(SLO)$/hierarchydatasupplier.obj	\
    $(SLO)$/dynamicresultset.obj        \
    $(SLO)$/hierarchydatasource.obj     \
    $(SLO)$/hierarchyuri.obj            

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCPHIER_MAJOR)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)  \
    $(UCBHELPERLIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------------------------------

.ENDIF # L10N_framework
.INCLUDE: target.mk


ALLTAR : $(MISC)/ucphier1.component

$(MISC)/ucphier1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucphier1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucphier1.component
