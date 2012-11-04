# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# CliUnoApiTarget class

# Handles creation of a CLI assembly from UNO types.rdb.

gb_CliUnoApiTarget_TARGET := $(call gb_Executable_get_target_for_build,climaker)
gb_CliUnoApiTarget_COMMAND := $(gb_Helper_set_ld_path) $(gb_CliUnoApiTarget_TARGET)

define gb_CliUnoApiTarget__command
$(call gb_Output_announce,$(2),$(true),CLI,4)
$(call gb_Helper_abbreviate_dirs,\
	$(gb_CliUnoApiTarget_COMMAND) \
		--out $(1) \
		--assembly-company "LibreOffice" \
		--assembly-description "This assembly contains metadata for the LibreOffice API." \
		--assembly-version $(CLI_UNOAPI_VERSION) \
		--keyfile $(CLI_UNOAPI_KEYFILE) \
		$(foreach api,$(CLI_UNOAPI_DEPS),-X $(api)) \
		$(foreach assembly,$(CLI_UNOAPI_ASSEMBLIES),-r $(assembly)) \
		$(CLI_UNOAPI_API) \
)
endef

$(dir $(call gb_CliUnoApiTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliUnoApiTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

# TODO depending on the whole URE might be overkill, but I do not have a
# Windows machine to debug it...
$(call gb_CliUnoApiTarget_get_target,%) : \
		$(gb_CliUnoApiTarget_TARGET) \
		$(call gb_Library_get_target,$(gb_CPPU_ENV)_uno) \
		$(call gb_Package_get_target,cppuhelper_unorc) \
		$(call gb_Rdb_get_outdir_target,ure/services)
	$(call gb_CliUnoApiTarget__command,$@,$*,$<)

.PHONY : $(call gb_CliUnoApiTarget_get_clean_target,%)
$(call gb_CliUnoApiTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CLI,4)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CliUnoApiTarget_get_target,$*) \
	)

define gb_CliUnoApiTarget_CliUnoApiTarget
$(call gb_CliUnoApiTarget_get_target,$(1)) : CLI_UNOAPI_API :=
$(call gb_CliUnoApiTarget_get_target,$(1)) : CLI_UNOAPI_ASSEMBLIES :=
$(call gb_CliUnoApiTarget_get_target,$(1)) : CLI_UNOAPI_DEPS :=
$(call gb_CliUnoApiTarget_get_target,$(1)) : CLI_UNOAPI_KEYFILE :=
$(call gb_CliUnoApiTarget_get_target,$(1)) : CLI_UNOAPI_VERSION :=

$(call gb_CliUnoApiTarget_get_target,$(1)) :| $(dir $(call gb_CliUnoApiTarget_get_target,$(1))).dir

endef

define gb_CliUnoApiTarget_set_keyfile
$(call gb_CliUnoApiTarget_get_target,$(1)) : CLI_UNOAPI_KEYFILE := $(2)
$(call gb_CliUnoApiTarget_get_target,$(1)) : $(2)

endef

define gb_CliUnoApiTarget_set_version
$(call gb_CliUnoApiTarget_get_target,$(1)) : CLI_UNOAPI_VERSION := $(2)

endef

define gb_CliUnoApiTarget_wrap_api
$(call gb_CliUnoApiTarget_get_target,$(1)) : CLI_UNOAPI_API := $(call gb_UnoApi_get_target,$(2))
$(call gb_CliUnoApiTarget_get_target,$(1)) : $(call gb_UnoApi_get_target,$(2))

endef

define gb_CliUnoApiTarget__use_api
$(call gb_CliUnoApiTarget_get_target,$(1)) : CLI_UNOAPI_DEPS += $(2)
$(call gb_CliUnoApiTarget_get_target,$(1)) : $(2)

endef

define gb_CliUnoApiTarget_use_api
$(foreach api,$(2),$(call gb_CliUnoApiTarget__use_api,$(1),$(call gb_UnoApi_get_target,$(api))))

endef

define gb_CliUnoApiTarget_use_assembly
$(call gb_CliUnoApiTarget_get_target,$(1)) : CLI_UNOAPI_ASSEMBLIES += $(call gb_CliAssemblyTarget_get_dll,$(2))
$(call gb_CliUnoApiTarget_get_target,$(1)) : $(call gb_CliAssemblyTarget_get_target,$(2))

endef

define gb_CliUnoApiTarget_use_assemblies
$(foreach assembly,$(2),$(call gb_CliUnoApiTarget_use_assembly,$(1),$(assembly)))

endef

# CliUnoApi class

# Handles creation and delivery of a CLI assembly from UNO types.rdb.

gb_CliUnoApi_KEYFILE_DEFAULT := $(gb_CliAssemblyTarget_KEYFILE_DEFAULT)

$(dir $(call gb_CliUnoApi_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliUnoApi_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliUnoApi_get_target,%) :
	touch $@

.PHONY : $(call gb_CliUnoApi_get_clean_target,%)
$(call gb_CliUnoApi_get_clean_target,%) :
	rm -f $(call gb_CliUnoApi_get_target,$*)

# Create a CLI library for UNO API.
#
# gb_CliUnoApi_CliUnoApi target
define gb_CliUnoApi_CliUnoApi
$(call gb_CliUnoApiTarget_CliUnoApiTarget,$(1))
$(call gb_CliAssemblyTarget_CliAssemblyTarget,$(1),$(call gb_CliUnoApiTarget_get_target,$(1)))

$(call gb_CliUnoApiTarget_set_keyfile,$(1),$(gb_CliUnoApi_KEYFILE_DEFAULT))

$(call gb_CliUnoApi_get_target,$(1)) : $(call gb_CliUnoApiTarget_get_target,$(1))
$(call gb_CliUnoApi_get_target,$(1)) : $(call gb_CliAssemblyTarget_get_target,$(1))
$(call gb_CliUnoApi_get_target,$(1)) :| $(dir $(call gb_CliUnoApi_get_target,$(1))).dir
$(call gb_CliUnoApi_get_clean_target,$(1)) : $(call gb_CliUnoApiTarget_get_clean_target,$(1))
$(call gb_CliUnoApi_get_clean_target,$(1)) : $(call gb_CliAssemblyTarget_get_clean_target,$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_CliUnoApiTarget_get_target,$(1)),$(call gb_CliUnoApiTarget_get_clean_target,$(1))))

endef

# Set the configuration file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliUnoApi_set_configfile assembly configfile
define gb_CliUnoApi_set_configfile
$(call gb_CliAssemblyTarget_set_configfile,$(1),$(2))

endef

# Set the sign key file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliUnoApi_set_keyfile assembly keyfile
define gb_CliUnoApi_set_keyfile
$(call gb_CliUnoApiTarget_set_keyfile,$(1),$(2))
$(call gb_CliAssemblyTarget_set_keyfile,$(1),$(2))

endef

define gb_CliUnoApi_set_assembly_version
$(call gb_CliUnoApiTarget_set_version,$(1),$(2))

endef

# Set the policy file for the assembly.
#
# The file is given by complete path.
#
# gb_CliUnoApiTarget_set_policy assembly policyfile version
define gb_CliUnoApi_set_policy
$(call gb_CliAssemblyTarget_set_policy,$(1),$(2),$(3))

endef

define gb_CliUnoApi_wrap_api
$(call gb_CliUnoApiTarget_wrap_api,$(1),$(2))

endef

define gb_CliUnoApi_use_api
$(call gb_CliUnoApiTarget_use_api,$(1),$(2))

endef

# Use another assembly for compilation.
#
# gb_CliUnoApi_use_assembly assembly dep
define gb_CliUnoApi_use_assembly
$(call gb_CliUnoApiTarget_use_assembly,$(1),$(2))

endef

# Use other assemblies for compilation.
#
# gb_CliUnoApi_use_assemblies assembly dep(s)
define gb_CliUnoApi_use_assemblies
$(call gb_CliUnoApiTarget_use_assemblies,$(1),$(2))

endef

# vim: set noet sw=4 ts=4:
