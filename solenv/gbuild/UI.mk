# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# class UILocalizeTarget

gb_UILocalizeTarget_WORKDIR := $(WORKDIR)/UILocalizeTarget

gb_UILocalizeTarget_TARGET := $(call gb_Executable_get_target_for_build,uiex)
gb_UILocalizeTarget_COMMAND := $(gb_Helper_set_ldpath) $(gb_UILocalizeTarget_TARGET)

define gb_UILocalizeTarget__command
$(call gb_Output_announce,$(2),$(true),UIX,1)
MERGEINPUT=`$(gb_MKTEMP)` && \
echo $(POFILES) > $${MERGEINPUT} && \
$(call gb_Helper_abbreviate_dirs,\
	$(gb_UILocalizeTarget_COMMAND) \
		-i $(UI_FILE) \
		-o $(1) \
		-l $(UI_LANG) \
		-m $${MERGEINPUT} ) && \
rm -rf $${MERGEINPUT}

endef

$(dir $(call gb_UILocalizeTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UILocalizeTarget_get_target,%) : $(gb_UILocalizeTarget_TARGET)
	$(call gb_UILocalizeTarget__command,$@,$*)

.PHONY : $(call gb_UILocalizeTarget_get_clean_target,%)
$(call gb_UILocalizeTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UIX,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UILocalizeTarget_get_target,$*) \
	)

# Translates one .ui file
#
# gb_UILocalizeTarget_UILocalizeTarget target source lang
define gb_UILocalizeTarget_UILocalizeTarget
$(call gb_UILocalizeTarget_get_target,$(1)) : POFILES := $(foreach lang,$(gb_UITarget_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(2))).po)
$(call gb_UILocalizeTarget_get_target,$(1)) : UI_FILE := $(SRCDIR)/$(2).ui
$(call gb_UILocalizeTarget_get_target,$(1)) : UI_LANG := $(3)

$(call gb_UILocalizeTarget_get_target,$(1)) : $(foreach lang,$(gb_UITarget_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(2))).po)
$(call gb_UILocalizeTarget_get_target,$(1)) : $$(UI_FILE)
$(call gb_UILocalizeTarget_get_target,$(1)) :| $(dir $(call gb_UILocalizeTarget_get_target,$(1))).dir

endef

# class UITarget

# en-US is a special case
gb_UITarget_LANGS := $(filter-out en-US,$(gb_WITH_LANG))

gb_UITarget_LOCALIZED_DIR := $(gb_UILocalizeTarget_WORKDIR)

gb_UITarget_get_uifile_for_lang = $(subst $(gb_UITarget_LOCALIZED_DIR)/,,$(call gb_UILocalizeTarget_get_target,$(1)/$(2)))

$(dir $(call gb_UITarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_UITarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UITarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),UI ,2)
	$(call gb_Helper_abbreviate_dirs,\
		touch $@ \
	)

.PHONY : $(call gb_UITarget_get_clean_target,%)
$(call gb_UITarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UI ,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UITarget_get_target,$*) \
	)

# Translates a set of .ui files
#
# gb_UITarget_UITarget modulename
define gb_UITarget_UITarget
$(call gb_UITarget_get_target,$(1)) :| $(dir $(call gb_UITarget_get_target,$(1))).dir

endef

define gb_UITarget__add_uifile_for_lang
$(call gb_UILocalizeTarget_UILocalizeTarget,$(2)/$(3),$(2),$(3))
$(call gb_UITarget_get_target,$(1)) : $(call gb_UILocalizeTarget_get_target,$(2)/$(3))
$(call gb_UITarget_get_clean_target,$(1)) : $(call gb_UILocalizeTarget_get_clean_target,$(2)/$(3))

endef

# Adds an .ui file for translation
#
# gb_UITarget_add_uifile target uifile
define gb_UITarget_add_uifile
$(foreach lang,$(gb_UITarget_LANGS),$(call gb_UITarget__add_uifile_for_lang,$(1),$(2),$(lang)))

endef

# Adds multiple .ui files for translation
#
# gb_UITarget_add_uifiles target uifile(s)
define gb_UITarget_add_uifiles
$(foreach uifile,$(2),$(call gb_UITarget_add_uifile,$(1),$(uifile)))

endef

# class UI

# Handles UI description files (suffix .ui) for the new layouting mechanism.

gb_UI_LANGS := $(gb_UITarget_LANGS)

$(dir $(call gb_UI_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_UI_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UI_get_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		touch $@ \
	)

.PHONY : $(call gb_UI_get_clean_target,%)
$(call gb_UI_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UI_get_target,$*) \
	)

# Translates and delivers a set of .ui files
#
# gb_UI_UI modulename
define gb_UI_UI
$(call gb_Package_Package_internal,$(1)_ui,$(SRCDIR))
$(call gb_UI_get_target,$(1)) :| $(dir $(call gb_UI_get_target,$(1))).dir
$(call gb_UI_get_target,$(1)) :| $(call gb_Package_get_target,$(1)_ui)
$(call gb_UI_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(1)_ui)

ifneq ($(gb_UI_LANGS),)
$(call gb_UITarget_UITarget,$(1))
$(call gb_Package_Package_internal,$(1)_ui_localized,$(gb_UITarget_LOCALIZED_DIR))
$(call gb_UI_get_target,$(1)) : $(call gb_UITarget_get_target,$(1))
$(call gb_UI_get_target,$(1)) :| $(call gb_Package_get_target,$(1)_ui_localized)
$(call gb_UI_get_clean_target,$(1)) : $(call gb_UITarget_get_clean_target,$(1))
$(call gb_UI_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(1)_ui_localized)
endif

$$(eval $$(call gb_Module_register_target,$(call gb_UI_get_target,$(1)),$(call gb_UI_get_clean_target,$(1))))
endef

# gb_UI__get_outdir_filename target file lang?
gb_UI__get_outdir_filename = xml/uiconfig/$(1)/ui/$(if $(3),res/$(3)/)$(notdir $(2)).ui

# gb_UI__add_uifile target package destfile srcfile lang?
define gb_UI__package_uifile
$(call gb_Package_add_file,$(2),$(call gb_UI__get_outdir_filename,$(1),$(3),$(5)),$(4))

endef

# gb_UI__add_uifile_direct target file
define gb_UI__add_uifile_direct
$(call gb_UI__package_uifile,$(1),$(1)_ui,$(2),$(2).ui)

endef

# gb_UI__add_uifile_for_lang target file lang
define gb_UI__add_uifile_for_lang
$(call gb_UI__package_uifile,$(1),$(1)_ui_localized,$(2),$(call gb_UITarget_get_uifile_for_lang,$(2),$(3)),$(3))

endef

define gb_UI__add_uifile
$(call gb_UI__add_uifile_direct,$(1),$(2))

ifneq ($(gb_UI_LANGS),)
$(call gb_UITarget_add_uifile,$(1),$(2))
$(foreach lang,$(gb_UI_LANGS),$(call gb_UI__add_uifile_for_lang,$(1),$(2),$(lang)))
endif

endef

# Adds .ui file to the package
#
# The file is relative to $(SRCDIR) and without extension.
#
# gb_UI_add_uifile target uifile
define gb_UI_add_uifile
$(call gb_UI__add_uifile,$(1),$(2))

ifneq ($(gb_UI_LANGS),)
$(call gb_UITarget_add_uifile,$(1),$(2))
endif

endef

# Adds multiple .ui files to the package
#
# gb_UI_add_uifiles target uifile(s)
define gb_UI_add_uifiles
$(foreach uifile,$(2),$(call gb_UI__add_uifile,$(1),$(uifile)))

ifneq ($(gb_UI_LANGS),)
$(call gb_UITarget_add_uifiles,$(1),$(2))
endif

endef

# vim: set noet sw=4 ts=4:
