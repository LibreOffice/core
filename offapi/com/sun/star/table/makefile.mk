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
    TableBorderDistances.idl\
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
