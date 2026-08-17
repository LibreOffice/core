# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,scripting_scripthandler))

$(eval $(call gb_CppunitTest_add_exception_objects,scripting_scripthandler, \
    scripting/qa/cppunit/scripthandler/ScriptHandlerTest \
))

$(eval $(call gb_CppunitTest_use_libraries,scripting_scripthandler, \
    cppu \
    cppuhelper \
    sal \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_externals,scripting_scripthandler,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_api,scripting_scripthandler,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_ure,scripting_scripthandler))
$(eval $(call gb_CppunitTest_use_vcl,scripting_scripthandler))
$(eval $(call gb_CppunitTest_use_rdb,scripting_scripthandler,services))
$(eval $(call gb_CppunitTest_use_configuration,scripting_scripthandler))

# vim: set noet sw=4 ts=4:
