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
DropDownFormFieldDialog::DropDownFormFieldDialog(vcl::Window* pParent,
                                                 mark::IFieldmark* pDropDownField)
    : SvxStandardDialog(pParent, "DropDownFormFieldDialog",
                        "modules/swriter/ui/dropdownformfielddialog.ui")
    , m_pDropDownField(pDropDownField)
    , m_bListHasChanged(false)
{
    get(m_xListItemEntry, "item_entry");
    get(m_xListAddButton, "add_button");
    get(m_xListItemsTreeView, "items_treeview");
    get(m_xListRemoveButton, "remove_button");
    get(m_xListUpButton, "up_button");
    get(m_xListDownButton, "down_button");

    m_xListItemsTreeView->set_width_request(m_xListItemEntry->get_preferred_size().Width());
    m_xListItemsTreeView->set_height_request(m_xListItemEntry->get_preferred_size().Height() * 5);

    m_xListItemsTreeView->SetSelectHdl(LINK(this, DropDownFormFieldDialog, SelectHdl));

    m_xListItemEntry->SetModifyHdl(LINK(this, DropDownFormFieldDialog, ModifyEditHdl));
    m_xListItemEntry->SetReturnActionLink(LINK(this, DropDownFormFieldDialog, ReturnActionHdl));

    Link<Button*, void> aPushButtonLink(LINK(this, DropDownFormFieldDialog, ButtonPushedHdl));
    m_xListAddButton->SetClickHdl(aPushButtonLink);
    m_xListRemoveButton->SetClickHdl(aPushButtonLink);
    m_xListUpButton->SetClickHdl(aPushButtonLink);
    m_xListDownButton->SetClickHdl(aPushButtonLink);

    InitControls();
}

DropDownFormFieldDialog::~DropDownFormFieldDialog() {}

IMPL_LINK_NOARG(DropDownFormFieldDialog, SelectHdl, ListBox&, void) { UpdateButtons(); }

IMPL_LINK_NOARG(DropDownFormFieldDialog, ModifyEditHdl, Edit&, void) { UpdateButtons(); }

IMPL_LINK_NOARG(DropDownFormFieldDialog, ReturnActionHdl, ReturnActionEdit&, void)
{
    AppendItemToList();
}

IMPL_LINK(DropDownFormFieldDialog, ButtonPushedHdl, Button*, pButton, void)
{
    if (pButton == m_xListAddButton)
    {
        AppendItemToList();
    }
    else if (m_xListItemsTreeView->GetSelectedEntryPos() != LISTBOX_ENTRY_NOTFOUND)
    {
        int nSelPos = m_xListItemsTreeView->GetSelectedEntryPos();
        if (pButton == m_xListRemoveButton)
        {
            m_xListItemsTreeView->RemoveEntry(nSelPos);
            if (m_xListItemsTreeView->GetEntryCount() > 0)
                m_xListItemsTreeView->SelectEntryPos(nSelPos > 0 ? nSelPos - 1 : 0);
        }
        else if (pButton == m_xListUpButton)
        {
            const OUString sEntry = m_xListItemsTreeView->GetSelectedEntry();
            m_xListItemsTreeView->RemoveEntry(nSelPos);
            nSelPos--;
            m_xListItemsTreeView->InsertEntry(sEntry, nSelPos);
            m_xListItemsTreeView->SelectEntryPos(nSelPos);
        }
        else if (pButton == m_xListDownButton)
        {
            const OUString sEntry = m_xListItemsTreeView->GetSelectedEntry();
            m_xListItemsTreeView->RemoveEntry(nSelPos);
            nSelPos++;
            m_xListItemsTreeView->InsertEntry(sEntry, nSelPos);
            m_xListItemsTreeView->SelectEntryPos(nSelPos);
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
                m_xListItemsTreeView->InsertEntry(rItem);

            // Select the current one
            auto pResult = pParameters->find(ODF_FORMDROPDOWN_RESULT);
            if (pResult != pParameters->end())
            {
                sal_Int32 nSelection = -1;
                pResult->second >>= nSelection;
                if (vListEntries.getLength() > nSelection)
                    m_xListItemsTreeView->SelectEntry(vListEntries[nSelection]);
            }
        }
    }
    UpdateButtons();
}

void DropDownFormFieldDialog::AppendItemToList()
{
    if (m_xListAddButton->IsEnabled())
    {
        const OUString sEntry(m_xListItemEntry->GetText());
        if (!sEntry.isEmpty())
        {
            m_xListItemsTreeView->InsertEntry(sEntry);
            m_xListItemsTreeView->SelectEntry(sEntry);
            m_bListHasChanged = true;

            // Clear entry
            m_xListItemEntry->SetText(OUString());
            m_xListItemEntry->GrabFocus();
        }
        UpdateButtons();
    }
}

void DropDownFormFieldDialog::UpdateButtons()
{
    m_xListAddButton->Enable(
        !m_xListItemEntry->GetText().isEmpty()
        && (LISTBOX_ENTRY_NOTFOUND
            == m_xListItemsTreeView->GetEntryPos(m_xListItemEntry->GetText())));

    int nSelPos = m_xListItemsTreeView->GetSelectedEntryPos();
    m_xListRemoveButton->Enable(nSelPos != LISTBOX_ENTRY_NOTFOUND);
    m_xListUpButton->Enable(nSelPos != LISTBOX_ENTRY_NOTFOUND && nSelPos != 0);
    m_xListDownButton->Enable(nSelPos != LISTBOX_ENTRY_NOTFOUND
                              && nSelPos < m_xListItemsTreeView->GetEntryCount() - 1);
}

void DropDownFormFieldDialog::Apply()
{
    if (m_pDropDownField != nullptr && m_bListHasChanged)
    {
        mark::IFieldmark::parameter_map_t* pParameters = m_pDropDownField->GetParameters();

        css::uno::Sequence<OUString> vListEntries(m_xListItemsTreeView->GetEntryCount());
        for (int nIndex = 0; nIndex < m_xListItemsTreeView->GetEntryCount(); ++nIndex)
        {
            vListEntries[nIndex] = m_xListItemsTreeView->GetEntry(nIndex);
        }

        if (vListEntries.getLength() != 0)
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
