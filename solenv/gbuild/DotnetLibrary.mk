# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

##############################
# DotnetLibrary Target Class #
##############################

####### Constant Strings #########

gb_DotnetLibrary__CONFIG_RELEASE := -c Release
gb_DotnetLibrary__CONFIG_DEBUG := -c Debug

####### Helper Functions #########

define gb_DotnetLibrary__get_build_config
$(if $(or \
		$(filter TRUE,$(strip $(debug))), \
		$(filter TRUE,$(strip $(ENABLE_DBGUTIL))) \
	), \
	$(gb_DotnetLibrary__CONFIG_DEBUG), \
	$(gb_DotnetLibrary__CONFIG_RELEASE))

endef

define gb_DotnetLibrary__escape_quotes
$(strip $(subst ",\",$(1)))

endef

####### Build and Clean Targets #########

.PHONY : $(call gb_DotnetLibrary_get_clean_target,%)
$(call gb_DotnetLibrary_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),NET,4)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_DotnetLibrary_get_target,$*))

$(call gb_DotnetLibrary_get_target,%) :
	$(call gb_Output_announce,$*,$(true),NET,4)
	$(call gb_Trace_StartRange,$*,NET)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(call gb_DotnetLibrary_get_workdir,$*) && \
		P=$(DOTNET_PROJECT_FILE) && \
		echo "<Project Sdk=\"Microsoft.NET.Sdk\">" > $$P && \
		echo "<PropertyGroup>" >> $$P && \
		echo "$(DOTNET_PROPERTY_ELEMENTS)" >> $$P && \
		echo "</PropertyGroup>" >> $$P && \
		echo "<ItemGroup>" >> $$P && \
		echo "$(DOTNET_ITEM_ELEMENTS)" >> $$P && \
		echo "</ItemGroup>" >> $$P && \
		echo "</Project>" >> $$P && \
		dotnet build $$P $(DOTNET_BUILD_FLAGS) \
			-o $(call gb_DotnetLibrary_get_workdir,$*) \
			> $@.log 2>&1 || \
			(cat $@.log \
				&& echo \
				&& echo "A library failed to build. To retry the build, use:" \
				&& echo "    make DotnetLibrary_$*" \
				&& echo "cd into the module directory to run the build faster" \
				&& echo \
				&& false) && \
		touch $@)
	$(call gb_Trace_EndRange,$*,NET)

####### Library Target Constructors #########

define gb_DotnetLibrary__common_ctor
$(call gb_DotnetLibrary_get_target,$(1)) : DOTNET_BUILD_FLAGS := $(strip $(call gb_DotnetLibrary__get_build_config))
$(call gb_DotnetLibrary_get_target,$(1)) : DOTNET_PROPERTY_ELEMENTS := <TargetFramework>netstandard20</TargetFramework>
$(call gb_DotnetLibrary_get_target,$(1)) : DOTNET_ITEM_ELEMENTS :=
$(call gb_DotnetLibrary_get_target,$(1)) : DOTNET_PROJECT_FILE := $(call gb_DotnetLibrary_get_workdir,$(1))/$(1).$(2)

$(eval $(call gb_Module_register_target, \
	$(call gb_DotnetLibrary_get_target,$(1)), \
	$(call gb_DotnetLibrary_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),DotnetLibrary)

endef

# Generates one csproj file from given inputs and builds it
# call gb_DotnetLibrary_CsLibrary,targetname
define gb_DotnetLibrary_CsLibrary
$(call gb_DotnetLibrary__common_ctor,$(1),csproj)

endef

# Generates one fsproj file from given inputs and builds it
# call gb_DotnetLibrary_FsLibrary,targetname
define gb_DotnetLibrary_FsLibrary
$(call gb_DotnetLibrary__common_ctor,$(1),fsproj)

endef

# Generates one vbproj file from given inputs and builds it
# call gb_DotnetLibrary_VbLibrary,targetname
define gb_DotnetLibrary_VbLibrary
$(call gb_DotnetLibrary__common_ctor,$(1),vbproj)

endef

####### Target Property Setters #########

# Add flags used for compilation
# call gb_DotnetLibrary_add_build_flags,target,flags
define gb_DotnetLibrary_add_build_flags
$(call gb_DotnetLibrary_get_target,$(1)) : DOTNET_BUILD_FLAGS += $(2)

endef

# Add <PropertyGroup> elements to the project file
# call gb_DotnetLibrary_add_properties,target,properties
define gb_DotnetLibrary_add_properties
$(call gb_DotnetLibrary_get_target,$(1)) : DOTNET_PROPERTY_ELEMENTS += $(strip $(call gb_DotnetLibrary__escape_quotes,$(2)))

endef

# Add <ItemGroup> elements to the project file
# call gb_DotnetLibrary_add_items,target,items
define gb_DotnetLibrary_add_items
$(call gb_DotnetLibrary_get_target,$(1)) : DOTNET_ITEM_ELEMENTS += $(strip $(call gb_DotnetLibrary__escape_quotes,$(2)))

endef

# Add one source file to the project file
# This add it to the project, and makes it a build dependency
# so the library is rebuilt if the source changes
# call gb_DotnetLibrary_add_source,target,source
define gb_DotnetLibrary_add_source
$(call gb_DotnetLibrary_get_target,$(1)) : $(SRCDIR)/$(strip $(2))
$(call gb_DotnetLibrary_add_items,$(1),<Compile Include="$(SRCDIR)/$(strip $(2))"/>)

endef

# Add source files to the project file
# call gb_DotnetLibrary_add_sources,target,sources
define gb_DotnetLibrary_add_sources
$(foreach source,$(2),$(call gb_DotnetLibrary_add_source,$(1),$(source)))

endef

# Link to a DotnetLibrary_CsLibrary target
# call gb_DotnetLibrary_link_cs_project,target,project
define gb_DotnetLibrary_link_cs_project
$(call gb_DotnetLibrary_get_target,$(1)) : $(call gb_DotnetLibrary_get_target,$(strip $(2)))
$(call gb_DotnetLibrary_add_items,$(1),<ProjectReference Include="$(call gb_DotnetLibrary_get_workdir,$(strip $(2)))/$(strip $(2)).csproj"/>)

endef

# Link to a DotnetLibrary_FsLibrary target
# call gb_DotnetLibrary_link_fs_project,target,project
define gb_DotnetLibrary_link_fs_project
$(call gb_DotnetLibrary_get_target,$(1)) : $(call gb_DotnetLibrary_get_target,$(strip $(2)))
$(call gb_DotnetLibrary_add_items,$(1),<ProjectReference Include="$(call gb_DotnetLibrary_get_workdir,$(strip $(2)))/$(strip $(2)).fsproj"/>)

endef

# Link to a DotnetLibrary_VbLibrary target
# call gb_DotnetLibrary_link_vb_project,target,project
define gb_DotnetLibrary_link_vb_project
$(call gb_DotnetLibrary_get_target,$(1)) : $(call gb_DotnetLibrary_get_target,$(strip $(2)))
$(call gb_DotnetLibrary_add_items,$(1),<ProjectReference Include="$(call gb_DotnetLibrary_get_workdir,$(strip $(2)))/$(strip $(2)).vbproj"/>)

endef

# vim: set noet sw=4 ts=4:
