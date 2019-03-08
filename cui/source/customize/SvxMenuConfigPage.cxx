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

#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <typeinfo>

#include <vcl/commandinfoprovider.hxx>
#include <vcl/help.hxx>
#include <vcl/weld.hxx>
#include <vcl/decoview.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>

#include <sfx2/app.hxx>
#include <sfx2/sfxdlg.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/filedlghelper.hxx>
#include <svl/stritem.hxx>
#include <svtools/miscopt.hxx>
#include <vcl/svlbitm.hxx>
#include <vcl/treelistentry.hxx>
#include <vcl/viewdataentry.hxx>
#include <tools/diagnose_ex.h>

#include <algorithm>
#include <strings.hrc>
#include <helpids.h>

#include <acccfg.hxx>
#include <cfg.hxx>
#include <SvxMenuConfigPage.hxx>
#include <SvxConfigPageHelper.hxx>
#include <dialmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <unotools/configmgr.hxx>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/FileSystemStorageFactory.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/XFrames.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/ui/ItemType.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/ui/ImageManager.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationListener.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/util/thePathSettings.hpp>

#include <dlgname.hxx>

SvxMenuConfigPage::SvxMenuConfigPage(TabPageParent pParent, const SfxItemSet& rSet, bool bIsMenuBar)
    : SvxConfigPage(pParent, rSet)
    , m_bIsMenuBar(bIsMenuBar)
{
    m_xGearBtn = m_xBuilder->weld_menu_button("menugearbtn");
    m_xGearBtn->show();
    m_xContentsListBox.reset(new SvxMenuEntriesListBox(m_xBuilder->weld_tree_view("menucontents"), this));
    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();
    rTreeView.connect_size_allocate(LINK(this, SvxMenuConfigPage, MenuEntriesSizeAllocHdl));
    Size aSize(m_xFunctions->get_size_request());
    rTreeView.set_size_request(aSize.Width(), aSize.Height());
    MenuEntriesSizeAllocHdl(aSize);
    rTreeView.set_hexpand(true);
    rTreeView.set_vexpand(true);
    rTreeView.show();

    rTreeView.connect_changed(
        LINK( this, SvxMenuConfigPage, SelectMenuEntry ) );

    rTreeView.connect_model_changed(LINK(this, SvxMenuConfigPage, ListModifiedHdl));

    m_xGearBtn->connect_selected(LINK(this, SvxMenuConfigPage, GearHdl));

    m_xCommandCategoryListBox->connect_changed(LINK(this, SvxMenuConfigPage, SelectCategory));

    m_xMoveUpButton->connect_clicked( LINK( this, SvxConfigPage, MoveHdl) );
    m_xMoveDownButton->connect_clicked( LINK( this, SvxConfigPage, MoveHdl) );

    m_xAddCommandButton->connect_clicked( LINK( this, SvxMenuConfigPage, AddCommandHdl ) );
    m_xRemoveCommandButton->connect_clicked( LINK( this, SvxMenuConfigPage, RemoveCommandHdl ) );

    m_xInsertBtn->connect_selected(
        LINK( this, SvxMenuConfigPage, InsertHdl ) );
    m_xModifyBtn->connect_selected(
        LINK( this, SvxMenuConfigPage, ModifyItemHdl ) );
    m_xResetBtn->connect_clicked(
        LINK( this, SvxMenuConfigPage, ResetMenuHdl ) );

    // These operations are not possible on menus/context menus yet
    m_xModifyBtn->remove_item("changeIcon");
    m_xModifyBtn->remove_item("resetIcon");
    m_xModifyBtn->remove_item("restoreItem");

    if ( !bIsMenuBar )
    {
        //TODO: Remove this when the gear button is implemented for context menus
        m_xGearBtn->set_sensitive(false);
        m_xGearBtn->hide();
    }
    else
    {
        // TODO: Remove this when it is possible to reset menubar menus individually
        m_xResetBtn->set_sensitive(false);
    }
}

IMPL_LINK_NOARG(SvxMenuConfigPage, ListModifiedHdl, weld::TreeView&, void)
{
    // regenerate with the current ordering within the list
    SvxEntries* pEntries = GetTopLevelSelection()->GetEntries();
    pEntries->clear();

    for (int i = 0; i < m_xContentsListBox->n_children(); ++i)
        pEntries->push_back(reinterpret_cast<SvxConfigEntry*>(m_xContentsListBox->get_id(i).toInt64()));

    GetSaveInData()->SetModified();
    GetTopLevelSelection()->SetModified();
    UpdateButtonStates();
}

IMPL_LINK(SvxMenuConfigPage, MenuEntriesSizeAllocHdl, const Size&, rSize, void)
{
    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();
    std::vector<int> aWidths;
    int nImageColWidth = rTreeView.get_checkbox_column_width();
    aWidths.push_back(nImageColWidth);
    aWidths.push_back(rSize.Width() - 2 * nImageColWidth);
    rTreeView.set_column_fixed_widths(aWidths);
}

SvxMenuConfigPage::~SvxMenuConfigPage()
{
    disposeOnce();
}

// Populates the Menu combo box
void SvxMenuConfigPage::Init()
{
    // ensure that the UI is cleared before populating it
    m_xTopLevelListBox->clear();
    m_xContentsListBox->clear();

    ReloadTopLevelListBox();

    m_xTopLevelListBox->set_active(0);
    SelectElement();

    m_xCommandCategoryListBox->Init(
        comphelper::getProcessComponentContext(),
        m_xFrame,
        vcl::CommandInfoProvider::GetModuleIdentifier(m_xFrame));
    m_xCommandCategoryListBox->categorySelected(m_xFunctions.get(), OUString(), GetSaveInData());
}

void SvxMenuConfigPage::dispose()
{
    for (int i = 0, nCount = m_xSaveInListBox->get_count(); i < nCount; ++i)
        delete reinterpret_cast<SaveInData*>(m_xSaveInListBox->get_id(i).toInt64());
    m_xSaveInListBox->clear();

    SvxConfigPage::dispose();
}

IMPL_LINK_NOARG(SvxMenuConfigPage, SelectMenuEntry, weld::TreeView&, void)
{
    UpdateButtonStates();
}

void SvxMenuConfigPage::UpdateButtonStates()
{
    // Disable Up and Down buttons depending on current selection
    int selection = m_xContentsListBox->get_selected_index();

    bool  bIsSeparator =
        selection != -1 && reinterpret_cast<SvxConfigEntry*>(m_xContentsListBox->get_id(selection).toInt64())->IsSeparator();
    bool bIsValidSelection =
        !(m_xContentsListBox->n_children() == 0 || selection == -1);

    m_xMoveUpButton->set_sensitive(
        bIsValidSelection &&  selection != 0 );
    m_xMoveDownButton->set_sensitive(
        bIsValidSelection && selection != m_xContentsListBox->n_children() - 1);

    m_xRemoveCommandButton->set_sensitive( bIsValidSelection );

    m_xModifyBtn->set_sensitive( bIsValidSelection && !bIsSeparator);

    //Handle the gear button
    if (m_bIsMenuBar)
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();
        // Add option (gear_add) will always be enabled
        m_xGearBtn->set_item_sensitive( "menu_gear_delete", pMenuData->IsDeletable() );
        m_xGearBtn->set_item_sensitive( "menu_gear_rename", pMenuData->IsRenamable() );
        m_xGearBtn->set_item_sensitive( "menu_gear_move", pMenuData->IsMovable() );
    }
}

void SvxMenuConfigPage::DeleteSelectedTopLevel()
{
    SvxConfigEntry* pMenuData = GetTopLevelSelection();

    SvxEntries* pParentEntries =
        FindParentForChild( GetSaveInData()->GetEntries(), pMenuData );

    SvxConfigPageHelper::RemoveEntry( pParentEntries, pMenuData );
    delete pMenuData;

    ReloadTopLevelListBox();

    GetSaveInData()->SetModified( );
}

void SvxMenuConfigPage::DeleteSelectedContent()
{
    int nActEntry = m_xContentsListBox->get_selected_index();

    if (nActEntry != -1)
    {
        // get currently selected menu entry
        SvxConfigEntry* pMenuEntry =
            reinterpret_cast<SvxConfigEntry*>(m_xContentsListBox->get_id(nActEntry).toInt64());

        // get currently selected menu
        SvxConfigEntry* pMenu = GetTopLevelSelection();

        // remove menu entry from the list for this menu
        SvxConfigPageHelper::RemoveEntry( pMenu->GetEntries(), pMenuEntry );

        // remove menu entry from UI
        m_xContentsListBox->remove(nActEntry);

        // if this is a submenu entry, redraw the menus list box
        if ( pMenuEntry->IsPopup() )
        {
            ReloadTopLevelListBox();
        }

        // delete data for menu entry
        delete pMenuEntry;

        GetSaveInData()->SetModified();
        pMenu->SetModified();
    }
}

short SvxMenuConfigPage::QueryReset()
{
    OUString msg = CuiResId( RID_SVXSTR_CONFIRM_MENU_RESET );

    OUString saveInName = m_xSaveInListBox->get_active_text();

    OUString label = SvxConfigPageHelper::replaceSaveInName( msg, saveInName );

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetDialogFrameWeld(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   label));
    return xQueryBox->run();
}

void SvxMenuConfigPage::SelectElement()
{
    m_xContentsListBox->clear();

    SvxConfigEntry* pMenuData = GetTopLevelSelection();

    if ( pMenuData )
    {
        SvxEntries* pEntries = pMenuData->GetEntries();

        int i = 0;
        for (auto const& entry : *pEntries)
        {
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(entry)));
            m_xContentsListBox->insert(i, sId);
            InsertEntryIntoUI(entry, i, 0);
            ++i;
        }
    }

    UpdateButtonStates();
}

IMPL_LINK(SvxMenuConfigPage, GearHdl, const OString&, rIdent, void)
{
    if (rIdent == "menu_gear_add")
    {
        SvxMainMenuOrganizerDialog aDialog(GetDialogFrameWeld(),
            GetSaveInData()->GetEntries(), nullptr, true );

        if (aDialog.run() == RET_OK)
        {
            GetSaveInData()->SetEntries(aDialog.ReleaseEntries());
            ReloadTopLevelListBox(aDialog.GetSelectedEntry());
            GetSaveInData()->SetModified();
        }
    }
    else if (rIdent == "menu_gear_delete")
    {
        DeleteSelectedTopLevel();
    }
    else if (rIdent == "menu_gear_rename")
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();

        OUString sCurrentName( SvxConfigPageHelper::stripHotKey( pMenuData->GetName() ) );
        OUString sDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        SvxNameDialog aNameDialog(GetDialogFrameWeld(), sCurrentName, sDesc);
        aNameDialog.set_help_id(HID_SVX_CONFIG_RENAME_MENU);
        aNameDialog.set_title(CuiResId(RID_SVXSTR_RENAME_MENU));

        if ( aNameDialog.run() == RET_OK )
        {
            OUString sNewName = aNameDialog.GetName();

            if ( sCurrentName == sNewName )
                return;

            pMenuData->SetName( sNewName );

            ReloadTopLevelListBox();

            GetSaveInData()->SetModified();
        }
    }
    else if (rIdent == "menu_gear_move")
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();

        SvxMainMenuOrganizerDialog aDialog(GetDialogFrameWeld(), GetSaveInData()->GetEntries(),
                pMenuData, false );
        if (aDialog.run() == RET_OK)
        {
            GetSaveInData()->SetEntries(aDialog.ReleaseEntries());

            ReloadTopLevelListBox();

            GetSaveInData()->SetModified();
        }
    }
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown gear menu option: " << rIdent);
        return;
    }

    UpdateButtonStates();
}

IMPL_LINK_NOARG(SvxMenuConfigPage, SelectCategory, weld::ComboBox&, void)
{
    OUString aSearchTerm( m_xSearchEdit->get_text() );

    m_xCommandCategoryListBox->categorySelected(m_xFunctions.get(), aSearchTerm, GetSaveInData());
}

IMPL_LINK_NOARG( SvxMenuConfigPage, AddCommandHdl, weld::Button&, void )
{
    int nPos = AddFunction();
    if (nPos == -1)
        return;
    SvxConfigEntry* pEntry =
        reinterpret_cast<SvxConfigEntry*>(m_xContentsListBox->get_id(nPos).toInt64());
    InsertEntryIntoUI(pEntry, nPos, 0);
}

IMPL_LINK_NOARG( SvxMenuConfigPage, RemoveCommandHdl, weld::Button&, void )
{
    DeleteSelectedContent();
    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
    }
}

IMPL_LINK(SvxMenuConfigPage, InsertHdl, const OString&, rIdent, void)
{
    if (rIdent == "insertseparator")
    {
        SvxConfigEntry* pNewEntryData = new SvxConfigEntry;
        pNewEntryData->SetUserDefined();
        int nPos = InsertEntry(pNewEntryData, -1);
        InsertEntryIntoUI(pNewEntryData, nPos, 0);
    }
    else if (rIdent == "insertsubmenu")
    {
        OUString aNewName;
        OUString aDesc = CuiResId( RID_SVXSTR_SUBMENU_NAME );

        SvxNameDialog aNameDialog(GetDialogFrameWeld(), aNewName, aDesc);
        aNameDialog.set_help_id(HID_SVX_CONFIG_NAME_SUBMENU);
        aNameDialog.set_title(CuiResId( RID_SVXSTR_ADD_SUBMENU));

        if (aNameDialog.run() == RET_OK)
        {
            aNewName = aNameDialog.GetName();

            SvxConfigEntry* pNewEntryData =
                new SvxConfigEntry( aNewName, aNewName, true, /*bParentData*/false );
            pNewEntryData->SetName( aNewName );
            pNewEntryData->SetUserDefined();

            int nPos = InsertEntry(pNewEntryData, -1);
            InsertEntryIntoUI(pNewEntryData, nPos, 0);

            ReloadTopLevelListBox();

            m_xContentsListBox->scroll_to_row(nPos);
            m_xContentsListBox->select(nPos);

            GetSaveInData()->SetModified();
        }

    }
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown insert option: " << rIdent);
        return;
    }

    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
    }
}

IMPL_LINK(SvxMenuConfigPage, ModifyItemHdl, const OString&, rIdent, void)
{
    if (rIdent == "renameItem")
    {
        int nActEntry = m_xContentsListBox->get_selected_index();
        SvxConfigEntry* pEntry =
            reinterpret_cast<SvxConfigEntry*>(m_xContentsListBox->get_id(nActEntry).toInt64());

        OUString aNewName( SvxConfigPageHelper::stripHotKey( pEntry->GetName() ) );
        OUString aDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        SvxNameDialog aNameDialog(GetDialogFrameWeld(), aNewName, aDesc);
        aNameDialog.set_help_id(HID_SVX_CONFIG_RENAME_MENU_ITEM);
        aNameDialog.set_title(CuiResId(RID_SVXSTR_RENAME_MENU));

        if (aNameDialog.run() == RET_OK)
        {
            aNewName = aNameDialog.GetName();

            pEntry->SetName( aNewName );
            m_xContentsListBox->set_text(nActEntry, aNewName, 1);

            GetSaveInData()->SetModified();
            GetTopLevelSelection()->SetModified();
        }
    }
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown insert option: " << rIdent);
        return;
    }

    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
    }
}

IMPL_LINK_NOARG(SvxMenuConfigPage, ResetMenuHdl, weld::Button&, void)
{
    SvxConfigEntry* pMenuData = GetTopLevelSelection();

    if (pMenuData == nullptr)
    {
        SAL_WARN("cui.customize", "RHB top level selection is null. A menu must be selected to reset!");
        return;
    }

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetDialogFrameWeld(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   CuiResId(RID_SVXSTR_CONFIRM_RESTORE_DEFAULT_MENU)));

    // Resetting individual top-level menus is not possible at the moment.
    // So we are resetting only if it is a context menu
    if (!m_bIsMenuBar && xQueryBox->run() == RET_YES)
    {
        sal_Int32 nPos = m_xTopLevelListBox->get_active();
        ContextMenuSaveInData* pSaveInData = static_cast< ContextMenuSaveInData* >(GetSaveInData());

        pSaveInData->ResetContextMenu(pMenuData);

        // ensure that the UI is cleared before populating it
        m_xTopLevelListBox->clear();
        m_xContentsListBox->clear();

        ReloadTopLevelListBox();

        // Reselect the resetted menu
        m_xTopLevelListBox->set_active(nPos);
        SelectElement();
    }
}

SaveInData* SvxMenuConfigPage::CreateSaveInData(
    const css::uno::Reference< css::ui::XUIConfigurationManager >& xCfgMgr,
    const css::uno::Reference< css::ui::XUIConfigurationManager >& xParentCfgMgr,
    const OUString& aModuleId,
    bool bDocConfig )
{
    if ( !m_bIsMenuBar )
        return static_cast< SaveInData* >( new ContextMenuSaveInData( xCfgMgr, xParentCfgMgr, aModuleId, bDocConfig ) );

    return static_cast< SaveInData* >( new MenuSaveInData( xCfgMgr, xParentCfgMgr, aModuleId, bDocConfig ) );
}

 /* vim:set shiftwidth=4 softtabstop=4 expandtab: */
