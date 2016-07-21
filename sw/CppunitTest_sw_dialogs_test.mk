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

$(eval $(call gb_CppunitTest_CppunitTest,sw_dialogs_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_dialogs_test, \
    sw/qa/unit/sw-dialogs-test \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_dialogs_test))

$(eval $(call gb_CppunitTest_set_include,sw_dialogs_test,\
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_dialogs_test, \
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
    sw \
    swui \
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
    vcl \
    xo \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_external,sw_dialogs_test,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_dialogs_test))

$(eval $(call gb_CppunitTest_use_ure,sw_dialogs_test))
$(eval $(call gb_CppunitTest_use_vcl_non_headless_with_windows,sw_dialogs_test))

$(eval $(call gb_CppunitTest_use_rdb,sw_dialogs_test,services))

# $(eval $(call gb_CppunitTest_use_configuration,sw_dialogs_test))
$(eval $(call gb_CppunitTest_use_instdir_configuration,sw_dialogs_test))

# vim: set noet sw=4 ts=4:
