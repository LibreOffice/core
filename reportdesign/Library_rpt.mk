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

$(eval $(call gb_Library_use_sdk_api,rpt))

$(eval $(call gb_Library_use_libraries,rpt,\
    comphelper \
    cppu \
    cppuhelper \
    dbtools \
    dbu \
    editeng \
    fwe \
    i18nisolang1 \
    sal \
    sfx \
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

$(eval $(call gb_Library_add_standard_system_libs,rpt))

$(eval $(call gb_Library_set_componentfile,rpt,reportdesign/util/rpt))

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
    reportdesign/source/core/sdr/ModuleHelper \
    reportdesign/source/core/sdr/PropertyForward \
    reportdesign/source/core/sdr/ReportDrawPage \
    reportdesign/source/core/sdr/ReportUndoFactory \
    reportdesign/source/core/sdr/RptModel \
    reportdesign/source/core/sdr/RptObject \
    reportdesign/source/core/sdr/RptObjectListener \
    reportdesign/source/core/sdr/RptPage \
    reportdesign/source/core/sdr/UndoActions \
    reportdesign/source/core/sdr/UndoEnv \
    reportdesign/source/shared/corestrings \
))

# vim: set noet sw=4 ts=4:
