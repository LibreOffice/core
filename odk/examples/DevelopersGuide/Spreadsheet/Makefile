#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.5 $
#
#  last change: $Author: rt $ $Date: 2004-05-18 13:28:30 $
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

# Builds the SpreadSheet examples of the Developers Guide.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
OUT_COMP_CLASS = $(OUT_CLASS)/SpreadsheetExamples
OUT_COMP_GEN = $(OUT_MISC)/SpreadsheetExamples

COMPONENT_NAME=ExampleAddIn
COMPONENT_RDB_NAME = $(COMPONENT_NAME).uno.rdb
COMPONENT_RDB = $(OUT_COMP_GEN)/$(COMPONENT_RDB_NAME)
COMPONENT_PACKAGE = $(OUT_BIN)/$(COMPONENT_NAME).uno.zip
COMPONENT_PACKAGE_URL = $(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT_NAME).uno.zip")
COMPONENT_JAR_NAME = $(COMPONENT_NAME).uno.jar
COMPONENT_JAR = $(OUT_CLASS)/$(COMPONENT_JAR_NAME)
COMPONENT_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMPONENT_NAME).uno.Manifest

REGISTERFLAG = $(OUT_MISC)$(PS)devguide_$(COMPONENT_NAME)_register_component.flag

IDLFILES = XExampleAddIn.idl

# normally the idl file should be stored in a directory tree fitting the module structure,
# for the example we know the module structure
PACKAGE = com/sun/star/sheet/addin

JAVAFILES  = \
	   GeneralTableSample.java	\
	   SpreadsheetSample.java	\
	   SpreadsheetDocHelper.java	\
	   ViewSample.java	\
	   ExampleDataPilotSource.java		\
	   ExampleAddIn.java

GENJAVAFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/$(PACKAGE)/%.java,$(IDLFILES))
GENURDFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES))

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))
CLASSFILES += $(subst $(OUT_COMP_GEN),$(OUT_COMP_CLASS),$(GENJAVAFILES:.java=.class))

$(COMPONENT_NAME)_CLASSFILES = $(COMPONENT_NAME).class \
		$(COMPONENT_NAME)$(QUOTE)$$_$(COMPONENT_NAME).class \
		$(COMPONENT_NAME)Result.class \
		$(COMPONENT_NAME)Thread.class

$(COMPONENT_NAME)_CLASSFILES += $(subst $(OUT_COMP_GEN)/,,$(GENJAVAFILES:.java=.class))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : SpreadSheetExamples

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: $(basename $(basename $(@F)))> $@

$(OUT_COMP_GEN)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(IDLC) -I. -I$(IDL_DIR) -O$(OUT_COMP_GEN) $^

$(OUT_COMP_GEN)/%.rdb : $(GENURDFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	$(REGMERGE) $@ /UCR $(GENURDFILES)

$(OUT_COMP_GEN)/%.java : $(COMPONENT_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(JAVAMAKER) -BUCR -nD -T$(subst /,.,$(patsubst $(OUT_COMP_GEN)/%.java,%,$@)) -O$(OUT_COMP_GEN) $(COMPONENT_RDB) $(DKREGISTRYNAME)

$(CLASSFILES) : $(JAVAFILES) $(GENJAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_JAVAC) $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(GENJAVAFILES) $(JAVAFILES) 

$(COMPONENT_JAR) : $(COMPONENT_MANIFESTFILE) $(CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && $(SDK_JAR) cvfm ../$(@F) $(<F) $($(basename $(basename $(@F)))_CLASSFILES)

$(COMPONENT_PACKAGE) : $(COMPONENT_RDB) $(COMPONENT_JAR) 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_CLASS))
	cd $(subst /,$(PS),$(OUT_CLASS)) && $(SDK_JAR) cvfM ../bin/$(@F) $(^F)
	$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_CLASS)/$(<F)))

$(REGISTERFLAG) : $(COMPONENT_PACKAGE)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	$(DEPLOYTOOL) $(COMPONENT_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

SpreadSheetExamples : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo make GeneralTableSample.run
	@echo make SpreadsheetSample.run
	@echo make ViewSample.run
	@echo --------
	@echo The Calc add-in component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description. You can also load the "$(QM)ExampleAddIn.sxc$(QM)" document to
	@echo see how the function can be used.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	$(SDK_JAVA) -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMPONENT_PACKAGE_URL)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMPONENT_JAR)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(REGISTERFLAG)))
