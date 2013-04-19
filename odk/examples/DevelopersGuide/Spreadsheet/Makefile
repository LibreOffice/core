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

# Builds the SpreadSheet examples of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
SAMPLE_NAME=DevGuideSpreadsheetExamples
SAMPLE_CLASS_OUT=$(OUT_CLASS)/$(SAMPLE_NAME)
SAMPLE_GEN_OUT=$(OUT_MISC)/$(SAMPLE_NAME)

# Example add-in component
COMP1_NAME=ExampleAddIn
COMP1_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(COMP1_NAME)
COMP1_GEN_OUT=$(SAMPLE_GEN_OUT)/$(COMP1_NAME)
COMP1_RDB_NAME=$(COMP1_NAME).uno.rdb
COMP1_RDB=$(COMP1_GEN_OUT)/$(COMP1_RDB_NAME)
COMP1_PACKAGE=$(OUT_BIN)/$(COMP1_NAME).$(UNOOXT_EXT)
COMP1_PACKAGE_URL=$(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP1_NAME).$(UNOOXT_EXT)")
COMP1_JAR_NAME=$(COMP1_NAME).uno.jar
COMP1_JAR=$(SAMPLE_CLASS_OUT)/$(COMP1_JAR_NAME)
COMP1_MANIFESTFILE=$(COMP1_GEN_OUT)/$(COMP1_NAME).uno.Manifest
COMP1_UNOPKG_MANIFEST=$(COMP1_GEN_OUT)/$(COMP1_NAME)/META-INF/manifest.xml
COMP1_REGISTERFLAG=$(SAMPLE_GEN_OUT)/devguide_$(COMP1_NAME)_register_component.flag
COMP1_COMPONENTS=$(COMP1_NAME).components

# Example DataPilot component
COMP2_NAME=ExampleDataPilotSource
COMP2_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(COMP2_NAME)
COMP2_GEN_OUT=$(SAMPLE_GEN_OUT)/$(COMP2_NAME)
COMP2_PACKAGE=$(OUT_BIN)/$(COMP2_NAME).$(UNOOXT_EXT)
COMP2_PACKAGE_URL=$(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP2_NAME).$(UNOOXT_EXT)")
COMP2_JAR_NAME=$(COMP2_NAME).uno.jar
COMP2_JAR=$(SAMPLE_CLASS_OUT)/$(COMP2_JAR_NAME)
COMP2_MANIFESTFILE=$(COMP2_GEN_OUT)/$(COMP2_NAME).uno.Manifest
COMP2_UNOPKG_MANIFEST=$(COMP2_GEN_OUT)/$(COMP2_NAME)/META-INF/manifest.xml
COMP2_REGISTERFLAG=$(SAMPLE_GEN_OUT)/devguide_$(COMP2_NAME)_register_component.flag
COMP2_COMPONENTS=$(COMP2_NAME).components

#REGISTERFLAG = $(OUT_MISC)$(PS)devguide_$(SAMPLE_NAME)_register_component.flag

APP1_NAME=GeneralTableSample
APP1_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(APP1_NAME)
APP1_GEN_OUT=$(SAMPLE_GEN_OUT)/$(APP1_NAME)
APP1_JAR=$(SAMPLE_CLASS_OUT)/$(APP1_NAME).jar

APP2_NAME=SpreadsheetSample
APP2_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(APP2_NAME)
APP2_GEN_OUT=$(SAMPLE_GEN_OUT)/$(APP2_NAME)
APP2_JAR=$(SAMPLE_CLASS_OUT)/$(APP2_NAME).jar

APP3_NAME=ViewSample
APP3_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(APP3_NAME)
APP3_GEN_OUT=$(SAMPLE_GEN_OUT)/$(APP3_NAME)
APP3_JAR=$(SAMPLE_CLASS_OUT)/$(APP3_NAME).jar


COMP1_IDLFILES = XExampleAddIn.idl

# normally the idl file should be stored in a directory tree fitting the 
# module structure, for the example we know the module structure
PACKAGE = org/openoffice/sheet/addin

COMP1_JAVAFILES  = \
	   ExampleAddIn.java

COMP2_JAVAFILES  = \
	   ExampleDataPilotSource.java

APP_JAVAFILES  = \
	   SpreadsheetDocHelper.java

APP_CLASSFILES = $(patsubst %.java,$(SAMPLE_CLASS_OUT)/%.class,$(APP_JAVAFILES))
APP_CLASSNAMES = $(patsubst %.java,%.class,$(APP_JAVAFILES))

COMP1_GENCLASSFILES = $(patsubst %.idl,$(COMP1_CLASS_OUT)/$(PACKAGE)/%.class,$(COMP1_IDLFILES))
COMP1_GENTYPELIST = $(subst /,.,$(patsubst %.idl,-T$(PACKAGE)/% ,$(COMP1_IDLFILES)))
COMP1_GENURDFILES = $(patsubst %.idl,$(COMP1_GEN_OUT)/%.urd,$(COMP1_IDLFILES))

COMP1_CLASSFILES = $(patsubst %.java,$(COMP1_CLASS_OUT)/%.class,$(COMP1_JAVAFILES))

COMP2_CLASSFILES = $(patsubst %.java,$(COMP2_CLASS_OUT)/%.class,$(COMP2_JAVAFILES))


SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(SAMPLE_CLASS_OUT)\
		$(PATH_SEPARATOR)$(COMP1_CLASS_OUT)\
		$(PATH_SEPARATOR)$(COMP2_CLASS_OUT)\
		$(PATH_SEPARATOR)$(APP1_CLASS_OUT)\
		$(PATH_SEPARATOR)$(APP2_CLASS_OUT)\
		$(PATH_SEPARATOR)$(APP3_CLASS_OUT))


