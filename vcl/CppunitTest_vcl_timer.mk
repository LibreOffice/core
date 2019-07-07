# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_timer))

$(eval $(call gb_CppunitTest_set_include,vcl_timer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_timer, \
	vcl/qa/cppunit/timer \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_timer,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,vcl_timer, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	test \
	tl \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,vcl_timer))

$(eval $(call gb_CppunitTest_use_ure,vcl_timer))
$(eval $(call gb_CppunitTest_use_vcl,vcl_timer))

$(eval $(call gb_CppunitTest_use_components,vcl_timer,\
	configmgr/source/configmgr \
	i18npool/util/i18npool \
	ucb/source/core/ucb1 \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_timer))

# vim: set noet sw=4 ts=4:
