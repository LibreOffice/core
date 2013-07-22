# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,firebird_sdbc))

$(eval $(call gb_Library_use_sdk_api,firebird_sdbc))

$(eval $(call gb_Library_use_externals,firebird_sdbc,\
	boost_headers \
	libfbembed \
))

$(eval $(call gb_Library_set_include,firebird_sdbc,\
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_use_libraries,firebird_sdbc, \
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    sal \
    salhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,firebird_sdbc,connectivity/source/drivers/firebird/firebird_sdbc))

$(eval $(call gb_Library_add_exception_objects,firebird_sdbc,\
    connectivity/source/drivers/firebird/Blob \
    connectivity/source/drivers/firebird/FConnection \
    connectivity/source/drivers/firebird/FDatabaseMetaData \
    connectivity/source/drivers/firebird/FDriver \
    connectivity/source/drivers/firebird/FPreparedStatement \
    connectivity/source/drivers/firebird/FResultSet \
    connectivity/source/drivers/firebird/FResultSetMetaData \
    connectivity/source/drivers/firebird/FServices \
    connectivity/source/drivers/firebird/FStatement \
    connectivity/source/drivers/firebird/Util \
))

# vim: set noet sw=4 ts=4:
