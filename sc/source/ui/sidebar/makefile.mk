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
TARGET=sidebar
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
    AlignmentPropertyPanel.src          \
    CellAppearancePropertyPanel.src     \
    NumberFormatPropertyPanel.src

SLOFILES = \
    $(SLO)$/ScPanelFactory.obj                  \
    $(SLO)$/AlignmentPropertyPanel.obj          \
    $(SLO)$/CellLineStyleControl.obj            \
    $(SLO)$/CellLineStylePopup.obj              \
    $(SLO)$/CellLineStyleValueSet.obj           \
    $(SLO)$/CellBorderUpdater.obj               \
    $(SLO)$/CellAppearancePropertyPanel.obj     \
    $(SLO)$/CellBorderStyleControl.obj          \
    $(SLO)$/CellBorderStylePopup.obj            \
    $(SLO)$/NumberFormatPropertyPanel.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

