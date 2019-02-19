# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svx_unit))

$(eval $(call gb_CppunitTest_use_external,svx_unit,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,svx_unit))

$(eval $(call gb_CppunitTest_set_include,svx_unit,\
    -I$(SRCDIR)/svx/source/inc \
    -I$(SRCDIR)/svx/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,svx_unit, \
	svx/qa/unit/svdraw/test_SdrTextObject \
	svx/qa/unit/customshapes \
	svx/qa/unit/unodraw \
	svx/qa/unit/xoutdev \
	svx/qa/unit/XTableImportExportTest \
))

$(eval $(call gb_CppunitTest_use_libraries,svx_unit, \
	basegfx \
	sal \
	sfx \
	svxcore \
	svx \
	tl \
	unotest \
	vcl \
	utl \
	comphelper \
	cppuhelper \
	cppu \
	test \
	unotest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,svx_unit))
$(eval $(call gb_CppunitTest_use_ure,svx_unit))
$(eval $(call gb_CppunitTest_use_vcl,svx_unit))
$(eval $(call gb_CppunitTest_use_rdb,svx_unit,services))
$(eval $(call gb_CppunitTest_use_configuration,svx_unit))

# vim: set noet sw=4 ts=4:
