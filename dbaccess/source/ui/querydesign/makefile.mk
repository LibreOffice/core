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
            $(SLO)$/ConnectionLine.obj		\
            $(SLO)$/ConnectionLineAccess.obj	\
            $(SLO)$/JAccess.obj			\
            $(SLO)$/JoinController.obj		\
            $(SLO)$/JoinDesignView.obj		\
            $(SLO)$/JoinExchange.obj		\
            $(SLO)$/JoinTableView.obj		\
            $(SLO)$/QTableConnection.obj	\
            $(SLO)$/QTableConnectionData.obj	\
            $(SLO)$/QTableWindow.obj		\
            $(SLO)$/QTableWindowData.obj	\
            $(SLO)$/QueryDesignView.obj		\
            $(SLO)$/QueryMoveTabWinUndoAct.obj	\
            $(SLO)$/QueryTabConnUndoAction.obj	\
            $(SLO)$/QueryTabWinUndoAct.obj	\
            $(SLO)$/QueryTableView.obj		\
            $(SLO)$/QueryTextView.obj		\
            $(SLO)$/QueryViewSwitch.obj		\
            $(SLO)$/SelectionBrowseBox.obj	\
            $(SLO)$/TableConnection.obj		\
            $(SLO)$/TableConnectionData.obj	\
            $(SLO)$/TableFieldDescription.obj	\
            $(SLO)$/TableWindow.obj		\
            $(SLO)$/TableWindowAccess.obj	\
            $(SLO)$/TableWindowData.obj		\
            $(SLO)$/TableWindowListBox.obj	\
            $(SLO)$/TableWindowTitle.obj	\
            $(SLO)$/querycontainerwindow.obj    \
            $(SLO)$/querycontroller.obj		\
            $(SLO)$/querydlg.obj		\
            $(SLO)$/queryview.obj

SLOFILES =\
            $(EXCEPTIONSFILES)			\
            $(SLO)$/ConnectionLineData.obj	\
            $(SLO)$/TableFieldInfo.obj

SRS1NAME=$(TARGET)
SRC1FILES =  query.src		\
            querydlg.src	\

# --- Targets -------------------------------------------------------


.INCLUDE :  target.mk

$(SRS)$/$(TARGET).srs: $(SOLARINCDIR)$/svx$/globlmn.hrc


