# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,scripting_basprov))

$(eval $(call gb_CppunitTest_add_exception_objects,scripting_basprov, \
    scripting/qa/cppunit/basprov/basprovtest \
))

$(eval $(call gb_CppunitTest_use_libraries,scripting_basprov, \
    cppu \
    cppuhelper \
    sal \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
))

$(eval $(call gb_CppunitTest_use_externals,scripting_basprov,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_api,scripting_basprov,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_ure,scripting_basprov))
$(eval $(call gb_CppunitTest_use_vcl,scripting_basprov))
$(eval $(call gb_CppunitTest_use_rdb,scripting_basprov,services))
$(eval $(call gb_CppunitTest_use_configuration,scripting_basprov))

# vim: set noet sw=4 ts=4:
