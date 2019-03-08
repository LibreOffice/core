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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ooxmlexport8))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ooxmlexport8, \
    sw/qa/extras/ooxmlexport/ooxmlexport8 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ooxmlexport8, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    unotest \
    utl \
    sfx \
    sw \
    tl \
    vcl \
    svxcore \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ooxmlexport8,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ooxmlexport8,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

ifeq ($(OS),MACOSX)

$(eval $(call gb_CppunitTest_add_cxxflags,sw_ooxmlexport8,\
    $(gb_OBJCXXFLAGS) \
))

$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,sw_ooxmlexport8,\
	AppKit \
))

endif

$(eval $(call gb_CppunitTest_use_api,sw_ooxmlexport8,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ooxmlexport8))
$(eval $(call gb_CppunitTest_use_vcl,sw_ooxmlexport8))

$(eval $(call gb_CppunitTest_use_rdb,sw_ooxmlexport8,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_ooxmlexport8))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_ooxmlexport8,\
    modules/swriter \
))

# vim: set noet sw=4 ts=4:
