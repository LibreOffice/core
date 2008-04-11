#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.15 $
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

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=csstable
PACKAGE=com$/sun$/star$/table

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AccessibleCellView.idl      \
    AccessibleTableView.idl	\
    BorderLine.idl\
    Cell.idl\
    CellAddress.idl\
    CellContentType.idl\
    CellCursor.idl\
    CellHoriJustify.idl\
    CellOrientation.idl\
    CellProperties.idl\
    CellRange.idl\
    CellRangeAddress.idl\
    CellVertJustify.idl\
    ShadowFormat.idl\
    ShadowLocation.idl\
    TableBorder.idl\
    TableChart.idl\
    TableCharts.idl\
    TableChartsEnumeration.idl\
    TableColumn.idl\
    TableColumns.idl\
    TableColumnsEnumeration.idl\
    TableOrientation.idl\
    TableRow.idl\
    TableRows.idl\
    TableRowsEnumeration.idl\
    TableSortDescriptor.idl\
        TableSortDescriptor2.idl\
        TableSortField.idl\
        TableSortFieldType.idl\
    XAutoFormattable.idl\
    XCell.idl\
    XMergeableCell.idl\
    XMergeableCellRange.idl\
    XCellCursor.idl\
    XCellRange.idl\
    XColumnRowRange.idl\
    XTableChart.idl\
    XTableCharts.idl\
    XTableChartsSupplier.idl\
    XTableColumns.idl\
    XTableRows.idl\
    CellRangeListSource.idl\
    CellValueBinding.idl\
    ListPositionCellBinding.idl\
    XTable.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
