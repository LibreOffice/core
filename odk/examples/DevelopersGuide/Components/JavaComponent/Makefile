#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.3 $
#
#  last change: $Author: hr $ $Date: 2003-06-30 15:13:44 $
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

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings

COMPONENT_NAME=JavaComponent
OUT_COMP_CLASS = $(OUT_CLASS)/$(COMPONENT_NAME)
OUT_COMP_GEN = $(OUT_MISC)/$(COMPONENT_NAME)
COMPONENT_RDB_NAME = $(COMPONENT_NAME).uno.rdb
COMPONENT_RDB = $(OUT_COMP_GEN)/$(COMPONENT_RDB_NAME)
COMPONENT_PACKAGE = $(OUT_BIN)/$(COMPONENT_NAME).uno.zip
COMPONENT_PACKAGE_URL = $(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT_NAME).uno.zip")
COMPONENT_JAR_NAME = $(COMPONENT_NAME).uno.jar
COMPONENT_JAR = $(OUT_CLASS)/$(COMPONENT_JAR_NAME)
COMPONENT_MANIFESTFILE = $(OUT_COMP_CLASS)/$(COMPONENT_NAME).uno.Manifest

REGISTERFLAG = $(OUT_MISC)$(PS)devguide_$(COMPONENT_NAME)_register_component.flag

IDLFILES = XSomethingA.idl \
	 XSomethingB.idl

# normally the idl file should be stored in a directory tree fitting the module structure,
# for the example we know the module structure
PACKAGE = com/sun/star/test

JAVAFILES  = \
	   TestServiceProvider.java \
	   TestComponentA.java \
	   TestComponentB.java

JAVAFILES2 = RunComponent.java

GENJAVAFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/$(PACKAGE)/%.java,$(IDLFILES))
GENURDFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES))

CLASSFILES = $(subst $(OUT_COMP_GEN),$(OUT_COMP_CLASS),$(GENJAVAFILES:.java=.class))
CLASSFILES += $(patsubst %.java,$(OUT_COMP_CLASS)/JavaComp/%.class,$(JAVAFILES))
CLASSFILES += $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES2))

$(COMPONENT_NAME)_CLASSFILES = JavaComp/TestServiceProvider.class\
		      JavaComp/TestComponentA.class \
		      JavaComp/TestComponentB.class

$(COMPONENT_NAME)_CLASSFILES += $(subst $(OUT_COMP_GEN)/,,$(GENJAVAFILES:.java=.class))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : \
	JavaComponentExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: JavaComp.TestServiceProvider> $@

$(OUT_COMP_GEN)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	idlc -I. -I$(IDL_DIR) -O$(OUT_COMP_GEN) $^

$(OUT_COMP_GEN)/%.rdb : $(GENURDFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	regmerge $@ /UCR $(GENURDFILES)

$(OUT_COMP_GEN)/%.java : $(COMPONENT_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javamaker -BUCR -nD -T$(subst /,.,$(patsubst $(OUT_COMP_GEN)/%.java,%,$@)) -O$(OUT_COMP_GEN) $(COMPONENT_RDB) $(DKREGISTRYNAME)

$(CLASSFILES) : $(GENJAVAFILES) $(JAVAFILES) $(JAVAFILES2) 
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(GENJAVAFILES) $(JAVAFILES) $(JAVAFILES2) 

$(OUT_CLASS)/%.jar : $(OUT_COMP_CLASS)/%.Manifest $(CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(OUT_COMP_CLASS) && jar cvfm ../$(@F) $(<F) $($(basename $(basename $(@F)))_CLASSFILES)

$(OUT_BIN)/%.zip : $(OUT_COMP_GEN)/%.rdb $(OUT_CLASS)/%.jar
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_CLASS))
	cd $(OUT_CLASS) && jar cvfM ../bin/$(@F) $(^F)
	$(DEL) $(subst /,$(PS),$(OUT_CLASS)/$(<F))

$(REGISTERFLAG) : $(COMPONENT_PACKAGE)
ifneq "$(SDK_AUTO_DEPLOYMENT)" ""
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

JavaComponentExample : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo make RunComponent.run
	@echo make RunComponent.local
	@echo --------
	@echo The Java components were installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

%.local: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@) local

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_JAR))
	-$(DEL) $(subst /,$(PS),$(REGISTERFLAG))
