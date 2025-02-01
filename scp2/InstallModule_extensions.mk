# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InstallModule_InstallModule,scp2/extensions))

$(eval $(call gb_InstallModule_use_auto_install_libs,scp2/extensions,extensions_bsh))
$(eval $(call gb_InstallModule_use_auto_install_libs,scp2/extensions,extensions_rhino))

$(eval $(call gb_InstallModule_add_scpfiles,scp2/extensions,\
    scp2/source/extensions/file_extensions \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/extensions,\
    scp2/source/extensions/module_extensions \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
