#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.28 $
#
#   last change: $Author: mtg $ $Date: 2001-03-30 14:59:17 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..

PRJNAME=xmloff
TARGET=text
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

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
