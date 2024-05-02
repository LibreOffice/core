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

$(eval $(call gb_CppunitTest_CppunitTest,writerfilter_filter))

$(eval $(call gb_CppunitTest_use_externals,writerfilter_filter,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,writerfilter_filter, \
    writerfilter/qa/cppunittests/filter/WriterFilter \
))

$(eval $(call gb_CppunitTest_use_libraries,writerfilter_filter, \
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

$(eval $(call gb_CppunitTest_use_sdk_api,writerfilter_filter))

$(eval $(call gb_CppunitTest_use_ure,writerfilter_filter))
$(eval $(call gb_CppunitTest_use_vcl,writerfilter_filter))

$(eval $(call gb_CppunitTest_use_rdb,writerfilter_filter,services))

$(eval $(call gb_CppunitTest_use_custom_headers,writerfilter_filter,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,writerfilter_filter))

# we need to explicitly depend on library writerfilter because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,writerfilter_filter) : $(call gb_Library_get_target,writerfilter)

# vim: set noet sw=4 ts=4:
