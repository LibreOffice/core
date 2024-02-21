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

# template for rtfexport tests
define sw_rtfexport_test

$(eval $(call gb_CppunitTest_CppunitTest,sw_rtfexport$(1)))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_rtfexport$(1)))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_rtfexport$(1), \
    sw/qa/extras/rtfexport/rtfexport$(1) \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_rtfexport$(1), \
    comphelper \
    cppu \
    cppuhelper \
    docmodel \
    editeng \
    i18nlangtag \
    sal \
    sfx \
    subsequenttest \
    svl \
    sw \
    swqahelper \
    test \
    unotest \
    utl \
    vcl \
    tl \
    basegfx \
))

$(eval $(call gb_CppunitTest_use_externals,sw_rtfexport$(1),\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_rtfexport$(1),\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_rtfexport$(1),\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_rtfexport$(1)))
$(eval $(call gb_CppunitTest_use_vcl,sw_rtfexport$(1)))

$(eval $(call gb_CppunitTest_use_rdb,sw_rtfexport$(1),services))

$(eval $(call gb_CppunitTest_use_configuration,sw_rtfexport$(1)))

endef

# vim: set noet sw=4 ts=4:
