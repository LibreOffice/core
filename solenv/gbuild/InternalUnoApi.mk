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

.PHONY : $(call gb_InternalUnoApi_get_clean_target,%)
$(call gb_InternalUnoApi_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_InternalUnoApi_get_target,$*))

# Note: The rdb root for the internal rdbs should be / . On the other
# side, UnoApiHeadersTarget expects UCR and it is really not easy to
# change, because the information would have to be duplicated at the
# calling side. So we simply do both .-)
# TODO: Should it come clear that these rdbs (installed into
# solver/$INPATH/rdb) are actually not needed for anything, this could
# be simplified.

define gb_InternalUnoApi_InternalUnoApi
$(call gb_UnoApiTarget_UnoApiTarget,$(1)_out)
$(call gb_UnoApiTarget_UnoApiTarget,$(1))
$(call gb_UnoApiHeadersTarget_UnoApiHeadersTarget,$(1))

$(call gb_UnoApiTarget_set_root,$(1)_out,/)
$(call gb_UnoApiTarget_set_root,$(1),UCR)
$(call gb_UnoApiTarget_merge_api,$(1),$(1)_out)

$(call gb_InternalUnoApi_get_target,$(1)) :| $(dir $(call gb_InternalUnoApi_get_target,$(1))).dir
$(call gb_InternalUnoApi_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1)_out)
$(call gb_InternalUnoApi_get_clean_target,$(1)) : $(call gb_UnoApiTarget_get_clean_target,$(1)_out)
$(call gb_InternalUnoApi_get_clean_target,$(1)) : $(call gb_UnoApiHeadersTarget_get_clean_target,$(1))

$(call gb_UnoApiTarget_get_headers_target,$(1)_out) : $(gb_Helper_MISCDUMMY)
$(call gb_UnoApiTarget_get_headers_target,$(1)) : $(call gb_UnoApiTarget_get_headers_target,$(1)_out)
$(call gb_UnoApiTarget_get_external_headers_target,$(1)_out) : $(gb_Helper_MISCDUMMY)
$(call gb_UnoApiTarget_get_external_headers_target,$(1)) : $(call gb_UnoApiTarget_get_external_headers_target,$(1)_out)

$(call gb_Deliver_add_deliverable,$(call gb_InternalUnoApi_get_target,$(1)),$(call gb_UnoApiTarget_get_target,$(1)_out),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_InternalUnoApi_get_target,$(1)),$(call gb_InternalUnoApi_get_clean_target,$(1))))

endef

define gb_InternalUnoApi_add_idlfile
$(call gb_UnoApiTarget_add_idlfile,$(1)_out,$(2),$(3))

endef

define gb_InternalUnoApi_add_idlfiles
$(call gb_UnoApiTarget_add_idlfiles,$(1)_out,$(2),$(3))

endef

define gb_InternalUnoApi__use_api
$(call gb_UnoApiHeadersTarget_use_api,$(1),$(2))
$(call gb_InternalUnoApi_get_target,$(1)_out) : $(call gb_UnoApiTarget_get_target,$(2))
$(call gb_UnoApiTarget_get_external_headers_target,$(1)_out) : $(call gb_UnoApiTarget_get_headers_target,$(2))

endef

define gb_InternalUnoApi_add_api
$$(call gb_Output_error,gb_InternalUnoApi_add_api: use gb_InternalUnoApi_use_api instead.)
endef

define gb_InternalUnoApi_use_api
$(foreach rdb,$(2),$(call gb_InternalUnoApi__use_api,$(1),$(rdb)))

endef

define gb_InternalUnoApi_add_api_dependency
$$(call gb_Output_error,gb_InternalUnoApi_add_api_dependency: use gb_InternalUnoApi_define_api_dependency instead.)
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

define gb_InternalUnoApi_add_api_dependencies
$$(call gb_Output_error,gb_InternalUnoApi_add_api_dependencies: use gb_InternalUnoApi_define_api_dependencies instead.)
endef

define gb_InternalUnoApi_define_api_dependencies
$(foreach dep,$(3),$(call gb_InternalUnoApi_define_api_dependency,$(1),$(2),$(dep)))

endef

define gb_InternalUnoApi_set_types
$(call gb_UnoApiTarget_set_types,$(1)_out,$(2))

endef

define gb_InternalUnoApi_set_include
$(call gb_UnoApiTarget_set_include,$(1)_out,$(2))

endef

# vim: set noet sw=4 ts=4:
