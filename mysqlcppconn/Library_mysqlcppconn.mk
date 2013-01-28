# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mysqlcppconn))

$(eval $(call gb_Library_use_unpacked,mysqlcppconn,mysqlcppconn))

$(eval $(call gb_Library_use_externals,mysqlcppconn, \
	mysql \
	boost_headers \
))

$(eval $(call gb_Library_set_warnings_not_errors,mysqlcppconn))

$(eval $(call gb_Library_set_generated_cxx_suffix,mysqlcppconn,cpp))

$(eval $(call gb_Library_set_include,mysqlcppconn,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,mysqlcppconn) \
	-I$(call gb_UnpackedTarball_get_dir,mysqlcppconn)/cppconn \
))

# HACK: generate import lib for msvc
ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_ldflags,mysqlcppconn,\
	/EXPORT:sql_mysql_get_driver_instance \
))
endif

$(eval $(call gb_Library_add_defs,mysqlcppconn,\
	-DCPPCONN_LIB_BUILD \
	$(if $(filter WNT,$(OS)),\
	-DCPPDBC_WIN32 \
	-D_CRT_SECURE_NO_WARNINGS \
	-D_SCL_SECURE_NO_WARNINGS ) \
))

$(eval $(call gb_Library_add_generated_exception_objects,mysqlcppconn,\
	UnpackedTarball/mysqlcppconn/driver/mysql_art_resultset \
	UnpackedTarball/mysqlcppconn/driver/mysql_art_rset_metadata \
	UnpackedTarball/mysqlcppconn/driver/mysql_connection \
	UnpackedTarball/mysqlcppconn/driver/mysql_debug \
	UnpackedTarball/mysqlcppconn/driver/mysql_driver \
	UnpackedTarball/mysqlcppconn/driver/mysql_metadata \
	UnpackedTarball/mysqlcppconn/driver/mysql_parameter_metadata \
	UnpackedTarball/mysqlcppconn/driver/mysql_prepared_statement \
	UnpackedTarball/mysqlcppconn/driver/mysql_ps_resultset \
	UnpackedTarball/mysqlcppconn/driver/mysql_ps_resultset_metadata \
	UnpackedTarball/mysqlcppconn/driver/mysql_resultbind \
	UnpackedTarball/mysqlcppconn/driver/mysql_resultset \
	UnpackedTarball/mysqlcppconn/driver/mysql_resultset_metadata \
	UnpackedTarball/mysqlcppconn/driver/mysql_statement \
	UnpackedTarball/mysqlcppconn/driver/mysql_util \
	UnpackedTarball/mysqlcppconn/driver/mysql_warning \
	UnpackedTarball/mysqlcppconn/driver/nativeapi/mysql_client_api \
	UnpackedTarball/mysqlcppconn/driver/nativeapi/library_loader \
	UnpackedTarball/mysqlcppconn/driver/nativeapi/mysql_native_driver_wrapper \
	UnpackedTarball/mysqlcppconn/driver/nativeapi/mysql_native_connection_wrapper \
	UnpackedTarball/mysqlcppconn/driver/nativeapi/mysql_native_resultset_wrapper \
	UnpackedTarball/mysqlcppconn/driver/nativeapi/mysql_native_statement_wrapper \
))

# vim: set noet sw=4 ts=4:
