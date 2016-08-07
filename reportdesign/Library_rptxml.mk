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



$(eval $(call gb_Library_Library,rptxml))
$(eval $(call gb_Library_add_precompiled_header,rptxml,$(SRCDIR)/reportdesign/inc/pch/precompiled_rptxml))

$(eval $(call gb_Library_set_componentfile,rptxml,reportdesign/util/rptxml))

$(eval $(call gb_Library_add_api,rptxml,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,rptxml,\
	-I$(SRCDIR)/reportdesign/inc \
	-I$(SRCDIR)/reportdesign/source/filter/xml \
	-I$(SRCDIR)/reportdesign/source/inc \
	-I$(SRCDIR)/reportdesign/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_linked_libs,rptxml,\
	tl \
	svl \
	cppuhelper \
	cppu \
	comphelper \
	utl \
	vcl \
	sal \
	sfx \
	dbtools \
	xo \
	sot \
	ootk \
	dba \
	stl \
	editeng \
	rpt \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,rptxml,\
	reportdesign/source/filter/xml/xmlfilter				\
	reportdesign/source/filter/xml/xmlReport				\
	reportdesign/source/filter/xml/xmlMasterFields			\
	reportdesign/source/filter/xml/xmlGroup				\
	reportdesign/source/filter/xml/xmlHelper				\
	reportdesign/source/filter/xml/xmlSection				\
	reportdesign/source/filter/xml/xmlComponent			\
	reportdesign/source/filter/xml/xmlControlProperty		\
	reportdesign/source/filter/xml/xmlFormattedField		\
	reportdesign/source/filter/xml/xmlReportElement		\
	reportdesign/source/filter/xml/xmlReportElementBase	\
	reportdesign/source/filter/xml/xmlCondPrtExpr			\
	reportdesign/source/filter/xml/xmlImage				\
	reportdesign/source/filter/xml/xmlFunction				\
	reportdesign/source/filter/xml/xmlStyleImport			\
	reportdesign/source/filter/xml/xmlAutoStyle			\
	reportdesign/source/filter/xml/xmlExport				\
	reportdesign/source/filter/xml/dbloader2				\
	reportdesign/source/filter/xml/xmlFormatCondition		\
	reportdesign/source/filter/xml/xmlPropertyHandler		\
	reportdesign/source/filter/xml/xmlColumn				\
	reportdesign/source/filter/xml/xmlCell					\
	reportdesign/source/filter/xml/xmlTable				\
	reportdesign/source/filter/xml/xmlFixedContent			\
	reportdesign/source/filter/xml/xmlSubDocument			\
	reportdesign/source/filter/xml/xmlservices				\
	reportdesign/source/filter/xml/xmlImportDocumentHandler \
	reportdesign/source/filter/xml/xmlExportDocumentHandler \
))

$(eval $(call gb_Library_add_noexception_objects,rptxml, \
	reportdesign/source/shared/xmlstrings \
))

# vim: set noet sw=4 ts=4:
