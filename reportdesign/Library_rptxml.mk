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

$(eval $(call gb_Library_Library,rptxml))
$(eval $(call gb_Library_add_package_headers,rptxml,reportdesign_inc))
$(eval $(call gb_Library_add_precompiled_header,rptxml,$(SRCDIR)/reportdesign/inc/pch/precompiled_rptxml))

$(eval $(call gb_Library_set_componentfile,rptxml,reportdesign/source/filter/xml/rptxml))

$(eval $(call gb_Library_set_include,rptxml,\
	-I$(SRCDIR)/reportdesign/inc \
	-I$(SRCDIR)/reportdesign/source/filter/xml \
	-I$(SRCDIR)/reportdesign/source/inc \
	-I$(SRCDIR)/reportdesign/inc/pch \
	$$(SOLARINC) \
	-I$(OUTDIR)/inc/offuh \
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
	tk \
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
