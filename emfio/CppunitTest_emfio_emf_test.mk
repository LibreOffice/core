# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,emfio_emf))

$(eval $(call gb_CppunitTest_use_externals,emfio_emf,\
    libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,emfio_emf,\
    emfio/qa/cppunit/emf/EmfImportTest \
))

$(eval $(call gb_CppunitTest_use_sdk_api,emfio_emf))

$(eval $(call gb_CppunitTest_use_libraries,emfio_emf,\
    basegfx \
    drawinglayer \
    cppu \
    cppuhelper \
    comphelper \
    sal \
    svt \
    test \
    unotest \
    tl \
    vcl \
    utl \
))

$(eval $(call gb_CppunitTest_use_ure,emfio_emf))
$(eval $(call gb_CppunitTest_use_vcl,emfio_emf))

$(eval $(call gb_CppunitTest_use_components,emfio_emf,\
    configmgr/source/configmgr \
    dtrans/util/mcnttype \
    emfio/emfio \
    framework/util/fwk \
    i18npool/util/i18npool \
    package/source/xstor/xstor \
    package/util/package2 \
    toolkit/util/tk \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
))

$(eval $(call gb_CppunitTest_use_configuration,emfio_emf))

# vim: set noet sw=4 ts=4:
