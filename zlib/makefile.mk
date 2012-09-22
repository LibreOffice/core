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

PRJNAME=zlib
TARGET=zlib

.IF "$(GUI)" == "UNX"
.IF "$(SYSTEM_ZLIB)" == "YES"
all:
    @echo "An already available installation of zlib should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=zlib-1.2.7
TARFILE_MD5=2ab442d169156f34c379c968f3f482dd

PATCH_FILES=zlib-1.2.7.patch
ADDITIONAL_FILES=makefile.mk

#relative to CONFIGURE_DIR

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

OUT2INC= \
    zlib.h \
    zconf.h \
    contrib$/minizip$/unzip.h \
    contrib$/minizip$/ioapi.h

PATCHED_HEADERS=$(INCCOM)$/patched$/zlib.h

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

ALLTAR: $(PATCHED_HEADERS)

$(PATCHED_HEADERS) : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    @$(PERL) make_patched_header.pl $@ $(PRJNAME)
