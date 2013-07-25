# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sd_uimpress))

$(eval $(call gb_CppunitTest_set_include,sd_uimpress,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
))

$(eval $(call gb_CppunitTest_use_api,sd_uimpress,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_library_objects,sd_uimpress,sd))

$(eval $(call gb_CppunitTest_use_libraries,sd_uimpress,\
    avmedia \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    i18nlangtag \
    msfilter \
    sal \
    sax \
    salhelper \
    sb \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
	test \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_system_win32_libs,sd_uimpress,\
	uuid \
	ws2_32 \
))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_CppunitTest_add_libs,sd_uimpress,\
	-lobjc \
))

$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,sd_uimpress,\
	Foundation \
	IOBluetooth \
))
endif

$(eval $(call gb_CppunitTest_use_externals,sd_uimpress,\
	boost_headers \
    gtk \
    dbus \
    avahi \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_uimpress,\
    sd/qa/unit/uimpress \
))

$(eval $(call gb_CppunitTest_use_ure,sd_uimpress))

$(eval $(call gb_CppunitTest_use_components,sd_uimpress,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    ucb/source/core/ucb1 \
))

$(eval $(call gb_CppunitTest_use_configuration,sd_uimpress))

# vim: set noet sw=4 ts=4:
