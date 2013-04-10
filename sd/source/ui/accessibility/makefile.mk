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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=accessibility
ENABLE_EXCEPTIONS=TRUE
AUTOSEG=true
PRJINC=..$/slidesorter

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =      									\
    $(SLO)$/AccessibleDocumentViewBase.obj			\
    $(SLO)$/AccessibleDrawDocumentView.obj			\
    $(SLO)$/AccessibleOutlineView.obj				\
    $(SLO)$/AccessiblePresentationShape.obj			\
    $(SLO)$/AccessiblePresentationGraphicShape.obj	\
    $(SLO)$/AccessiblePresentationOLEShape.obj		\
    $(SLO)$/AccessibleViewForwarder.obj				\
    $(SLO)$/AccessibleOutlineEditSource.obj			\
    $(SLO)$/AccessiblePageShape.obj					\
    $(SLO)$/AccessibleSlideSorterView.obj			\
    $(SLO)$/AccessibleSlideSorterObject.obj			\
    $(SLO)$/SdShapeTypes.obj

#SIDEBAR_TO_BE_REMOVED
#	$(SLO)$/AccessibleScrollPanel.obj				\
#	$(SLO)$/AccessibleTreeNode.obj					\

EXCEPTIONSFILES= 

SRS2NAME = accessibility
SRC2FILES = accessibility.src

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

