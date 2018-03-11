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

$(eval $(call gb_CppunitTest_CppunitTest,sc_databaserangeobj))

$(eval $(call gb_CppunitTest_use_external,sc_databaserangeobj,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_databaserangeobj, \
    sc/qa/extras/scdatabaserangeobj \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_databaserangeobj, \
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
))

$(eval $(call gb_CppunitTest_set_include,sc_databaserangeobj,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_databaserangeobj))

$(eval $(call gb_CppunitTest_use_ure,sc_databaserangeobj))
$(eval $(call gb_CppunitTest_use_vcl,sc_databaserangeobj))

$(eval $(call gb_CppunitTest_use_components,sc_databaserangeobj,\
    $(sc_unoapi_common_components) \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_databaserangeobj))

# vim: set noet sw=4 ts=4:
