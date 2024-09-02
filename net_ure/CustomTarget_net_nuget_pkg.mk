# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,net_ure/net_uretypes))

net_nuget_pkg_DIR := $(gb_CustomTarget_workdir)/net_ure/net_nuget_pkg
net_nuget_pkg_ID := LibreOffice.Bindings
net_nuget_pkg_VERSION := 0.1.0
net_nuget_pkg_PROJECT := $(net_nuget_pkg_DIR)/nuget_pkg.csproj
net_nuget_pkg_FILE := $(net_nuget_pkg_DIR)/$(net_nuget_pkg_ID).$(net_nuget_pkg_VERSION).nupkg

$(call gb_CustomTarget_get_target,net_ure/net_nuget_pkg) : $(net_nuget_pkg_FILE)

$(net_nuget_pkg_PROJECT) : \
		$(call gb_DotnetLibrary_get_target,net_bridge) \
		$(call gb_DotnetLibrary_get_target,net_oootypes) \
		| $(net_nuget_pkg_DIR)/.dir
	$(file  >$@,<Project Sdk="Microsoft.NET.Sdk">)
	$(file >>$@,<PropertyGroup>)
	$(file >>$@,<TargetFramework>netstandard2.0</TargetFramework>)
	$(file >>$@,<PackageId>$(net_nuget_pkg_ID)</PackageId>)
	$(file >>$@,<Version>$(net_nuget_pkg_VERSION)</Version>)
	$(file >>$@,<Company>LibreOffice</Company>)
	$(file >>$@,<Description>Libraries for the new .NET language bindings for UNO.</Description>)
	$(file >>$@,<IncludeBuildOutput>false</IncludeBuildOutput>)
	$(file >>$@,</PropertyGroup>)
	$(file >>$@,<ItemGroup>)
	$(file >>$@,<Content Include="$(call gb_DotnetLibrary_get_target,net_basetypes)" Pack="true" PackagePath="lib/$$(TargetFramework)"/>)
	$(file >>$@,<Content Include="$(call gb_DotnetLibrary_get_target,net_uretypes)" Pack="true" PackagePath="lib/$$(TargetFramework)"/>)
	$(file >>$@,<Content Include="$(call gb_DotnetLibrary_get_target,net_oootypes)" Pack="true" PackagePath="lib/$$(TargetFramework)"/>)
	$(file >>$@,<Content Include="$(call gb_DotnetLibrary_get_target,net_bridge)" Pack="true" PackagePath="lib/$$(TargetFramework)"/>)
	$(file >>$@,</ItemGroup>)
	$(file >>$@,</Project>)

$(net_nuget_pkg_FILE) : $(net_nuget_pkg_PROJECT)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),NET,4)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),NET)

	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Helper_print_on_error,\
			"$(DOTNET)" pack $< -o $(dir $@),\
			$(net_nuget_pkg_DIR)/log))

	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),NET)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
