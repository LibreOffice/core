#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.16 $
#
#  last change: $Author: obo $ $Date: 2003-10-20 13:13:42 $
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

# Builds the Java InstanceInspector component example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT1_NAME=InstanceInspector
OUT_COMP_CLASS=$(OUT_CLASS)/$(COMPONENT1_NAME)
OUT_COMP_GEN=$(OUT_MISC)/$(COMPONENT1_NAME)
COMPONENT1_RDB_NAME=$(COMPONENT1_NAME).uno.rdb
COMPONENT1_RDB=$(OUT_COMP_GEN)/$(COMPONENT1_RDB_NAME)
COMPONENT1_PACKAGE=$(OUT_BIN)/$(COMPONENT1_NAME).uno.zip
COMPONENT1_PACKAGE_URL=$(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT1_NAME).uno.zip")
COMPONENT1_JAR_NAME=$(COMPONENT1_NAME).uno.jar
COMPONENT1_JAR=$(OUT_CLASS)/$(COMPONENT1_JAR_NAME)
COMPONENT1_MANIFESTFILE=$(OUT_COMP_CLASS)/$(COMPONENT1_NAME).uno.Manifest

REGISTERFLAG=$(OUT_MISC)$(PS)java_$(COMPONENT1_NAME)_register_component.flag

IDLFILES = XInstanceInspector.idl
# differentiate between real type and service descriptions
IDLFILES2= InstanceInspector.idl

# normally the idl file should be stored in a directory tree fitting the module structure,
# for the example we know the module structure
PACKAGE = org/openoffice

JAVAFILES  = InstanceInspector.java \
	Test.java

GENJAVAFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/$(PACKAGE)/%.java,$(IDLFILES))
GENURDFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES))
GENURDFILES += $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES2))

CLASSFILES = $(patsubst %.java,$(OUT_COMP_CLASS)/%.class,$(JAVAFILES))
CLASSFILES += $(subst $(OUT_COMP_GEN),$(OUT_COMP_CLASS),$(GENJAVAFILES:.java=.class))

$(COMPONENT1_NAME)_CLASSFILES = $(COMPONENT1_NAME).class \
		$(COMPONENT1_NAME)$(QUOTE)$$_$(COMPONENT1_NAME).class \
		$(COMPONENT1_NAME)$(QUOTE)$$1.class \
		$(COMPONENT1_NAME)$(QUOTE)$$2.class \
		$(COMPONENT1_NAME)$(QUOTE)$$3.class \
		$(COMPONENT1_NAME)$(QUOTE)$$4.class

$(COMPONENT1_NAME)_CLASSFILES += $(subst $(OUT_COMP_GEN)/,,$(GENJAVAFILES:.java=.class))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(OFFICE_CLASSES_DIR)/jurt.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/unoil.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/ridl.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/sandbox.jar\
		$(PATH_SEPARATOR)$(OFFICE_CLASSES_DIR)/juh.jar\
		$(PATH_SEPARATOR)$(OUT_COMP_CLASS))


# Targets
.PHONY: ALL
ALL : JavaInstanceInspectorExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_CLASS)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo RegistrationClassName: $(basename $(basename $(@F)))> $@

$(OUT_COMP_GEN)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	idlc -C -I. -I$(IDL_DIR) -O$(OUT_COMP_GEN) $<

$(OUT_COMP_GEN)/%.rdb : $(GENURDFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	regmerge $@ /UCR $(GENURDFILES)

$(OUT_COMP_GEN)/%.java : $(COMPONENT1_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javamaker -BUCR -nD -T$(subst /,.,$(patsubst $(OUT_COMP_GEN)/%.java,%,$@)) -O$(OUT_COMP_GEN) $(COMPONENT1_RDB) $(DKREGISTRYNAME)

$(CLASSFILES) : $(JAVAFILES) $(GENJAVAFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	javac $(JAVAC_FLAGS) -classpath "$(SDK_CLASSPATH)" -d $(OUT_COMP_CLASS) $(GENJAVAFILES) $(JAVAFILES) 

$(OUT_CLASS)/%.jar : $(OUT_COMP_CLASS)/%.Manifest $(CLASSFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	cd $(subst /,$(PS),$(OUT_COMP_CLASS)) && jar cvfm ../$(@F) $(<F) $($(basename $(basename $(@F)))_CLASSFILES)

$(OUT_BIN)/%.zip : $(OUT_COMP_GEN)/%.rdb $(OUT_CLASS)/%.jar
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_CLASS))
	cd $(subst /,$(PS),$(OUT_CLASS)) && jar cvfM ../bin/$(@F) $(^F)
	$(DEL) $(subst /,$(PS),$(OUT_CLASS)/$(<F))

$(REGISTERFLAG) : $(COMPONENT1_PACKAGE)
ifneq "$(SDK_AUTO_DEPLOYMENT)" ""
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	$(DEPLOYTOOL) $(COMPONENT1_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

JavaInstanceInspectorExample : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo make Test.run
	@echo ------
	@echo The InstanceInspector component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description.
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_COMP_CLASS)/%.class
	java -classpath "$(SDK_CLASSPATH)" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_CLASS))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(COMPONENT1_JAR))
	-$(DEL) $(subst /,$(PS),$(REGISTERFLAG))
