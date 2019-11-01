# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_B2DPolyPolyLineDrawableHelper))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_B2DPolyPolyLineDrawableHelper, \
    vcl/qa/cppunit/drawables/B2DPolyPolyLineDrawableHelperTest \
))

$(eval $(call gb_CppunitTest_set_include,vcl_B2DPolyPolyLineDrawableHelper,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_B2DPolyPolyLineDrawableHelper, \
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	sal \
    svt \
	test \
	tl \
	unotest \
	vcl \
	utl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_B2DPolyPolyLineDrawableHelper))

$(eval $(call gb_CppunitTest_use_ure,vcl_B2DPolyPolyLineDrawableHelper))
$(eval $(call gb_CppunitTest_use_vcl,vcl_B2DPolyPolyLineDrawableHelper))

$(eval $(call gb_CppunitTest_use_components,vcl_B2DPolyPolyLineDrawableHelper,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    ucb/source/core/ucb1 \
    unotools/util/utl \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_B2DPolyPolyLineDrawableHelper))

# vim: set noet sw=4 ts=4:
