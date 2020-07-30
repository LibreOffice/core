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

$(eval $(call gb_CppunitTest_CppunitTest,sw_rtfexport2))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_rtfexport2))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_rtfexport2, \
    sw/qa/extras/rtfexport/rtfexport2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_rtfexport2, \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    sfx \
    sw \
	swqahelper \
    test \
    unotest \
    utl \
    vcl \
    tl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_rtfexport2,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_rtfexport2,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_rtfexport2,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_rtfexport2))
$(eval $(call gb_CppunitTest_use_vcl,sw_rtfexport2))

$(eval $(call gb_CppunitTest_use_rdb,sw_rtfexport2,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_rtfexport2))

# vim: set noet sw=4 ts=4:
