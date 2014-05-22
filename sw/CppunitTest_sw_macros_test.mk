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

$(eval $(call gb_CppunitTest_CppunitTest,sw_macros_test))

$(eval $(call gb_CppunitTest_use_external,sw_macros_test,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_macros_test, \
    sw/qa/core/macros-test \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_macros_test, \
    avmedia \
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
    sw \
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
    vbahelper \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sw_macros_test,\
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_macros_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_macros_test))

$(eval $(call gb_CppunitTest_use_components,sw_macros_test,\
	basic/util/sb \
	embeddedobj/util/embobj \
	comphelper/util/comphelp \
	configmgr/source/configmgr \
	dbaccess/util/dba \
	filter/source/config/cache/filterconfig1 \
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
	sw/util/msword \
	sw/util/sw \
	sw/util/swd \
	sw/util/vbaswobj \
	toolkit/util/tk \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
	ucb/source/ucp/tdoc/ucptdoc1 \
	unotools/util/utl \
	unoxml/source/rdf/unordf \
	unoxml/source/service/unoxml \
	xmloff/util/xo \
	xmlsecurity/util/$(if $(filter WNT,$(OS)),xsec_xmlsec.windows,xsec_xmlsec) \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_macros_test))

$(eval $(call gb_CppunitTest_use_unittest_configuration,sw_macros_test))

# vim: set noet sw=4 ts=4:
