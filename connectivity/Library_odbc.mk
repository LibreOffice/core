# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,odbc))

$(eval $(call gb_Library_set_warnings_not_errors,odbc))

$(eval $(call gb_Library_set_componentfile,odbc,connectivity/source/drivers/odbc/odbc))

$(eval $(call gb_Library_use_sdk_api,odbc))

$(eval $(call gb_Library_set_include,odbc,\
	$$(INCLUDE) \
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
))

$(eval $(call gb_Library_use_external,odbc,odbc_headers))

$(eval $(call gb_Library_use_libraries,odbc,\
	odbcbase \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,odbc,\
	connectivity/source/drivers/odbc/oservices \
	connectivity/source/drivers/odbc/ORealDriver \
	connectivity/source/drivers/odbc/OFunctions \
))

# vim: set noet sw=4 ts=4:
