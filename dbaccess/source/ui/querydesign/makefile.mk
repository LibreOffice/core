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
PRJNAME=dbaccess
TARGET=querydesign

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES =\
            $(SLO)$/ConnectionLineAccess.obj	\
            $(SLO)$/TableWindowAccess.obj		\
            $(SLO)$/JAccess.obj					\
            $(SLO)$/JoinDesignView.obj			\
            $(SLO)$/JoinController.obj			\
            $(SLO)$/QueryDesignView.obj			\
            $(SLO)$/SelectionBrowseBox.obj		\
            $(SLO)$/querydlg.obj				\
            $(SLO)$/QueryTabWinUndoAct.obj		\
            $(SLO)$/QueryMoveTabWinUndoAct.obj	\
            $(SLO)$/QueryTabConnUndoAction.obj	\
            $(SLO)$/TableFieldDescription.obj	\
            $(SLO)$/JoinTableView.obj			\
            $(SLO)$/QueryViewSwitch.obj			\
            $(SLO)$/QueryTableView.obj			\
            $(SLO)$/TableWindowData.obj			\
            $(SLO)$/QTableWindow.obj			\
            $(SLO)$/TableWindow.obj				\
            $(SLO)$/JoinExchange.obj			\
            $(SLO)$/TableWindowListBox.obj		\
            $(SLO)$/TableWindowTitle.obj		\
            $(SLO)$/QueryTextView.obj			\
            $(SLO)$/QTableConnection.obj		\
            $(SLO)$/querycontroller.obj			\
            $(SLO)$/querycontainerwindow.obj    \
            $(SLO)$/queryview.obj               \
            $(SLO)$/TableConnection.obj			\
            $(SLO)$/TableConnectionData.obj

SLOFILES =\
            $(EXCEPTIONSFILES)					\
            $(SLO)$/TableFieldInfo.obj			\
            $(SLO)$/QTableConnectionData.obj	\
            $(SLO)$/QTableWindowData.obj		\
            $(SLO)$/ConnectionLine.obj			\
            $(SLO)$/ConnectionLineData.obj

SRS1NAME=$(TARGET)
SRC1FILES =  query.src		\
            querydlg.src	\

# --- Targets -------------------------------------------------------


.INCLUDE :  target.mk

$(SRS)$/$(TARGET).srs: $(SOLARINCDIR)$/svx$/globlmn.hrc


