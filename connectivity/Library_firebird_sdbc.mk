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
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
	-I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_set_precompiled_header,firebird_sdbc,$(SRCDIR)/connectivity/inc/pch/precompiled_firebird_sdbc))

$(eval $(call gb_Library_use_libraries,firebird_sdbc, \
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    sal \
    salhelper \
    utl \
    svt \
    vcl \
))

$(eval $(call gb_Library_set_componentfile,firebird_sdbc,connectivity/source/drivers/firebird/firebird_sdbc))

$(eval $(call gb_Library_add_exception_objects,firebird_sdbc,\
    connectivity/source/drivers/firebird/Blob \
    connectivity/source/drivers/firebird/Clob \
    connectivity/source/drivers/firebird/Catalog \
    connectivity/source/drivers/firebird/Column \
    connectivity/source/drivers/firebird/Columns \
    connectivity/source/drivers/firebird/Connection \
    connectivity/source/drivers/firebird/DatabaseMetaData \
    connectivity/source/drivers/firebird/Driver \
    connectivity/source/drivers/firebird/Indexes \
    connectivity/source/drivers/firebird/Keys \
    connectivity/source/drivers/firebird/PreparedStatement \
    connectivity/source/drivers/firebird/ResultSet \
    connectivity/source/drivers/firebird/ResultSetMetaData \
    connectivity/source/drivers/firebird/Services \
    connectivity/source/drivers/firebird/Statement \
    connectivity/source/drivers/firebird/StatementCommonBase \
    connectivity/source/drivers/firebird/Table \
    connectivity/source/drivers/firebird/Tables \
    connectivity/source/drivers/firebird/User \
    connectivity/source/drivers/firebird/Users \
    connectivity/source/drivers/firebird/Util \
))

# vim: set noet sw=4 ts=4:
