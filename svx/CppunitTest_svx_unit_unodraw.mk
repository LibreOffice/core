# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svx_unit_unodraw))

$(eval $(call gb_CppunitTest_use_externals,svx_unit_unodraw,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_sdk_api,svx_unit_unodraw))

$(eval $(call gb_CppunitTest_set_include,svx_unit_unodraw,\
    -I$(SRCDIR)/svx/source/inc \
    -I$(SRCDIR)/svx/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,svx_unit_unodraw, \
	svx/qa/unit/unodraw \
))

$(eval $(call gb_CppunitTest_use_libraries,svx_unit_unodraw, \
	basegfx \
	docmodel \
	drawinglayer \
	drawinglayercore \
    editeng \
	sal \
    subsequenttest \
	sfx \
    svl \
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

$(eval $(call gb_CppunitTest_use_sdk_api,svx_unit_unodraw))
$(eval $(call gb_CppunitTest_use_ure,svx_unit_unodraw))
$(eval $(call gb_CppunitTest_use_vcl,svx_unit_unodraw))
$(eval $(call gb_CppunitTest_use_rdb,svx_unit_unodraw,services))
$(eval $(call gb_CppunitTest_use_configuration,svx_unit_unodraw))

$(eval $(call gb_CppunitTest_add_arguments,svx_unit_unodraw, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
