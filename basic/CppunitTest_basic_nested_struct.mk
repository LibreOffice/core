# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,basic_nested_struct))

$(eval $(call gb_CppunitTest_add_exception_objects,basic_nested_struct, \
    basic/qa/cppunit/test_nested_struct \
))

# add a list of all needed libraries here
$(eval $(call gb_CppunitTest_use_libraries,basic_nested_struct, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    sb \
    sot \
    svl \
    svt \
    tl \
    utl \
    vcl \
    xmlscript \
    test \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_add_standard_system_libs,basic_nested_struct))

ifeq ($(GUI),WNT)
$(eval $(call gb_CppunitTest_use_system_win32_libs,basic_nested_struct, \
	oleaut32 \
))
endif

$(eval $(call gb_CppunitTest_set_include,basic_nested_struct,\
-I$(SRCDIR)/basic/source/inc \
-I$(SRCDIR)/basic/inc \
$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,basic_nested_struct,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,basic_nested_struct))

$(eval $(call gb_CppunitTest_use_components,basic_nested_struct,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
))
$(eval $(call gb_CppunitTest_use_configuration,basic_nested_struct))

# vim: set noet sw=4 ts=4:
