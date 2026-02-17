/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/SetStringOperation.hxx>

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
SetStringOperation::SetStringOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                       const ScAddress& rPosition, OUString const& rString,
                                       bool bApi)
    : Operation(OperationType::SetString, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mrString(rString)
{
}

bool SetStringOperation::runImplementation()
{
    ScAddress aPosition = convertAddress(mrPosition);

    ScDocShellModificator aModificator(mrDocShell);

    ScDocument& rDoc = mrDocShell.GetDocument();
    bool bUndo = rDoc.IsUndoEnabled();

    bool bHeight = rDoc.HasAttrib(ScRange(aPosition), HasAttrFlags::NeedHeight);

    ScCellValue aOldVal;
    if (bUndo)
        aOldVal.assign(rDoc, aPosition);

    ScSetStringParam aParam;
    aParam.setTextInput();
    rDoc.SetString(aPosition, mrString, &aParam);

    if (bUndo)
    {
        SfxUndoManager* pUndoMgr = mrDocShell.GetUndoManager();
        ScCellValue aNewVal;
        aNewVal.assign(rDoc, aPosition);
        pUndoMgr->AddUndoAction(
            std::make_unique<ScUndoSetCell>(&mrDocShell, aPosition, aOldVal, aNewVal));
    }

    syncSheetViews();

    if (bHeight)
        mrDocFunc.AdjustRowHeight(ScRange(aPosition), true, mbApi);

    mrDocShell.PostPaintCell(aPosition);
    aModificator.SetDocumentModified();

    // #103934#; notify editline and cell in edit mode
    if (mbApi)
        mrDocFunc.NotifyInputHandler(aPosition);

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
