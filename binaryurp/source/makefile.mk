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
PRJNAME = binaryurp
TARGET = binaryurp

ENABLE_EXCEPTIONS = TRUE
VISIBILITY_HIDDEN = TRUE

.INCLUDE: settings.mk

DLLPRE =

SLOFILES = \
    $(SLO)/binaryany.obj \
    $(SLO)/bridge.obj \
    $(SLO)/bridgefactory.obj \
    $(SLO)/currentcontext.obj \
    $(SLO)/incomingrequest.obj \
    $(SLO)/lessoperators.obj \
    $(SLO)/marshal.obj \
    $(SLO)/outgoingrequests.obj \
    $(SLO)/proxy.obj \
    $(SLO)/reader.obj \
    $(SLO)/unmarshal.obj \
    $(SLO)/writer.obj

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1RPATH = URELIB
SHL1STDLIBS = $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB)
SHL1TARGET = binaryurp.uno
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

ALLTAR : $(MISC)/binaryurp.component

$(MISC)/binaryurp.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        binaryurp.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_URE_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt binaryurp.component
