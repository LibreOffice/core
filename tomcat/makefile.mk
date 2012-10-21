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

PRJNAME=tomcat
TARGET=servlet

.IF "$(SOLAR_JAVA)" != ""
# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : antsettings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=apache-tomcat-5.5.36-src
TARFILE_MD5=63574e3ada44f473892a61a2da433a59

TARFILE_ROOTDIR=apache-tomcat-5.5.36-src

ADDITIONAL_FILES = servletapi/build.xml
#CONVERTFILES=build$/build.xml

PATCH_FILES=tomcat.patch

BUILD_DIR=servletapi
BUILD_ACTION=$(ANT)

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.INCLUDE : tg_ext.mk

.ELSE
@all:
    @echo java disabled
.ENDIF
