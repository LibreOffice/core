#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#***********************************************************************/

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
