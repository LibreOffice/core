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



PRJ=.

PRJNAME=mdds
TARGET=mdds

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=mdds_0.3.0
TARFILE_MD5=cf8a6967f7de535ae257fa411c98eb88
PATCH_FILES=

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
    @$(GNUCOPY) -r $(PACKAGE_DIR)$/$(TARFILE_NAME)$/inc$/mdds $(INCCOM)
    @$(TOUCH) $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)

$(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE) : $(PACKAGE_DIR)$/$(NORMALIZE_FLAG_FILE)

