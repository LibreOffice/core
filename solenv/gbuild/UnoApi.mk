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

.PHONY : $(call gb_UnoApi_get_clean_target,%)
$(call gb_UnoApi_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UnoApi_get_target,$*))

define gb_UnoApi_UnoApi
$(call gb_UnoApiTarget_UnoApiTarget,$(1))
$(call gb_UnoApiHeadersTarget_UnoApiHeadersTarget,$(1))
$(call gb_Package_Package_internal,$(1)_idl,$(SRCDIR))
$(call gb_Package_Package_internal,$(1)_inc,$(call gb_UnoApiHeadersTarget_get_dir,$(1)))

$(call gb_UnoApiTarget_set_root,$(1),UCR)

$(call gb_UnoApi_get_target,$(1)) :| $(dir $(call gb_UnoApi_get_target,$(1))).dir
$(call gb_UnoApi_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApi_get_target,$(1)) :| $(call gb_Package_get_target,$(1)_idl)
$(call gb_UnoApi_get_target,$(1)) :| $(call gb_Package_get_target,$(1)_inc)
$(call gb_UnoApi_get_clean_target,$(1)) : $(call gb_UnoApiTarget_get_clean_target,$(1))
$(call gb_UnoApi_get_clean_target,$(1)) : $(call gb_UnoApiHeadersTarget_get_clean_target,$(1))
$(call gb_UnoApi_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(1)_idl)
$(call gb_UnoApi_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(1)_inc)

$(call gb_UnoApiTarget_get_headers_target,$(1)) : $(call gb_Package_get_target,$(1)_idl)
$(call gb_Package_get_preparation_target,$(1)_inc) : $(call gb_UnoApiHeadersTarget_get_target,$(1))

$(call gb_Deliver_add_deliverable,$(call gb_UnoApi_get_target,$(1)),$(call gb_UnoApiTarget_get_target,$(1)),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_UnoApi_get_target,$(1)),$(call gb_UnoApi_get_clean_target,$(1))))

endef

define gb_UnoApi__add_idlfile
$(call gb_Package_add_file,$(1)_idl,$(patsubst $(1)/%,idl/%,$(2)),$(2))

endef

define gb_UnoApi__add_headerfile_impl
$(call gb_Package_add_file,$(1)_inc,inc/$(2),$(3))
$(call gb_UnoApiHeadersTarget_add_headerfile,$(1),$(2),$(3))

endef

# Note: the expression in 3rd arg of call of gb_UnoApi__add_headerfile_impl
# removes the leftmost dir in the subpath .-)
define gb_UnoApi__add_headerfile
$(call gb_UnoApi__add_headerfile_impl,$(1),$(2),$(subst $() $(),/,$(wordlist 2,$(words $(subst /, ,$(2))),$(subst /, ,$(2)))))

endef

define gb_UnoApi__add_idlfile_noheader
$(call gb_UnoApi__add_idlfile,$(1),$(2).idl)

endef

define gb_UnoApi__add_idlfile_nohdl
$(call gb_UnoApi__add_idlfile_noheader,$(1),$(2))
$(call gb_UnoApi__add_headerfile,$(1),$(2).hpp)

endef

define gb_UnoApi__add_idlfile_full
$(call gb_UnoApi__add_idlfile_nohdl,$(1),$(2))
$(call gb_UnoApi__add_headerfile,$(1),$(2).hdl)

endef

# for interfaces, exceptions, structs, enums, constant groups
define gb_UnoApi_add_idlfile
$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(3))
$(call gb_UnoApi__add_idlfile_full,$(1),$(2)/$(3))

endef

define gb_UnoApi_add_idlfiles
$(call gb_UnoApiTarget_add_idlfiles,$(1),$(2),$(3))
$(foreach idl,$(3),$(call gb_UnoApi__add_idlfile_full,$(1),$(2)/$(idl)))

endef

# for new-style services
define gb_UnoApi_add_idlfile_nohdl
$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(3))
$(call gb_UnoApi__add_idlfile_nohdl,$(1),$(2)/$(3))

endef

define gb_UnoApi_add_idlfiles_nohdl
$(call gb_UnoApiTarget_add_idlfiles,$(1),$(2),$(3))
$(foreach idl,$(3),$(call gb_UnoApi__add_idlfile_nohdl,$(1),$(2)/$(idl)))

endef

# for old-style services and modules
define gb_UnoApi_add_idlfile_noheader
$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(3))
$(call gb_UnoApi__add_idlfile_noheader,$(1),$(2)/$(3))

endef

define gb_UnoApi_add_idlfiles_noheader
$(call gb_UnoApiTarget_add_idlfiles,$(1),$(2),$(3))
$(foreach idl,$(3),$(call gb_UnoApi__add_idlfile_noheader,$(1),$(2)/$(idl)))

endef

define gb_UnoApi__use_api
$(call gb_UnoApiHeadersTarget_use_api,$(1),$(2))
$(call gb_UnoApi_get_target,$(1)) :| $(call gb_UnoApi_get_target,$(2))
$(call gb_UnoApiTarget_get_external_headers_target,$(1)) : $(call gb_UnoApiTarget_get_headers_target,$(2))

endef

define gb_UnoApi_add_api
$$(call gb_Output_error,gb_UnoApi_add_api: use gb_UnoApi_use_api instead.)
endef

define gb_UnoApi_use_api
$(foreach rdb,$(2),$(call gb_UnoApi__use_api,$(1),$(rdb)))

endef

define gb_UnoApi_add_reference_rdbfile
$$(call gb_Output_error,gb_UnoApi_add_reference_rdbfile: use gb_UnoApi_set_reference_rdbfile instead.)

endef

define gb_UnoApi_set_reference_rdbfile
$(call gb_UnoApiTarget_set_reference_rdbfile,$(1),$(2))

endef

define gb_UnoApi_set_include
$(call gb_UnoApiTarget_set_include,$(1),$(2))

endef

# vim: set noet sw=4 ts=4:
