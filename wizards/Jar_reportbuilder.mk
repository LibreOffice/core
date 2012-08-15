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

$(eval $(call gb_Jar_Jar,reportbuilderwizard))

$(eval $(call gb_Jar_use_jars,reportbuilderwizard,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
	$(OUTDIR)/bin/commonwizards.jar \
	$(OUTDIR)/bin/report.jar \
))

$(eval $(call gb_Jar_set_packageroot,reportbuilderwizard,com))

$(eval $(call gb_Jar_set_manifest,reportbuilderwizard,$(SRCDIR)/wizards/com/sun/star/wizards/reportbuilder/MANIFEST.MF))

$(eval $(call gb_Jar_add_sourcefiles,reportbuilderwizard,\
	wizards/com/sun/star/wizards/reportbuilder/ReportBuilderImplementation \
	wizards/com/sun/star/wizards/reportbuilder/layout/ColumnarSingleColumn \
	wizards/com/sun/star/wizards/reportbuilder/layout/ColumnarThreeColumns \
	wizards/com/sun/star/wizards/reportbuilder/layout/ColumnarTwoColumns \
	wizards/com/sun/star/wizards/reportbuilder/layout/DesignTemplate \
	wizards/com/sun/star/wizards/reportbuilder/layout/InBlocksLabelsAbove \
	wizards/com/sun/star/wizards/reportbuilder/layout/InBlocksLabelsLeft \
	wizards/com/sun/star/wizards/reportbuilder/layout/LayoutConstants \
	wizards/com/sun/star/wizards/reportbuilder/layout/ReportBuilderLayouter \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionEmptyObject \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionLabel \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionObject \
	wizards/com/sun/star/wizards/reportbuilder/layout/SectionTextField \
	wizards/com/sun/star/wizards/reportbuilder/layout/Tabular \
))

# vim: set noet sw=4 ts=4:
