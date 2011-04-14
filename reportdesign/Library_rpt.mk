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

$(eval $(call gb_Library_Library,rpt))
$(eval $(call gb_Library_add_package_headers,rpt,reportdesign_inc))
$(eval $(call gb_Library_add_precompiled_header,rpt,$(SRCDIR)/reportdesign/inc/pch/precompiled_reportdesign))

$(eval $(call gb_Library_set_componentfile,rpt,reportdesign/util/rpt))

$(eval $(call gb_Library_set_include,rpt,\
	-I$(SRCDIR)/reportdesign/inc \
	-I$(SRCDIR)/reportdesign/source/core/inc \
	-I$(SRCDIR)/reportdesign/source/inc \
	-I$(SRCDIR)/reportdesign/inc/pch \
	$$(SOLARINC) \
	-I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,rpt,\
	$$(DEFS) \
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
	tk \
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
