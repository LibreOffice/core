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

$(eval $(call gb_UnpackedTarball_add_file,mariadb,include/mysql_version.h,libmariadb/configs/mysql_version.h))

ifneq ($(OS),WNT)
ifeq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_file,mariadb,include/my_config.h,libmariadb/configs/mac_my_config.h))
else
$(eval $(call gb_UnpackedTarball_add_file,mariadb,include/my_config.h,libmariadb/configs/linux_my_config.h))
endif
endif # $(OS),WNT

$(eval $(call gb_UnpackedTarball_set_patchlevel,mariadb,1))

# libmariadb/mariadb-static-inline.patch upstream at
# <https://mariadb.atlassian.net/browse/CONC-18> "no external definition of
# non-static inline local_thr_alarm in libmariadb/net.c":
$(eval $(call gb_UnpackedTarball_add_patches,mariadb,\
    libmariadb/mariadb-thread.patch \
    libmariadb/mariadb-swap.patch \
    libmariadb/mariadb-trunk-40.patch \
    libmariadb/mariadb-static-inline.patch \
))

$(eval $(call gb_UnpackedTarball_fix_end_of_line,mariadb,\
	libmysql/libmysql_exports.def \
))

# vim: set noet sw=4 ts=4:
