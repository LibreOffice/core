# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sdbt))

$(eval $(call gb_Library_set_include,sdbt,\
    $$(INCLUDE) \
	-I$(SRCDIR)/dbaccess/source/inc/ \
	-I$(SRCDIR)/dbaccess/source/sdbtools/inc \
))

$(eval $(call gb_Library_use_external,sdbt,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,sdbt,$(SRCDIR)/dbaccess/inc/pch/precompiled_sdbt))

$(eval $(call gb_Library_use_sdk_api,sdbt))

$(eval $(call gb_Library_use_libraries,sdbt,\
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    sal \
    salhelper \
    tl \
    utl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_set_componentfile,sdbt,dbaccess/util/sdbt))

$(eval $(call gb_Library_add_exception_objects,sdbt,\
    dbaccess/source/sdbtools/connection/connectiontools \
    dbaccess/source/sdbtools/connection/datasourcemetadata \
    dbaccess/source/sdbtools/connection/objectnames \
    dbaccess/source/sdbtools/connection/tablename \
    dbaccess/source/sdbtools/misc/module_sdbt \
    dbaccess/source/sdbtools/misc/sdbt_services \
    dbaccess/source/shared/sdbtstrings \
))

# vim: set noet sw=4 ts=4:
