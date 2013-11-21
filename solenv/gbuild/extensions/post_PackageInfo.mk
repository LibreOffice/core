# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

gb_PackageInfo_InstallModules := \
	base \
	calc \
	graphicsfilter \
	tde \
	impress \
	onlineupdate \
	gnome \
	kde \
	math \
	ooo \
	writer \
	ure \
	activexbinarytable \
	ooobinarytable \
	winexplorerextbinarytable \

define gb_PackageInfo_emit_binaries_command
@touch $(foreach suf,executables libraries files,$(gb_PackageInfo_get_target)/$(1).$(suf))
@$(foreach executable,$(gb_Executable_MODULE_$(1)),echo "$(patsubst $(INSTDIR)/%,%,$(call gb_Executable_get_target,$(executable)))" >> $(gb_PackageInfo_get_target)/$(1).executables &&) true
@$(foreach library,$(gb_Library_MODULE_$(1)),echo "$(patsubst $(INSTDIR)/%,%,$(call gb_Library_get_target,$(library)))" >> $(gb_PackageInfo_get_target)/$(1).libraries &&) true

endef

define gb_PackageInfo_emit_help_for_one_target
$(foreach suf,cfg db ht idxl/_0.cfs idxl/segments_3 idxl/segments.gen jar key,echo "help/$(1).$(suf)" >> $(2) && ) true

endef

define gb_PackageInfo_emit_help_for_one_lang
@touch $(foreach suf,executables libraries files,$(gb_PackageInfo_get_target)/help-$(1).$(suf))
$(foreach target,$(gb_AllLangHelp_ALLTARGETS),$(call gb_PackageInfo_emit_help_for_one_target,$(1)/$(target),$(gb_PackageInfo_get_target)/help-$(1).files))

endef

.PHONY: packageinfo
packageinfo:
	@rm -rf $(gb_PackageInfo_get_target) && mkdir $(gb_PackageInfo_get_target)
	$(foreach installmodule,$(gb_PackageInfo_InstallModules),$(call gb_PackageInfo_emit_binaries_command,$(installmodule)))
	$(foreach helplang,$(gb_HELP_LANGS),$(call gb_PackageInfo_emit_help_for_one_lang,$(helplang)))


# vim: set noet sw=4 ts=4:
