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



PRJ = ..
PRJNAME = uui
TARGET = uui
RESTARGET = $(TARGET)

GEN_HID = true
NO_BSYMBOLIC = true

.INCLUDE: settings.mk

SHL1TARGET = $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB = i$(TARGET)

SHL1VERSIONMAP = $(SOLARENV)/src/component.map
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

SHL1LIBS = \
    $(SLB)$/source.lib

SHL1STDLIBS = \
    $(SVTOOLLIB) \
    $(SVLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)

RESLIB1NAME = $(RESTARGET)
RESLIB1SRSFILES = \
    $(SRS)$/source.srs

.INCLUDE: target.mk

ALLTAR : $(MISC)/uui.component

$(MISC)/uui.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        uui.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt uui.component
