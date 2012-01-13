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



PRJ = ../../..
PRJNAME = desktop
TARGET = test_deployment_boxt

ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk
.INCLUDE: rtlbootstrap.mk
.INCLUDE: versionlist.mk

my_version = $(OOOBASEVERSIONMAJOR).$(OOOBASEVERSIONMINOR)

DLLPRE =

SLOFILES = $(SHL1OBJS)

SHL1TARGET = boxt.uno
SHL1OBJS = $(SLO)/boxt.obj
SHL1RPATH = BOXT
SHL1STDLIBS = \
    $(CPPUHELPERLIB) $(CPPULIB) $(MSFILTERLIB) $(SALLIB) $(TOOLSLIB) $(VCLLIB)
SHL1VERSIONMAP = $(SOLARENV)/src/reg-component.map
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

ALLTAR : $(MISC)/boxt.oxt

$(MISC)/boxt.oxt .ERRREMOVE : manifest.xml description.xml Addons.xcu \
        ProtocolHandler.xcu $(SHL1TARGETN)
    $(RM) -r $@ $(MISC)/$(TARGET).zip
    $(MKDIR) $(MISC)/$(TARGET).zip
    $(MKDIR) $(MISC)/$(TARGET).zip/META-INF
    $(SED) -e 's|@PATH@|$(SHL1TARGETN:f)|g' < manifest.xml \
        > $(MISC)/$(TARGET).zip/META-INF/manifest.xml
    $(SED) -e 's|@PLATFORM@|$(RTL_OS:l)_$(RTL_ARCH:l)|g' \
        -e 's|@VERSION@|$(my_version)|g' < description.xml \
        > $(MISC)/$(TARGET).zip/description.xml
    $(COPY) Addons.xcu ProtocolHandler.xcu $(SHL1TARGETN) $(MISC)/$(TARGET).zip
    cd $(MISC)/$(TARGET).zip && zip ../boxt.oxt META-INF/manifest.xml \
        description.xml Addons.xcu ProtocolHandler.xcu $(SHL1TARGETN:f)
