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

$(eval $(call gb_CppunitTest_CppunitScreenShot,dbaccess_dialogs_test))

$(eval $(call gb_CppunitTest_add_exception_objects,dbaccess_dialogs_test, \
    dbaccess/qa/unit/dbaccess-dialogs-test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,dbaccess_dialogs_test))

$(eval $(call gb_CppunitTest_set_include,dbaccess_dialogs_test,\
    -I$(SRCDIR)/dbaccess/source/inc \
    -I$(SRCDIR)/dbaccess/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_libraries,dbaccess_dialogs_test, \
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

$(eval $(call gb_CppunitTest_use_external,dbaccess_dialogs_test,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,dbaccess_dialogs_test))

$(eval $(call gb_CppunitTest_use_ure,dbaccess_dialogs_test))
$(eval $(call gb_CppunitTest_use_vcl_non_headless_with_windows,dbaccess_dialogs_test))

$(eval $(call gb_CppunitTest_use_rdb,dbaccess_dialogs_test,services))

$(eval $(call gb_CppunitTest_use_configuration,dbaccess_dialogs_test))

$(eval $(call gb_CppunitTest_use_uiconfigs,dbaccess_dialogs_test,\
	dbaccess \
))

# vim: set noet sw=4 ts=4:
