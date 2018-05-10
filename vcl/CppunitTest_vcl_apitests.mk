# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_apitests))

$(eval $(call gb_CppunitTest_set_include,vcl_apitests,\
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_apitests, \
    vcl/qa/api/XGraphicTest \
))

$(eval $(call gb_CppunitTest_use_externals,vcl_apitests,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,vcl_apitests, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    svt \
    test \
    tl \
    unotest \
    vcl \
))

$(eval $(call gb_CppunitTest_use_api,vcl_apitests,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,vcl_apitests))
$(eval $(call gb_CppunitTest_use_vcl,vcl_apitests))
$(eval $(call gb_CppunitTest_use_configuration,vcl_apitests))

$(eval $(call gb_CppunitTest_use_components,vcl_apitests,\
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
))


# vim: set noet sw=4 ts=4:
