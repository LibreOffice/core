# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mariadb))

$(eval $(call gb_Library_use_sdk_api,mariadb))

$(eval $(call gb_Library_use_externals,mariadb,\
	mariadb \
))

$(eval $(call gb_Library_use_libraries,mariadb, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,mariadb,connectivity/source/drivers/mariadb/mariadb))

$(eval $(call gb_Library_add_exception_objects,mariadb,\
    connectivity/source/drivers/mariadb/propertyids \
    connectivity/source/drivers/mariadb/SConnection \
    connectivity/source/drivers/mariadb/SDatabaseMetaData \
    connectivity/source/drivers/mariadb/SDriver \
    connectivity/source/drivers/mariadb/SPreparedStatement \
    connectivity/source/drivers/mariadb/SResultSet \
    connectivity/source/drivers/mariadb/SResultSetMetaData \
    connectivity/source/drivers/mariadb/SServices \
    connectivity/source/drivers/mariadb/SStatement \
))

# vim: set noet sw=4 ts=4:
