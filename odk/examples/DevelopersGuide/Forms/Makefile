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

# Builds the Forms example of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
SAMPLE_NAME=FormsExamples
SAMPLE_CLASS_OUT=$(OUT_CLASS)/$(SAMPLE_NAME)
SAMPLE_GEN_OUT=$(OUT_MISC)/$(SAMPLE_NAME)

APP1_NAME=DataAwareness
APP1_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(APP1_NAME)
APP1_JAR=$(SAMPLE_CLASS_OUT)/$(APP1_NAME).jar
APP2_NAME=ValueBinding
APP2_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(APP2_NAME)
APP2_JAR=$(SAMPLE_CLASS_OUT)/$(APP2_NAME).jar
APP3_NAME=SpreadsheetValueBinding
APP3_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(APP3_NAME)
APP3_JAR=$(SAMPLE_CLASS_OUT)/$(APP3_NAME).jar
APP4_NAME=ControlValidation
APP4_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(APP4_NAME)
APP4_JAR=$(SAMPLE_CLASS_OUT)/$(APP4_NAME).jar

COMMON_CLASS_OUT = $(SAMPLE_CLASS_OUT)/common
COMMON_JAVAFILES  = \
	ButtonOperator.java \
	ComponentTreeTraversal.java \
	ControlLock.java \
	DocumentHelper.java \
	DocumentType.java \
	DocumentViewHelper.java \
	FLTools.java \
	FormLayer.java \
	GridFieldValidator.java \
	InteractionRequest.java \
	KeyGenerator.java \
	SalesFilter.java \
	UNO.java \
	SpreadsheetView.java \
	WaitForInput.java \
	DocumentBasedExample.java \
	TableCellTextBinding.java \
	SpreadsheetDocument.java \
	NumericValidator.java \
	TextValidator.java \
	BooleanValidator.java \
	ListSelectionValidator.java \
	DateValidator.java \
	TimeValidator.java \
	ControlValidator.java \
	SingleControlValidation.java \
	HsqlDatabase.java \
	RowSet.java \
	URLHelper.java

COMMON_CLASSFILES = $(patsubst %.java,$(COMMON_CLASS_OUT)/%.class,$(COMMON_JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(SAMPLE_CLASS_OUT)\
		$(PATH_SEPARATOR)$(COMMON_CLASS_OUT))


# Targets
.PHONY: ALL
ALL : \
    FormsExample

include $(SETTINGS)/stdtarget.mk

$(COMMON_CLASSFILES) : $(COMMON_JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(COMMON_CLASS_OUT) $^

# rule for client/example application manifest file
$(SAMPLE_GEN_OUT)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: com.sun.star.lib.loader.Loader> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $*>> $@

$(APP1_CLASS_OUT)/$(APP1_NAME).class : $(APP1_NAME).java $(COMMON_CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(APP1_CLASS_OUT) $<

$(APP2_CLASS_OUT)/$(APP2_NAME).class : $(APP2_NAME).java $(COMMON_CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(APP2_CLASS_OUT) $<

$(APP3_CLASS_OUT)/$(APP3_NAME).class : $(APP3_NAME).java $(COMMON_CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(APP3_CLASS_OUT) $<

$(APP4_CLASS_OUT)/$(APP4_NAME).class : $(APP4_NAME).java $(COMMON_CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(APP4_CLASS_OUT) $<

$(APP1_JAR) : $(SAMPLE_GEN_OUT)/$(APP1_NAME).mf $(APP1_CLASS_OUT)/$(APP1_NAME).class $(COMMON_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+$(SDK_JAR) cvfm $@ $< -C $(APP1_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ -C $(COMMON_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

$(APP2_JAR) : $(SAMPLE_GEN_OUT)/$(APP2_NAME).mf $(APP2_CLASS_OUT)/$(APP2_NAME).class $(COMMON_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+$(SDK_JAR) cvfm $@ $< -C $(APP2_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ -C $(COMMON_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

$(APP3_JAR) : $(SAMPLE_GEN_OUT)/$(APP3_NAME).mf $(APP3_CLASS_OUT)/$(APP3_NAME).class $(COMMON_CLASSFILES) 
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+$(SDK_JAR) cvfm $@ $< -C $(APP3_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ -C $(COMMON_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

$(APP4_JAR) : $(SAMPLE_GEN_OUT)/$(APP4_NAME).mf $(APP4_CLASS_OUT)/$(APP4_NAME).class $(COMMON_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+$(SDK_JAR) cvfm $@ $< -C $(APP4_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ -C $(COMMON_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

FormsExample : $(APP1_JAR)  $(APP2_JAR)  $(APP3_JAR)  $(APP4_JAR)
	@echo --------------------------------------------------------------------------------
	@echo "$(QM)$(APP1_NAME)$(QM)" demonstrates various things around data aware controls.
	@echo "$(QM)$(APP2_NAME)$(QM)" demonstrates the use of value bindings for form controls.
	@echo "$(QM)$(APP3_NAME)$(QM)" demonstrates the usage of the com.sun.star.table.CellValueBinding service
	@echo "$(QM)$(APP4_NAME)$(QM)" demonstrates the form control validation API

	@echo Please use one of the following commands to run an example:
	@echo -
	@echo $(MAKE) $(APP1_NAME).run
	@echo $(MAKE) $(APP2_NAME).run
	@echo $(MAKE) $(APP3_NAME).run
	@echo $(MAKE) $(APP4_NAME).run
	@echo -
	@echo Note: The data awareness example needs a running data source whith name
	@echo $(SQM)      $(SQM)"$(QM)OO_SDK_Demo_DB$(QM)". Make sure that you have created such a data source,
	@echo -
	@echo The document "$(QM)ProgrammaticScriptAssignment.odt$(QM)" demonstrates
	@echo how you can programmatically create form controls, and associated 
	@echo scripts with certain events.
	@echo -
	@echo $(MAKE) ProgrammaticScriptAssignment.odt.load
	@echo --------------------------------------------------------------------------------

ProgrammaticScriptAssignment.odt.load : $(COMP_REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

%.run: $(SAMPLE_CLASS_OUT)/%.jar
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $<

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_CLASS_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
