# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,dbaccess_embeddeddb_performancetest))

$(eval $(call gb_CppunitTest_use_external,dbaccess_embeddeddb_performancetest,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,dbaccess_embeddeddb_performancetest, \
    dbaccess/qa/unit/embeddeddb_performancetest \
))

$(eval $(call gb_CppunitTest_use_libraries,dbaccess_embeddeddb_performancetest, \
    comphelper \
    cppu \
    cppuhelper \
    dbaxml \
    dbtools \
    firebird_sdbc \
    sal \
    subsequenttest \
    utl \
    test \
    tk \
    tl \
    unotest \
    xo \
))

$(eval $(call gb_CppunitTest_use_api,dbaccess_embeddeddb_performancetest,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,dbaccess_embeddeddb_performancetest))
$(eval $(call gb_CppunitTest_use_vcl,dbaccess_embeddeddb_performancetest))

$(eval $(call gb_CppunitTest_use_rdb,dbaccess_embeddeddb_performancetest,services))

$(eval $(call gb_CppunitTest_use_configuration,dbaccess_embeddeddb_performancetest))

# vim: set noet sw=4 ts=4:
