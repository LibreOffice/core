# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# CliCSharpTarget class

# platform defined
#  gb_CliCSharpTarget__command

gb_CliCSharpTarget__get_source = $(SRCDIR)/$(1).cs
gb_CliCSharpTarget__get_generated_source = $(WORKDIR)/$(1).cs

$(dir $(call gb_CliCSharpTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliCSharpTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliCSharpTarget_get_target,%) :
	$(call gb_CliCSharpTarget__command,$@,$*,$<,$?,$^)

.PHONY : $(call gb_CliCSharpTarget_get_clean_target,%)
$(call gb_CliCSharpTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CSC,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CliCSharpTarget_get_target,$*) \
	)

# Compile one or more C# source files.
#
# gb_CliCSharpTarget_CliCSharpTarget assembly
define gb_CliCSharpTarget_CliCSharpTarget
$(call gb_CliCSharpTarget_get_target,$(1)) : CLI_ASSEMBLIES :=
$(call gb_CliCSharpTarget_get_target,$(1)) : CLI_SOURCES :=
$(call gb_CliCSharpTarget_get_target,$(1)) : CLI_CSCFLAGS :=
$(call gb_CliCSharpTarget_get_target,$(1)) : CLI_KEYFILE :=

$(call gb_CliCSharpTarget_get_target,$(1)) :| $(dir $(call gb_CliCSharpTarget_get_target,$(1))).dir

endef

# Use another assembly for compilation.
#
# gb_CliCSharpTarget_use_assembly assembly dep
define gb_CliCSharpTarget_use_assembly
$(call gb_CliCSharpTarget_get_target,$(1)) : CLI_ASSEMBLIES += $(call gb_CliAssemblyTarget_get_dll,$(2))
$(call gb_CliCSharpTarget_get_target,$(1)) : $(call gb_CliAssemblyTarget_get_target,$(2))

endef

# Use other assemblies for compilation.
#
# gb_CliCSharpTarget_use_assembly assembly dep(s)
define gb_CliCSharpTarget_use_assemblies
$(foreach assembly,$(2),$(call gb_CliCSharpTarget_use_assembly,$(1),$(assembly)))

endef

# Add a source file to compile.
#
# gb_CliCSharpTarget_add_csfile assembly csfile
define gb_CliCSharpTarget_add_csfile
$(call gb_CliCSharpTarget_get_target,$(1)) : CLI_SOURCES += $(call gb_CliCSharpTarget__get_source,$(2))
$(call gb_CliCSharpTarget_get_target,$(1)) : $(call gb_CliCSharpTarget__get_source,$(2))

endef

# Add source files to compile.
#
# gb_CliCSharpTarget_add_csfiles assembly csfile(s)
define gb_CliCSharpTarget_add_csfiles
$(foreach csfile,$(2),$(call gb_CliCSharpTarget_add_csfile,$(1),$(csfile)))

endef

# Add a generated source file to compile.
#
# gb_CliCSharpTarget_add_generated_csfile assembly csfile
define gb_CliCSharpTarget_add_generated_csfile
$(call gb_CliCSharpTarget_get_target,$(1)) : CLI_SOURCES += $(call gb_CliCSharpTarget__get_generated_source,$(2))
$(call gb_CliCSharpTarget_get_target,$(1)) : $(call gb_CliCSharpTarget__get_generated_source,$(2))

endef

# Add generated source files to compile.
#
# gb_CliCSharpTarget_add_generated_csfiles assembly csfile(s)
define gb_CliCSharpTarget_add_generated_csfiles
$(foreach csfile,$(2),$(call gb_CliCSharpTarget_add_generated_csfile,$(1),$(csfile)))

endef

# Add flags used for compilation.
# 
# gb_CliCSharpTarget_add_csflags assembly flags
define gb_CliCSharpTarget_add_csflags
$(call gb_CliCSharpTarget_get_target,$(1)) : CLI_CSCFLAGS += $(2)

endef

# CliAssembly class

# Handles creation of a CLI assembly from C# sources.

$(dir $(call gb_CliAssembly_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliAssembly_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliAssembly_get_target,%) :
	touch $@

.PHONY : $(call gb_CliAssembly_get_clean_target,%)
$(call gb_CliAssembly_get_clean_target,%) :
	rm -f $(call gb_CliAssembly_get_target,$*)

# Create a CLI assembly from C# sources.
#
# gb_CliAssembly_CliAssembly assembly
define gb_CliAssembly_CliAssembly
$(call gb_CliCSharpTarget_CliCSharpTarget,$(1))
$(call gb_CliAssemblyTarget_CliAssemblyTarget,$(1),$(call gb_CliCSharpTarget_get_target,$(1)))

$(call gb_CliAssembly_get_target,$(1)) : $(call gb_CliCSharpTarget_get_target,$(1))
$(call gb_CliAssembly_get_target,$(1)) : $(call gb_CliAssemblyTarget_get_target,$(1))
$(call gb_CliAssembly_get_target,$(1)) :| $(dir $(call gb_CliAssembly_get_target,$(1))).dir
$(call gb_CliAssembly_get_clean_target,$(1)) : $(call gb_CliCSharpTarget_get_clean_target,$(1))
$(call gb_CliAssembly_get_clean_target,$(1)) : $(call gb_CliAssemblyTarget_get_clean_target,$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_CliAssembly_get_target,$(1)),$(call gb_CliAssembly_get_clean_target,$(1))))

endef

# Set the configuration file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliAssembly_set_configfile assembly configfile
define gb_CliAssembly_set_configfile
$(call gb_CliAssemblyTarget_set_configfile,$(1),$(2))

endef

# Set the sign key file for the assembly.
#
# The file is given by complete path.
# 
# gb_CliAssembly_set_keyfile assembly keyfile
define gb_CliAssembly_set_keyfile
$(call gb_CliCSharpTarget_get_target,$(1)) : CLI_KEYFILE := $(2)
$(call gb_CliAssemblyTarget_set_keyfile,$(1),$(2))

endef

# Set the policy file for the assembly.
#
# The file is given by complete path.
#
# gb_CliAssembly_set_policy assembly policyfile version
define gb_CliAssembly_set_policy
$(call gb_CliAssemblyTarget_set_policy,$(1),$(2),$(3))

endef

# Use another assembly for compilation.
#
# gb_CliAssembly_use_assembly assembly dep
define gb_CliAssembly_use_assembly
$(call gb_CliCSharpTarget_use_assembly,$(1),$(2))

endef

# Use other assemblies for compilation.
#
# gb_CliAssembly_use_assemblies assembly dep(s)
define gb_CliAssembly_use_assemblies
$(call gb_CliCSharpTarget_use_assemblies,$(1),$(2))

endef

# Add a C# source file to compile.
#
# The file is specified without extension and with path relative to
# $(SRCDIR).
#
# gb_CliAssembly_add_csfile assembly csfile
define gb_CliAssembly_add_csfile
$(call gb_CliCSharpTarget_add_csfile,$(1),$(2))

endef

# Add C# source file(s) to compile.
#
# The files are specified without extension and with path relative to
# $(SRCDIR).
#
# gb_CliAssembly_add_csfiles assembly csfile(s)
define gb_CliAssembly_add_csfiles
$(call gb_CliCSharpTarget_add_csfiles,$(1),$(2))

endef

# Add a generated C# source file to compile.
#
# The file is specified without extension and with path relative to
# $(WORKDIR).
#
# gb_CliAssembly_add_generated_csfile assembly csfile
define gb_CliAssembly_add_generated_csfile
$(call gb_CliCSharpTarget_add_generated_csfile,$(1),$(2))

endef

# Add generated C# source files to compile
#
# The files are specified without extension and with path relative to
# $(WORKDIR).
#
# gb_CliAssembly_add_generated_csfiles assembly csfile(s)
define gb_CliAssembly_add_generated_csfiles
$(call gb_CliCSharpTarget_add_generated_csfiles,$(1),$(2))

endef

# Add flags used for compilation.
#
# gb_CliAssembly_add_csflags assembly flags
define gb_CliAssembly_add_csflags
$(call gb_CliCSharpTarget_add_csflags,$(1),$(2))

endef

# vim: set noet sw=4 ts=4:
