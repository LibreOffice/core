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

# Builds the Java NotesAccess example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
EXAMPLE_NAME=NotesAccessExample
OUT_APP_CLASS = $(OUT_CLASS)/$(EXAMPLE_NAME)

APP_NAME=NotesAccess
APP_JAR=$(OUT_APP_CLASS)/$(APP_NAME).jar

JAVAFILES  = \
	NotesAccess.java

CLASSFILES = $(patsubst %.java,$(OUT_APP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_APP_CLASS))


# Targets
.PHONY: ALL
ALL : \
    JavaNotesAccessExample

include $(SETTINGS)/stdtarget.mk

NotesAPIs : 
	@echo --------------------------------------------------------------------------------
	@echo This example uses external APIs from Lotus Notes/Domino. You can download a 
	@echo trial version from "$(QM)http://www.lotus.com/downloads$(QM)". After installation
	@echo please do the following:
	@echo  1. Add the install path to the PATH or LD_LIBRARY_PATH of this shell.
	@echo     e.g "$(QM)PATH=C:\Program Files\lotus\notes;...$(QM)"
	@echo  2. Add the "$(QM)Notes.jar$(QM)" to your CLASSPATH
	@echo     e.g. "$(QM)CLASSPATH=C:\Program Files\lotus\notes\Notes.jar;$(QM)"
	@echo --------------------------------------------------------------------------------


$(CLASSFILES) : $(JAVAFILES) 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_APP_CLASS) $(JAVAFILES) 

$(OUT_APP_CLASS)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Class-Path: Notes.jar> $@
	@echo Main-Class: com.sun.star.lib.loader.Loader>> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $*>> $@

$(APP_JAR) : $(OUT_APP_CLASS)/$(APP_NAME).mf $(OUT_APP_CLASS)/$(APP_NAME).class
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(OUT_APP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(APP_NAME).mf $(APP_NAME).class
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

JavaNotesAccessExample : NotesAPIs $(APP_JAR)
	@echo --------------------------------------------------------------------------------
	@echo NOTE: Before you run this example make sure that the exaternal "$(QM)Notes.jar$(QM)" can
	@echo be found besides "$(QM)$(APP_JAR)"$(QM)
	@echo and that the PATH variable includes the Notes installation directory.
	@echo -   
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) NotesAccess.run
	@echo --------------------------------------------------------------------------------

%.run: $(APP_JAR)
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $< "$(QM)$(QM)" "$(QM)$(QM)" "$(QM)$(QM)" Stocks.nsf

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst \\,\,$(subst /,$(PS),$(OUT_APP_CLASS)))
