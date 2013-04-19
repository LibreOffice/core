# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,hwpfilter_test_hwpfilter))

$(eval $(call gb_CppunitTest_use_external,hwpfilter_test_hwpfilter,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,hwpfilter_test_hwpfilter, \
    hwpfilter/qa/cppunit/test_hwpfilter \
))

$(eval $(call gb_CppunitTest_use_libraries,hwpfilter_test_hwpfilter, \
    ucbhelper \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    unotest \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_api,hwpfilter_test_hwpfilter,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,hwpfilter_test_hwpfilter))

$(eval $(call gb_CppunitTest_use_components,hwpfilter_test_hwpfilter,\
    configmgr/source/configmgr \
    hwpfilter/source/hwp \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_use_configuration,hwpfilter_test_hwpfilter))

# vim: set noet sw=4 ts=4:
