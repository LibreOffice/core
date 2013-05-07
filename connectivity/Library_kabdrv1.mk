# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,kabdrv1))

$(eval $(call gb_Library_use_sdk_api,kabdrv1))

$(eval $(call gb_Library_use_externals,kabdrv1,\
	boost_headers \
	kde \
))

$(eval $(call gb_Library_set_include,kabdrv1,\
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_add_libs,kabdrv1,\
	-lkabc \
))

$(eval $(call gb_Library_use_libraries,kabdrv1,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	dbtools \
	comphelper \
))

$(eval $(call gb_Library_add_exception_objects,kabdrv1,\
	connectivity/source/drivers/kab/KColumns \
	connectivity/source/drivers/kab/KTable \
	connectivity/source/drivers/kab/KTables \
	connectivity/source/drivers/kab/KCatalog \
	connectivity/source/drivers/kab/KResultSet \
	connectivity/source/drivers/kab/KStatement \
	connectivity/source/drivers/kab/KPreparedStatement \
	connectivity/source/drivers/kab/KDatabaseMetaData \
	connectivity/source/drivers/kab/KConnection \
	connectivity/source/drivers/kab/KResultSetMetaData \
	connectivity/source/drivers/kab/kcondition \
	connectivity/source/drivers/kab/korder \
	connectivity/source/drivers/kab/kfields \
	connectivity/source/drivers/kab/KDEInit \
))

# vim: set noet sw=4 ts=4:
