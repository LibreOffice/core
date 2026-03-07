/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/UpdatePivotTableOperation.hxx>
#include <operation/PivotTableOperationCommon.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <undodat.hxx>
#include <scresid.hxx>
#include <hints.hxx>

namespace sc
{
UpdatePivotTableOperation::UpdatePivotTableOperation(ScDocShell& rDocShell, ScDPObject& rDPObj,
                                                     bool bRecord, bool bApi)
    : Operation(OperationType::PivotTableUpdate, bRecord, bApi)
    , mrDocShell(rDocShell)
    , mrDPObj(rDPObj)
{
}

bool UpdatePivotTableOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

    if (!checkSheetViewProtection())
        return false;

    if (!sc::pivot::isEditable(mrDocShell, mrDPObj.GetOutRange(), mbApi,
                               sc::EditAction::UpdatePivotTable))
        return false;

    ScDocumentUniquePtr pOldUndoDoc;
    ScDocumentUniquePtr pNewUndoDoc;

    ScDPObject aUndoDPObj(mrDPObj); // For undo or revert on failure.

    ScDocument& rDoc = mrDocShell.GetDocument();
    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    if (mbRecord)
        sc::pivot::createUndoDoc(pOldUndoDoc, rDoc, mrDPObj.GetOutRange());

    mrDPObj.SetAllowMove(false);
    mrDPObj.ReloadGroupTableData();
    if (!mrDPObj.SyncAllDimensionMembers())
        return false;

    mrDPObj.InvalidateData(); // before getting the new output area

    //  make sure the table has a name (not set by dialog)
    if (mrDPObj.GetName().isEmpty())
        mrDPObj.SetName(rDoc.GetDPCollection()->CreateNewName());

    ScRange aNewOut;
    if (!sc::pivot::checkNewOutputRange(mrDPObj, mrDocShell, aNewOut, mbApi,
                                        sc::EditAction::UpdatePivotTable))
    {
        mrDPObj = aUndoDPObj;
        return false;
    }

    //  test if new output area is empty except for old area
    if (!mbApi)
    {
        if (!sc::pivot::lcl_EmptyExcept(rDoc, aNewOut, mrDPObj.GetOutRange()))
        {
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
                ScDocShell::GetActiveDialogParent(), VclMessageType::Question,
                VclButtonsType::YesNo, ScResId(STR_PIVOT_NOTEMPTY)));
            xQueryBox->set_default_response(RET_YES);
            if (xQueryBox->run() == RET_NO)
            {
                mrDPObj = aUndoDPObj;
                return false;
            }
        }
    }

    if (mbRecord)
        sc::pivot::createUndoDoc(pNewUndoDoc, rDoc, aNewOut);

    mrDPObj.Output(aNewOut.aStart);
    mrDocShell.PostPaintGridAll(); //! only necessary parts

    if (mbRecord)
    {
        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoDataPilot>(
            &mrDocShell, std::move(pOldUndoDoc), std::move(pNewUndoDoc), &aUndoDPObj, &mrDPObj,
            false));
    }

    // notify API objects
    rDoc.BroadcastUno(ScDataPilotModifiedHint(mrDPObj.GetName()));
    aModificator.SetDocumentModified();
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
