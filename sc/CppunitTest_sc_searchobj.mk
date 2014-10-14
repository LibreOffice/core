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

$(eval $(call gb_CppunitTest_CppunitTest,sc_searchobj))

$(eval $(call gb_CppunitTest_use_external,sc_searchobj,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_searchobj, \
    sc/qa/perf/scsearchobj \
    sc/qa/perf/perf_instrumentation \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_searchobj, \
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
    vbahelper \
    vcl \
    xo \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sc_searchobj,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_searchobj,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_searchobj))
$(eval $(call gb_CppunitTest_use_vcl,sc_searchobj))


$(eval $(call gb_CppunitTest_use_rdb,sc_searchobj,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_searchobj))

$(eval $(call gb_CppunitTest_use_unittest_configuration,sc_searchobj))

# vim: set noet sw=4 ts=4:
