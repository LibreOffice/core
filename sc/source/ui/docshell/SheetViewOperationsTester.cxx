/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SheetViewOperationsTester.hxx>
#include <SheetViewManager.hxx>
#include <docsh.hxx>
#include <viewdata.hxx>
#include <sal/log.hxx>

namespace sc
{
namespace
{
/** Return operation as string. */
constexpr std::string_view getOperationName(OperationType eOperation)
{
    switch (eOperation)
    {
        case OperationType::Unknown:
            return "Unknown";
        case OperationType::DeleteContent:
            return "DeleteContent";
        case OperationType::DeleteCell:
            return "DeleteCell";
        case OperationType::TransliterateText:
            return "TransliterateText";
        case OperationType::SetNormalString:
            return "SetNormalString";
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
        case OperationType::TabOperation:
            return "TabOperation";
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
    }
    return "";
}

/** Supported operations by sheet view. */
constexpr bool isSupported(OperationType eOperationType)
{
    return eOperationType == OperationType::EnterData
           || eOperationType == OperationType::SetNormalString
           || eOperationType == OperationType::Sort;
}

/** Operations on default view that unsync all the sheet views. */
constexpr bool doesUnsyncAllSheetView(OperationType eOperationType)
{
    bool bOperationAllowed = eOperationType == OperationType::EnterData
                             || eOperationType == OperationType::SetNormalString;

    return !bOperationAllowed;
}

/** Operations that unsync the current sheet view. */
constexpr bool doesUnsyncSheetView(OperationType eOperationType)
{
    return eOperationType == OperationType::Sort;
}

} // end anonymous namespace

bool SheetViewOperationsTester::check(OperationType eOperationType) const
{
    if (!mpViewData)
        return true;

    auto& rDocument = mpViewData->GetDocument();
    SCTAB nTab = mpViewData->GetTabNumber();

    // Never allow direct changes to the data holder sheet of the sheet view.
    if (rDocument.IsSheetViewHolder(nTab))
        return false;

    sc::SheetViewID nSheetViewID = mpViewData->GetSheetViewID();

    std::shared_ptr<sc::SheetViewManager> pSheetViewManager = rDocument.GetSheetViewManager(nTab);

    if (nSheetViewID == sc::DefaultSheetViewID && doesUnsyncAllSheetView(eOperationType))
    {
        // Only unsync if there are sheet views.
        if (!pSheetViewManager->isEmpty())
        {
            pSheetViewManager->unsyncAllSheetViews();
            SAL_INFO("sc.ui", "Operation '" << getOperationName(eOperationType)
                                            << "' unsynced all sheet views for TAB " << nTab);
        }
    }
    else
    {
        bool bSupported = isSupported(eOperationType);
        SAL_INFO_IF(!bSupported, "sc.ui",
                    "Operation '" << getOperationName(eOperationType)
                                  << "' not supported on sheet view '" << nSheetViewID << "'");

        if (bSupported && doesUnsyncSheetView(eOperationType))
        {
            std::shared_ptr<sc::SheetView> pSheetView = pSheetViewManager->get(nSheetViewID);
            pSheetView->unsync();
            SAL_INFO("sc.ui", "Operation '" << getOperationName(eOperationType)
                                            << "' unsynced sheet view '" << nSheetViewID << "'");
        }
        return bSupported;
    }

    return true;
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
