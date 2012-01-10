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
PRJNAME=ucb
TARGET=ucb
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

# Version
UCB_MAJOR=1

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""

SLOFILES=\
    $(SLO)$/identify.obj \
    $(SLO)$/ucb.obj \
    $(SLO)$/ucbserv.obj \
    $(SLO)$/ucbstore.obj \
    $(SLO)$/ucbprops.obj \
    $(SLO)$/provprox.obj \
    $(SLO)$/ucbcmds.obj \
    $(SLO)$/cmdenv.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

SHL1TARGET=$(TARGET)$(UCB_MAJOR)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(UCBHELPERLIB)
SHL1LIBS=\
    $(LIB1TARGET) \
    $(SLB)$/regexp.lib
SHL1IMPLIB=i$(TARGET)

.IF "$(GUI)" == "OS2"
DEF1EXPORTFILE=exports2.dxp
.ELSE
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
.ENDIF

DEF1NAME=$(SHL1TARGET)

.ENDIF # L10N_framework

.INCLUDE: target.mk


ALLTAR : $(MISC)/ucb1.component

$(MISC)/ucb1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucb1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucb1.component
