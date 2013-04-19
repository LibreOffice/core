# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Note: targets for this class are the same as for UnoApi, therefore
# there is no need to define them again (and if we did, they would
# overwrite the UnoApi targets anyway...)

define gb_UnoApiMerge_UnoApiMerge
$(call gb_UnoApiTarget_UnoApiTarget,$(1))

$(call gb_UnoApiTarget_set_root,$(1),/)

$(call gb_UnoApiMerge_get_target,$(1)) :| $(dir $(call gb_UnoApiMerge_get_target,$(1))).dir
$(call gb_UnoApiMerge_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiMerge_get_clean_target,$(1)) : $(call gb_UnoApiTarget_get_clean_target,$(1))

$(call gb_UnoApiTarget_get_headers_target,$(1)) : $(gb_Helper_MISCDUMMY)

$(call gb_Deliver_add_deliverable,$(call gb_UnoApiMerge_get_target,$(1)),$(call gb_UnoApiTarget_get_target,$(1)),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_UnoApiMerge_get_target,$(1)),$(call gb_UnoApiMerge_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),UnoApiMerge)

endef

define gb_UnoApiMerge_add_rdbfile
$$(call gb_Output_error,gb_UnoApiMerge_add_rdbfile: use gb_UnoApiMerge_use_api instead.)
endef

define gb_UnoApiMerge__use_api
$(call gb_UnoApiTarget_merge_api,$(1),$(2))
$(call gb_UnoApiTarget_get_external_headers_target,$(1)) : $(call gb_UnoApiTarget_get_headers_target,$(2))

endef

define gb_UnoApiMerge_add_rdbfiles
$$(call gb_Output_error,gb_UnoApiMerge_add_rdbfiles: use gb_UnoApiMerge_use_api instead.)
endef

define gb_UnoApiMerge_use_api
$(foreach rdb,$(2),$(call gb_UnoApiMerge__use_api,$(1),$(rdb)))

endef

define gb_UnoApiMerge_add_reference_rdbfile
$$(call gb_Output_error,gb_UnoApiMerge_add_reference_rdbfile: use gb_UnoApiMerge_set_reference_rdbfile instead.)

endef

define gb_UnoApiMerge_set_reference_rdbfile
$(call gb_UnoApiTarget_set_reference_rdbfile,$(1),$(2))

endef

# vim: set noet sw=4 ts=4:
