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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ww8export2))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ww8export2, \
    sw/qa/extras/ww8export/ww8export2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ww8export2, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    emboleobj \
    sal \
    sfx \
    test \
    unotest \
    utl \
    sw \
    tl \
    svxcore \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ww8export2,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ww8export2,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_ww8export2,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ww8export2))
$(eval $(call gb_CppunitTest_use_vcl,sw_ww8export2))

$(eval $(call gb_CppunitTest_use_rdb,sw_ww8export2,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_ww8export2))

# At least testTdf118133::Import_Export_Import (sw/qa/extras/ww8export/ww8export2.cxx) depends on
# TIFFReader from Library_gie, which is loaded dynamically in vcl/source/filter/graphicfilter.cxx:
$(call gb_CppunitTest_get_target,sw_ww8export2): $(call gb_Library_get_target,gie)

# vim: set noet sw=4 ts=4:
