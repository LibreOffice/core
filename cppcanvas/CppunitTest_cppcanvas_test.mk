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

$(eval $(call gb_CppunitTest_unset_headless,cppcanvas_test))

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
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_api,cppcanvas_test,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,cppcanvas_test))

$(eval $(call gb_CppunitTest_use_components,cppcanvas_test,\
	canvas/source/cairo/cairocanvas \
	canvas/source/factory/canvasfactory \
    configmgr/source/configmgr \
    fileaccess/source/fileacc \
    i18npool/util/i18npool \
	linguistic/source/lng \
	package/util/package2 \
    sax/source/expatwrap/expwrap \
    ucb/source/core/ucb1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_configuration,cppcanvas_test))

# vim: set noet sw=4 ts=4:
