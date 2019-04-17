# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,test_xpath))

$(eval $(call gb_CppunitTest_add_exception_objects,test_xpath, \
    test/qa/cppunit/test_xpath \
))

$(eval $(call gb_CppunitTest_use_externals,test_xpath,\
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,test_xpath, \
    cppu \
    cppuhelper \
    sal \
    test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,test_xpath))

# $(eval $(call gb_CppunitTest_use_ure,test_xpath))

# vim: set noet sw=4 ts=4:
