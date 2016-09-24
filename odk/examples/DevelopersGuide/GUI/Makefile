#*************************************************************************
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright 2000, 2010 Oracle and/or its affiliates.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of Sun Microsystems, Inc. nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#     
#**************************************************************************

# Builds the GUI examples of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

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
