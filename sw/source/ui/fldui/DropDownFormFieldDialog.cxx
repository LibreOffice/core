/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DropDownFormFieldDialog.hxx>
#include <vcl/event.hxx>
#include <IMark.hxx>
#include <xmloff/odffields.hxx>

namespace sw
{
DropDownFormFieldDialog::DropDownFormFieldDialog(weld::Window* pParent,
                                                 mark::IFieldmark* pDropDownField)
    : GenericDialogController(pParent, "modules/swriter/ui/dropdownformfielddialog.ui",
                              "DropDownFormFieldDialog")
    , m_pDropDownField(pDropDownField)
    , m_bListHasChanged(false)
    , m_xListItemEntry(m_xBuilder->weld_entry("item_entry"))
    , m_xListAddButton(m_xBuilder->weld_button("add_button"))
    , m_xListItemsTreeView(m_xBuilder->weld_tree_view("items_treeview"))
    , m_xListRemoveButton(m_xBuilder->weld_button("remove_button"))
    , m_xListUpButton(m_xBuilder->weld_button("up_button"))
    , m_xListDownButton(m_xBuilder->weld_button("down_button"))
{
    m_xListItemEntry->connect_key_press(LINK(this, DropDownFormFieldDialog, KeyPressedHdl));
    m_xListItemEntry->connect_changed(LINK(this, DropDownFormFieldDialog, EntryChangedHdl));

    m_xListItemsTreeView->set_size_request(m_xListItemEntry->get_preferred_size().Width(),
                                           m_xListItemEntry->get_preferred_size().Height() * 5);
    m_xListItemsTreeView->connect_changed(LINK(this, DropDownFormFieldDialog, ListChangedHdl));

    Link<weld::Button&, void> aPushButtonLink(LINK(this, DropDownFormFieldDialog, ButtonPushedHdl));
    m_xListAddButton->connect_clicked(aPushButtonLink);
    m_xListRemoveButton->connect_clicked(aPushButtonLink);
    m_xListUpButton->connect_clicked(aPushButtonLink);
    m_xListDownButton->connect_clicked(aPushButtonLink);

    InitControls();
}

DropDownFormFieldDialog::~DropDownFormFieldDialog() {}

IMPL_LINK_NOARG(DropDownFormFieldDialog, ListChangedHdl, weld::TreeView&, void) { UpdateButtons(); }

IMPL_LINK(DropDownFormFieldDialog, KeyPressedHdl, const KeyEvent&, rEvent, bool)
{
    if (rEvent.GetKeyCode().GetCode() == KEY_RETURN && !m_xListItemEntry->get_text().isEmpty())
    {
        AppendItemToList();
        return true;
    }
    return false;
}

IMPL_LINK_NOARG(DropDownFormFieldDialog, EntryChangedHdl, weld::Entry&, void) { UpdateButtons(); }

IMPL_LINK(DropDownFormFieldDialog, ButtonPushedHdl, weld::Button&, rButton, void)
{
    if (&rButton == m_xListAddButton.get())
    {
        AppendItemToList();
    }
    else if (m_xListItemsTreeView->get_selected_index() != -1)
    {
        int nSelPos = m_xListItemsTreeView->get_selected_index();
        if (&rButton == m_xListRemoveButton.get())
        {
            m_xListItemsTreeView->remove(nSelPos);
            if (m_xListItemsTreeView->n_children() > 0)
                m_xListItemsTreeView->select(nSelPos ? nSelPos - 1 : 0);
        }
        else if (&rButton == m_xListUpButton.get())
        {
            const OUString sEntry = m_xListItemsTreeView->get_selected_text();
            m_xListItemsTreeView->remove(nSelPos);
            nSelPos--;
            m_xListItemsTreeView->insert_text(nSelPos, sEntry);
            m_xListItemsTreeView->select(nSelPos);
        }
        else if (&rButton == m_xListDownButton.get())
        {
            const OUString sEntry = m_xListItemsTreeView->get_selected_text();
            m_xListItemsTreeView->remove(nSelPos);
            nSelPos++;
            m_xListItemsTreeView->insert_text(nSelPos, sEntry);
            m_xListItemsTreeView->select(nSelPos);
        }
        m_bListHasChanged = true;
    }
    UpdateButtons();
}

void DropDownFormFieldDialog::InitControls()
{
    if (m_pDropDownField != nullptr)
    {
        const mark::IFieldmark::parameter_map_t* const pParameters
            = m_pDropDownField->GetParameters();

        auto pListEntries = pParameters->find(ODF_FORMDROPDOWN_LISTENTRY);
        if (pListEntries != pParameters->end())
        {
            css::uno::Sequence<OUString> vListEntries;
            pListEntries->second >>= vListEntries;
            for (const OUString& rItem : vListEntries)
                m_xListItemsTreeView->append_text(rItem);

            // Select the current one
            auto pResult = pParameters->find(ODF_FORMDROPDOWN_RESULT);
            if (pResult != pParameters->end())
            {
                sal_Int32 nSelection = -1;
                pResult->second >>= nSelection;
                if (vListEntries.getLength() > nSelection)
                    m_xListItemsTreeView->select_text(vListEntries[nSelection]);
            }
        }
    }
    UpdateButtons();
}

void DropDownFormFieldDialog::AppendItemToList()
{
    if (m_xListAddButton->get_sensitive())
    {
        const OUString sEntry(m_xListItemEntry->get_text());
        if (!sEntry.isEmpty())
        {
            m_xListItemsTreeView->append_text(sEntry);
            m_xListItemsTreeView->select_text(sEntry);
            m_bListHasChanged = true;

            // Clear entry
            m_xListItemEntry->set_text(OUString());
            m_xListItemEntry->grab_focus();
        }
        UpdateButtons();
    }
}

void DropDownFormFieldDialog::UpdateButtons()
{
    m_xListAddButton->set_sensitive(!m_xListItemEntry->get_text().isEmpty()
                                    && m_xListItemsTreeView->find_text(m_xListItemEntry->get_text())
                                           == -1);

    int nSelPos = m_xListItemsTreeView->get_selected_index();
    m_xListRemoveButton->set_sensitive(nSelPos != -1);
    m_xListUpButton->set_sensitive(nSelPos > 0);
    m_xListDownButton->set_sensitive(nSelPos != -1
                                     && nSelPos < m_xListItemsTreeView->n_children() - 1);
}

void DropDownFormFieldDialog::Apply()
{
    if (m_pDropDownField != nullptr && m_bListHasChanged)
    {
        mark::IFieldmark::parameter_map_t* pParameters = m_pDropDownField->GetParameters();

        css::uno::Sequence<OUString> vListEntries(m_xListItemsTreeView->n_children());
        for (int nIndex = 0; nIndex < m_xListItemsTreeView->n_children(); ++nIndex)
        {
            vListEntries[nIndex] = m_xListItemsTreeView->get_text(nIndex);
        }

        if (m_xListItemsTreeView->n_children() != 0)
        {
            (*pParameters)[ODF_FORMDROPDOWN_LISTENTRY] <<= vListEntries;
        }
        else
        {
            pParameters->erase(ODF_FORMDROPDOWN_LISTENTRY);
        }

        // After editing the drop down field's list we don't specify the selected item
        pParameters->erase(ODF_FORMDROPDOWN_RESULT);
    }
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
