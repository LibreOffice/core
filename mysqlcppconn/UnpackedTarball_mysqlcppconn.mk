# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,mysqlcppconn))

$(eval $(call gb_UnpackedTarball_set_tarball,mysqlcppconn,$(MYSQLCPPCONN_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_file,mysqlcppconn,cppconn/config.h,mysqlcppconn/config.h))

ifeq ($(SYSTEM_MARIADB),YES)
$(eval $(call gb_UnpackedTarball_add_file,mysqlcppconn,driver/nativeapi/binding_config.h,mysqlcppconn/binding_static.h))
else
$(eval $(call gb_UnpackedTarball_add_file,mysqlcppconn,driver/nativeapi/binding_config.h,mysqlcppconn/binding_dynamic.h))
endif
$(eval $(call gb_UnpackedTarball_add_patches,mysqlcppconn,\
	mysqlcppconn/patches/mysql-connector-c++-1.1.0.patch \
	$(if $(filter NO,$(SYSTEM_MARIADB)), \
	mysqlcppconn/patches/mysql-connector-c++-1.1.0-mysql-binding-dynamic.patch) \
	mysqlcppconn/patches/default_to_protocol_tcp.patch \
))

# vim: set noet sw=4 ts=4:
