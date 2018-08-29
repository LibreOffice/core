# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, you can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mysql))

$(eval $(call gb_Library_use_libraries,mysql,\
	sal \
))

$(eval $(call gb_Library_add_exception_objects,mysql,\
	connectivity/source/drivers/mysqlc/mysql_dummy \
))

# vim: set noet sw=4 ts=4:
