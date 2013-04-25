# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# CliExecutableTarget class

gb_CliExecutableTarget_EXT := $(gb_CliAssembly_POLICYEXT)

gb_CliExecutableTarget_CSCFLAGS := \
	-unsafe \

gb_CliExecutableTarget_CSCFLAGS_DEBUG := \
	-debug \

ifeq ($(strip $(debug)),)
gb_CliExecutableTarget__get_csflags = $(gb_CliExecutableTarget_CSCFLAGS)
else
gb_CliExecutableTarget__get_csflags = $(gb_CliExecutableTarget_CSCFLAGS) $(gb_CliExecutableTarget_CSCFLAGS_DEBUG)
endif

gb_CliExecutableTarget__get_source = $(SRCDIR)/$(1).cs
gb_CliExecutableTarget__get_generated_source = $(WORKDIR)/$(1).cs

define gb_CliExecutableTarget__command
$(call gb_Output_announce,$(2),$(true),MKBUNDLE,3)
$(call gb_Helper_abbreviate_dirs,\
	gmcs $(call gb_Helper_windows_path, \
		$(call gb_CliExecutableTarget__get_csflags) \
		$(CLI_CSCFLAGS) \
		-out:$(1).exe \
		$(foreach assembly,$(CLI_ASSEMBLIES),-reference:$(assembly)) \
		$(CLI_SOURCES) \
	) \
)
$(call gb_Helper_abbreviate_dirs,\
	$(MKBUNDLE) $(call gb_Helper_windows_path, \
	        --deps \
		-o $(1) \
		$(1).exe \
		$(CLI_ASSEMBLIES)
	) \
)
endef

$(dir $(call gb_CliExecutableTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliExecutableTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliExecutableTarget_get_target,%) :
	$(call gb_CliExecutableTarget__command,$@,$*,$<,$?,$^)

.PHONY : $(call gb_CliExecutableTarget_get_clean_target,%)
$(call gb_CliExecutableTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),MKBUNDLE,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CliExecutableTarget_get_target,$*) \
	)

# Compiles one or more C# source files
#
# gb_CliExecutableTarget_CliExecutableTarget target
define gb_CliExecutableTarget_CliExecutableTarget
$(call gb_CliExecutableTarget_get_target,$(1)) : CLI_ASSEMBLIES :=
$(call gb_CliExecutableTarget_get_target,$(1)) : CLI_SOURCES :=
$(call gb_CliExecutableTarget_get_target,$(1)) : CLI_CSCFLAGS :=

$(call gb_CliExecutableTarget_get_target,$(1)) :| $(dir $(call gb_CliExecutableTarget_get_target,$(1))).dir

endef

define gb_CliExecutableTarget_use_assembly
$(call gb_CliExecutableTarget_get_target,$(1)) : CLI_ASSEMBLIES += $(call gb_CliLibrary_get_target,$(2))
$(call gb_CliExecutableTarget_get_target,$(1)) : $(call gb_CliLibrary_get_target,$(2))

endef

define gb_CliExecutableTarget_use_assemblies
$(foreach assembly,$(2),$(call gb_CliExecutableTarget_use_assembly,$(1),$(assembly)))

endef

# Add a source file to compile
define gb_CliExecutableTarget_add_csfile
$(call gb_CliExecutableTarget_get_target,$(1)) : CLI_SOURCES += $(call gb_CliExecutableTarget__get_source,$(2))
$(call gb_CliExecutableTarget_get_target,$(1)) : $(call gb_CliExecutableTarget__get_source,$(2))

endef

# Add source files to compile
define gb_CliExecutableTarget_add_csfiles
$(foreach csfile,$(2),$(call gb_CliExecutableTarget_add_csfile,$(1),$(csfile)))

endef

# Add a generated source file to compile
define gb_CliExecutableTarget_add_generated_csfile
$(call gb_CliExecutableTarget_get_target,$(1)) : CLI_SOURCES += $(call gb_CliExecutableTarget__get_generated_source,$(2))
$(call gb_CliExecutableTarget_get_target,$(1)) : $(call gb_CliExecutableTarget__get_generated_source,$(2))

endef

# Add generated source files to compile
define gb_CliExecutableTarget_add_generated_csfiles
$(foreach csfile,$(2),$(call gb_CliExecutableTarget_add_generated_csfile,$(1),$(csfile)))

endef

# Add flags used for compilation
define gb_CliExecutableTarget_add_csflags
$(call gb_CliExecutableTarget_get_target,$(1)) : CLI_CSCFLAGS += $(2)

endef

# CliExecutable class

gb_CliExecutable_EXT := $(gb_CliExecutableTarget_EXT)

# Compiles one or more C# source files and bundle with other libraries
#
# gb_CliExecutable_CliExecutable target
define gb_CliExecutable_CliExecutable
$(call gb_CliExecutableTarget_CliExecutableTarget,$(1))

$(call gb_CliExecutable_get_target,$(1)) : $(call gb_CliExecutableTarget_get_target,$(1))
$(call gb_CliExecutable_get_target,$(1)) :| $(dir $(call gb_CliExecutable_get_target,$(1))).dir
$(call gb_CliExecutable_get_clean_target,$(1)) : $(call gb_CliExecutableTarget_get_clean_target,$(1))

$(call gb_Deliver_add_deliverable,$(call gb_CliExecutable_get_target,$(1)),$(call gb_CliExecutableTarget_get_target,$(1)),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_CliExecutable_get_target,$(1)),$(call gb_CliExecutable_get_clean_target,$(1))))

$(call gb_Executable_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2)) \
 	| $(dir $(call gb_Executable_get_target,$(1))).dir
endef

define gb_CliExecutable_set_configfile
$(call gb_CliAssembly_set_configfile,$(1),$(2))

endef

define gb_CliExecutable_use_assembly
$(call gb_CliExecutableTarget_use_assembly,$(1),$(2))

endef

define gb_CliExecutable_use_assemblies
$(call gb_CliExecutableTarget_use_assemblies,$(1),$(2))

endef

# Add a source file to compile
define gb_CliExecutable_add_csfile
$(call gb_CliExecutableTarget_add_csfile,$(1),$(2))

endef

# Add source files to compile
define gb_CliExecutable_add_csfiles
$(call gb_CliExecutableTarget_add_csfiles,$(1),$(2))

endef

# Add a generated source file to compile
define gb_CliExecutable_add_generated_csfile
$(call gb_CliExecutableTarget_add_generated_csfile,$(1),$(2))

endef

# Add generated source files to compile
define gb_CliExecutable_add_generated_csfiles
$(call gb_CliExecutableTarget_add_generated_csfiles,$(1),$(2))

endef

# Add flags used for compilation
define gb_CliExecutable_add_csflags
$(call gb_CliExecutableTarget_add_csflags,$(1),$(2))

endef

# vim: set noet sw=4 ts=4:
