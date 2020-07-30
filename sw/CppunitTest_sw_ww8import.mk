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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ww8import))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_ww8import))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ww8import, \
    sw/qa/extras/ww8import/ww8import \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ww8import, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    sal \
    test \
    unotest \
    vcl \
    sfx \
    svl \
    sw \
	swqahelper \
    tl \
	utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ww8import,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ww8import,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_ww8import,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ww8import))
$(eval $(call gb_CppunitTest_use_vcl,sw_ww8import))

$(eval $(call gb_CppunitTest_use_rdb,sw_ww8import,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_ww8import))

# vim: set noet sw=4 ts=4:
