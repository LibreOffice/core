# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,writer))

$(eval $(call gb_Library_set_componentfile,writer,connectivity/source/drivers/writer/writer,services))

$(eval $(call gb_Library_use_external,writer,boost_headers))

$(eval $(call gb_Library_use_sdk_api,writer))

$(eval $(call gb_Library_set_include,writer,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_use_libraries,writer,\
	cppu \
	cppuhelper \
	svl \
	tl \
	utl \
	sal \
	salhelper \
	dbtools \
	file \
	comphelper \
))

$(eval $(call gb_Library_add_exception_objects,writer,\
	connectivity/source/drivers/writer/WCatalog \
	connectivity/source/drivers/writer/WConnection \
	connectivity/source/drivers/writer/WDatabaseMetaData \
	connectivity/source/drivers/writer/WDriver \
	connectivity/source/drivers/writer/WTable \
	connectivity/source/drivers/writer/WTables \
))

# vim: set noet sw=4 ts=4:
