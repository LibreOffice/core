# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# CliUnoApi class

gb_CliUnoApi_EXT := $(gb_CliAssembly_POLICYEXT)

gb_CliUnoApi_KEYFILE_DEFAULT := $(gb_CliAssembly_KEYFILE_DEFAULT)

gb_CliUnoApi_DEPS := $(call gb_Executable_get_runtime_dependencies,climaker)
gb_CliUnoApi_COMMAND := $(call gb_Executable_get_command,climaker)

define gb_CliUnoApi__command
$(call gb_Helper_abbreviate_dirs,\
	$(gb_CliUnoApi_COMMAND) \
		--out $(1) \
		--assembly-company "LibreOffice" \
		--assembly-description "This assembly contains metadata for the LibreOffice API." \
		--assembly-version $(CLI_UNOAPI_VERSION) \
		--keyfile $(CLI_UNOAPI_KEYFILE) \
		$(if $(gb_VERBOSE),--verbose) \
		$(foreach api,$(CLI_UNOAPI_DEPS),-X $(api)) \
		$(foreach assembly,$(CLI_UNOAPI_ASSEMBLIES),-r $(assembly)) \
		$(CLI_UNOAPI_API) \
)
endef

.PHONY : $(call gb_CliUnoApi_get_clean_target,%)
$(call gb_CliUnoApi_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CLI,4)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CliUnoApi_get_target,$*) \
	)


# Create a CLI library for UNO API
#
# gb_CliUnoApi_CliUnoApi target
define gb_CliUnoApi_CliUnoApi
$(call gb_CliAssembly_CliAssembly,$(1),$(call gb_CliUnoApi_get_target,$(1)))

$(call gb_CliUnoApi_get_target,$(1)) : CLI_UNOAPI_API :=
$(call gb_CliUnoApi_get_target,$(1)) : CLI_UNOAPI_ASSEMBLIES :=
$(call gb_CliUnoApi_get_target,$(1)) : CLI_UNOAPI_DEPS :=
$(call gb_CliUnoApi_get_target,$(1)) : CLI_UNOAPI_KEYFILE :=
$(call gb_CliUnoApi_get_target,$(1)) : CLI_UNOAPI_VERSION :=

$(call gb_CliUnoApi_set_keyfile,$(1),$(gb_CliUnoApi_KEYFILE_DEFAULT))

$(call gb_CliUnoApi_get_target,$(1)) :| $(call gb_CliAssembly_get_target,$(1))
$(call gb_CliUnoApi_get_target,$(1)) :| $(dir $(call gb_CliUnoApi_get_target,$(1))).dir
$(call gb_CliUnoApi_get_clean_target,$(1)) : $(call gb_CliAssembly_get_clean_target,$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_CliUnoApi_get_target,$(1)),$(call gb_CliUnoApi_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),CliUnoApi)


$(call gb_CliUnoApi_get_target,$(1)) : $(gb_CliUnoApi_DEPS)
	$$(call gb_Output_announce,$(1),$(true),CLI,4)
	$$(call gb_Trace_StartRange,$(1),CLI)
	$$(call gb_CliUnoApi__command,$$@,$(1))
	$$(call gb_Trace_EndRange,$(1),CLI)

endef

define gb_CliUnoApi_set_configfile
$(call gb_CliAssembly_set_configfile,$(1),$(2),$(3))

endef

define gb_CliUnoApi_set_keyfile
$(call gb_CliAssembly_set_keyfile,$(1),$(2))
$(call gb_CliUnoApi_get_target,$(1)) : CLI_UNOAPI_KEYFILE := $(2)
$(call gb_CliUnoApi_get_target,$(1)) : $(2)

endef

define gb_CliUnoApi_set_assembly_version
$(call gb_CliUnoApi_get_target,$(1)) : CLI_UNOAPI_VERSION := $(2)

endef

define gb_CliUnoApi_set_policy
$(call gb_CliAssembly_set_policy,$(1),$(2),$(3))

endef

define gb_CliUnoApi_wrap_api
$(call gb_CliUnoApi_get_target,$(1)) : CLI_UNOAPI_API := $(call gb_UnoApiTarget_get_target,$(2))
$(call gb_CliUnoApi_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(2))

endef

define gb_CliUnoApi__use_api
$(call gb_CliUnoApi_get_target,$(1)) : CLI_UNOAPI_DEPS += $(2)
$(call gb_CliUnoApi_get_target,$(1)) : $(2)

endef

define gb_CliUnoApi_use_api
$(foreach api,$(2),$(call gb_CliUnoApi__use_api,$(1),$(call gb_UnoApiTarget_get_target,$(api))))

endef

define gb_CliUnoApi_use_assembly
$(call gb_CliUnoApi_get_target,$(1)) : CLI_UNOAPI_ASSEMBLIES += $(call gb_CliUnoApi_get_target,$(2))
$(call gb_CliUnoApi_get_target,$(1)) : $(call gb_CliUnoApi_get_target,$(2))

endef

define gb_CliUnoApi_use_assemblies
$(foreach assembly,$(2),$(call gb_CliUnoApi_use_assembly,$(1),$(assembly)))

endef

# vim: set noet sw=4 ts=4:
