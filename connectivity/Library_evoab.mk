# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,evoab))

$(eval $(call gb_Library_set_include,evoab,\
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_use_externals,evoab,\
	boost_headers \
	gobject \
))

$(eval $(call gb_Library_set_componentfile,evoab,connectivity/source/drivers/evoab2/evoab,services))

$(eval $(call gb_Library_use_sdk_api,evoab))

$(eval $(call gb_Library_use_libraries,evoab,\
	comphelper \
	cppu \
	cppuhelper \
	svl \
	tl \
	utl \
	ucbhelper \
	sal \
	salhelper \
	dbtools \
	file \
))

$(eval $(call gb_Library_add_exception_objects,evoab,\
	connectivity/source/drivers/evoab2/NDriver \
	connectivity/source/drivers/evoab2/NTable \
	connectivity/source/drivers/evoab2/NColumns \
	connectivity/source/drivers/evoab2/NTables \
	connectivity/source/drivers/evoab2/NCatalog \
	connectivity/source/drivers/evoab2/NConnection \
	connectivity/source/drivers/evoab2/NDatabaseMetaData \
	connectivity/source/drivers/evoab2/NStatement \
	connectivity/source/drivers/evoab2/NPreparedStatement \
	connectivity/source/drivers/evoab2/NResultSet \
	connectivity/source/drivers/evoab2/NResultSetMetaData \
	connectivity/source/drivers/evoab2/EApi \
))

# vim: set noet sw=4 ts=4:
