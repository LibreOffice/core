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

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_tiledrendering))

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
    sot \
    svl \
    svt \
    svxcore \
    sc \
    scfilt \
    scui \
    subsequenttest \
    test \
    unotest \
    $(call gb_Helper_optional,SCRIPTING, \
        vbahelper) \
    vcl \
    tl \
    utl \
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

$(eval $(call gb_CppunitTest_use_sdk_api,sc_tiledrendering))
$(eval $(call gb_CppunitTest_use_api,sc_tiledrendering,oovbaapi))

$(eval $(call gb_CppunitTest_use_ure,sc_tiledrendering))
$(eval $(call gb_CppunitTest_use_vcl,sc_tiledrendering))

$(eval $(call gb_CppunitTest_use_rdb,sc_tiledrendering,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_tiledrendering))

$(eval $(call gb_CppunitTest_add_arguments,sc_tiledrendering, \
    -env:arg-env=$(gb_Helper_LIBRARY_PATH_VAR)"$$$${$(gb_Helper_LIBRARY_PATH_VAR)+=$$$$$(gb_Helper_LIBRARY_PATH_VAR)}" \
))

# vim: set noet sw=4 ts=4:
