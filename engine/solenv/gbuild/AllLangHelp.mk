# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# class AllLangHelp

# Creates all language versions of a module.

gb_AllLangHelp_HELPDIRNAME := helpcontent2
gb_AllLangHelp_HELPDIR := $(gb_AllLangHelp_HELPDIRNAME)/source

gb_AllLangHelp__get_helpname = $(1)/$(2)

$(call gb_AllLangHelp_get_helpfiles_target,%): | \
        $(dir $(call gb_AllLangHelp_get_helpfiles_target,%)).dir
	touch $@

$(gb_AllLangHelp_targetdir)/%/.dir :
	mkdir -p $(@D) && touch $@

$(gb_AllLangHelp_targetdir)/.dir :
	mkdir -p $(@D) && touch $@

$(gb_AllLangHelp_targetdir)/% :
	touch $@

$(call gb_AllLangHelp_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(gb_AllLangHelp_targetdir)/$* $(call gb_AllLangHelp_get_helpfiles_target,$*) \
	)

# gb_AllLangHelp_AllLangHelp__one_lang module lang helpname
define gb_AllLangHelp_AllLangHelp__one_lang
$(call gb_HelpTarget_HelpTarget,$(3),$(2))

$(gb_AllLangHelp_targetdir)/$(1) : $(call gb_HelpTarget_get_target,$(3))
$(call gb_AllLangHelp_get_clean_target,$(1)) : $(call gb_HelpTarget_get_clean_target,$(3))

endef

# Create a help module for all languages.
#
# gb_AllLangHelp_AllLangHelp module
define gb_AllLangHelp_AllLangHelp
$(foreach lang,$(gb_HELP_LANGS),\
	$(call gb_AllLangHelp_AllLangHelp__one_lang,$(1),$(lang),$(call gb_AllLangHelp__get_helpname,$(1),$(lang))))

$(gb_AllLangHelp_targetdir)/$(1) :| $(dir $(gb_AllLangHelp_targetdir)/$(1)).dir

$$(eval $$(call gb_Module_register_target,$(gb_AllLangHelp_targetdir)/$(1),$(call gb_AllLangHelp_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),AllLangHelp,$(gb_AllLangHelp_targetdir)/$(1))

endef

# gb_AllLangHelp_set_treefile module treefile
define gb_AllLangHelp_set_treefile
$(foreach lang,$(gb_HELP_LANGS),\
	$(call gb_HelpTarget_set_treefile,$(call gb_AllLangHelp__get_helpname,$(1),$(lang)),$(2),$(gb_AllLangHelp_HELPDIR)/text) \
)

endef

# Add one or more help files.
#
# gb_AllLangHelp_add_helpfiles module file(s) [optional bookmark token]
define gb_AllLangHelp_add_helpfiles
$(foreach lang,$(gb_HELP_LANGS),\
	$(call gb_HelpTarget_add_helpfiles,$(call gb_AllLangHelp__get_helpname,$(1),$(lang)),$(2)) \
)
gb_AllLangHelp_$(1)_HELPFILES += $(addsuffix .xhp,$(2))
gb_AllLangHelp_$(or $(3),$(1))_BOOKMARK_HELPFILES += $(addsuffix .xhp,$(2))
$(call gb_AllLangHelp_get_helpfiles_target,$(1)): $(addprefix $(SRCDIR)/,$(addsuffix .xhp,$(2)))

endef

# gb_AllLangHelp__use_module module other-module lang
define gb_AllLangHelp__use_module
$(call gb_HelpTarget_use_module,$(call gb_AllLangHelp__get_helpname,$(1),$(3)),$(call gb_AllLangHelp__get_helpname,$(2),$(3)))

endef

# Use references from another help module's files.
#
# gb_AllLangHelp_use_module module other-module
define gb_AllLangHelp_use_module
$(foreach lang,$(gb_HELP_LANGS),$(call gb_AllLangHelp__use_module,$(1),$(2),$(lang)))

endef

# Use references from other help modules' files.
#
# gb_AllLangHelp_use_module module other-module(s)
define gb_AllLangHelp_use_modules
$(foreach module,$(2),$(call gb_AllLangHelp_use_module,$(1),$(module)))

endef

# vim: set noet sw=4 ts=4:
