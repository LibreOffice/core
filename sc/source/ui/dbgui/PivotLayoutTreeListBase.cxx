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

ScPivotLayoutTreeListBase::ScPivotLayoutTreeListBase(std::unique_ptr<weld::TreeView> xControl, SvPivotTreeListType eType)
    : mxControl(std::move(xControl))
    , maDropTargetHelper(*this)
    , meType(eType)
    , mpParent(nullptr)
{
    mxControl->connect_focus_in(LINK(this, ScPivotLayoutTreeListBase, GetFocusHdl));
    mxControl->connect_mnemonic_activate(LINK(this, ScPivotLayoutTreeListBase, MnemonicActivateHdl));
    mxControl->connect_focus_out(LINK(this, ScPivotLayoutTreeListBase, LoseFocusHdl));
}

ScPivotLayoutTreeListBase::~ScPivotLayoutTreeListBase()
{
}

void ScPivotLayoutTreeListBase::Setup(ScPivotLayoutDialog* pParent)
{
    mpParent = pParent;
}

ScPivotLayoutTreeDropTarget::ScPivotLayoutTreeDropTarget(ScPivotLayoutTreeListBase& rTreeView)
    : DropTargetHelper(rTreeView.get_widget().get_drop_target())
    , m_rTreeView(rTreeView)
{
}

sal_Int8 ScPivotLayoutTreeDropTarget::AcceptDrop(const AcceptDropEvent& rEvt)
{
    // to enable the autoscroll when we're close to the edges
    weld::TreeView& rWidget = m_rTreeView.get_widget();
    rWidget.get_dest_row_at_pos(rEvt.maPosPixel, nullptr, true);
    return DND_ACTION_MOVE;
}

sal_Int8 ScPivotLayoutTreeDropTarget::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    weld::TreeView& rWidget = m_rTreeView.get_widget();
    weld::TreeView* pSource = rWidget.get_drag_source();
    if (!pSource)
        return DND_ACTION_NONE;

    std::unique_ptr<weld::TreeIter> xTarget(rWidget.make_iterator());
    int nTargetPos = -1;
    if (rWidget.get_dest_row_at_pos(rEvt.maPosPixel, xTarget.get(), true))
        nTargetPos = rWidget.get_iter_index_in_parent(*xTarget);
    m_rTreeView.InsertEntryForSourceTarget(*pSource, nTargetPos);
    rWidget.unset_drag_dest_row();
    return DND_ACTION_NONE;
}

void ScPivotLayoutTreeListBase::PushEntriesToPivotFieldVector(ScPivotFieldVector& rVector)
{
    std::unique_ptr<weld::TreeIter> xEachEntry(mxControl->make_iterator());
    if (!mxControl->get_iter_first(*xEachEntry))
        return;

    std::optional<ScPivotField> oDataField;

    do
    {
        ScItemValue* pItemValue = weld::fromId<ScItemValue*>(mxControl->get_id(*xEachEntry));
        ScPivotFuncData& rFunctionData = pItemValue->maFunctionData;

        ScPivotField aField;
        aField.nCol          = rFunctionData.mnCol;
        aField.mnOriginalDim = rFunctionData.mnOriginalDim;
        aField.nFuncMask     = rFunctionData.mnFuncMask;
        aField.mnDupCount    = rFunctionData.mnDupCount;
        aField.maFieldRef    = rFunctionData.maFieldRef;

        if (aField.nCol == PIVOT_DATA_FIELD)
            oDataField = aField;
        else
            rVector.push_back(aField);
    } while (mxControl->iter_next(*xEachEntry));

    if (oDataField)
        rVector.push_back(*oDataField);
}

void ScPivotLayoutTreeListBase::InsertEntryForSourceTarget(weld::TreeView& /*pSource*/, int /*nTarget*/)
{
}

void ScPivotLayoutTreeListBase::RemoveEntryForItem(const ScItemValue* pItemValue)
{
    OUString sId(weld::toId(pItemValue));
    int nPos = mxControl->find_id(sId);
    if (nPos == -1)
        return;
    mxControl->remove(nPos);
}

IMPL_LINK_NOARG(ScPivotLayoutTreeListBase, GetFocusHdl, weld::Widget&, void)
{
    if (!mpParent)
        return;
    mpParent->mpPreviouslyFocusedListBox = this;
}

IMPL_LINK_NOARG(ScPivotLayoutTreeListBase, MnemonicActivateHdl, weld::Widget&, bool)
{
    if (!mpParent || !mpParent->mpPreviouslyFocusedListBox)
        return false;
    weld::TreeView& rSource = mpParent->mpPreviouslyFocusedListBox->get_widget();
    int nEntry = rSource.get_cursor_index();
    if (nEntry != -1)
        InsertEntryForSourceTarget(rSource, -1);
    return true;
}

IMPL_LINK_NOARG(ScPivotLayoutTreeListBase, LoseFocusHdl, weld::Widget&, void)
{
    if (!mpParent)
        return;
    mpParent->mpPreviouslyFocusedListBox = nullptr;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
