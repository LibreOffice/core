/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
UpdatePivotTableOperation::UpdatePivotTableOperation(ScDocShell& rDocShell, ScDPObject& rDPObject,
                                                     bool bRecord, bool bApi)
    : PivotTableOperation(OperationType::PivotTableUpdate, rDocShell, bRecord, bApi)
    , mrDPObject(rDPObject)
{
}

bool UpdatePivotTableOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

    ScDocument& rDoc = mrDocShell.GetDocument();

    // Use default view's DPObject when called from a sheet view
    ScDPObject* pDPObject = findDefaultViewDPObject(mrDPObject);
    if (!pDPObject)
        pDPObject = &mrDPObject;

    // Apply the SaveData from the input DPObject (which may have layout changes)
    if (pDPObject != &mrDPObject && mrDPObject.GetSaveData())
        pDPObject->SetSaveData(*mrDPObject.GetSaveData());

    if (!sc::pivot::isEditable(mrDocShell, pDPObject->GetOutRange(), mbApi,
                               sc::EditAction::UpdatePivotTable))
        return false;

    ScDocumentUniquePtr pOldUndoDoc;
    ScDocumentUniquePtr pNewUndoDoc;

    ScDPObject aUndoDPObject(*pDPObject); // For undo or revert on failure.

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    if (mbRecord)
        sc::pivot::createUndoDoc(pOldUndoDoc, rDoc, pDPObject->GetOutRange());

    pDPObject->SetAllowMove(false);
    pDPObject->ReloadGroupTableData();
    if (!pDPObject->SyncAllDimensionMembers())
        return false;

    pDPObject->InvalidateData(); // before getting the new output area

    //  make sure the table has a name (not set by dialog)
    if (pDPObject->GetName().isEmpty())
        pDPObject->SetName(rDoc.GetDPCollection()->CreateNewName());

    ScRange aNewOut;
    if (!sc::pivot::checkNewOutputRange(*pDPObject, mrDocShell, aNewOut, mbApi,
                                        sc::EditAction::UpdatePivotTable))
    {
        *pDPObject = aUndoDPObject;
        return false;
    }

    //  test if new output area is empty except for old area
    if (!mbApi)
    {
        if (!sc::pivot::lcl_EmptyExcept(rDoc, aNewOut, pDPObject->GetOutRange()))
        {
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
                ScDocShell::GetActiveDialogParent(), VclMessageType::Question,
                VclButtonsType::YesNo, ScResId(STR_PIVOT_NOTEMPTY)));
            xQueryBox->set_default_response(RET_YES);
            if (xQueryBox->run() == RET_NO)
            {
                *pDPObject = aUndoDPObject;
                return false;
            }
        }
    }

    if (mbRecord)
        sc::pivot::createUndoDoc(pNewUndoDoc, rDoc, aNewOut);

    pDPObject->Output(aNewOut.aStart);
    mrDocShell.PostPaintGridAll(); //! only necessary parts

    if (mbRecord)
    {
        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoDataPilot>(
            mrDocShell, std::move(pOldUndoDoc), std::move(pNewUndoDoc), &aUndoDPObject, pDPObject,
            false));
    }

    syncSheetViews();

    // notify API objects
    rDoc.BroadcastUno(ScDataPilotModifiedHint(pDPObject->GetName()));
    aModificator.SetDocumentModified();
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
