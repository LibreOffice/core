# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,dbahsql))

$(eval $(call gb_Library_set_include,dbahsql,\
    $$(INCLUDE) \
    -I$(WORKDIR)/YaccTarget/connectivity/source/parse \
))

$(eval $(call gb_Library_use_external,dbahsql,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,dbahsql,$(SRCDIR)/dbaccess/inc/pch/precompiled_dbahsql))

$(eval $(call gb_Library_use_sdk_api,dbahsql))

$(eval $(call gb_Library_use_libraries,dbahsql,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    dbtools \
))

$(eval $(call gb_Library_add_exception_objects,dbahsql,\
    dbaccess/source/filter/hsqldb/hsqlimport \
    dbaccess/source/filter/hsqldb/parseschema \
    dbaccess/source/filter/hsqldb/createparser \
    dbaccess/source/filter/hsqldb/columndef \
    dbaccess/source/filter/hsqldb/fbcreateparser \
))

# vim: set noet sw=4 ts=4:
