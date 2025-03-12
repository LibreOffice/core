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

$(eval $(call gb_CppunitTest_CppunitTest,sd_lokit_search))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sd_lokit_search))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_lokit_search, \
    sd/qa/unit/tiledrendering/LOKitSearchTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sd_lokit_search, \
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
    sdqahelper \
    test \
    unotest \
    vcl \
    tl \
    utl \
))

$(eval $(call gb_CppunitTest_use_externals,sd_lokit_search,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sd_lokit_search,\
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/qa/unit \
	-I$(SRCDIR)/sd/qa/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sd_lokit_search))

$(eval $(call gb_CppunitTest_use_ure,sd_lokit_search))
$(eval $(call gb_CppunitTest_use_vcl,sd_lokit_search))

$(eval $(call gb_CppunitTest_use_rdb,sd_lokit_search,services))

$(eval $(call gb_CppunitTest_use_configuration,sd_lokit_search))

$(eval $(call gb_CppunitTest_use_uiconfigs,sd_lokit_search, \
    modules/simpress \
    svx \
))


$(eval $(call gb_CppunitTest_add_arguments,sd_lokit_search, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
