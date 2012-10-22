# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# CliConfigTarget class

gb_CliConfigTarget_TARGET := $(SOLARENV)/bin/clipatchconfig.pl
gb_CliConfigTarget_COMMAND := $(PERL) -w $(gb_CliConfigTarget_TARGET)

gb_CliConfigTarget_VERSIONFILE_DEFAULT := $(SRCDIR)/cli_ure/version/version.txt

define gb_CliConfigTarget__command
$(call gb_Output_announce,$(2),$(true),CPA,1)
$(call gb_Helper_abbreviate_dirs,\
	$(gb_CliConfigTarget_COMMAND) $(3) $(CLI_CONFIG_VERSIONFILE) $(1) \
)
endef

$(dir $(call gb_CliConfigTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliConfigTarget_get_target,%) :
	$(call gb_CliConfigTarget__command,$@,$*,$<)

$(call gb_CliConfigTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CPA,1)
	rm -f $(call gb_CliConfigTarget_get_target,$*)

# Subst. variables in a config file
#
# gb_CliConfigTarget_CliConfigTarget target source
define gb_CliConfigTarget_CliConfigTarget
$(call gb_CliConfigTarget_get_target,$(1)) : CLI_CONFIG_VERSIONFILE := $(gb_CliConfigTarget_VERSIONFILE_DEFAULT)

$(call gb_CliConfigTarget_get_target,$(1)) : $(2)
$(call gb_CliConfigTarget_get_target,$(1)) : $(gb_CliConfigTarget_TARGET)
$(call gb_CliConfigTarget_get_target,$(1)) : $(gb_CliConfigTarget_VERSIONFILE_DEFAULT)
$(call gb_CliConfigTarget_get_target,$(1)) :| $(dir $(call gb_CliConfigTarget_get_target,$(1))).dir

endef

# CliAssemblyTarget class

# platform:
#  CliAssemblyTarget_POLICYEXT
#  CliAssemblyTarget_get_dll

gb_CliAssemblyTarget_KEYFILE_DEFAULT := $(SRCDIR)/cli_ure/source/cliuno.snk

define gb_CliAssemblyTarget__command
$(call gb_Output_announce,$(2),$(true),AL ,2)
$(call gb_Helper_abbreviate_dirs,\
	al \
		-out:$(CLI_ASSEMBLY_OUTFILE) \
		-version:$(CLI_ASSEMBLY_VERSION) \
		-keyfile:$(call gb_Helper_windows_path,$(CLI_ASSEMBLY_KEYFILE)) \
		-link:$(CLI_ASSEMBLY_CONFIGFILE) \
		$(if $(CLI_ASSEMBLY_PLATFORM),-platform:$(CLI_ASSEMBLY_PLATFORM)) && \
	touch $(1) \
)
endef

$(dir $(call gb_CliAssemblyTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliAssemblyTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliAssemblyTarget_get_target,%) :
	$(if $(strip $(CLI_ASSEMBLY_VERSION)),,$(call gb_Output_error,assembly version not set))
	$(if $(strip $(CLI_ASSEMBLY_CONFIGFILE)),,$(call gb_Output_error,assembly configuration file not set))
	$(if $(strip $(CLI_ASSEMBLY_NAME)),,$(call gb_Output_error,assembly name not set))
	$(call gb_CliAssemblyTarget__command,$@,$*,$<)

$(call gb_CliAssemblyTarget_get_assembly_target,%) :
	touch $@

.PHONY : $(call gb_CliAssemblyTarget_get_clean_target,%)
$(call gb_CliAssemblyTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),AL ,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CliAssemblyTarget_get_target,$*) $(CLI_ASSEMBLY_OUTFILE) \
	)

# Create a CLI assembly
define gb_CliAssemblyTarget_CliAssemblyTarget
$(call gb_CliAssemblyTarget_get_target,$(1)) : CLI_ASSEMBLY_CONFIGFILE :=
$(call gb_CliAssemblyTarget_get_target,$(1)) : CLI_ASSEMBLY_KEYFILE := $(gb_CliAssemblyTarget_KEYFILE_DEFAULT)
$(call gb_CliAssemblyTarget_get_target,$(1)) : CLI_ASSEMBLY_OUTFILE :=
$(call gb_CliAssemblyTarget_get_target,$(1)) : CLI_ASSEMBLY_PLATFORM :=
$(call gb_CliAssemblyTarget_get_target,$(1)) : CLI_ASSEMBLY_VERSION :=

$(call gb_CliAssemblyTarget_get_clean_target,$(1)) : CLI_ASSEMBLY_OUTFILE :=

$(call gb_CliAssemblyTarget_get_target,$(1)) :| $(dir $(call gb_CliAssemblyTarget_get_target,$(1))).dir

endef

define gb_CliAssemblyTarget_set_configfile
$(call gb_CliAssemblyTarget_get_target,$(1)) : CLI_ASSEMBLY_CONFIGFILE := $(2)
$(call gb_CliAssemblyTarget_get_target,$(1)) : $(2)

endef

define gb_CliAssemblyTarget_set_keyfile
$(call gb_CliAssemblyTarget_get_target,$(1)) : CLI_ASSEMBLY_KEYFILE := $(2)
$(call gb_CliAssemblyTarget_get_target,$(1)) : $(2)

endef

define gb_CliAssemblyTarget_set_name
$(call gb_CliAssemblyTarget_get_target,$(1)) \
$(call gb_CliAssemblyTarget_get_clean_target,$(1)) : \
	CLI_ASSEMBLY_OUTFILE := $(call gb_CliAssemblyTarget_get_assembly_target,$(2))
$(call gb_CliAssemblyTarget_get_assembly_target,$(2)) : $(call gb_CliAssemblyTarget_get_target,$(1))

endef

define gb_CliAssemblyTarget_set_platform
$(call gb_CliAssemblyTarget_get_target,$(1)) : CLI_ASSEMBLY_PLATFORM := $(2)

endef

define gb_CliAssemblyTarget_set_version
$(call gb_CliAssemblyTarget_get_target,$(1)) : CLI_ASSEMBLY_VERSION := $(2)

endef

# CliAssembly class

gb_CliAssembly_KEYFILE_DEFAULT := $(gb_CliAssemblyTarget_KEYFILE_DEFAULT)
gb_CliAssembly_POLICYEXT := $(gb_CliAssemblyTarget_POLICYEXT)

gb_CliAssembly_get_dll = $(call gb_CliAssemblyTarget_get_dll,$(1))

$(call gb_CliAssembly_get_target,%) :
	$(call gb_Output_announce,$*,$(true),CLA,3)
	mkdir -p $(dir $@) && touch $@

.PHONY : $(call gb_CliAssembly_get_clean_target,%)
$(call gb_CliAssembly_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CLA,3)
	rm -f $(call gb_CliAssembly_get_target,$*)

define gb_CliAssembly_CliAssembly
$(call gb_CliAssemblyTarget_CliAssemblyTarget,$(1))
$(call gb_Package_Package_internal,$(1)_assembly,$(WORKDIR))

$(call gb_CliAssembly_get_target,$(1)) : $(call gb_CliAssemblyTarget_get_target,$(1))
$(call gb_CliAssembly_get_target,$(1)) :| $(call gb_Package_get_target,$(1)_assembly)
$(call gb_CliAssembly_get_clean_target,$(1)) : $(call gb_CliAssemblyTarget_get_clean_target,$(1))
$(call gb_CliAssembly_get_clean_target,$(1)) : $(call gb_Package_get_clean_target,$(1)_assembly)

endef

define gb_CliAssembly__set_configfile_impl
$(call gb_CliAssemblyTarget_set_configfile,$(1),$(2))
$(call gb_Package_add_file,$(1)_assembly,bin/$(notdir $(2)),$(subst $(WORKDIR)/,,$(2)))

endef

define gb_CliAssembly__set_configfile
$(call gb_CliConfigTarget_CliConfigTarget,$(2),$(3))
$(call gb_CliAssembly__set_configfile_impl,$(1),$(call gb_CliConfigTarget_get_target,$(2)))

endef

define gb_CliAssembly_set_configfile
$(call gb_CliAssembly__set_configfile,$(1),$(patsubst %_config,%,$(2)),$(SRCDIR)/$(2))

endef

define gb_CliAssembly_set_keyfile
$(call gb_CliAssemblyTarget_set_keyfile,$(1),$(2))

endef

define gb_CliAssembly_set_platform
$(call gb_CliAssemblyTarget_set_platform,$(1),$(2))

endef

define gb_CliAssembly__set_policy
$(call gb_Package_add_file,$(1)_assembly,bin/$(notdir $(2)),$(subst $(WORKDIR)/,,$(2)))

endef

define gb_CliAssembly_set_policy
$(call gb_CliAssemblyTarget_set_version,$(1),$(3))
$(call gb_CliAssemblyTarget_set_name,$(1),$(2))
$(call gb_CliAssembly__set_policy,$(1),$(call gb_CliAssemblyTarget_get_assembly_target,$(2)))

endef

# vim: set noet sw=4 ts=4:
