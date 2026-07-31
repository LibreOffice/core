# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,connectivity_firebird_test))

$(eval $(call gb_CppunitTest_use_externals,connectivity_firebird_test,\
    boost_headers \
    libxml2 \
))


$(eval $(call gb_CppunitTest_add_exception_objects,connectivity_firebird_test, \
    connectivity/qa/connectivity/firebird/firebird \
))

$(eval $(call gb_CppunitTest_use_libraries,connectivity_firebird_test, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    sal \
    sfx \
    subsequenttest \
    svl \
    test \
    tl \
    unotest \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_api,connectivity_firebird_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,connectivity_firebird_test))
$(eval $(call gb_CppunitTest_use_vcl,connectivity_firebird_test))

$(eval $(call gb_CppunitTest_use_rdb,connectivity_firebird_test,services))

$(eval $(call gb_CppunitTest_use_configuration,connectivity_firebird_test))

# vim: set noet sw=4 ts=4:
