#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

# Builds the Java Calc Addin example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMP_NAME=ToDo
COMP_CLASS_OUT=$(OUT_CLASS)/$(COMP_NAME)
COMP_GEN_OUT=$(OUT_MISC)/$(COMP_NAME)
COMP_RDB_NAME=$(COMP_NAME).uno.rdb
COMP_RDB=$(COMP_GEN_OUT)/$(COMP_RDB_NAME)
COMP_PACKAGE=$(OUT_BIN)/$(COMP_NAME).$(UNOOXT_EXT)
COMP_PACKAGE_URL=$(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOOXT_EXT)")
COMP_JAR_NAME=$(COMP_NAME).uno.jar
COMP_JAR=$(COMP_CLASS_OUT)/$(COMP_JAR_NAME)
COMP_JAR_MANIFEST=$(COMP_CLASS_OUT)/$(COMP_NAME).uno.Manifest
COMP_UNOPKG_MANIFEST = $(COMP_GEN_OUT)/META-INF/manifest.xml
COMP_REGISTERFLAG=$(COMP_GEN_OUT)$(PS)java_$(COMP_NAME)_register_component.flag
COMP_COMPONENTS=$(COMP_NAME).components

IDLFILES = XToDo.idl \
	ToDo.idl

# normally the idl file should be stored in a directory tree fitting the module 
# structure, for the example we know the module structure
PACKAGE = org/openoffice

COMP_JAVAFILES  = ToDo.java
COMP_CLASSFILES = $(patsubst %.java,$(COMP_CLASS_OUT)/%.class,$(COMP_JAVAFILES))

GEN_CLASSFILES = $(patsubst %.idl,$(COMP_CLASS_OUT)/$(PACKAGE)/%.class,$(IDLFILES))
GEN_TYPELIST = $(subst /,.,$(patsubst %.idl,-T$(PACKAGE)/% ,$(IDLFILES)))
GEN_URDFILES = $(patsubst %.idl,$(COMP_GEN_OUT)/%.urd,$(IDLFILES))


$(COMP_NAME)_CLASSFILES = $(COMP_NAME).class \
		$(COMP_NAME)$(QUOTE)$$$(COMP_NAME)Impl.class

$(COMP_NAME)_CLASSFILES += $(subst $(COMP_CLASS_OUT)/,,$(GEN_CLASSFILES))

SDK_CLASSPATH = $(subst $(EMPTYSTRING) $(PATH_SEPARATOR),$(PATH_SEPARATOR),$(CLASSPATH)\
		$(PATH_SEPARATOR)$(COMP_CLASS_OUT))


# Targets
.PHONY: ALL
ALL : JavaToDoExample

include $(SETTINGS)/stdtarget.mk

$(COMP_CLASS_OUT)/%.Manifest :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo UNO-Type-Path: $(basename $*).uno.jar> $@
	@echo RegistrationClassName: $(basename $(basename $(@F)))>> $@

$(COMP_GEN_OUT)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(IDLC) -C -I. -I$(IDL_DIR) -O$(COMP_GEN_OUT) $<

$(COMP_GEN_OUT)/%.rdb : $(GEN_URDFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(REGMERGE) $@ /UCR $(GEN_URDFILES)

$(GEN_CLASSFILES) : $(COMP_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(JAVAMAKER) -BUCR -nD $(GEN_TYPELIST) -O$(COMP_CLASS_OUT) $(COMP_RDB) -X$(URE_TYPES) -X$(OFFICE_TYPES)

$(COMP_CLASSFILES) : $(COMP_JAVAFILES) $(GEN_CLASSFILES)
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
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-typelibrary;type=RDB$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(subst /META-INF,,$(subst $(OUT_MISC)/,,$(@D))).uno.rdb$(QM)"/$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-components$(QM)">> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(COMP_NAME).components$(QM)"/$(CSEP)>> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

# rule for component package file
$(COMP_PACKAGE) : $(COMP_RDB) $(COMP_JAR) $(COMP_UNOPKG_MANIFEST) $(COMP_NAME).components
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ $(COMP_NAME).components
	cd $(subst /,$(PS),$(COMP_GEN_OUT)) && $(SDK_ZIP) -u ../../bin/$(@F) $(COMP_RDB_NAME)
	cd $(subst /,$(PS),$(COMP_CLASS_OUT)) && $(SDK_ZIP) -u ../../bin/$(@F) $(COMP_JAR_NAME)
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

JavaToDoExample : $(COMP_REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The $(COMP_NAME) Java component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo Please load the "$(QM)ToDo.ods$(QM)" document to see how this component works, see
	@echo the example description.
	@echo -
	@echo $(MAKE) ToDo.ods.load
	@echo --------------------------------------------------------------------------------

ToDo.ods.load : $(COMP_REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(COMP_CLASS_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(COMP_GEN_OUT))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_PACKAGE_URL)))
