#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: mi $ $Date: 2000-11-08 12:43:47 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=api

TARGET=csssheet
PACKAGE=com$/sun$/star$/sheet

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AddIn.idl\
    Border.idl\
    CellAnnotation.idl\
    CellAnnotations.idl\
    CellAreaLink.idl\
    CellAreaLinks.idl\
    CellDeleteMode.idl\
    CellFlags.idl\
    CellFormatRanges.idl\
    CellFormatRangesEnumeration.idl\
    CellInsertMode.idl\
    Cells.idl\
    CellsEnumeration.idl\
    ConditionOperator.idl\
    ConsolidationDescriptor.idl\
    DatabaseImportDescriptor.idl\
    DatabaseRange.idl\
    DatabaseRanges.idl\
    DataImportMode.idl\
    DataPilotField.idl\
    DataPilotFieldOrientation.idl\
    DataPilotFields.idl\
    DataPilotSource.idl\
    DataPilotSourceDimension.idl\
    DataPilotSourceDimensions.idl\
    DataPilotSourceHierarchies.idl\
    DataPilotSourceHierarchy.idl\
    DataPilotSourceLevel.idl\
    DataPilotSourceLevels.idl\
    DataPilotSourceMember.idl\
    DataPilotSourceMembers.idl\
    DataPilotTable.idl\
    DataPilotTableDescriptor.idl\
    DataPilotTables.idl\
    DataResult.idl\
    DataResultFlags.idl\
    DDELink.idl\
    DDELinks.idl\
    FillDateMode.idl\
    FillDirection.idl\
    FillMode.idl\
    FilterConnection.idl\
    FilterOperator.idl\
    FormulaResult.idl\
    FunctionAccess.idl\
    FunctionArgument.idl\
    FunctionDescription.idl\
    FunctionDescriptions.idl\
    GeneralFunction.idl\
    GlobalSheetSettings.idl\
    GoalResult.idl\
    HeaderFooterContent.idl\
    LabelRange.idl\
    LabelRanges.idl\
    MemberResult.idl\
    MemberResultFlags.idl\
    NamedRange.idl\
    NamedRangeFlag.idl\
    NamedRanges.idl\
    PasteOperation.idl\
    RecentFunctions.idl\
    ResultEvent.idl\
    Scenarios.idl\
    SheetCell.idl\
    SheetCellCursor.idl\
    SheetCellRange.idl\
    SheetCellRanges.idl\
    SheetFilterDescriptor.idl\
    SheetLink.idl\
    SheetLinkMode.idl\
    SheetLinks.idl\
    SheetRangesQuery.idl\
    SheetSortDescriptor.idl\
    Spreadsheet.idl\
    SpreadsheetDocument.idl\
    SpreadsheetDocumentSettings.idl\
    Spreadsheets.idl\
    SpreadsheetView.idl\
    SpreadsheetViewPane.idl\
    SpreadsheetViewSettings.idl\
    SubTotalColumn.idl\
    SubTotalDescriptor.idl\
    SubTotalField.idl\
    TableAutoFormat.idl\
    TableAutoFormatField.idl\
    TableAutoFormats.idl\
    TableConditionalEntry.idl\
    TableConditionalFormat.idl\
    TableFilterField.idl\
    TableOperationMode.idl\
    TablePageBreakData.idl\
    TablePageStyle.idl\
    TableValidation.idl\
    ValidationAlertStyle.idl\
    ValidationType.idl\
    VolatileResult.idl\
    XAddIn.idl\
    XAreaLink.idl\
    XAreaLinks.idl\
    XArrayFormulaRange.idl\
    XCalculatable.idl\
    XCellAddressable.idl\
    XCellFormatRangesSupplier.idl\
    XCellRangeAddressable.idl\
    XCellRangeData.idl\
    XCellRangeMovement.idl\
    XCellRangeReferrer.idl\
    XCellRangesQuery.idl\
    XCellSeries.idl\
    XConsolidatable.idl\
    XConsolidationDescriptor.idl\
    XDatabaseRange.idl\
    XDatabaseRanges.idl\
    XDataPilotDescriptor.idl\
    XDataPilotMemberResults.idl\
    XDataPilotResults.idl\
    XDataPilotTable.idl\
    XDataPilotTables.idl\
    XDataPilotTablesSupplier.idl\
    XDDELink.idl\
    XDimensionsSupplier.idl\
    XDocumentAuditing.idl\
    XFillAcrossSheet.idl\
    XFormulaQuery.idl\
    XFunctionAccess.idl\
    XFunctionDescriptions.idl\
    XGoalSeek.idl\
    XHeaderFooterContent.idl\
    XHierarchiesSupplier.idl\
    XLabelRange.idl\
    XLabelRanges.idl\
    XLevelsSupplier.idl\
    XMembersSupplier.idl\
    XMultipleOperation.idl\
    XNamedRange.idl\
    XNamedRanges.idl\
    XPrintAreas.idl\
    XRecentFunctions.idl\
    XResultListener.idl\
    XScenario.idl\
    XScenarios.idl\
    XScenariosSupplier.idl\
    XSheetAnnotation.idl\
    XSheetAnnotationAnchor.idl\
    XSheetAnnotations.idl\
    XSheetAnnotationsSupplier.idl\
    XSheetAuditing.idl\
    XSheetCellCursor.idl\
    XSheetCellRange.idl\
    XSheetCellRanges.idl\
    XSheetCondition.idl\
    XSheetConditionalEntries.idl\
    XSheetConditionalEntry.idl\
    XSheetFilterable.idl\
    XSheetFilterableEx.idl\
    XSheetFilterDescriptor.idl\
    XSheetLinkable.idl\
    XSheetOperation.idl\
    XSheetOutline.idl\
    XSheetPageBreak.idl\
    XSheetPastable.idl\
    XSpreadsheet.idl\
    XSpreadsheetDocument.idl\
    XSpreadsheets.idl\
    XSpreadsheetView.idl\
    XSubTotalCalculatable.idl\
    XSubTotalDescriptor.idl\
    XSubTotalField.idl\
    XUsedAreaCursor.idl\
    XViewFreezable.idl\
    XViewPane.idl\
    XViewPanesSupplier.idl\
    XViewSplitable.idl\
    XVolatileResult.idl\
    _NamedRange.idl\

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
