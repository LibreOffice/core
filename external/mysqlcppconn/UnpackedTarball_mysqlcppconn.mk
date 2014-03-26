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

$(eval $(call gb_UnpackedTarball_add_file,mysqlcppconn,cppconn/config.h,external/mysqlcppconn/config.h))

$(eval $(call gb_UnpackedTarball_add_file,mysqlcppconn,driver/nativeapi/binding_config.h,external/mysqlcppconn/binding_config.h))

$(eval $(call gb_UnpackedTarball_add_patches,mysqlcppconn,\
	external/mysqlcppconn/patches/mysql-connector-c++-1.1.0.patch \
	external/mysqlcppconn/patches/default_to_protocol_tcp.patch \
	external/mysqlcppconn/patches/warnings.patch.0 \
))

# vim: set noet sw=4 ts=4:
