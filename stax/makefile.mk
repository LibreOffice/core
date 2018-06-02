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

PRJNAME=stax
TARGET=stax
USE_JAVAVER=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : antsettings.mk

.IF "$(SOLAR_JAVA)" != ""
.IF "$(JAVANUMVER:s/.//)" >= "000100060000" || "$(JDK)"=="gcj"
all:
    @echo "Your java version already contains StAX"
.ENDIF			# "$(JAVANUMVER:s/.//)" >= "000100060000" 
.IF "$(SYSTEM_SAXON)" == "YES"
all:
    @echo "An already available installation of saxon should exist on your system."
    @echo "Therefore the files provided here does not need to be built in addition."
.ENDIF

.IF "$(BUILD_STAX)" != "YES"

$(OUT)$/class$/stax-1.2.0.jar : $(PRJ)$/download$/stax-1.2.0.jar
    +$(COPY) $< $@

.ELSE 			#  "$(BUILD_STAX)" != "YES"

# --- Files --------------------------------------------------------
TARFILE_NAME=stax-src-1.2.0
TARFILE_MD5=980143f96b3f6ce45d2e4947da21a5e9
TARFILE_ROOTDIR=src

BUILD_ACTION=$(JAVAC) javax$/xml$/stream$/*.java && jar -cf stax-1.2.0.jar javax$/xml$/stream$/*.class javax$/xml$/stream$/events$/*.class javax$/xml$/stream$/util$/*.class

OUT2CLASS=stax-1.2.0.jar
.ENDIF			#  "$(BUILD_STAX)" != "YES"

.ELSE			# $(SOLAR_JAVA)!= ""
nojava:
    @echo "Not building $(PRJNAME) because Java is disabled"
.ENDIF			# $(SOLAR_JAVA)!= ""
# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk
.ENDIF

