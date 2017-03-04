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

$(eval $(call gb_CppunitTest_CppunitTest,sw_tiledrendering))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_tiledrendering, \
    sw/qa/extras/tiledrendering/tiledrendering \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_tiledrendering, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    sal \
    sfx \
    svl \
    svt \
	svxcore \
    sw \
    test \
    unotest \
    vcl \
    tl \
	utl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_tiledrendering,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_tiledrendering,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_tiledrendering))

$(eval $(call gb_CppunitTest_use_ure,sw_tiledrendering))
$(eval $(call gb_CppunitTest_use_vcl,sw_tiledrendering))

$(eval $(call gb_CppunitTest_use_rdb,sw_tiledrendering,services))

$(eval $(call gb_CppunitTest_use_configuration,sw_tiledrendering))

# vim: set noet sw=4 ts=4:
