#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 18:00:28 $
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
PRJINC=$(PRJ)$/source
PRJNAME=reportdesign
TARGET=rptxml

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.INCLUDE :  	settings.mk

# --- Files -------------------------------------
SLOFILES=	$(SLO)$/xmlfilter.obj				\
            $(SLO)$/xmlReport.obj				\
            $(SLO)$/xmlMasterFields.obj			\
            $(SLO)$/xmlGroup.obj				\
            $(SLO)$/xmlHelper.obj				\
            $(SLO)$/xmlSection.obj				\
            $(SLO)$/xmlComponent.obj			\
            $(SLO)$/xmlControlProperty.obj		\
            $(SLO)$/xmlFormattedField.obj		\
            $(SLO)$/xmlFixedText.obj			\
            $(SLO)$/xmlReportElement.obj		\
            $(SLO)$/xmlReportElementBase.obj	\
            $(SLO)$/xmlCondPrtExpr.obj			\
            $(SLO)$/xmlImage.obj				\
            $(SLO)$/xmlFunction.obj				\
            $(SLO)$/xmlStyleImport.obj			\
            $(SLO)$/xmlAutoStyle.obj			\
            $(SLO)$/xmlExport.obj				\
            $(SLO)$/dbloader2.obj				\
            $(SLO)$/xmlFormatCondition.obj		\
            $(SLO)$/xmlPropertyHandler.obj		\
            $(SLO)$/xmlColumn.obj				\
            $(SLO)$/xmlCell.obj					\
            $(SLO)$/xmlTable.obj				\
            $(SLO)$/xmlFixedContent.obj			\
            $(SLO)$/xmlSubDocument.obj			\
            $(SLO)$/xmlservices.obj				\
            $(SLO)$/xmlImportDocumentHandler.obj \
            $(SLO)$/xmlExportDocumentHandler.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
