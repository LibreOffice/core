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

PRJNAME=so_expat
TARGET=so_expat

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_EXPAT)" == "YES"
all:
        @echo "An already available installation of expat should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

TARFILE_NAME=expat-2.2.1
TARFILE_MD5=d9c3baeab58774cefc2f04faf29f2cf8
ADDITIONAL_FILES=lib$/makefile.mk
PATCH_FILES=$(TARFILE_NAME).patch \
            expat-winapi.patch

CONFIGURE_DIR=
.IF "$(OS)"=="WNT"
CONFIGURE_ACTION=
.ELSE
CONFIGURE_ACTION=.$/configure
.ENDIF

BUILD_DIR=lib
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk
