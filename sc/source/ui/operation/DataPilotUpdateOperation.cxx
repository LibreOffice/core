/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/DataPilotUpdateOperation.hxx>
#include <operation/CreatePivotTableOperation.hxx>
#include <operation/RemovePivotTableOperation.hxx>
#include <operation/UpdatePivotTableOperation.hxx>
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
DataPilotUpdateOperation::DataPilotUpdateOperation(ScDocShell& rDocShell, ScDPObject* pOldObj,
                                                   ScDPObject const* pNewObj, bool bRecord,
                                                   bool bApi, bool bAllowMove)
    : Operation(OperationType::PivotTableUpdate, bRecord, bApi)
    , mrDocShell(rDocShell)
    , mpOldObj(pOldObj)
    , mpNewObj(pNewObj)
    , mbAllowMove(bAllowMove)
{
}

bool DataPilotUpdateOperation::runImplementation()
{
    // Delegate certain to sub-operations. Probably we need to find a better way how to do this.

    if (!mpOldObj)
    {
        if (!mpNewObj)
            return false;

        CreatePivotTableOperation aOperation(mrDocShell, *mpNewObj, mbRecord, mbApi);
        return aOperation.run();
    }

    if (!mpNewObj)
    {
        RemovePivotTableOperation aOperation(mrDocShell, *mpOldObj, mbRecord, mbApi);
        return aOperation.run();
    }

    if (mpOldObj == mpNewObj)
    {
        UpdatePivotTableOperation aOperation(mrDocShell, *mpOldObj, mbRecord, mbApi);
        return aOperation.run();
    }

    OSL_ASSERT(mpOldObj && mpNewObj && mpOldObj != mpNewObj);

    ScDocShellModificator aModificator(mrDocShell);
    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

    ScRangeList aRanges;
    aRanges.push_back(mpOldObj->GetOutRange());
    aRanges.push_back(
        ScRange(mpNewObj->GetOutRange()
                    .aStart)); // at least one cell in the output position must be editable.

    if (!checkSheetViewProtection())
        return false;

    if (!sc::pivot::isEditable(mrDocShell, aRanges, mbApi))
        return false;

    ScDocumentUniquePtr pOldUndoDoc;
    ScDocumentUniquePtr pNewUndoDoc;

    ScDPObject aUndoDPObj(*mpOldObj); // for undo or revert on failure

    ScDocument& rDoc = mrDocShell.GetDocument();
    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    if (mbRecord)
        sc::pivot::createUndoDoc(pOldUndoDoc, rDoc, mpOldObj->GetOutRange());

    mpNewObj->WriteSourceDataTo(*mpOldObj); // copy source data

    ScDPSaveData* pData = mpNewObj->GetSaveData();
    OSL_ENSURE(pData, "no SaveData from living DPObject");
    if (pData)
        mpOldObj->SetSaveData(*pData); // copy SaveData

    mpOldObj->SetAllowMove(mbAllowMove);
    mpOldObj->ReloadGroupTableData();
    mpOldObj->SyncAllDimensionMembers();
    mpOldObj->InvalidateData(); // before getting the new output area

    //  make sure the table has a name (not set by dialog)
    if (mpOldObj->GetName().isEmpty())
        mpOldObj->SetName(rDoc.GetDPCollection()->CreateNewName());

    ScRange aNewOut;
    if (!sc::pivot::checkNewOutputRange(*mpOldObj, mrDocShell, aNewOut, mbApi))
    {
        *mpOldObj = aUndoDPObj;
        return false;
    }

    //  test if new output area is empty except for old area
    if (!mbApi)
    {
        // OutRange of mpOldObj (pDestObj) is still old area
        if (!sc::pivot::lcl_EmptyExcept(rDoc, aNewOut, mpOldObj->GetOutRange()))
        {
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
                ScDocShell::GetActiveDialogParent(), VclMessageType::Question,
                VclButtonsType::YesNo, ScResId(STR_PIVOT_NOTEMPTY)));
            xQueryBox->set_default_response(RET_YES);
            if (xQueryBox->run() == RET_NO)
            {
                //! like above (not editable)
                *mpOldObj = aUndoDPObj;
                return false;
            }
        }
    }

    if (mbRecord)
        sc::pivot::createUndoDoc(pNewUndoDoc, rDoc, aNewOut);

    mpOldObj->Output(aNewOut.aStart);
    mrDocShell.PostPaintGridAll(); //! only necessary parts

    if (mbRecord)
    {
        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoDataPilot>(
            &mrDocShell, std::move(pOldUndoDoc), std::move(pNewUndoDoc), &aUndoDPObj, mpOldObj,
            mbAllowMove));
    }

    // notify API objects
    rDoc.BroadcastUno(ScDataPilotModifiedHint(mpOldObj->GetName()));
    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
