# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,hsqldb))

include $(SRCDIR)/hsqldb/version.mk

$(eval $(call gb_UnpackedTarball_set_tarball,hsqldb,17410483b5b5f267aa18b7e00b65e6e0-hsqldb_$(HSQLDB_VERSION).zip))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,hsqldb,\
	build/build.xml \
	src/org/hsqldb/Library.java \
	src/org/hsqldb/persist/HsqlDatabaseProperties.java \
))

$(eval $(call gb_UnpackedTarball_add_patches,hsqldb,\
	hsqldb/patches/i96823.patch \
	hsqldb/patches/i97032.patch \
	hsqldb/patches/i103528.patch \
	hsqldb/patches/i104901.patch \
	hsqldb/patches/fdo36824.patch \
	$(if $(filter-out 1.5,$(JAVA_SOURCE_VER)),\
		hsqldb/patches/jdbc-4.1.patch \
	) \
))

# vim: set noet sw=4 ts=4:
