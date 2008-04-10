#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.36 $
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

PRJ=..$/..

PRJNAME=xmloff
TARGET=text

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------


SLOFILES =	\
        $(SLO)$/txtdrope.obj \
        $(SLO)$/txtdropi.obj \
        $(SLO)$/txtexppr.obj \
        $(SLO)$/txtflde.obj \
        $(SLO)$/txtfldi.obj \
        $(SLO)$/txtftne.obj \
        $(SLO)$/txtimp.obj \
        $(SLO)$/txtimppr.obj \
        $(SLO)$/txtparae.obj \
        $(SLO)$/txtparai.obj \
        $(SLO)$/txtprhdl.obj \
        $(SLO)$/txtprmap.obj \
        $(SLO)$/txtsecte.obj \
        $(SLO)$/txtstyle.obj \
        $(SLO)$/txtstyli.obj \
        $(SLO)$/txtvfldi.obj \
        $(SLO)$/XMLAutoMarkFileContext.obj \
        $(SLO)$/XMLAutoTextEventExport.obj \
        $(SLO)$/XMLAutoTextEventImport.obj \
        $(SLO)$/XMLAutoTextContainerEventImport.obj \
        $(SLO)$/XMLChangedRegionImportContext.obj \
        $(SLO)$/XMLChangeElementImportContext.obj \
        $(SLO)$/XMLChangeInfoContext.obj \
        $(SLO)$/XMLFootnoteBodyImportContext.obj \
        $(SLO)$/XMLFootnoteConfigurationImportContext.obj \
        $(SLO)$/XMLFootnoteImportContext.obj \
        $(SLO)$/XMLIndexAlphabeticalSourceContext.obj \
        $(SLO)$/XMLIndexBodyContext.obj \
        $(SLO)$/XMLIndexBibliographyConfigurationContext.obj \
        $(SLO)$/XMLIndexBibliographyEntryContext.obj \
        $(SLO)$/XMLIndexBibliographySourceContext.obj \
        $(SLO)$/XMLIndexChapterInfoEntryContext.obj \
        $(SLO)$/XMLIndexIllustrationSourceContext.obj \
        $(SLO)$/XMLIndexMarkExport.obj \
        $(SLO)$/XMLIndexObjectSourceContext.obj \
        $(SLO)$/XMLIndexSimpleEntryContext.obj \
        $(SLO)$/XMLIndexSpanEntryContext.obj \
        $(SLO)$/XMLIndexSourceBaseContext.obj \
        $(SLO)$/XMLIndexTOCContext.obj \
        $(SLO)$/XMLIndexTOCSourceContext.obj \
        $(SLO)$/XMLIndexTOCStylesContext.obj \
        $(SLO)$/XMLIndexTableSourceContext.obj \
        $(SLO)$/XMLIndexTabStopEntryContext.obj \
        $(SLO)$/XMLIndexTemplateContext.obj \
        $(SLO)$/XMLIndexTitleTemplateContext.obj \
        $(SLO)$/XMLIndexUserSourceContext.obj \
        $(SLO)$/XMLLineNumberingExport.obj \
        $(SLO)$/XMLLineNumberingImportContext.obj \
        $(SLO)$/XMLLineNumberingSeparatorImportContext.obj \
        $(SLO)$/XMLPropertyBackpatcher.obj \
        $(SLO)$/XMLRedlineExport.obj \
        $(SLO)$/XMLSectionExport.obj \
        $(SLO)$/XMLSectionFootnoteConfigExport.obj \
        $(SLO)$/XMLSectionFootnoteConfigImport.obj \
        $(SLO)$/XMLSectionImportContext.obj \
        $(SLO)$/XMLSectionSourceDDEImportContext.obj \
        $(SLO)$/XMLSectionSourceImportContext.obj \
        $(SLO)$/XMLTextCharStyleNamesElementExport.obj \
        $(SLO)$/XMLTextColumnsContext.obj \
        $(SLO)$/XMLTextColumnsExport.obj \
        $(SLO)$/XMLTextFrameContext.obj \
        $(SLO)$/XMLTextFrameHyperlinkContext.obj \
        $(SLO)$/XMLTextHeaderFooterContext.obj \
        $(SLO)$/XMLTextListAutoStylePool.obj \
        $(SLO)$/XMLTextListBlockContext.obj \
        $(SLO)$/XMLTextListItemContext.obj \
        $(SLO)$/XMLTextMarkImportContext.obj \
        $(SLO)$/XMLTextMasterPageContext.obj \
        $(SLO)$/XMLTextMasterPageExport.obj \
        $(SLO)$/XMLTextMasterStylesContext.obj \
        $(SLO)$/XMLTextNumRuleInfo.obj \
        $(SLO)$/XMLTextPropertySetContext.obj \
        $(SLO)$/XMLTextShapeImportHelper.obj \
        $(SLO)$/XMLTextShapeStyleContext.obj \
        $(SLO)$/XMLTextTableContext.obj \
        $(SLO)$/XMLChangeImportContext.obj \
        $(SLO)$/XMLStringBufferImportContext.obj \
        $(SLO)$/XMLTrackedChangesImportContext.obj \
        $(SLO)$/XMLCalculationSettingsContext.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
