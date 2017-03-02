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

$(eval $(call gb_CppunitTest_CppunitScreenShot,cui_dialogs_test4))

$(eval $(call gb_CppunitTest_add_exception_objects,cui_dialogs_test4, \
    cui/qa/unit/cui-dialogs-test_4 \
))

$(eval $(call gb_CppunitTest_use_sdk_api,cui_dialogs_test4))

$(eval $(call gb_CppunitTest_set_include,cui_dialogs_test4,\
    -I$(SRCDIR)/cui/source/inc \
    -I$(SRCDIR)/cui/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_libraries,cui_dialogs_test4, \
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
    cui \
    test \
    tl \
    tk \
    ucbhelper \
    unotest \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_CppunitTest_use_external,cui_dialogs_test4,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,cui_dialogs_test4))

$(eval $(call gb_CppunitTest_use_ure,cui_dialogs_test4))
$(eval $(call gb_CppunitTest_use_vcl_non_headless_with_windows,cui_dialogs_test4))

$(eval $(call gb_CppunitTest_use_rdb,cui_dialogs_test4,services))

# $(eval $(call gb_CppunitTest_use_configuration,cui_dialogs_test4))
$(eval $(call gb_CppunitTest_use_instdir_configuration,cui_dialogs_test4))

# vim: set noet sw=4 ts=4:
