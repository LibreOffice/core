# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,oox_crypto))

$(eval $(call gb_CppunitTest_add_exception_objects,oox_crypto,\
    oox/qa/unit/CryptoTest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,oox_crypto))

ifeq ($(TLS),OPENSSL)
$(eval $(call gb_CppunitTest_externals,oox_crypto,\
	openssl \
	openssl_headers \
))
else
ifeq ($(TLS),NSS)
$(eval $(call gb_CppunitTest_use_externals,oox_crypto,\
       plc4 \
       nss3 \
))
endif
endif

$(eval $(call gb_CppunitTest_use_libraries,oox_crypto,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    drawinglayer \
    msfilter \
    sal \
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

$(eval $(call gb_CppunitTest_use_api,oox_crypto,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,oox_crypto))
$(eval $(call gb_CppunitTest_use_vcl,oox_crypto))
$(eval $(call gb_CppunitTest_use_configuration,oox_crypto))

$(eval $(call gb_CppunitTest_use_components,oox_crypto,\
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    filter/source/config/cache/filterconfig1 \
    filter/source/storagefilterdetect/storagefd \
    i18npool/util/i18npool \
    package/source/xstor/xstor \
    package/util/package2 \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    uui/util/uui \
    vcl/vcl.common \
    sax/source/expatwrap/expwrap \
))


# vim: set noet sw=4 ts=4:
