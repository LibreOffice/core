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

$(eval $(call gb_CppunitTest_CppunitTest,sc_perfobj))

$(eval $(call gb_CppunitTest_use_external,sc_perfobj,boost_headers))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_perfobj))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_perfobj, \
    sc/qa/perf/scperfobj \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_perfobj, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nlangtag \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sc \
    sfx \
    sot \
    subsequenttest \
    svl \
    svt \
    svx \
    svxcore \
    test \
    tk \
    tl \
    ucbhelper \
    unotest \
    utl \
    $(call gb_Helper_optional,SCRIPTING, \
        vbahelper) \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_set_include,sc_perfobj,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_perfobj, \
    oovbaapi \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_perfobj))

$(eval $(call gb_CppunitTest_use_ure,sc_perfobj))
$(eval $(call gb_CppunitTest_use_vcl,sc_perfobj))


$(eval $(call gb_CppunitTest_use_rdb,sc_perfobj,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_perfobj))

# vim: set noet sw=4 ts=4:
