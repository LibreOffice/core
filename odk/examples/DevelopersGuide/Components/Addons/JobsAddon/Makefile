#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.5 $
#
#  last change: $Author: rt $ $Date: 2004-05-18 13:20:37 $
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

# Builds the DevGuide Jobs Addon component example of the SDK.

PRJ=../../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=AsyncJob
OUT_COMP_CLASS=$(OUT_CLASS)/$(COMPONENT_NAME)
OUT_COMP_MISC=$(OUT_MISC)/$(COMPONENT_NAME)
COMPONENT_PACKAGE=$(OUT_BIN)/$(COMPONENT_NAME).uno.zip
COMPONENT_PACKAGE_URL=$(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT_NAME).uno.zip")
COMPONENT_JAR_NAME=$(COMPONENT_NAME).uno.jar
COMPONENT_JAR=$(OUT_CLASS)/$(COMPONENT_JAR_NAME)
COMPONENT_MANIFESTFILE=$(OUT_COMP_CLASS)/$(COMPONENT_NAME).uno.Manifest

REGISTERFLAG=$(OUT_COMP_MISC)$(PS)java_$(COMPONENT_NAME)_register_component.flag

JAVAFILES  = AsyncJob.java

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : DevGuideJobsAddon

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: $(basename $(basename $(@F)))> $@

$(CLASSFILES) : $(JAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES) 

$(COMPONENT_JAR) : $(COMPONENT_MANIFESTFILE) $(CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAR) cvfm $@ $< -C $(OUT_COMP_CLASS) .

$(COMPONENT_PACKAGE) : $(COMPONENT_JAR) Addons.xcu Jobs.xcu
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_MISC))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_COMP_MISC))
	$(COPY) Addons.xcu $(subst /,$(PS),$(OUT_COMP_MISC))
	$(COPY) Jobs.xcu $(subst /,$(PS),$(OUT_COMP_MISC))
	cd $(subst /,$(PS),$(OUT_COMP_MISC)) && $(SDK_JAR) cvfM ../../bin/$(@F) $(^F)

$(REGISTERFLAG) : $(COMPONENT_PACKAGE)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	$(DEPLOYTOOL) $(COMPONENT_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

DevGuideJobsAddon : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The "$(QM)AsyncJob$(QM)" addon component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_MISC))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMPONENT_PACKAGE_URL)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMPONENT_JAR)))
