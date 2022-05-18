/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <contentcontroldlg.hxx>

#include <vcl/weld.hxx>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <IDocumentState.hxx>
#include <swuiexp.hxx>

using namespace com::sun::star;

SwContentControlDlg::SwContentControlDlg(weld::Window* pParent, SwWrtShell& rWrtShell)
    : SfxDialogController(pParent, "modules/swriter/ui/contentcontroldlg.ui",
                          "ContentControlDialog")
    , m_rWrtShell(rWrtShell)
    , m_xShowingPlaceHolderCB(m_xBuilder->weld_check_button("showing_place_holder"))
    , m_xListItemsFrame(m_xBuilder->weld_frame("listitemsframe"))
    , m_xListItems(m_xBuilder->weld_tree_view("listitems"))
    , m_xListItemButtons(m_xBuilder->weld_box("listitembuttons"))
    , m_xInsertBtn(m_xBuilder->weld_button("add"))
    , m_xRenameBtn(m_xBuilder->weld_button("modify"))
    , m_xDeleteBtn(m_xBuilder->weld_button("remove"))
    , m_xMoveUpBtn(m_xBuilder->weld_button("moveup"))
    , m_xMoveDownBtn(m_xBuilder->weld_button("movedown"))
    , m_xOk(m_xBuilder->weld_button("ok"))
{
    m_xListItems->connect_changed(LINK(this, SwContentControlDlg, SelectionChangedHdl));
    m_xOk->connect_clicked(LINK(this, SwContentControlDlg, OkHdl));

    // Only 2 items would be visible by default.
    m_xListItems->set_size_request(-1, m_xListItems->get_height_rows(8));
    // Only the first column would have a non-zero size by default in the SvHeaderTabListBox case.
    m_xListItems->set_column_fixed_widths({ 100, 100 });

    m_xInsertBtn->connect_clicked(LINK(this, SwContentControlDlg, InsertHdl));
    m_xRenameBtn->connect_clicked(LINK(this, SwContentControlDlg, RenameHdl));
    m_xDeleteBtn->connect_clicked(LINK(this, SwContentControlDlg, DeleteHdl));
    m_xMoveUpBtn->connect_clicked(LINK(this, SwContentControlDlg, MoveUpHdl));
    m_xMoveDownBtn->connect_clicked(LINK(this, SwContentControlDlg, MoveDownHdl));

    const SwPosition* pStart = rWrtShell.GetCursor()->Start();
    SwTextNode* pTextNode = pStart->nNode.GetNode().GetTextNode();
    if (!pTextNode)
    {
        return;
    }

    SwTextAttr* pAttr = pTextNode->GetTextAttrAt(pStart->nContent.GetIndex(),
                                                 RES_TXTATR_CONTENTCONTROL, SwTextNode::PARENT);
    if (!pAttr)
    {
        return;
    }

    SwTextContentControl* pTextContentControl = static_txtattr_cast<SwTextContentControl*>(pAttr);
    const SwFormatContentControl& rFormatContentControl = pTextContentControl->GetContentControl();
    m_pContentControl = rFormatContentControl.GetContentControl();

    bool bShowingPlaceHolder = m_pContentControl->GetShowingPlaceHolder();
    TriState eShowingPlaceHolder = bShowingPlaceHolder ? TRISTATE_TRUE : TRISTATE_FALSE;
    m_xShowingPlaceHolderCB->set_state(eShowingPlaceHolder);
    m_xShowingPlaceHolderCB->save_state();

    if (m_pContentControl->HasListItems())
    {
        for (const auto& rListItem : m_pContentControl->GetListItems())
        {
            int nRow = m_xListItems->n_children();
            m_xListItems->append_text(rListItem.m_aDisplayText);
            m_xListItems->set_text(nRow, rListItem.m_aValue, 1);
        }
        m_aSavedListItems = m_pContentControl->GetListItems();
    }
    else
    {
        m_xListItemsFrame->set_visible(false);
        m_xListItemButtons->set_visible(false);
    }
}

SwContentControlDlg::~SwContentControlDlg() {}

IMPL_LINK_NOARG(SwContentControlDlg, OkHdl, weld::Button&, void)
{
    if (!m_pContentControl)
    {
        return;
    }

    bool bChanged = false;
    if (m_xShowingPlaceHolderCB->get_state_changed_from_saved())
    {
        bool bShowingPlaceHolder = m_xShowingPlaceHolderCB->get_state() == TRISTATE_TRUE;
        m_pContentControl->SetShowingPlaceHolder(bShowingPlaceHolder);
        bChanged = true;
    }

    std::vector<SwContentControlListItem> aItems;
    for (int i = 0; i < m_xListItems->n_children(); ++i)
    {
        SwContentControlListItem aItem;
        aItem.m_aDisplayText = m_xListItems->get_text(i, 0);
        aItem.m_aValue = m_xListItems->get_text(i, 1);
        aItems.push_back(aItem);
    }
    if (aItems != m_aSavedListItems)
    {
        m_pContentControl->SetListItems(aItems);
        bChanged = true;
    }

    if (bChanged)
    {
        m_rWrtShell.GetDoc()->getIDocumentState().SetModified();
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SwContentControlDlg, InsertHdl, weld::Button&, void)
{
    SwContentControlListItem aItem;
    SwAbstractDialogFactory& rFact = swui::GetFactory();
    ScopedVclPtr<VclAbstractDialog> pDlg(
        rFact.CreateSwContentControlListItemDlg(m_xDialog.get(), aItem));
    if (!pDlg->Execute())
    {
        return;
    }

    if (aItem.m_aDisplayText.isEmpty() && aItem.m_aValue.isEmpty())
    {
        // Maintain the invariant that value can't be empty.
        return;
    }

    if (aItem.m_aValue.isEmpty())
    {
        aItem.m_aValue = aItem.m_aDisplayText;
    }

    int nRow = m_xListItems->n_children();
    m_xListItems->append_text(aItem.m_aDisplayText);
    m_xListItems->set_text(nRow, aItem.m_aValue, 1);
}

IMPL_LINK_NOARG(SwContentControlDlg, RenameHdl, weld::Button&, void)
{
    int nRow = m_xListItems->get_selected_index();
    if (nRow < 0)
    {
        return;
    }

    SwContentControlListItem aItem;
    aItem.m_aDisplayText = m_xListItems->get_text(nRow, 0);
    aItem.m_aValue = m_xListItems->get_text(nRow, 1);
    SwAbstractDialogFactory& rFact = swui::GetFactory();
    ScopedVclPtr<VclAbstractDialog> pDlg(
        rFact.CreateSwContentControlListItemDlg(m_xDialog.get(), aItem));
    if (!pDlg->Execute())
    {
        return;
    }

    if (aItem.m_aDisplayText.isEmpty() && aItem.m_aValue.isEmpty())
    {
        // Maintain the invariant that value can't be empty.
        return;
    }

    if (aItem.m_aValue.isEmpty())
    {
        aItem.m_aValue = aItem.m_aDisplayText;
    }

    m_xListItems->set_text(nRow, aItem.m_aDisplayText, 0);
    m_xListItems->set_text(nRow, aItem.m_aValue, 1);
}

IMPL_LINK_NOARG(SwContentControlDlg, DeleteHdl, weld::Button&, void)
{
    int nRow = m_xListItems->get_selected_index();
    if (nRow < 0)
    {
        return;
    }

    m_xListItems->remove(nRow);
}

IMPL_LINK_NOARG(SwContentControlDlg, MoveUpHdl, weld::Button&, void)
{
    int nRow = m_xListItems->get_selected_index();
    if (nRow <= 0)
    {
        return;
    }

    SwContentControlListItem aItem;
    aItem.m_aDisplayText = m_xListItems->get_text(nRow, 0);
    aItem.m_aValue = m_xListItems->get_text(nRow, 1);
    m_xListItems->remove(nRow);
    --nRow;
    m_xListItems->insert_text(nRow, aItem.m_aDisplayText);
    m_xListItems->set_text(nRow, aItem.m_aValue, 1);
    m_xListItems->select(nRow);
}

IMPL_LINK_NOARG(SwContentControlDlg, MoveDownHdl, weld::Button&, void)
{
    int nRow = m_xListItems->get_selected_index();
    int nEndPos = m_xListItems->n_children() - 1;
    if (nRow < 0 || nRow >= nEndPos)
    {
        return;
    }

    SwContentControlListItem aItem;
    aItem.m_aDisplayText = m_xListItems->get_text(nRow, 0);
    aItem.m_aValue = m_xListItems->get_text(nRow, 1);
    m_xListItems->remove(nRow);
    ++nRow;
    m_xListItems->insert_text(nRow, aItem.m_aDisplayText);
    m_xListItems->set_text(nRow, aItem.m_aValue, 1);
    m_xListItems->select(nRow);
}

IMPL_LINK_NOARG(SwContentControlDlg, SelectionChangedHdl, weld::TreeView&, void)
{
    if (!m_xListItems->has_focus())
    {
        return;
    }

    int nRow = m_xListItems->get_selected_index();
    if (nRow < 0)
    {
        m_xRenameBtn->set_sensitive(false);
        m_xDeleteBtn->set_sensitive(false);
    }
    else
    {
        m_xRenameBtn->set_sensitive(true);
        m_xDeleteBtn->set_sensitive(true);
    }

    if (nRow <= 0)
    {
        m_xMoveUpBtn->set_sensitive(false);
    }
    else
    {
        m_xMoveUpBtn->set_sensitive(true);
    }

    int nEndPos = m_xListItems->n_children() - 1;
    if (nRow < 0 || nRow >= nEndPos)
    {
        m_xMoveDownBtn->set_sensitive(false);
    }
    else
    {
        m_xMoveDownBtn->set_sensitive(true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