# Targets
.PHONY: ALL
ALL : $(SAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

$(SAMPLE_GEN_OUT)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo UNO-Type-Path: $(basename $(notdir $*)).uno.jar> $@
	@echo RegistrationClassName: $(basename $(basename $(@F)))>> $@

# IDLs only for component 1 relevant
$(COMP1_GEN_OUT)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(IDLC) -I. -I$(IDL_DIR) -O$(COMP1_GEN_OUT) $^

$(COMP1_RDB) : $(COMP1_GENURDFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(REGMERGE) $@ /UCR $<

$(COMP1_CLASS_OUT)/$(PACKAGE)/%.class : $(COMP1_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(JAVAMAKER) -nD $(COMP1_GENTYPELIST) -O$(COMP1_CLASS_OUT) $(COMP1_RDB) -X$(URE_TYPES) -X$(OFFICE_TYPES)

$(COMP1_CLASSFILES) : $(COMP1_JAVAFILES) $(COMP1_GENCLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(COMP1_CLASS_OUT) $(COMP1_JAVAFILES)

$(COMP2_CLASSFILES) : $(COMP2_JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(COMP2_CLASS_OUT) $^

$(COMP1_JAR) : $(COMP1_MANIFESTFILE) $(COMP1_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAR) cvfm $@ $< -C $(COMP1_CLASS_OUT) .

$(COMP2_JAR) : $(COMP2_MANIFESTFILE) $(COMP2_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAR) cvfm $@ $< -C $(COMP2_CLASS_OUT) .

$(COMP1_UNOPKG_MANIFEST) :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-typelibrary;type=RDB$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(subst /META-INF,,$(subst $(COMP1_GEN_OUT)/,,$(@D))).uno.rdb$(QM)"/$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-components$(QM)">> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(COMP1_COMPONENTS)$(QM)"/$(CSEP)>> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

$(COMP2_UNOPKG_MANIFEST) :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-components$(QM)">> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(COMP2_COMPONENTS)$(QM)"/$(CSEP)>> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

$(COMP1_PACKAGE) : $(COMP1_RDB) $(COMP1_JAR) $(COMP1_UNOPKG_MANIFEST) $(COMP1_COMPONENTS)
	echo "####" $(@)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ $(COMP1_COMPONENTS)
	cd $(subst /,$(PS),$(COMP1_GEN_OUT)) && $(SDK_ZIP) -u ../../../bin/$(@F) $(<F)
	cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)) && $(SDK_ZIP) -u ../../bin/$(@F) $(COMP1_JAR_NAME)
	cd $(subst /,$(PS),$(COMP1_GEN_OUT)/$(subst .$(UNOOXT_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../../bin/$(@F) META-INF/manifest.xml

$(COMP2_PACKAGE) : $(COMP2_JAR) $(COMP2_UNOPKG_MANIFEST) $(COMP2_COMPONENTS)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ $(COMP2_COMPONENTS)
	cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)) && $(SDK_ZIP) -u ../../bin/$(@F) $(<F)
	cd $(subst /,$(PS),$(COMP2_GEN_OUT)/$(subst .$(UNOOXT_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../../bin/$(@F) META-INF/manifest.xml

$(SAMPLE_GEN_OUT)/devguide_$(COMP1_NAME)_register_component.flag : $(OUT_BIN)/$(COMP1_NAME).$(UNOOXT_EXT)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(DEPLOYTOOL) $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP1_NAME).$(UNOOXT_EXT)")
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

$(SAMPLE_GEN_OUT)/devguide_$(COMP2_NAME)_register_component.flag : $(OUT_BIN)/$(COMP2_NAME).$(UNOOXT_EXT)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(DEPLOYTOOL) $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP2_NAME).$(UNOOXT_EXT)")
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

$(APP_CLASSFILES) : $(APP_JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(SAMPLE_CLASS_OUT) $(APP_JAVAFILES) 

$(APP1_CLASS_OUT)/%.class : %.java $(APP_CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(APP1_CLASS_OUT) $<

$(APP2_CLASS_OUT)/%.class : %.java $(APP_CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(APP2_CLASS_OUT) $<

$(APP3_CLASS_OUT)/%.class : %.java $(APP_CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(APP3_CLASS_OUT) $<

$(SAMPLE_GEN_OUT)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: com.sun.star.lib.loader.Loader> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $(basename $(@F))>> $@

$(APP1_JAR) : $(APP1_GEN_OUT)/$(APP1_NAME).mf $(APP1_CLASS_OUT)/$(APP1_NAME).class $(APP_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+$(SDK_JAR) cvfm $@ $< -C $(APP1_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)
	+cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)) && $(SDK_JAR) uvf $(@F) $(APP_CLASSNAMES)

$(APP2_JAR) : $(APP2_GEN_OUT)/$(APP2_NAME).mf $(APP2_CLASS_OUT)/$(APP2_NAME).class $(APP_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+$(SDK_JAR) cvfm $@ $< -C $(APP2_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)
	+cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)) && $(SDK_JAR) uvf $(@F) $(APP_CLASSNAMES)

$(APP3_JAR) : $(APP3_GEN_OUT)/$(APP3_NAME).mf $(APP3_CLASS_OUT)/$(APP3_NAME).class $(APP_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+$(SDK_JAR) cvfm $@ $< -C $(APP3_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)
	+cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)) && $(SDK_JAR) uvf $(@F) $(APP_CLASSNAMES)

$(SAMPLE_NAME) : $(COMP1_REGISTERFLAG) $(COMP2_REGISTERFLAG) $(APP1_JAR) $(APP2_JAR) $(APP3_JAR) 
	@echo --------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo $(MAKE) $(APP1_NAME).run
	@echo $(MAKE) $(APP2_NAME).run
	@echo $(MAKE) $(APP3_NAME).run
	@echo --------
	@echo The "$(QM)$(COMP1_NAME)$(QM)" and "$(QM)$(COMP2_NAME)$(QM)"  component are installed if
	@echo SDK_AUTO_DEPLOYMENT = YES. You can use this components inside your office 
	@echo installation, see the example descriptions. You can also load the "$(QM)ExampleAddIn.ods$(QM)"
	@echo  document to see how the add-in functions can be used. For more details about the 
	@echo "$(QM)$(COMP2_NAME)$(QM)" component see the "$(QM)DataPilotReadme.txt"$(QM).
	@echo -
	@echo $(MAKE) ExampleAddIn.ods.load
	@echo --------------------------------------------------------------------------------

%.run: $(SAMPLE_CLASS_OUT)/%.jar
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $<

ExampleAddIn.ods.load : $(COMP1_REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_CLASS_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP1_PACKAGE_URL)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP2_PACKAGE_URL)))
