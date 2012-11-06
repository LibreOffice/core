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

PRJ=..
PRJNAME=odk
TARGET=odk

# ------------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: makefile.pmk
# ------------------------------------------------------------------
.IF "$(L10N_framework)"==""

ZIP1TARGET=odkexamples
ZIP1FLAGS=-u -r
ZIP1DIR=$(PRJ)
ZIP1LIST=examples -x "*.svn*" -x "*CVS*" -x "*makefile.mk" -x "*Inspector*" -x "*Container1*" -x "*Storage*" -x "*register_component*" -x "*examples.html" -x "*ConverterServlet*" -x "*NotesAccess*" -x "*delphi*"

.ENDIF

.INCLUDE :  target.mk
.IF "$(L10N_framework)"==""
ALLTAR:\
    $(BIN)$/$(PRODUCTZIPFILE)
#    $(BIN)$/odk_oo.zip
.ENDIF

$(BIN)$/$(PRODUCTZIPFILE) : $(SDK_CONTENT_CHECK_FILES) $(SDK_CHECK_FLAGS)
    cd $(BIN)$/$(PRODUCT_NAME) && zip -urq ..$/$(PRODUCTZIPFILE) . $(CHECKZIPRESULT)
#	cd $(BIN)$/$(PRODUCT_NAME) && zip -urq ..$/$(PRODUCTZIPFILE) . -x "idl/*" $(CHECKZIPRESULT)
#	cd $(BIN)$/$(PRODUCT_NAME) && zip -urq ..$/odkidl.zip idl/* $(CHECKZIPRESULT)

ODK_OO_FILES=\
    $(PRJ)$/index.html \
    $(PRJ)$/docs$/tools.html \
    $(PRJ)$/docs$/notsupported.html \
    $(PRJ)$/docs$/install.html \
    $(PRJ)$/docs$/sdk_styles.css \
    $(PRJ)$/docs$/images$/arrow-1.gif \
    $(PRJ)$/docs$/images$/arrow-3.gif \
    $(PRJ)$/docs$/images$/bg_table.gif \
    $(PRJ)$/docs$/images$/bg_table2.gif \
    $(PRJ)$/docs$/images$/bg_table3.gif \
    $(PRJ)$/docs$/images$/nav_down.png \
    $(PRJ)$/docs$/images$/nav_home.png \
    $(PRJ)$/docs$/images$/nav_left.png \
    $(PRJ)$/docs$/images$/nav_right.png \
    $(PRJ)$/docs$/images$/nav_up.png \
    $(PRJ)$/docs$/images$/sdk_head-1.gif \
    $(PRJ)$/docs$/images$/sdk_head-2.gif \
    $(PRJ)$/docs$/images$/sdk_head-3.gif \
    $(PRJ)$/docs$/images$/sdk_line-1.gif \
    $(PRJ)$/docs$/images$/sdk_line-2.gif \
    $(PRJ)$/examples$/examples.html \
    $(PRJ)$/examples$/DevelopersGuide$/examples.html


$(BIN)$/odk_oo.zip : $(ODK_OO_FILES)
    cd $(PRJ) && zip -urq $(subst,$(PRJ)$/, $(BIN)$/odk_oo.zip) $(subst,$(PRJ)$/, $<) $(CHECKZIPRESULT)

