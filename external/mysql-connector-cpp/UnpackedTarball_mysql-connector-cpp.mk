# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,mysql-connector-cpp))

$(eval $(call gb_UnpackedTarball_set_tarball,mysql-connector-cpp,$(MYSQL_CONNECTOR_CPP_TARBALL)))

$(eval $(call gb_UnpackedTarball_add_file,mysql-connector-cpp,cppconn/config.h,external/mysql-connector-cpp/config.h))
$(eval $(call gb_UnpackedTarball_add_file,mysql-connector-cpp,version_info.h,external/mysql-connector-cpp/version_info.h))

$(eval $(call gb_UnpackedTarball_add_file,mysql-connector-cpp,driver/nativeapi/binding_config.h,external/mysql-connector-cpp/binding_config.h))

$(eval $(call gb_UnpackedTarball_add_patches,mysql-connector-cpp,\
	external/mysql-connector-cpp/patches/mysql-connector-c++-1.1.0.patch \
	external/mysql-connector-cpp/patches/warnings.patch.0 \
	external/mysql-connector-cpp/patches/enable-libmysql-proxy.patch \
	external/mysql-connector-cpp/patches/dynexcspec.patch.0 \
))

# vim: set noet sw=4 ts=4:
