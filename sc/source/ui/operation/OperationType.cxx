/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/OperationType.hxx>

namespace sc
{
/** Return operation type as string. */
std::string_view operationTypeString(OperationType eOperation)
{
    switch (eOperation)
    {
        case OperationType::Unknown:
            return "Unknown";
        case OperationType::ApplyAttributes:
            return "ApplyAttributes";
        case OperationType::ApplyAttributesWithChangedRange:
            return "ApplyAttributesWithChangedRange";
        case OperationType::ApplyAttributesToCell:
            return "ApplyAttributesToCell";
        case OperationType::DeleteContent:
            return "DeleteContent";
        case OperationType::DeleteCell:
            return "DeleteCell";
        case OperationType::TransliterateText:
            return "TransliterateText";
        case OperationType::SetNormalString:
            return "SetNormalString";
        case OperationType::SetValue:
            return "SetValue";
        case OperationType::SetString:
            return "SetString";
        case OperationType::SetTextEdit:
            return "SetTextEdit";
        case OperationType::SetFormula:
            return "SetFormula";
        case OperationType::SetNoteText:
            return "SetNoteText";
        case OperationType::ReplaceNoteText:
            return "ReplaceNoteText";
        case OperationType::InsertColumnsBefore:
            return "InsertColumnsBefore";
        case OperationType::InsertColumnsAfter:
            return "InsertColumnsAfter";
        case OperationType::InsertRowsBefore:
            return "InsertRowsBefore";
        case OperationType::InsertRowsAfter:
            return "InsertRowsAfter";
        case OperationType::InsertCellsDown:
            return "InsertCellsDown";
        case OperationType::InsertCellsRight:
            return "InsertCellsRight";
        case OperationType::DeleteColumns:
            return "DeleteColumns";
        case OperationType::DeleteRows:
            return "DeleteRows";
        case OperationType::DeleteCellsLeft:
            return "DeleteCellsLeft";
        case OperationType::DeleteCellsUp:
            return "DeleteCellsUp";
        case OperationType::MoveBlock:
            return "MoveBlock";
        case OperationType::ClearItems:
            return "ClearItems";
        case OperationType::ChangeIndent:
            return "ChangeIndent";
        case OperationType::AutoFormat:
            return "AutoFormat";
        case OperationType::EnterMatrix:
            return "EnterMatrix";
        case OperationType::MultipleOps:
            return "MultipleOps";
        case OperationType::FillSimple:
            return "FillSimple";
        case OperationType::FillSeries:
            return "FillSeries";
        case OperationType::FillAuto:
            return "FillAuto";
        case OperationType::MergeCells:
            return "MergeCells";
        case OperationType::InsertNameList:
            return "InsertNameList";
        case OperationType::ConvertFormulaToValue:
            return "ConvertFormulaToValue";
        case OperationType::Sort:
            return "Sort";
        case OperationType::Query:
            return "Query";
        case OperationType::SubTotals:
            return "SubTotals";
        case OperationType::PivotTableUpdate:
            return "PivotTableUpdate";
        case OperationType::PivotTableRemove:
            return "PivotTableRemove";
        case OperationType::PivotTableCreate:
            return "PivotTableCreate";
        case OperationType::SparklineInsert:
            return "SparklineInsert";
        case OperationType::SparklineDelete:
            return "SparklineDelete";
        case OperationType::SparklineChange:
            return "SparklineChange";
        case OperationType::SparklineGroup:
            return "SparklineGroup";
        case OperationType::SparklineUngroup:
            return "SparklineUngroup";
        case OperationType::SparklineGroupDelete:
            return "SparklineGroupDelete";
        case OperationType::SparklineGroupChange:
            return "SparklineGroupChange";
        case OperationType::EnterData:
            return "EnterData";
        case OperationType::InsertSheetView:
            return "InsertSheetView";
    }
    return "";
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
