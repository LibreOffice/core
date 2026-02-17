/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <operation/SetNormalStringOperation.hxx>

#include <docfuncutil.hxx>
#include <docfunc.hxx>
#include <address.hxx>
#include <editable.hxx>
#include <undocell.hxx>
#include <validat.hxx>
#include <detfunc.hxx>

#include <memory>

namespace sc
{
SetNormalStringOperation::SetNormalStringOperation(ScDocFunc& rDocFunc, ScDocShell& rDocShell,
                                                   const ScAddress& rPosition,
                                                   const OUString& rText, bool bApi)
    : Operation(OperationType::SetNormalString, true, bApi)
    , mrDocFunc(rDocFunc)
    , mrDocShell(rDocShell)
    , mrPosition(rPosition)
    , mrText(rText)
{
}

bool SetNormalStringOperation::runImplementation()
{
    ScDocShellModificator aModificator(mrDocShell);
    ScDocument& rDoc = mrDocShell.GetDocument();

    bool bUndo(rDoc.IsUndoEnabled());

    ScAddress aPosition = convertAddress(mrPosition);

    ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
        rDoc, aPosition.Tab(), aPosition.Col(), aPosition.Row(), aPosition.Col(), aPosition.Row());
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    bool bEditDeleted = (rDoc.GetCellType(aPosition) == CELLTYPE_EDIT);
    ScUndoEnterData::ValuesType aOldValues;

    if (bUndo)
    {
        ScUndoEnterData::Value aOldValue;

        aOldValue.mnTab = aPosition.Tab();
        aOldValue.maCell.assign(rDoc, aPosition);

        const ScPatternAttr* pPattern
            = rDoc.GetPattern(aPosition.Col(), aPosition.Row(), aPosition.Tab());
        if (const SfxUInt32Item* pItem
            = pPattern->GetItemSet().GetItemIfSet(ATTR_VALUE_FORMAT, false))
        {
            aOldValue.mbHasFormat = true;
            aOldValue.mnFormat = pItem->GetValue();
        }
        else
            aOldValue.mbHasFormat = false;

        aOldValues.push_back(aOldValue);
    }

    tools::Long nBefore(mrDocShell.GetTwipWidthHint(aPosition));
    mbIsNumberFormatSet = rDoc.SetString(aPosition.Col(), aPosition.Row(), aPosition.Tab(), mrText);
    tools::Long nAfter(mrDocShell.GetTwipWidthHint(aPosition));

    if (bUndo)
    {
        //  because of ChangeTracking, UndoAction can be created only after SetString was called
        mrDocShell.GetUndoManager()->AddUndoAction(
            std::make_unique<ScUndoEnterData>(&mrDocShell, aPosition, aOldValues, mrText, nullptr));
    }

    if (bEditDeleted || rDoc.HasAttrib(ScRange(aPosition), HasAttrFlags::NeedHeight))
        mrDocFunc.AdjustRowHeight(ScRange(aPosition), true, mbApi);

    syncSheetViews();

    mrDocShell.PostPaintCell(aPosition, std::max(nBefore, nAfter));
    aModificator.SetDocumentModified();

    // notify input handler here the same way as in PutCell
    if (mbApi)
        mrDocFunc.NotifyInputHandler(aPosition);

    const SfxUInt32Item* pItem = rDoc.GetAttr(aPosition, ATTR_VALIDDATA);
    const ScValidationData* pData = rDoc.GetValidationEntry(pItem->GetValue());
    if (pData)
    {
        ScRefCellValue aCell(rDoc, aPosition);
        if (pData->IsDataValid(aCell, aPosition))
            ScDetectiveFunc(rDoc, aPosition.Tab())
                .DeleteCirclesAt(aPosition.Col(), aPosition.Row());
    }

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
