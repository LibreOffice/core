# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_DotnetLibrary_CsLibrary,net_oootypes))

$(call gb_DotnetLibrary_get_target,net_oootypes) : \
    $(call gb_CustomTarget_get_target,net_oootypes)

$(eval $(call gb_DotnetLibrary_add_generated_sources,net_oootypes,\
    $(gb_CustomTarget_workdir)/net_ure/net_oootypes, \
        **/*.cs \
))

$(eval $(call gb_DotnetLibrary_link_cs_library,net_oootypes,net_uretypes))

$(eval $(call gb_DotnetLibrary_add_properties,net_oootypes,\
    <AssemblyName>net_oootypes</AssemblyName> \
    <Version>0.1.0</Version> \
    <Company>LibreOffice</Company> \
    <Description>LibreOffice datatypes for the .NET language UNO binding.</Description> \
))

# vim: set noet sw=4 ts=4:
