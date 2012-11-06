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

OUT_COMP_CLASS = $(OUT_CLASS)/ComponentThumbsExample
OUT_COMP_GEN = $(OUT_MISC)/ComponentThumbsExample

COMP_NAME=ImageShrink
COMP_RDB_NAME = $(COMP_NAME).uno.rdb
COMP_RDB = $(OUT_COMP_GEN)/$(COMP_RDB_NAME)
COMP_PACKAGE = $(OUT_BIN)/$(COMP_NAME).$(UNOOXT_EXT)
COMP_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOOXT_EXT)")
COMP_JAR_NAME = $(COMP_NAME).uno.jar
COMP_JAR = $(OUT_COMP_CLASS)/$(COMP_JAR_NAME)
COMP_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMP_NAME).uno.Manifest
COMP_UNOPKG_MANIFEST = $(OUT_COMP_CLASS)/$(COMP_NAME)/META-INF/manifest.xml
REGISTERFLAG = $(OUT_MISC)$(PS)devguide_$(COMP_NAME)_register_component.flag
COMP_COMPONENTS=$(COMP_NAME).components

IDL_PACKAGE=org/openoffice/test

IDLFILES = XImageShrink.idl \
    XImageShrinkFilter.idl \
    ImageShrink.idl

GENCLASSFILES = $(patsubst %.idl,$(OUT_COMP_CLASS)/$(IDL_PACKAGE)/%.class,$(IDLFILES))
GENCLASSNAMES = $(patsubst %.idl,$(IDL_PACKAGE)/%.class,$(IDLFILES))
GENTYPELIST = $(subst /,.,$(patsubst %.idl,-T$(IDL_PACKAGE)/% ,$(IDLFILES)))
GENURDFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES))

APP1_NAME=Thumbs
APP1_JAR=$(OUT_COMP_CLASS)/$(APP1_NAME).jar
