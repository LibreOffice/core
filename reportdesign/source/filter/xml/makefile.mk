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
