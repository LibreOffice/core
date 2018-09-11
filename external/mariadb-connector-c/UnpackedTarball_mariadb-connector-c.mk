# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,mariadb-connector-c))

$(eval $(call gb_UnpackedTarball_set_tarball,mariadb-connector-c,$(MARIADB_CONNECTOR_C_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_file,mariadb-connector-c,include/mysql_version.h,external/mariadb-connector-c/configs/mysql_version.h))

ifneq ($(OS),WNT)
ifeq ($(OS),MACOSX)
$(eval $(call gb_UnpackedTarball_add_file,mariadb-connector-c,include/my_config.h,external/mariadb-connector-c/configs/mac_my_config.h))
else
$(eval $(call gb_UnpackedTarball_add_file,mariadb-connector-c,include/my_config.h,external/mariadb-connector-c/configs/linux_my_config.h))
endif
endif # $(OS),WNT

$(eval $(call gb_UnpackedTarball_set_patchlevel,mariadb-connector-c,1))

# mariadb-connector-c/mariadb-static-inline.patch upstream at
# <https://mariadb.atlassian.net/browse/CONC-18> "no external definition of
# non-static inline local_thr_alarm in libmariadb/net.c":
$(eval $(call gb_UnpackedTarball_add_patches,mariadb-connector-c,\
    external/mariadb-connector-c/mariadb-swap.patch \
    $(if $(filter MSC,$(COM)),external/mariadb-connector-c/mariadb-msvc.patch.1) \
    external/mariadb-connector-c/mariadb-inline.patch.1 \
    external/mariadb-connector-c/mariadb-CONC-104.patch.1 \
    external/mariadb-connector-c/clang-cl.patch.0 \
))

# vim: set noet sw=4 ts=4:
