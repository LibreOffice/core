#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.8 $
#
#  last change: $Author: rt $ $Date: 2005-03-29 12:17:54 $
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright (c) 2003 by Sun Microsystems, Inc.
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
include $(SETTINGS)/dk.mk

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

COMPONENT_NAME=CalcAddins
COMPONENT_RDB_NAME=$(COMPONENT_NAME).uno.rdb
COMPONENT_RDB=$(OUT_COMP_GEN)/$(COMPONENT_RDB_NAME)
COMPONENT_PACKAGE=$(OUT_BIN)/$(COMPONENT_NAME).$(UNOPKG_EXT)
COMPONENT_PACKAGE_URL=$(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMPONENT_NAME).$(UNOPKG_EXT)")
COMPONENT_JAR_NAME=$(COMPONENT_NAME).uno.jar
COMPONENT_JAR=$(OUT_COMP_CLASS)/$(COMPONENT_JAR_NAME)
COMPONENT_MANIFESTFILE=$(OUT_COMP_CLASS)/$(COMPONENT_NAME).uno.Manifest
COMPONENT_UNOPKG_MANIFEST = $(OUT_COMP_CLASS)/$(COMPONENT_NAME)/META-INF/manifest.xml
REGISTERFLAG=$(OUT_MISC)$(PS)java_$(COMPONENT_NAME)_register_component.flag

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

$(COMPONENT_NAME)_CLASSFILES = $(COMPONENT_NAME).class \
		$(COMPONENT_NAME)$(QUOTE)$$_$(COMPONENT_NAME).class

#$(COMPONENT_NAME)_CLASSFILES += $(subst $(OUT_COMP_GEN)/,,$(GENJAVAFILES:.java=.class))
$(COMPONENT_NAME)_CLASSFILES += $(subst $(OUT_COMP_CLASS)/,,$(GENCLASSFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

# Targets
.PHONY: ALL
ALL : $(EXAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

# rule for a component manifest file
$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: $(basename $*)> $@

$(OUT_COMP_GEN)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(IDLC) -C -I. -I$(IDL_DIR) -O$(OUT_COMP_GEN) $<

$(OUT_COMP_GEN)/%.rdb : $(GENURDFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(REGMERGE) $@ /UCR $(GENURDFILES)

#$(OUT_COMP_GEN)/%.java : $(COMPONENT_RDB)
$(GENCLASSFILES) : $(COMPONENT_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(JAVAMAKER) -BUCR -nD $(GENTYPELIST) -O$(OUT_COMP_CLASS) $(COMPONENT_RDB) $(OFFICE_TYPE_LIBRARY)

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
$(COMPONENT_JAR) : $(COMPONENT_MANIFESTFILE) $(COMPCLASSFILES)
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
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-component;type=Java$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(subst /META-INF,,$(subst $(OUT_COMP_CLASS)/,,$(@D))).uno.jar$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

# rule for component package file
$(COMPONENT_PACKAGE) : $(COMPONENT_RDB) $(COMPONENT_JAR) $(COMPONENT_UNOPKG_MANIFEST)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_COMP_CLASS))
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_ZIP) ../../bin/$(@F) $(COMPONENT_RDB_NAME) $(COMPONENT_JAR_NAME)
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)/$(subst .$(UNOPKG_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml
	$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_COMP_CLASS)/$(<F)))

$(REGISTERFLAG) : $(COMPONENT_PACKAGE)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(DEPLOYTOOL) $(COMPONENT_PACKAGE_URL)
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

$(EXAMPLE_NAME) : $(REGISTERFLAG) $(APP1_JAR) $(APP2_JAR) $(APP3_JAR)
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

CalcAddins.ods.load : $(REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMPONENT_PACKAGE_URL)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(REGISTERFLAG)))
