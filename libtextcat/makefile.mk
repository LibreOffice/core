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

PRJNAME=libtextcat
TARGET=libtextcat

.IF "$(SYSTEM_LIBTEXTCAT)" == "YES"
all:
        @echo "An already available installation of libtextcat should exist on your system."
        @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=libtextcat-2.2
TARFILE_MD5=128cfc86ed5953e57fe0f5ae98b62c2e
TARFILE_ROOTDIR=libtextcat-2.2

PATCH_FILES=libtextcat-2.2.patch


ADDITIONAL_FILES= \
                src$/utf8misc.h \
                src$/utf8misc.c \
                src$/win32_config.h \
                src$/makefile.mk \
                src$/libtextcat.map

.IF "$(GUI)"=="UNX"
#CONFIGURE_DIR=$(BUILD_DIR)

#relative to CONFIGURE_DIR
CONFIGURE_ACTION=configure CFLAGS="$(ARCH_FLAGS) $(EXTRA_CFLAGS)"
CONFIGURE_FLAGS=$(eq,$(OS),MACOSX CPPFLAGS="$(EXTRA_CDEFS)" $(NULL))

BUILD_ACTION=make

OUT2LIB=$(BUILD_DIR)$/src$/.libs$/libtextcat*$(DLLPOST)

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2"
BUILD_ACTION=cd src && dmake $(MAKEMACROS)
.ENDIF # "$(GUI)"=="WNT" || "$(GUI)"=="OS2"


OUT2INC= \
    $(BUILD_DIR)$/src$/config.h \
    $(BUILD_DIR)$/src$/common.h \
    $(BUILD_DIR)$/src$/fingerprint.h \
    $(BUILD_DIR)$/src$/textcat.h \
    $(BUILD_DIR)$/src$/wg_mempool.h


# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

