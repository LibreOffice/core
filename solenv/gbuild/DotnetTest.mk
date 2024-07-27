# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

###########################
# DotnetTest Target Class #
###########################

####### Constant Strings #########

gb_DotnetTest_CS := cs
gb_DotnetTest_FS := fs
gb_DotnetTest_VB := vb

define gb_DotnetTest__TEST_NUGETS
<PackageReference Include="NUnit" Version="4.1.0" /> \
<PackageReference Include="NUnit3TestAdapter" Version="4.5.0" /> \
<PackageReference Include="Microsoft.NET.Test.Sdk" Version="17.10.0" /> \

endef

####### Build and Clean Targets #########

# Template for a target to generate the project file for a DotnetTest
define gb_DotnetTest__project_target
$$(gb_DotnetTest_$(1)_project) :
	$$(shell mkdir -p $$(dir $$@))
	$$(file  >$$@,<Project Sdk="Microsoft.NET.Sdk">)
	$$(file >>$$@,<PropertyGroup>)
	$$(file >>$$@,$$(DOTNET_PROPERTY_ELEMENTS))
	$$(file >>$$@,</PropertyGroup>)
	$$(file >>$$@,<ItemGroup>)
	$$(file >>$$@,$$(DOTNET_ITEM_ELEMENTS))
	$$(file >>$$@,</ItemGroup>)
	$$(file >>$$@,</Project>)

endef

# Template for a target to build and run a DotnetTest
define gb_DotnetTest__build_target
$$(call gb_DotnetTest_get_target,$(1)) : $$(gb_DotnetTest_$(1)_project)
	$$(call gb_Output_announce,$(1),$(true),NET,4)
	$$(call gb_Trace_StartRange,$(1),NET)
	$$(call gb_Helper_abbreviate_dirs,\
		$$(call gb_Helper_print_on_error,\
			"$$(DOTNET)" test $$< $$(DOTNET_BUILD_FLAGS) -o $$(dir $$@),\
			$$(gb_DotnetTest_workdir)/$(1)/log))
	$$(call gb_Trace_EndRange,$(1),NET)

endef

# Template for a target to clean a DotnetTest
define gb_DotnetTest__clean_target
$$(call gb_DotnetTest_get_clean_target,$(1)) :
	$$(call gb_Output_announce,$(1),$(false),NET,4)
	$$(call gb_Helper_abbreviate_dirs,\
		rm -rf $$(gb_DotnetTest_$(1)_project))

endef

####### Test Target Constructor #########

# Generates one test project for the given language, instantiating 
# the project file, build/run and clean targets from above templates
# call gb_DotnetTest_DotnetTest,targetname,language
define gb_DotnetTest_DotnetTest
gb_DotnetTest_$(1)_language := $(2)
gb_DotnetTest_$(1)_project := $(gb_DotnetTest_workdir)/$(1)/$(1).$(2)proj

$$(gb_DotnetTest_$(1)_project) : DOTNET_PROPERTY_ELEMENTS := <TargetFramework>net8.0</TargetFramework>
$$(gb_DotnetTest_$(1)_project) : DOTNET_PROPERTY_ELEMENTS += <IsPackable>false</IsPackable>
$$(gb_DotnetTest_$(1)_project) : DOTNET_PROPERTY_ELEMENTS += <IsTestProject>true</IsTestProject>
$$(gb_DotnetTest_$(1)_project) : DOTNET_PROPERTY_ELEMENTS += <AssemblyName>$(1)</AssemblyName>
$$(gb_DotnetTest_$(1)_project) : DOTNET_ITEM_ELEMENTS := $(gb_DotnetTest__TEST_NUGETS)
$$(eval $$(call gb_DotnetTest__project_target,$(1)))

$$(call gb_DotnetTest_get_target,$(1)) : DOTNET_BUILD_FLAGS := $(if $(ENABLE_DEBUG),-c Debug,-c Release)
.PHONY : $$(call gb_DotnetTest_get_target,$(1))
$$(eval $$(call gb_DotnetTest__build_target,$(1)))

.PHONY : $$(call gb_DotnetTest_get_clean_target,$(1))
$$(eval $$(call gb_DotnetTest__clean_target,$(1)))

$$(eval $$(call gb_Module_register_target, \
	$(call gb_DotnetTest_get_target,$(1)), \
	$(call gb_DotnetTest_get_clean_target,$(1))))
$(call gb_Helper_make_userfriendly_targets,$(1),DotnetTest)

endef

####### Target Property Setters #########

# Add flags used for compilation
# call gb_DotnetTest_add_build_flags,target,flags
define gb_DotnetTest_add_build_flags
$(call gb_DotnetTest_get_target,$(1)) : DOTNET_BUILD_FLAGS += $(2)

endef

# Add <PropertyGroup> elements to the project file
# call gb_DotnetTest_add_properties,target,properties
define gb_DotnetTest_add_properties
$(gb_DotnetTest_$(1)_project) : DOTNET_PROPERTY_ELEMENTS += $(2)

endef

# Add <ItemGroup> elements to the project file
# call gb_DotnetTest_add_items,target,items
define gb_DotnetTest_add_items
$(gb_DotnetTest_$(1)_project) : DOTNET_ITEM_ELEMENTS += $(2)

endef

# Add one source file to the project file
# This adds it to the project, and makes it a build dependency
# so the test is rebuilt if the source changes
# call gb_DotnetTest_add_source,target,source
define gb_DotnetTest_add_source
$(gb_DotnetTest_$(1)_project) : $(SRCDIR)/$(2).$(gb_DotnetTest_$(1)_language)
$(call gb_DotnetTest_add_items,$(1),<Compile Include="$(SRCDIR)/$(2).$(gb_DotnetTest_$(1)_language)"/>)

endef

# Add source files to the project file
# This adds them to the project, and makes it them build dependency
# so the test is rebuilt if the sources change
# call gb_DotnetTest_add_sources,target,sources
define gb_DotnetTest_add_sources
$(foreach source,$(2),$(call gb_DotnetTest_add_source,$(1),$(source)))

endef

# Link to a DotnetLibrary target
# call gb_DotnetTest_link_library,target,library
define gb_DotnetTest_link_library
$(gb_DotnetTest_$(1)_project) : $(call gb_DotnetLibrary_get_target,$(2))
$(call gb_DotnetTest_add_items,$(1),<ProjectReference Include="$(gb_DotnetLibrary_$(2)_project)"/>)

endef

# vim: set noet sw=4 ts=4:
