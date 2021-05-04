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

$(eval $(call gb_CppunitTest_CppunitTest,canvas_test))

$(eval $(call gb_CppunitTest_add_exception_objects,canvas_test, \
    canvas/qa/cppunit/canvastest \
))

$(eval $(call gb_CppunitTest_use_external,canvas_test,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,canvas_test, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    tl \
    unotest \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,canvas_test))

$(eval $(call gb_CppunitTest_use_ure,canvas_test))
$(eval $(call gb_CppunitTest_use_vcl,canvas_test))

$(eval $(call gb_CppunitTest_use_components,canvas_test,\
    canvas/source/vcl/vclcanvas \
    canvas/source/factory/canvasfactory \
    configmgr/source/configmgr \
    i18npool/util/i18npool \
))

$(eval $(call gb_CppunitTest_use_configuration,canvas_test))

# vim: set noet sw=4 ts=4:
