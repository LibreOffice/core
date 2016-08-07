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



$(eval $(call gb_Library_Library,rpt))
$(eval $(call gb_Library_add_package_headers,rpt,reportdesign_inc))
$(eval $(call gb_Library_add_precompiled_header,rpt,$(SRCDIR)/reportdesign/inc/pch/precompiled_reportdesign))

$(eval $(call gb_Library_set_componentfile,rpt,reportdesign/util/rpt))

$(eval $(call gb_Library_add_api,rpt,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,rpt,\
	-I$(SRCDIR)/reportdesign/inc \
	-I$(SRCDIR)/reportdesign/source/core/inc \
	-I$(SRCDIR)/reportdesign/source/inc \
	-I$(SRCDIR)/reportdesign/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,rpt,\
	-DREPORTDESIGN_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,rpt,\
	editeng \
	sb \
	tl \
	svt \
	svl \
	cppuhelper \
	cppu \
	comphelper \
	dbtools \
	dbui \
	utl \
	vcl \
	sal \
	stl \
	i18nisolang1 \
	ucbhelper \
	sfx \
	salhelper \
	fwe \
	svx \
	svxcore \
	ootk \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,rpt,\
		reportdesign/source/core/api/FormattedField		\
		reportdesign/source/core/api/Groups				\
		reportdesign/source/core/api/Group				\
		reportdesign/source/core/api/Section				\
		reportdesign/source/core/api/services			\
		reportdesign/source/core/api/FixedText			\
		reportdesign/source/core/api/ImageControl		\
		reportdesign/source/core/api/ReportDefinition	\
		reportdesign/source/core/api/ReportComponent		\
		reportdesign/source/core/api/ReportEngineJFree	\
		reportdesign/source/core/api/Tools				\
		reportdesign/source/core/api/Function			\
		reportdesign/source/core/api/Functions			\
		reportdesign/source/core/api/ReportControlModel	\
		reportdesign/source/core/api/FormatCondition		\
		reportdesign/source/core/api/ReportVisitor		\
		reportdesign/source/core/api/Shape				\
		reportdesign/source/core/api/FixedLine \
		reportdesign/source/core/misc/conditionalexpression \
        reportdesign/source/core/misc/conditionupdater \
        reportdesign/source/core/misc/reportformula \
		reportdesign/source/core/resource/core_resource \
		reportdesign/source/core/sdr/RptModel			\
		reportdesign/source/core/sdr/RptPage				\
		reportdesign/source/core/sdr/UndoActions			\
		reportdesign/source/core/sdr/RptObject			\
		reportdesign/source/core/sdr/RptObjectListener	\
		reportdesign/source/core/sdr/ModuleHelper		\
		reportdesign/source/core/sdr/PropertyForward		\
		reportdesign/source/core/sdr/ReportDrawPage		\
		reportdesign/source/core/sdr/ReportUndoFactory   \
		reportdesign/source/core/sdr/UndoEnv \
		reportdesign/source/core/sdr/formatnormalizer \
))

$(eval $(call gb_Library_add_noexception_objects,rpt,\
	reportdesign/source/shared/corestrings	\
))

# vim: set noet sw=4 ts=4:
