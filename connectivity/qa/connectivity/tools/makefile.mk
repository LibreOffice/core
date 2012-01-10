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



PRJ = ../../..
TARGET  = ConnectivityTools
PRJNAME = connectivity
PACKAGE = connectivity/tools

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

.IF "$(SOLAR_JAVA)" == ""
all:
    @echo "Java not available. Build skipped"
.ELSE

#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunnerLight.jar
JAVAFILES       := $(shell @$(FIND) . -name "*.java")

#----- make a jar from compiled files ------------------------------

JARCLASSDIRS = $(PACKAGE)
JARTARGET    = $(TARGET).jar

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL :   ALLTAR
.ELSE
ALL: 	ALLDEP
.ENDIF

.ENDIF # "$(SOLAR_JAVA)" == ""

.INCLUDE :  target.mk
