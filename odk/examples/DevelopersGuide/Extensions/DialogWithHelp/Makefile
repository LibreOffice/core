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

# Builds the DialogWithHelp example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
SAMPLE_NAME=ExtDialogWithHelp
SAMPLE_GEN_OUT=$(OUT_MISC)/$(SAMPLE_NAME)

COMP_NAME=DialogWithHelp
COMP_GEN_OUT=$(SAMPLE_GEN_OUT)/$(COMP_NAME)
COMP_PACKAGE = $(OUT_BIN)/$(COMP_NAME).$(UNOOXT_EXT)
COMP_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOOXT_EXT)")
COMP_UNOPKG_MANIFEST = $(COMP_GEN_OUT)/META-INF/manifest.xml
COMP_REGISTERFLAG=$(COMP_GEN_OUT)$(PS)devguide_basic_$(COMP_NAME)_register_component.flag

COMP_LIBRARY_FILES=\
	DialogWithHelp/dialog.xlb\
	DialogWithHelp/script.xlb\
	DialogWithHelp/Module1.xba\
	DialogWithHelp/Dialog1.xdl

COMP_MISC_FILES=\
	META-INF/manifest.xml\
	Addons.xcu\
	description.xml

COMP_HELP_FILES=\
	help/de/com.foocorp.foo-ext/page1.xhp\
	help/de/com.foocorp.foo-ext/page2.xhp\
	help/de/com.foocorp.foo-ext/subfolder/anotherpage.xhp\
	help/en/com.foocorp.foo-ext/page1.xhp\
	help/en/com.foocorp.foo-ext/page2.xhp\
	help/en/com.foocorp.foo-ext/subfolder/anotherpage.xhp\

# Targets
.PHONY: ALL
ALL : $(SAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

# rule for component package file
$(COMP_PACKAGE) : $(COMP_LIBRARY_FILES) $(COMP_MISC_FILES) $(COMP_HELP_FILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ $(COMP_LIBRARY_FILES)
	$(SDK_ZIP) $@ $(COMP_MISC_FILES)
	$(SDK_ZIP) $@ $(COMP_HELP_FILES)

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

$(SAMPLE_NAME) : $(COMP_REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The "$(QM)DialogWithHelp$(QM)" library was installed in your office installation
	@echo if SDK_AUTO_DEPLOYMENT = YES. You can use it in the BASIC IDE as a normal
	@echo Basic library. You also will find a new top level menu "$(QM)Foo Extension$(QM)"
	@echo with one menu entry "$(QM)Start Foo Dialog$(QM)" to start it directly. If a
	@echo document is active there is also a "$(QM)Foo Toolbar Item$(QM)" doing the same.
	@echo All help index entries provided by the help content start with Foo.
	@echo --------------------------------------------------------------------------------

$(COMP_NAME).odt.load : $(COMP_REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(COMP_GEN_OUT))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_PACKAGE_URL)))
