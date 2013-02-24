# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mysqlc))

$(eval $(call gb_Library_set_warnings_not_errors,mysqlc))

$(eval $(call gb_Library_use_externals,mysqlc,\
	boost_headers \
	mysql \
	mysqlcppconn \
))

ifeq ($(SYSTEM_MYSQL_CPPCONN),NO)
$(eval $(call gb_Library_add_libs,mysqlc,\
	$(if $(filter-out WNT,$(OS)),$(if $(filter MACOSX SOLARIS,$(OS)),-lz -lm,\
	-rdynamic -lz -lcrypt -lnsl -lm)) \
))
endif

$(eval $(call gb_Library_use_sdk_api,mysqlc))

$(eval $(call gb_Library_use_libraries,mysqlc,\
	cppu \
	sal \
	salhelper \
	cppuhelper \
))

MYSQL_LIBFILE :=$(if $(filter WNT,$(OS)),libmysql.dll,$(if $(filter MACOSX,$(OS)),libmysql.16.dylib,libmysql.so.16))

$(eval $(call gb_Library_add_defs,mysqlc,\
	-DCPPDBC_EXPORTS \
	-DCPPCON_LIB_BUILD \
	-DMYSQLC_VERSION_MAJOR=$(MYSQLC_MAJOR) \
	-DMYSQLC_VERSION_MINOR=$(MYSQLC_MINOR) \
	-DMYSQLC_VERSION_MICRO=$(MYSQLC_MICRO) \
	$(if $(filter NO,$(SYSTEM_MYSQL)),-DMYSQL_LIB=\"$(MYSQL_LIBFILE)\") \
	$(if $(filter NO,$(SYSTEM_MYSQL_CPPCONN)),\
	-DCPPCONN_LIB=\"$(call gb_Library_get_runtime_filename,mysqlcppconn)\") \
))

$(eval $(call gb_Library_add_exception_objects,mysqlc,\
	mysqlc/source/mysqlc_driver \
	mysqlc/source/mysqlc_services \
	mysqlc/source/mysqlc_connection \
	mysqlc/source/mysqlc_resultset \
	mysqlc/source/mysqlc_resultsetmetadata \
	mysqlc/source/mysqlc_statement \
	mysqlc/source/mysqlc_preparedstatement \
	mysqlc/source/mysqlc_databasemetadata \
	mysqlc/source/mysqlc_types \
	mysqlc/source/mysqlc_general \
	mysqlc/source/mysqlc_propertyids \
))

$(eval $(call gb_Library_set_componentfile,mysqlc,mysqlc/source/mysqlc))

# vim: set noet sw=4 ts=4:
