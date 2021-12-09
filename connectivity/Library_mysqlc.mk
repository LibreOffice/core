# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mysqlc))

$(eval $(call gb_Library_use_externals,mysqlc,\
	boost_headers \
	mariadb-connector-c \
	openssl \
))

$(eval $(call gb_Library_set_include,mysqlc,\
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_add_libs,mysqlc,\
	$(if $(filter-out WNT,$(OS)),$(if $(filter HAIKU MACOSX SOLARIS,$(OS)),\
	-lz -lm,-rdynamic -lz -lcrypt -lm)) \
	$(if $(filter LINUX,$(OS)),-ldl,) \
))

$(eval $(call gb_Library_use_sdk_api,mysqlc))

$(eval $(call gb_Library_use_libraries,mysqlc,\
	cppu \
    dbtools \
	sal \
	salhelper \
	comphelper \
	cppuhelper \
))

$(eval $(call gb_Library_add_defs,mysqlc,\
	-DCPPDBC_EXPORTS \
	-DCPPCONN_LIB_BUILD \
	-DMARIADBC_VERSION_MAJOR=$(MARIADBC_MAJOR) \
	-DMARIADBC_VERSION_MINOR=$(MARIADBC_MINOR) \
	-DMARIADBC_VERSION_MICRO=$(MARIADBC_MICRO) \
	$(if $(BUNDLE_MARIADB_CONNECTOR_C),-DBUNDLE_MARIADB=\"$(LIBMARIADB)\") \
))

$(eval $(call gb_Library_add_exception_objects,mysqlc,\
	connectivity/source/drivers/mysqlc/mysqlc_driver \
	connectivity/source/drivers/mysqlc/mysqlc_services \
	connectivity/source/drivers/mysqlc/mysqlc_connection \
	connectivity/source/drivers/mysqlc/mysqlc_resultset \
	connectivity/source/drivers/mysqlc/mysqlc_prepared_resultset \
	connectivity/source/drivers/mysqlc/mysqlc_resultsetmetadata \
	connectivity/source/drivers/mysqlc/mysqlc_statement \
	connectivity/source/drivers/mysqlc/mysqlc_preparedstatement \
	connectivity/source/drivers/mysqlc/mysqlc_databasemetadata \
	connectivity/source/drivers/mysqlc/mysqlc_types \
	connectivity/source/drivers/mysqlc/mysqlc_general \
))

$(eval $(call gb_Library_set_componentfile,mysqlc,connectivity/source/drivers/mysqlc/mysqlc,services))

# vim: set noet sw=4 ts=4:
