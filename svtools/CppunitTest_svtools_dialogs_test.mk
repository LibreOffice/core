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

$(eval $(call gb_CppunitTest_CppunitTest,svtools_dialogs_test))

$(eval $(call gb_CppunitTest_add_exception_objects,svtools_dialogs_test, \
    svtools/qa/unit/svtools-dialogs-test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,svtools_dialogs_test))

$(eval $(call gb_CppunitTest_set_include,desktop_dialogs_test,\
    -I$(SRCDIR)/svtools/source/inc \
    -I$(SRCDIR)/svtools/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_libraries,svtools_dialogs_test, \
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

$(eval $(call gb_CppunitTest_use_external,svtools_dialogs_test,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,svtools_dialogs_test))

$(eval $(call gb_CppunitTest_use_ure,svtools_dialogs_test))
$(eval $(call gb_CppunitTest_use_vcl_non_headless_with_windows,svtools_dialogs_test))

$(eval $(call gb_CppunitTest_use_rdb,svtools_dialogs_test,services))

# $(eval $(call gb_CppunitTest_use_configuration,svtools_dialogs_test))
$(eval $(call gb_CppunitTest_use_instdir_configuration,svtools_dialogs_test))

# vim: set noet sw=4 ts=4:
