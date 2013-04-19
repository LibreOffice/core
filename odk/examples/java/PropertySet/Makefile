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

# Builds the Java PropertySet component example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
COMP_NAME=PropTest
COMP_CLASS_OUT=$(OUT_CLASS)/$(COMP_NAME)
COMP_GEN_OUT=$(OUT_MISC)/$(COMP_NAME)
COMP_PACKAGE=$(OUT_BIN)/$(COMP_NAME).$(UNOOXT_EXT)
COMP_PACKAGE_URL=$(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOOXT_EXT)")
COMP_JAR_NAME=$(COMP_NAME).uno.jar
COMP_JAR=$(COMP_CLASS_OUT)/$(COMP_JAR_NAME)
COMP_JAR_MANIFEST=$(COMP_CLASS_OUT)/$(COMP_NAME).uno.Manifest
COMP_UNOPKG_MANIFEST = $(COMP_GEN_OUT)/META-INF/manifest.xml
COMP_REGISTERFLAG=$(COMP_GEN_OUT)$(PS)java_$(COMP_NAME)_register_component.flag
COMP_COMPONENTS=$(COMP_NAME).components

COMP_JAVAFILES  = PropTest.java

COMP_CLASSFILES = $(patsubst %.java,$(COMP_CLASS_OUT)/%.class,$(COMP_JAVAFILES))

$(COMP_NAME)_CLASSFILES = $(COMP_NAME).class

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(COMP_CLASS_OUT))


# Targets
.PHONY: ALL
ALL : JavaPropertySetExample

include $(SETTINGS)/stdtarget.mk

$(COMP_CLASS_OUT)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: $(basename $(basename $(@F)))> $@

$(COMP_CLASSFILES) : $(COMP_JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(COMP_CLASS_OUT) $(COMP_JAVAFILES) 

# rule for component jar file
$(COMP_JAR) : $(COMP_JAR_MANIFEST) $(COMP_CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(subst /,$(PS),$(COMP_CLASS_OUT)) && $(SDK_JAR) cvfm $(@F) $(<F) $($(basename $(basename $(@F)))_CLASSFILES)

# rule for component package manifest
$(COMP_GEN_OUT)/%/manifest.xml :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-components$(QM)">> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(COMP_NAME).components$(QM)"/$(CSEP)>> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

# rule for component package file
$(COMP_PACKAGE) : $(COMP_JAR) $(COMP_UNOPKG_MANIFEST) $(COMP_NAME).components
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ $(COMP_NAME).components
	cd $(subst /,$(PS),$(COMP_CLASS_OUT)) && $(SDK_ZIP) -u ../../bin/$(@F) $(<F)
	cd $(subst /,$(PS),$(COMP_GEN_OUT)) && $(SDK_ZIP) -u ../../bin/$(@F) META-INF/manifest.xml


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

JavaPropertySetExample : $(COMP_REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The PropTest was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo Load the "$(QM)PropertySet.odt$(QM)" document to see how this component works. You can
	@echo use this component inside your office installation, see the example description.
	@echo -
	@echo $(MAKE) PropertySet.odt.load
	@echo --------------------------------------------------------------------------------

PropertySet.odt.load : $(COMP_REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(COMP_CLASS_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(COMP_GEN_OUT))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_PACKAGE_URL)))
