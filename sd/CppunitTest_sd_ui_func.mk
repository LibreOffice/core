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

$(eval $(call gb_CppunitTest_CppunitTest,sd_ui_func))

$(eval $(call gb_CppunitTest_use_externals,sd_ui_func,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sd_ui_func))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_ui_func, \
    sd/qa/ui/func/func \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_ui_func, \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    docmodel \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    i18nutil \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sd \
    sfx \
    sot \
    subsequenttest \
    svl \
    svt \
    svx \
    svxcore \
    test \
    tl \
    tk \
    ucbhelper \
    unotest \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_set_include,sd_ui_func,\
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/source/ui/slidesorter/inc \
    -I$(SRCDIR)/sd/qa/unit \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_ui_func))

$(eval $(call gb_CppunitTest_use_ure,sd_ui_func))
$(eval $(call gb_CppunitTest_use_vcl,sd_ui_func))

$(eval $(call gb_CppunitTest_use_rdb,sd_ui_func,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sd_ui_func,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sd_ui_func))

$(eval $(call gb_CppunitTest_add_arguments,sd_ui_func, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
