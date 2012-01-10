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

PRJNAME=ooo_beanshell
TARGET=ooo_beanshell

.IF "$(SOLAR_JAVA)"==""

all:
        @echo java disabled

.ELIF "$(DISABLE_BEANSHELL)"=="YES"

all:
        @echo beanshell support disabled

.ELSE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=bsh-2.0b1-src
TARFILE_MD5=ea570af93c284aa9e5621cd563f54f4d
TARFILE_ROOTDIR=BeanShell
PATCH_FILES=bsh-2.0b1-src.patch

ADDITIONAL_FILES=makefile.mk

BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ENDIF
