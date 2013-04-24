# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,odbcbase))

$(eval $(call gb_Library_set_warnings_not_errors,odbcbase))

$(eval $(call gb_Library_use_packages,odbcbase,\
	connectivity_generated \
))

$(eval $(call gb_Library_use_sdk_api,odbcbase))

$(eval $(call gb_Library_set_include,odbcbase,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_add_defs,odbcbase,\
	-DOOO_DLLIMPLEMENTATION_ODBCBASE \
))

$(eval $(call gb_Library_use_externals,odbcbase,\
	boost_headers \
	odbc_headers \
))

$(eval $(call gb_Library_use_libraries,odbcbase,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	dbtools \
	comphelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,odbcbase,\
	connectivity/source/drivers/odbcbase/OPreparedStatement \
	connectivity/source/drivers/odbcbase/OStatement \
	connectivity/source/drivers/odbcbase/OResultSetMetaData \
	connectivity/source/drivers/odbcbase/OResultSet \
	connectivity/source/drivers/odbcbase/OTools \
	connectivity/source/drivers/odbcbase/ODatabaseMetaDataResultSet \
	connectivity/source/drivers/odbcbase/ODatabaseMetaData \
	connectivity/source/drivers/odbcbase/ODriver \
	connectivity/source/drivers/odbcbase/OConnection \
))

# vim: set noet sw=4 ts=4:
