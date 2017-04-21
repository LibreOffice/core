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

$(eval $(call gb_CppunitTest_CppunitScreenShot,reportdesign_dialogs_test))

$(eval $(call gb_CppunitTest_add_exception_objects,reportdesign_dialogs_test, \
    reportdesign/qa/unit/reportdesign-dialogs-test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,reportdesign_dialogs_test))

$(eval $(call gb_CppunitTest_set_include,desktop_dialogs_test,\
    -I$(SRCDIR)/reportdesign/source/inc \
    -I$(SRCDIR)/reportdesign/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_libraries,reportdesign_dialogs_test, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    i18nlangtag \
    i18nutil \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sfx \
    sot \
    svl \
    svt \
    test \
    tl \
    tk \
    ucbhelper \
    unotest \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_use_external,reportdesign_dialogs_test,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,reportdesign_dialogs_test))

$(eval $(call gb_CppunitTest_use_ure,reportdesign_dialogs_test))
$(eval $(call gb_CppunitTest_use_vcl_non_headless_with_windows,reportdesign_dialogs_test))

$(eval $(call gb_CppunitTest_use_rdb,reportdesign_dialogs_test,services))

$(eval $(call gb_CppunitTest_use_configuration,reportdesign_dialogs_test))

$(eval $(call gb_CppunitTest_use_uiconfigs,reportdesign_dialogs_test,\
	modules/dbreport \
))

# vim: set noet sw=4 ts=4:
