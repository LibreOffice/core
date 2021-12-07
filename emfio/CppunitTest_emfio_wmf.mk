# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,emfio_wmf))

$(eval $(call gb_CppunitTest_set_include,emfio_wmf,\
    $$(INCLUDE) \
    -I$(SRCDIR)/emfio/inc \
))

$(eval $(call gb_CppunitTest_use_externals,emfio_wmf,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,emfio_wmf, \
    emfio/qa/cppunit/wmf/wmfimporttest \
))

$(eval $(call gb_CppunitTest_use_libraries,emfio_wmf,\
    emfio \
    sal \
    test \
    tl \
    unotest \
    vcl \
))

$(eval $(call gb_CppunitTest_use_components,emfio_wmf,\
    configmgr/source/configmgr \
    emfio/emfio \
    i18npool/util/i18npool \
    ucb/source/core/ucb1 \
    unotools/util/utl \
))

$(eval $(call gb_CppunitTest_use_configuration,emfio_wmf))
$(eval $(call gb_CppunitTest_use_sdk_api,emfio_wmf))
$(eval $(call gb_CppunitTest_use_ure,emfio_wmf))
$(eval $(call gb_CppunitTest_use_vcl,emfio_wmf))
$(eval $(call gb_CppunitTest_use_rdb,emfio_wmf,services))

# vim: set noet sw=4 ts=4:
