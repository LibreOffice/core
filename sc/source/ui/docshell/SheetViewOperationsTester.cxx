/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <SheetViewOperationsTester.hxx>
#include <operation/Operation.hxx>
#include <operation/OperationType.hxx>
#include <SheetViewManager.hxx>
#include <docsh.hxx>
#include <viewdata.hxx>
#include <sal/log.hxx>
#include <dbdata.hxx>
#include <queryparam.hxx>
#include <sortparam.hxx>

namespace sc
{
namespace
{
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
            SAL_INFO("sc.ui", "Operation '" << operationTypeString(eOperationType)
                                            << "' unsynced all sheet views for TAB " << nTab);
        }
    }
    else
    {
        bool bSupported = isSupported(eOperationType);
        SAL_INFO_IF(!bSupported, "sc.ui",
                    "Operation '" << operationTypeString(eOperationType)
                                  << "' not supported on sheet view '" << nSheetViewID << "'");

        if (bSupported && doesUnsyncSheetView(eOperationType))
        {
            std::shared_ptr<sc::SheetView> pSheetView = pSheetViewManager->get(nSheetViewID);
            pSheetView->unsync();
            SAL_INFO("sc.ui", "Operation '" << operationTypeString(eOperationType)
                                            << "' unsynced sheet view '" << nSheetViewID << "'");
        }
        return bSupported;
    }

    return true;
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
