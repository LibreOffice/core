# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,firebird-sdbc))

$(eval $(call gb_Library_use_sdk_api,firebird-sdbc))

$(eval $(call gb_Library_use_externals,firebird-sdbc,\
	firebird \
))

$(eval $(call gb_Library_use_libraries,firebird-sdbc, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,firebird-sdbc,connectivity/source/drivers/firebird/firebird-sdbc))

$(eval $(call gb_Library_add_exception_objects,firebird-sdbc,\
    connectivity/source/drivers/firebird/propertyids \
    connectivity/source/drivers/firebird/FConnection \
    connectivity/source/drivers/firebird/FDatabaseMetaData \
    connectivity/source/drivers/firebird/FDriver \
    connectivity/source/drivers/firebird/FPreparedStatement \
    connectivity/source/drivers/firebird/FResultSet \
    connectivity/source/drivers/firebird/FResultSetMetaData \
    connectivity/source/drivers/firebird/FServices \
    connectivity/source/drivers/firebird/FStatement \
))

# vim: set noet sw=4 ts=4:
