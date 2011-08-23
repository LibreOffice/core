#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

PRJ=..$/..$/..

PRJNAME=binfilter
TARGET=xmloff_text

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
CDEFS += -DCONV_STAR_FONTS

INC+= -I$(PRJ)$/inc$/bf_xmloff

# --- Files --------------------------------------------------------


SLOFILES =	\
        $(SLO)$/xmloff_txtdrope.obj \
        $(SLO)$/xmloff_txtdropi.obj \
        $(SLO)$/xmloff_txtexppr.obj \
        $(SLO)$/xmloff_txtflde.obj \
        $(SLO)$/xmloff_txtfldi.obj \
        $(SLO)$/xmloff_txtftne.obj \
        $(SLO)$/xmloff_txtimp.obj \
        $(SLO)$/xmloff_txtimppr.obj \
        $(SLO)$/xmloff_txtparae.obj \
        $(SLO)$/xmloff_txtparai.obj \
        $(SLO)$/xmloff_txtprhdl.obj \
        $(SLO)$/xmloff_txtprmap.obj \
        $(SLO)$/xmloff_txtsecte.obj \
        $(SLO)$/xmloff_txtstyle.obj \
        $(SLO)$/xmloff_txtstyli.obj \
        $(SLO)$/xmloff_txtvfldi.obj \
        $(SLO)$/xmloff_XMLAutoMarkFileContext.obj \
        $(SLO)$/xmloff_XMLChangedRegionImportContext.obj \
        $(SLO)$/xmloff_XMLChangeElementImportContext.obj \
        $(SLO)$/xmloff_XMLChangeInfoContext.obj \
        $(SLO)$/xmloff_XMLFootnoteBodyImportContext.obj \
        $(SLO)$/xmloff_XMLFootnoteConfigurationImportContext.obj \
        $(SLO)$/xmloff_XMLFootnoteImportContext.obj \
        $(SLO)$/xmloff_XMLIndexAlphabeticalSourceContext.obj \
        $(SLO)$/xmloff_XMLIndexBodyContext.obj \
        $(SLO)$/xmloff_XMLIndexBibliographyConfigurationContext.obj \
        $(SLO)$/xmloff_XMLIndexBibliographyEntryContext.obj \
        $(SLO)$/xmloff_XMLIndexBibliographySourceContext.obj \
        $(SLO)$/xmloff_XMLIndexChapterInfoEntryContext.obj \
        $(SLO)$/xmloff_XMLIndexIllustrationSourceContext.obj \
        $(SLO)$/xmloff_XMLIndexMarkExport.obj \
        $(SLO)$/xmloff_XMLIndexObjectSourceContext.obj \
        $(SLO)$/xmloff_XMLIndexSimpleEntryContext.obj \
        $(SLO)$/xmloff_XMLIndexSpanEntryContext.obj \
        $(SLO)$/xmloff_XMLIndexSourceBaseContext.obj \
        $(SLO)$/xmloff_XMLIndexTOCContext.obj \
        $(SLO)$/xmloff_XMLIndexTOCSourceContext.obj \
        $(SLO)$/xmloff_XMLIndexTOCStylesContext.obj \
        $(SLO)$/xmloff_XMLIndexTableSourceContext.obj \
        $(SLO)$/xmloff_XMLIndexTabStopEntryContext.obj \
        $(SLO)$/xmloff_XMLIndexTemplateContext.obj \
        $(SLO)$/xmloff_XMLIndexTitleTemplateContext.obj \
        $(SLO)$/xmloff_XMLIndexUserSourceContext.obj \
        $(SLO)$/xmloff_XMLLineNumberingExport.obj \
        $(SLO)$/xmloff_XMLLineNumberingImportContext.obj \
        $(SLO)$/xmloff_XMLLineNumberingSeparatorImportContext.obj \
        $(SLO)$/xmloff_XMLPropertyBackpatcher.obj \
        $(SLO)$/xmloff_XMLRedlineExport.obj \
        $(SLO)$/xmloff_XMLSectionExport.obj \
        $(SLO)$/xmloff_XMLSectionFootnoteConfigExport.obj \
        $(SLO)$/xmloff_XMLSectionFootnoteConfigImport.obj \
        $(SLO)$/xmloff_XMLSectionImportContext.obj \
        $(SLO)$/xmloff_XMLSectionSourceDDEImportContext.obj \
        $(SLO)$/xmloff_XMLSectionSourceImportContext.obj \
        $(SLO)$/xmloff_XMLTextCharStyleNamesElementExport.obj \
        $(SLO)$/xmloff_XMLTextColumnsContext.obj \
        $(SLO)$/xmloff_XMLTextColumnsExport.obj \
        $(SLO)$/xmloff_XMLTextFrameContext.obj \
        $(SLO)$/xmloff_XMLTextFrameHyperlinkContext.obj \
        $(SLO)$/xmloff_XMLTextHeaderFooterContext.obj \
        $(SLO)$/xmloff_XMLTextListAutoStylePool.obj \
        $(SLO)$/xmloff_XMLTextListBlockContext.obj \
        $(SLO)$/xmloff_XMLTextListItemContext.obj \
        $(SLO)$/xmloff_XMLTextMarkImportContext.obj \
        $(SLO)$/xmloff_XMLTextMasterPageContext.obj \
        $(SLO)$/xmloff_XMLTextMasterPageExport.obj \
        $(SLO)$/xmloff_XMLTextMasterStylesContext.obj \
        $(SLO)$/xmloff_XMLTextNumRuleInfo.obj \
        $(SLO)$/xmloff_XMLTextPropertySetContext.obj \
        $(SLO)$/xmloff_XMLTextShapeImportHelper.obj \
        $(SLO)$/xmloff_XMLTextShapeStyleContext.obj \
        $(SLO)$/xmloff_XMLTextTableContext.obj \
        $(SLO)$/xmloff_XMLChangeImportContext.obj \
        $(SLO)$/xmloff_XMLStringBufferImportContext.obj \
        $(SLO)$/xmloff_XMLTrackedChangesImportContext.obj \
        $(SLO)$/xmloff_XMLCalculationSettingsContext.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
