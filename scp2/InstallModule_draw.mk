# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InstallModule_InstallModule,scp2/draw))

$(eval $(call gb_InstallModule_define_if_set,scp2/draw,\
	ENABLE_MACOSX_MACLIKE_APP_STRUCTURE \
	ENABLE_MACOSX_SANDBOX \
))

$(eval $(call gb_InstallModule_add_templates,scp2/draw,\
    scp2/source/templates/module_langpack_draw \
))

$(eval $(call gb_InstallModule_add_scpfiles,scp2/draw,\
    scp2/source/draw/file_draw \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/draw,\
    scp2/source/draw/module_draw \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
