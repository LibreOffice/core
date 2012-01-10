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
PRJNAME=extensions
TARGET=oooimprovecore

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

EXCEPTIONSFILES= \
        $(SLO)$/core.obj \
        $(SLO)$/oooimprovecore_services.obj \

SLOFILES= \
        $(EXCEPTIONSFILES) \
        $(SLO)$/oooimprovecore_module.obj \

LIB1TARGET=$(SLB)$/$(TARGET)_t.lib
LIB1FILES= \
        $(SLB)$/$(TARGET).lib \

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
.IF "$(GUI)"=="OS2"
SHL1TARGET=oooimpc$(DLLPOSTFIX)
.ENDIF

SHL1STDLIBS= \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB) \
        $(VCLLIB) \
        $(SVLLIB) \
        $(SVTOOLLIB) \
        $(SFXLIB) \
        $(SVXCORELIB) \
        $(SVXLIB) \
        $(TOOLSLIB)


SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Targets ----------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/oooimprovecore.component

$(MISC)/oooimprovecore.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt oooimprovecore.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt oooimprovecore.component
