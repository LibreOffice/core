#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# Builds the custom-tool-panel C++ component example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMP_NAME=custom-tool-panel
COMP_IMPL_NAME=$(COMP_NAME).uno.$(SHAREDLIB_EXT)
OUT_COMP_INC=$(OUT_INC)/$(COMP_NAME)
OUT_COMP_GEN=$(OUT_MISC)/$(COMP_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMP_NAME)
COMP_PACKAGE = $(OUT_BIN)/$(COMP_NAME).$(UNOOXT_EXT)
COMP_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOOXT_EXT)")
COMP_UNOPKG_MANIFEST = $(OUT_COMP_GEN)/$(COMP_NAME)/META-INF/manifest.xml
COMP_MAPFILE = $(OUT_COMP_GEN)/$(COMP_NAME).uno.map
COMP_COMPONENTS = $(OUT_COMP_GEN)/$(COMP_NAME).components
COMP_REGISTERFLAG = $(OUT_MISC)/cpp_$(COMP_NAME)_register_component.flag
COMP_TYPEFLAG = $(OUT_MISC)/cpp_$(COMP_NAME)_types.flag


# Allow deploying the extension when running it

#this should be in odk/settings/std.mk
OUT_DEPLOY=$(OUT)/deploy

OUT_COMP_DEPLOY=$(OUT_DEPLOY)/$(COMP_NAME)
OUT_DEPLOY_DIR=$(subst /,$(PS),$(OUT_COMP_DEPLOY))
OUT_DEPLOY_URL=$(URLPREFIX)$(subst \\,\,$(OUT_DEPLOY_DIR))
ENV_USER_INSTALL=-env:"UserInstallation=$(OUT_DEPLOY_URL)"
OFFICE_START_OPTIONS=-nologo -nofirststartwizard -norestore -writer
OXT_DEPLOYEDFLAG=$(OUT_MISC)/cpp_$(COMP_NAME)_deployed_oxt.flag


CXXFILES = \
           ctp_factory.cxx \
           ctp_panel.cxx \
           ctp_services.cxx

SLOFILES = $(patsubst %.cxx,$(OUT_COMP_SLO)/%.$(OBJ_EXT),$(CXXFILES))

# Add OSL_DEBUG_LEVEL to compiler the flags (for OSL_TRACE et. al.)
ifeq "$(DEBUG)" "yes"
CC_FLAGS += -DOSL_DEBUG_LEVEL=2
endif

# Targets
.PHONY: ALL
ALL : \
	CustomToolPanelExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(STL_INCLUDES) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<


#$(COMP_MAPFILE) : $(SLOFILES)
#	-$(MKDIR) $(subst /,$(PS),$(@D))
#	cat $(PRJ)/settings/component.uno.map > $(COMP_MAPFILE) 
#ifeq "$(OS)" "MACOSX"
#	nm -gx $(SLOFILES) | $(ADDSYMBOLS) >> $(COMP_MAPFILE)
#endif

ifeq "$(OS)" "WIN"
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
	$(LINK) $(COMP_LINK_FLAGS) /OUT:$@ \
	/MAP:$(OUT_COMP_GEN)/$(subst $(SHAREDLIB_EXT),map,$(@F)) $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) msvcrt.lib kernel32.lib
	$(LINK_MANIFEST)
else
#$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES) $(COMP_MAPFILE)
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(COMP_LINK_FLAGS) $(LINK_LIBS) -o $@ $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) $(STC++LIB) $(CPPUHELPERDYLIB) $(CPPUDYLIB) $(SALDYLIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS)  $@
endif
endif

# rule for component package manifest
$(OUT_COMP_GEN)/%/manifest.xml :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:meda-type="$(QM)application/vnd.sun.star.configuration-data$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)CalcWindowState.xcu$(QM)"/$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.configuration-data$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)DrawWindowState.xcu$(QM)"/$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.configuration-data$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)ImpressWindowState.xcu$(QM)"/$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.configuration-data$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)WriterWindowState.xcu$(QM)"/$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.configuration-data$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)Factories.xcu$(QM)"/$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-components;platform=$(UNOPKG_PLATFORM)$(QM)">> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(COMP_NAME).components$(QM)"/$(CSEP)>> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

$(COMP_COMPONENTS) :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)components xmlns="$(QM)http://openoffice.org/2010/uno-components$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)component loader="$(QM)com.sun.star.loader.SharedLibrary$(QM)" uri="$(QM)$(UNOPKG_PLATFORM)/$(COMP_IMPL_NAME)$(QM)"$(CSEP) >> $@
	@echo $(SQM)    $(SQM)$(OSEP)implementation name="$(QM)org.openoffice.comp.example.custompanel.ToolPanelFactory$(QM)"$(CSEP) >> $@
	@echo $(SQM)      $(SQM)$(OSEP)service name="$(QM)org.openoffice.example.colorpanel.ToolPanelFactory$(QM)"/$(CSEP) >> $@
	@echo $(SQM)    $(SQM)$(OSEP)/implementation$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)/component$(CSEP) >> $@
	@echo $(OSEP)/components$(CSEP) >> $@

# rule for component package file
$(COMP_PACKAGE) : $(SHAREDLIB_OUT)/$(COMP_IMPL_NAME) WriterWindowState.xcu CalcWindowState.xcu DrawWindowState.xcu ImpressWindowState.xcu Factories.xcu $(COMP_UNOPKG_MANIFEST) $(COMP_COMPONENTS)
	-$(MKDIR) $(subst /,$(PS),$(@D)) && $(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN)/$(UNOPKG_PLATFORM))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_COMP_GEN)/$(UNOPKG_PLATFORM))
	cd $(subst /,$(PS),$(OUT_COMP_GEN)) && $(SDK_ZIP) -u ../../bin/$(@F) $(COMP_NAME).components
	cd $(subst /,$(PS),$(OUT_COMP_GEN)) && $(SDK_ZIP) -u ../../bin/$(@F) $(UNOPKG_PLATFORM)/$(<F)
	$(SDK_ZIP) -u $@  WriterWindowState.xcu CalcWindowState.xcu DrawWindowState.xcu ImpressWindowState.xcu Factories.xcu panel.png
	cd $(subst /,$(PS),$(OUT_COMP_GEN)/$(subst .$(UNOOXT_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml


CustomToolPanelExample : $(COMP_PACKAGE)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to deploy the example!
	@echo -
	@echo $(MAKE) CustomToolPanelExample.run
	@echo -
	@echo --------------------------------------------------------------------------------


CustomToolPanelExample.run: $(OXT_DEPLOYEDFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(OFFICE_START_OPTIONS)  $(ENV_USER_INSTALL)

$(OXT_DEPLOYEDFLAG) : $(COMP_PACKAGE)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(OUT_DEPLOY))
	-$(MKDIR) $(subst /,$(PS),$(OUT_DEPLOY_DIR))
	$(DEPLOYTOOL) $(COMP_PACKAGE_URL) $(ENV_USER_INSTALL)
	@echo flagged > $(subst /,$(PS),$@)




.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_DEPLOY_DIR))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_PACKAGE_URL)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_REGISTERFLAG)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_TYPEFLAG)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OXT_DEPLOYEDFLAG)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(SHAREDLIB_OUT)/$(COMP_NAME).*))
