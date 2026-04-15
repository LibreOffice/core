/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/ReplacePivotTableOperation.hxx>
#include <operation/PivotTableOperationCommon.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <osl/diagnose.h>

#include <dpsave.hxx>
#include <undodat.hxx>
#include <scresid.hxx>
#include <hints.hxx>

namespace sc
{
ReplacePivotTableOperation::ReplacePivotTableOperation(ScDocShell& rDocShell, ScDPObject* pDPObject,
                                                       ScDPObject const* pNewDPObject, bool bRecord,
                                                       bool bApi, bool bAllowMove)
    : PivotTableOperation(OperationType::PivotTableReplace, rDocShell, bRecord, bApi)
    , mpDPObject(pDPObject)
    , mpNewDPObject(pNewDPObject)
    , mbAllowMove(bAllowMove)
{
}

bool ReplacePivotTableOperation::runImplementation()
{
    OSL_ASSERT(mpDPObject && mpNewDPObject && mpDPObject != mpNewDPObject);

    ScDocShellModificator aModificator(mrDocShell);
    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

    // Resolve to the default view's DPObject when called from a sheet view
    ScDPObject* pDPObject = findDefaultViewDPObject(*mpDPObject);
    if (!pDPObject)
        pDPObject = mpDPObject;

    ScRangeList aRanges;
    aRanges.push_back(pDPObject->GetOutRange());
    aRanges.push_back(ScRange(
        convertAddress(mpNewDPObject->GetOutRange()
                           .aStart))); // at least one cell in the output position must be editable.

    if (!sc::pivot::isEditable(mrDocShell, aRanges, mbApi))
        return false;

    ScDocumentUniquePtr pOldUndoDoc;
    ScDocumentUniquePtr pNewUndoDoc;

    ScDPObject aUndoDPObject(*pDPObject); // for undo or revert on failure

    ScDocument& rDoc = mrDocShell.GetDocument();
    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    if (mbRecord)
        sc::pivot::createUndoDoc(pOldUndoDoc, rDoc, pDPObject->GetOutRange());

    mpNewDPObject->WriteSourceDataTo(*pDPObject); // copy source data

    ScDPSaveData* pData = mpNewDPObject->GetSaveData();
    OSL_ENSURE(pData, "no SaveData from living DPObject");
    if (pData)
        pDPObject->SetSaveData(*pData); // copy SaveData

    pDPObject->SetAllowMove(mbAllowMove);
    pDPObject->ReloadGroupTableData();
    pDPObject->SyncAllDimensionMembers();
    pDPObject->InvalidateData(); // before getting the new output area

    //  make sure the table has a name (not set by dialog)
    if (pDPObject->GetName().isEmpty())
        pDPObject->SetName(rDoc.GetDPCollection()->CreateNewName());

    ScRange aNewOut;
    if (!sc::pivot::checkNewOutputRange(*pDPObject, mrDocShell, aNewOut, mbApi))
    {
        *pDPObject = aUndoDPObject;
        return false;
    }

    //  test if new output area is empty except for old area
    if (!mbApi)
    {
        // OutRange of pDPObject is still old area
        if (!sc::pivot::lcl_EmptyExcept(rDoc, aNewOut, pDPObject->GetOutRange()))
        {
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
                ScDocShell::GetActiveDialogParent(), VclMessageType::Question,
                VclButtonsType::YesNo, ScResId(STR_PIVOT_NOTEMPTY)));
            xQueryBox->set_default_response(RET_YES);
            if (xQueryBox->run() == RET_NO)
            {
                //! like above (not editable)
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
            mbAllowMove));
    }

    syncSheetViews();

    // notify API objects
    rDoc.BroadcastUno(ScDataPilotModifiedHint(pDPObject->GetName()));
    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
