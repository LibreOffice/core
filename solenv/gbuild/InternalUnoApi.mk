# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(dir $(call gb_InternalUnoApi_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $(@)))

$(dir $(call gb_InternalUnoApi_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $(@)))

$(call gb_InternalUnoApi_get_target,%) :
	$(call gb_Output_announce,$*,$(true),UNI,5) \
	touch $@

.PHONY : $(call gb_InternalUnoApi_get_clean_target,%)
$(call gb_InternalUnoApi_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UNI,5) \
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_InternalUnoApi_get_target,$*))

define gb_InternalUnoApi_InternalUnoApi
$(call gb_UnoApiTarget_UnoApiTarget,$(1),$(2))
$(call gb_UnoApiHeadersTarget_UnoApiHeadersTarget,$(1))

$(call gb_InternalUnoApi_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_InternalUnoApi_get_target,$(1)) :| $(dir $(call gb_InternalUnoApi_get_target,$(1))).dir
$(call gb_InternalUnoApi_get_clean_target,$(1)) : $(call gb_UnoApiHeadersTarget_get_clean_target,$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_InternalUnoApi_get_target,$(1)),$(call gb_InternalUnoApi_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),InternalUnoApi)

endef

define gb_InternalUnoApi_add_idlfile
$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(3))

endef

define gb_InternalUnoApi_add_idlfiles
$(call gb_UnoApiTarget_add_idlfiles,$(1),$(2),$(3))

endef

define gb_InternalUnoApi__use_api
$(call gb_UnoApiTarget_use_api,$(1),$(2))
$(call gb_UnoApiHeadersTarget_use_api,$(1),$(2))
$(call gb_InternalUnoApi_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(2))

endef

define gb_InternalUnoApi_use_api
$(foreach rdb,$(2),$(call gb_InternalUnoApi__use_api,$(1),$(rdb)))

endef

# Express that the rdb $(2) depends on rdb $(3).
#
# This information is already available in the UnoApiTarget definition
# for $(2), but this may not be loaded if we are building from a
# different module. Thus, this is a necessary hack to make generation of
# headers on demand work.
#
# I suppose it would be possible to store the list of required rdbs for
# a rdb to a file and then load it when headers' generation is requested,
# but it feels like overkill...
define gb_InternalUnoApi_define_api_dependency
$(call gb_UnoApiHeadersTarget_use_api,$(2),$(3))

endef

define gb_InternalUnoApi_define_api_dependencies
$(foreach dep,$(3),$(call gb_InternalUnoApi_define_api_dependency,$(1),$(2),$(dep)))

endef

# vim: set noet sw=4 ts=4:
