# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# CliSignTarget class

define gb_CliSignTarget__command
$(call gb_Output_announce,$(1),$(true),SN ,4)
$(call gb_Helper_abbreviate_dirs,\
	$(GNUCOPY) $(CLI_SIGN_LIBRARY) $(1).tmp && \
	sn -R $(1).tmp $(CLI_SIGN_KEYFILE) && \
	mv $(1).tmp $(1) \
)
endef

$(dir $(call gb_CliSignTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliSignTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliSignTarget_get_target,%) :
	$(call gb_CliSignTarget__command,$@,$*,$<)

$(call gb_CliSignTarget_get_external_target,%) :
	touch $@

.PHONY : $(call gb_CliSignTarget_get_clean_target,%)
$(call gb_CliSignTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SN ,4)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_CliSignTarget_get_target,$*) \
			 $(call gb_CliSignTarget_get_external_target,$*) \
	)

# Sign a native assembly.
#
# gb_CliSignTarget_CliSignTarget assembly
define gb_CliSignTarget_CliSignTarget
$(call gb_CliSignTarget_get_target,$(1)) : CLI_SIGN_KEYFILE :=
$(call gb_CliSignTarget_get_target,$(1)) : CLI_SIGN_LIBRARY := $(2)

$(call gb_CliSignTarget_get_external_target,$(1)) :| $(dir $(call gb_CliSignTarget_get_target,$(1))).dir
$(call gb_CliSignTarget_get_target,$(1)) :| $(dir $(call gb_CliSignTarget_get_target,$(1))).dir

endef

# Set the sign key file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliSignTarget_set_keyfile assembly keyfile
define gb_CliSignTarget_set_keyfile
$(call gb_CliSignTarget_get_target,$(1)) : CLI_SIGN_KEYFILE := $(2)
$(call gb_CliSignTarget_get_target,$(1)) : $(2)

endef

# Use another assembly for compilation.
#
# gb_CliSignTarget_use_assembly assembly dep
define gb_CliSignTarget_use_assembly
$(call gb_CliSignTarget_get_external_target,$(1)) : $(call gb_CliAssemblyTarget_get_target,$(2))

endef

# Use other assemblies for compilation.
#
# gb_CliSignTarget_use_assemblies assembly dep(s)
define gb_CliSignTarget_use_assemblies
$(foreach assembly,$(2),$(call gb_CliSignTarget_use_assembly,$(1),$(assembly)))

endef

# CliLibrary class

# Handles creation of a CLI assembly from C++ sources.

# platform defined
#  gb_CliLibrary_LIBS
#  gb_CliLibrary_CliLibrary_platform
#  gb_CliLibrary_get_dll
#  gb_CliLibrary_add_delayload_dll
#  gb_CliLibrary_set_link_keyfile

gb_CliLibrary_PLATFORM_DEFAULT := x86

$(dir $(call gb_CliLibrary_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliLibrary_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliLibrary_get_target,%) :
	touch $@

.PHONY : $(call gb_CliLibrary_get_clean_target,%)
$(call gb_CliLibrary_get_clean_target,%) :
	rm -f $(call gb_CliLibrary_get_target,$*)

gb_CliLibrary_get_linktargetname = CliLibrary/$(1)

# Create a CLI assembly from C++ sources.
#
# gb_CliLibrary assembly
define gb_CliLibrary_CliLibrary
$(call gb_CliLibrary__CliLibrary_impl,$(1),$(call gb_CliLibrary_get_linktargetname,$(1)))

endef

define gb_CliLibrary__CliLibrary_impl
$(call gb_LinkTarget_LinkTarget,$(2),CliLibrary_$(1))
$(call gb_CliSignTarget_CliSignTarget,$(1),$(call gb_CliLibrary_get_dll,$(1)))
$(call gb_CliAssemblyTarget_CliAssemblyTarget,$(1),$(call gb_CliSignTarget_get_target,$(1)))

$(call gb_LinkTarget_set_targettype,$(2),CliLibrary)
$(call gb_LinkTarget_add_defs,$(2),gb_CliLibrary_DEFS)
$(call gb_LinkTarget_add_libs,$(2),gb_CliLibrary_LIBS)

$(call gb_CliLibrary_CliLibrary_platform,$(1),$(2),$(call gb_CliLibrary_get_dll,$(1)))

$(call gb_CliSignTarget_set_keyfile,$(1),$(gb_CliAssemblyTarget_KEYFILE_DEFAULT))
$(call gb_CliAssemblyTarget_set_platform,$(1),$(gb_CliLibrary_PLATFORM_DEFAULT))

$(call gb_CliSignTarget_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_CliLibrary_get_target,$(1)) : $(call gb_CliSignTarget_get_target,$(1))
$(call gb_CliLibrary_get_target,$(1)) : $(call gb_CliAssemblyTarget_get_target,$(1))
$(call gb_CliLibrary_get_target,$(1)) :| $(dir $(call gb_CliLibrary_get_target,$(1))).dir

$(call gb_CliLibrary_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(1))
$(call gb_CliLibrary_get_clean_target,$(1)) : $(call gb_CliSignTarget_get_clean_target,$(1))
$(call gb_CliLibrary_get_clean_target,$(1)) : $(call gb_CliAssemblyTarget_get_clean_target,$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_CliLibrary_get_target,$(1)),$(call gb_CliLibrary_get_clean_target,$(1))))

endef

# Set the configuration file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliLibrary_set_configfile assembly configfile
define gb_CliLibrary_set_configfile
$(call gb_CliAssemblyTarget_set_configfile,$(1),$(2))

endef

# Set the sign key file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliLibrary_set_keyfile assembly keyfile
define gb_CliLibrary_set_keyfile
$(call gb_CliSignTarget_set_keyfile,$(1),$(2))
$(call gb_CliAssemblyTarget_set_keyfile,$(1),$(2))
$(call gb_CliLibrary_set_link_keyfile,$(1),$(2))

endef

# Set the platform of the assembly.
# TODO is this really needed?
#
# gb_CliLibrary_set_platform assembly platform
define gb_CliLibrary_set_platform
$(call gb_CliAssemblyTarget_set_platform,$(1),$(2))

endef

# Set the policy file for the assembly.
#
# The file is given by complete path.
#
# gb_CliLibrary_set_policy assembly policyfile version
define gb_CliLibrary_set_policy
$(call gb_CliAssemblyTarget_set_policy,$(1),$(2),$(3))

endef

# Use another assembly for compilation.
#
# gb_CliLibrary_use_assembly assembly dep
define gb_CliLibrary_use_assembly
$(call gb_CliSignTarget_use_assembly,$(1),$(2))

endef

# Use other assemblies for compilation.
#
# gb_CliLibrary_use_assemblies assembly dep(s)
define gb_CliLibrary_use_assemblies
$(call gb_CliSignTarget_use_assemblies,$(call gb_CliLibrary_get_linktargetname,$(1)),$(2))

endef

# Use a public UNO API library.
#
# E.g., cppu, sal.
#
# gb_CliLibrary_use_api_library assembly library
define gb_CliLibrary_use_api_library
$(call gb_LinkTarget_use_library,$(call gb_CliLibrary_get_linktargetname,$(1)),$(2))
$(call gb_CliLibrary_add_delayload_dll,$(1),$(2))

endef

# Use public UNO API libraries.
#
# E.g., cppu, sal.
#
# gb_CliLibrary_use_api_libraries assembly library
define gb_CliLibrary_use_api_libraries
$(foreach library,$(2),$(call gb_CliLibrary_use_api_library,$(1),$(library)))

endef

# straight forwards to LinkTarget

define gb_CliLibrary_use_internal_bootstrap_api
$(call gb_Library_use_internal_bootstrap_api,$(call gb_CliLibrary_get_linktargetname,$(1)),$(2))

endef

define gb_CliLibrary_use_packages
$(call gb_Library_use_packages,$(call gb_CliLibrary_get_linktargetname,$(1)),$(2))

endef

define gb_CliLibrary_add_exception_objects
$(call gb_Library_add_exception_objects,$(call gb_CliLibrary_get_linktargetname,$(1)),$(2))

endef

define gb_CliLibrary_add_generated_exception_objects
$(call gb_Library_add_generated_exception_objects,$(call gb_CliLibrary_get_linktargetname,$(1)),$(2))

endef

# vim: set noet sw=4 ts=4:
