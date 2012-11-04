# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# CliConfigTarget class

# Substitutes variables in an assembly configuration file.

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

# CliPolicyTarget class

# Creates an associated policy dll for an assembly.

gb_CliPolicyTarget_KEYFILE_DEFAULT := $(SRCDIR)/cli_ure/source/cliuno.snk

$(dir $(call gb_CliPolicyTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliPolicyTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliPolicyTarget_get_target,%) :
	$(if $(strip $(CLI_ASSEMBLY_VERSION)),,$(call gb_Output_error,assembly version not set))
	$(if $(strip $(CLI_ASSEMBLY_CONFIGFILE)),,$(call gb_Output_error,assembly configuration file not set))
	$(if $(strip $(CLI_ASSEMBLY_OUTFILE)),,$(call gb_Output_error,assembly name not set))
	$(call gb_CliPolicyTarget__command,$@,$*,$<)

$(call gb_CliPolicyTarget_get_assembly_target,%) :
	touch $@

.PHONY : $(call gb_CliPolicyTarget_get_clean_target,%)
$(call gb_CliPolicyTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),AL ,2)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CliPolicyTarget_get_target,$*) $(CLI_ASSEMBLY_OUTFILE) \
	)

# Create a policy dll.
#
# gb_CliPolicyTarget_CliPolicyTarget assembly
define gb_CliPolicyTarget_CliPolicyTarget
$(call gb_CliPolicyTarget_get_target,$(1)) : CLI_ASSEMBLY_CONFIGFILE :=
$(call gb_CliPolicyTarget_get_target,$(1)) : CLI_ASSEMBLY_KEYFILE := $(gb_CliPolicyTarget_KEYFILE_DEFAULT)
$(call gb_CliPolicyTarget_get_target,$(1)) : CLI_ASSEMBLY_OUTFILE :=
$(call gb_CliPolicyTarget_get_target,$(1)) : CLI_ASSEMBLY_PLATFORM :=
$(call gb_CliPolicyTarget_get_target,$(1)) : CLI_ASSEMBLY_VERSION :=

$(call gb_CliPolicyTarget_get_clean_target,$(1)) : CLI_ASSEMBLY_OUTFILE :=

$(call gb_CliPolicyTarget_get_target,$(1)) :| $(dir $(call gb_CliPolicyTarget_get_target,$(1))).dir

endef

# Get the assembly dll.
#
# gb_CliAssemblyTarget_get_dll assembly
gb_CliAssemblyTarget_get_dll = $(call gb_CliAssemblyTarget_get_outdir_target,$(1)).dll

# Set the configuration file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliPolicyTarget_set_configfile assembly configfile
define gb_CliPolicyTarget_set_configfile
$(call gb_CliPolicyTarget_get_target,$(1)) : CLI_ASSEMBLY_CONFIGFILE := $(2)
$(call gb_CliPolicyTarget_get_target,$(1)) : $(2)

endef

# Set the sign key file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliPolicyTarget_set_keyfile assembly keyfile
define gb_CliPolicyTarget_set_keyfile
$(call gb_CliPolicyTarget_get_target,$(1)) : CLI_ASSEMBLY_KEYFILE := $(2)
$(call gb_CliPolicyTarget_get_target,$(1)) : $(2)

endef

# Set the name of the created policy dll.
#
# gb_CliPolicyTarget_set_name assembly policy
define gb_CliPolicyTarget_set_name
$(call gb_CliPolicyTarget_get_target,$(1)) \
$(call gb_CliPolicyTarget_get_clean_target,$(1)) : \
	CLI_ASSEMBLY_OUTFILE := $(call gb_CliPolicyTarget_get_assembly_target,$(2))
$(call gb_CliPolicyTarget_get_assembly_target,$(2)) : $(call gb_CliPolicyTarget_get_target,$(1))

endef

# Set the platform of the assembly.
#
# Only applicable for C++ assemblies (CliLibrary).
#
# gb_CliPolicyTarget_set_platform assembly platform
define gb_CliPolicyTarget_set_platform
$(call gb_CliPolicyTarget_get_target,$(1)) : CLI_ASSEMBLY_PLATFORM := $(2)

endef

# Set the version of the assembly.
#
# gb_CliPolicyTarget_set_version assembly version
define gb_CliPolicyTarget_set_version
$(call gb_CliPolicyTarget_get_target,$(1)) : CLI_ASSEMBLY_VERSION := $(2)

endef

# CliAssemblyTarget class

# Handles creation and delivery of signed multi-file CLI assembly.
#
# This class groups common parts of the process of creation and delivery
# of a signed assembly. It does not specify how to create the "core" dll
# (which must be supplied externally). This is because there are various
# ways how to create the assembly, so they are handled by separate
# classes (CliAssembly, CliLibrary, CliUnoApi).
#
# The delivered files are
# * the assembly itself (.dll)
# * the associated policy (.dll)
# * the associated configuration file (.config)

gb_CliAssemblyTarget_KEYFILE_DEFAULT := $(gb_CliPolicyTarget_KEYFILE_DEFAULT)

$(dir $(call gb_CliAssemblyTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliAssemblyTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliAssemblyTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),CLI,5)
	$(call gb_Helper_abbreviate_dirs,\
		touch $@ \
	)

.PHONY : $(call gb_CliAssemblyTarget_get_clean_target,%)
$(call gb_CliAssemblyTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CLI,5)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(CLI_ASSEMBLY_OUTFILES) $(call gb_CliAssemblyTarget_get_target,$*) \
	)

# Define an assembly.
#
# gb_CliAssemblyTarget_CliAssemblyTarget assembly dll
define gb_CliAssemblyTarget_CliAssemblyTarget
$(call gb_CliPolicyTarget_CliPolicyTarget,$(1))

$(call gb_CliAssemblyTarget_get_clean_target,$(1)) : CLI_ASSEMBLY_OUTFILES :=

$(call gb_CliAssemblyTarget_get_target,$(1)) : $(call gb_CliPolicyTarget_get_target,$(1))
$(call gb_CliAssemblyTarget_get_target,$(1)) :| $(dir (call gb_CliAssemblyTarget_get_target,$(1))).dir
$(call gb_CliAssemblyTarget_get_clean_target,$(1)) : $(call gb_CliPolicyTarget_get_clean_target,$(1))

$(call gb_CliAssemblyTarget__add_file,$(1),$(2))

endef

define gb_CliAssemblyTarget__add_file_impl
$(call gb_Deliver_add_deliverable,$(2),$(3),$(1))

$(call gb_CliAssemblyTarget_get_clean_target,$(1)) : CLI_ASSEMBLY_OUTFILES += $(2)
$(call gb_CliAssemblyTarget_get_target,$(1)) : $(2)
$(2) : $(3)

endef

define gb_CliAssemblyTarget__add_file
$(call gb_CliAssemblyTarget__add_file_impl,$(1),$(call gb_CliAssemblyTarget_get_outdir_target,$(notdir $(2))),$(2))

endef

define gb_CliAssemblyTarget__set_configfile_impl
$(call gb_CliPolicyTarget_set_configfile,$(1),$(2))
$(call gb_CliAssemblyTarget__add_file,$(1),$(2))

endef

define gb_CliAssemblyTarget__set_configfile
$(call gb_CliConfigTarget_CliConfigTarget,$(2),$(3))
$(call gb_CliAssemblyTarget__set_configfile_impl,$(1),$(call gb_CliConfigTarget_get_target,$(2)))

endef

# Set the configuration file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliAssemblyTarget_set_configfile assembly configfile
define gb_CliAssemblyTarget_set_configfile
$(call gb_CliAssemblyTarget__set_configfile,$(1),$(patsubst %_config,%,$(2)),$(SRCDIR)/$(2))

endef

# Set the sign key file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliAssemblyTarget_set_keyfile assembly keyfile
define gb_CliAssemblyTarget_set_keyfile
$(call gb_CliPolicyTarget_set_keyfile,$(1),$(2))

endef

# Set the platform of the assembly.
#
# Only applicable for C++ assemblies (CliLibrary).
#
# gb_CliAssemblyTarget_set_platform assembly platform
define gb_CliAssemblyTarget_set_platform
$(call gb_CliPolicyTarget_set_platform,$(1),$(2))

endef

# Set the policy file for the assembly.
#
# The file is given by complete path.
#
# gb_CliAssemblyTarget_set_policy assembly policyfile version
define gb_CliAssemblyTarget_set_policy
$(call gb_CliPolicyTarget_set_version,$(1),$(3))
$(call gb_CliPolicyTarget_set_name,$(1),$(2))
$(call gb_CliAssemblyTarget__add_file,$(1),$(call gb_CliPolicyTarget_get_assembly_target,$(2)))

endef

# vim: set noet sw=4 ts=4:
