# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sfx2_documenttabbar_integration))

$(eval $(call gb_CppunitTest_use_external,sfx2_documenttabbar_integration,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,sfx2_documenttabbar_integration, \
    sfx2/qa/cppunit/test_documenttabbar_integration \
))

$(eval $(call gb_CppunitTest_use_libraries,sfx2_documenttabbar_integration, \
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
    sw \
    test \
    tl \
    tk \
    ucbhelper \
    unotest \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_use_api,sfx2_documenttabbar_integration,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_ure,sfx2_documenttabbar_integration))
$(eval $(call gb_CppunitTest_use_vcl,sfx2_documenttabbar_integration))

$(eval $(call gb_CppunitTest_use_rdb,sfx2_documenttabbar_integration,services))

$(eval $(call gb_CppunitTest_use_custom_headers,sfx2_documenttabbar_integration,\
	officecfg/registry \
))

$(eval $(call gb_CppunitTest_use_configuration,sfx2_documenttabbar_integration))

$(eval $(call gb_CppunitTest_use_more_fonts,sfx2_documenttabbar_integration))

# Enable document loading capabilities
$(eval $(call gb_CppunitTest_use_components,sfx2_documenttabbar_integration,\
    basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    connectivity/source/drivers/calc/calc \
    connectivity/source/drivers/writer/writer \
    connectivity/source/manager/sdbc2 \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    scripting/source/basprov/basprov \
    scripting/util/scriptframe \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    xmloff/util/xo \
))

# vim: set noet sw=4 ts=4: