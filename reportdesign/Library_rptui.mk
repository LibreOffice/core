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

$(eval $(call gb_Library_Library,rptui))

$(eval $(call gb_Library_set_include,rptui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/reportdesign/inc \
    -I$(SRCDIR)/reportdesign/source/inc \
    -I$(SRCDIR)/reportdesign/source/ui/inc \
))

$(eval $(call gb_Library_use_sdk_api,rptui))

$(eval $(call gb_Library_use_libraries,rptui,\
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    dbu \
    editeng \
    for \
    forui \
    fwe \
    i18nisolang1 \
    rpt \
    sal \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
    tk \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,rptui))

$(eval $(call gb_Library_set_componentfile,rptui,reportdesign/util/rptui))

$(eval $(call gb_Library_add_exception_objects,rptui,\
    reportdesign/source/shared/uistrings \
    reportdesign/source/ui/dlg/AddField \
    reportdesign/source/ui/dlg/CondFormat \
    reportdesign/source/ui/dlg/Condition \
    reportdesign/source/ui/dlg/DateTime \
    reportdesign/source/ui/dlg/dlgpage \
    reportdesign/source/ui/dlg/Formula \
    reportdesign/source/ui/dlg/GroupExchange \
    reportdesign/source/ui/dlg/GroupsSorting \
    reportdesign/source/ui/dlg/Navigator \
    reportdesign/source/ui/dlg/PageNumber \
    reportdesign/source/ui/inspection/DataProviderHandler \
    reportdesign/source/ui/inspection/DefaultInspection \
    reportdesign/source/ui/inspection/GeometryHandler \
    reportdesign/source/ui/inspection/metadata \
    reportdesign/source/ui/inspection/ReportComponentHandler \
    reportdesign/source/ui/misc/ColorListener \
    reportdesign/source/ui/misc/FunctionHelper \
    reportdesign/source/ui/misc/rptuiservices \
    reportdesign/source/ui/misc/RptUndo \
    reportdesign/source/ui/misc/statusbarcontroller \
    reportdesign/source/ui/misc/toolboxcontroller \
    reportdesign/source/ui/misc/UITools \
    reportdesign/source/ui/report/DesignView \
    reportdesign/source/ui/report/dlgedclip \
    reportdesign/source/ui/report/dlgedfac \
    reportdesign/source/ui/report/dlgedfunc \
    reportdesign/source/ui/report/EndMarker \
    reportdesign/source/ui/report/FixedTextColor \
    reportdesign/source/ui/report/FormattedFieldBeautifier \
    reportdesign/source/ui/report/propbrw \
    reportdesign/source/ui/report/ReportController \
    reportdesign/source/ui/report/ReportControllerObserver \
    reportdesign/source/ui/report/ReportSection \
    reportdesign/source/ui/report/ReportWindow \
    reportdesign/source/ui/report/ScrollHelper \
    reportdesign/source/ui/report/SectionView \
    reportdesign/source/ui/report/SectionWindow \
    reportdesign/source/ui/report/StartMarker \
    reportdesign/source/ui/report/ViewsWindow \
))

# vim: set noet sw=4 ts=4:
