/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/CreatePivotTableOperation.hxx>
#include <operation/PivotTableOperationCommon.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <dpsave.hxx>
#include <undodat.hxx>
#include <scresid.hxx>
#include <hints.hxx>

namespace sc
{
CreatePivotTableOperation::CreatePivotTableOperation(ScDocShell& rDocShell,
                                                     ScDPObject const& rDPObj, bool bRecord,
                                                     bool bApi)
    : Operation(OperationType::PivotTableCreate, bRecord, bApi)
    , mrDocShell(rDocShell)
    , mrDPObj(rDPObj)
{
}

bool CreatePivotTableOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

    if (!checkSheetViewProtection())
        return false;

    // At least one cell in the output range should be editable. Check in advance.
    ScDocument& rDoc = mrDocShell.GetDocument();
    if (!rDoc.IsImportingXML()
        && !sc::pivot::isEditable(mrDocShell, ScRange(mrDPObj.GetOutRange().aStart), mbApi))
        return false;

    ScDocumentUniquePtr pNewUndoDoc;

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    //  output range must be set at pNewObj
    std::unique_ptr<ScDPObject> pDestObj(new ScDPObject(mrDPObj));

    ScDPObject& rDestObj = *pDestObj;

    // #i94570# When changing the output position in the dialog, a new table is created
    // with the settings from the old table, including the name.
    // So we have to check for duplicate names here (before inserting).
    if (rDoc.GetDPCollection()->GetByName(rDestObj.GetName()))
        rDestObj.SetName(OUString()); // ignore the invalid name, create a new name below

    // Synchronize groups between linked tables
    {
        const ScDPDimensionSaveData* pGroups = nullptr;
        bool bRefFound = rDoc.GetDPCollection()->GetReferenceGroups(rDestObj, &pGroups);
        if (bRefFound)
        {
            ScDPSaveData* pSaveData = rDestObj.GetSaveData();
            if (pSaveData)
                pSaveData->SetDimensionData(pGroups);
        }
    }

    rDoc.GetDPCollection()->InsertNewTable(std::move(pDestObj));

    rDestObj.ReloadGroupTableData();
    rDestObj.SyncAllDimensionMembers();
    rDestObj.InvalidateData(); // before getting the new output area

    //  make sure the table has a name (not set by dialog)
    if (rDestObj.GetName().isEmpty())
        rDestObj.SetName(rDoc.GetDPCollection()->CreateNewName());

    bool bOverflow = false;
    ScRange aNewOut = rDestObj.GetNewOutputRange(bOverflow);

    if (bOverflow)
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(STR_PIVOT_ERROR);

        return false;
    }

    if (!rDoc.IsImportingXML())
    {
        ScEditableTester aTester
            = ScEditableTester::CreateAndTestRange(rDoc, aNewOut, sc::EditAction::Unknown);
        if (!aTester.IsEditable())
        {
            //  destination area isn't editable
            if (!mbApi)
                mrDocShell.ErrorMessage(aTester.GetMessageId());

            return false;
        }
    }

    //  test if new output area is empty except for old area
    if (!mbApi)
    {
        bool bEmpty
            = rDoc.IsBlockEmpty(aNewOut.aStart.Col(), aNewOut.aStart.Row(), aNewOut.aEnd.Col(),
                                aNewOut.aEnd.Row(), aNewOut.aStart.Tab());

        if (!bEmpty)
        {
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
                ScDocShell::GetActiveDialogParent(), VclMessageType::Question,
                VclButtonsType::YesNo, ScResId(STR_PIVOT_NOTEMPTY)));
            xQueryBox->set_default_response(RET_YES);
            if (xQueryBox->run() == RET_NO)
            {
                //! like above (not editable)
                return false;
            }
        }
    }

    if (mbRecord)
        sc::pivot::createUndoDoc(pNewUndoDoc, rDoc, aNewOut);

    rDestObj.Output(aNewOut.aStart);
    mrDocShell.PostPaintGridAll(); //! only necessary parts

    if (mbRecord)
    {
        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoDataPilot>(
            &mrDocShell, nullptr, std::move(pNewUndoDoc), nullptr, &rDestObj, false));
    }

    // notify API objects
    rDoc.BroadcastUno(ScDataPilotModifiedHint(rDestObj.GetName()));
    aModificator.SetDocumentModified();

    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
