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

# Builds the Configuration example of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
SAMPLE_NAME=DevGuideConfigExamples
SAMPLE_CLASS_OUT = $(OUT_CLASS)/$(SAMPLE_NAME)
SAMPLE_GEN_OUT = $(OUT_MISC)/$(SAMPLE_NAME)

# some special macro names for separating the example jar stuff
APP1_NAME=ConfigExamples
APP1_CLASS_OUT=$(SAMPLE_CLASS_OUT)/$(APP1_NAME)
APP1_GEN_OUT=$(SAMPLE_GEN_OUT)/$(APP1_NAME)
APP1_JAR=$(SAMPLE_CLASS_OUT)/$(APP1_NAME).jar
APP1_JAR_MANIFEST=$(APP1_GEN_OUT)/$(APP1_NAME).mf

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(SAMPLE_CLASS_OUT)\
		$(PATH_SEPARATOR)$(APP1_CLASS_OUT))

# Targets
.PHONY: ALL
ALL : $(SAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

# rule for example application class files
$(APP1_CLASS_OUT)/%.class : %.java
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(APP1_CLASS_OUT) $<

# rule for client/example application manifest file
$(APP1_GEN_OUT)/%.mf :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo Main-Class: com.sun.star.lib.loader.Loader> $@
	$(ECHOLINE)>> $@
	@echo Name: com/sun/star/lib/loader/Loader.class>> $@
	@echo Application-Class: $*>> $@

# rule for client/example application jar file
$(APP1_JAR) : $(APP1_JAR_MANIFEST) $(APP1_CLASS_OUT)/$(APP1_NAME).class
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAR) cvfm $@  $< -C $(APP1_CLASS_OUT) .
	+$(SDK_JAR) uvf $@ $(SDK_JAVA_UNO_BOOTSTRAP_FILES)

$(SAMPLE_NAME) : $(APP1_JAR)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) $(APP1_NAME).run
	@echo --------------------------------------------------------------------------------

%.run: $(SAMPLE_CLASS_OUT)/%.jar
	$(SDK_JAVA) -Dcom.sun.star.lib.loader.unopath="$(OFFICE_PROGRAM_PATH)" -jar $<

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_CLASS_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
