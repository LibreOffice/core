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

# Builds the Charts examples of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
SAMPLE_NAME=DevGuideChartExample
SAMPLE_CLASS_OUT = $(OUT_CLASS)/$(SAMPLE_NAME)
SAMPLE_GEN_OUT = $(OUT_MISC)/$(SAMPLE_NAME)

APP1_NAME=ChartInCalc
APP1_JAR=$(SAMPLE_CLASS_OUT)/$(APP1_NAME).jar
APP2_NAME=ChartInDraw
APP2_JAR=$(SAMPLE_CLASS_OUT)/$(APP2_NAME).jar
APP3_NAME=ChartInWriter
APP3_JAR=$(SAMPLE_CLASS_OUT)/$(APP3_NAME).jar
APP4_NAME=ListenAtCalcRangeInDraw
APP4_JAR=$(SAMPLE_CLASS_OUT)/$(APP4_NAME).jar
APP5_NAME=SelectionChangeListener
APP5_JAR=$(SAMPLE_CLASS_OUT)/$(APP5_NAME).jar

COMP_NAME=JavaSampleChartAddIn
COMP_PACKAGE = $(OUT_BIN)/$(COMP_NAME).$(UNOOXT_EXT)
COMP_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOOXT_EXT)")
COMP_JAR_NAME = $(COMP_NAME).uno.jar
COMP_JAR = $(SAMPLE_CLASS_OUT)/$(COMP_JAR_NAME)
COMP_MANIFESTFILE = $(SAMPLE_CLASS_OUT)/$(COMP_NAME).uno.Manifest
COMP_UNOPKG_MANIFEST = $(SAMPLE_CLASS_OUT)/$(COMP_NAME)/META-INF/manifest.xml
COMP_REGISTERFLAG = $(SAMPLE_GEN_OUT)/devguide_$(COMP_NAME)_register_component.flag
COMP_COMPONENTS=$(COMP_NAME).components

COMPJAVAFILES  = \
	   JavaSampleChartAddIn.java

APP_JAVAFILES  = \
	   Helper.java \
	   CalcHelper.java \
	   ChartHelper.java

COMPCLASSFILES= $(patsubst %.java,$(SAMPLE_CLASS_OUT)/%.class,$(COMPJAVAFILES))

APP_CLASSFILES= $(patsubst %.java,$(SAMPLE_CLASS_OUT)/%.class,$(APP_JAVAFILES))
APP_CLASSNAMES= $(patsubst %.java,%.class,$(APP_JAVAFILES))

$(COMP_NAME)_CLASSFILES = $(COMP_NAME).class

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(SAMPLE_CLASS_OUT))


# Targets
.PHONY: ALL
ALL : $(SAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

$(SAMPLE_CLASS_OUT)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: $(basename $(basename $(@F)))> $@

# rule for client/example application class files, explicit dependencies to common
# java files which are used in all examples.
$(SAMPLE_CLASS_OUT)/%.class : %.java $(APP_JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(SAMPLE_CLASS_OUT) $< $(APP_JAVAFILES)

$(COMPCLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(SAMPLE_CLASS_OUT) $(COMPJAVAFILES) 

# rule for client/example application manifest file
$(SAMPLE_CLASS_OUT)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: com.sun.star.lib.loader.Loader> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $*>> $@

# rule for client/example application jar file
$(SAMPLE_CLASS_OUT)/%.jar : $(SAMPLE_CLASS_OUT)/%.mf $(SAMPLE_CLASS_OUT)/%.class
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	+cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)) && $(SDK_JAR) cvfm $(@F) $*.mf $**.class $(APP_CLASSNAMES)

	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

# rule for component jar file
$(COMP_JAR) : $(COMP_MANIFESTFILE) $(COMPCLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)) && $(SDK_JAR) cvfm $(@F) $(<F) $($(basename $(basename $(@F)))_CLASSFILES)

# rule for component package manifest
$(SAMPLE_CLASS_OUT)/%/manifest.xml :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-components$(QM)">> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(COMP_COMPONENTS)$(QM)"/$(CSEP)>> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

# rule for component package file
$(COMP_PACKAGE) : $(COMP_JAR) $(COMP_UNOPKG_MANIFEST) $(COMP_COMPONENTS)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ $(COMP_COMPONENTS)
	cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)) && $(SDK_ZIP) ../../bin/$(@F) -u $(<F)
	cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)/$(subst .$(UNOOXT_EXT),,$(@F))) && $(SDK_ZIP) ../../../bin/$(@F) -u META-INF/manifest.xml

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

$(APP1_JAR) : $(SAMPLE_CLASS_OUT)/$(APP1_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP1_NAME).class
$(APP2_JAR) : $(SAMPLE_CLASS_OUT)/$(APP2_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP2_NAME).class
$(APP3_JAR) : $(SAMPLE_CLASS_OUT)/$(APP3_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP3_NAME).class
$(APP4_JAR) : $(SAMPLE_CLASS_OUT)/$(APP4_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP4_NAME).class
$(APP5_JAR) : $(SAMPLE_CLASS_OUT)/$(APP5_NAME).mf $(SAMPLE_CLASS_OUT)/$(APP5_NAME).class

$(SAMPLE_NAME) : $(COMP_REGISTERFLAG) $(APP1_JAR)  $(APP2_JAR)  $(APP3_JAR)  $(APP4_JAR)  $(APP5_JAR)
	@echo --------------------------------------------------------------------------------
	@echo When you run the "$(QM)$(APP5_NAME)$(QM)" example please select the
	@echo legend or the title to add a $(APP5_NAME). The example terminates
	@echo when the document is closed.
	@echo -
	@echo Please use the following commands to execute the examples!
	@echo -
	@echo $(MAKE) $(APP1_NAME).run
	@echo $(MAKE) $(APP2_NAME).run
	@echo $(MAKE) $(APP3_NAME).run
	@echo $(MAKE) $(APP4_NAME).run
	@echo $(MAKE) $(APP5_NAME).run
	@echo --------
	@echo The Chart add-in component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo Load the "$(QM)AddInChart.ods$(QM)" document in your office to see the new chart type,
	@echo and see the example description.
	@echo -
	@echo $(MAKE) AddInChart.ods.load
	@echo --------------------------------------------------------------------------------

%.run: $(SAMPLE_CLASS_OUT)/%.jar
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $<

AddInChart.ods.load : $(REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_CLASS_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_PACKAGE_URL)))
