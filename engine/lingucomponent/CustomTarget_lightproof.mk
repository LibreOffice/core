# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,lingucomponent/lightproof))

lightproof_DIR := $(gb_CustomTarget_workdir)/lingucomponent/lightproof
lightproof_COMPILER := $(SRCDIR)/lingucomponent/source/spellcheck/lightproof/tools/lpcompile.py
lightproof_PYTHON := $(call gb_ExternalExecutable_get_command,python)

# The rules are compiled here rather than with the dictionaries, because a
# build can be configured without the bundled dictionaries and still wants
# sentence checking: there is no system package to fall back on for it, the
# way there is for spelling. The rule tables are read from the source tree,
# which is there whether that module is built or not.
#
# The package directory under dictionaries/ is not always the package name,
# so map the one to the other here.
lightproof_DIR_en := en
lightproof_DIR_hu_HU := hu_HU
lightproof_DIR_ru_RU := ru_RU

define lightproof_rules
$(lightproof_DIR)/$(1).lpr : \
		$(lightproof_COMPILER) \
		$(SRCDIR)/dictionaries/$(lightproof_DIR_$(1))/pythonpath/lightproof_$(1).py \
		$(SRCDIR)/dictionaries/$(lightproof_DIR_$(1))/pythonpath/lightproof_impl_$(1).py \
		$(SRCDIR)/dictionaries/$(lightproof_DIR_$(1))/pythonpath/lightproof_opts_$(1).py \
		| $(lightproof_DIR)/.dir \
		$(call gb_ExternalExecutable_get_dependencies,python)
	$$(call gb_Output_announce,$(1).lpr,$(true),LPR,1)
	$$(call gb_Helper_abbreviate_dirs, \
		$(lightproof_PYTHON) $(lightproof_COMPILER) \
			$(SRCDIR)/dictionaries/$(lightproof_DIR_$(1)) $(1) $$@)

endef

$(eval $(call lightproof_rules,en))
$(eval $(call lightproof_rules,hu_HU))
$(eval $(call lightproof_rules,ru_RU))

# vim: set noet sw=4 ts=4:
