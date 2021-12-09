# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,hsqldb))

$(eval $(call gb_Library_set_include,hsqldb,\
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_externals,hsqldb,\
	boost_headers \
	hsqldb \
))

$(eval $(call gb_Library_use_sdk_api,hsqldb))

$(eval $(call gb_Library_use_libraries,hsqldb,\
	comphelper \
	cppu \
	cppuhelper \
	dbtools \
	jvmfwk \
	sal \
	salhelper \
	tl \
	utl \
	i18nlangtag \
))

$(eval $(call gb_Library_set_componentfile,hsqldb,connectivity/source/drivers/hsqldb/hsqldb,services))

$(eval $(call gb_Library_add_exception_objects,hsqldb,\
	connectivity/source/drivers/hsqldb/HCatalog \
	connectivity/source/drivers/hsqldb/HColumns \
	connectivity/source/drivers/hsqldb/HConnection \
	connectivity/source/drivers/hsqldb/HDriver \
	connectivity/source/drivers/hsqldb/HStorageAccess \
	connectivity/source/drivers/hsqldb/HStorageMap \
	connectivity/source/drivers/hsqldb/HTable \
	connectivity/source/drivers/hsqldb/HTables \
	connectivity/source/drivers/hsqldb/HTerminateListener \
	connectivity/source/drivers/hsqldb/HTools \
	connectivity/source/drivers/hsqldb/HUser \
	connectivity/source/drivers/hsqldb/HUsers \
	connectivity/source/drivers/hsqldb/HView \
	connectivity/source/drivers/hsqldb/HViews \
	connectivity/source/drivers/hsqldb/StorageFileAccess \
	connectivity/source/drivers/hsqldb/StorageNativeInputStream \
	connectivity/source/drivers/hsqldb/StorageNativeOutputStream \
	connectivity/source/drivers/hsqldb/accesslog \
))

# vim: set noet sw=4 ts=4:
