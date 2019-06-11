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

$(eval $(call gb_CppunitTest_CppunitTest,desktop_lib))

$(eval $(call gb_CppunitTest_add_exception_objects,desktop_lib, \
	desktop/qa/desktop_lib/test_desktop_lib \
))

$(eval $(call gb_CppunitTest_use_libraries,desktop_lib, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sc \
	scfilt \
	sfx \
	sofficeapp \
	subsequenttest \
	sw \
	test \
	unotest \
	utl \
	tl \
	vcl \
))

$(eval $(call gb_CppunitTest_use_externals,desktop_lib, \
    boost_headers \
    cairo \
))

$(eval $(call gb_CppunitTest_set_include,desktop_lib,\
    -I$(SRCDIR)/desktop/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,desktop_lib))

$(eval $(call gb_CppunitTest_use_ure,desktop_lib))

$(eval $(call gb_CppunitTest_use_vcl,desktop_lib))

$(eval $(call gb_CppunitTest_use_rdb,desktop_lib,services))

$(eval $(call gb_CppunitTest_use_configuration,desktop_lib))

$(eval $(call gb_CppunitTest_use_more_fonts,desktop_lib))

$(eval $(call gb_CppunitTest_use_packages,desktop_lib, \
    scripting_scriptbindinglib \
    wizards_basicshare \
    wizards_basicsrvaccess2base \
    wizards_basicsrvdepot \
    wizards_basicsrveuro \
    wizards_basicsrvform \
    wizards_basicsrvgimmicks \
    wizards_basicsrvimport \
    wizards_basicsrvtemplate \
    wizards_basicsrvtools \
))

$(eval $(call gb_CppunitTest_use_uiconfigs,desktop_lib, \
    cui \
    modules/swriter \
))

# vim: set noet sw=4 ts=4:
