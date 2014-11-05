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

$(eval $(call gb_CppunitTest_CppunitTest,sw_uiwriter))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_uiwriter, \
    sw/qa/extras/uiwriter/uiwriter \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_uiwriter, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sfx \
    svl \
    svt \
	svxcore \
    sw \
    test \
    unotest \
    vcl \
    tl \
	utl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_uiwriter,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_uiwriter,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/uibase/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_uiwriter,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_uiwriter))
$(eval $(call gb_CppunitTest_use_vcl,sw_uiwriter))

$(eval $(call gb_CppunitTest_use_components,sw_uiwriter,\
    basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    embeddedobj/util/embobj \
    filter/source/config/cache/filterconfig1 \
    filter/source/storagefilterdetect/storagefd \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    package/util/package2 \
    package/source/xstor/xstor \
    sw/util/msword \
    sw/util/sw \
    sw/util/swd \
    sax/source/expatwrap/expwrap \
    sfx2/util/sfx \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    unoxml/source/service/unoxml \
    uui/util/uui \
    $(if $(filter-out MACOSX WNT,$(OS)), \
        $(if $(ENABLE_HEADLESS),, \
            vcl/vcl.unx \
        ) \
    ) \
    $(if $(filter DESKTOP,$(BUILD_TYPE)),xmlhelp/util/ucpchelp1) \
	writerfilter/util/writerfilter \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_uiwriter))

$(eval $(call gb_CppunitTest_use_unittest_configuration,sw_uiwriter))

# vim: set noet sw=4 ts=4:
