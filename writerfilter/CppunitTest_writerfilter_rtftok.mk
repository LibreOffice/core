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

$(eval $(call gb_CppunitTest_CppunitTest,writerfilter_rtftok))

$(eval $(call gb_CppunitTest_use_externals,writerfilter_rtftok,\
	boost_headers \
	libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,writerfilter_rtftok, \
    writerfilter/qa/cppunittests/rtftok/rtfdispatchflag \
    writerfilter/qa/cppunittests/rtftok/rtfdispatchsymbol \
    writerfilter/qa/cppunittests/rtftok/rtfdispatchvalue \
    writerfilter/qa/cppunittests/rtftok/rtfdocumentimpl \
    writerfilter/qa/cppunittests/rtftok/rtfsdrimport \
    writerfilter/qa/cppunittests/rtftok/rtfsprm \
    writerfilter/qa/cppunittests/rtftok/rtftokenizer \
))

$(eval $(call gb_CppunitTest_use_libraries,writerfilter_rtftok, \
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

$(eval $(call gb_CppunitTest_use_sdk_api,writerfilter_rtftok))

$(eval $(call gb_CppunitTest_use_ure,writerfilter_rtftok))
$(eval $(call gb_CppunitTest_use_vcl,writerfilter_rtftok))

$(eval $(call gb_CppunitTest_use_rdb,writerfilter_rtftok,services))

$(eval $(call gb_CppunitTest_use_custom_headers,writerfilter_rtftok,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,writerfilter_rtftok))

# we need to explicitly depend on library writerfilter because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,writerfilter_rtftok) : $(call gb_Library_get_target,writerfilter)

# vim: set noet sw=4 ts=4:
