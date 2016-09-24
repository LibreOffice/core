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
	mysqlcppconn \
))

ifeq ($(SYSTEM_MYSQL_CPPCONN),)
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

$(eval $(call gb_Library_add_defs,mysqlc,\
	-DCPPDBC_EXPORTS \
	-DCPPCONN_LIB_BUILD \
	-DMARIADBC_VERSION_MAJOR=$(MARIADBC_MAJOR) \
	-DMARIADBC_VERSION_MINOR=$(MARIADBC_MINOR) \
	-DMARIADBC_VERSION_MICRO=$(MARIADBC_MICRO) \
	$(if $(SYSTEM_MYSQL_CPPCONN),,\
	-DCPPCONN_LIB=\"$(call gb_Library_get_runtime_filename,mysqlcppconn)\") \
	$(if $(BUNDLE_MARIADB),-DBUNDLE_MARIADB=\"$(LIBMARIADB)\") \
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
))

$(eval $(call gb_Library_set_componentfile,mysqlc,mysqlc/source/mysqlc))

$(eval $(call gb_Library_set_external_code,mysqlc))

# vim: set noet sw=4 ts=4:
