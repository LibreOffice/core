# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# CliLibraryTarget class

gb_CliLibraryTarget_EXT := $(gb_CliAssembly_POLICYEXT)

gb_CliLibraryTarget_CSCFLAGS := \
	-noconfig \
	-warnaserror+

gb_CliLibraryTarget_CSCFLAGS_DEBUG := \
	-checked+ \
	-define:DEBUG \
	-define:TRACE \

ifeq ($(strip $(debug)),)
ifeq ($(strip $(PRODUCT)),)
gb_CliLibraryTarget__get_csflags = $(gb_CliLibraryTarget_CSCFLAGS) $(gb_CliLibraryTarget_CSCFLAGS_DEBUG)
else
gb_CliLibraryTarget__get_csflags = $(gb_CliLibraryTarget_CSCFLAGS) -o
endif
else
gb_CliLibraryTarget__get_csflags = $(gb_CliLibraryTarget_CSCFLAGS) $(gb_CliLibraryTarget_CSCFLAGS_DEBUG) -debug+
endif

gb_CliLibraryTarget__get_source = $(SRCDIR)/$(1).cs
gb_CliLibraryTarget__get_generated_source = $(WORKDIR)/$(1).cs

define gb_CliLibraryTarget__command
$(call gb_Output_announce,$(2),$(true),CSC,3)
$(call gb_Helper_abbreviate_dirs,\
	csc \
		$(call gb_CliLibraryTarget__get_csflags) \
		$(CLI_CSCFLAGS) \
		-target:library \
		-out:$(1) \
		-reference:System.dll \
		$(foreach assembly,$(CLI_ASSEMBLIES),-reference:$(assembly).dll) \
		$(CLI_SOURCES) \
)
endef

$(dir $(call gb_CliLibraryTarget_get_target,%)).dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(dir $(call gb_CliLibraryTarget_get_target,%))%/.dir :
	$(if $(wildcard $(dir $@)),,mkdir -p $(dir $@))

$(call gb_CliLibraryTarget_get_target,%) :
	$(call gb_CliLibraryTarget__command,$@,$*,$<,$?,$^)

.PHONY : $(call gb_CliLibraryTarget_get_clean_target,%)
$(call gb_CliLibraryTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CSC,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CliLibraryTarget_get_target,$*) \
	)

# Compiles one or more C# source files
#
# gb_CliLibraryTarget_CliLibraryTarget target
define gb_CliLibraryTarget_CliLibraryTarget
$(call gb_CliLibraryTarget_get_target,$(1)) : CLI_ASSEMBLIES :=
$(call gb_CliLibraryTarget_get_target,$(1)) : CLI_SOURCES :=
$(call gb_CliLibraryTarget_get_target,$(1)) : CLI_CSCFLAGS :=

$(call gb_CliLibraryTarget_get_target,$(1)) :| $(dir $(call gb_CliLibraryTarget_get_target,$(1))).dir

endef

define gb_CliLibraryTarget_use_assembly
$(call gb_CliLibraryTarget_get_target,$(1)) : CLI_ASSEMBLIES += $(call gb_CliLibrary_get_target,$(2))
$(call gb_CliLibraryTarget_get_target,$(1)) : $(call gb_CliLibrary_get_target,$(2))

endef

define gb_CliLibraryTarget_use_assemblies
$(foreach assembly,$(2),$(call gb_CliLibraryTarget_use_assembly,$(1),$(assembly)))

endef

# Add a source file to compile
define gb_CliLibraryTarget_add_csfile
$(call gb_CliLibraryTarget_get_target,$(1)) : CLI_SOURCES += $(call gb_CliLibraryTarget__get_source,$(2))
$(call gb_CliLibraryTarget_get_target,$(1)) : $(call gb_CliLibraryTarget__get_source,$(2))

endef

# Add source files to compile
define gb_CliLibraryTarget_add_csfiles
$(foreach csfile,$(2),$(call gb_CliLibraryTarget_add_csfile,$(1),$(csfile)))

endef

# Add a generated source file to compile
define gb_CliLibraryTarget_add_generated_csfile
$(call gb_CliLibraryTarget_get_target,$(1)) : CLI_SOURCES += $(call gb_CliLibraryTarget__get_generated_source,$(2))
$(call gb_CliLibraryTarget_get_target,$(1)) : $(call gb_CliLibraryTarget__get_generated_source,$(2))

endef

# Add generated source files to compile
define gb_CliLibraryTarget_add_generated_csfiles
$(foreach csfile,$(2),$(call gb_CliLibraryTarget_add_generated_csfile,$(1),$(csfile)))

endef

# Add flags used for compilation
define gb_CliLibraryTarget_add_csflags
$(call gb_CliLibraryTarget_get_target,$(1)) : CLI_CSCFLAGS += $(2)

endef

# CliLibrary class

gb_CliLibrary_EXT := $(gb_CliLibraryTarget_EXT)

# Compiles one or more C# source files
#
# gb_CliLibrary_CliLibrary target
define gb_CliLibrary_CliLibrary
$(call gb_CliLibraryTarget_CliLibraryTarget,$(1))
$(call gb_CliAssembly_CliAssembly,$(1))

$(call gb_CliLibrary_get_target,$(1)) : $(call gb_CliLibraryTarget_get_target,$(1))
$(call gb_CliLibrary_get_target,$(1)) :| $(call gb_CliAssembly_get_target,$(1))
$(call gb_CliLibrary_get_target,$(1)) :| $(dir $(call gb_CliLibrary_get_target,$(1))).dir
$(call gb_CliLibrary_get_clean_target,$(1)) : $(call gb_CliLibraryTarget_get_clean_target,$(1))
$(call gb_CliLibrary_get_clean_target,$(1)) : $(call gb_CliAssembly_get_clean_target,$(1))

$(call gb_Deliver_add_deliverable,$(call gb_CliLibrary_get_target,$(1)),$(call gb_CliLibraryTarget_get_target,$(1)),$(1))

$$(eval $$(call gb_Module_register_target,$(call gb_CliLibrary_get_target,$(1)),$(call gb_CliLibrary_get_clean_target,$(1))))

endef

define gb_CliLibrary_set_configfile
$(call gb_CliAssembly_set_configfile,$(1),$(2))

endef

define gb_CliLibrary_set_keyfile
$(call gb_CliAssembly_set_keyfile,$(1),$(2))

endef

define gb_CliLibrary_set_policy
$(call gb_CliAssembly_set_policy,$(1),$(2),$(3))

endef

define gb_CliLibrary_use_assembly
$(call gb_CliLibraryTarget_use_assembly,$(1),$(2))

endef

define gb_CliLibrary_use_assemblies
$(call gb_CliLibraryTarget_use_assemblies,$(1),$(2))

endef

# Add a source file to compile
define gb_CliLibrary_add_csfile
$(call gb_CliLibraryTarget_add_csfile,$(1),$(2))

endef

# Add source files to compile
define gb_CliLibrary_add_csfiles
$(call gb_CliLibraryTarget_add_csfiles,$(1),$(2))

endef

# Add a generated source file to compile
define gb_CliLibrary_add_generated_csfile
$(call gb_CliLibraryTarget_add_generated_csfile,$(1),$(2))

endef

# Add generated source files to compile
define gb_CliLibrary_add_generated_csfiles
$(call gb_CliLibraryTarget_add_generated_csfiles,$(1),$(2))

endef

# Add flags used for compilation
define gb_CliLibrary_add_csflags
$(call gb_CliLibraryTarget_add_csflags,$(1),$(2))

endef

# vim: set noet sw=4 ts=4:
