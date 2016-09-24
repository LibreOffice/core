# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# InstallModule class

$(dir $(call gb_InstallModule_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_InstallModule_get_target,%) :
	$(call gb_Output_announce,$*,$(true),IMO,3)
	touch $@

$(call gb_InstallModule_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),IMO,3)
	rm -f $(call gb_InstallModule_get_target,$*)

define gb_InstallModule_InstallModule
$(call gb_InstallModuleTarget_InstallModuleTarget,$(1))

$(call gb_InstallModule_get_target,$(1)) : $(call gb_InstallModuleTarget_get_target,$(1))
$(call gb_InstallModule_get_target,$(1)) :| $(dir $(call gb_InstallModule_get_target,$(1))).dir
$(call gb_InstallModule_get_clean_target,$(1)) : $(call gb_InstallModuleTarget_get_clean_target,$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_InstallModule_get_target,$(1)),$(call gb_InstallModule_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),InstallModule)

endef

define gb_InstallModule_use_auto_install_libs
$(call gb_InstallModuleTarget_use_auto_install_libs,$(1),$(2))

endef

define gb_InstallModule_add_defs
$(call gb_InstallModuleTarget_add_defs,$(1),$(2))

endef

define gb_InstallModule_define_if_set
$(call gb_InstallModuleTarget_define_if_set,$(1),$(2))

endef

define gb_InstallModule_define_value_if_set
$(call gb_InstallModuleTarget_define_value_if_set,$(1),$(2))

endef

define gb_InstallModule_add_scpfile
$(call gb_InstallModuleTarget_add_scpfile,$(1),$(2))

endef

define gb_InstallModule_add_scpfiles
$(call gb_InstallModuleTarget_add_scpfiles,$(1),$(2))

endef

define gb_InstallModule_add_localized_scpfile
$(call gb_InstallModuleTarget_add_localized_scpfile,$(1),$(2))

endef

define gb_InstallModule_add_localized_scpfiles
$(call gb_InstallModuleTarget_add_localized_scpfiles,$(1),$(2))

endef

define gb_InstallModule_add_template
$(call gb_InstallModuleTarget_add_template,$(1),$(2))

endef

define gb_InstallModule_add_templates
$(call gb_InstallModuleTarget_add_templates,$(1),$(2))

endef

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
