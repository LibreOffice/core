/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dropdowncontentcontrolbutton.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <edtwin.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <strings.hrc>
#include <formatcontentcontrol.hxx>

void SwDropDownContentControlButton::InitDropdown()
{
    std::vector<SwContentControlListItem> aListItems = m_rContentControl.GetListItems();

    for (const auto& rListItem : aListItems)
        m_xTreeView->append_text(rListItem.m_aDisplayText);

    if (aListItems.empty())
    {
        m_xTreeView->append_text(SwResId(STR_DROP_DOWN_EMPTY_LIST));
    }

    int nHeight = m_xTreeView->get_height_rows(
        std::min<int>(Application::GetSettings().GetStyleSettings().GetListBoxMaximumLineCount(),
                      m_xTreeView->n_children()));
    m_xTreeView->set_size_request(-1, nHeight);
    Size aSize(m_xTreeView->get_preferred_size());
    aSize.AdjustWidth(4);
    aSize.AdjustHeight(4);
    tools::Long nMinListWidth = GetSizePixel().Width();
    aSize.setWidth(std::max(aSize.Width(), nMinListWidth));
    m_xTreeView->set_size_request(aSize.Width(), aSize.Height());
}

IMPL_LINK(SwDropDownContentControlButton, ListBoxHandler, weld::TreeView&, rBox, bool)
{
    OUString sSelection = rBox.get_selected_text();
    if (sSelection == SwResId(STR_DROP_DOWN_EMPTY_LIST))
    {
        m_xPopup->popdown();
        return true;
    }

    sal_Int32 nSelection = rBox.get_selected_index();
    if (nSelection >= 0)
    {
        // TODO update the doc model
        SwView& rView = static_cast<SwEditWin*>(GetParent())->GetView();
        rView.GetDocShell()->SetModified();
    }

    m_xPopup->popdown();

    return true;
}

SwDropDownContentControlButton::SwDropDownContentControlButton(
    SwEditWin* pEditWin, const SwContentControl& rContentControl)
    : SwContentControlButton(pEditWin, rContentControl)
{
}

SwDropDownContentControlButton::~SwDropDownContentControlButton() { disposeOnce(); }

void SwDropDownContentControlButton::LaunchPopup()
{
    m_xPopupBuilder = Application::CreateBuilder(GetFrameWeld(),
                                                 "modules/swriter/ui/contentcontroldropdown.ui");
    m_xPopup = m_xPopupBuilder->weld_popover("ContentControlDropDown");
    m_xTreeView = m_xPopupBuilder->weld_tree_view("list");
    InitDropdown();
    m_xTreeView->connect_row_activated(LINK(this, SwDropDownContentControlButton, ListBoxHandler));
    SwContentControlButton::LaunchPopup();
    m_xTreeView->grab_focus();
}

void SwDropDownContentControlButton::DestroyPopup()
{
    m_xTreeView.reset();
    SwContentControlButton::DestroyPopup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
