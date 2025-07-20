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

$(eval $(call gb_CppunitTest_CppunitTest,sc_sheetview_test))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_sheetview_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_sheetview_test, \
    sc/qa/unit/tiledrendering/SheetViewTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_sheetview_test, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    sal \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    sc \
    scfilt \
    scui \
    scqahelper \
    subsequenttest \
    test \
    unotest \
    $(call gb_Helper_optional,SCRIPTING, vbahelper) \
    vcl \
    tl \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sc_sheetview_test,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sc_sheetview_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/qa/unit/helper \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_sheetview_test))
$(eval $(call gb_CppunitTest_use_api,sc_sheetview_test,oovbaapi))
$(eval $(call gb_CppunitTest_use_ure,sc_sheetview_test))
$(eval $(call gb_CppunitTest_use_vcl,sc_sheetview_test))
$(eval $(call gb_CppunitTest_use_rdb,sc_sheetview_test,services))
$(eval $(call gb_CppunitTest_use_configuration,sc_sheetview_test))

$(eval $(call gb_CppunitTest_add_arguments,sc_sheetview_test, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

$(eval $(call gb_CppunitTest_use_uiconfigs,sc_sheetview_test, \
    modules/scalc \
    sfx \
    svt \
))

# vim: set noet sw=4 ts=4:
