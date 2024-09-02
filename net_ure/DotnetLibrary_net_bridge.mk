# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_DotnetLibrary_DotnetLibrary,net_bridge,$(gb_DotnetLibrary_CS)))

$(eval $(call gb_DotnetLibrary_add_sources,net_bridge,\
	net_ure/source/bridge/helper/DisposeGuard \
	net_ure/source/bridge/helper/StructHelper \
	net_ure/source/bridge/helper/TypeHelper \
	net_ure/source/bridge/helper/WeakBase \
	net_ure/source/bridge/helper/WeakComponentBase \
))

$(eval $(call gb_DotnetLibrary_add_sources,net_bridge,\
	net_ure/source/bridge/native/InteropMethods \
	net_ure/source/bridge/native/InteropTypes \
	net_ure/source/bridge/native/Marshaller \
	net_ure/source/bridge/native/NativeBootstrap \
	net_ure/source/bridge/native/NativeUnoProxy \
	net_ure/source/bridge/native/NetEnvironment \
	net_ure/source/bridge/native/WeakIndexTable \
	net_ure/source/bridge/native/WeakOidTypeTable \
))

$(eval $(call gb_DotnetLibrary_add_properties,net_bridge,\
	<AllowUnsafeBlocks>true</AllowUnsafeBlocks> \
	<GenerateRuntimeConfigurationFiles>true</GenerateRuntimeConfigurationFiles> \
))

$(eval $(call gb_DotnetLibrary_add_items,net_bridge,\
	<PackageReference Include="System.Reflection.DispatchProxy" Version="4.7.1" /> \
))

$(eval $(call gb_DotnetLibrary_link_library,net_bridge,net_uretypes))

$(eval $(call gb_DotnetLibrary_add_properties,net_bridge,\
	<Version>0.1.0</Version> \
	<Company>LibreOffice</Company> \
	<Description>Interprocess bridge for the .NET language UNO binding.</Description> \
))

# vim: set noet sw=4 ts=4:
