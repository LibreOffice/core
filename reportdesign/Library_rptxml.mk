# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,rptxml))

$(eval $(call gb_Library_set_include,rptxml,\
    $$(INCLUDE) \
    -I$(SRCDIR)/reportdesign/inc \
    -I$(SRCDIR)/reportdesign/source/inc \
))

$(eval $(call gb_Library_use_external,rptxml,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,rptxml,reportdesign/inc/pch/precompiled_rptxml))

$(eval $(call gb_Library_use_sdk_api,rptxml))

$(eval $(call gb_Library_use_custom_headers,rptxml,\
    officecfg/registry \
))

$(eval $(call gb_Library_use_libraries,rptxml,\
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    editeng \
    rpt \
    sal \
    salhelper \
    sax \
    sfx \
    sot \
    svl \
    svxcore \
    tl \
    ucbhelper \
    utl \
    vcl \
    xo \
))

$(eval $(call gb_Library_set_componentfile,rptxml,reportdesign/util/rptxml,services))

$(eval $(call gb_Library_add_exception_objects,rptxml,\
    reportdesign/source/filter/xml/dbloader2 \
    reportdesign/source/filter/xml/xmlAutoStyle \
    reportdesign/source/filter/xml/xmlCell \
    reportdesign/source/filter/xml/xmlColumn \
    reportdesign/source/filter/xml/xmlComponent \
    reportdesign/source/filter/xml/xmlCondPrtExpr \
    reportdesign/source/filter/xml/xmlControlProperty \
    reportdesign/source/filter/xml/xmlExport \
    reportdesign/source/filter/xml/xmlExportDocumentHandler \
    reportdesign/source/filter/xml/xmlfilter \
    reportdesign/source/filter/xml/xmlFixedContent \
    reportdesign/source/filter/xml/xmlFormatCondition \
    reportdesign/source/filter/xml/xmlFormattedField \
    reportdesign/source/filter/xml/xmlFunction \
    reportdesign/source/filter/xml/xmlGroup \
    reportdesign/source/filter/xml/xmlHelper \
    reportdesign/source/filter/xml/xmlImage \
    reportdesign/source/filter/xml/xmlImportDocumentHandler \
    reportdesign/source/filter/xml/xmlMasterFields \
    reportdesign/source/filter/xml/xmlPropertyHandler \
    reportdesign/source/filter/xml/xmlReport \
    reportdesign/source/filter/xml/xmlReportElement \
    reportdesign/source/filter/xml/xmlReportElementBase \
    reportdesign/source/filter/xml/xmlSection \
    reportdesign/source/filter/xml/xmlStyleImport \
    reportdesign/source/filter/xml/xmlSubDocument \
    reportdesign/source/filter/xml/xmlTable \
))

# vim: set noet sw=4 ts=4:
