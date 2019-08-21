# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_drawable))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_drawable, \
    vcl/qa/cppunit/DrawableTest \
))

$(eval $(call gb_CppunitTest_set_include,vcl_drawable,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_drawable, \
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

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_drawable))

$(eval $(call gb_CppunitTest_use_ure,vcl_drawable))
$(eval $(call gb_CppunitTest_use_vcl,vcl_drawable))

$(eval $(call gb_CppunitTest_use_components,vcl_drawable,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    ucb/source/core/ucb1 \
    unotools/util/utl \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_drawable))

# vim: set noet sw=4 ts=4:
