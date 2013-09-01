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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ww8export))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ww8export, \
    sw/qa/extras/ww8export/ww8export \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ww8export, \
    comphelper \
    cppu \
    cppuhelper \
    $(if $(filter WNT-TRUE,$(OS)-$(DISABLE_ATL)),,emboleobj) \
    sal \
    test \
    unotest \
    utl \
    sw \
    tl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ww8export,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ww8export,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_ww8export,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_ww8export))

$(eval $(call gb_CppunitTest_use_components,sw_ww8export,\
    basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    embeddedobj/util/embobj \
    fileaccess/source/fileacc \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sw/util/msword \
    sw/util/sw \
    sw/util/swd \
    sfx2/util/sfx \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    unoxml/source/service/unoxml \
    $(if $(filter DESKTOP,$(BUILD_TYPE)),xmlhelp/util/ucpchelp1) \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_ww8export))

$(eval $(call gb_CppunitTest_use_filter_configuration,sw_ww8export))

$(eval $(call gb_CppunitTest_use_unittest_configuration,sw_ww8export))

# vim: set noet sw=4 ts=4:
