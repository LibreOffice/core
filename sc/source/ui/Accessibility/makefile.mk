#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 15:19:00 $
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
TARGET=accessibility

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  										\
        $(SLO)$/AccessibleContextBase.obj			\
        $(SLO)$/AccessibleTableBase.obj				\
        $(SLO)$/AccessibleDocument.obj				\
        $(SLO)$/AccessibleSpreadsheet.obj			\
        $(SLO)$/AccessibleCell.obj					\
        $(SLO)$/AccessibilityHints.obj				\
        $(SLO)$/AccessibleDocumentBase.obj			\
        $(SLO)$/AccessibleCellBase.obj				\
        $(SLO)$/AccessibleDocumentPagePreview.obj	\
        $(SLO)$/AccessiblePreviewTable.obj			\
        $(SLO)$/AccessiblePreviewCell.obj			\
        $(SLO)$/AccessiblePreviewHeaderCell.obj		\
        $(SLO)$/AccessiblePageHeader.obj			\
        $(SLO)$/AccessibleText.obj					\
        $(SLO)$/AccessiblePageHeaderArea.obj		\
        $(SLO)$/DrawModelBroadcaster.obj			\
        $(SLO)$/AccessibleEditObject.obj			\
        $(SLO)$/AccessibleDataPilotControl.obj		\
        $(SLO)$/AccessibleCsvControl.obj

EXCEPTIONSFILES=									\
        $(SLO)$/AccessibleContextBase.obj			\
        $(SLO)$/AccessibleTableBase.obj				\
        $(SLO)$/AccessibleDocument.obj				\
        $(SLO)$/AccessibleSpreadsheet.obj			\
        $(SLO)$/AccessibleCell.obj					\
        $(SLO)$/AccessibleDocumentBase.obj			\
        $(SLO)$/AccessibleCellBase.obj				\
        $(SLO)$/AccessibleDocumentPagePreview.obj	\
        $(SLO)$/AccessiblePreviewTable.obj			\
        $(SLO)$/AccessiblePreviewCell.obj			\
        $(SLO)$/AccessiblePreviewHeaderCell.obj		\
        $(SLO)$/AccessiblePageHeader.obj			\
        $(SLO)$/AccessiblePageHeaderArea.obj		\
        $(SLO)$/DrawModelBroadcaster.obj			\
        $(SLO)$/AccessibleEditObject.obj			\
        $(SLO)$/AccessibleDataPilotControl.obj		\
        $(SLO)$/AccessibleCsvControl.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk


