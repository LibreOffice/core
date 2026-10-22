# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sot_ole))

$(eval $(call gb_CppunitTest_add_exception_objects,sot_ole, \
    sot/qa/cppunit/OLESimpleStorageUnitTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sot_ole, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sot \
    tl \
    unotest \
    test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sot_ole,))

$(eval $(call gb_CppunitTest_use_configuration,sot_ole))
$(eval $(call gb_CppunitTest_use_ure,sot_ole))
$(eval $(call gb_CppunitTest_use_vcl,sot_ole))

$(eval $(call gb_CppunitTest_use_components,sot_ole,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    sot/util/sot \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
))

# vim: set noet sw=4 ts=4:
