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

# Builds the GUI examples of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
SAMPLE_NAME=GUIExamples
SAMPLE_CLASS_OUT = $(OUT_CLASS)/$(SAMPLE_NAME)
SAMPLE_GEN_OUT = $(OUT_MISC)/$(SAMPLE_NAME)


APP1_NAME=DialogDocument
APP1_JAR=$(SAMPLE_CLASS_OUT)/$(APP1_NAME).jar
APP2_NAME=MessageBox
APP2_JAR=$(SAMPLE_CLASS_OUT)/$(APP2_NAME).jar
APP3_NAME=UnoDialogSample
APP3_JAR=$(SAMPLE_CLASS_OUT)/$(APP3_NAME).jar
APP4_NAME=UnoDialogSample2
APP4_JAR=$(SAMPLE_CLASS_OUT)/$(APP4_NAME).jar
APP5_NAME=UnoMenu
APP5_JAR=$(SAMPLE_CLASS_OUT)/$(APP5_NAME).jar
APP6_NAME=UnoMenu2
APP6_JAR=$(SAMPLE_CLASS_OUT)/$(APP6_NAME).jar
APP7_NAME=ImageControlSample
APP7_JAR=$(SAMPLE_CLASS_OUT)/$(APP7_NAME).jar

APP_JAVAFILES  = \
	RoadmapItemStateChangeListener.java \
	UnoDialogSample.java \
	UnoMenu.java

APP_CLASSFILES = $(patsubst %.java,$(SAMPLE_CLASS_OUT)/%.class,$(APP_JAVAFILES))
#	$(SAMPLE_CLASS_OUT)/UnoDialogSample\&RoadmapItemStateChangeListener.class

APP_CLASSNAMES = $(patsubst %.java,%.class,$(APP_JAVAFILES))
#APP_CLASSNAMES = $(patsubst $(SAMPLE_CLASS_OUT)/%.class,%.class,$(APP_CLASSFILES))


SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(SAMPLE_CLASS_OUT))


# Targets
.PHONY: ALL
ALL : \
    GUIExamples


include $(SETTINGS)/stdtarget.mk


$(APP_CLASSFILES) : $(APP_JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(SAMPLE_CLASS_OUT) $(APP_JAVAFILES)

$(SAMPLE_CLASS_OUT)/%.class : %.java $(APP_CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(SAMPLE_CLASS_OUT) $<

$(SAMPLE_CLASS_OUT)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: com.sun.star.lib.loader.Loader> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $*>> $@

$(SAMPLE_CLASS_OUT)/%.jar : $(SAMPLE_CLASS_OUT)/%.mf $(SAMPLE_CLASS_OUT)/%.class $(APP_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)) && $(SDK_JAR) cvfm $(@F) $*.mf $*.class $(APP_CLASSNAMES)
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

$(APP1_JAR) : $(SAMPLE_CLASS_OUT)/$(APP1_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP1_NAME).class
$(APP2_JAR) : $(SAMPLE_CLASS_OUT)/$(APP2_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP2_NAME).class
$(APP3_JAR) : $(SAMPLE_CLASS_OUT)/$(APP3_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP3_NAME).class
$(APP4_JAR) : $(SAMPLE_CLASS_OUT)/$(APP4_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP4_NAME).class
$(APP5_JAR) : $(SAMPLE_CLASS_OUT)/$(APP5_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP5_NAME).class
$(APP6_JAR) : $(SAMPLE_CLASS_OUT)/$(APP6_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP6_NAME).class
$(APP7_JAR) : $(SAMPLE_CLASS_OUT)/$(APP7_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP7_NAME).class

GUIExamples : $(APP1_JAR) $(APP2_JAR) $(APP3_JAR) $(APP4_JAR) $(APP5_JAR) $(APP6_JAR) $(APP7_JAR)
	@echo --------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo $(MAKE) $(APP1_NAME).run
	@echo $(MAKE) $(APP2_NAME).run
	@echo $(MAKE) $(APP3_NAME).run
	@echo $(MAKE) $(APP4_NAME).run
	@echo $(MAKE) $(APP5_NAME).run
	@echo $(MAKE) $(APP6_NAME).run
	@echo $(MAKE) $(APP7_NAME).run
	@echo --------

%.run: $(SAMPLE_CLASS_OUT)/%.jar
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $<

$(APP7_NAME).run: $(SAMPLE_CLASS_OUT)/$(APP7_NAME).jar
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $< .$(PS)oologo.gif


.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_CLASS_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
