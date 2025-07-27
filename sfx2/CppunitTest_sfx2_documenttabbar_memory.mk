# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sfx2_documenttabbar_memory))

$(eval $(call gb_CppunitTest_use_external,sfx2_documenttabbar_memory,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,sfx2_documenttabbar_memory, \
    sfx2/qa/cppunit/test_documenttabbar_memory \
))

$(eval $(call gb_CppunitTest_use_libraries,sfx2_documenttabbar_memory, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    fwk \
    i18nlangtag \
    i18nutil \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
    test \
    tl \
    tk \
    ucbhelper \
    unotest \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_use_api,sfx2_documenttabbar_memory,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_ure,sfx2_documenttabbar_memory))
$(eval $(call gb_CppunitTest_use_vcl,sfx2_documenttabbar_memory))

$(eval $(call gb_CppunitTest_use_rdb,sfx2_documenttabbar_memory,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sfx2_documenttabbar_memory,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sfx2_documenttabbar_memory))

# Memory tests may need more time and resources
$(eval $(call gb_CppunitTest_set_non_application_font_use,sfx2_documenttabbar_memory))

# vim: set noet sw=4 ts=4: