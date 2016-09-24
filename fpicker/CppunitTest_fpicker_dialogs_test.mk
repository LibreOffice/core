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

$(eval $(call gb_CppunitTest_CppunitTest,fpicker_dialogs_test))

$(eval $(call gb_CppunitTest_add_exception_objects,fpicker_dialogs_test, \
    fpicker/qa/unit/fpicker-dialogs-test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,fpicker_dialogs_test))

$(eval $(call gb_CppunitTest_use_libraries,fpicker_dialogs_test, \
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
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_external,fpicker_dialogs_test,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,fpicker_dialogs_test))

$(eval $(call gb_CppunitTest_use_ure,fpicker_dialogs_test))
$(eval $(call gb_CppunitTest_use_vcl_non_headless_with_windows,fpicker_dialogs_test))

$(eval $(call gb_CppunitTest_use_rdb,fpicker_dialogs_test,services))

# $(eval $(call gb_CppunitTest_use_configuration,fpicker_dialogs_test))
$(eval $(call gb_CppunitTest_use_instdir_configuration,fpicker_dialogs_test))

# vim: set noet sw=4 ts=4:
