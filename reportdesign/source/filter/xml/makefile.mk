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
