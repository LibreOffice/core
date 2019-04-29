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

ScPivotLayoutTreeListBase::ScPivotLayoutTreeListBase(std::unique_ptr<weld::TreeView> xControl, SvPivotTreeListType eType)
    : mxControl(std::move(xControl))
    , meType(eType)
    , mpParent(nullptr)
{
    mxControl->connect_focus_in(LINK(this, ScPivotLayoutTreeListBase, GetFocusHdl));
    mxControl->connect_focus_out(LINK(this, ScPivotLayoutTreeListBase, LoseFocusHdl));

#if 0
    SetDragDropMode(DragDropMode::CTRL_MOVE | DragDropMode::APP_MOVE  | DragDropMode::APP_DROP);
#endif
}

ScPivotLayoutTreeListBase::~ScPivotLayoutTreeListBase()
{
}

void ScPivotLayoutTreeListBase::Setup(ScPivotLayoutDialog* pParent)
{
    mpParent = pParent;
}

#if 0
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
#endif

bool ScPivotLayoutTreeListBase::HasEntry(const weld::TreeIter& rEntry)
{
    std::unique_ptr<weld::TreeIter> xEntry(mxControl->make_iterator());
    if (!mxControl->get_iter_first(*xEntry))
        return false;

    do
    {
        if (mxControl->iter_compare(*xEntry, rEntry))
            return true;
    } while (mxControl->iter_next(*xEntry));

    return false;
}

void ScPivotLayoutTreeListBase::PushEntriesToPivotFieldVector(ScPivotFieldVector& rVector)
{
    std::unique_ptr<weld::TreeIter> xEachEntry(mxControl->make_iterator());
    if (!mxControl->get_iter_first(*xEachEntry))
        return;
    do
    {
        ScItemValue* pItemValue = reinterpret_cast<ScItemValue*>(mxControl->get_id(*xEachEntry).toInt64());
        ScPivotFuncData& rFunctionData = pItemValue->maFunctionData;

        ScPivotField aField;
        aField.nCol          = rFunctionData.mnCol;
        aField.mnOriginalDim = rFunctionData.mnOriginalDim;
        aField.nFuncMask     = rFunctionData.mnFuncMask;
        aField.mnDupCount    = rFunctionData.mnDupCount;
        aField.maFieldRef    = rFunctionData.maFieldRef;
        rVector.push_back(aField);
    } while (mxControl->iter_next(*xEachEntry));
}

#if 0
void ScPivotLayoutTreeListBase::InsertEntryForSourceTarget(SvTreeListEntry* /*pSource*/, SvTreeListEntry* /*pTarget*/)
{}
#endif

void ScPivotLayoutTreeListBase::RemoveEntryForItem(const ScItemValue* pItemValue)
{
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(pItemValue)));
    mxControl->remove_id(sId);
}

IMPL_LINK_NOARG(ScPivotLayoutTreeListBase, GetFocusHdl, weld::Widget&, void)
{
    if (!mpParent || !mpParent->mpPreviouslyFocusedListBox)
        return;
#if 0
    if (GetGetFocusFlags() & GetFocusFlags::Mnemonic)
    {
        SvTreeListEntry* pEntry = mpParent->mpPreviouslyFocusedListBox->GetCurEntry();
        if (pEntry)
            InsertEntryForSourceTarget(pEntry, nullptr);
        mpParent->mpPreviouslyFocusedListBox->GrabFocus();
    }
#endif
}

IMPL_LINK_NOARG(ScPivotLayoutTreeListBase, LoseFocusHdl, weld::Widget&, void)
{
    if (mpParent)
        mpParent->mpPreviouslyFocusedListBox = this;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
