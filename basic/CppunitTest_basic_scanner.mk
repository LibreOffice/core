# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,basic_scanner))

$(eval $(call gb_CppunitTest_use_ure,basic_scanner))

$(eval $(call gb_CppunitTest_add_exception_objects,basic_scanner, \
	basic/qa/cppunit/test_scanner \
))

$(eval $(call gb_CppunitTest_use_library_objects,basic_scanner,sb))

$(eval $(call gb_CppunitTest_use_libraries,basic_scanner, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
	i18nlangtag \
    sot \
    svl \
    svt \
    tl \
    utl \
    vcl \
    xmlscript \
	$(gb_UWINAPI) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_system_win32_libs,basic_scanner, \
	oleaut32 \
))
endif

$(eval $(call gb_CppunitTest_set_include,basic_scanner,\
-I$(SRCDIR)/basic/source/inc \
-I$(SRCDIR)/basic/inc \
$$(INCLUDE) \
))

# vim: set noet sw=4 ts=4:
