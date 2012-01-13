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



PRJ=..$/..

PRJNAME=apache-commons
TARGET=commons-lang

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# override buildfile
ANT_BUILDFILE=build.xml

.INCLUDE : antsettings.mk

TAR!:=$(GNUTAR)

.IF "$(SOLAR_JAVA)" != "" && "$(ENABLE_MEDIAWIKI)" == "YES"
# --- Files --------------------------------------------------------

TARFILE_NAME=commons-lang-2.3-src
TARFILE_MD5=2ae988b339daec234019a7066f96733e

TARFILE_ROOTDIR=commons-lang-2.3-src

#PATCH_FILES=$(PRJ)$/patches$/logging.patch

#CONVERTFILES=build.xml
                
OUT2CLASS=dist$/commons-lang-2.3.jar

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj -f $(ANT_BUILDFILE) jar
.ELSE
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -f $(ANT_BUILDFILE) jar
.ENDIF

.ENDIF # $(SOLAR_JAVA)!= ""

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != "" && "$(ENABLE_MEDIAWIKI)" == "YES"
.INCLUDE : tg_ext.mk
.ENDIF

