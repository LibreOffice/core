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

$(eval $(call gb_CppunitTest_CppunitTest,oox_wpc_drawing_canvas))

$(eval $(call gb_CppunitTest_use_externals,oox_wpc_drawing_canvas,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,oox_wpc_drawing_canvas, \
    oox/qa/unit/wpc_drawing_canvas \
))

$(eval $(call gb_CppunitTest_use_libraries,oox_wpc_drawing_canvas, \
    comphelper \
    cppu \
    cppuhelper \
    docmodel \
    oox \
    sal \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,oox_wpc_drawing_canvas))

$(eval $(call gb_CppunitTest_use_ure,oox_wpc_drawing_canvas))
$(eval $(call gb_CppunitTest_use_vcl,oox_wpc_drawing_canvas))

$(eval $(call gb_CppunitTest_use_rdb,oox_wpc_drawing_canvas,services))

$(eval $(call gb_CppunitTest_use_custom_headers,oox_wpc_drawing_canvas,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,oox_wpc_drawing_canvas))

$(eval $(call gb_CppunitTest_add_arguments,oox_wpc_drawing_canvas, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
