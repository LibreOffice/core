/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/RemovePivotTableOperation.hxx>
#include <operation/PivotTableOperationCommon.hxx>

#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdoole2.hxx>

#include <undodat.hxx>
#include <scresid.hxx>
#include <drwlayer.hxx>
#include <chartlis.hxx>
#include <ChartTools.hxx>

namespace sc
{
RemovePivotTableOperation::RemovePivotTableOperation(ScDocShell& rDocShell,
                                                     ScDPObject const& rDPObj, bool bRecord,
                                                     bool bApi)
    : Operation(OperationType::PivotTableRemove, bRecord, bApi)
    , mrDocShell(rDocShell)
    , mrDPObj(rDPObj)
{
}

bool RemovePivotTableOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    weld::WaitObject aWait(ScDocShell::GetActiveDialogParent());

    if (!checkSheetViewProtection())
        return false;

    if (!sc::pivot::isEditable(mrDocShell, mrDPObj.GetOutRange(), mbApi))
        return false;

    ScDocument& rDoc = mrDocShell.GetDocument();

    if (!mbApi)
    {
        // If we come from GUI - ask to delete the associated pivot charts too...
        std::vector<SdrOle2Obj*> aListOfObjects
            = sc::tools::getAllPivotChartsConnectedTo(mrDPObj.GetName(), &mrDocShell);

        ScDrawLayer* pModel = rDoc.GetDrawLayer();

        if (pModel && !aListOfObjects.empty())
        {
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
                ScDocShell::GetActiveDialogParent(), VclMessageType::Question,
                VclButtonsType::YesNo, ScResId(STR_PIVOT_REMOVE_PIVOTCHART)));
            xQueryBox->set_default_response(RET_YES);
            if (xQueryBox->run() == RET_NO)
            {
                return false;
            }
            else
            {
                for (SdrOle2Obj* pChartObject : aListOfObjects)
                {
                    rDoc.GetChartListenerCollection()->removeByName(pChartObject->GetName());
                    pModel->AddUndo(std::make_unique<SdrUndoDelObj>(*pChartObject));
                    pChartObject->getSdrPageFromSdrObject()->RemoveObject(
                        pChartObject->GetOrdNum());
                }
            }
        }
    }

    ScDocumentUniquePtr pOldUndoDoc;
    std::unique_ptr<ScDPObject> pUndoDPObj;

    if (mbRecord)
        pUndoDPObj.reset(new ScDPObject(mrDPObj)); // copy old settings for undo

    if (mbRecord && !rDoc.IsUndoEnabled())
        mbRecord = false;

    //  delete table

    ScRange aRange = mrDPObj.GetOutRange();
    SCTAB nTab = aRange.aStart.Tab();

    if (mbRecord)
        sc::pivot::createUndoDoc(pOldUndoDoc, rDoc, aRange);

    rDoc.DeleteAreaTab(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                       aRange.aEnd.Row(), nTab, InsertDeleteFlags::ALL);
    rDoc.RemoveFlagsTab(aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(),
                        aRange.aEnd.Row(), nTab, ScMF::Auto);

    rDoc.GetDPCollection()->FreeTable(&mrDPObj); // object is deleted here

    mrDocShell.PostPaintGridAll(); //! only necessary parts
    mrDocShell.PostPaint(aRange, PaintPartFlags::Grid);

    if (mbRecord)
    {
        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoDataPilot>(
            &mrDocShell, std::move(pOldUndoDoc), nullptr, pUndoDPObj.get(), nullptr, false));

        // pUndoDPObj is copied
    }

    aModificator.SetDocumentModified();
    return true;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
