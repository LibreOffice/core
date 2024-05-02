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

$(eval $(call gb_CppunitTest_CppunitTest,sw_writerfilter_filter))

$(eval $(call gb_CppunitTest_use_externals,sw_writerfilter_filter,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_writerfilter_filter, \
    sw/qa/writerfilter/cppunittests/filter/WriterFilter \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_writerfilter_filter, \
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

$(eval $(call gb_CppunitTest_use_sdk_api,sw_writerfilter_filter))

$(eval $(call gb_CppunitTest_use_ure,sw_writerfilter_filter))
$(eval $(call gb_CppunitTest_use_vcl,sw_writerfilter_filter))

$(eval $(call gb_CppunitTest_use_rdb,sw_writerfilter_filter,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_writerfilter_filter,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_writerfilter_filter))

# we need to explicitly depend on library sw_writerfilter because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,sw_writerfilter_filter) : $(call gb_Library_get_target,sw_writerfilter)

ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_writerfilter_filter,abort))
endif

# vim: set noet sw=4 ts=4:
