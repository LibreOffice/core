# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,connectivity_mork))

$(eval $(call gb_CppunitTest_set_include,connectivity_mork,\
	-I$(SRCDIR)/connectivity/source/inc \
	-I$(SRCDIR)/connectivity/source/drivers/mork \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,connectivity_mork,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_ure,connectivity_mork))

$(eval $(call gb_CppunitTest_use_sdk_api,connectivity_mork))

$(eval $(call gb_CppunitTest_add_exception_objects,connectivity_mork, \
	connectivity/qa/connectivity/mork/DriverTest \
))

$(eval $(call gb_CppunitTest_use_libraries,connectivity_mork, \
    comphelper \
    cppu \
    cppuhelper \
    i18nisolang1 \
    mork \
    sal \
    salhelper \
    sb \
    test \
    unotest \
    ucbhelper \
    utl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_components,connectivity_mork,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    connectivity/source/drivers/mork/mork \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_use_configuration,connectivity_mork))

# vim: set noet sw=4 ts=4:
