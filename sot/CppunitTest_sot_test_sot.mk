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

$(eval $(call gb_CppunitTest_CppunitTest,sot_test_sot))

$(eval $(call gb_CppunitTest_add_exception_objects,sot_test_sot, \
    sot/qa/cppunit/test_sot \
))

$(eval $(call gb_CppunitTest_use_libraries,sot_test_sot, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sot \
    tl \
    unotest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sot_test_sot,))

$(eval $(call gb_CppunitTest_use_ure,sot_test_sot))

$(eval $(call gb_CppunitTest_use_components,sot_test_sot,\
    i18npool/util/i18npool \
))

# vim: set noet sw=4 ts=4:
