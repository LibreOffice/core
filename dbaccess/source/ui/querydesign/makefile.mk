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


