# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Configuration_Configuration,driver_mysql))

$(eval $(call gb_Configuration_add_spool_modules,driver_mysql,connectivity/registry/mysql,\
	org/openoffice/Office/DataAccess/Drivers-mysql.xcu \
))

$(eval $(call gb_Configuration_add_localized_datas,driver_mysql,connectivity/registry/mysql,\
	org/openoffice/Office/DataAccess/Drivers.xcu \
))

# vim: set noet sw=4 ts=4:
