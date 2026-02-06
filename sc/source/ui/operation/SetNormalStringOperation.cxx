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

    if (!checkSheetViewProtection())
        return false;

    ScEditableTester aTester = ScEditableTester::CreateAndTestBlock(
        rDoc, mrPosition.Tab(), mrPosition.Col(), mrPosition.Row(), mrPosition.Col(),
        mrPosition.Row());
    if (!aTester.IsEditable())
    {
        if (!mbApi)
            mrDocShell.ErrorMessage(aTester.GetMessageId());
        return false;
    }

    bool bEditDeleted = (rDoc.GetCellType(mrPosition) == CELLTYPE_EDIT);
    ScUndoEnterData::ValuesType aOldValues;

    if (bUndo)
    {
        ScUndoEnterData::Value aOldValue;

        aOldValue.mnTab = mrPosition.Tab();
        aOldValue.maCell.assign(rDoc, mrPosition);

        const ScPatternAttr* pPattern
            = rDoc.GetPattern(mrPosition.Col(), mrPosition.Row(), mrPosition.Tab());
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

    tools::Long nBefore(mrDocShell.GetTwipWidthHint(mrPosition));
    mbIsNumberFormatSet
        = rDoc.SetString(mrPosition.Col(), mrPosition.Row(), mrPosition.Tab(), mrText);
    tools::Long nAfter(mrDocShell.GetTwipWidthHint(mrPosition));

    if (bUndo)
    {
        //  because of ChangeTracking, UndoAction can be created only after SetString was called
        mrDocShell.GetUndoManager()->AddUndoAction(std::make_unique<ScUndoEnterData>(
            &mrDocShell, mrPosition, aOldValues, mrText, nullptr));
    }

    if (bEditDeleted || rDoc.HasAttrib(ScRange(mrPosition), HasAttrFlags::NeedHeight))
        mrDocFunc.AdjustRowHeight(ScRange(mrPosition), true, mbApi);

    mrDocShell.PostPaintCell(mrPosition, std::max(nBefore, nAfter));
    aModificator.SetDocumentModified();

    // notify input handler here the same way as in PutCell
    if (mbApi)
        mrDocFunc.NotifyInputHandler(mrPosition);

    const SfxUInt32Item* pItem = rDoc.GetAttr(mrPosition, ATTR_VALIDDATA);
    const ScValidationData* pData = rDoc.GetValidationEntry(pItem->GetValue());
    if (pData)
    {
        ScRefCellValue aCell(rDoc, mrPosition);
        if (pData->IsDataValid(aCell, mrPosition))
            ScDetectiveFunc(rDoc, mrPosition.Tab())
                .DeleteCirclesAt(mrPosition.Col(), mrPosition.Row());
    }

    return true;
}
} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
