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

$(eval $(call gb_CppunitTest_CppunitTest,slideshow_engine))

$(eval $(call gb_CppunitTest_use_externals,slideshow_engine,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_exception_objects,slideshow_engine, \
    slideshow/qa/engine/engine \
))

$(eval $(call gb_CppunitTest_use_libraries,slideshow_engine, \
    comphelper \
    cppu \
    slideshow \
    cppuhelper \
    sal \
    subsequenttest \
    test \
    unotest \
    utl \
    tl \
))

$(eval $(call gb_CppunitTest_use_sdk_api,slideshow_engine))

$(eval $(call gb_CppunitTest_set_include,slideshow_engine,\
    -I$(SRCDIR)/slideshow/source/inc \
    -I$(SRCDIR)/slideshow/source/engine/animationnodes \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_ure,slideshow_engine))
$(eval $(call gb_CppunitTest_use_vcl,slideshow_engine))

$(eval $(call gb_CppunitTest_use_rdb,slideshow_engine,services))

$(eval $(call gb_CppunitTest_use_custom_headers,slideshow_engine,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,slideshow_engine))

# vim: set noet sw=4 ts=4:
