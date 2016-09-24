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

# Builds the OfficeDevLinguistic examples of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
EXAMPLE_NAME=OfficeDevLinguisticExample
OUT_COMP_CLASS = $(OUT_CLASS)/$(EXAMPLE_NAME)

COMP1_NAME=SampleHyphenator
COMP1_PACKAGE = $(OUT_BIN)/$(COMP1_NAME).$(UNOOXT_EXT)
COMP1_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP1_NAME).$(UNOOXT_EXT)")
COMP1_JAR_NAME = $(COMP1_NAME).uno.jar
COMP1_JAR = $(OUT_COMP_CLASS)/$(COMP1_JAR_NAME)
COMP1_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMP1_NAME).uno.Manifest
COMP1_UNOPKG_MANIFEST = $(OUT_COMP_CLASS)/$(COMP1_NAME)/META-INF/manifest.xml
COMP1_COMPONENTS=$(COMP1_NAME).components

COMP2_NAME=SampleSpellChecker
COMP2_PACKAGE = $(OUT_BIN)/$(COMP2_NAME).$(UNOOXT_EXT)
COMP2_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP2_NAME).$(UNOOXT_EXT)")
COMP2_JAR_NAME = $(COMP2_NAME).uno.jar
COMP2_JAR = $(OUT_COMP_CLASS)/$(COMP2_JAR_NAME)
COMP2_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMP2_NAME).uno.Manifest
COMP2_UNOPKG_MANIFEST = $(OUT_COMP_CLASS)/$(COMP2_NAME)/META-INF/manifest.xml
COMP2_COMPONENTS=$(COMP2_NAME).components

COMP3_NAME=SampleThesaurus
COMP3_PACKAGE = $(OUT_BIN)/$(COMP3_NAME).$(UNOOXT_EXT)
COMP3_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP3_NAME).$(UNOOXT_EXT)")
COMP3_JAR_NAME = $(COMP3_NAME).uno.jar
COMP3_JAR = $(OUT_COMP_CLASS)/$(COMP3_JAR_NAME)
COMP3_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMP3_NAME).uno.Manifest
COMP3_UNOPKG_MANIFEST = $(OUT_COMP_CLASS)/$(COMP3_NAME)/META-INF/manifest.xml
COMP3_COMPONENTS=$(COMP2_NAME).components

APP1_NAME=LinguisticExamples
APP1_JAR=$(OUT_COMP_CLASS)/$(APP1_NAME).jar

REGISTERFLAG = $(OUT_MISC)/devguide_officedevlinguistic_register_component.flag

JAVAFILES  = \
	OneInstanceFactory.java \
	PropChgHelper.java \
	PropChgHelper_Hyph.java \
	PropChgHelper_Spell.java \
	XHyphenatedWord_impl.java \
	XMeaning_impl.java \
	XPossibleHyphens_impl.java \
	XSpellAlternatives_impl.java \
	SampleHyphenator.java \
	SampleSpellChecker.java \
	SampleThesaurus.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

$(COMP1_NAME)_CLASSFILES = XHyphenatedWord_impl.class \
	XPossibleHyphens_impl.class \
	PropChgHelper.class \
	PropChgHelper_Hyph.class \
	OneInstanceFactory.class \
	$(COMP1_NAME).class

$(COMP2_NAME)_CLASSFILES = XSpellAlternatives_impl.class \
	PropChgHelper_Spell.class \
	PropChgHelper.class \
	OneInstanceFactory.class \
	$(COMP2_NAME).class

$(COMP3_NAME)_CLASSFILES = XMeaning_impl.class \
	OneInstanceFactory.class \
	$(COMP3_NAME).class

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

.PHONY: ALL
ALL : $(EXAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: $(basename $(basename $(@F)))> $@

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

# NOTE: because of gnu make problems with full qualified paths, the pattern 
# rules for the component jars and the packages doesn't work proper and we 
# defined explicit rules

# rule for component package manifest
$(OUT_COMP_CLASS)/%/manifest.xml :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-components$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(subst /META-INF,,$(subst $(OUT_COMP_CLASS)/,,$(@D))).components$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

$(COMP1_JAR) : $(COMP1_MANIFESTFILE) $(CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(<F) $($(basename $(basename $(@F)))_CLASSFILES)

$(COMP1_PACKAGE) : $(COMP1_JAR) $(COMP1_UNOPKG_MANIFEST) $(COMP1_COMPONENTS)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ $(COMP1_COMPONENTS)
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_ZIP) -u ../../bin/$(@F) $(<F)
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)/$(subst .$(UNOOXT_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml

$(COMP2_JAR) : $(COMP2_MANIFESTFILE) $(CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(<F) $($(basename $(basename $(@F)))_CLASSFILES)

$(COMP2_PACKAGE) : $(COMP2_JAR) $(COMP2_UNOPKG_MANIFEST) $(COMP2_COMPONENTS)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ $(COMP2_COMPONENTS)
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_ZIP) -u ../../bin/$(@F) $(<F)
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)/$(subst .$(UNOOXT_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml

$(COMP3_JAR) : $(COMP3_MANIFESTFILE) $(CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(<F) $($(basename $(basename $(@F)))_CLASSFILES)

$(COMP3_PACKAGE) : $(COMP3_JAR) $(COMP3_UNOPKG_MANIFEST) $(COMP3_COMPONENTS)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ $(COMP3_COMPONENTS)
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_ZIP) -u ../../bin/$(@F) $(<F)
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)/$(subst .$(UNOOXT_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml

$(REGISTERFLAG) : $(COMP1_PACKAGE) $(COMP2_PACKAGE) $(COMP3_PACKAGE)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	$(DEPLOYTOOL) $(COMP1_PACKAGE_URL)
	$(DEPLOYTOOL) $(COMP2_PACKAGE_URL)
	$(DEPLOYTOOL) $(COMP3_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

$(OUT_COMP_CLASS)/%.class : %.java
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $<

$(OUT_COMP_CLASS)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: com.sun.star.lib.loader.Loader> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $*>> $@

$(APP1_JAR) : $(OUT_COMP_CLASS)/$(APP1_NAME).mf $(OUT_COMP_CLASS)/$(APP1_NAME).class 
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_JAR) cvfm $(@F) $(basename $(@F)).mf $(basename $(@F)).class $(basename $(@F))$(QUOTE)$$Client.class
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)


$(EXAMPLE_NAME) : $(REGISTERFLAG) $(APP1_JAR) 
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) $(APP1_NAME).run 
	@echo --------
	@echo Before you can run the examples the components "$(QM)$(COMP1_NAME)$(QM)", 
	@echo "$(QM)$(COMP2_NAME)$(QM)" and "$(QM)$(COMP3_NAME)$(QM)" must be deployed. 
	@echo The components will be automatically deployed if SDK_AUTO_DEPLOYMENT = YES.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.jar
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $<

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP1_PACKAGE_URL)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP2_PACKAGE_URL)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP3_PACKAGE_URL)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(REGISTERFLAG)))
