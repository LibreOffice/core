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

$(eval $(call gb_CppunitTest_CppunitTest,sd_tiledrendering2))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sd_tiledrendering2))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_tiledrendering2, \
    sd/qa/unit/tiledrendering2/tiledrendering2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_tiledrendering2, \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    sal \
    sfx \
    subsequenttest \
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

$(eval $(call gb_CppunitTest_use_externals,sd_tiledrendering2,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sd_tiledrendering2,\
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/qa/unit \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_tiledrendering2))

$(eval $(call gb_CppunitTest_use_ure,sd_tiledrendering2))
$(eval $(call gb_CppunitTest_use_vcl,sd_tiledrendering2))

$(eval $(call gb_CppunitTest_use_rdb,sd_tiledrendering2,services))

$(eval $(call gb_CppunitTest_use_configuration,sd_tiledrendering2))

$(eval $(call gb_CppunitTest_use_uiconfigs,sd_tiledrendering2, \
    modules/simpress \
    svx \
))


$(eval $(call gb_CppunitTest_add_arguments,sd_tiledrendering2, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
