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

$(eval $(call gb_CppunitTest_CppunitTest,cppcanvas_test))

$(eval $(call gb_CppunitTest_add_exception_objects,cppcanvas_test, \
    cppcanvas/qa/unit/test \
))

$(eval $(call gb_CppunitTest_use_external,cppcanvas_test,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,cppcanvas_test, \
	cppcanvas \
	basegfx \
	canvastools \
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	svt \
	test \
	tl \
	unotest \
	vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,cppcanvas_test))

$(eval $(call gb_CppunitTest_use_ure,cppcanvas_test))
$(eval $(call gb_CppunitTest_use_vcl,cppcanvas_test))

$(eval $(call gb_CppunitTest_use_components,cppcanvas_test,\
	canvas/source/vcl/vclcanvas \
	canvas/source/factory/canvasfactory \
    configmgr/source/configmgr \
    i18npool/util/i18npool \
	linguistic/source/lng \
	package/util/package2 \
    sax/source/expatwrap/expwrap \
    ucb/source/core/ucb1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_configuration,cppcanvas_test))

# vim: set noet sw=4 ts=4:
