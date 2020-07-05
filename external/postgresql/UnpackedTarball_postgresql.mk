# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,postgresql))

$(eval $(call gb_UnpackedTarball_set_tarball,postgresql,$(POSTGRESQL_TARBALL),,postgresql))

$(eval $(call gb_UnpackedTarball_update_autoconf_configs,postgresql,config))

$(eval $(call gb_UnpackedTarball_set_patchlevel,postgresql,3))

$(eval $(call gb_UnpackedTarball_add_patches,postgresql, \
	external/postgresql/postgresql-libs-leak.patch \
	external/postgresql/postgresql-9.2.1-libreoffice.patch \
	external/postgresql/windows.patch.0 \
	external/postgresql/postgresql.exit.patch.0 \
))

ifeq ($(SYSTEM_ZLIB),)
$(eval $(call gb_UnpackedTarball_add_patches,postgresql, \
	external/postgresql/internal-zlib.patch.1 \
))
endif

# vim: set noet sw=4 ts=4:
