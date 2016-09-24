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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ww8import))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ww8import, \
    sw/qa/extras/ww8import/ww8import \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ww8import, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    test \
    unotest \
    sw \
	utl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ww8import,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ww8import,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_ww8import))

$(eval $(call gb_CppunitTest_use_ure,sw_ww8import))
$(eval $(call gb_CppunitTest_use_vcl,sw_ww8import))

$(eval $(call gb_CppunitTest_use_components,sw_ww8import,\
	basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    package/util/package2 \
    package/source/xstor/xstor \
    sw/util/msword \
    sw/util/sw \
    sw/util/swd \
    sfx2/util/sfx \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    uui/util/uui \
    $(if $(filter DESKTOP,$(BUILD_TYPE)),xmlhelp/util/ucpchelp1) \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_ww8import))

# vim: set noet sw=4 ts=4:
