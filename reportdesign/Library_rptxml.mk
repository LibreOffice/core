# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,rptxml))

$(eval $(call gb_Library_set_include,rptxml,\
    $$(INCLUDE) \
    -I$(SRCDIR)/reportdesign/inc \
    -I$(SRCDIR)/reportdesign/source/inc \
))

$(eval $(call gb_Library_use_sdk_api,rptxml))

$(eval $(call gb_Library_use_libraries,rptxml,\
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    editeng \
    rpt \
    sal \
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
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,rptxml))

$(eval $(call gb_Library_set_componentfile,rptxml,reportdesign/util/rptxml))

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
    reportdesign/source/filter/xml/xmlservices \
    reportdesign/source/filter/xml/xmlStyleImport \
    reportdesign/source/filter/xml/xmlSubDocument \
    reportdesign/source/filter/xml/xmlTable \
    reportdesign/source/shared/xmlstrings \
))

# vim: set noet sw=4 ts=4:
