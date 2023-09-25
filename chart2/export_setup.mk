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

# template for export tests
define chart2_export$(1)_test

$(eval $(call gb_CppunitTest_CppunitTest,chart2_export$(1)))

$(eval $(call gb_CppunitTest_use_externals,chart2_export$(1), \
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,chart2_export$(1), \
    chart2/qa/extras/chart2export$(1) \
))

$(eval $(call gb_CppunitTest_use_libraries,chart2_export$(1), \
    $(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sc \
    sw \
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
    $(call gb_Helper_optional,SCRIPTING, \
        vbahelper) \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_set_include,chart2_export$(1),\
    -I$(SRCDIR)/chart2/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,chart2_export$(1)))

$(eval $(call gb_CppunitTest_use_ure,chart2_export$(1)))
$(eval $(call gb_CppunitTest_use_vcl,chart2_export$(1)))
$(eval $(call gb_CppunitTest_use_rdb,chart2_export$(1),services))

$(eval $(call gb_CppunitTest_use_uiconfigs,chart2_export$(1), \
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_configuration,chart2_export$(1)))

$(call gb_CppunitTest_get_target,chart2_export$(1)): $(call gb_Package_get_target,postprocess_images)

$(eval $(call gb_CppunitTest_add_arguments,chart2_export$(1), \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

endef

# vim: set noet sw=4 ts=4:
