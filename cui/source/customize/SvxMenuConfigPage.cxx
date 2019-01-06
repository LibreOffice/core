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
#include <o3tl/make_unique.hxx>
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

SvxMenuConfigPage::SvxMenuConfigPage(vcl::Window *pParent, const SfxItemSet& rSet, bool bIsMenuBar)
    : SvxConfigPage(pParent, rSet)
    , m_bIsMenuBar( bIsMenuBar )
{
    m_pFunctions->SetSlotMode(SfxSlotMode::MENUCONFIG);

    m_pContentsListBox = VclPtr<SvxMenuEntriesListBox>::Create(m_pEntries, this);
    m_pContentsListBox->set_grid_left_attach(0);
    m_pContentsListBox->set_grid_top_attach(0);
    m_pContentsListBox->set_hexpand(true);
    m_pContentsListBox->set_vexpand(true);
    m_pContentsListBox->Show();

    m_pTopLevelListBox->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectMenu ) );

    m_pContentsListBox->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectMenuEntry ) );

    m_pGearBtn->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, GearHdl ) );

    m_pCommandCategoryListBox->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectCategory ) );

    m_pMoveUpButton->SetClickHdl ( LINK( this, SvxConfigPage, MoveHdl) );
    m_pMoveDownButton->SetClickHdl ( LINK( this, SvxConfigPage, MoveHdl) );

    m_pAddCommandButton->SetClickHdl( LINK( this, SvxMenuConfigPage, AddCommandHdl ) );
    m_pRemoveCommandButton->SetClickHdl( LINK( this, SvxMenuConfigPage, RemoveCommandHdl ) );

    m_pInsertBtn->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, InsertHdl ) );
    m_pModifyBtn->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, ModifyItemHdl ) );
    m_pResetBtn->SetClickHdl(
        LINK( this, SvxMenuConfigPage, ResetMenuHdl ) );

    PopupMenu* pPopup = m_pModifyBtn->GetPopupMenu();
    // These operations are not possible on menus/context menus yet
    pPopup->EnableItem( pPopup->GetItemId("changeIcon"), false );
    pPopup->EnableItem( pPopup->GetItemId("resetIcon"), false );
    pPopup->EnableItem( pPopup->GetItemId("restoreItem"), false );
    pPopup->RemoveDisabledEntries();

    PopupMenu* pGearMenu = m_pGearBtn->GetPopupMenu();
    pGearMenu->EnableItem( pGearMenu->GetItemId("gear_iconAndText"), false );
    pGearMenu->EnableItem( pGearMenu->GetItemId("gear_iconOnly"), false );
    pGearMenu->EnableItem( pGearMenu->GetItemId("gear_textOnly"), false );
    pGearMenu->RemoveDisabledEntries();

    if ( !bIsMenuBar )
    {
        //TODO: Remove this when the gear button is implemented for context menus
        m_pGearBtn->Disable();
        m_pGearBtn->Hide();
    }
    else
    {
        // TODO: Remove this when it is possible to reset menubar menus individually
        m_pResetBtn->Disable();
    }
}

SvxMenuConfigPage::~SvxMenuConfigPage()
{
    disposeOnce();
}

// Populates the Menu combo box
void SvxMenuConfigPage::Init()
{
    // ensure that the UI is cleared before populating it
    m_pTopLevelListBox->Clear();
    m_pContentsListBox->Clear();

    ReloadTopLevelListBox();

    m_pTopLevelListBox->SelectEntryPos(0);
    m_pTopLevelListBox->GetSelectHdl().Call(*m_pTopLevelListBox);

    m_pCommandCategoryListBox->Init(
        comphelper::getProcessComponentContext(),
        m_xFrame,
        vcl::CommandInfoProvider::GetModuleIdentifier(m_xFrame));
    m_pCommandCategoryListBox->categorySelected( m_pFunctions, OUString(), GetSaveInData() );
}

void SvxMenuConfigPage::dispose()
{
    for ( sal_Int32 i = 0 ; i < m_pSaveInListBox->GetEntryCount(); ++i )
    {
        delete static_cast<SaveInData*>(m_pSaveInListBox->GetEntryData( i ));
    }
    m_pSaveInListBox->Clear();

    SvxConfigPage::dispose();
}

IMPL_LINK_NOARG( SvxMenuConfigPage, SelectMenuEntry, SvTreeListBox *, void )
{
    UpdateButtonStates();
}

void SvxMenuConfigPage::UpdateButtonStates()
{
    // Disable Up and Down buttons depending on current selection
    SvTreeListEntry* selection = m_pContentsListBox->GetCurEntry();

    bool  bIsSeparator =
        selection && static_cast<SvxConfigEntry*>(selection->GetUserData())->IsSeparator();
    bool bIsValidSelection =
        !(m_pContentsListBox->GetEntryCount() == 0 || selection == nullptr);

    m_pMoveUpButton->Enable(
        bIsValidSelection &&  selection != m_pContentsListBox->First() );
    m_pMoveDownButton->Enable(
        bIsValidSelection && selection != m_pContentsListBox->Last() );

    m_pRemoveCommandButton->Enable( bIsValidSelection );

    m_pModifyBtn->Enable( bIsValidSelection && !bIsSeparator);

    //Handle the gear button
    if (m_bIsMenuBar)
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();
        PopupMenu* pGearPopup = m_pGearBtn->GetPopupMenu();
        // Add option (gear_add) will always be enabled
        pGearPopup->EnableItem( "gear_delete", pMenuData->IsDeletable() );
        pGearPopup->EnableItem( "gear_rename", pMenuData->IsRenamable() );
        pGearPopup->EnableItem( "gear_move", pMenuData->IsMovable() );
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
    SvTreeListEntry *pActEntry = m_pContentsListBox->FirstSelected();

    if ( pActEntry != nullptr )
    {
        // get currently selected menu entry
        SvxConfigEntry* pMenuEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        // get currently selected menu
        SvxConfigEntry* pMenu = GetTopLevelSelection();

        // remove menu entry from the list for this menu
        SvxConfigPageHelper::RemoveEntry( pMenu->GetEntries(), pMenuEntry );

        // remove menu entry from UI
        m_pContentsListBox->GetModel()->Remove( pActEntry );

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

    OUString saveInName = m_pSaveInListBox->GetEntry(
        m_pSaveInListBox->GetSelectedEntryPos() );

    OUString label = SvxConfigPageHelper::replaceSaveInName( msg, saveInName );

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   label));
    return xQueryBox->run();
}

IMPL_LINK_NOARG( SvxMenuConfigPage, SelectMenu, ListBox&, void )
{
    m_pContentsListBox->Clear();

    SvxConfigEntry* pMenuData = GetTopLevelSelection();

    if ( pMenuData )
    {
        SvxEntries* pEntries = pMenuData->GetEntries();

        for (auto const& entry : *pEntries)
        {
            InsertEntryIntoUI(entry);
        }
    }

    UpdateButtonStates();
}

IMPL_LINK( SvxMenuConfigPage, GearHdl, MenuButton *, pButton, void )
{
    OString sIdent = pButton->GetCurItemIdent();

    if (sIdent == "gear_add")
    {
        SvxMainMenuOrganizerDialog aDialog(GetFrameWeld(),
            GetSaveInData()->GetEntries(), nullptr, true );

        if (aDialog.run() == RET_OK)
        {
            GetSaveInData()->SetEntries(aDialog.ReleaseEntries());
            ReloadTopLevelListBox(aDialog.GetSelectedEntry());
            GetSaveInData()->SetModified();
        }
    }
    else if (sIdent == "gear_delete")
    {
        DeleteSelectedTopLevel();
    }
    else if (sIdent == "gear_rename")
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();

        OUString sCurrentName( SvxConfigPageHelper::stripHotKey( pMenuData->GetName() ) );
        OUString sDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        SvxNameDialog aNameDialog( GetFrameWeld(), sCurrentName, sDesc );
        aNameDialog.set_help_id( HID_SVX_CONFIG_RENAME_MENU );
        aNameDialog.set_title( CuiResId( RID_SVXSTR_RENAME_MENU ) );

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
    else if (sIdent == "gear_move")
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();

        SvxMainMenuOrganizerDialog aDialog(GetFrameWeld(), GetSaveInData()->GetEntries(),
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
        SAL_WARN("cui.customize", "Unknown gear menu option: " << sIdent);
        return;
    }

    UpdateButtonStates();
}

IMPL_LINK_NOARG( SvxMenuConfigPage, SelectCategory, ListBox&, void )
{
    OUString aSearchTerm( m_pSearchEdit->GetText() );

    m_pCommandCategoryListBox->categorySelected( m_pFunctions, aSearchTerm, GetSaveInData() );
}

IMPL_LINK_NOARG( SvxMenuConfigPage, AddCommandHdl, Button *, void )
{
    AddFunction();
}

IMPL_LINK_NOARG( SvxMenuConfigPage, RemoveCommandHdl, Button *, void )
{
    DeleteSelectedContent();
    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
    }
}

IMPL_LINK( SvxMenuConfigPage, InsertHdl, MenuButton *, pButton, void )
{
    OString sIdent = pButton->GetCurItemIdent();

    if (sIdent == "insertseparator")
    {
        SvxConfigEntry* pNewEntryData = new SvxConfigEntry;
        pNewEntryData->SetUserDefined();
        InsertEntry( pNewEntryData );
    }
    else if (sIdent == "insertsubmenu")
    {
        OUString aNewName;
        OUString aDesc = CuiResId( RID_SVXSTR_SUBMENU_NAME );

        SvxNameDialog aNameDialog(GetFrameWeld(), aNewName, aDesc);
        aNameDialog.set_help_id(HID_SVX_CONFIG_NAME_SUBMENU);
        aNameDialog.set_title(CuiResId( RID_SVXSTR_ADD_SUBMENU));

        if (aNameDialog.run() == RET_OK)
        {
            aNewName = aNameDialog.GetName();

            SvxConfigEntry* pNewEntryData =
                new SvxConfigEntry( aNewName, aNewName, true, /*bParentData*/false );
            pNewEntryData->SetName( aNewName );
            pNewEntryData->SetUserDefined();

            InsertEntry( pNewEntryData );

            ReloadTopLevelListBox();

            GetSaveInData()->SetModified();
        }

    }
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown insert option: " << sIdent);
        return;
    }

    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
    }
}

IMPL_LINK( SvxMenuConfigPage, ModifyItemHdl, MenuButton *, pButton, void )
{
    OString sIdent = pButton->GetCurItemIdent();

    SAL_WARN("cui.customize", "sIdent: " << sIdent);

    if (sIdent == "renameItem")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        OUString aNewName( SvxConfigPageHelper::stripHotKey( pEntry->GetName() ) );
        OUString aDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        SvxNameDialog aNameDialog(GetFrameWeld(), aNewName, aDesc);
        aNameDialog.set_help_id(HID_SVX_CONFIG_RENAME_MENU_ITEM);
        aNameDialog.set_title(CuiResId(RID_SVXSTR_RENAME_MENU));

        if (aNameDialog.run() == RET_OK)
        {
            aNewName = aNameDialog.GetName();

            pEntry->SetName( aNewName );
            m_pContentsListBox->SetEntryText( pActEntry, aNewName );

            GetSaveInData()->SetModified();
            GetTopLevelSelection()->SetModified();
        }
    }
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown insert option: " << sIdent);
        return;
    }

    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
    }
}

IMPL_LINK_NOARG( SvxMenuConfigPage, ResetMenuHdl, Button *, void )
{
    SvxConfigEntry* pMenuData = GetTopLevelSelection();

    if (pMenuData == nullptr)
    {
        SAL_WARN("cui.customize", "RHB top level selection is null. A menu must be selected to reset!");
        return;
    }

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   CuiResId(RID_SVXSTR_CONFIRM_RESTORE_DEFAULT_MENU)));

    // Resetting individual top-level menus is not possible at the moment.
    // So we are resetting only if it is a context menu
    if (!m_bIsMenuBar && xQueryBox->run() == RET_YES)
    {
        sal_Int32 nPos = m_pTopLevelListBox->GetSelectedEntryPos();
        ContextMenuSaveInData* pSaveInData = static_cast< ContextMenuSaveInData* >(GetSaveInData());

        pSaveInData->ResetContextMenu(pMenuData);

        // ensure that the UI is cleared before populating it
        m_pTopLevelListBox->Clear();
        m_pContentsListBox->Clear();

        ReloadTopLevelListBox();

        // Reselect the resetted menu
        m_pTopLevelListBox->SelectEntryPos(nPos);
        m_pTopLevelListBox->GetSelectHdl().Call(*m_pTopLevelListBox);
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
