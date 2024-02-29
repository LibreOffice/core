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

$(eval $(call gb_CppunitTest_CppunitTest,sw_tiledrendering2))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sw_tiledrendering2))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_tiledrendering2, \
    sw/qa/extras/tiledrendering2/tiledrendering2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_tiledrendering2, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    docmodel \
    sal \
    sfx \
    subsequenttest \
    svl \
    svt \
    svxcore \
    sw \
    swqahelper \
    test \
    unotest \
    vcl \
    tl \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_tiledrendering2,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_tiledrendering2,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_tiledrendering2,\
    udkapi \
    offapi \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_tiledrendering2))
$(eval $(call gb_CppunitTest_use_vcl,sw_tiledrendering2))

$(eval $(call gb_CppunitTest_use_rdb,sw_tiledrendering2,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_tiledrendering2))

ifneq ($(filter MORE_FONTS,$(BUILD_TYPE)),)
$(eval $(call gb_CppunitTest_set_non_application_font_use,sw_tiledrendering2,abort))
endif

# vim: set noet sw=4 ts=4:
