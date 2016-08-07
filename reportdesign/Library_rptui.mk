###############################################################
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
###############################################################



$(eval $(call gb_Library_Library,rptui))
$(eval $(call gb_Library_add_precompiled_header,rptui,$(SRCDIR)/reportdesign/inc/pch/precompiled_rptui))

$(eval $(call gb_Library_set_componentfile,rptui,reportdesign/util/rptui))

$(eval $(call gb_Library_add_api,rptui,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,rptui,\
	-I$(SRCDIR)/reportdesign/inc \
	-I$(SRCDIR)/reportdesign/source/ui/inc \
	-I$(SRCDIR)/reportdesign/source/inc \
	-I$(SRCDIR)/reportdesign/inc/pch \
	$$(INCLUDE) \
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
	ootk \
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
