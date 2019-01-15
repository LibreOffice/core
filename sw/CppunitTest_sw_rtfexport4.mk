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

$(eval $(call gb_CppunitTest_CppunitTest,sw_rtfexport4))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_rtfexport4, \
    sw/qa/extras/rtfexport/rtfexport4 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_rtfexport4, \
    comphelper \
    cppu \
	cppuhelper \
    sal \
    sfx \
	sw \
    test \
    unotest \
	utl \
	tl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_rtfexport4,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_rtfexport4,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_add_cxxflags,sw_rtfexport4,\
	-bigobj \
))
endif

$(eval $(call gb_CppunitTest_use_api,sw_rtfexport4,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_rtfexport4))
$(eval $(call gb_CppunitTest_use_vcl,sw_rtfexport4))

$(eval $(call gb_CppunitTest_use_rdb,sw_rtfexport4,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_rtfexport4))

# vim: set noet sw=4 ts=4:
