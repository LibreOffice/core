# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,hsqldb))

$(eval $(call gb_UnpackedTarball_set_tarball,hsqldb,$(HSQLDB_TARBALL),,hsqldb))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,hsqldb,\
	build/build.xml \
	src/org/hsqldb/Library.java \
	src/org/hsqldb/persist/HsqlDatabaseProperties.java \
))

$(eval $(call gb_UnpackedTarball_set_patchlevel,hsqldb,2))

$(eval $(call gb_UnpackedTarball_add_patches,hsqldb,\
	external/hsqldb/patches/i96823.patch \
	external/hsqldb/patches/i97032.patch \
	external/hsqldb/patches/i103528.patch \
	external/hsqldb/patches/i104901.patch \
	external/hsqldb/patches/fdo36824.patch \
	external/hsqldb/patches/limit_as_table_alias.patch \
	external/hsqldb/patches/hsqldb-runFinalizersOnExit.patch \
	external/hsqldb/patches/jdbc-4.1.patch \
	external/hsqldb/patches/multipleResultSets.patch \
	external/hsqldb/patches/disable-dump-script.patch \
))

# vim: set noet sw=4 ts=4:
