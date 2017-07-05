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

#include "sal/config.h"

#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <typeinfo>

#include <vcl/commandinfoprovider.hxx>
#include <vcl/help.hxx>
#include <vcl/layout.hxx>
#include <vcl/msgbox.hxx>
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
#include <svtools/svlbitm.hxx>
#include "svtools/treelistentry.hxx"
#include "svtools/viewdataentry.hxx"
#include <tools/diagnose_ex.h>
#include <toolkit/helper/vclunohelper.hxx>

#include <algorithm>
#include <cuires.hrc>
#include "helpid.hrc"

#include "acccfg.hxx"
#include "cfg.hxx"
#include "SvxMenuConfigPage.hxx"
#include "SvxConfigPageHelper.hxx"
#include "eventdlg.hxx"
#include <dialmgr.hxx>

#include <comphelper/documentinfo.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/random.hxx>
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
#include <com/sun/star/frame/theUICommandDescription.hpp>
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
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/util/thePathSettings.hpp>

#include "dlgname.hxx"

SvxMenuConfigPage::SvxMenuConfigPage(vcl::Window *pParent, const SfxItemSet& rSet, bool bIsMenuBar)
    : SvxConfigPage(pParent, rSet)
    , m_bIsMenuBar( bIsMenuBar )
{
    m_pContentsListBox = VclPtr<SvxMenuEntriesListBox>::Create(m_pEntries, this);
    m_pContentsListBox->set_grid_left_attach(0);
    m_pContentsListBox->set_grid_top_attach(0);
    m_pContentsListBox->set_hexpand(true);
    m_pContentsListBox->set_vexpand(true);
    m_pContentsListBox->Show();

    m_pAddSubmenuButton->Enable();
    m_pAddSubmenuButton->Show();

    m_pTopLevelListBox->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectMenu ) );

    m_pContentsListBox->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectMenuEntry ) );

    m_pMoveUpButton->SetClickHdl ( LINK( this, SvxConfigPage, MoveHdl) );
    m_pMoveDownButton->SetClickHdl ( LINK( this, SvxConfigPage, MoveHdl) );

    m_pNewTopLevelButton->SetClickHdl  (
        LINK( this, SvxMenuConfigPage, NewMenuHdl ) );

    m_pAddCommandsButton->SetClickHdl  (
        LINK( this, SvxMenuConfigPage, AddCommandsHdl ) );

    m_pAddSeparatorButton->SetClickHdl  (
        LINK( this, SvxMenuConfigPage, AddSeparatorHdl ) );

    m_pAddSubmenuButton->SetClickHdl  (
        LINK( this, SvxMenuConfigPage, AddSubmenuHdl ) );

    m_pDeleteCommandButton->SetClickHdl  (
        LINK( this, SvxMenuConfigPage, DeleteCommandHdl ) );

    PopupMenu* pMenu = m_pModifyTopLevelButton->GetPopupMenu();
    pMenu->SetMenuFlags(
        pMenu->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );

    m_pModifyTopLevelButton->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, MenuSelectHdl ) );

    PopupMenu* pEntry = m_pModifyCommandButton->GetPopupMenu();
    pEntry->SetMenuFlags(
        pEntry->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );

    m_pModifyCommandButton->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, EntrySelectHdl ) );

    if ( !bIsMenuBar )
    {
        m_pTopLevel->set_label( CuiResId( RID_SVXSTR_PRODUCTNAME_CONTEXTMENUS ) );
        m_pNewTopLevelButton->Hide();
        pMenu->HideItem( pMenu->GetItemId( "move" ) );
        pMenu->HideItem( pMenu->GetItemId( "menuitem3" ) );
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
    PopupMenu* pPopup = m_pModifyCommandButton->GetPopupMenu();

    // Disable Up and Down buttons depending on current selection
    SvTreeListEntry* selection = m_pContentsListBox->GetCurEntry();

    if ( m_pContentsListBox->GetEntryCount() == 0 || selection == nullptr )
    {
        m_pMoveUpButton->Enable( false );
        m_pMoveDownButton->Enable( false );
        m_pDeleteCommandButton->Enable(false);

        pPopup->EnableItem( "modrename", false );

        m_pDescriptionField->SetText("");

        return;
    }

    SvTreeListEntry* first = m_pContentsListBox->First();
    SvTreeListEntry* last = m_pContentsListBox->Last();

    m_pMoveUpButton->Enable( selection != first );
    m_pMoveDownButton->Enable( selection != last );

    SvxConfigEntry* pEntryData =
        static_cast<SvxConfigEntry*>(selection->GetUserData());

    if ( pEntryData->IsSeparator() )
    {
        pPopup->EnableItem( "modrename", false );

        m_pDescriptionField->SetText("");

        m_pAddSeparatorButton->Enable( false );
        m_pDeleteCommandButton->Enable();
    }
    else
    {
        pPopup->EnableItem( "modrename" );

        m_pDeleteCommandButton->Enable();
        m_pAddSeparatorButton->Enable();

        m_pDescriptionField->SetText(pEntryData->GetHelpText());
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
        m_pSaveInListBox->GetSelectEntryPos() );

    OUString label = SvxConfigPageHelper::replaceSaveInName( msg, saveInName );

    ScopedVclPtrInstance<QueryBox> qbox( this, WB_YES_NO, label );

    return qbox->Execute();
}

IMPL_LINK_NOARG( SvxMenuConfigPage, SelectMenu, ListBox&, void )
{
    m_pContentsListBox->Clear();

    SvxConfigEntry* pMenuData = GetTopLevelSelection();
    m_pModifyTopLevelButton->Enable( pMenuData != nullptr );
    m_pModifyCommandButton->Enable( pMenuData != nullptr );
    m_pAddCommandsButton->Enable( pMenuData != nullptr );
    m_pAddSeparatorButton->Enable( pMenuData != nullptr );
    m_pAddSubmenuButton->Enable( pMenuData != nullptr );

    PopupMenu* pPopup = m_pModifyTopLevelButton->GetPopupMenu();
    if ( pMenuData )
    {
        pPopup->EnableItem( "delete", pMenuData->IsDeletable() );
        pPopup->EnableItem( "rename", pMenuData->IsRenamable() );
        pPopup->EnableItem( "move", pMenuData->IsMovable() );

        SvxEntries* pEntries = pMenuData->GetEntries();
        SvxEntries::const_iterator iter = pEntries->begin();

        for ( ; iter != pEntries->end(); ++iter )
        {
            SvxConfigEntry* pEntry = *iter;
            InsertEntryIntoUI( pEntry );
        }
    }

    UpdateButtonStates();
}

IMPL_LINK( SvxMenuConfigPage, MenuSelectHdl, MenuButton *, pButton, void )
{
    OString sIdent = pButton->GetCurItemIdent();

    if (sIdent == "delete")
    {
        DeleteSelectedTopLevel();
    }
    else if (sIdent == "rename")
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();

        OUString aNewName( SvxConfigPageHelper::stripHotKey( pMenuData->GetName() ) );
        OUString aDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        VclPtrInstance< SvxNameDialog > pNameDialog( this, aNewName, aDesc );
        pNameDialog->SetHelpId( HID_SVX_CONFIG_RENAME_MENU );
        pNameDialog->SetText( CuiResId( RID_SVXSTR_RENAME_MENU ) );

        if ( pNameDialog->Execute() == RET_OK ) {
            pNameDialog->GetName( aNewName );
            pMenuData->SetName( aNewName );

            ReloadTopLevelListBox();

            GetSaveInData()->SetModified();
        }
    }
    else if (sIdent == "move")
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();

        VclPtr<SvxMainMenuOrganizerDialog> pDialog(
            VclPtr<SvxMainMenuOrganizerDialog>::Create( this,
                GetSaveInData()->GetEntries(), pMenuData ));

        if ( pDialog->Execute() == RET_OK )
        {
            GetSaveInData()->SetEntries( pDialog->GetEntries() );

            ReloadTopLevelListBox( pDialog->GetSelectedEntry() );

            GetSaveInData()->SetModified();
        }
    }
}

IMPL_LINK( SvxMenuConfigPage, EntrySelectHdl, MenuButton *, pButton, void )
{
    OString sIdent = pButton->GetCurItemIdent();
    if (sIdent == "modrename")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        OUString aNewName( SvxConfigPageHelper::stripHotKey( pEntry->GetName() ) );
        OUString aDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        VclPtrInstance< SvxNameDialog > pNameDialog( this, aNewName, aDesc );
        pNameDialog->SetHelpId( HID_SVX_CONFIG_RENAME_MENU_ITEM );
        pNameDialog->SetText( CuiResId( RID_SVXSTR_RENAME_MENU ) );

        if ( pNameDialog->Execute() == RET_OK ) {
            pNameDialog->GetName(aNewName);

            pEntry->SetName( aNewName );
            m_pContentsListBox->SetEntryText( pActEntry, aNewName );

            GetSaveInData()->SetModified();
            GetTopLevelSelection()->SetModified();
        }
    }
    else
    {
        return;
    }

    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
    }
}

IMPL_LINK_NOARG( SvxMenuConfigPage, AddFunctionHdl, SvxScriptSelectorDialog&, void )
{
    AddFunction();
}

IMPL_LINK_NOARG( SvxMenuConfigPage, NewMenuHdl, Button *, void )
{
    VclPtrInstance<SvxMainMenuOrganizerDialog> pDialog(
        nullptr, GetSaveInData()->GetEntries(), nullptr, true );

    if ( pDialog->Execute() == RET_OK )
    {
        GetSaveInData()->SetEntries( pDialog->GetEntries() );
        ReloadTopLevelListBox( pDialog->GetSelectedEntry() );
        GetSaveInData()->SetModified();
    }
}

IMPL_LINK_NOARG( SvxMenuConfigPage, AddCommandsHdl, Button *, void )
{
    if ( m_pSelectorDlg == nullptr )
    {
        // Create Script Selector which also shows builtin commands
        m_pSelectorDlg = VclPtr<SvxScriptSelectorDialog>::Create( this, true, m_xFrame );

        m_pSelectorDlg->SetAddHdl(
            LINK( this, SvxMenuConfigPage, AddFunctionHdl ) );

        m_pSelectorDlg->SetDialogDescription( CuiResId( RID_SVXSTR_MENU_ADDCOMMANDS_DESCRIPTION ) );
    }

    // Position the Script Selector over the Add button so it is
    // beside the menu contents list and does not obscure it
    m_pSelectorDlg->SetPosPixel( m_pAddCommandsButton->GetPosPixel() );

    m_pSelectorDlg->SetImageProvider( GetSaveInData() );

    m_pSelectorDlg->Execute();
}

IMPL_LINK_NOARG( SvxMenuConfigPage, AddSeparatorHdl, Button *, void )
{
    SvxConfigEntry* pNewEntryData = new SvxConfigEntry;
    pNewEntryData->SetUserDefined();
    InsertEntry( pNewEntryData );

    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
    }
}

IMPL_LINK_NOARG( SvxMenuConfigPage, AddSubmenuHdl, Button *, void )
{
    OUString aNewName;
    OUString aDesc = CuiResId( RID_SVXSTR_SUBMENU_NAME );

    VclPtrInstance< SvxNameDialog > pNameDialog( this, aNewName, aDesc );
    pNameDialog->SetHelpId( HID_SVX_CONFIG_NAME_SUBMENU );
    pNameDialog->SetText( CuiResId( RID_SVXSTR_ADD_SUBMENU ) );

    if ( pNameDialog->Execute() == RET_OK )
    {
        pNameDialog->GetName(aNewName);

        SvxConfigEntry* pNewEntryData =
            new SvxConfigEntry( aNewName, aNewName, true );
        pNewEntryData->SetName( aNewName );
        pNewEntryData->SetUserDefined();

        InsertEntry( pNewEntryData );

        ReloadTopLevelListBox();

        GetSaveInData()->SetModified();

        UpdateButtonStates();
    }
}

IMPL_LINK_NOARG( SvxMenuConfigPage, DeleteCommandHdl, Button *, void )
{
    DeleteSelectedContent();
    if ( GetSaveInData()->IsModified() )
    {
        UpdateButtonStates();
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
