# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_fixedimage))

$(eval $(call gb_CppunitTest_set_include,vcl_fixedimage,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_fixedimage, \
	vcl/qa/cppunit/fixedimage \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_fixedimage,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,vcl_fixedimage, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svt \
	test \
	tk \
	tl \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_add_defs,vcl_fixedimage,\
    -DVCL_INTERNALS \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_fixedimage))

$(eval $(call gb_CppunitTest_use_ure,vcl_fixedimage))
$(eval $(call gb_CppunitTest_use_vcl,vcl_fixedimage))

$(eval $(call gb_CppunitTest_use_components,vcl_fixedimage,\
	configmgr/source/configmgr \
	i18npool/util/i18npool \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
	framework/util/fwk \
	sfx2/util/sfx \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_fixedimage))

$(eval $(call gb_CppunitTest_use_uiconfigs,vcl_fixedimage, \
    vcl \
))

# vim: set noet sw=4 ts=4:
