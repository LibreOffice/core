# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,scripting_beanshell))

$(eval $(call gb_CppunitTest_add_exception_objects,scripting_beanshell, \
    scripting/qa/cppunit/beanshell/beanshelltest \
))

$(eval $(call gb_CppunitTest_use_libraries,scripting_beanshell, \
    cppu \
    cppuhelper \
    jvmfwk \
    sal \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
))

$(eval $(call gb_CppunitTest_use_externals,scripting_beanshell,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_api,scripting_beanshell,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_jars,scripting_beanshell, \
    ScriptProviderForBeanShell \
))

$(eval $(call gb_CppunitTest_use_ure,scripting_beanshell))
$(eval $(call gb_CppunitTest_use_vcl,scripting_beanshell))
$(eval $(call gb_CppunitTest_use_rdbs,scripting_beanshell, \
    services \
    scriptproviderforbeanshell \
))
$(eval $(call gb_CppunitTest_use_configuration,scripting_beanshell))

# vim: set noet sw=4 ts=4:
