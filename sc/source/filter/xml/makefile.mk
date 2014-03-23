#**************************************************************
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
#**************************************************************



PRJ=..$/..$/..

PRJNAME=sc
TARGET=xml

ENABLE_EXCEPTIONS=TRUE

AUTOSEG=true

PROJECTPCH4DLL=TRUE
PROJECTPCH=filt_pch
PROJECTPCHSOURCE=..\pch\filt_pch

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk

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

