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

# Builds the Java Spreadsheet examples of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
EXAMPLE_NAME=JavaSpreadsheetExamples
OUT_COMP_CLASS = $(OUT_CLASS)/$(EXAMPLE_NAME)
OUT_COMP_GEN = $(OUT_MISC)/$(EXAMPLE_NAME)

APP1_NAME=EuroAdaption
APP1_JAR=$(OUT_COMP_CLASS)/$(APP1_NAME).jar
APP2_NAME=SCalc
APP2_JAR=$(OUT_COMP_CLASS)/$(APP2_NAME).jar
APP3_NAME=ChartTypeChange
APP3_JAR=$(OUT_COMP_CLASS)/$(APP3_NAME).jar

COMP_NAME=CalcAddins
COMP_RDB_NAME=$(COMP_NAME).uno.rdb
COMP_RDB=$(OUT_COMP_GEN)/$(COMP_RDB_NAME)
COMP_PACKAGE=$(OUT_BIN)/$(COMP_NAME).$(UNOOXT_EXT)
COMP_PACKAGE_URL=$(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOOXT_EXT)")
COMP_JAR_NAME=$(COMP_NAME).uno.jar
COMP_JAR=$(OUT_COMP_CLASS)/$(COMP_JAR_NAME)
COMP_MANIFESTFILE=$(OUT_COMP_CLASS)/$(COMP_NAME).uno.Manifest
COMP_UNOPKG_MANIFEST = $(OUT_COMP_CLASS)/$(COMP_NAME)/META-INF/manifest.xml
COMP_REGISTERFLAG=$(OUT_MISC)$(PS)java_$(COMP_NAME)_register_component.flag
COMP_COMPONENTS=$(COMP_NAME).components

IDLFILES = XCalcAddins.idl

# normally the idl file should be stored in a directory tree fitting the module structure,
# for the example we know the module structure
PACKAGE = org/openoffice/sheet/addin

COMPJAVAFILES  = CalcAddins.java

#GENJAVAFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/$(PACKAGE)/%.java,$(IDLFILES))
GENCLASSFILES = $(patsubst %.idl,$(OUT_COMP_CLASS)/$(PACKAGE)/%.class,$(IDLFILES))
GENTYPELIST = $(subst /,.,$(patsubst %.idl,-T$(PACKAGE)/% ,$(IDLFILES)))
GENURDFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES))

COMPCLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(COMPJAVAFILES))
#COMPCLASSFILES += $(subst $(OUT_COMP_GEN),$(OUT_COMP_CLASS),$(GENJAVAFILES:.java=.class))

$(COMP_NAME)_CLASSFILES = $(COMP_NAME).class \
		$(COMP_NAME)$(QUOTE)$$_$(COMP_NAME).class

#$(COMP_NAME)_CLASSFILES += $(subst $(OUT_COMP_GEN)/,,$(GENJAVAFILES:.java=.class))
$(COMP_NAME)_CLASSFILES += $(subst $(OUT_COMP_CLASS)/,,$(GENCLASSFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

# Targets
.PHONY: ALL
ALL : $(EXAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

# rule for a component manifest file
$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo UNO-Type-Path: $(basename $*).uno.jar> $@
	@echo RegistrationClassName: $(basename $*)>> $@

$(OUT_COMP_GEN)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(IDLC) -C -I. -I$(IDL_DIR) -O$(OUT_COMP_GEN) $<

$(OUT_COMP_GEN)/%.rdb : $(GENURDFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(REGMERGE) $@ /UCR $(GENURDFILES)

#$(OUT_COMP_GEN)/%.java : $(COMP_RDB)
$(GENCLASSFILES) : $(COMP_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(JAVAMAKER) -nD $(GENTYPELIST) -O$(OUT_COMP_CLASS) $(COMP_RDB) -X$(URE_TYPES) -X$(OFFICE_TYPES)

$(OUT_COMP_CLASS)/%.class : %.java
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $<

# only the component depends on the generated types
$(COMPCLASSFILES) : $(JAVAFILES) $(GENCLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(COMPJAVAFILES) 

# rule for client/example application manifest file
$(OUT_COMP_CLASS)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: com.sun.star.lib.loader.Loader> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $*>> $@

# rule for client/example application jar file
$(OUT_COMP_CLASS)/%.jar : $(OUT_COMP_CLASS)/%.mf $(OUT_COMP_CLASS)/%.class
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_JAR) cvfm $(@F) $*.mf $*.class
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

# rule for component jar file
$(COMP_JAR) : $(COMP_MANIFESTFILE) $(COMPCLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(<F) $($(basename $(basename $(@F)))_CLASSFILES)

# rule for component package manifest
$(OUT_COMP_CLASS)/%/manifest.xml :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-typelibrary;type=RDB$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(subst /META-INF,,$(subst $(OUT_COMP_CLASS)/,,$(@D))).uno.rdb$(QM)"/$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-components$(QM)">> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(COMP_NAME).components$(QM)"/$(CSEP)>> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

# rule for component package file
$(COMP_PACKAGE) : $(COMP_RDB) $(COMP_JAR) $(COMP_UNOPKG_MANIFEST) $(COMP_NAME).components
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_COMP_CLASS))
	$(SDK_ZIP) $@ -u $(COMP_NAME).components
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_ZIP) -u ../../bin/$(@F) $(COMP_RDB_NAME) $(COMP_JAR_NAME)
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)/$(subst .$(UNOOXT_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml
	$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_COMP_CLASS)/$(<F)))

$(COMP_REGISTERFLAG) : $(COMP_PACKAGE)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(DEPLOYTOOL) $(COMP_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

$(APP1_JAR) : $(OUT_COMP_CLASS)/$(APP1_NAME).mf $(OUT_COMP_CLASS)/$(APP1_NAME).class
$(APP2_JAR) : $(OUT_COMP_CLASS)/$(APP2_NAME).mf $(OUT_COMP_CLASS)/$(APP2_NAME).class
$(APP3_JAR) : $(OUT_COMP_CLASS)/$(APP3_NAME).mf $(OUT_COMP_CLASS)/$(APP3_NAME).class

$(EXAMPLE_NAME) : $(COMP_REGISTERFLAG) $(APP1_JAR) $(APP2_JAR) $(APP3_JAR)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the examples!
	@echo -
	@echo $(MAKE) EuroAdaption.run
	@echo $(MAKE) SCalc.run
	@echo $(MAKE) ChartTypeChange.run
	@echo --------------------------------------------------------------------------------
	@echo The Calc add-in component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description. You can also load the "$(QM)CalcAddins.ods$(QM)" document to see 
	@echo how the add-in functions works.
	@echo -
	@echo $(MAKE) CalcAddins.ods.load
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.jar
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $<

CalcAddins.ods.load : $(COMP_REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_PACKAGE_URL)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_REGISTERFLAG)))
