# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Extension_Extension,mysql-connector-ooo,mysqlc/source))

$(eval $(call gb_Extension_use_default_description,mysql-connector-ooo))
$(eval $(call gb_Extension_use_default_license,mysql-connector-ooo))

$(eval $(call gb_Extension_add_library,mysql-connector-ooo,mysqlc))

$(eval $(call gb_Extension_add_file,mysql-connector-ooo,components.rdb,$(call gb_Rdb_get_target,mysqlc)))

$(eval $(call gb_Extension_add_file,mysql-connector-ooo,registry/data/org/openoffice/Office/DataAccess/Drivers.xcu,$(call gb_XcuFile_for_extension,mysqlc/source/registry/data/org/openoffice/Office/DataAccess/Drivers.xcu)))

$(eval $(call gb_Extension_add_files,mysql-connector-ooo,images,\
    $(SRCDIR)/icon-themes/galaxy/desktop/res/extension_32.png \
))

ifeq ($(SYSTEM_MYSQL_CPPCONN),NO)
$(eval $(call gb_Extension_add_library,mysql-connector-ooo,mysqlcppconn))
endif

# vim: set noet sw=4 ts=4:
