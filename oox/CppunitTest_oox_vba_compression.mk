# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,oox_vba_compression))

$(eval $(call gb_CppunitTest_add_exception_objects,oox_vba_compression,\
    oox/qa/unit/vba_compression \
))

$(eval $(call gb_CppunitTest_use_sdk_api,oox_vba_compression))

$(eval $(call gb_CppunitTest_use_libraries,oox_vba_compression,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    expwrap \
    drawinglayer \
    msfilter \
    sal \
    i18nlangtag \
    oox \
    sax \
    sfx \
    svl \
    svt \
    svx \
    svxcore \
    sot \
    tl \
    unotest \
    utl \
    vcl \
    xo \
    xmlscript \
))

# vim: set noet sw=4 ts=4:
