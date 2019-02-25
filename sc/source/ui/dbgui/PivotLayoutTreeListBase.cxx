/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 */

#include <PivotLayoutTreeListBase.hxx>
#include <PivotLayoutDialog.hxx>

#include <vcl/treelistentry.hxx>

ScPivotLayoutTreeListBase::ScPivotLayoutTreeListBase(vcl::Window* pParent, WinBits nBits, SvPivotTreeListType eType)
    : SvTreeListBox(pParent, nBits)
    , meType(eType)
    , mpParent(nullptr)
{
    SetHighlightRange();
    SetDragDropMode(DragDropMode::CTRL_MOVE | DragDropMode::APP_MOVE  | DragDropMode::APP_DROP);
}

ScPivotLayoutTreeListBase::~ScPivotLayoutTreeListBase()
{
    disposeOnce();
}

void ScPivotLayoutTreeListBase::dispose()
{
    mpParent.clear();
    SvTreeListBox::dispose();
}

void ScPivotLayoutTreeListBase::Setup(ScPivotLayoutDialog* pParent)
{
    mpParent = pParent;
}

DragDropMode ScPivotLayoutTreeListBase::NotifyStartDrag(TransferDataContainer& /*aTransferDataContainer*/,
                                                  SvTreeListEntry* /*pEntry*/ )
{
    return GetDragDropMode();
}

void ScPivotLayoutTreeListBase::DragFinished(sal_Int8 /*nDropAction*/)
{}

bool ScPivotLayoutTreeListBase::NotifyAcceptDrop(SvTreeListEntry* /*pEntry*/)
{
    return true;
}

TriState ScPivotLayoutTreeListBase::NotifyMoving(SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
                                                SvTreeListEntry*& /*rpNewParent*/, sal_uLong& /*rNewChildPos*/)
{
    InsertEntryForSourceTarget(pSource, pTarget);
    return TRISTATE_FALSE;
}

TriState ScPivotLayoutTreeListBase::NotifyCopying(SvTreeListEntry* /*pTarget*/, SvTreeListEntry* /*pSource*/,
                                                 SvTreeListEntry*& /*rpNewParent*/, sal_uLong& /*rNewChildPos*/)
{
    return TRISTATE_FALSE;
}

bool ScPivotLayoutTreeListBase::HasEntry(const SvTreeListEntry* pEntry)
{
    SvTreeListEntry* pEachEntry;
    for (pEachEntry = First(); pEachEntry != nullptr; pEachEntry = Next(pEachEntry))
    {
        if(pEachEntry == pEntry)
            return true;
    }
    return false;
}

void ScPivotLayoutTreeListBase::PushEntriesToPivotFieldVector(ScPivotFieldVector& rVector)
{
    SvTreeListEntry* pEachEntry;
    for (pEachEntry = First(); pEachEntry != nullptr; pEachEntry = Next(pEachEntry))
    {
        ScItemValue* pItemValue = static_cast<ScItemValue*>(pEachEntry->GetUserData());
        ScPivotFuncData& rFunctionData = pItemValue->maFunctionData;

        ScPivotField aField;
        aField.nCol          = rFunctionData.mnCol;
        aField.mnOriginalDim = rFunctionData.mnOriginalDim;
        aField.nFuncMask     = rFunctionData.mnFuncMask;
        aField.mnDupCount    = rFunctionData.mnDupCount;
        aField.maFieldRef    = rFunctionData.maFieldRef;
        rVector.push_back(aField);
    }
}

void ScPivotLayoutTreeListBase::InsertEntryForSourceTarget(SvTreeListEntry* /*pSource*/, SvTreeListEntry* /*pTarget*/)
{}

void ScPivotLayoutTreeListBase::RemoveEntryForItem(const ScItemValue* pItemValue)
{
    SvTreeListEntry* pEachEntry;
    for (pEachEntry = First(); pEachEntry != nullptr; pEachEntry = Next(pEachEntry))
    {
        ScItemValue* pEachItemValue = static_cast<ScItemValue*>(pEachEntry->GetUserData());
        if (pEachItemValue == pItemValue)
        {
            GetModel()->Remove(pEachEntry);
            return;
        }
    }
}

void ScPivotLayoutTreeListBase::GetFocus()
{
    SvTreeListBox::GetFocus();

    if (!mpParent || !mpParent->mpPreviouslyFocusedListBox)
        return;

    if (GetGetFocusFlags() & GetFocusFlags::Mnemonic)
    {
        SvTreeListEntry* pEntry = mpParent->mpPreviouslyFocusedListBox->GetCurEntry();
        if (pEntry)
            InsertEntryForSourceTarget(pEntry, nullptr);
        mpParent->mpPreviouslyFocusedListBox->GrabFocus();
    }
}

void ScPivotLayoutTreeListBase::LoseFocus()
{
    SvTreeListBox::LoseFocus();
    if (mpParent)
        mpParent->mpPreviouslyFocusedListBox = this;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
