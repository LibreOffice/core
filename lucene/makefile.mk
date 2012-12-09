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

PRJNAME=so_lucene
TARGET=so_lucene


# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :	antsettings.mk
# --- Files --------------------------------------------------------

.IF "$(SOLAR_JAVA)" != ""

LUCENE_MAJOR=2
LUCENE_MINOR=9
LUCENE_MICRO=4

LUCENE_NAME=lucene-$(LUCENE_MAJOR).$(LUCENE_MINOR).$(LUCENE_MICRO)
# NOTE that the jar names do not contain the micro version
LUCENE_CORE_JAR=lucene-core-$(LUCENE_MAJOR).$(LUCENE_MINOR).$(LUCENE_MICRO)-dev.jar
LUCENE_ANALYZERS_JAR=lucene-analyzers-$(LUCENE_MAJOR).$(LUCENE_MINOR).$(LUCENE_MICRO)-dev.jar

TARFILE_NAME=$(LUCENE_NAME)-src
TARFILE_MD5=17960f35b2239654ba608cf1f3e256b3
TARFILE_ROOTDIR=$(LUCENE_NAME)

PATCH_FILES=lucene.patch 

.IF "$(OS)" == "WNT"
PATCH_FILES+= long_path.patch
.ENDIF

BUILD_DIR=.
BUILD_ACTION= ${ANT} -buildfile .$/contrib$/analyzers$/build.xml

OUT2BIN=.$/build$/$(LUCENE_CORE_JAR) .$/build$/contrib$/analyzers/common$/$(LUCENE_ANALYZERS_JAR)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk 
.ENDIF
