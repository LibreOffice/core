# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# class UILocalizeTarget

# Produces translations for one .ui file.

gb_UILocalizeTarget_WORKDIR := $(WORKDIR)/UILocalizeTarget

gb_UILocalizeTarget_TARGET := $(call gb_Executable_get_target_for_build,uiex)
gb_UILocalizeTarget_COMMAND := $(gb_Helper_set_ld_path) $(gb_UILocalizeTarget_TARGET)
gb_UILocalizeTarget_LANGS := $(filter-out qtz,$(filter-out en-US,$(gb_WITH_LANG)))

define gb_UILocalizeTarget__command
$(call gb_Output_announce,$(2),$(true),UIX,1)
MERGEINPUT=`$(gb_MKTEMP)` && \
echo $(POFILES) > $${MERGEINPUT} && \
$(call gb_Helper_abbreviate_dirs,\
	$(gb_UILocalizeTarget_COMMAND) \
		-i $(UI_FILE) \
		-o $(call gb_UILocalizeTarget_get_workdir,$(2)) \
		-l all \
		-m $${MERGEINPUT} \
	&& touch $(1) \
) && \
rm -rf $${MERGEINPUT}
endef

$(dir $(call gb_UILocalizeTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

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

# Produce translations for one .ui file
#
# gb_UILocalizeTarget_UILocalizeTarget target
define gb_UILocalizeTarget_UILocalizeTarget
$(call gb_UILocalizeTarget__UILocalizeTarget_impl,$(1),$(foreach lang,$(gb_UILocalizeTarget_LANGS),$(gb_POLOCATION)/$(lang)/$(patsubst %/,%,$(dir $(1))).po))

endef

# gb_UILocalizeTarget__UILocalizeTarget_impl target pofiles
define gb_UILocalizeTarget__UILocalizeTarget_impl
$(call gb_UILocalizeTarget_get_target,$(1)) : POFILES := $(2)
$(call gb_UILocalizeTarget_get_target,$(1)) : UI_FILE := $(SRCDIR)/$(1).ui

$(call gb_UILocalizeTarget_get_target,$(1)) : $(2)
$(call gb_UILocalizeTarget_get_target,$(1)) : $(SRCDIR)/$(1).ui
$(call gb_UILocalizeTarget_get_target,$(1)) :| \
	$(dir $(call gb_UILocalizeTarget_get_target,$(1))).dir \
	$(call gb_UILocalizeTarget_get_workdir,$(1))/.dir

$(2) :

endef

# class UI

# Handles UI description files (suffix .ui) for the new layouting mechanism.

# en-US is the default, so there is no translation for it
gb_UI_LANGS := $(filter-out en-US,$(gb_WITH_LANG))

$(dir $(call gb_UI_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_UI_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_UI_get_target,%) :
	$(call gb_Output_announce,$*,$(true),UI ,2)
	$(call gb_Helper_abbreviate_dirs,\
		touch $@ \
	)

.PHONY : $(call gb_UI_get_clean_target,%)
$(call gb_UI_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UI ,2)
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
$(call gb_Package_Package_internal,$(1)_ui_localized,$(gb_UILocalizeTarget_WORKDIR))
$(call gb_UI_get_target,$(1)) :| $(call gb_Package_get_target,$(1)_ui_localized)
$(call gb_UI_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(1)_ui_localized)
endif

$$(eval $$(call gb_Module_register_target,$(call gb_UI_get_target,$(1)),$(call gb_UI_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),UI)

endef

# gb_UI__get_outdir_filename target file lang?
gb_UI__get_outdir_filename = xml/uiconfig/$(1)/ui/$(if $(3),res/$(3)/)$(notdir $(2))$(if $(3),,.ui)

# gb_UI__add_uifile target package destfile srcfile lang?
define gb_UI__package_uifile
$(call gb_Package_add_file,$(2),$(call gb_UI__get_outdir_filename,$(1),$(3),$(5)),$(4))

endef

# gb_UI__add_uifile target file
define gb_UI__add_uifile
$(call gb_UI__package_uifile,$(1),$(1)_ui,$(2),$(2).ui)

endef

# gb_UI__add_uifile_for_lang target file lang
define gb_UI__add_uifile_for_lang
$(call gb_UI__package_uifile,$(1),$(1)_ui_localized,$(2),$(2)/$(3).ui,$(3))

endef

# Adds .ui file to the package
#
# The file is relative to $(SRCDIR) and without extension.
#
# gb_UI_add_uifile target uifile
define gb_UI_add_uifile
$(call gb_UI__add_uifile,$(1),$(2))

ifneq ($(gb_UI_LANGS),)
$(call gb_UILocalizeTarget_UILocalizeTarget,$(2))
$(call gb_UI_get_target,$(1)) : $(call gb_UILocalizeTarget_get_target,$(2))
$(call gb_UI_get_clean_target,$(1)) : $(call gb_UILocalizeTarget_get_clean_target,$(2))
$(call gb_Package_get_preparation_target,$(1)_ui_localized) : $(call gb_UILocalizeTarget_get_target,$(2))
$(foreach lang,$(gb_UI_LANGS),$(call gb_UI__add_uifile_for_lang,$(1),$(2),$(lang)))
endif

endef

# Adds multiple .ui files to the package
#
# gb_UI_add_uifiles target uifile(s)
define gb_UI_add_uifiles
$(foreach uifile,$(2),$(call gb_UI_add_uifile,$(1),$(uifile)))

endef

# vim: set noet sw=4 ts=4:
