# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,basic_macros))

$(eval $(call gb_CppunitTest_add_exception_objects,basic_macros, \
	basic/qa/cppunit/basictest \
	basic/qa/cppunit/basic_coverage \
	basic/qa/cppunit/test_append \
	basic/qa/cppunit/test_compiler_checks \
	basic/qa/cppunit/test_language_conditionals \
	basic/qa/cppunit/test_nested_struct \
	basic/qa/cppunit/test_vba \
	basic/qa/cppunit/test_global_as_new \
	basic/qa/cppunit/test_global_array \
	basic/qa/cppunit/test_tdf149714_bits_per_pixel \
))

$(eval $(call gb_CppunitTest_use_libraries,basic_macros, \
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	salhelper \
	sb \
	sot \
	svl \
	svt \
	test \
	tl \
	unotest \
	utl \
	vcl \
	xmlscript \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_system_win32_libs,basic_macros, \
	oleaut32 \
	odbc32 \
))
endif

$(eval $(call gb_CppunitTest_use_api,basic_macros,\
    offapi \
    udkapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,basic_macros))
$(eval $(call gb_CppunitTest_use_vcl,basic_macros))

$(eval $(call gb_CppunitTest_use_rdb,basic_macros,services))

$(eval $(call gb_CppunitTest_use_configuration,basic_macros))

# vim: set noet sw=4 ts=4:
