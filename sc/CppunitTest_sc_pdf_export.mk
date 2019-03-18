# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_pdf_export))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_pdf_export, \
    sc/qa/extras/scpdfexport \
))

$(eval $(call gb_CppunitTest_use_externals,sc_pdf_export, \
    boost_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_pdf_export, \
    comphelper \
    cppu \
    editeng \
    sal \
    sax \
    sc \
    sfx \
    svl \
    test \
    tl \
    unotest \
    utl \
))

$(eval $(call gb_CppunitTest_set_include,sc_pdf_export,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_pdf_export,\
    udkapi \
    offapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_pdf_export))
$(eval $(call gb_CppunitTest_use_vcl,sc_pdf_export))

$(eval $(call gb_CppunitTest_use_components,sc_pdf_export,\
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    filter/source/config/cache/filterconfig1 \
    filter/source/pdf/pdffilter \
    framework/util/fwk \
    i18npool/util/i18npool \
    sc/util/sc \
    sfx2/util/sfx \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
    uui/util/uui \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_pdf_export))

# vim: set noet sw=4 ts=4:
