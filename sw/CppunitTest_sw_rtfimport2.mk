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

$(eval $(call gb_CppunitTest_CppunitTest,sw_rtfimport2))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_rtfimport2))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_rtfimport2, \
    sw/qa/extras/rtfimport/rtfimport2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_rtfimport2, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sfx \
	subsequenttest \
	i18nlangtag \
	svl \
	sw \
	swqahelper \
    test \
    unotest \
    vcl \
    tl \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_rtfimport2,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_rtfimport2,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_rtfimport2,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_rtfimport2))
$(eval $(call gb_CppunitTest_use_vcl,sw_rtfimport2))

$(eval $(call gb_CppunitTest_use_rdb,sw_rtfimport2,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_rtfimport2))

$(eval $(call gb_CppunitTest_use_uiconfigs,sw_rtfimport2,\
    modules/swriter \
))

$(eval $(call gb_CppunitTest_use_more_fonts,sw_rtfimport2))

# vim: set noet sw=4 ts=4:
