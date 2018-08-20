# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mysqlcppconn))

$(eval $(call gb_Library_use_unpacked,mysqlcppconn,mysql-connector-cpp))

$(eval $(call gb_Library_use_externals,mysqlcppconn, \
	boost_headers \
	mariadb-connector-c \
))

ifneq ($(OS),WNT)

$(eval $(call gb_Library_add_libs,mysqlcppconn,\
	$(if $(filter-out MACOSX,$(OS)),-ldl) \
))

endif

ifeq ($(OS),MACOSX)
ifeq ($(SYSTEM_MARIADB_CONNECTOR_C),)
$(eval $(call gb_Library_use_externals,mysqlcppconn,\
	    iconv \
))
endif
endif

$(eval $(call gb_Library_add_cxxflags,mysqlcppconn,$(filter-out -Werror=%,$(gb_CXX03FLAGS))))

$(eval $(call gb_Library_set_external_code,mysqlcppconn))

$(eval $(call gb_Library_set_warnings_not_errors,mysqlcppconn))

$(eval $(call gb_Library_set_generated_cxx_suffix,mysqlcppconn,cpp))

$(eval $(call gb_Library_set_include,mysqlcppconn,\
	$$(INCLUDE) \
	-I$(call gb_UnpackedTarball_get_dir,mysql-connector-cpp) \
	-I$(call gb_UnpackedTarball_get_dir,mysql-connector-cpp)/cppconn \
))

$(eval $(call gb_Library_add_defs,mysqlcppconn,\
	-Dmysqlcppconn_EXPORTS \
	$(if $(filter WNT,$(OS)),\
	-DCPPDBC_WIN32 \
	-D_CRT_SECURE_NO_WARNINGS \
	-D_SCL_SECURE_NO_WARNINGS ) \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,mysqlcppconn,\
	-lpthread \
))
endif

$(eval $(call gb_Library_add_generated_exception_objects,mysqlcppconn,\
	UnpackedTarball/mysql-connector-cpp/driver/mysql_art_resultset \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_art_rset_metadata \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_connection \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_debug \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_driver \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_metadata \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_parameter_metadata \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_prepared_statement \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_ps_resultset \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_ps_resultset_metadata \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_resultbind \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_resultset \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_resultset_metadata \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_statement \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_uri \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_util \
	UnpackedTarball/mysql-connector-cpp/driver/mysql_warning \
	UnpackedTarball/mysql-connector-cpp/driver/nativeapi/mysql_client_api \
	UnpackedTarball/mysql-connector-cpp/driver/nativeapi/library_loader \
	UnpackedTarball/mysql-connector-cpp/driver/nativeapi/mysql_native_driver_wrapper \
	UnpackedTarball/mysql-connector-cpp/driver/nativeapi/mysql_native_connection_wrapper \
	UnpackedTarball/mysql-connector-cpp/driver/nativeapi/mysql_native_resultset_wrapper \
	UnpackedTarball/mysql-connector-cpp/driver/nativeapi/mysql_native_statement_wrapper \
))

# vim: set noet sw=4 ts=4:
