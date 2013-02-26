# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

gb_AllLangHelp_HELPDIRNAME := helpcontent2
gb_AllLangHelp_AUXDIRNAME := auxiliary
gb_AllLangHelp_HELPDIR := $(gb_AllLangHelp_HELPDIRNAME)/source
gb_AllLangHelp_AUXDIR := $(gb_AllLangHelp_HELPDIR)/$(gb_AllLangHelp_AUXDIRNAME)
gb_AllLangHelp_TRANLATIONSDIR := $(SRCDIR)/translations

# class AllLangHelp

# Creates and delivers all language versions of a module.

gb_AllLangHelp_LANGS := $(gb_WITH_LANG)

define gb_AllLangHelp__translation_exists
$(or \
	$(filter en-US,$(1)),\
	$(and \
		$(wildcard $(SRCDIR)/$(gb_AllLangHelp_AUXDIR)/$(1)),\
		$(wildcard $(gb_AllLangHelp_TRANLATIONSDIR)/source/$(1)/$(gb_AllLangHelp_HELPDIRNAME)) \
	) \
)
endef

gb_AllLangHelp__get_helpname = $(1)/$(2)
gb_AllLangHelp__get_zipname = $(1)_$(2)

$(dir $(call gb_AllLangHelp_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_AllLangHelp_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_AllLangHelp_get_target,%) :
	$(call gb_Output_announce,$*,$(true),ALH,5)
	touch $@

$(call gb_AllLangHelp_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),ALH,5)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_AllLangHelp_get_target,$*) $(HELP_DELIVERABLES) \
	)

# gb_AllLangHelp_AllLangHelp__one_lang module lang helpname zipname
define gb_AllLangHelp_AllLangHelp__one_lang
$(call gb_HelpTarget_HelpTarget,$(3),$(1),$(2))
$(call gb_HelpTarget_set_configdir,$(3),$(gb_AllLangHelp_AUXDIR))
$(call gb_HelpTarget_set_helpdir,$(3),$(gb_AllLangHelp_HELPDIR))

$(call gb_HelpTarget_get_outdir_target,$(4)) : $(call gb_HelpTarget_get_zipfile,$(3))
$(call gb_AllLangHelp_get_target,$(1)) : $(call gb_HelpTarget_get_outdir_target,$(4))
$(call gb_AllLangHelp_get_target,$(1)) : $(call gb_HelpTarget_get_target,$(3))
$(call gb_AllLangHelp_get_clean_target,$(1)) : $(call gb_HelpTarget_get_clean_target,$(3))
$(call gb_AllLangHelp_get_clean_target,$(1)) : HELP_DELIVERABLES += $(call gb_HelpTarget_get_outdir_target,$(4))

$(call gb_Deliver_add_deliverable,$(call gb_HelpTarget_get_outdir_target,$(4)),$(call gb_HelpTarget_get_zipfile,$(3)),$(1))

endef

# Create and deliver help packs for a module for all languages.
#
# gb_AllLangHelp_AllLangHelp module
define gb_AllLangHelp_AllLangHelp
$(call gb_AllLangHelp_get_clean_target,$(1)) : HELP_DELIVERABLES :=

$(foreach lang,$(gb_AllLangHelp_LANGS),\
	$(if $(call gb_AllLangHelp__translation_exists,$(lang)),\
		$(call gb_AllLangHelp_AllLangHelp__one_lang,$(1),$(lang),$(call gb_AllLangHelp__get_helpname,$(1),$(lang)),$(call gb_AllLangHelp__get_zipname,$(1),$(lang)))))

$(call gb_AllLangHelp_get_target,$(1)) :| $(dir $(call gb_AllLangHelp_get_target,$(1))).dir

$$(eval $$(call gb_Module_register_target,$(call gb_AllLangHelp_get_target,$(1)),$(call gb_AllLangHelp_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),AllLangHelp)

endef

# gb_AllLangHelp_set_treefile module treefile
define gb_AllLangHelp_set_treefile
$(foreach lang,$(gb_AllLangHelp_LANGS),$(call gb_HelpTarget_set_treefile,$(call gb_AllLangHelp__get_helpname,$(1),$(lang)),$(2),$(gb_AllLangHelp_HELPDIR)/text))

endef

# Add a help file.
#
# gb_AllLangHelp_add_helpfile module file
define gb_AllLangHelp_add_helpfile
$(foreach lang,$(gb_AllLangHelp_LANGS),$(call gb_HelpTarget_add_helpfile,$(call gb_AllLangHelp__get_helpname,$(1),$(lang)),$(2)))

endef

# Add one or more help files.
#
# gb_AllLangHelp_add_helpfiles module file(s)
define gb_AllLangHelp_add_helpfiles
$(foreach lang,$(gb_AllLangHelp_LANGS),$(call gb_HelpTarget_add_helpfiles,$(call gb_AllLangHelp__get_helpname,$(1),$(lang)),$(2)))

endef

# Add additional localized file to the help pack.
#
# gb_AllLangHelp_add_localized_file module basedir file
define gb_AllLangHelp_add_localized_file
$(foreach lang,$(gb_AllLangHelp_LANGS),$(call gb_HelpTarget_add_file,$(call gb_AllLangHelp__get_helpname,$(1),$(lang)),$(2)/$(lang)/$(3)))

endef

# Add additional localized file(s) to the help pack.
#
# gb_AllLangHelp_add_localized_files module basedir file(s)
define gb_AllLangHelp_add_localized_files
$(foreach lang,$(gb_AllLangHelp_LANGS),$(call gb_HelpTarget_add_files,$(call gb_AllLangHelp__get_helpname,$(1),$(lang)),$(addprefix $(2)/$(lang)/,$(3))))

endef

# gb_AllLangHelp__use_module module other-module lang
define gb_AllLangHelp__use_module
$(call gb_HelpTarget_use_module,$(call gb_AllLangHelp__get_helpname,$(1),$(3)),$(call gb_AllLangHelp__get_helpname,$(2),$(3)))

endef

# Use references from another help module's files.
#
# gb_AllLangHelp_use_module module other-module
define gb_AllLangHelp_use_module
$(foreach lang,$(gb_AllLangHelp_LANGS),$(call gb_AllLangHelp__use_module,$(1),$(2),$(lang)))

endef

# Use references from other help modules' files.
#
# gb_AllLangHelp_use_module module other-module(s)
define gb_AllLangHelp_use_modules
$(foreach module,$(2),$(call gb_AllLangHelp_use_module,$(1),$(module)))

endef

# gb_AllLangHelp__use_linked_module module other-module lang
define gb_AllLangHelp__use_linked_module
$(call gb_HelpTarget_use_linked_module,$(call gb_AllLangHelp__get_helpname,$(1),$(3)),$(call gb_AllLangHelp__get_helpname,$(2),$(3)))

endef

# Link with another help module.
#
# gb_AllLangHelp_use_linked_module module other-module
define gb_AllLangHelp_use_linked_module
$(foreach lang,$(gb_AllLangHelp_LANGS),$(call gb_AllLangHelp__use_linked_module,$(1),$(2),$(lang)))

endef

# Link with other help module(s).
#
# gb_AllLangHelp_use_linked_module module other-module(s)
define gb_AllLangHelp_use_linked_modules
$(foreach module,$(2),$(call gb_AllLangHelp_use_linked_module,$(1),$(module)))

endef

# vim: set noet sw=4 ts=4:
