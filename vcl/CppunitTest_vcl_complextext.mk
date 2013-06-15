# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_complextext))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_complextext, \
    vcl/qa/cppunit/complextext \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_complextext, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
    svt \
	test \
	tl \
	unotest \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_api,vcl_complextext,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_ure,vcl_complextext))

$(eval $(call gb_CppunitTest_use_components,vcl_complextext,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    ucb/source/core/ucb1 \
))

$(eval $(call gb_CppunitTest_use_configuration,vcl_complextext))

# vim: set noet sw=4 ts=4:
