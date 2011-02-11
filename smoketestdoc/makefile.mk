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

PRJ = ..
PRJNAME = smoketestoo_native
TARGET = data

.INCLUDE: settings.mk

$(BIN)/smoketestdoc.sxw: $(MISC)/zip/smoketestdoc.sxw
    $(COPY) $< $@

$(MISC)/zip/smoketestdoc.sxw: \
        Events.xml \
        Global.xml \
        OptionsDlg.xml \
        Test_10er.xml \
        Test_DB.xml \
        Test_Ext.xml \
        content.xml \
        dialog-lb.xml \
        dialog-lc.xml \
        manifest.xml \
        meta.xml \
        script-lb.xml \
        script-lc.xml \
        settings.xml \
        styles.xml
    $(RM) -r $(MISC)/zip
    $(MKDIR) $(MISC)/zip
    $(COPY) content.xml meta.xml settings.xml styles.xml $(MISC)/zip/
    printf application/vnd.sun.xml.writer > $(MISC)/zip/mimetype
    $(MKDIR) $(MISC)/zip/META-INF
    $(COPY) manifest.xml $(MISC)/zip/META-INF/
    $(MKDIR) $(MISC)/zip/Basic
    $(COPY) script-lc.xml $(MISC)/zip/Basic/
    $(MKDIR) $(MISC)/zip/Basic/Standard
    $(COPY) script-lb.xml Events.xml Global.xml Test_10er.xml Test_DB.xml \
        Test_Ext.xml $(MISC)/zip/Basic/Standard/
    $(MKDIR) $(MISC)/zip/Dialogs
    $(COPY) dialog-lc.xml $(MISC)/zip/Dialogs/
    $(MKDIR) $(MISC)/zip/Dialogs/Standard
    $(COPY) dialog-lb.xml OptionsDlg.xml $(MISC)/zip/Dialogs/Standard/
    cd $(MISC)/zip && zip -rD $(@:f) .

.INCLUDE: target.mk
