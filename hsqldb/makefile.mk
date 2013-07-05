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

PRJNAME=hsqldb
TARGET=so_hsqldb

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# override buildfile
ANT_BUILDFILE=build$/build.xml

.INCLUDE : antsettings.mk

.INCLUDE :  version.mk

.IF "$(SOLAR_JAVA)" != ""
# --- Files --------------------------------------------------------

TARFILE_NAME=hsqldb_$(HSQLDB_VERSION)
TARFILE_MD5=17410483b5b5f267aa18b7e00b65e6e0

TARFILE_ROOTDIR=hsqldb

CONVERTFILES=build$/build.xml \
            src$/org$/hsqldb$/persist$/HsqlDatabaseProperties.java \
            src$/org$/hsqldb$/Library.java

PATCH_FILES=patches$/i121754.patch

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj -f $(ANT_BUILDFILE) jar
.ELSE
.IF "$(debug)"!=""
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.debug="on" -f $(ANT_BUILDFILE) jar
.ELSE
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -f $(ANT_BUILDFILE) jar
.ENDIF
.ENDIF

.ENDIF # $(SOLAR_JAVA)!= ""

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk
.ENDIF

