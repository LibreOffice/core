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
TARGET=log

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

# ... object files ............................
EXCEPTIONSFILES= \
        $(SLO)$/consolehandler.obj      \
        $(SLO)$/csvformatter.obj        \
        $(SLO)$/filehandler.obj         \
        $(SLO)$/log_services.obj        \
        $(SLO)$/logger.obj              \
        $(SLO)$/loggerconfig.obj        \
        $(SLO)$/loghandler.obj          \
        $(SLO)$/plaintextformatter.obj  \

SLOFILES= \
        $(EXCEPTIONSFILES) \
        $(SLO)$/log_module.obj          \
        $(SLO)$/logrecord.obj           \

# --- library -----------------------------------

LIB1TARGET=$(SLB)$/$(TARGET)_t.lib
LIB1FILES=\
        $(SLB)$/$(TARGET).lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)

SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Targets ----------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/log.component

$(MISC)/log.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        log.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt log.component
