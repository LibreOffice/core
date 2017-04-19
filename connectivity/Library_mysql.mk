# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mysql))

$(eval $(call gb_Library_set_componentfile,mysql,connectivity/source/drivers/mysql/mysql))

$(eval $(call gb_Library_use_external,mysql,boost_headers))

$(eval $(call gb_Library_use_sdk_api,mysql))

$(eval $(call gb_Library_set_include,mysql,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_set_precompiled_header,mysql,$(SRCDIR)/connectivity/inc/pch/precompiled_mysql))

$(eval $(call gb_Library_use_libraries,mysql,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	dbtools \
	comphelper \
))

$(eval $(call gb_Library_add_exception_objects,mysql,\
	connectivity/source/drivers/mysql/YDriver \
	connectivity/source/drivers/mysql/YTables \
	connectivity/source/drivers/mysql/YTable \
	connectivity/source/drivers/mysql/YViews \
	connectivity/source/drivers/mysql/YCatalog \
	connectivity/source/drivers/mysql/YColumns \
	connectivity/source/drivers/mysql/YUser \
	connectivity/source/drivers/mysql/YUsers \
	connectivity/source/drivers/mysql/Yservices \
))

# vim: set noet sw=4 ts=4:
