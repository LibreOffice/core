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
    jvmfwk \
    sal \
    subsequenttest \
    utl \
    test \
    tk \
    tl \
    unotest \
    xo \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_api,dbaccess_embeddeddb_performancetest,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,dbaccess_embeddeddb_performancetest))
$(eval $(call gb_CppunitTest_use_vcl,dbaccess_embeddeddb_performancetest))

$(eval $(call gb_CppunitTest_use_components,dbaccess_embeddeddb_performancetest,\
    basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    connectivity/source/cpool/dbpool2 \
    connectivity/source/drivers/firebird/firebird_sdbc \
    connectivity/source/drivers/hsqldb/hsqldb \
    connectivity/source/drivers/jdbc/jdbc \
    connectivity/source/manager/sdbc2 \
    dbaccess/util/dba \
    dbaccess/source/filter/xml/dbaxml \
    dbaccess/util/dbu \
    filter/source/config/cache/filterconfig1 \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sfx2/util/sfx \
    svl/source/fsstor/fsstorage \
    svl/util/svl \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,dbaccess_embeddeddb_performancetest))

# vim: set noet sw=4 ts=4:
