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

PRJ=..$/..$/..$/..$/..

PRJNAME=api

TARGET=cssawtgrid
PACKAGE=com$/sun$/star$/awt$/grid

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------
IDLFILES=\
    GridColumnEvent.idl\
    GridDataEvent.idl\
    SelectionEventType.idl\
    GridSelectionEvent.idl\
    XGridSelectionListener.idl\
    XGridSelection.idl\
    XGridColumn.idl\
     XGridColumnListener.idl\
    XGridDataListener.idl\
    DefaultGridDataModel.idl\
    XGridDataModel.idl\
    XMutableGridDataModel.idl\
    XGridColumnModel.idl\
    XGridControl.idl\
    UnoControlGrid.idl\
    UnoControlGridModel.idl\
    GridInvalidDataException.idl\
    GridInvalidModelException.idl\
    XSortableGridData.idl\
    SortableGridDataModel.idl\
    XSortableMutableGridDataModel.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
