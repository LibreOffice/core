# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,linguistic_restprotocol))

$(eval $(call gb_CppunitTest_add_exception_objects,linguistic_restprotocol, \
    linguistic/qa/restprotocol \
))

$(eval $(call gb_CppunitTest_use_libraries,linguistic_restprotocol, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    svt \
    utl \
    test \
    unotest \
))

$(eval $(call gb_CppunitTest_use_api,linguistic_restprotocol,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_configuration,linguistic_restprotocol))

$(eval $(call gb_CppunitTest_use_ure,linguistic_restprotocol))

$(eval $(call gb_CppunitTest_use_rdb,linguistic_restprotocol,services))

$(eval $(call gb_CppunitTest_use_custom_headers,linguistic_restprotocol,\
    officecfg/registry \
))

# vim: set noet sw=4 ts=4:
