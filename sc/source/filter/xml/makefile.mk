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

PRJ=..$/..$/..

PRJNAME=sc
TARGET=xml

ENABLE_EXCEPTIONS=TRUE

AUTOSEG=true

PROJECTPCH4DLL=TRUE
PROJECTPCH=filt_pch
PROJECTPCHSOURCE=..\pch\filt_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/sheetdata.obj \
        $(SLO)$/xmlwrap.obj \
        $(SLO)$/xmlimprt.obj \
        $(SLO)$/xmlexprt.obj \
        $(SLO)$/xmlbodyi.obj \
        $(SLO)$/xmltabi.obj \
        $(SLO)$/xmlexternaltabi.obj \
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
        $(SLO)$/XMLEmptyContext.obj \
        $(SLO)$/XMLCodeNameProvider.obj


NOOPTFILES= \
        $(SLO)$/xmlcvali.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

