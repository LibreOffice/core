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
