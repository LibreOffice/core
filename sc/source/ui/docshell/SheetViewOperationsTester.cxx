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
constexpr std::string_view getOperationName(Operation eOperation)
{
    switch (eOperation)
    {
        case Operation::Unknown:
            return "Unknown";
        case Operation::DeleteContent:
            return "DeleteContent";
        case Operation::DeleteCell:
            return "DeleteCell";
        case Operation::TransliterateText:
            return "TransliterateText";
        case Operation::SetNormalString:
            return "SetNormalString";
        case Operation::SetNoteText:
            return "SetNoteText";
        case Operation::ReplaceNoteText:
            return "ReplaceNoteText";
        case Operation::InsertColumnsBefore:
            return "InsertColumnsBefore";
        case Operation::InsertColumnsAfter:
            return "InsertColumnsAfter";
        case Operation::InsertRowsBefore:
            return "InsertRowsBefore";
        case Operation::InsertRowsAfter:
            return "InsertRowsAfter";
        case Operation::InsertCellsDown:
            return "InsertCellsDown";
        case Operation::InsertCellsRight:
            return "InsertCellsRight";
        case Operation::DeleteColumns:
            return "DeleteColumns";
        case Operation::DeleteRows:
            return "DeleteRows";
        case Operation::DeleteCellsLeft:
            return "DeleteCellsLeft";
        case Operation::DeleteCellsUp:
            return "DeleteCellsUp";
        case Operation::MoveBlock:
            return "MoveBlock";
        case Operation::ClearItems:
            return "ClearItems";
        case Operation::ChangeIndent:
            return "ChangeIndent";
        case Operation::AutoFormat:
            return "AutoFormat";
        case Operation::EnterMatrix:
            return "EnterMatrix";
        case Operation::TabOperation:
            return "TabOperation";
        case Operation::FillSimple:
            return "FillSimple";
        case Operation::FillSeries:
            return "FillSeries";
        case Operation::FillAuto:
            return "FillAuto";
        case Operation::MergeCells:
            return "MergeCells";
        case Operation::InsertNameList:
            return "InsertNameList";
        case Operation::ConvertFormulaToValue:
            return "ConvertFormulaToValue";
        case Operation::Sort:
            return "Sort";
        case Operation::Query:
            return "Query";
        case Operation::SubTotals:
            return "SubTotals";
        case Operation::PivotTableUpdate:
            return "PivotTableUpdate";
        case Operation::PivotTableRemove:
            return "PivotTableRemove";
        case Operation::PivotTableCreate:
            return "PivotTableCreate";
        case Operation::SparklineInsert:
            return "SparklineInsert";
        case Operation::SparklineDelete:
            return "SparklineDelete";
        case Operation::SparklineChange:
            return "SparklineChange";
        case Operation::SparklineGroup:
            return "SparklineGroup";
        case Operation::SparklineUngroup:
            return "SparklineUngroup";
        case Operation::SparklineGroupDelete:
            return "SparklineGroupDelete";
        case Operation::SparklineGroupChange:
            return "SparklineGroupChange";
        case Operation::EnterData:
            return "EnterData";
    }
    return "";
}

/** Supported operations by sheet view. */
constexpr bool isSupported(Operation eOperation)
{
    return eOperation == Operation::EnterData || eOperation == Operation::SetNormalString
           || eOperation == Operation::Sort;
}

/** Operations on default view that unsync all the sheet views. */
constexpr bool doesUnsycAllSheetView(Operation eOperation)
{
    bool bOperationAllowed
        = eOperation == Operation::EnterData || eOperation == Operation::SetNormalString;

    return !bOperationAllowed;
}

/** Operations that unsync the current sheet view. */
constexpr bool doesUnsycSheetView(Operation eOperation) { return eOperation == Operation::Sort; }

} // end anonymous namespace

bool SheetViewOperationsTester::check(Operation eOperation) const
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

    if (nSheetViewID == sc::DefaultSheetViewID && doesUnsycAllSheetView(eOperation))
    {
        // Only unsync if there are sheet views.
        if (!pSheetViewManager->isEmpty())
        {
            pSheetViewManager->unsyncAllSheetViews();
            SAL_INFO("sc.ui", "Operation '" << getOperationName(eOperation)
                                            << "' unsynced all sheet views for TAB " << nTab);
        }
    }
    else
    {
        bool bSupported = isSupported(eOperation);
        SAL_INFO_IF(!bSupported, "sc.ui",
                    "Operation '" << getOperationName(eOperation)
                                  << "' not supported on sheet view '" << nSheetViewID << "'");

        if (bSupported && doesUnsycSheetView(eOperation))
        {
            std::shared_ptr<sc::SheetView> pSheetView = pSheetViewManager->get(nSheetViewID);
            pSheetView->unsync();
            SAL_INFO("sc.ui", "Operation '" << getOperationName(eOperation)
                                            << "' unsynced sheet view '" << nSheetViewID << "'");
        }
        return bSupported;
    }

    return true;
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
