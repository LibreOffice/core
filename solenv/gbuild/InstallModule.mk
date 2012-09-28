# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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

define gb_InstallModule_define_mingw_dll_if_set
$(call gb_InstallModuleTarget_define_mingw_dll_if_set,$(1),$(2))

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
