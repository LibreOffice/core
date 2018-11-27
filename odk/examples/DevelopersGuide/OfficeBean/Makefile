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

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
OUT_COMP_CLASS = $(OUT_CLASS)/OOoBeanViewer

PACKAGE = org/libreoffice/sdk/examples/developers_guide/office_bean

JAVAFILES  = \
	OOoBeanViewer.java \

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/$(PACKAGE)/%.class,$(JAVAFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(URE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(URE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(URE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/officebean.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))

# Targets
.PHONY: ALL
ifeq "$(OS)" "MACOSX"
ALL :
	@printf 'This example does not work on macOS\n'
else
ALL : \
    OOoBeanViewer
endif

include $(SETTINGS)/stdtarget.mk

$(CLASSFILES) : $(JAVAFILES) 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(JAVAFILES)

OOoBeanViewer: $(CLASSFILES)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) OOoBeanViewer.run
	@echo --------------------------------------------------------------------------------

%.run: $(CLASSFILES)
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(subst /,.,$(PACKAGE)).$(basename $@)

%.dbg: $(JAR1_JAR)
	jdb -classpath "$(SDK_CLASSPATH)" $(subst /,.,$(PACKAGE)).$(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))

