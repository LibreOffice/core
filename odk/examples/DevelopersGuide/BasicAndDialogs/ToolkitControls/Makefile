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

# Builds the CreatingDialogs example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
SAMPLE_NAME=BaDToolkitControls
SAMPLE_GEN_OUT=$(OUT_MISC)/$(SAMPLE_NAME)

COMP_NAME=ToolkitControls
COMP_GEN_OUT=$(SAMPLE_GEN_OUT)/$(COMP_NAME)
COMP_PACKAGE = $(OUT_BIN)/$(COMP_NAME).$(UNOOXT_EXT)
COMP_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOOXT_EXT)")
COMP_UNOPKG_MANIFEST = $(COMP_GEN_OUT)/META-INF/manifest.xml
COMP_REGISTERFLAG=$(COMP_GEN_OUT)$(PS)devguide_basic_$(COMP_NAME)_register_component.flag

COMP_LIBRARY_FILES=\
	ToolkitControls/dialog.xlb\
	ToolkitControls/FileDialog.xba\
	ToolkitControls/FileDialogDlg.xdl\
	ToolkitControls/MultiPage.xba\
	ToolkitControls/MultiPageDlg.xdl\
	ToolkitControls/ProgressBar.xba\
	ToolkitControls/ProgressBarDlg.xdl\
	ToolkitControls/script.xlb\
	ToolkitControls/ScrollBar.xba\
	ToolkitControls/ScrollBarDlg.xdl

# Targets
.PHONY: ALL
ALL : $(SAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

# rule for component package manifest
$(COMP_GEN_OUT)/%/manifest.xml :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.basic-library$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(COMP_NAME)$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

# rule for component package file
$(COMP_PACKAGE) : $(COMP_UNOPKG_MANIFEST) $(COMP_LIBRARY_FILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(SDK_ZIP) $@ ToolkitControls/dialog.xlb \
	ToolkitControls/FileDialog.xba ToolkitControls/FileDialogDlg.xdl \
	ToolkitControls/MultiPage.xba ToolkitControls/MultiPageDlg.xdl \
	ToolkitControls/ProgressBar.xba ToolkitControls/ProgressBarDlg.xdl \
	ToolkitControls/script.xlb ToolkitControls/ScrollBar.xba \
	ToolkitControls/ScrollBarDlg.xdl 
	cd $(subst /,$(PS),$(COMP_GEN_OUT)) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml

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
	@echo The "$(QM)ToolkitControls$(QM)" library was installed in your office installation
	@echo if SDK_AUTO_DEPLOYMENT = YES. You can use it in the BASIC IDE as a normal
	@echo Basic library. 
	@echo You can also load the document "$(QM)$(COMP_NAME).odt$(QM)" and press a button 
	@echo to run one of the example macros.
	@echo -
	@echo $(MAKE) $(COMP_NAME).odt.load
	@echo --------------------------------------------------------------------------------

$(COMP_NAME).odt.load : $(COMP_REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(COMP_GEN_OUT))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_PACKAGE_URL)))
