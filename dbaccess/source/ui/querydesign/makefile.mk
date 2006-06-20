#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: hr $ $Date: 2006-06-20 03:29:33 $
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
PRJNAME=dbaccess
TARGET=querydesign

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

EXCEPTIONSFILES =\
            $(SLO)$/ConnectionLineAccess.obj	\
            $(SLO)$/TableWindowAccess.obj		\
            $(SLO)$/JAccess.obj					\
            $(SLO)$/JoinDesignView.obj			\
            $(SLO)$/JoinController.obj			\
            $(SLO)$/QueryDesignView.obj			\
            $(SLO)$/TableFieldData.obj			\
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


