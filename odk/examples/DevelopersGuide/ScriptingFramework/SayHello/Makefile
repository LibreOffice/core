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

# Builds the OOoBean of the Developers Guide

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
SAMPLE_NAME=SayHello
SAMPLE_CLASS_OUT = $(OUT_CLASS)/ScriptingFramework/$(SAMPLE_NAME)
SAMPLE_GEN_OUT = $(OUT_MISC)/ScriptingFramework

SCRIPT_NAME=SayHello
SCRIPT_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(SCRIPT_NAME)
SCRIPT_GEN_OUT=$(SAMPLE_GEN_OUT)/$(SCRIPT_NAME)
SCRIPT_JAR_NAME=$(SCRIPT_NAME).jar
SCRIPT_JAR=$(SAMPLE_CLASS_OUT)/$(SCRIPT_JAR_NAME)
SCRIPT_PACKAGE = $(OUT_BIN)/$(SCRIPT_NAME).$(UNOOXT_EXT)
SCRIPT_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(SCRIPT_NAME).$(UNOOXT_EXT)")
SCRIPT_REGISTERFLAG=$(SCRIPT_GEN_OUT)$(PS)devguide_scriptingframework_$(SCRIPT_NAME)_register_scriptpkg.flag

JAVAFILES  = \
	SayHello/SayHello.java

CLASSFILES = $(patsubst %.java,$(SCRIPT_CLASS_OUT)/%.class,$(subst $(SAMPLE_NAME)/,,$(JAVAFILES)))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(SAMPLE_CLASS_OUT))

# Targets
.PHONY: ALL
ALL : \
	SayHelloExample

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(JAVAFILES) 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(SCRIPT_CLASS_OUT) $(JAVAFILES)

$(SCRIPT_JAR) : $(CLASSFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAR) cvf $@ $(CLASSNAMES) -C $(SCRIPT_CLASS_OUT) .

# rule for component package file
$(SCRIPT_PACKAGE) : $(SCRIPT_JAR) META-INF/manifest.xml $(SCRIPT_NAME)/parcel-descriptor.xml
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ META-INF/manifest.xml $(SCRIPT_NAME)/parcel-descriptor.xml
	cd $(subst /,$(PS),$(SAMPLE_CLASS_OUT)/..) && $(SDK_ZIP) -u ../../bin/$(@F) $(SAMPLE_NAME)/$(SCRIPT_JAR_NAME)

$(SCRIPT_REGISTERFLAG) : $(SCRIPT_PACKAGE)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(DEPLOYTOOL) $(SCRIPT_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif


SayHelloExample : $(SCRIPT_REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The "$(QM)$(SCRIPT_NAME)$(QM)" scripting package was installed in your office installation 
	@echo if SDK_AUTO_DEPLOYMENT = YES. If you want to run the script, start the Office 
	@echo and use the Tools/Macros/Run Macro menu item.
	@echo You can also load the document "$(QM)$(SCRIPT_NAME).odt$(QM)" and press the button to run the
	@echo script.
	@echo -
	@echo $(MAKE) $(SCRIPT_NAME).odt.load
	@echo --------------------------------------------------------------------------------

$(SCRIPT_NAME).odt.load : $(SCRIPT_REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

%.run: $(CLASSFILES)
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(subst /,.,$(PACKAGE)).$(basename $@)

%.dbg: $(JAR1_JAR)
	jdb -classpath "$(SDK_CLASSPATH)" $(subst /,.,$(PACKAGE)).$(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_CLASS_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SCRIPT_GEN_OUT))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(SCRIPT_PACKAGE_URL)))

