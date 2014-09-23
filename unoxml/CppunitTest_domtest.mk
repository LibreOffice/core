# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,unoxml_test_domtest))

$(eval $(call gb_CppunitTest_add_exception_objects,unoxml_test_domtest, \
    unoxml/test/domtest \
))

$(eval $(call gb_CppunitTest_set_include,unoxml_test_domtest,\
    -I$(SRCDIR)/unoxml/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,unoxml_test_domtest,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,unoxml_test_domtest, \
    basegfx \
	comphelper \
	cppu \
	cppuhelper \
	drawinglayer \
	editeng \
	for \
    forui \
	i18nlangtag \
	i18nutil \
	msfilter \
	lng \
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
	unotest \
	vcl \
	tk \
	tl \
	ucbhelper \
	utl \
	vbahelper \
	xmlreader \
	xo \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,unoxml_test_domtest,\
	boost_headers \
	icuuc \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_components,unoxml_test_domtest,\
    basic/util/sb \
    #sembeddedobj/util/embobj \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    embeddedobj/util/embobj \
    filter/source/config/cache/filterconfig1 \
    filter/source/storagefilterdetect/storagefd \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    scripting/source/basprov/basprov \
    scripting/util/scriptframe \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,unoxml_test_domtest))
$(eval $(call gb_CppunitTest_use_ure,unoxml_test_domtest))
$(eval $(call gb_CppunitTest_use_vcl,unoxml_test_domtest))

$(eval $(call gb_CppunitTest_use_unittest_configuration,unoxml_test_domtest))

# vim: set noet sw=4 ts=4:
