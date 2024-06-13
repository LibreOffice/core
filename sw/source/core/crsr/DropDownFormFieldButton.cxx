/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DropDownFormFieldButton.hxx>
#include <edtwin.hxx>
#include <bookmark.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <xmloff/odffields.hxx>
#include <IMark.hxx>
#include <view.hxx>
#include <docsh.hxx>
#include <strings.hrc>

/**
 * Popup dialog for drop-down form field showing the list items of the field.
 * The user can select the item using this popup while filling in a form.
 */

void DropDownFormFieldButton::InitDropdown()
{
    const sw::mark::IFieldmark::parameter_map_t* const pParameters = m_rFieldmark.GetParameters();

    sw::mark::IFieldmark::parameter_map_t::const_iterator pListEntries
        = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
    css::uno::Sequence<OUString> vListEntries;
    if (pListEntries != pParameters->end())
    {
        pListEntries->second >>= vListEntries;
        for (OUString const& i : vListEntries)
            m_xTreeView->append_text(i);
    }

    if (!vListEntries.hasElements())
    {
        m_xTreeView->append_text(SwResId(STR_DROP_DOWN_EMPTY_LIST));
    }

    // Select the current one
    sw::mark::IFieldmark::parameter_map_t::const_iterator pResult
        = pParameters->find(ODF_FORMDROPDOWN_RESULT);
    if (pResult != pParameters->end())
    {
        sal_Int32 nSelection = -1;
        pResult->second >>= nSelection;
        m_xTreeView->set_cursor(nSelection);
        m_xTreeView->select(nSelection);
    }

    auto nHeight = m_xTreeView->get_height_rows(
        std::min<int>(Application::GetSettings().GetStyleSettings().GetListBoxMaximumLineCount(),
                      m_xTreeView->n_children()));
    m_xTreeView->set_size_request(-1, nHeight);
    Size lbSize(m_xTreeView->get_preferred_size());
    lbSize.AdjustWidth(4);
    lbSize.AdjustHeight(4);
    auto nMinListWidth = GetSizePixel().Width();
    lbSize.setWidth(std::max(lbSize.Width(), nMinListWidth));
    m_xTreeView->set_size_request(lbSize.Width(), lbSize.Height());
}

IMPL_LINK(DropDownFormFieldButton, MyListBoxHandler, weld::TreeView&, rBox, bool)
{
    OUString sSelection = rBox.get_selected_text();
    if (sSelection == SwResId(STR_DROP_DOWN_EMPTY_LIST))
    {
        m_xFieldPopup->popdown();
        return true;
    }

    sal_Int32 nSelection = rBox.get_selected_index();
    if (nSelection >= 0)
    {
        (*m_rFieldmark.GetParameters())[ODF_FORMDROPDOWN_RESULT] <<= nSelection;
        m_rFieldmark.Invalidate();
        SwView& rView = static_cast<SwEditWin*>(GetParent())->GetView();
        rView.GetDocShell()->SetModified();
    }

    m_xFieldPopup->popdown();

    return true;
}

DropDownFormFieldButton::DropDownFormFieldButton(SwEditWin* pEditWin,
                                                 sw::mark::DropDownFieldmark& rFieldmark)
    : FormFieldButton(pEditWin, rFieldmark)
{
}

DropDownFormFieldButton::~DropDownFormFieldButton() { disposeOnce(); }

void DropDownFormFieldButton::LaunchPopup()
{
    m_xFieldPopupBuilder
        = Application::CreateBuilder(GetFrameWeld(), u"modules/swriter/ui/formdropdown.ui"_ustr);
    m_xFieldPopup = m_xFieldPopupBuilder->weld_popover(u"FormDropDown"_ustr);
    m_xTreeView = m_xFieldPopupBuilder->weld_tree_view(u"list"_ustr);
    InitDropdown();
    m_xTreeView->connect_row_activated(LINK(this, DropDownFormFieldButton, MyListBoxHandler));
    FormFieldButton::LaunchPopup();
    m_xTreeView->grab_focus();
}

void DropDownFormFieldButton::DestroyPopup()
{
    m_xTreeView.reset();
    FormFieldButton::DestroyPopup();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
