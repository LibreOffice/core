# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# CliNativeLibraryTarget class

gb_CliNativeLibraryTarget_EXT := $(gb_CliAssembly_POLICYEXT)

gb_CliNativeLibraryTarget__get_library = $(call gb_CliAssembly_get_dll,$(1))

define gb_CliNativeLibraryTarget__command
$(call gb_Output_announce,$(1),$(true),SN ,4)
$(call gb_Helper_abbreviate_dirs,\
	$(GNUCOPY) $(CLI_NATIVE_LIBRARY) $(1).tmp && \
	sn -R $(1).tmp $(CLI_NATIVE_KEYFILE) && \
	mv $(1).tmp $(1) \
)
endef

$(call gb_CliNativeLibraryTarget_get_target,%) :
	$(call gb_CliNativeLibraryTarget__command,$@,$*,$<)

$(call gb_CliNativeLibraryTarget_get_external_target,%) :
	touch $@

.PHONY : $(call gb_CliNativeLibraryTarget_get_clean_target,%)
$(call gb_CliNativeLibraryTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),SN ,4)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_CliNativeLibraryTarget_get_target,$*) \
			 $(call gb_CliNativeLibraryTarget_get_external_target,$*) \
	)

define gb_CliNativeLibraryTarget_CliNativeLibraryTarget
$(call gb_CliNativeLibraryTarget_get_target,$(1)) : CLI_NATIVE_ASSEMBLIES := $(gb_Helper_MISCDUMMY)
$(call gb_CliNativeLibraryTarget_get_target,$(1)) : CLI_NATIVE_KEYFILE :=
$(call gb_CliNativeLibraryTarget_get_target,$(1)) : CLI_NATIVE_LIBRARY :=

$(call gb_CliNativeLibraryTarget_get_external_target,$(1)) :| $(dir $(call gb_CliNativeLibraryTarget_get_target,$(1))).dir
$(call gb_CliNativeLibraryTarget_get_target,$(1)) :| $(dir $(call gb_CliNativeLibraryTarget_get_target,$(1))).dir

endef

define gb_CliNativeLibraryTarget_wrap_library
$(call gb_CliNativeLibraryTarget_get_target,$(1)) : CLI_NATIVE_LIBRARY := $(call gb_CliNativeLibraryTarget__get_library,$(2))
$(call gb_CliNativeLibraryTarget_get_target,$(1)) : $(call gb_Library_get_target,$(2))
$(call gb_Library_get_external_headers_target,$(2)) : $(call gb_CliNativeLibraryTarget_get_external_target,$(1))

endef

define gb_CliNativeLibraryTarget_set_keyfile
$(call gb_CliNativeLibraryTarget_get_target,$(1)) : CLI_NATIVE_KEYFILE := $(2)
$(call gb_CliNativeLibraryTarget_get_target,$(1)) : $(2)

endef

define gb_CliNativeLibraryTarget_use_assembly
$(call gb_CliNativeLibraryTarget_get_target,$(1)) : CLI_NATIVE_ASSEMBLIES += $(call gb_CliLibrary_get_target,$(2))
$(call gb_CliNativeLibraryTarget_get_external_target,$(1)) : $(call gb_CliLibrary_get_target,$(2))

endef

define gb_CliNativeLibraryTarget_use_assemblies
$(foreach assembly,$(2),$(call gb_CliNativeLibraryTarget_use_assembly,$(1),$(assembly)))

endef

# CliNativeLibrary class

gb_CliNativeLibrary_PLATFORM_DEFAULT := x86
gb_CliNativeLibrary_EXT := $(gb_CliNativeLibraryTarget_EXT)

# Create a CLI library for a native library
#
# CliNativeLibrary target
define gb_CliNativeLibrary_CliNativeLibrary
$(call gb_CliNativeLibraryTarget_CliNativeLibraryTarget,$(1))
$(call gb_CliAssembly_CliAssembly,$(1))

$(call gb_CliNativeLibraryTarget_set_keyfile,$(1),$(gb_CliAssembly_KEYFILE_DEFAULT))
$(call gb_CliAssembly_set_platform,$(1),$(gb_CliNativeLibrary_PLATFORM_DEFAULT))

$(call gb_CliNativeLibrary_get_target,$(1)) : $(call gb_CliNativeLibraryTarget_get_target,$(1))
$(call gb_CliNativeLibrary_get_target,$(1)) :| $(call gb_CliAssembly_get_target,$(1))
$(call gb_CliNativeLibrary_get_target,$(1)) :| $(dir $(call gb_CliNativeLibrary_get_target,$(1))).dir
$(call gb_CliNativeLibrary_get_clean_target,$(1)) : $(call gb_CliNativeLibraryTarget_get_clean_target,$(1))
$(call gb_CliNativeLibrary_get_clean_target,$(1)) : $(call gb_CliAssembly_get_clean_target,$(1))

$(call gb_Deliver_add_deliverable,$(call gb_CliNativeLibrary_get_target,$(1)),$(call gb_CliNativeLibraryTarget_get_target,$(1)),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_CliNativeLibrary_get_target,$(1)),$(call gb_CliNativeLibrary_get_clean_target,$(1))))

endef

define gb_CliNativeLibrary_set_configfile
$(call gb_CliAssembly_set_configfile,$(1),$(2))

endef

define gb_CliNativeLibrary_set_keyfile
$(call gb_CliNativeLibraryTarget_set_keyfile,$(1),$(2))
$(call gb_CliAssembly_set_keyfile,$(1),$(2))

endef

define gb_CliNativeLibrary_set_platform
$(call gb_CliAssembly_set_platform,$(1),$(2))

endef

define gb_CliNativeLibrary_set_policy
$(call gb_CliAssembly_set_policy,$(1),$(2),$(3))

endef

define gb_CliNativeLibrary_wrap_library
$(call gb_CliNativeLibraryTarget_wrap_library,$(1),$(2))

endef

define gb_CliNativeLibrary_use_assembly
$(call gb_CliNativeLibraryTarget_use_assembly,$(1),$(2))

endef

define gb_CliNativeLibrary_use_assemblies
$(call gb_CliNativeLibraryTarget_use_assemblies,$(1),$(2))

endef

# vim: set noet sw=4 ts=4:
