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



PRJ = ..$/..$/..
PRJNAME = ucb
TARGET = ucpext.uno
ENABLE_EXCEPTIONS = TRUE

.INCLUDE : settings.mk
DLLPRE =

SLOFILES= \
        $(SLO)$/ucpext_provider.obj \
        $(SLO)$/ucpext_content.obj \
        $(SLO)$/ucpext_services.obj \
        $(SLO)$/ucpext_resultset.obj \
        $(SLO)$/ucpext_datasupplier.obj

SHL1STDLIBS = \
        $(UCBHELPERLIB)     \
        $(COMPHELPERLIB)    \
        $(CPPUHELPERLIB)    \
        $(CPPULIB)          \
        $(SALLIB)           \
        $(SALHELPERLIB)

SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1TARGET = $(TARGET)

SHL1DEPN =
SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def

DEF1NAME = $(SHL1TARGET)

.INCLUDE : target.mk

ALLTAR : $(MISC)/ucpext.component

$(MISC)/ucpext.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpext.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpext.component
