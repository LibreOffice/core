# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,rpt))

$(eval $(call gb_Library_set_include,rpt,\
    $$(INCLUDE) \
    -I$(SRCDIR)/reportdesign/inc \
    -I$(SRCDIR)/reportdesign/source/inc \
    -I$(SRCDIR)/reportdesign/source/core/inc \
))

$(eval $(call gb_Library_add_defs,rpt,\
    -DREPORTDESIGN_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_external,rpt,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,rpt,reportdesign/inc/pch/precompiled_rpt))

$(eval $(call gb_Library_use_sdk_api,rpt))

$(eval $(call gb_Library_use_custom_headers,rpt,\
    officecfg/registry \
))

$(eval $(call gb_Library_use_libraries,rpt,\
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    dbu \
    editeng \
    fwk \
    i18nlangtag \
    sal \
    salhelper \
    sfx \
    svl \
    svt \
    svxcore \
    svx \
    tk \
    tl \
    utl \
    vcl \
))

$(eval $(call gb_Library_set_componentfile,rpt,reportdesign/util/rpt,services))

$(eval $(call gb_Library_add_exception_objects,rpt,\
    reportdesign/source/core/api/FixedLine \
    reportdesign/source/core/api/FixedText \
    reportdesign/source/core/api/FormatCondition \
    reportdesign/source/core/api/FormattedField \
    reportdesign/source/core/api/Function \
    reportdesign/source/core/api/Functions \
    reportdesign/source/core/api/Group \
    reportdesign/source/core/api/Groups \
    reportdesign/source/core/api/ImageControl \
    reportdesign/source/core/api/ReportComponent \
    reportdesign/source/core/api/ReportControlModel \
    reportdesign/source/core/api/ReportDefinition \
    reportdesign/source/core/api/ReportEngineJFree \
    reportdesign/source/core/api/ReportVisitor \
    reportdesign/source/core/api/Section \
    reportdesign/source/core/api/services \
    reportdesign/source/core/api/Shape \
    reportdesign/source/core/api/Tools \
    reportdesign/source/core/misc/conditionalexpression \
    reportdesign/source/core/misc/conditionupdater \
    reportdesign/source/core/misc/reportformula \
    reportdesign/source/core/resource/core_resource \
    reportdesign/source/core/sdr/formatnormalizer \
    reportdesign/source/core/sdr/PropertyForward \
    reportdesign/source/core/sdr/ReportDrawPage \
    reportdesign/source/core/sdr/ReportUndoFactory \
    reportdesign/source/core/sdr/RptModel \
    reportdesign/source/core/sdr/RptObject \
    reportdesign/source/core/sdr/RptObjectListener \
    reportdesign/source/core/sdr/RptPage \
    reportdesign/source/core/sdr/UndoActions \
    reportdesign/source/core/sdr/UndoEnv \
))

# vim: set noet sw=4 ts=4:
