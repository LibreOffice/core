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
