# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,postgresql))

$(eval $(call gb_UnpackedTarball_set_tarball,postgresql,$(POSTGRESQL_TARBALL)))

$(eval $(call gb_UnpackedTarball_set_patchlevel,postgresql,3))

$(eval $(call gb_UnpackedTarball_add_patches,postgresql, \
	postgresql/postgresql-moz-ldap.patch \
	postgresql/postgresql-libs-leak.patch \
	postgresql/libpq-parallel-build.patch \
	postgresql/postgresql-9.1.1-autoreconf.patch \
	postgresql/postgresql-9.1.1-libreoffice.patch \
))

# vim: set noet sw=4 ts=4:
