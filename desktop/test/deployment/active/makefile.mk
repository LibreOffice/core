#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ = ../../..
PRJNAME = desktop
TARGET = test_deployment_active

ENABLE_EXCEPTIONS = TRUE

PACKAGE = com/sun/star/comp/test/deployment/active_java
JAVAFILES = Dispatch.java Provider.java Services.java
JARFILES = juh.jar ridl.jar unoil.jar

.INCLUDE: settings.mk

DLLPRE =

SLOFILES = $(SHL1OBJS)

SHL1TARGET = active_native.uno
SHL1OBJS = $(SLO)/active_native.obj
SHL1RPATH = OXT
SHL1STDLIBS = $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB)
SHL1VERSIONMAP = $(SOLARENV)/src/reg-component.map
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

.IF "$(ENABLE_JAVA)" != ""

ALLTAR : $(MISC)/active.oxt

$(MISC)/active.oxt : manifest.xml description.xml Addons.xcu \
        ProtocolHandler.xcu $(SHL1TARGETN) $(MISC)/$(TARGET)/active_java.jar \
        active_python.py
    $(RM) $@
    $(RM) -r $(MISC)/$(TARGET)/active.oxt-zip
    $(MKDIR) $(MISC)/$(TARGET)/active.oxt-zip
    $(MKDIRHIER) $(MISC)/$(TARGET)/active.oxt-zip/META-INF
    $(SED) -e 's|@PATH@|$(SHL1TARGETN:f)|g' \
        -e 's|@PLATFORM@|$(RTL_OS:l)_$(RTL_ARCH:l)|g' < manifest.xml \
        > $(MISC)/$(TARGET)/active.oxt-zip/META-INF/manifest.xml
    $(COPY) description.xml Addons.xcu ProtocolHandler.xcu $(SHL1TARGETN) \
        $(MISC)/$(TARGET)/active_java.jar active_python.py \
        $(MISC)/$(TARGET)/active.oxt-zip/
    cd $(MISC)/$(TARGET)/active.oxt-zip && zip ../../active.oxt \
        META-INF/manifest.xml description.xml Addons.xcu ProtocolHandler.xcu \
        $(SHL1TARGETN:f) active_java.jar active_python.py

$(MISC)/$(TARGET)/active_java.jar : MANIFEST.MF $(JAVATARGET)
    $(MKDIRHIER) $(@:d)
    $(RM) $@
    $(RM) -r $(MISC)/$(TARGET)/active_java.jar-zip
    $(MKDIR) $(MISC)/$(TARGET)/active_java.jar-zip
    $(MKDIRHIER) $(MISC)/$(TARGET)/active_java.jar-zip/META-INF \
        $(MISC)/$(TARGET)/active_java.jar-zip/$(PACKAGE)
    $(COPY) MANIFEST.MF $(MISC)/$(TARGET)/active_java.jar-zip/META-INF/
    $(COPY) $(foreach,i,$(JAVAFILES:b) $(CLASSDIR)/$(PACKAGE)/$i.class) \
        $(MISC)/$(TARGET)/active_java.jar-zip/$(PACKAGE)/
    cd $(MISC)/$(TARGET)/active_java.jar-zip && zip ../active_java.jar \
        META-INF/MANIFEST.MF $(foreach,i,$(JAVAFILES:b) $(PACKAGE)/$i.class)

.ENDIF
