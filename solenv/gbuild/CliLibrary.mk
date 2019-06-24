# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# CliLibrary class

gb_CliLibrary_EXT := $(gb_CliAssembly_POLICYEXT)

gb_CliLibrary_CSCFLAGS := \
	-noconfig \
	-nologo \

gb_CliLibrary_CSCFLAGS_DEBUG := \
	-checked+ \
	-define:DEBUG \
	-define:TRACE \

ifeq ($(strip $(debug)),)
ifeq ($(strip $(ENABLE_DBGUTIL)),TRUE)
gb_CliLibrary__get_csflags = $(gb_CliLibrary_CSCFLAGS) $(gb_CliLibrary_CSCFLAGS_DEBUG)
else
gb_CliLibrary__get_csflags = $(gb_CliLibrary_CSCFLAGS) -o
endif
else
gb_CliLibrary__get_csflags = $(gb_CliLibrary_CSCFLAGS) $(gb_CliLibrary_CSCFLAGS_DEBUG) -debug+
endif

gb_CliLibrary__get_source = $(SRCDIR)/$(1).cs
gb_CliLibrary__get_generated_source = $(WORKDIR)/$(1).cs

# csc has silly problems handling files passed on command line
define gb_CliLibrary__command
$(call gb_Output_announce,$(2),$(true),CSC,3)
	csc \
		$(call gb_CliLibrary__get_csflags) \
		$(CLI_CSCFLAGS) \
		-target:library \
		-out:$(1) \
		-keyfile:$(CLI_KEYFILE) \
		-reference:System.dll \
		$(foreach assembly,$(CLI_ASSEMBLIES),-reference:$(assembly)) \
		$(subst /,\,$(CLI_SOURCES)) \


endef

.PHONY : $(call gb_CliLibrary_get_clean_target,%)
$(call gb_CliLibrary_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),CSC,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_CliLibrary_get_target,$*) \
	)


# Compiles one or more C# source files
#
# gb_CliLibrary_CliLibrary target
define gb_CliLibrary_CliLibrary
$(call gb_CliAssembly_CliAssembly,$(1),$(call gb_CliLibrary_get_target,$(1)))

$(call gb_CliLibrary_get_target,$(1)) : CLI_ASSEMBLIES :=
$(call gb_CliLibrary_get_target,$(1)) : CLI_SOURCES :=
$(call gb_CliLibrary_get_target,$(1)) : CLI_CSCFLAGS :=
$(call gb_CliLibrary_get_target,$(1)) : CLI_KEYFILE :=


$(call gb_CliLibrary_get_target,$(1)) :| $(call gb_CliAssembly_get_target,$(1))
$(call gb_CliLibrary_get_target,$(1)) :| $(dir $(call gb_CliLibrary_get_target,$(1))).dir
$(call gb_CliLibrary_get_clean_target,$(1)) : $(call gb_CliAssembly_get_clean_target,$(1))

$$(eval $$(call gb_Module_register_target,CliLibrary_$(1),$(call gb_CliLibrary_get_target,$(1)),$(call gb_CliLibrary_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),CliLibrary)

$(call gb_CliLibrary_get_target,$(1)) :
	$$(call gb_CliLibrary__command,$$@,$(1))

endef

define gb_CliLibrary_set_configfile
$(call gb_CliAssembly_set_configfile,$(1),$(2),$(3))

endef

define gb_CliLibrary_set_keyfile
$(call gb_CliLibrary_get_target,$(1)) : CLI_KEYFILE := $(2)
$(call gb_CliAssembly_set_keyfile,$(1),$(2))

endef

define gb_CliLibrary_set_policy
$(call gb_CliAssembly_set_policy,$(1),$(2),$(3))

endef

define gb_CliLibrary_use_assembly
$(call gb_CliLibrary_get_target,$(1)) : CLI_ASSEMBLIES += $(call gb_CliLibrary_get_target,$(2))
$(call gb_CliLibrary_get_target,$(1)) : $(call gb_CliLibrary_get_target,$(2))

endef

define gb_CliLibrary_use_assemblies
$(foreach assembly,$(2),$(call gb_CliLibrary_use_assembly,$(1),$(assembly)))

endef

# Add a source file to compile
define gb_CliLibrary_add_csfile
$(call gb_CliLibrary_get_target,$(1)) : CLI_SOURCES += $(call gb_CliLibrary__get_source,$(2))
$(call gb_CliLibrary_get_target,$(1)) : $(call gb_CliLibrary__get_source,$(2))

endef

# Add source files to compile
define gb_CliLibrary_add_csfiles
$(foreach csfile,$(2),$(call gb_CliLibrary_add_csfile,$(1),$(csfile)))

endef

# Add a generated source file to compile
define gb_CliLibrary_add_generated_csfile
$(call gb_CliLibrary_get_target,$(1)) : CLI_SOURCES += $(call gb_CliLibrary__get_generated_source,$(2))
$(call gb_CliLibrary_get_target,$(1)) : $(call gb_CliLibrary__get_generated_source,$(2))

endef

# Add generated source files to compile
define gb_CliLibrary_add_generated_csfiles
$(foreach csfile,$(2),$(call gb_CliLibrary_add_generated_csfile,$(1),$(csfile)))

endef

# Add flags used for compilation
define gb_CliLibrary_add_csflags
$(call gb_CliLibrary_get_target,$(1)) : CLI_CSCFLAGS += $(2)

endef

# vim: set noet sw=4 ts=4:
