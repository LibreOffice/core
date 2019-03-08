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

$(eval $(call gb_CppunitTest_CppunitTest,sw_rtfimport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_rtfimport, \
    sw/qa/extras/rtfimport/rtfimport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_rtfimport, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sfx \
	i18nlangtag \
	sw \
    test \
    unotest \
    vcl \
    tl \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_rtfimport,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_rtfimport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_add_cxxflags,sw_rtfimport,\
	-bigobj \
))
endif

$(eval $(call gb_CppunitTest_use_api,sw_rtfimport,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_rtfimport))
$(eval $(call gb_CppunitTest_use_vcl,sw_rtfimport))

$(eval $(call gb_CppunitTest_use_rdb,sw_rtfimport,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_rtfimport))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_rtfimport,\
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_rtfimport))

# vim: set noet sw=4 ts=4:
