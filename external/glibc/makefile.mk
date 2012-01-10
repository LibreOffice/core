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



PRJ=..

PRJNAME=external
TARGET=getopt

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(HAVE_GETOPT)" != "YES" || "$(HAVE_READDIR_R)" != "YES"
TARFILE_NAME=glibc-2.1.3-stub
TARFILE_MD5=4a660ce8466c9df01f19036435425c3a
TARFILE_ROOTDIR=glibc-2.1.3
ADDITIONAL_FILES=posix$/makefile.mk posix$/config.h
.IF "$(HAVE_READDIR_R)" != "YES"
ADDITIONAL_FILES += posix$/readdir_r.c
.ENDIF

PATCH_FILES=$(PRJ)$/glibc-2.1.3.patch

#CONFIGURE_DIR=glibc-2.1.3/posix
CONFIGURE_ACTION=

BUILD_DIR=posix
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

OUT2INC= \
    posix/getopt.h \
    posix/config.h

.ELSE
@all:
    @echo "Nothing to do here."
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

