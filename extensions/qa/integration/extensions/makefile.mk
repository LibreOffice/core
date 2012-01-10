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



PRJ = ..$/..$/..
TARGET  = ExtensionsIntegrationTests
PRJNAME = extensions
PACKAGE = integration$/$(PRJNAME)

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
JAVAFILES       := $(shell @$(FIND) .$/*.java)
JAVACLASSFILES	:= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

# --- Runner Settings ----------------------------------------------

# create connection string for OOoRunner
.IF "$(RUNNER_CONNECTION_STRING)" == ""
    .IF "$(OOO_RUNNER_PORT)" == ""
        OOO_RUNNER_PORT=8100
    .ENDIF
    .IF "$(OOO_RUNNER_HOST)" == ""
        OOO_RUNNER_HOST=localhost
    .ENDIF
    RUNNER_CONNECTION_STRING=socket,host=$(OOO_RUNNER_HOST),port=$(OOO_RUNNER_PORT)
.ENDIF

# classpath and argument list
RUNNER_CLASSPATH = -cp $(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)$/OOoRunner.jar$(PATH_SEPERATOR)$(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)$/ConnectivityTools.jar
RUNNER_ARGS = org.openoffice.Runner -TestBase java_complex -cs $(RUNNER_CONNECTION_STRING)

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL :   ALLTAR
.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk

run:
    +java $(RUNNER_CLASSPATH) $(RUNNER_ARGS) -sce extensions_complex.sce

run_%:
    +java $(RUNNER_CLASSPATH) $(RUNNER_ARGS) -o integration.$(PRJNAME).$(@:s/run_//)

