# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,mork))

$(eval $(call gb_Library_set_componentfile,mork,connectivity/source/drivers/mork/mork))

$(eval $(call gb_Library_add_defs,mork,\
	-DLO_DLLIMPLEMENTATION_MORK \
))

$(eval $(call gb_Library_set_include,mork,\
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_libraries,mork, \
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    sal \
    salhelper \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_sdk_api,mork))

$(eval $(call gb_Library_add_exception_objects,mork, \
    connectivity/source/drivers/mork/MColumns \
    connectivity/source/drivers/mork/MColumnAlias \
    connectivity/source/drivers/mork/MNSFolders \
    connectivity/source/drivers/mork/MNSINIParser \
    connectivity/source/drivers/mork/MNSProfileDiscover \
    connectivity/source/drivers/mork/MorkParser \
    connectivity/source/drivers/mork/MCatalog \
    connectivity/source/drivers/mork/MConnection \
    connectivity/source/drivers/mork/MDatabaseMetaData \
    connectivity/source/drivers/mork/MDatabaseMetaDataHelper \
    connectivity/source/drivers/mork/MDriver \
    connectivity/source/drivers/mork/MStatement \
    connectivity/source/drivers/mork/MResultSet \
    connectivity/source/drivers/mork/MResultSetMetaData \
    connectivity/source/drivers/mork/MPreparedStatement \
    connectivity/source/drivers/mork/MQueryHelper \
    connectivity/source/drivers/mork/MServices \
    connectivity/source/drivers/mork/MTable \
    connectivity/source/drivers/mork/MTables \
))

# vim: set noet sw=4 ts=4:
