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

$(eval $(call gb_CppunitTest_CppunitTest,sc_tiledrendering))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_tiledrendering, \
    sc/qa/unit/tiledrendering/tiledrendering \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_tiledrendering, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    sal \
    sfx \
    svl \
    svt \
    svxcore \
    sc \
    scui \
    test \
    unotest \
    vcl \
    tl \
    utl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sc_tiledrendering,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sc_tiledrendering,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_tiledrendering,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_tiledrendering))
$(eval $(call gb_CppunitTest_use_vcl,sc_tiledrendering))

$(eval $(call gb_CppunitTest_use_rdb,sc_tiledrendering,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_tiledrendering))

$(call gb_CppunitTest_get_target,sc_tiledrendering) : $(call gb_AllLangResTarget_get_target,sc)

# vim: set noet sw=4 ts=4:
