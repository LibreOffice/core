# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,oox_tokenmap))

$(eval $(call gb_CppunitTest_add_exception_objects,oox_tokenmap,\
    oox/qa/token/tokenmap-test \
))

$(eval $(call gb_CppunitTest_use_custom_headers,oox_tokenmap,oox/generated))

$(eval $(call gb_CppunitTest_use_sdk_api,oox_tokenmap))

$(eval $(call gb_CppunitTest_use_library_objects,oox_tokenmap,oox))

ifeq ($(TLS),OPENSSL)
$(eval $(call gb_CppunitTest_use_externals,oox_tokenmap,\
	openssl \
	openssl_headers \
))
else
ifeq ($(TLS),NSS)
$(eval $(call gb_CppunitTest_use_externals,oox_tokenmap,\
       plc4 \
       nss3 \
))
endif
endif

$(eval $(call gb_CppunitTest_use_libraries,oox_tokenmap,\
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
    sax \
    sfx \
    svl \
    svt \
    svx \
    svxcore \
    sot \
    tl \
    utl \
    vcl \
    xo \
    xmlscript \
))

# vim: set noet sw=4 ts=4:
