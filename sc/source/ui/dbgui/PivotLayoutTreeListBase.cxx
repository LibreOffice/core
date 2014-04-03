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

#include "PivotLayoutTreeListBase.hxx"
#include "PivotLayoutDialog.hxx"

#include <reffact.hxx>
#include <svtools/treelistentry.hxx>
#include "scabstdlg.hxx"

using namespace std;

ScPivotLayoutTreeListBase::ScPivotLayoutTreeListBase(Window* pParent, WinBits nBits, SvPivotTreeListType eType) :
    SvTreeListBox(pParent, nBits),
    meType(eType)
{
    SetHighlightRange();
    SetDragDropMode(SV_DRAGDROP_CTRL_MOVE | SV_DRAGDROP_APP_MOVE  | SV_DRAGDROP_APP_DROP);
}

ScPivotLayoutTreeListBase::~ScPivotLayoutTreeListBase()
{}

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

sal_Int8 ScPivotLayoutTreeListBase::AcceptDrop(const AcceptDropEvent& rEvent)
{
    return SvTreeListBox::AcceptDrop(rEvent);
}

bool ScPivotLayoutTreeListBase::NotifyAcceptDrop(SvTreeListEntry* /*pEntry*/)
{
    return true;
}

sal_Bool ScPivotLayoutTreeListBase::NotifyMoving(SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
                                                SvTreeListEntry*& /*rpNewParent*/, sal_uLong& /*rNewChildPos*/)
{
    InsertEntryForSourceTarget(pSource, pTarget);
    return sal_False;
}

sal_Bool ScPivotLayoutTreeListBase::NotifyCopying(SvTreeListEntry* /*pTarget*/, SvTreeListEntry* /*pSource*/,
                                                 SvTreeListEntry*& /*rpNewParent*/, sal_uLong& /*rNewChildPos*/)
{
    return sal_False;
}

bool ScPivotLayoutTreeListBase::HasEntry(SvTreeListEntry* pEntry)
{
    SvTreeListEntry* pEachEntry;
    for (pEachEntry = First(); pEachEntry != NULL; pEachEntry = Next(pEachEntry))
    {
        if(pEachEntry == pEntry)
            return true;
    }
    return false;
}

void ScPivotLayoutTreeListBase::PushEntriesToPivotFieldVector(ScPivotFieldVector& rVector)
{
    SvTreeListEntry* pEachEntry;
    for (pEachEntry = First(); pEachEntry != NULL; pEachEntry = Next(pEachEntry))
    {
        ScItemValue* pItemValue = (ScItemValue*) pEachEntry->GetUserData();
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

void ScPivotLayoutTreeListBase::InsertEntryForItem(ScItemValue* /*pItemValue*/, sal_uLong /*nPosition*/)
{}

void ScPivotLayoutTreeListBase::RemoveEntryForItem(ScItemValue* pItemValue)
{
    SvTreeListEntry* pEachEntry;
    for (pEachEntry = First(); pEachEntry != NULL; pEachEntry = Next(pEachEntry))
    {
        ScItemValue* pEachItemValue = (ScItemValue*) pEachEntry->GetUserData();
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

    if( GetGetFocusFlags() & GETFOCUS_MNEMONIC )
    {
        SvTreeListEntry* pEntry = mpParent->mpPreviouslyFocusedListBox->GetCurEntry();
        InsertEntryForSourceTarget(pEntry, NULL);

        if(mpParent->mpPreviouslyFocusedListBox != NULL)
            mpParent->mpPreviouslyFocusedListBox->GrabFocus();
    }

    mpParent->mpCurrentlyFocusedListBox = this;
}

void ScPivotLayoutTreeListBase::LoseFocus()
{
    SvTreeListBox::LoseFocus();
    mpParent->mpPreviouslyFocusedListBox = this;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
