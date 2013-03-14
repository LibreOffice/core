# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,mariadb))

$(eval $(call gb_UnpackedTarball_set_tarball,mariadb,$(MARIADB_TARBALL)))

# This was generated on a 64-bit linux, will have to conditionalize it if it is broken
# for another configuration.
$(eval $(call gb_UnpackedTarball_add_file,mariadb,include/my_config.h,libmariadb/my_config.h))

$(eval $(call gb_UnpackedTarball_add_file,mariadb,include/mysql_version.h,libmariadb/mysql_version.h))

$(eval $(call gb_UnpackedTarball_set_patchlevel,mariadb,1))

$(eval $(call gb_UnpackedTarball_add_patches,mariadb,\
    libmariadb/mariadb-thread.patch \
    libmariadb/mariadb-swap.patch \
    libmariadb/mariadb-trunk-40.patch \
))

# vim: set noet sw=4 ts=4:
