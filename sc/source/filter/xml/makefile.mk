#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: sab $ $Date: 2000-11-10 17:17:59 $
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
        xmlmapch.cxx \
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
        XMLExportDataPilot.cxx \
        XMLExportDatabaseRanges.cxx

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
        $(SLO)$/xmlmapch.obj \
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
        $(SLO)$/XMLExportDataPilot.obj \
        $(SLO)$/XMLExportDatabaseRanges.obj


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

