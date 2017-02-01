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

ifneq "$(OO_SDK_OUT)" ""
OUT=$(subst \,/,$(OO_SDK_OUT))/$(OS)example.out
else
OUT=$(PRJ)/$(OS)example.out
endif

UNOPKG_EXT=uno.pkg
UNOOXT_EXT=oxt

OUT_BIN=$(OUT)/bin
OUT_INC=$(OUT)/inc
OUT_SLO=$(OUT)/slo
OUT_LIB=$(OUT)/lib
OUT_SLB=$(OUT)/slb
OUT_MISC=$(OUT)/misc
OUT_OBJ=$(OUT)/obj
OUT_CLASS=$(OUT)/class
IDL_DIR=$(PRJ)/idl
BIN_DIR=$(PRJ)/bin
CLASSES_DIR=$(PRJ)/classes
URE_CLASSES_DIR=$(subst \,/,$(OO_SDK_URE_JAVA_DIR))
ifeq "$(PLATFORM)" "macosx"
OFFICE_CLASSES_DIR=$(subst \,/,$(OFFICE_PROGRAM_PATH))/../Resources/java
else
OFFICE_CLASSES_DIR=$(subst \,/,$(OFFICE_PROGRAM_PATH))/classes
endif
COMP_PACKAGE_DIR=$(subst /,$(PS),$(OUT_BIN))

SDKTYPEFLAG=$(OUT_MISC)/oosdk_cpp_types.flag

URE_TYPES="$(subst \,/,$(URE_MISC)$(PS)types.rdb)"
URE_SERVICES="$(subst \\,\,$(URE_MISC)$(PS)services.rdb)"

ifeq "$(PLATFORM)" "macosx"
OFFICE_TYPES="$(subst \,/,$(OFFICE_PROGRAM_PATH)$(PS)..$(PS)Resources$(PS)types$(PS)offapi.rdb)"
OFFICE_SERVICES="$(subst \\,\,$(OFFICE_PROGRAM_PATH)$(PS)..$(PS)Resources$(PS)services$(PS)services.rdb)"
else
OFFICE_TYPES="$(subst \,/,$(OFFICE_PROGRAM_PATH)$(PS)types$(PS)offapi.rdb)"
OFFICE_SERVICES="$(subst \\,\,$(OFFICE_PROGRAM_PATH)$(PS)services$(PS)services.rdb)"
endif

OFFICE_TYPE_LIBRARY="$(OFFICE_TYPES)"

JAVA_OPTIONS=
ifneq "$(OO_SDK_JAVA_HOME)" ""
JAVA_BITS := $(shell $(OO_SDK_JAVA_HOME)/bin/java -version 2>&1 | tail -1 | cut -d " " -f3)
ifeq "$(JAVA_BITS)" "64-Bit"
ifneq "$(PROCTYPE)" "x86_64"
JAVA_OPTIONS=-d32
endif
endif
endif

DEPLOYTOOL="$(OFFICE_PROGRAM_PATH)$(PS)unopkg" add -f
SDK_JAVA="$(OO_SDK_JAVA_HOME)/bin/java" $(JAVA_OPTIONS)
SDK_JAVAC="$(OO_SDK_JAVA_HOME)/bin/javac"
SDK_JAR="$(OO_SDK_JAVA_HOME)/bin/jar"
ifneq "$(OO_SDK_ZIP_HOME)" ""
SDK_ZIP="$(OO_SDK_ZIP_HOME)/zip"
else
SDK_ZIP=zip
endif
ifneq "$(OO_SDK_CAT_HOME)" ""
SDK_CAT="$(OO_SDK_CAT_HOME)/cat"
else
SDK_CAT=cat
endif
ifneq "$(OO_SDK_SED_HOME)" ""
SDK_SED="$(OO_SDK_SED_HOME)/sed"
else
SDK_SED=sed
endif
IDLC="$(OO_SDK_HOME)/bin/idlc"
CPPUMAKER="$(OO_SDK_HOME)/bin/cppumaker"
JAVAMAKER="$(OO_SDK_HOME)/bin/javamaker"
REGMERGE="$(OO_SDK_URE_BIN_DIR)/regmerge"

SDK_JAVA_UNO_BOOTSTRAP_FILES=\
    -C $(CLASSES_DIR) $(SQM)com/sun/star/lib/loader/$(SQM) \
    -C $(CLASSES_DIR) $(SQM)win/unowinreg.dll$(SQM)
