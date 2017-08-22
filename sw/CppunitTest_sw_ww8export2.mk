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

$(eval $(call gb_CppunitTest_CppunitTest,sw_ww8export2))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_ww8export2, \
    sw/qa/extras/ww8export/ww8export2 \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_ww8export2, \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    $(if $(filter WNT-TRUE,$(OS)-$(DISABLE_ATL)),,emboleobj) \
    sal \
    sfx \
    test \
    unotest \
    utl \
    sw \
    tl \
))

$(eval $(call gb_CppunitTest_use_externals,sw_ww8export2,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_ww8export2,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_ww8export2))

$(eval $(call gb_CppunitTest_use_ure,sw_ww8export2))
$(eval $(call gb_CppunitTest_use_vcl,sw_ww8export2))

$(eval $(call gb_CppunitTest_use_components,sw_ww8export2,\
    basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    embeddedobj/util/embobj \
    emfio/emfio \
    filter/source/config/cache/filterconfig1 \
    filter/source/storagefilterdetect/storagefd \
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
    unoxml/source/rdf/unordf \
    uui/util/uui \
    $(if $(filter DESKTOP,$(BUILD_TYPE)),xmlhelp/util/ucpchelp1) \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_ww8export2))

# vim: set noet sw=4 ts=4:
