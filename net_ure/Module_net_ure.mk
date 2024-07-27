# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_Module_Module,net_ure))

ifeq ($(ENABLE_DOTNET)_$(filter ODK,$(BUILD_TYPE)),TRUE_ODK)
$(eval $(call gb_Module_add_targets,net_ure,\
	CustomTarget_net_uretypes \
	CustomTarget_net_oootypes \
	DotnetLibrary_net_basetypes \
	DotnetLibrary_net_uretypes \
	DotnetLibrary_net_oootypes \
	Package_net_basetypes \
	Package_net_uretypes \
	Package_net_oootypes \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,net_ure,\
	DotnetTest_net_basetypes_tests \
))
endif

# vim: set noet sw=4 ts=4:
