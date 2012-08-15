# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Jar_Jar,report))

$(eval $(call gb_Jar_use_jars,report,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
))

$(eval $(call gb_Jar_set_manifest,report,$(SRCDIR)/wizards/com/sun/star/wizards/report/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,report,com))

$(eval $(call gb_Jar_add_sourcefiles,report,\
	wizards/com/sun/star/wizards/report/CallReportWizard \
	wizards/com/sun/star/wizards/report/Dataimport \
	wizards/com/sun/star/wizards/report/DBColumn \
	wizards/com/sun/star/wizards/report/GroupFieldHandler \
	wizards/com/sun/star/wizards/report/IReportBuilderLayouter \
	wizards/com/sun/star/wizards/report/IReportDefinitionReadAccess \
	wizards/com/sun/star/wizards/report/IReportDocument \
	wizards/com/sun/star/wizards/report/RecordTable \
	wizards/com/sun/star/wizards/report/ReportFinalizer \
	wizards/com/sun/star/wizards/report/ReportImplementationHelper \
	wizards/com/sun/star/wizards/report/ReportLayouter \
	wizards/com/sun/star/wizards/report/ReportTextDocument \
	wizards/com/sun/star/wizards/report/ReportTextImplementation \
	wizards/com/sun/star/wizards/report/ReportWizard \
))

$(eval $(call gb_Jar_set_componentfile,report,wizards/com/sun/star/wizards/report/report,OOO))

# vim: set noet sw=4 ts=4:
