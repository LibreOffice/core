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

$(eval $(call gb_CppunitTest_CppunitTest,sd_uiimpress))

$(eval $(call gb_CppunitTest_use_externals,sd_uiimpress,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sd_uiimpress))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_uiimpress, \
    sd/qa/unit/uiimpress \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_uiimpress, \
    $(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
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

$(eval $(call gb_CppunitTest_set_include,sd_uiimpress,\
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/source/ui/slidesorter/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_uiimpress))

$(eval $(call gb_CppunitTest_use_externals,sd_uiimpress,\
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_ure,sd_uiimpress))
$(eval $(call gb_CppunitTest_use_vcl,sd_uiimpress))

$(eval $(call gb_CppunitTest_use_rdb,sd_uiimpress,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sd_uiimpress,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sd_uiimpress))

$(eval $(call gb_CppunitTest_add_arguments,sd_uiimpress, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
