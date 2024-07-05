# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,package2_test))

$(eval $(call gb_CppunitTest_add_exception_objects,package2_test, \
    package/qa/cppunit/test_package \
    package/qa/cppunit/test_zippackage \
))

$(eval $(call gb_CppunitTest_use_libraries,package2_test, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    unotest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,package2_test))

$(eval $(call gb_CppunitTest_use_components,package2_test,\
    configmgr/source/configmgr \
    package/source/xstor/xstor \
    package/util/package2 \
	sax/source/expatwrap/expwrap \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_use_ure,package2_test))

$(eval $(call gb_CppunitTest_use_configuration,package2_test))

# vim: set noet sw=4 ts=4:
