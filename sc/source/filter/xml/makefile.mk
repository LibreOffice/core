#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.32 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 20:02:23 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=sc
TARGET=xml

ENABLE_EXCEPTIONS=TRUE

AUTOSEG=true

PROJECTPCH4DLL=TRUE
PROJECTPCH=filt_pch
PROJECTPCHSOURCE=..\pch\filt_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = \
        xmlwrap.cxx \
        xmlimprt.cxx \
        xmlexprt.cxx \
        xmlbodyi.cxx \
        xmltabi.cxx \
        xmlrowi.cxx \
        xmlcelli.cxx \
        xmlconti.cxx \
        xmlcoli.cxx \
        xmlsubti.cxx \
        xmlnexpi.cxx \
        xmldrani.cxx \
        xmlfilti.cxx \
        xmlsorti.cxx \
        xmlstyle.cxx \
        xmlstyli.cxx \
        xmldpimp.cxx \
        xmlannoi.cxx \
        xmlsceni.cxx \
        xmlcvali.cxx \
        XMLTableMasterPageExport.cxx \
        xmllabri.cxx \
        XMLTableHeaderFooterContext.cxx \
        XMLDetectiveContext.cxx \
        XMLCellRangeSourceContext.cxx \
        XMLConsolidationContext.cxx \
        XMLConverter.cxx \
        XMLExportIterator.cxx \
        XMLColumnRowGroupExport.cxx \
        XMLStylesExportHelper.cxx \
        XMLStylesImportHelper.cxx \
        XMLExportDataPilot.cxx \
        XMLExportDatabaseRanges.cxx \
        XMLTableShapeImportHelper.cxx \
        XMLTableShapesContext.cxx \
        XMLExportDDELinks.cxx \
        XMLDDELinksContext.cxx \
        XMLCalculationSettingsContext.cxx \
        XMLTableSourceContext.cxx \
        XMLTextPContext.cxx \
        XMLTableShapeResizer.cxx \
        XMLChangeTrackingExportHelper.cxx \
        xmlfonte.cxx \
        XMLChangeTrackingImportHelper.cxx \
        XMLTrackedChangesContext.cxx \
        XMLExportSharedData.cxx \
        XMLEmptyContext.cxx

SLOFILES =  \
        $(SLO)$/xmlwrap.obj \
        $(SLO)$/xmlimprt.obj \
        $(SLO)$/xmlexprt.obj \
        $(SLO)$/xmlbodyi.obj \
        $(SLO)$/xmltabi.obj \
        $(SLO)$/xmlrowi.obj \
        $(SLO)$/xmlcelli.obj \
        $(SLO)$/xmlconti.obj \
        $(SLO)$/xmlcoli.obj \
        $(SLO)$/xmlsubti.obj \
        $(SLO)$/xmlnexpi.obj \
        $(SLO)$/xmldrani.obj \
        $(SLO)$/xmlfilti.obj \
        $(SLO)$/xmlsorti.obj \
        $(SLO)$/xmlstyle.obj \
        $(SLO)$/xmlstyli.obj \
        $(SLO)$/xmldpimp.obj \
        $(SLO)$/xmlannoi.obj \
        $(SLO)$/xmlsceni.obj \
        $(SLO)$/xmlcvali.obj \
        $(SLO)$/XMLTableMasterPageExport.obj \
        $(SLO)$/xmllabri.obj \
        $(SLO)$/XMLTableHeaderFooterContext.obj \
        $(SLO)$/XMLDetectiveContext.obj \
        $(SLO)$/XMLCellRangeSourceContext.obj \
        $(SLO)$/XMLConsolidationContext.obj \
        $(SLO)$/XMLConverter.obj \
        $(SLO)$/XMLExportIterator.obj \
        $(SLO)$/XMLColumnRowGroupExport.obj \
        $(SLO)$/XMLStylesExportHelper.obj \
        $(SLO)$/XMLStylesImportHelper.obj \
        $(SLO)$/XMLExportDataPilot.obj \
        $(SLO)$/XMLExportDatabaseRanges.obj \
        $(SLO)$/XMLTableShapeImportHelper.obj \
        $(SLO)$/XMLTableShapesContext.obj \
        $(SLO)$/XMLExportDDELinks.obj \
        $(SLO)$/XMLDDELinksContext.obj \
        $(SLO)$/XMLCalculationSettingsContext.obj \
        $(SLO)$/XMLTableSourceContext.obj \
        $(SLO)$/XMLTextPContext.obj \
        $(SLO)$/XMLTableShapeResizer.obj \
        $(SLO)$/XMLChangeTrackingExportHelper.obj \
        $(SLO)$/xmlfonte.obj \
        $(SLO)$/XMLChangeTrackingImportHelper.obj \
        $(SLO)$/XMLTrackedChangesContext.obj \
        $(SLO)$/XMLExportSharedData.obj \
        $(SLO)$/XMLEmptyContext.obj


NOOPTFILES= \
        $(SLO)$/xmlcvali.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

