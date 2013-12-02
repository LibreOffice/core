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
))

$(eval $(call gb_CppunitTest_use_libraries,package2_test, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    unotest \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_api,package2_test,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_components,package2_test,\
    configmgr/source/configmgr \
    package/util/package2 \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_use_ure,package2_test))

$(eval $(call gb_CppunitTest_use_configuration,package2_test))

# vim: set noet sw=4 ts=4:
