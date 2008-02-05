#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.27 $
#
#   last change: $Author: ihi $ $Date: 2008-02-05 15:53:10 $
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

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=csssheet
PACKAGE=com$/sun$/star$/sheet

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AccessibleCell.idl\
    AccessibleCsvCell.idl\
    AccessibleCsvRuler.idl\
    AccessibleCsvTable.idl\
    AccessiblePageHeaderFooterAreasView.idl\
    AccessibleSpreadsheet.idl\
    AccessibleSpreadsheetDocumentView.idl\
    AccessibleSpreadsheetPageView.idl\
    ActivationEvent.idl\
    AddIn.idl\
    Border.idl\
    CellAnnotation.idl\
    CellAnnotationShape.idl\
    CellAnnotations.idl\
    CellAnnotationsEnumeration.idl\
    CellAreaLink.idl\
    CellAreaLinks.idl\
    CellAreaLinksEnumeration.idl\
    CellDeleteMode.idl\
    CellFlags.idl\
    CellFormatRanges.idl\
    CellFormatRangesEnumeration.idl\
    CellInsertMode.idl\
    Cells.idl\
    CellsEnumeration.idl\
    ComplexReference.idl\
    ConditionOperator.idl\
    ConsolidationDescriptor.idl\
    DDELink.idl\
    DDELinks.idl\
    DDELinksEnumeration.idl\
    DataImportMode.idl\
    DataPilotDescriptor.idl\
    DataPilotField.idl\
    DataPilotFieldAutoShowInfo.idl\
    DataPilotFieldFilter.idl\
    DataPilotFieldGroup.idl\
    DataPilotFieldGroupBy.idl\
    DataPilotFieldGroupInfo.idl\
    DataPilotFieldGroupItem.idl\
    DataPilotFieldGroups.idl\
    DataPilotFieldLayoutInfo.idl\
    DataPilotFieldLayoutMode.idl\
    DataPilotFieldOrientation.idl\
    DataPilotFieldReference.idl\
    DataPilotFieldReferenceItemType.idl\
    DataPilotFieldReferenceType.idl\
    DataPilotFieldShowItemsMode.idl\
    DataPilotFieldSortInfo.idl\
    DataPilotFieldSortMode.idl\
    DataPilotFields.idl\
    DataPilotFieldsEnumeration.idl\
    DataPilotItem.idl\
    DataPilotItems.idl\
    DataPilotItemsEnumeration.idl\
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
    DataPilotTables.idl\
    DataPilotTablesEnumeration.idl\
    DataResult.idl\
    DataResultFlags.idl\
    DatabaseImportDescriptor.idl\
    DatabaseRange.idl\
    DatabaseRanges.idl\
    DatabaseRangesEnumeration.idl\
    DDELinkMode.idl\
    DocumentSettings.idl\
    FillDateMode.idl\
    FillDirection.idl\
    FillMode.idl\
    FilterConnection.idl\
    FilterOperator.idl\
    FormulaLanguage.idl\
    FormulaMapGroup.idl\
    FormulaMapGroupSpecialOffset.idl\
    FormulaOpCodeMapEntry.idl\
    FormulaParser.idl\
    FormulaResult.idl\
    FormulaToken.idl\
    FunctionAccess.idl\
    FunctionArgument.idl\
    FunctionCategory.idl\
    FunctionDescription.idl\
    FunctionDescriptionEnumeration.idl\
    FunctionDescriptions.idl\
    GeneralFunction.idl\
    GlobalSheetSettings.idl\
    GoalResult.idl\
    HeaderFooterContent.idl\
    LabelRange.idl\
    LabelRanges.idl\
    LabelRangesEnumeration.idl\
    LocalizedName.idl\
    MemberResult.idl\
    MemberResultFlags.idl\
    MoveDirection.idl\
    NamedRange.idl\
    NamedRangeFlag.idl\
    NamedRanges.idl\
    NamedRangesEnumeration.idl\
    PasteOperation.idl\
    RangeSelectionArguments.idl\
    RangeSelectionEvent.idl\
    RecentFunctions.idl\
    ReferenceFlags.idl\
    ResultEvent.idl\
    Scenario.idl\
    Scenarios.idl\
    ScenariosEnumeration.idl\
    Shape.idl\
    SheetCell.idl\
    SheetCellCursor.idl\
    SheetCellRange.idl\
    SheetCellRanges.idl\
    SheetCellRangesEnumeration.idl\
    SheetFilterDescriptor.idl\
    SheetLink.idl\
    SheetLinkMode.idl\
    SheetLinks.idl\
    SheetLinksEnumeration.idl\
    SheetRangesQuery.idl\
    SheetSortDescriptor.idl\
    SheetSortDescriptor2.idl\
    SingleReference.idl\
    Spreadsheet.idl\
    SpreadsheetDocument.idl\
    SpreadsheetDocumentSettings.idl\
    SpreadsheetDrawPage.idl\
    SpreadsheetView.idl\
    SpreadsheetViewPane.idl\
    SpreadsheetViewPanesEnumeration.idl\
    SpreadsheetViewSettings.idl\
    Spreadsheets.idl\
    SpreadsheetsEnumeration.idl\
    StatusBarFunction.idl\
    SubTotalColumn.idl\
    SubTotalDescriptor.idl\
    SubTotalField.idl\
    SubTotalFieldsEnumeration.idl\
    TableAutoFormat.idl\
    TableAutoFormatEnumeration.idl\
    TableAutoFormatField.idl\
    TableAutoFormats.idl\
    TableAutoFormatsEnumeration.idl\
    TableCellStyle.idl\
    TableCellStyle.idl\
    TableConditionalEntry.idl\
    TableConditionalEntryEnumeration.idl\
    TableConditionalFormat.idl\
    TableFilterField.idl\
    TableOperationMode.idl\
    TablePageBreakData.idl\
    TablePageStyle.idl\
    TableValidation.idl\
    TableValidationVisibility.idl\
    UniqueCellFormatRanges.idl\
    UniqueCellFormatRangesEnumeration.idl\
    ValidationAlertStyle.idl\
    ValidationType.idl\
    VolatileResult.idl\
    XActivationBroadcaster.idl\
    XActivationEventListener.idl\
    XAddIn.idl\
    XAreaLink.idl\
    XAreaLinks.idl\
    XArrayFormulaRange.idl\
    XArrayFormulaTokens.idl\
    XCalculatable.idl\
    XCellAddressable.idl\
    XCellFormatRangesSupplier.idl\
    XCellRangeAddressable.idl\
    XCellRangeData.idl\
    XCellRangeFormula.idl\
    XCellRangeMovement.idl\
    XCellRangeReferrer.idl\
    XCellRangesAccess.idl\
    XCellRangesQuery.idl\
    XCellSeries.idl\
    XCompatibilityNames.idl\
    XConsolidatable.idl\
    XConsolidationDescriptor.idl\
    XDDELink.idl\
    XDDELinkResults.idl\
    XDDELinks.idl\
    XDataPilotDescriptor.idl\
    XDataPilotField.idl\
    XDataPilotFieldGrouping.idl\
    XDataPilotMemberResults.idl\
    XDataPilotResults.idl\
    XDataPilotTable.idl\
    XDataPilotTables.idl\
    XDataPilotTablesSupplier.idl\
    XDatabaseRange.idl\
    XDatabaseRanges.idl\
    XDimensionsSupplier.idl\
    XDocumentAuditing.idl\
    XEnhancedMouseClickBroadcaster.idl\
    XExternalSheetName.idl\
    XFillAcrossSheet.idl\
    XFormulaOpCodeMapper.idl\
    XFormulaParser.idl\
    XFormulaQuery.idl\
    XFormulaTokens.idl\
    XFunctionAccess.idl\
    XFunctionDescriptions.idl\
    XGoalSeek.idl\
    XHeaderFooterContent.idl\
    XHierarchiesSupplier.idl\
    XLabelRange.idl\
    XLabelRanges.idl\
    XLevelsSupplier.idl\
    XMembersSupplier.idl\
    XMultiFormulaTokens.idl\
    XMultipleOperation.idl\
    XNamedRange.idl\
    XNamedRanges.idl\
    XPrintAreas.idl\
    XRangeSelection.idl\
    XRangeSelectionChangeListener.idl\
    XRangeSelectionListener.idl\
    XRecentFunctions.idl\
    XResultListener.idl\
    XScenario.idl\
    XScenarioEnhanced.idl\
    XScenarios.idl\
    XScenariosSupplier.idl\
    XSheetAnnotation.idl\
    XSheetAnnotationAnchor.idl\
    XSheetAnnotationShapeSupplier.idl\
    XSheetAnnotations.idl\
    XSheetAnnotationsSupplier.idl\
    XSheetAuditing.idl\
    XSheetCellCursor.idl\
    XSheetCellRange.idl\
    XSheetCellRangeContainer.idl\
    XSheetCellRanges.idl\
    XSheetCondition.idl\
    XSheetConditionalEntries.idl\
    XSheetConditionalEntry.idl\
    XSheetFilterDescriptor.idl\
    XSheetFilterable.idl\
    XSheetFilterableEx.idl\
    XSheetLinkable.idl\
    XSheetOperation.idl\
    XSheetOutline.idl\
    XSheetPageBreak.idl\
    XSheetPastable.idl\
    XSpreadsheet.idl\
    XSpreadsheetDocument.idl\
    XSpreadsheetView.idl\
    XSpreadsheets.idl\
    XSubTotalCalculatable.idl\
    XSubTotalDescriptor.idl\
    XSubTotalField.idl\
    XUniqueCellFormatRangesSupplier.idl\
    XUsedAreaCursor.idl\
    XViewFreezable.idl\
    XViewPane.idl\
    XViewPanesSupplier.idl\
    XViewSplitable.idl\
    XVolatileResult.idl\
    _NamedRange.idl\
    SolverConstraintOperator.idl\
    SolverConstraint.idl\
    XSolver.idl\
    XSolverDescription.idl\
    Solver.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
