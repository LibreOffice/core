# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,rptui))

$(eval $(call gb_Library_set_include,rptui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/reportdesign/inc \
    -I$(SRCDIR)/reportdesign/source/inc \
    -I$(SRCDIR)/reportdesign/source/ui/inc \
))

$(eval $(call gb_Library_use_external,rptui,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,rptui,reportdesign/inc/pch/precompiled_rptui))

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
    fwk \
    i18nlangtag \
    rpt \
    sal \
    salhelper \
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
))

$(eval $(call gb_Library_set_componentfile,rptui,reportdesign/util/rptui,services))

$(eval $(call gb_Library_add_exception_objects,rptui,\
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
    reportdesign/source/ui/misc/RptUndo \
    reportdesign/source/ui/misc/statusbarcontroller \
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
