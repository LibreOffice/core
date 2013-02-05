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

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk
include $(SETTINGS)/platform.mk

include project.mk

COMP_NAME=SelectionModeController

OUT_COMP_INC=$(OUT_INC)/$(COMP_NAME)
OUT_COMP_GEN=$(OUT_MISC)/$(COMP_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMP_NAME)
OUT_COMP_LIB=$(SHAREDLIB_OUT)/$(COMP_NAME)
OUT_COMP_BIN=$(OUT_BIN)/$(COMP_NAME)
OUT_COMP_DEPLOY=$(OUT_DEPLOY)/$(COMP_NAME)

COMP_IMPL_NAME=$(COMP_NAME).uno.$(SHAREDLIB_EXT)
COMP_PACKAGE_FILENAME=$(COMP_NAME)-$(EXTENSION_VERSION)-$(subst _,-,$(EXTENSION_PLATFORM)).$(UNOOXT_EXT)
COMP_PACKAGE = $(OUT_COMP_BIN)/$(COMP_PACKAGE_FILENAME)
COMP_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE)")
COMP_UNOPKG_MANIFEST    = $(OUT_COMP_GEN)/$(COMPONENT_NAME)/META-INF/manifest.xml
COMP_UNOPKG_DESCRIPTION = $(OUT_COMP_GEN)/$(COMPONENT_NAME)/description.xml
COMP_MAPFILE = $(OUT_COMP_GEN)/$(COMP_NAME).uno.map
COMP_COMPONENTS = $(OUT_COMP_GEN)/$(COMP_NAME).components

# xcu
COMP_ADDONS_CFG=registry/data/org/openoffice/Office/Addons.xcu
COMP_CONTROLLER_CFG=registry/data/org/openoffice/Office/UI/Controller.xcu

DESCRIPTION_IDENTIFIER=$(EXTENSION_ID)
DESCRIPTION_PLATFORM=$(TARGET_PLATFORM)
DESCRIPTION_UPDATE_FILENAME=$(EXTENSION_PLATFORM)-update.xml

# Allow deploying the extension when running it
ENV_USER_INSTALL=-env:"UserInstallation=$(URLPREFIX)$(OUT_COMP_DEPLOY)"
OFFICE_START_OPTIONS=-nologo -nofirststartwizard -norestore -quickstart=no -writer
OXT_DEPLOYEDFLAG=$(OUT_COMP_GEN)/so_deployed_$(COMP_NAME).flag

# Files
COMP_CXXFILES = \
	StatusbarController.cxx \
	SelectionModeStatusbarController.cxx \
	compfactory.cxx

COMP_SLOFILES = $(patsubst %.cxx,$(OUT_COMP_SLO)/%.$(OBJ_EXT),$(COMP_CXXFILES))

CC_DEFINES+= \
	-DSTATUSBAR_CONTROLLER_IMPL_NAME=\"$(STATUSBARCONTROLLER_IMPLNAME)\" \
	-DSTATUSBAR_CONTROLLER_SERVICE_NAME=\"$(STATUSBARCONTROLLER_SERVICE_NAME)\"

# Targets and rules
.PHONY: ALL update-desc SelectionModeController SelectionModeController.run
ALL : \
	update-desc \
	SelectionModeController

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(STL_INCLUDES) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<


ifeq "$(OS)" "WIN"
$(OUT_COMP_LIB)/%.$(SHAREDLIB_EXT) : $(COMP_SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
	$(LINK) \
		$(COMP_LINK_FLAGS) \
		/OUT:$@ \
		/MAP:$(OUT_COMP_GEN)/$(subst $(SHAREDLIB_EXT),map,$(@F)) \
		$(COMP_SLOFILES) \
		$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) \
		msvcrt.lib kernel32.lib
	$(LINK_MANIFEST)
else
$(OUT_COMP_LIB)/%.$(SHAREDLIB_EXT) : $(COMP_SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(COMP_LINK_FLAGS) $(LINK_LIBS) -o $@ $(COMP_SLOFILES) \
		$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) \
		$(STLPORTLIB) $(STC++LIB) $(CPPUHELPERDYLIB) $(CPPUDYLIB) $(SALDYLIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS)  $@
endif
endif


# rule for extension Addons.xcu
$(OUT_COMP_GEN)/$(COMP_ADDONS_CFG) : $(COMP_ADDONS_CFG)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_CAT) $< | $(SDK_SED) \
	-e "s/#PROTOCOLHANDLER_PROTOCOL#/$(PROTOCOLHANDLER_PROTOCOL)/" > $@


# rule for extension Addons.xcu
$(OUT_COMP_GEN)/$(COMP_CONTROLLER_CFG) : $(COMP_CONTROLLER_CFG)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_CAT) $< | $(SDK_SED) \
	-e "s/#PROTOCOLHANDLER_PROTOCOL#/$(PROTOCOLHANDLER_PROTOCOL)/" \
	-e "s/#STATUSBARCONTROLLER_IMPLNAME#/$(STATUSBARCONTROLLER_IMPLNAME)/" > $@

# rule for extension description.xml
$(COMP_UNOPKG_DESCRIPTION) :  xml/description.xml
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_CAT) $< | $(SDK_SED) \
	-e "s/#EXTENSION_VERSION#/$(EXTENSION_VERSION)/" \
	-e "s/#DESCRIPTION_IDENTIFIER#/$(DESCRIPTION_IDENTIFIER)/" \
	-e "s/#EXTENSION_PLATFORM#/$(EXTENSION_PLATFORM)/" \
	-e "s/#DESCRIPTION_OOO_MIN_VER#/$(DESCRIPTION_OOO_MIN_VER)/" \
	-e "s/#DESCRIPTION_OOO_MAX_VER#/$(DESCRIPTION_OOO_MAX_VER)/" \
	-e "s/#DESCRIPTION_DEP_NAME#/$(DESCRIPTION_DEP_NAME)/" \
	-e "s/#DESCRIPTION_PUBLISHER_URL#/$(subst /,\/,$(DESCRIPTION_PUBLISHER_URL))/" \
	-e "s/#DESCRIPTION_PUBLISHER_NAME#/$(DESCRIPTION_PUBLISHER_NAME)/" \
	-e "s/#DESCRIPTION_DISPLAY_NAME#/$(DESCRIPTION_DISPLAY_NAME)/" \
	-e "s/#DESCRIPTION_UPDATE#/$(subst /,\/,$(DESCRIPTION_UPDATE_BASEURL)$(DESCRIPTION_UPDATE_FILENAME))/"  > $@


# rule for update description
$(OUT_COMP_BIN)/$(DESCRIPTION_UPDATE_FILENAME) : xml/update.xml
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_CAT) $< | $(SDK_SED) \
		-e "s/#DESCRIPTION_IDENTIFIER#/$(DESCRIPTION_IDENTIFIER)/" \
		-e "s/#EXTENSION_VERSION#/$(EXTENSION_VERSION)/" \
		-e "s/#DESCRIPTION_PUBLISHER_URL#/$(subst /,\/,$(DESCRIPTION_PUBLISHER_URL))/" \
		-e "s/#DESCRIPTION_PUBLISHER_NAME#/$(DESCRIPTION_PUBLISHER_NAME)/" \
		-e "s/#UPDATE_DOWNLOAD#/$(subst /,\/,$(UPDATE_DOWNLOAD_BASE)$(COMP_PACKAGE_FILENAME))/" \
		-e "s/#RELEASE_NOTES#/$(subst /,\/,$(RELEASE_NOTES))/" > $@

# rule for component package manifest
$(COMP_UNOPKG_MANIFEST) : xml/manifest.xml
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_CAT) $< | $(SDK_SED) \
		-e "s/#COMP_NAME#/$(COMP_NAME)/" \
		-e "s/#UNOPKG_PLATFORM#/$(UNOPKG_PLATFORM)/" > $@

# rule for component pasive registration
$(COMP_COMPONENTS) : xml/components.xml
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_CAT) $< | $(SDK_SED) \
		-e "s/#STATUSBARCONTROLLER_IMPLNAME#/$(STATUSBARCONTROLLER_IMPLNAME)/" \
		-e "s/#UNOPKG_PLATFORM#/$(UNOPKG_PLATFORM)/" \
		-e "s/#COMP_IMPL_NAME#/$(COMP_IMPL_NAME)/" > $@

$(OUT_COMP_GEN)/images/%.png : images/%.png
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$@)

$(OUT_COMP_GEN)/description/%.txt : description/%.txt
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$@)

# rule for component package file
$(COMP_PACKAGE) : $(OUT_COMP_LIB)/$(COMP_IMPL_NAME) \
			$(OUT_COMP_GEN)/$(COMP_ADDONS_CFG)  \
			$(OUT_COMP_GEN)/$(COMP_CONTROLLER_CFG)  \
			$(COMP_UNOPKG_MANIFEST) \
			$(COMP_COMPONENTS)  \
			$(COMP_UNOPKG_DESCRIPTION) \
			LICENSE \
			$(OUT_COMP_GEN)/images/icon.png \
			$(OUT_COMP_GEN)/images/icon_hc.png \
			$(OUT_COMP_GEN)/description/description_en-US.txt
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN)/$(UNOPKG_PLATFORM))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_COMP_GEN)/$(UNOPKG_PLATFORM))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN)/LICENSE)
	$(COPY) $(subst /,$(PS),LICENSE) $(subst /,$(PS),$(OUT_COMP_GEN)/LICENSE/)
	cd $(subst /,$(PS),$(OUT_COMP_GEN)) && $(SDK_ZIP) -u $(COMP_PACKAGE) $(COMP_NAME).components description.xml
	cd $(subst /,$(PS),$(OUT_COMP_GEN)) && $(SDK_ZIP) -ur $(COMP_PACKAGE) $(UNOPKG_PLATFORM)/ LICENSE/ META-INF/  registry/ images/ description/


update-desc : $(OUT_COMP_BIN)/$(DESCRIPTION_UPDATE_FILENAME)

SelectionModeController : $(COMP_PACKAGE)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to deploy the example!
	@echo -
	@echo $(MAKE) SelectionModeController.run
	@echo -
	@echo --------------------------------------------------------------------------------

SelectionModeController.run: $(OXT_DEPLOYEDFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(OFFICE_START_OPTIONS)  $(ENV_USER_INSTALL)

$(OXT_DEPLOYEDFLAG) : $(COMP_PACKAGE)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_DEPLOY))
	$(DEPLOYTOOL) $(COMP_PACKAGE_URL) $(ENV_USER_INSTALL)
	@echo flagged > $(subst /,$(PS),$@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_LIB))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_BIN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_DEPLOY))
