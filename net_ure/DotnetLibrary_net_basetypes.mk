# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_DotnetLibrary_CsLibrary,net_basetypes))

$(eval $(call gb_DotnetLibrary_add_sources,net_basetypes,\
    net_ure/source/basetypes/Any.cs \
    net_ure/source/basetypes/BoundAttribute.cs \
    net_ure/source/basetypes/Exception.cs \
    net_ure/source/basetypes/IQueryInterface.cs \
    net_ure/source/basetypes/RaisesAttribute.cs \
    net_ure/source/basetypes/UnoGeneratedAttribute.cs \
))

$(eval $(call gb_DotnetLibrary_add_properties,net_basetypes,\
    <AssemblyName>net_basetypes</AssemblyName> \
    <Version>0.1.0</Version> \
    <Company>LibreOffice</Company> \
    <Description>Base datatypes for the .NET language UNO binding.</Description> \
))

# vim: set noet sw=4 ts=4:
