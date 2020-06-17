# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,connectivity_mysql_test))

$(eval $(call gb_CppunitTest_use_external,connectivity_mysql_test,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,connectivity_mysql_test, \
    connectivity/qa/connectivity/mysql/mysql \
))

$(eval $(call gb_CppunitTest_use_libraries,connectivity_mysql_test, \
    comphelper \
    cppu \
    dbaxml \
    sal \
    subsequenttest \
    svt \
    test \
    unotest \
    utl \
    xo \
))

$(eval $(call gb_CppunitTest_use_api,connectivity_mysql_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,connectivity_mysql_test))
$(eval $(call gb_CppunitTest_use_vcl,connectivity_mysql_test))

$(eval $(call gb_CppunitTest_use_components,connectivity_mysql_test,\
    basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    connectivity/source/drivers/mysqlc/mysqlc \
    connectivity/source/manager/sdbc2 \
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
    uui/util/uui \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,connectivity_mysql_test))

# vim: set noet sw=4 ts=4:
