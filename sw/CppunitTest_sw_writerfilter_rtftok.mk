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

$(eval $(call gb_CppunitTest_CppunitTest,sw_writerfilter_rtftok))

$(eval $(call gb_CppunitTest_use_externals,sw_writerfilter_rtftok,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_writerfilter_rtftok, \
    sw/qa/writerfilter/cppunittests/rtftok/rtfdispatchflag \
    sw/qa/writerfilter/cppunittests/rtftok/rtfdispatchsymbol \
    sw/qa/writerfilter/cppunittests/rtftok/rtfdispatchvalue \
    sw/qa/writerfilter/cppunittests/rtftok/rtfdocumentimpl \
    sw/qa/writerfilter/cppunittests/rtftok/rtfsdrimport \
    sw/qa/writerfilter/cppunittests/rtftok/rtfsprm \
    sw/qa/writerfilter/cppunittests/rtftok/rtftokenizer \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_writerfilter_rtftok, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    oox \
    sal \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
    vcl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_writerfilter_rtftok))

$(eval $(call gb_CppunitTest_use_ure,sw_writerfilter_rtftok))
$(eval $(call gb_CppunitTest_use_vcl,sw_writerfilter_rtftok))

$(eval $(call gb_CppunitTest_use_rdb,sw_writerfilter_rtftok,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_writerfilter_rtftok,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_writerfilter_rtftok))

# we need to explicitly depend on library sw_writerfilter because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,sw_writerfilter_rtftok) : $(call gb_Library_get_target,sw_writerfilter)

ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_writerfilter_rtftok,abort))
endif

# vim: set noet sw=4 ts=4:
