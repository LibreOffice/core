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
#*************************************************************************

PRJNAME	= accessibility
PRJ		= ..$/..$/..$/..
TARGET	= java_uno_accessbridge
PACKAGE	= org$/openoffice$/accessibility

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

JARFILES = jurt.jar unoil.jar ridl.jar 
JAVAFILES = \
    AccessBridge.java \
    KeyHandler.java \
    PopupWindow.java \
    WindowsAccessBridgeAdapter.java

JAVACLASSFILES= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

JARTARGET               = $(TARGET).jar
JARCOMPRESS             = TRUE
JARCLASSDIRS            = $(PACKAGE) org/openoffice/java/accessibility 
CUSTOMMANIFESTFILE      = manifest

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

ALLTAR : $(MISC)/java_uno_accessbridge.component

$(MISC)/java_uno_accessbridge.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt java_uno_accessbridge.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)$(JARTARGET)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt java_uno_accessbridge.component
