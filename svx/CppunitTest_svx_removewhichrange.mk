# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,svx_removewhichrange))

$(eval $(call gb_CppunitTest_add_exception_objects,svx_removewhichrange, \
    svx/qa/unit/removewhichrange \
))

$(eval $(call gb_CppunitTest_use_externals,svx_removewhichrange, \
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,svx_removewhichrange, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    docmodel \
    drawinglayer \
    drawinglayercore \
    editeng \
    fwk \
    i18nlangtag \
    i18nutil \
    sal \
    salhelper \
    sax \
    sb \
    sfx \
    sot \
    svl \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    xo \
    $(call gb_Helper_optional,AVMEDIA,avmedia) \
    $(call gb_Helper_optional,DBCONNECTIVITY,dbtools) \
))

$(eval $(call gb_CppunitTest_use_library_objects,svx_removewhichrange, \
    svxcore \
))

$(eval $(call gb_CppunitTest_use_sdk_api,svx_removewhichrange))

ifeq ($(OS),MACOSX)
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,svx_removewhichrange, \
    Foundation \
))
endif

# vim: set noet sw=4 ts=4:
