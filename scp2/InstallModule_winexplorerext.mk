# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InstallModule_InstallModule,scp2/winexplorerext))

$(eval $(call gb_InstallModule_define_if_set,scp2/winexplorerext,\
	BUILD_X64 \
))

$(eval $(call gb_InstallModule_add_scpfiles,scp2/winexplorerext,\
    scp2/source/winexplorerext/file_winexplorerext \
    scp2/source/winexplorerext/registryitem_winexplorerext \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/winexplorerext,\
    scp2/source/winexplorerext/module_winexplorerext \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
