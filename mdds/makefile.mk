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

PRJ=.

PRJNAME=mdds
TARGET=mdds

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=mdds_0.6.0
TARFILE_MD5=3e89a35f253a4f1c7de68c57d851ef38
PATCH_FILES=\
	    mdds_0.6.0.patch \
	    0001-help-compiler-select-the-right-overload-of-vector-in.patch \
            mdds_0.6.0-unused-parameter.patch \
            mdds_0.6.0-unreachable-code.patch \
	    0001-fix-linking-error-with-boost-1.50.patch \


CONFIGURE_DIR=
CONFIGURE_ACTION=

BUILD_DIR=
BUILD_ACTION=
BUILD_FLAGS=

# --- Targets ------------------------------------------------------

.INCLUDE :	set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

# --- post-build ---------------------------------------------------

NORMALIZE_FLAG_FILE=so_normalized_$(TARGET)

$(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE) : $(PACKAGE_DIR)$/$(BUILD_FLAG_FILE)
    -@$(MKDIRHIER) $(INCCOM)
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/include$/mdds $(INCCOM)
    @$(TOUCH) $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)

$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) : $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)

