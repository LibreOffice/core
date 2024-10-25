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

$(eval $(call gb_CppunitTest_CppunitTest,editeng_core))

$(eval $(call gb_CppunitTest_add_exception_objects,editeng_core, \
    editeng/qa/unit/core-test \
    editeng/qa/unit/ESelectionTest \
    editeng/qa/unit/EPaMTest \
    editeng/qa/unit/EditLineTest \
    editeng/qa/unit/EditLineListTest \
    editeng/qa/unit/EditPaMTest \
    editeng/qa/unit/EditSelectionTest \
))

$(eval $(call gb_CppunitTest_use_library_objects,editeng_core,editeng))

$(eval $(call gb_CppunitTest_use_libraries,editeng_core, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    docmodel \
    i18nlangtag \
    i18nutil \
    lng \
    sal \
    salhelper \
    sax \
    sot \
    sfx \
    svl \
    svt \
    test \
    tk \
    tl \
    ucbhelper \
    unotest \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_use_externals,editeng_core,\
	boost_headers \
    icuuc \
	libxml2 \
	yrs \
))

$(eval $(call gb_CppunitTest_set_include,editeng_core,\
    -I$(SRCDIR)/editeng/inc \
    -I$(SRCDIR)/editeng/source/editeng \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,editeng_core))

$(eval $(call gb_CppunitTest_use_ure,editeng_core))
$(eval $(call gb_CppunitTest_use_vcl,editeng_core))

$(eval $(call gb_CppunitTest_use_components,editeng_core,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
	i18npool/source/search/i18nsearch \
    linguistic/source/lng \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
	sax/source/expatwrap/expwrap \
))

$(eval $(call gb_CppunitTest_use_configuration,editeng_core))

$(eval $(call gb_CppunitTest_use_more_fonts,editeng_core))

# vim: set noet sw=4 ts=4:
