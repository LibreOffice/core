/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/SetValueOperation.hxx>

#include <docfuncutil.hxx>
#include <docfunc.hxx>
#include <address.hxx>
#include <editable.hxx>
#include <undocell.hxx>
#include <validat.hxx>
#include <stringutil.hxx>

#include <memory>

namespace sc
{
SetValueOperation::SetValueOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                     const ScAddress& rPosition, double fValue, bool bApi)
    : Operation(OperationType::SetValue, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mfValue(fValue)
{
}

bool SetValueOperation::runImplementation()
{
    ScAddress const& rPos = mrPosition;

    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();
    bool bUndo = rDoc.IsUndoEnabled();
    bool bHeight = rDoc.HasAttrib(ScRange(rPos), HasAttrFlags::NeedHeight);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(rDoc, rPos);

    rDoc.SetValue(rPos, mfValue);

    if (bUndo)
    {
        SfxUndoManager* pUndoMgr = mrDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(rDoc, rPos);
        pUndoMgr->AddUndoAction(
            std::make_unique<ScUndoSetCell>(&mrDocShell, rPos, aOldVal, aNewVal));
    }

    if (bHeight)
        mrDocFunc.AdjustRowHeight(ScRange(rPos), true, mbApi);

    mrDocShell.PostPaintCell(rPos);
    aModificator.SetDocumentModified();

    // #103934#; notify editline and cell in edit mode
    if (mbApi)
        mrDocFunc.NotifyInputHandler(rPos);

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
