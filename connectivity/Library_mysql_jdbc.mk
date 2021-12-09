# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mysql_jdbc))

$(eval $(call gb_Library_set_componentfile,mysql_jdbc,connectivity/source/drivers/mysql_jdbc/mysql_jdbc,services))

$(eval $(call gb_Library_use_external,mysql_jdbc,boost_headers))

$(eval $(call gb_Library_use_sdk_api,mysql_jdbc))

$(eval $(call gb_Library_set_include,mysql_jdbc,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_set_precompiled_header,mysql_jdbc,connectivity/inc/pch/precompiled_mysql_jdbc))

$(eval $(call gb_Library_use_libraries,mysql_jdbc,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	utl \
	dbtools \
	comphelper \
))

$(eval $(call gb_Library_add_exception_objects,mysql_jdbc,\
	connectivity/source/drivers/mysql_jdbc/YDriver \
	connectivity/source/drivers/mysql_jdbc/YTables \
	connectivity/source/drivers/mysql_jdbc/YTable \
	connectivity/source/drivers/mysql_jdbc/YViews \
	connectivity/source/drivers/mysql_jdbc/YCatalog \
	connectivity/source/drivers/mysql_jdbc/YColumns \
	connectivity/source/drivers/mysql_jdbc/YUser \
	connectivity/source/drivers/mysql_jdbc/YUsers \
))

# vim: set noet sw=4 ts=4:
