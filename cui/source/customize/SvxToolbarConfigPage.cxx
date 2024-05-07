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

#include <sal/config.h>
#include <sal/log.hxx>

#include <vcl/event.hxx>
#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <vcl/commandevent.hxx>

#include <sfx2/sfxsids.hrc>
#include <svl/stritem.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <algorithm>
#include <helpids.h>
#include <strings.hrc>

#include <cfg.hxx>
#include <SvxToolbarConfigPage.hxx>
#include <SvxConfigPageHelper.hxx>
#include <dialmgr.hxx>

#include <dlgname.hxx>
#include <comphelper/processfactory.hxx>

SvxToolbarConfigPage::SvxToolbarConfigPage(weld::Container* pPage,
                                           weld::DialogController* pController,
                                           const SfxItemSet& rSet)
    : SvxConfigPage(pPage, pController, rSet)
{
    m_xGearBtn = m_xBuilder->weld_menu_button(u"toolbargearbtn"_ustr);
    m_xGearBtn->show();
    m_xContainer->set_help_id(HID_SVX_CONFIG_TOOLBAR);

    m_xContentsListBox.reset(
        new SvxToolbarEntriesListBox(m_xBuilder->weld_tree_view(u"toolcontents"_ustr), this));
    m_xDropTargetHelper.reset(
        new SvxConfigPageFunctionDropTarget(*this, m_xContentsListBox->get_widget()));

    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();
    Size aSize(m_xFunctions->get_size_request());
    rTreeView.set_size_request(aSize.Width(), aSize.Height());

    rTreeView.set_hexpand(true);
    rTreeView.set_vexpand(true);
    rTreeView.set_help_id(HID_SVX_CONFIG_TOOLBAR_CONTENTS);
    rTreeView.show();

    rTreeView.connect_changed(LINK(this, SvxToolbarConfigPage, SelectToolbarEntry));
    rTreeView.connect_popup_menu(LINK(this, SvxToolbarConfigPage, ContentContextMenuHdl));

    m_xFunctions->get_widget().connect_popup_menu(
        LINK(this, SvxToolbarConfigPage, FunctionContextMenuHdl));

    m_xTopLevelListBox->set_help_id(HID_SVX_TOPLEVELLISTBOX);
    m_xSaveInListBox->set_help_id(HID_SVX_SAVE_IN);
    m_xMoveUpButton->set_help_id(HID_SVX_UP_TOOLBAR_ITEM);
    m_xMoveDownButton->set_help_id(HID_SVX_DOWN_TOOLBAR_ITEM);
    m_xDescriptionField->set_help_id(HID_SVX_DESCFIELD);

    m_xCommandCategoryListBox->connect_changed(LINK(this, SvxToolbarConfigPage, SelectCategory));

    m_xGearBtn->connect_selected(LINK(this, SvxToolbarConfigPage, GearHdl));

    m_xMoveUpButton->connect_clicked(LINK(this, SvxToolbarConfigPage, MoveHdl));
    m_xMoveDownButton->connect_clicked(LINK(this, SvxToolbarConfigPage, MoveHdl));
    // Always enable Up and Down buttons
    // added for issue i53677 by shizhoubo
    m_xMoveDownButton->set_sensitive(true);
    m_xMoveUpButton->set_sensitive(true);

    m_xAddCommandButton->connect_clicked(LINK(this, SvxToolbarConfigPage, AddCommandHdl));
    m_xRemoveCommandButton->connect_clicked(LINK(this, SvxToolbarConfigPage, RemoveCommandHdl));

    m_xInsertBtn->connect_selected(LINK(this, SvxToolbarConfigPage, InsertHdl));
    m_xModifyBtn->connect_selected(LINK(this, SvxToolbarConfigPage, ModifyItemHdl));
    m_xResetBtn->connect_clicked(LINK(this, SvxToolbarConfigPage, ResetToolbarHdl));

    // "Insert Submenu" is irrelevant to the toolbars
    m_xInsertBtn->remove_item(u"insertsubmenu"_ustr);

    // Gear menu's "Move" action is irrelevant to the toolbars
    m_xGearBtn->set_item_sensitive(u"toolbar_gear_move"_ustr, false);

    // default toolbar to select is standardbar unless a different one
    // has been passed in
    m_aURLToSelect = ITEM_TOOLBAR_URL;
    m_aURLToSelect += "standardbar";

    const SfxPoolItem* pItem = rSet.GetItem(SID_CONFIG);

    if (pItem)
    {
        OUString text = static_cast<const SfxStringItem*>(pItem)->GetValue();
        if (text.startsWith(ITEM_TOOLBAR_URL))
        {
            m_aURLToSelect = text.copy(0);
        }
    }
}

void SvxToolbarConfigPage::ListModified()
{
    // regenerate with the current ordering within the list
    SvxEntries* pEntries = GetTopLevelSelection()->GetEntries();
    pEntries->clear();

    for (int i = 0; i < m_xContentsListBox->n_children(); ++i)
        pEntries->push_back(weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(i)));

    GetSaveInData()->SetModified();
    GetTopLevelSelection()->SetModified();

    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    if (pToolbar)
        static_cast<ToolbarSaveInData*>(GetSaveInData())->ApplyToolbar(pToolbar);
}

SvxToolbarConfigPage::~SvxToolbarConfigPage()
{
    for (int i = 0, nCount = m_xSaveInListBox->get_count(); i < nCount; ++i)
    {
        ToolbarSaveInData* pData = weld::fromId<ToolbarSaveInData*>(m_xSaveInListBox->get_id(i));
        delete pData;
    }
    m_xSaveInListBox->clear();
}

void SvxToolbarConfigPage::DeleteSelectedTopLevel()
{
    const sal_Int32 nSelectionPos = m_xTopLevelListBox->get_active();
    ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>(GetSaveInData());
    pSaveInData->RemoveToolbar(GetTopLevelSelection());

    int nCount = m_xTopLevelListBox->get_count();
    if (nCount > 1)
    {
        // select next entry after the one being deleted
        // selection position is indexed from 0 so need to
        // subtract one from the entry count
        if (nSelectionPos != nCount - 1)
        {
            m_xTopLevelListBox->set_active(nSelectionPos + 1);
        }
        else
        {
            m_xTopLevelListBox->set_active(nSelectionPos - 1);
        }
        SelectElement();

        // and now remove the entry
        m_xTopLevelListBox->remove(nSelectionPos);
    }
    else
    {
        ReloadTopLevelListBox();
    }
}

void SvxToolbarConfigPage::DeleteSelectedContent()
{
    int nActEntry = m_xContentsListBox->get_selected_index();

    if (nActEntry == -1)
        return;

    // get currently selected entry
    SvxConfigEntry* pEntry = weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(nActEntry));

    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    // remove entry from the list for this toolbar
    SvxConfigPageHelper::RemoveEntry(pToolbar->GetEntries(), pEntry);

    // remove toolbar entry from UI
    m_xContentsListBox->remove(nActEntry);

    // delete data for toolbar entry
    delete pEntry;

    static_cast<ToolbarSaveInData*>(GetSaveInData())->ApplyToolbar(pToolbar);
    UpdateButtonStates();

    // if this is the last entry in the toolbar and it is a user
    // defined toolbar pop up a dialog asking the user if they
    // want to delete the toolbar
    if (m_xContentsListBox->n_children() == 0 && GetTopLevelSelection()->IsDeletable())
    {
        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
            GetFrameWeld(), VclMessageType::Question, VclButtonsType::YesNo,
            CuiResId(RID_SXVSTR_CONFIRM_DELETE_TOOLBAR)));
        if (xQueryBox->run() == RET_YES)
        {
            DeleteSelectedTopLevel();
        }
    }
}

IMPL_LINK(SvxToolbarConfigPage, MoveHdl, weld::Button&, rButton, void)
{
    MoveEntry(&rButton == m_xMoveUpButton.get());
}

void SvxToolbarConfigPage::MoveEntry(bool bMoveUp)
{
    SvxConfigPage::MoveEntry(bMoveUp);

    // Apply change to currently selected toolbar
    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    if (pToolbar)
        static_cast<ToolbarSaveInData*>(GetSaveInData())->ApplyToolbar(pToolbar);
    else
    {
        SAL_WARN("cui.customize", "SvxToolbarConfigPage::MoveEntry(): no entry");
        UpdateButtonStates();
    }
}

void SvxToolbarConfigPage::Init()
{
    // ensure that the UI is cleared before populating it
    m_xTopLevelListBox->clear();
    m_xContentsListBox->clear();

    ReloadTopLevelListBox();

    sal_Int32 nCount = m_xTopLevelListBox->get_count();
    sal_Int32 nPos = nCount > 0 ? 0 : -1;

    if (!m_aURLToSelect.isEmpty())
    {
        for (sal_Int32 i = 0; i < nCount; ++i)
        {
            SvxConfigEntry* pData = weld::fromId<SvxConfigEntry*>(m_xTopLevelListBox->get_id(i));

            if (pData->GetCommand().equals(m_aURLToSelect))
            {
                nPos = i;
                break;
            }
        }

        // in future select the default toolbar: Standard
        m_aURLToSelect = ITEM_TOOLBAR_URL;
        m_aURLToSelect += "standardbar";
    }

    m_xTopLevelListBox->set_active(nPos);
    SelectElement();

    m_xCommandCategoryListBox->Init(comphelper::getProcessComponentContext(), m_xFrame,
                                    m_aModuleId);
    m_xCommandCategoryListBox->categorySelected(m_xFunctions.get(), OUString(), GetSaveInData());
    SelectFunctionHdl(m_xFunctions->get_widget());
}

SaveInData* SvxToolbarConfigPage::CreateSaveInData(
    const css::uno::Reference<css::ui::XUIConfigurationManager>& xCfgMgr,
    const css::uno::Reference<css::ui::XUIConfigurationManager>& xParentCfgMgr,
    const OUString& aModuleId, bool bDocConfig)
{
    return static_cast<SaveInData*>(
        new ToolbarSaveInData(xCfgMgr, xParentCfgMgr, aModuleId, bDocConfig));
}

IMPL_LINK_NOARG(SvxToolbarConfigPage, SelectToolbarEntry, weld::TreeView&, void)
{
    UpdateButtonStates();
}

IMPL_LINK(SvxToolbarConfigPage, GearHdl, const OUString&, rIdent, void)
{
    SvxConfigEntry* pCurrentToolbar = GetTopLevelSelection();

    if (rIdent == "toolbar_gear_add")
    {
        OUString prefix = CuiResId(RID_CUISTR_NEW_TOOLBAR);

        OUString aNewName
            = SvxConfigPageHelper::generateCustomName(prefix, GetSaveInData()->GetEntries());

        OUString aNewURL = SvxConfigPageHelper::generateCustomURL(GetSaveInData()->GetEntries());

        SvxNewToolbarDialog aNameDialog(GetFrameWeld(), aNewName);

        // Reflect the actual m_xSaveInListBox into the new toolbar dialog
        for (int i = 0, nCount = m_xSaveInListBox->get_count(); i < nCount; ++i)
            aNameDialog.m_xSaveInListBox->append_text(m_xSaveInListBox->get_text(i));

        aNameDialog.m_xSaveInListBox->set_active(m_xSaveInListBox->get_active());

        if (aNameDialog.run() == RET_OK)
        {
            aNewName = aNameDialog.GetName();

            // Where to save the new toolbar? (i.e. Modulewise or documentwise)
            int nInsertPos = aNameDialog.m_xSaveInListBox->get_active();

            ToolbarSaveInData* pData
                = weld::fromId<ToolbarSaveInData*>(m_xSaveInListBox->get_id(nInsertPos));

            if (GetSaveInData() != pData)
            {
                m_xSaveInListBox->set_active(nInsertPos);
                SelectSaveInLocation(*m_xSaveInListBox);
            }

            SvxConfigEntry* pToolbar = new SvxConfigEntry(aNewName, aNewURL, true, false);

            pToolbar->SetUserDefined();
            pToolbar->SetMain();

            pData->CreateToolbar(pToolbar);

            OUString sId(weld::toId(pToolbar));
            m_xTopLevelListBox->append(sId, pToolbar->GetName());
            m_xTopLevelListBox->set_active_id(sId);
            SelectElement();

            pData->SetModified();
        }
    }
    else if (rIdent == "toolbar_gear_delete")
    {
        if (pCurrentToolbar && pCurrentToolbar->IsDeletable())
        {
            DeleteSelectedTopLevel();
            UpdateButtonStates();
        }
    }
    else if (rIdent == "toolbar_gear_rename")
    {
        sal_Int32 nSelectionPos = m_xTopLevelListBox->get_active();
        SvxConfigEntry* pToolbar
            = weld::fromId<SvxConfigEntry*>(m_xTopLevelListBox->get_id(nSelectionPos));
        ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>(GetSaveInData());

        //Rename the toolbar
        OUString sCurrentName(SvxConfigPageHelper::stripHotKey(pToolbar->GetName()));
        OUString sDesc = CuiResId(RID_CUISTR_LABEL_NEW_NAME);

        SvxNameDialog aNameDialog(GetFrameWeld(), sCurrentName, sDesc);
        aNameDialog.set_help_id(HID_SVX_CONFIG_RENAME_TOOLBAR);
        aNameDialog.set_title(CuiResId(RID_CUISTR_RENAME_TOOLBAR));

        if (aNameDialog.run() == RET_OK)
        {
            OUString sNewName = aNameDialog.GetName();

            if (sCurrentName == sNewName)
                return;

            pToolbar->SetName(sNewName);
            pSaveInData->ApplyToolbar(pToolbar);

            // have to use remove and insert to change the name
            m_xTopLevelListBox->remove(nSelectionPos);
            OUString sId(weld::toId(pToolbar));
            m_xTopLevelListBox->insert(nSelectionPos, sNewName, &sId, nullptr, nullptr);
            m_xTopLevelListBox->set_active_id(sId);
        }
    }
    else if (rIdent == "toolbar_gear_iconOnly" || rIdent == "toolbar_gear_textOnly"
             || rIdent == "toolbar_gear_iconAndText")
    {
        ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>(GetSaveInData());

        if (pCurrentToolbar == nullptr || pSaveInData == nullptr)
        {
            SAL_WARN("cui.customize", "NULL toolbar or savein data");
            return;
        }

        sal_Int32 nStyle = 0;
        if (rIdent == "toolbar_gear_iconOnly")
            nStyle = 0;
        else if (rIdent == "toolbar_gear_textOnly")
            nStyle = 1;
        else if (rIdent == "toolbar_gear_iconAndText")
            nStyle = 2;

        pCurrentToolbar->SetStyle(nStyle);
        pSaveInData->SetSystemStyle(m_xFrame, pCurrentToolbar->GetCommand(), nStyle);

        SelectElement();
    }
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown gear menu option: " << rIdent);
        return;
    }
}

IMPL_LINK_NOARG(SvxToolbarConfigPage, SelectCategory, weld::ComboBox&, void)
{
    OUString aSearchTerm(m_xSearchEdit->get_text());

    m_xCommandCategoryListBox->categorySelected(m_xFunctions.get(), aSearchTerm, GetSaveInData());

    SelectFunctionHdl(m_xFunctions->get_widget());
}

IMPL_LINK_NOARG(SvxToolbarConfigPage, AddCommandHdl, weld::Button&, void) { AddFunction(); }

IMPL_LINK_NOARG(SvxToolbarConfigPage, RemoveCommandHdl, weld::Button&, void)
{
    DeleteSelectedContent();
}

IMPL_LINK(SvxToolbarConfigPage, InsertHdl, const OUString&, rIdent, void)
{
    if (rIdent == "insertseparator")
    {
        // Get the currently selected toolbar
        SvxConfigEntry* pToolbar = GetTopLevelSelection();

        SvxConfigEntry* pNewEntryData = new SvxConfigEntry;
        pNewEntryData->SetUserDefined();

        int nPos = AppendEntry(pNewEntryData, -1);
        InsertEntryIntoUI(pNewEntryData, m_xContentsListBox->get_widget(), nPos);

        static_cast<ToolbarSaveInData*>(GetSaveInData())->ApplyToolbar(pToolbar);

        UpdateButtonStates();
    }
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown insert option: " << rIdent);
        return;
    }
}

IMPL_LINK(SvxToolbarConfigPage, ModifyItemHdl, const OUString&, rIdent, void)
{
    bool bNeedsApply = false;

    // get currently selected toolbar
    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    if (rIdent.isEmpty() || pToolbar == nullptr)
    {
        SAL_WARN("cui.customize", "No toolbar selected, or empty rIdent!");
        return;
    }

    if (rIdent == "renameItem")
    {
        int nActEntry = m_xContentsListBox->get_selected_index();
        SvxConfigEntry* pEntry
            = weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(nActEntry));

        OUString aNewName(SvxConfigPageHelper::stripHotKey(pEntry->GetName()));
        OUString aDesc = CuiResId(RID_CUISTR_LABEL_NEW_NAME);

        SvxNameDialog aNameDialog(GetFrameWeld(), aNewName, aDesc);
        aNameDialog.set_help_id(HID_SVX_CONFIG_RENAME_TOOLBAR_ITEM);
        aNameDialog.set_title(CuiResId(RID_CUISTR_RENAME_TOOLBAR));

        if (aNameDialog.run() == RET_OK)
        {
            aNewName = aNameDialog.GetName();

            if (aNewName.isEmpty()) // tdf#80758 - Accelerator character ("~") is passed as
                pEntry->SetName(u"~"_ustr); // the button name in case of empty values.
            else
                pEntry->SetName(aNewName);

            m_xContentsListBox->set_text(nActEntry, aNewName, 0);
            bNeedsApply = true;
        }
    }
    else if (rIdent == "changeIcon")
    {
        int nActEntry = m_xContentsListBox->get_selected_index();
        SvxConfigEntry* pEntry
            = weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(nActEntry));

        SvxIconSelectorDialog aIconDialog(GetFrameWeld(), GetSaveInData()->GetImageManager(),
                                          GetSaveInData()->GetParentImageManager());

        if (aIconDialog.run() == RET_OK)
        {
            css::uno::Reference<css::graphic::XGraphic> newgraphic = aIconDialog.GetSelectedIcon();

            if (newgraphic.is())
            {
                css::uno::Sequence<OUString> aURLSeq{ pEntry->GetCommand() };

                if (!pEntry->GetBackupGraphic().is())
                {
                    css::uno::Reference<css::graphic::XGraphic> backup
                        = SvxConfigPageHelper::GetGraphic(GetSaveInData()->GetImageManager(),
                                                          aURLSeq[0]);

                    if (backup.is())
                    {
                        pEntry->SetBackupGraphic(backup);
                    }
                }

                css::uno::Sequence<css::uno::Reference<css::graphic::XGraphic>> aGraphicSeq{
                    newgraphic
                };
                try
                {
                    GetSaveInData()->GetImageManager()->replaceImages(
                        SvxConfigPageHelper::GetImageType(), aURLSeq, aGraphicSeq);

                    m_xContentsListBox->remove(nActEntry);

                    OUString sId(weld::toId(pEntry));
                    m_xContentsListBox->insert(nActEntry, sId);
                    m_xContentsListBox->set_toggle(nActEntry, pEntry->IsVisible() ? TRISTATE_TRUE
                                                                                  : TRISTATE_FALSE);
                    InsertEntryIntoUI(pEntry, m_xContentsListBox->get_widget(), nActEntry);

                    m_xContentsListBox->select(nActEntry);
                    m_xContentsListBox->scroll_to_row(nActEntry);

                    GetSaveInData()->PersistChanges(GetSaveInData()->GetImageManager());
                }
                catch (const css::uno::Exception&)
                {
                    TOOLS_WARN_EXCEPTION("cui.customize", "Error replacing image");
                }
            }
        }
    }
    else if (rIdent == "resetIcon")
    {
        int nActEntry = m_xContentsListBox->get_selected_index();
        SvxConfigEntry* pEntry
            = weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(nActEntry));

        css::uno::Sequence<css::uno::Reference<css::graphic::XGraphic>> aGraphicSeq{
            pEntry->GetBackupGraphic()
        };

        css::uno::Sequence<OUString> aURLSeq{ pEntry->GetCommand() };

        try
        {
            GetSaveInData()->GetImageManager()->replaceImages(SvxConfigPageHelper::GetImageType(),
                                                              aURLSeq, aGraphicSeq);

            m_xContentsListBox->remove(nActEntry);

            OUString sId(weld::toId(pEntry));
            m_xContentsListBox->insert(nActEntry, sId);
            m_xContentsListBox->set_toggle(nActEntry,
                                           pEntry->IsVisible() ? TRISTATE_TRUE : TRISTATE_FALSE);
            InsertEntryIntoUI(pEntry, m_xContentsListBox->get_widget(), nActEntry);

            m_xContentsListBox->select(nActEntry);
            m_xContentsListBox->scroll_to_row(nActEntry);

            // reset backup in entry
            pEntry->SetBackupGraphic(css::uno::Reference<css::graphic::XGraphic>());

            GetSaveInData()->PersistChanges(GetSaveInData()->GetImageManager());
        }
        catch (const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("cui.customize", "Error resetting image");
        }
    }
    else if (rIdent == "restoreItem")
    {
        int nActEntry = m_xContentsListBox->get_selected_index();
        SvxConfigEntry* pEntry
            = weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(nActEntry));

        ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>(GetSaveInData());

        OUString aSystemName = pSaveInData->GetSystemUIName(pEntry->GetCommand());

        if (!pEntry->GetName().equals(aSystemName))
        {
            pEntry->SetName(aSystemName);
            m_xContentsListBox->set_text(nActEntry, SvxConfigPageHelper::stripHotKey(aSystemName),
                                         0);
            bNeedsApply = true;
        }

        css::uno::Sequence<OUString> aURLSeq{ pEntry->GetCommand() };

        try
        {
            GetSaveInData()->GetImageManager()->removeImages(SvxConfigPageHelper::GetImageType(),
                                                             aURLSeq);

            // reset backup in entry
            pEntry->SetBackupGraphic(css::uno::Reference<css::graphic::XGraphic>());

            GetSaveInData()->PersistChanges(GetSaveInData()->GetImageManager());

            m_xContentsListBox->remove(nActEntry);

            OUString sId(weld::toId(pEntry));
            m_xContentsListBox->insert(nActEntry, sId);
            m_xContentsListBox->set_toggle(nActEntry,
                                           pEntry->IsVisible() ? TRISTATE_TRUE : TRISTATE_FALSE);
            InsertEntryIntoUI(pEntry, m_xContentsListBox->get_widget(), nActEntry);

            m_xContentsListBox->select(nActEntry);
            m_xContentsListBox->scroll_to_row(nActEntry);

            bNeedsApply = true;
        }
        catch (const css::uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("cui.customize", "Error restoring image");
        }
    }
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown insert option: " << rIdent);
        return;
    }

    if (bNeedsApply)
    {
        static_cast<ToolbarSaveInData*>(GetSaveInData())->ApplyToolbar(pToolbar);
        UpdateButtonStates();
    }
}

IMPL_LINK_NOARG(SvxToolbarConfigPage, ResetToolbarHdl, weld::Button&, void)
{
    sal_Int32 nSelectionPos = m_xTopLevelListBox->get_active();

    SvxConfigEntry* pToolbar
        = weld::fromId<SvxConfigEntry*>(m_xTopLevelListBox->get_id(nSelectionPos));

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
        GetFrameWeld(), VclMessageType::Question, VclButtonsType::YesNo,
        CuiResId(RID_CUISTR_CONFIRM_RESTORE_DEFAULT)));
    if (xQueryBox->run() == RET_YES)
    {
        ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>(GetSaveInData());

        pSaveInData->RestoreToolbar(pToolbar);

        SelectElement();
    }
}

void SvxToolbarConfigPage::UpdateButtonStates()
{
    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    int selection = m_xContentsListBox->get_selected_index();

    bool bIsSeparator
        = selection != -1
          && weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(selection))->IsSeparator();
    bool bIsValidSelection = (m_xContentsListBox->n_children() != 0 && selection != -1);

    m_xMoveUpButton->set_sensitive(bIsValidSelection);
    m_xMoveDownButton->set_sensitive(bIsValidSelection);

    m_xRemoveCommandButton->set_sensitive(bIsValidSelection);

    m_xModifyBtn->set_sensitive(bIsValidSelection && !bIsSeparator);

    // Handle the gear button
    // "toolbar_gear_add" option is always enabled
    m_xGearBtn->set_item_sensitive(u"toolbar_gear_delete"_ustr,
                                   pToolbar && pToolbar->IsDeletable());
    m_xGearBtn->set_item_sensitive(u"toolbar_gear_rename"_ustr,
                                   pToolbar && pToolbar->IsRenamable());
}

short SvxToolbarConfigPage::QueryReset()
{
    OUString msg = CuiResId(RID_CUISTR_CONFIRM_TOOLBAR_RESET);

    OUString saveInName = m_xSaveInListBox->get_active_text();

    OUString label = SvxConfigPageHelper::replaceSaveInName(msg, saveInName);

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(
        GetFrameWeld(), VclMessageType::Question, VclButtonsType::YesNo, label));
    return xQueryBox->run();
}

void SvxToolbarConfigPage::SelectElement()
{
    m_xContentsListBox->clear();

    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    if (pToolbar == nullptr)
    {
        //TODO: Disable related buttons
        m_xInsertBtn->set_sensitive(false);
        m_xResetBtn->set_sensitive(false);
        m_xGearBtn->set_sensitive(false);

        return;
    }
    else
    {
        m_xInsertBtn->set_sensitive(true);
        m_xResetBtn->set_sensitive(true);
        m_xGearBtn->set_sensitive(true);
    }

    switch (pToolbar->GetStyle())
    {
        case 0:
        {
            m_xGearBtn->set_item_active(u"toolbar_gear_iconOnly"_ustr, true);
            break;
        }
        case 1:
        {
            m_xGearBtn->set_item_active(u"toolbar_gear_textOnly"_ustr, true);
            break;
        }
        case 2:
        {
            m_xGearBtn->set_item_active(u"toolbar_gear_iconAndText"_ustr, true);
            break;
        }
    }

    int i = 0;
    SvxEntries* pEntries = pToolbar->GetEntries();
    for (auto const& entry : *pEntries)
    {
        OUString sId(weld::toId(entry));
        m_xContentsListBox->insert(i, sId);
        if (entry->IsBinding() && !entry->IsSeparator())
            m_xContentsListBox->set_toggle(i, entry->IsVisible() ? TRISTATE_TRUE : TRISTATE_FALSE);
        InsertEntryIntoUI(entry, m_xContentsListBox->get_widget(), i);
        ++i;
    }

    UpdateButtonStates();
}

void SvxToolbarConfigPage::AddFunction(int nTarget)
{
    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    if (pToolbar == nullptr)
        return;

    // Add the command to the contents listbox of the selected toolbar
    int nNewLBEntry = SvxConfigPage::AddFunction(nTarget, true /*bAllowDuplicates*/);

    if (nNewLBEntry == -1)
        return;

    SvxConfigEntry* pEntry = weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(nNewLBEntry));

    if (pEntry->IsBinding()) //TODO sep ?
    {
        pEntry->SetVisible(true);
        m_xContentsListBox->set_toggle(nNewLBEntry, TRISTATE_TRUE);
    }

    InsertEntryIntoUI(pEntry, m_xContentsListBox->get_widget(), nNewLBEntry);

    // Changes are not visible on the toolbar until this point
    // TODO: Figure out a way to show the changes on the toolbar, but revert if
    //       the dialog is closed by pressing "Cancel"
    // get currently selected toolbar and apply change
    if (pToolbar != nullptr)
    {
        static_cast<ToolbarSaveInData*>(GetSaveInData())->ApplyToolbar(pToolbar);
    }
}

SvxToolbarEntriesListBox::SvxToolbarEntriesListBox(std::unique_ptr<weld::TreeView> xParent,
                                                   SvxToolbarConfigPage* pPg)
    : SvxMenuEntriesListBox(std::move(xParent), pPg)
{
    m_xControl->connect_toggled(LINK(this, SvxToolbarEntriesListBox, CheckButtonHdl));
    m_xControl->connect_key_press(
        Link<const KeyEvent&, bool>()); //acknowledge we first remove the old one
    m_xControl->connect_key_press(
        LINK(this, SvxToolbarEntriesListBox, KeyInputHdl)); // then add the new one
}

SvxToolbarEntriesListBox::~SvxToolbarEntriesListBox() {}

void SvxToolbarEntriesListBox::ChangedVisibility(int nRow)
{
    SvxConfigEntry* pEntryData = weld::fromId<SvxConfigEntry*>(m_xControl->get_id(nRow));

    if (pEntryData->IsBinding())
    {
        pEntryData->SetVisible(m_xControl->get_toggle(nRow) == TRISTATE_TRUE);

        SvxConfigEntry* pToolbar = m_pPage->GetTopLevelSelection();

        ToolbarSaveInData* pToolbarSaveInData
            = static_cast<ToolbarSaveInData*>(m_pPage->GetSaveInData());

        pToolbarSaveInData->ApplyToolbar(pToolbar);
    }
}

IMPL_LINK(SvxToolbarEntriesListBox, CheckButtonHdl, const weld::TreeView::iter_col&, rRowCol, void)
{
    ChangedVisibility(m_xControl->get_iter_index_in_parent(rRowCol.first));
}

IMPL_LINK(SvxToolbarEntriesListBox, KeyInputHdl, const KeyEvent&, rKeyEvent, bool)
{
    // space key will change visibility of toolbar items
    if (rKeyEvent.GetKeyCode() == KEY_SPACE)
    {
        int nRow = m_xControl->get_selected_index();
        SvxConfigEntry* pEntryData = weld::fromId<SvxConfigEntry*>(m_xControl->get_id(nRow));
        if (pEntryData->IsBinding() && !pEntryData->IsSeparator())
        {
            m_xControl->set_toggle(nRow, m_xControl->get_toggle(nRow) == TRISTATE_TRUE
                                             ? TRISTATE_FALSE
                                             : TRISTATE_TRUE);
            ChangedVisibility(nRow);
        }
        return true;
    }
    return SvxMenuEntriesListBox::KeyInputHdl(rKeyEvent);
}

IMPL_LINK(SvxToolbarConfigPage, ContentContextMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();

    // Select clicked entry
    std::unique_ptr<weld::TreeIter> xIter(rTreeView.make_iterator());
    if (!rTreeView.get_dest_row_at_pos(rCEvt.GetMousePosPixel(), xIter.get(), false))
        return false;
    rTreeView.select(*xIter);
    SelectToolbarEntry(rTreeView);

    int nSelectIndex = m_xContentsListBox->get_selected_index();

    bool bIsSeparator
        = nSelectIndex != -1
          && weld::fromId<SvxConfigEntry*>(m_xContentsListBox->get_id(nSelectIndex))->IsSeparator();
    bool bIsValidSelection = (m_xContentsListBox->n_children() != 0 && nSelectIndex != -1);

    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(&rTreeView, u"cui/ui/entrycontextmenu.ui"_ustr));
    auto xContextMenu = xBuilder->weld_menu(u"menu"_ustr);
    xContextMenu->set_visible(u"add"_ustr, false);
    xContextMenu->set_visible(u"remove"_ustr, bIsValidSelection);
    xContextMenu->set_visible(u"rename"_ustr, bIsValidSelection && !bIsSeparator);
    xContextMenu->set_visible(u"changeIcon"_ustr, bIsValidSelection && !bIsSeparator);
    xContextMenu->set_visible(u"resetIcon"_ustr, bIsValidSelection && !bIsSeparator);
    xContextMenu->set_visible(u"restoreDefault"_ustr, bIsValidSelection && !bIsSeparator);
    OUString sCommand(xContextMenu->popup_at_rect(
        &rTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1, 1))));

    if (sCommand == "remove")
        RemoveCommandHdl(*m_xRemoveCommandButton);
    else if (sCommand == "rename")
        ModifyItemHdl(u"renameItem"_ustr);
    else if (sCommand == "changeIcon")
        ModifyItemHdl(u"changeIcon"_ustr);
    else if (sCommand == "resetIcon")
        ModifyItemHdl(u"resetIcon"_ustr);
    else if (sCommand == "restoreDefault")
        ModifyItemHdl(u"restoreItem"_ustr);
    else if (!sCommand.isEmpty())
        SAL_WARN("cui.customize", "Unknown context menu action: " << sCommand);
    return true;
}

IMPL_LINK(SvxToolbarConfigPage, FunctionContextMenuHdl, const CommandEvent&, rCEvt, bool)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    weld::TreeView& rTreeView = m_xFunctions->get_widget();

    // Select clicked entry
    std::unique_ptr<weld::TreeIter> xIter(rTreeView.make_iterator());
    if (!rTreeView.get_dest_row_at_pos(rCEvt.GetMousePosPixel(), xIter.get(), false))
        return false;
    rTreeView.select(*xIter);
    SelectFunctionHdl(rTreeView);
    std::unique_ptr<weld::Builder> xBuilder(
        Application::CreateBuilder(&rTreeView, u"cui/ui/entrycontextmenu.ui"_ustr));
    auto xContextMenu = xBuilder->weld_menu(u"menu"_ustr);
    xContextMenu->set_visible(u"add"_ustr, true);
    xContextMenu->set_visible(u"remove"_ustr, false);
    xContextMenu->set_visible(u"rename"_ustr, false);
    xContextMenu->set_visible(u"changeIcon"_ustr, false);
    xContextMenu->set_visible(u"resetIcon"_ustr, false);
    xContextMenu->set_visible(u"restoreDefault"_ustr, false);
    OUString sCommand(xContextMenu->popup_at_rect(
        &rTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1, 1))));

    if (sCommand == "add")
        AddCommandHdl(*m_xAddCommandButton);
    else if (!sCommand.isEmpty())
        SAL_WARN("cui.customize", "Unknown context menu action: " << sCommand);
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
