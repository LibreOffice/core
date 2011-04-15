#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,rptui))
$(eval $(call gb_Library_add_precompiled_header,rptui,$(SRCDIR)/reportdesign/inc/pch/precompiled_rptui))

$(eval $(call gb_Library_set_componentfile,rptui,reportdesign/util/rptui))

$(eval $(call gb_Library_set_include,rptui,\
	-I$(SRCDIR)/reportdesign/inc \
	-I$(SRCDIR)/reportdesign/source/ui/inc \
	-I$(SRCDIR)/reportdesign/source/inc \
	-I$(SRCDIR)/reportdesign/inc/pch \
	$$(SOLARINC) \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_linked_libs,rptui,\
	editeng \
	comphelper \
	cppuhelper \
	cppu \
	dbtools \
	i18nisolang1 \
	fwe \
	sal \
	salhelper \
	sfx \
	svl \
	svt \
	stl \
	vcl \
	for \
	tl \
	utl \
	ucbhelper \
	svxcore \
	svx \
	tk \
	sot \
	dba \
	vos3 \
	forui \
	dbui \
	rpt \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,rptui,\
	reportdesign/source/ui/dlg/dlgpage					\
	reportdesign/source/ui/dlg/Condition				\
	reportdesign/source/ui/dlg/CondFormat				\
	reportdesign/source/ui/dlg/GroupExchange			\
	reportdesign/source/ui/dlg/PageNumber				\
	reportdesign/source/ui/dlg/DateTime				\
	reportdesign/source/ui/dlg/AddField				\
	reportdesign/source/ui/dlg/Navigator				\
	reportdesign/source/ui/dlg/GroupsSorting           \
	reportdesign/source/ui/dlg/Formula \
	reportdesign/source/ui/inspection/DefaultInspection		\
	reportdesign/source/ui/inspection/ReportComponentHandler	\
	reportdesign/source/ui/inspection/GeometryHandler			\
	reportdesign/source/ui/inspection/DataProviderHandler		\
	reportdesign/source/ui/inspection/metadata \
	reportdesign/source/ui/misc/RptUndo					\
	reportdesign/source/ui/misc/ColorListener			\
	reportdesign/source/ui/misc/UITools					\
	reportdesign/source/ui/misc/rptuiservices			\
	reportdesign/source/ui/misc/toolboxcontroller		\
	reportdesign/source/ui/misc/statusbarcontroller		\
	reportdesign/source/ui/misc/FunctionHelper \
	reportdesign/source/ui/report/SectionWindow			\
	reportdesign/source/ui/report/ReportController		\
	reportdesign/source/ui/report/ReportControllerObserver \
	reportdesign/source/ui/report/FormattedFieldBeautifier \
	reportdesign/source/ui/report/FixedTextColor           \
	reportdesign/source/ui/report/ReportSection			\
	reportdesign/source/ui/report/SectionView				\
	reportdesign/source/ui/report/ViewsWindow				\
	reportdesign/source/ui/report/ScrollHelper			\
	reportdesign/source/ui/report/StartMarker				\
	reportdesign/source/ui/report/EndMarker				\
	reportdesign/source/ui/report/dlgedfunc				\
	reportdesign/source/ui/report/dlgedfac				\
	reportdesign/source/ui/report/dlgedclip				\
	reportdesign/source/ui/report/ReportWindow			\
	reportdesign/source/ui/report/DesignView				\
	reportdesign/source/ui/report/propbrw \
	reportdesign/source/shared/uistrings \
))

# vim: set noet sw=4 ts=4:
