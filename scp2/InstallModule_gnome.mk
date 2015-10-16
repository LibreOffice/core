# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InstallModule_InstallModule,scp2/gnome))

$(eval $(call gb_InstallModule_use_auto_install_libs,scp2/gnome,gnome))

$(eval $(call gb_InstallModule_define_if_set,scp2/gnome,\
	ENABLE_GIO \
    ENABLE_GTK \
))

$(eval $(call gb_InstallModule_add_scpfiles,scp2/gnome,\
    scp2/source/gnome/file_gnome \
))

$(eval $(call gb_InstallModule_add_localized_scpfiles,scp2/gnome,\
    scp2/source/gnome/module_gnome \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
