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
#include <svx/cuicharmap.hxx>
#include <svl/numformat.hxx>
#include <svl/zformat.hxx>

#include <wrtsh.hxx>
#include <ndtxt.hxx>
#include <textcontentcontrol.hxx>
#include <IDocumentState.hxx>
#include <swuiexp.hxx>
#include <numfmtlb.hxx>

using namespace com::sun::star;

SwContentControlDlg::SwContentControlDlg(weld::Window* pParent, SwWrtShell& rWrtShell)
    : SfxDialogController(pParent, u"modules/swriter/ui/contentcontroldlg.ui"_ustr,
                          u"ContentControlDialog"_ustr)
    , m_rWrtShell(rWrtShell)
    , m_xShowingPlaceHolderCB(m_xBuilder->weld_check_button(u"showing_place_holder"_ustr))
    , m_xAlias(m_xBuilder->weld_entry(u"aliasentry"_ustr))
    , m_xTag(m_xBuilder->weld_entry(u"tagentry"_ustr))
    , m_xId(m_xBuilder->weld_spin_button(u"idspinbutton"_ustr))
    , m_xTabIndex(m_xBuilder->weld_spin_button(u"tabindexspinbutton"_ustr))
    , m_xCheckboxFrame(m_xBuilder->weld_frame(u"checkboxframe"_ustr))
    , m_xCheckedState(m_xBuilder->weld_entry(u"checkboxcheckedentry"_ustr))
    , m_xCheckedStateBtn(m_xBuilder->weld_button(u"btncheckboxchecked"_ustr))
    , m_xUncheckedState(m_xBuilder->weld_entry(u"checkboxuncheckedentry"_ustr))
    , m_xUncheckedStateBtn(m_xBuilder->weld_button(u"btncheckboxunchecked"_ustr))
    , m_xListItemsFrame(m_xBuilder->weld_frame(u"listitemsframe"_ustr))
    , m_xListItems(m_xBuilder->weld_tree_view(u"listitems"_ustr))
    , m_xListItemButtons(m_xBuilder->weld_box(u"listitembuttons"_ustr))
    , m_xInsertBtn(m_xBuilder->weld_button(u"add"_ustr))
    , m_xRenameBtn(m_xBuilder->weld_button(u"modify"_ustr))
    , m_xDeleteBtn(m_xBuilder->weld_button(u"remove"_ustr))
    , m_xMoveUpBtn(m_xBuilder->weld_button(u"moveup"_ustr))
    , m_xMoveDownBtn(m_xBuilder->weld_button(u"movedown"_ustr))
    , m_xDateFrame(m_xBuilder->weld_frame(u"dateframe"_ustr))
    , m_xDateFormat(
          new SwNumFormatTreeView(m_xBuilder->weld_tree_view(u"date_formats_treeview"_ustr)))
    , m_xOk(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xCheckedStateBtn->connect_clicked(LINK(this, SwContentControlDlg, SelectCharHdl));
    m_xUncheckedStateBtn->connect_clicked(LINK(this, SwContentControlDlg, SelectCharHdl));
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
    SwTextNode* pTextNode = pStart->GetNode().GetTextNode();
    if (!pTextNode)
    {
        return;
    }

    SwTextAttr* pAttr = pTextNode->GetTextAttrAt(
        pStart->GetContentIndex(), RES_TXTATR_CONTENTCONTROL, ::sw::GetTextAttrMode::Parent);
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

    if (!m_pContentControl->GetAlias().isEmpty())
    {
        m_xAlias->set_text(m_pContentControl->GetAlias());
        m_xAlias->save_value();
    }

    if (!m_pContentControl->GetTag().isEmpty())
    {
        m_xTag->set_text(m_pContentControl->GetTag());
        m_xTag->save_value();
    }

    // The ID is supposed to be a unique ID, but it isn't really used for much
    // and in MS Word it (supposedly) is automatically made unique if it is a duplicate.
    // The main purpose for having it here is lookup, not modification,
    // since AFAIK the only use of the ID is for VBA macro name lookup.
    // Since it is used as unsigned in VBA, make the UI display the unsigned values too.
    m_xId->set_range(0, SAL_MAX_UINT32);
    m_xId->set_increments(1, 10);
    const sal_uInt32 nId = static_cast<sal_uInt32>(m_pContentControl->GetId());
    m_xId->set_value(nId);
    // a one-time chance to set the ID - only allow setting it when it is undefined.
    if (nId)
        m_xId->set_editable(false); // still available for copy/paste
    m_xId->save_value();

    // And on the contrary, the tabIndex is stored as unsigned,
    // even though humanly speaking it is much nicer to use -1 to indicate a no tab stop. Oh well.
    m_xTabIndex->set_range(SAL_MIN_INT32, SAL_MAX_INT32);
    m_xTabIndex->set_increments(1, 10);
    const sal_Int32 nTabIndex = static_cast<sal_Int32>(m_pContentControl->GetTabIndex());
    m_xTabIndex->set_value(nTabIndex);
    m_xTabIndex->save_value();

    if (m_pContentControl->GetCheckbox())
    {
        m_xCheckedState->set_text(m_pContentControl->GetCheckedState());
        m_xCheckedState->save_value();
        m_xUncheckedState->set_text(m_pContentControl->GetUncheckedState());
        m_xUncheckedState->save_value();
    }
    else
    {
        m_xCheckboxFrame->set_visible(false);
    }

    if (m_pContentControl->GetComboBox() || m_pContentControl->GetDropDown())
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

    if (m_pContentControl->GetDate())
    {
        m_xDateFormat->SetFormatType(SvNumFormatType::DATE);
        m_xDateFormat->SetShowLanguageControl(true);

        // Set height to double of the default.
        weld::TreeView& rTreeView = dynamic_cast<weld::TreeView&>(m_xDateFormat->get_widget());
        rTreeView.set_size_request(rTreeView.get_preferred_size().Width(),
                                   rTreeView.get_height_rows(10));

        OUString sFormatString = m_pContentControl->GetDateFormat();
        OUString sLang = m_pContentControl->GetDateLanguage();
        if (!sFormatString.isEmpty() && !sLang.isEmpty())
        {
            SvNumberFormatter* pNumberFormatter = m_rWrtShell.GetNumberFormatter();
            LanguageType aLangType = LanguageTag(sLang).getLanguageType();
            sal_uInt32 nFormat = pNumberFormatter->GetEntryKey(sFormatString, aLangType);
            if (nFormat == NUMBERFORMAT_ENTRY_NOT_FOUND)
            {
                sal_Int32 nCheckPos = 0;
                SvNumFormatType nType;
                pNumberFormatter->PutEntry(sFormatString, nCheckPos, nType, nFormat,
                                           LanguageTag(sLang).getLanguageType());
            }

            if (aLangType != LANGUAGE_DONTKNOW && nFormat != NUMBERFORMAT_ENTRY_NOT_FOUND)
            {
                m_xDateFormat->SetDefFormat(nFormat);
            }
        }
    }
    else
    {
        m_xDateFrame->set_visible(false);
    }
}

SwContentControlDlg::~SwContentControlDlg()
{
    if (m_xListItemDialog)
        m_xListItemDialog.disposeAndClear();
}

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

    if (m_xAlias->get_value_changed_from_saved())
    {
        m_pContentControl->SetAlias(m_xAlias->get_text());
        bChanged = true;
    }

    if (m_xTag->get_value_changed_from_saved())
    {
        m_pContentControl->SetTag(m_xTag->get_text());
        bChanged = true;
    }

    if (m_xId->get_value_changed_from_saved())
    {
        m_pContentControl->SetId(o3tl::narrowing<sal_Int32>(m_xId->get_value()));
        bChanged = true;
    }

    if (m_xTabIndex->get_value_changed_from_saved())
    {
        m_pContentControl->SetTabIndex(o3tl::narrowing<sal_uInt32>(m_xTabIndex->get_value()));
        bChanged = true;
    }

    if (m_xCheckedState->get_value_changed_from_saved())
    {
        m_pContentControl->SetCheckedState(m_xCheckedState->get_text());
    }

    if (m_xUncheckedState->get_value_changed_from_saved())
    {
        m_pContentControl->SetUncheckedState(m_xUncheckedState->get_text());
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

    if (m_pContentControl->GetDate())
    {
        SvNumberFormatter* pNumberFormatter = m_rWrtShell.GetNumberFormatter();
        const SvNumberformat* pFormat = pNumberFormatter->GetEntry(m_xDateFormat->GetFormat());
        if (pFormat)
        {
            if (pFormat->GetFormatstring() != m_pContentControl->GetDateFormat())
            {
                m_pContentControl->SetDateFormat(pFormat->GetFormatstring());
                bChanged = true;
            }

            OUString aLanguage = LanguageTag(pFormat->GetLanguage()).getBcp47();
            if (aLanguage != m_pContentControl->GetDateLanguage())
            {
                m_pContentControl->SetDateLanguage(aLanguage);
                bChanged = true;
            }
        }
    }

    if (bChanged)
    {
        m_rWrtShell.GetDoc()->getIDocumentState().SetModified();

        // Make sure that the cursor gets updated with the new list items.
        m_rWrtShell.HideCursor();
        m_rWrtShell.ShowCursor();
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK(SwContentControlDlg, SelectCharHdl, weld::Button&, rButton, void)
{
    SvxCharacterMap aMap(m_xDialog.get(), nullptr, nullptr);
    sal_UCS4 cBullet = 0;
    sal_Int32 nIndex = 0;
    if (&rButton == m_xCheckedStateBtn.get())
    {
        cBullet = m_pContentControl->GetCheckedState().iterateCodePoints(&nIndex);
    }
    else if (&rButton == m_xUncheckedStateBtn.get())
    {
        cBullet = m_pContentControl->GetUncheckedState().iterateCodePoints(&nIndex);
    }
    aMap.SetChar(cBullet);
    if (aMap.run() != RET_OK)
    {
        return;
    }

    cBullet = aMap.GetChar();
    if (&rButton == m_xCheckedStateBtn.get())
    {
        m_xCheckedState->set_text(OUString(&cBullet, 1));
    }
    else if (&rButton == m_xUncheckedStateBtn.get())
    {
        m_xUncheckedState->set_text(OUString(&cBullet, 1));
    }
}

IMPL_LINK_NOARG(SwContentControlDlg, InsertHdl, weld::Button&, void)
{
    std::shared_ptr<SwContentControlListItem> aItem = std::make_shared<SwContentControlListItem>();
    SwAbstractDialogFactory& rFact = swui::GetFactory();
    m_xListItemDialog = rFact.CreateSwContentControlListItemDlg(m_xDialog.get(), *aItem);
    m_xListItemDialog->StartExecuteAsync([this, aItem](sal_Int32 nResult) {
        if (nResult == RET_OK)
        {
            if (aItem->m_aDisplayText.isEmpty() && aItem->m_aValue.isEmpty())
            {
                // Maintain the invariant that value can't be empty.
                return;
            }

            if (aItem->m_aValue.isEmpty())
            {
                aItem->m_aValue = aItem->m_aDisplayText;
            }

            int nRow = m_xListItems->n_children();
            m_xListItems->append_text(aItem->m_aDisplayText);
            m_xListItems->set_text(nRow, aItem->m_aValue, 1);
        }

        m_xListItemDialog.disposeAndClear();
    });
}

IMPL_LINK_NOARG(SwContentControlDlg, RenameHdl, weld::Button&, void)
{
    int nRow = m_xListItems->get_selected_index();
    if (nRow < 0)
    {
        return;
    }

    std::shared_ptr<SwContentControlListItem> aItem = std::make_shared<SwContentControlListItem>();
    aItem->m_aDisplayText = m_xListItems->get_text(nRow, 0);
    aItem->m_aValue = m_xListItems->get_text(nRow, 1);
    SwAbstractDialogFactory& rFact = swui::GetFactory();
    m_xListItemDialog = rFact.CreateSwContentControlListItemDlg(m_xDialog.get(), *aItem);
    m_xListItemDialog->StartExecuteAsync([this, aItem, nRow](sal_Int32 nResult) {
        if (nResult == RET_OK)
        {
            if (aItem->m_aDisplayText.isEmpty() && aItem->m_aValue.isEmpty())
            {
                // Maintain the invariant that value can't be empty.
                return;
            }

            if (aItem->m_aValue.isEmpty())
            {
                aItem->m_aValue = aItem->m_aDisplayText;
            }

            m_xListItems->set_text(nRow, aItem->m_aDisplayText, 0);
            m_xListItems->set_text(nRow, aItem->m_aValue, 1);
        }

        m_xListItemDialog.disposeAndClear();
    });
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
