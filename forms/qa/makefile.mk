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



PRJ = ..
TARGET  = FormIntegrationTest
PRJNAME = forms

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar ConnectivityTools.jar
JAVAFILES       :=  $(shell @$(FIND) org -name "*.java") \
                    $(shell @$(FIND) integration -name "*.java")

#----- make a jar from compiled files ------------------------------

JARTARGET       = $(TARGET).jar

# --- Runner Settings ----------------------------------------------

# classpath and argument list
RUNNER_CLASSPATH = -cp "$(CLASSPATH)$(PATH_SEPERATOR)$(SOLARBINDIR)$/OOoRunner.jar$(PATH_SEPERATOR)$(SOLARBINDIR)$/ConnectivityTools.jar"
RUNNER_ARGS = org.openoffice.Runner -TestBase java_complex

# --- Targets ------------------------------------------------------

.IF "$(depend)" == ""
ALL :   ALLTAR
    @echo -----------------------------------------------------
    @echo - do a 'dmake show_targets' to show available targets
    @echo -----------------------------------------------------
.ELSE
ALL: 	ALLDEP
.ENDIF

.INCLUDE :  target.mk

show_targets: $(CLASSDIR)$/$(JARTARGET)
    +@$(AUGMENT_LIBRARY_PATH) java $(RUNNER_CLASSPATH) complexlib.ShowTargets $(foreach,i,$(JAVAFILES) $(i:s/.\$///:s/.java//))

run: $(CLASSDIR)$/$(JARTARGET)
    +$(COPY) integration$/forms$/*.props $(CLASSDIR) && $(AUGMENT_LIBRARY_PATH) java $(RUNNER_CLASSPATH) $(RUNNER_ARGS) -sce forms_all.sce

run_%: $(CLASSDIR)$/$(JARTARGET)
    +$(COPY) integration$/forms$/*.props $(CLASSDIR) && $(AUGMENT_LIBRARY_PATH) java $(RUNNER_CLASSPATH) $(RUNNER_ARGS) -o integration.$(PRJNAME).$(@:s/run_//)
