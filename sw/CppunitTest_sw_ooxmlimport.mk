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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ooxmlimport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ooxmlimport, \
    sw/qa/extras/ooxmlimport/ooxmlimport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ooxmlimport, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sfx \
    test \
    unotest \
    utl \
    sw \
    tl \
    vcl \
    svxcore \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ooxmlimport,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ooxmlimport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

ifeq ($(OS),MACOSX)

$(eval $(call gb_CppunitTest_add_cxxflags,sw_ooxmlimport,\
    $(gb_OBJCXXFLAGS) \
))

$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,sw_ooxmlimport,\
	AppKit \
))

endif

$(eval $(call gb_CppunitTest_use_api,sw_ooxmlimport,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ooxmlimport))
$(eval $(call gb_CppunitTest_use_vcl,sw_ooxmlimport))

$(eval $(call gb_CppunitTest_use_rdb,sw_ooxmlimport,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_ooxmlimport))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_ooxmlimport))

# vim: set noet sw=4 ts=4:
