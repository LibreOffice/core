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
TARGET = test_deployment_passive

ENABLE_EXCEPTIONS = TRUE

PACKAGE = com/sun/star/comp/test/deployment/passive_java
JAVAFILES = Dispatch.java Provider.java Services.java
JARFILES = juh.jar ridl.jar unoil.jar

my_platform_components = passive_native
my_generic_components = passive_java passive_python

.INCLUDE: settings.mk

DLLPRE =

SLOFILES = $(SHL1OBJS)

SHL1TARGET = passive_native.uno
SHL1OBJS = $(SLO)/passive_native.obj
SHL1RPATH = OXT
SHL1STDLIBS = $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB)
SHL1VERSIONMAP = $(SOLARENV)/src/component.map
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

ALLTAR : $(MISC)/passive.oxt

$(MISC)/passive.oxt : manifest.xml description.xml Addons.xcu \
        ProtocolHandler.xcu $(MISC)/$(TARGET)/platform.components \
        $(MISC)/$(TARGET)/generic.components $(SHL1TARGETN) \
        $(MISC)/$(TARGET)/passive_java.jar passive_python.py
    $(RM) $@
    $(RM) -r $(MISC)/$(TARGET)/passive.oxt-zip
    $(MKDIR) $(MISC)/$(TARGET)/passive.oxt-zip
    $(MKDIRHIER) $(MISC)/$(TARGET)/passive.oxt-zip/META-INF
    $(SED) -e 's|@PLATFORM@|$(RTL_OS:l)_$(RTL_ARCH:l)|g' < manifest.xml \
        > $(MISC)/$(TARGET)/passive.oxt-zip/META-INF/manifest.xml
    $(COPY) description.xml Addons.xcu ProtocolHandler.xcu \
        $(MISC)/$(TARGET)/platform.components \
        $(MISC)/$(TARGET)/generic.components $(SHL1TARGETN) \
        $(MISC)/$(TARGET)/passive_java.jar passive_python.py \
        $(MISC)/$(TARGET)/passive.oxt-zip/
    cd $(MISC)/$(TARGET)/passive.oxt-zip && zip ../../passive.oxt \
        META-INF/manifest.xml description.xml Addons.xcu ProtocolHandler.xcu \
        platform.components generic.components $(SHL1TARGETN:f) \
        passive_java.jar passive_python.py

$(MISC)/$(TARGET)/platform.components : $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/$(TARGET)/platform.components.input \
        $(my_platform_components:^"$(MISC)/$(TARGET)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(PWD)/$(MISC)/$(TARGET)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/$(TARGET)/platform.components.input

$(MISC)/$(TARGET)/platform.components.input :
    $(MKDIRHIER) $(@:d)
    echo '<list>' \
        '$(my_platform_components:^"<filename>":+".component</filename>")' \
        '</list>' > $@

$(MISC)/$(TARGET)/generic.components : $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/$(TARGET)/generic.components.input \
        $(my_generic_components:^"$(MISC)/$(TARGET)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(PWD)/$(MISC)/$(TARGET)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/$(TARGET)/generic.components.input

$(MISC)/$(TARGET)/generic.components.input :
    $(MKDIRHIER) $(@:d)
    echo '<list>' \
        '$(my_generic_components:^"<filename>":+".component</filename>")' \
        '</list>' > $@

$(MISC)/$(TARGET)/passive_native.component : \
        $(SOLARENV)/bin/createcomponent.xslt passive_native.component
    $(MKDIRHIER) $(@:d)
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_EXTENSION)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt passive_native.component

$(MISC)/$(TARGET)/passive_java.component : \
        $(SOLARENV)/bin/createcomponent.xslt passive_java.component
    $(MKDIRHIER) $(@:d)
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_EXTENSION)passive_java.jar' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt passive_java.component

$(MISC)/$(TARGET)/passive_python.component : \
        $(SOLARENV)/bin/createcomponent.xslt passive_python.component
    $(MKDIRHIER) $(@:d)
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_EXTENSION)passive_python.py' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt passive_python.component

$(MISC)/$(TARGET)/passive_java.jar : MANIFEST.MF $(JAVATARGET)
    $(MKDIRHIER) $(@:d)
    $(RM) $@
    $(RM) -r $(MISC)/$(TARGET)/passive_java.jar-zip
    $(MKDIR) $(MISC)/$(TARGET)/passive_java.jar-zip
    $(MKDIRHIER) $(MISC)/$(TARGET)/passive_java.jar-zip/META-INF \
        $(MISC)/$(TARGET)/passive_java.jar-zip/$(PACKAGE)
    $(COPY) MANIFEST.MF $(MISC)/$(TARGET)/passive_java.jar-zip/META-INF/
    $(COPY) $(foreach,i,$(JAVAFILES:b) $(CLASSDIR)/$(PACKAGE)/$i.class) \
        $(MISC)/$(TARGET)/passive_java.jar-zip/$(PACKAGE)/
    cd $(MISC)/$(TARGET)/passive_java.jar-zip && zip ../passive_java.jar \
        META-INF/MANIFEST.MF $(foreach,i,$(JAVAFILES:b) $(PACKAGE)/$i.class)
