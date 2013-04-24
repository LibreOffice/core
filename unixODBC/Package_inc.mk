# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,odbc_inc,$(SRCDIR)/unixODBC/inc))

$(eval $(call gb_Package_add_file,odbc_inc,inc/external/odbc/sql.h,sql.h))
$(eval $(call gb_Package_add_file,odbc_inc,inc/external/odbc/sqlext.h,sqlext.h))
$(eval $(call gb_Package_add_file,odbc_inc,inc/external/odbc/sqltypes.h,sqltypes.h))
$(eval $(call gb_Package_add_file,odbc_inc,inc/external/odbc/sqlucode.h,sqlucode.h))

# vim: set noet sw=4 ts=4:
