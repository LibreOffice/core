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
TARGET=commons-logging

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# override buildfile
ANT_BUILDFILE=build.xml

.INCLUDE : antsettings.mk

.IF "$(SOLAR_JAVA)" != "" && ( "$(ENABLE_MEDIAWIKI)" == "YES" || "$(ENABLE_REPORTBUILDER)" == "YES" )
# --- Files --------------------------------------------------------

TARFILE_NAME=commons-logging-1.1.3-src
TARFILE_MD5=e8e197d628436490886d17cffa108fe3
#TARFILE_NAME=commons-logging-1.1.1-src
#TARFILE_MD5=3c219630e4302863a9a83d0efde889db

TARFILE_ROOTDIR=commons-logging-1.1.3-src
#TARFILE_ROOTDIR=commons-logging-1.1.1-src

PATCH_FILES=$(PRJ)$/patches$/logging.patch

CONVERTFILES=build.xml

OUT2CLASS=target$/commons-logging-1.1.3.jar
#OUT2CLASS=target$/commons-logging-1.1.1-SNAPSHOT.jar

.IF "$(SYSTEM_TOMCAT)" != "YES"
SERVLETAPI_JAR := $(SOLARVER)$/$(INPATH)$/bin$(UPDMINOREXT)$/servlet-api.jar
.ENDIF

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj -Dservletapi.jar=$(SERVLETAPI_JAR) -f $(ANT_BUILDFILE) compile build-jar
.ELSE
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -f $(ANT_BUILDFILE) -Dservletapi.jar=$(SERVLETAPI_JAR) compile build-jar
.ENDIF

.ENDIF # $(SOLAR_JAVA)!= ""

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != "" && ( "$(ENABLE_MEDIAWIKI)" == "YES" || "$(ENABLE_REPORTBUILDER)" == "YES" )
.INCLUDE : tg_ext.mk
.ENDIF

