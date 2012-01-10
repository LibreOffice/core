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

PRJNAME=saxon
TARGET=saxon

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : antsettings.mk

.IF "$(SYSTEM_SAXON)" == "YES"

all:
    @echo "An already available installation of saxon should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."

.ELIF "$(DISABLE_SAXON)" == "YES"

all:
    @echo Support for saxon is disabled.

.ELIF "$(SOLAR_JAVA)" == ""

all:
    @echo No Java support.  Can not compile saxon.

.ELSE

# --- Files --------------------------------------------------------

TARFILE_NAME=source-9.0.0.7-bj
TARFILE_MD5=ada24d37d8d638b3d8a9985e80bc2978
ADDITIONAL_FILES=build.xml
TARFILE_ROOTDIR=src

PATCH_FILES=saxon-9.0.0.7-bj.patch

BUILD_ACTION=$(ANT) $(ANT_FLAGS) -Dsolarbindir=$(SOLARBINDIR) jar-bj 

OUT2CLASS= saxon-build$/9.0.0.7$/bj$/saxon9.jar

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ENDIF # "$(SOLAR_JAVA)" == ""
