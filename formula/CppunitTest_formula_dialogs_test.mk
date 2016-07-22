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

$(eval $(call gb_CppunitTest_CppunitTest,formula_dialogs_test))

$(eval $(call gb_CppunitTest_add_exception_objects,formula_dialogs_test, \
    formula/qa/unit/formula-dialogs-test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,formula_dialogs_test))

$(eval $(call gb_CppunitTest_use_libraries,formula_dialogs_test, \
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

$(eval $(call gb_CppunitTest_use_external,formula_dialogs_test,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,formula_dialogs_test))

$(eval $(call gb_CppunitTest_use_ure,formula_dialogs_test))
$(eval $(call gb_CppunitTest_use_vcl_non_headless_with_windows,formula_dialogs_test))

$(eval $(call gb_CppunitTest_use_rdb,formula_dialogs_test,services))

# $(eval $(call gb_CppunitTest_use_configuration,formula_dialogs_test))
$(eval $(call gb_CppunitTest_use_instdir_configuration,formula_dialogs_test))

# vim: set noet sw=4 ts=4:
