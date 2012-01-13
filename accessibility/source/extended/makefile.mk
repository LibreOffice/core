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



PRJ=..$/..

PRJNAME=accessibility
TARGET=extended

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------
.IF "$(OS)$(COM)"=="SOLARISI"
NOOPTFILES=$(SLO)$/accessibletabbarpagelist.obj
.ENDIF # "$(OS)$(COM)"=="SOLARISI"

SLOFILES=\
    $(SLO)$/AccessibleBrowseBoxCheckBoxCell.obj		\
    $(SLO)$/AccessibleBrowseBoxBase.obj		\
    $(SLO)$/AccessibleBrowseBox.obj			\
    $(SLO)$/AccessibleBrowseBoxTableCell.obj	\
    $(SLO)$/AccessibleBrowseBoxHeaderCell.obj	\
    $(SLO)$/AccessibleBrowseBoxTableBase.obj	\
    $(SLO)$/AccessibleBrowseBoxTable.obj		\
    $(SLO)$/AccessibleBrowseBoxHeaderBar.obj	\
    $(SLO)$/accessibleiconchoicectrl.obj		\
    $(SLO)$/accessibleiconchoicectrlentry.obj	\
    $(SLO)$/accessiblelistbox.obj			\
    $(SLO)$/accessiblelistboxentry.obj		\
    $(SLO)$/accessibletabbarbase.obj            \
    $(SLO)$/accessibletabbar.obj			\
    $(SLO)$/accessibletabbarpage.obj		\
    $(SLO)$/accessibletabbarpagelist.obj		\
    $(SLO)$/accessibletablistbox.obj		\
    $(SLO)$/accessibletablistboxtable.obj	\
    $(SLO)$/listboxaccessible.obj			\
    $(SLO)$/accessiblebrowseboxcell.obj     \
    $(SLO)$/accessibleeditbrowseboxcell.obj \
    $(SLO)$/textwindowaccessibility.obj \
    $(SLO)$/AccessibleGridControlBase.obj \
    $(SLO)$/AccessibleGridControl.obj \
    $(SLO)$/AccessibleGridControlTableBase.obj	\
    $(SLO)$/AccessibleGridControlHeader.obj \
    $(SLO)$/AccessibleGridControlTableCell.obj	\
    $(SLO)$/AccessibleGridControlHeaderCell.obj	\
    $(SLO)$/AccessibleGridControlTable.obj  \
    $(SLO)$/AccessibleToolPanelDeck.obj \
    $(SLO)$/AccessibleToolPanelDeckTabBar.obj \
    $(SLO)$/AccessibleToolPanelDeckTabBarItem.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

