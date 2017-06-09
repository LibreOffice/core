# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,emfio))

$(eval $(call gb_CppunitTest_set_componentfile,emfio,emfio/emfio))

$(eval $(call gb_CppunitTest_set_include,emfio,\
    $$(INCLUDE) \
    -I$(SRCDIR)/emfio/inc \
))

$(eval $(call gb_CppunitTest_use_externals,emfio,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_sdk_api,emfio))

$(eval $(call gb_CppunitTest_use_library_objects,emfio,\
    emfio \
))

$(eval $(call gb_CppunitTest_use_libraries,emfio,\
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
))

$(eval $(call gb_CppunitTest_add_exception_objects,emfio,\
    emfio/qa/cppunit/EmfImportTest \
))

$(eval $(call gb_CppunitTest_use_ure,emfio))
$(eval $(call gb_CppunitTest_use_vcl,emfio))

$(eval $(call gb_CppunitTest_use_components,emfio,\
    configmgr/source/configmgr \
    dtrans/util/mcnttype \
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

$(eval $(call gb_CppunitTest_use_configuration,emfio))

# vim: set noet sw=4 ts=4:
