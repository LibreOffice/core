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

$(eval $(call gb_CppunitTest_CppunitTest,sd_tiledrendering))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_tiledrendering, \
    sd/qa/unit/tiledrendering/tiledrendering \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_tiledrendering, \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    sal \
    sfx \
    svl \
    svt \
    svxcore \
    sd \
    test \
    unotest \
    vcl \
    tl \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sd_tiledrendering,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sd_tiledrendering,\
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/qa/unit \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_tiledrendering))

$(eval $(call gb_CppunitTest_use_ure,sd_tiledrendering))
$(eval $(call gb_CppunitTest_use_vcl,sd_tiledrendering))

$(eval $(call gb_CppunitTest_use_rdb,sd_tiledrendering,services))

$(eval $(call gb_CppunitTest_use_configuration,sd_tiledrendering))

# vim: set noet sw=4 ts=4:
