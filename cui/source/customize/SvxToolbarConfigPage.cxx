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
#include "SvxToolbarConfigPage.hxx"
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

SvxToolbarConfigPage::SvxToolbarConfigPage(vcl::Window *pParent, const SfxItemSet& rSet)
    : SvxConfigPage(pParent, rSet)
    , m_pMenu(get_menu("modifymenu"))
    , m_pEntry(get_menu("contentmenu"))
{
    SetHelpId( HID_SVX_CONFIG_TOOLBAR );

    m_pContentsListBox = VclPtr<SvxToolbarEntriesListBox>::Create(m_pEntries, this);
    m_pContentsListBox->set_grid_left_attach(0);
    m_pContentsListBox->set_grid_top_attach(0);
    m_pContentsListBox->set_hexpand(true);
    m_pContentsListBox->set_vexpand(true);
    m_pContentsListBox->Show();

    m_pTopLevelListBox->SetHelpId ( HID_SVX_TOPLEVELLISTBOX );
    m_pIconsOnlyRB->SetHelpId ( HID_SVX_ICONSONLY );
    m_pTextOnlyRB->SetHelpId ( HID_SVX_TEXTONLY );
    m_pIconsAndTextRB->SetHelpId ( HID_SVX_ICONSANDTEXT );
    m_pContentsListBox->SetHelpId( HID_SVX_CONFIG_TOOLBAR_CONTENTS );
    m_pNewTopLevelButton->SetHelpId( HID_SVX_NEW_TOOLBAR );
    m_pModifyTopLevelButton->SetHelpId( HID_SVX_MODIFY_TOOLBAR );
    m_pAddCommandsButton->SetHelpId( HID_SVX_NEW_TOOLBAR_ITEM );
    m_pAddSeparatorButton->SetHelpId ( HID_SVX_ADD_SEPARATOR );
    m_pModifyCommandButton->SetHelpId( HID_SVX_MODIFY_TOOLBAR_ITEM );
    m_pDeleteCommandButton->SetHelpId ( HID_SVX_REMOVEBUTTON );
    m_pResetTopLevelButton->SetHelpId ( HID_SVX_RESETBUTTON );
    m_pSaveInListBox->SetHelpId( HID_SVX_SAVE_IN );
    m_pMoveUpButton->SetHelpId( HID_SVX_UP_TOOLBAR_ITEM );
    m_pMoveDownButton->SetHelpId( HID_SVX_DOWN_TOOLBAR_ITEM );
    m_pDescriptionField->SetHelpId ( HID_SVX_DESCFIELD );

    m_pTopLevel->set_label(CuiResId(RID_SVXSTR_PRODUCTNAME_TOOLBARS));

    m_pTopLevelLabel->SetText( CuiResId( RID_SVXSTR_TOOLBAR ) );
    m_pModifyTopLevelButton->SetText( CuiResId( RID_SVXSTR_TOOLBAR ) );
    m_pContents->set_label(CuiResId(RID_SVXSTR_TOOLBAR_CONTENT));
    m_pContentsLabel->SetText( CuiResId( RID_SVXSTR_COMMANDS ) );

    // The reset button will be used in the toolbar config tab
    m_pResetTopLevelButton->Show();
    // These radio buttons will be used in the toolbar config tab
    m_pIconsOnlyRB->Enable();
    m_pTextOnlyRB->Enable();
    m_pIconsAndTextRB->Enable();
    m_pIconsOnlyRB->Show();
    m_pTextOnlyRB->Show();
    m_pIconsAndTextRB->Show();
    m_pToolbarStyleLabel->Show();

    m_pTopLevelListBox->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, SelectToolbar ) );
    m_pContentsListBox->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, SelectToolbarEntry ) );

    m_pIconsOnlyRB->SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, StyleChangeHdl ) );
    m_pTextOnlyRB->SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, StyleChangeHdl ) );
    m_pIconsAndTextRB->SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, StyleChangeHdl ) );

    m_pNewTopLevelButton->SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, NewToolbarHdl ) );

    m_pAddCommandsButton->SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, AddCommandsHdl ) );

    m_pAddSeparatorButton->SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, AddSeparatorHdl ) );

    m_pDeleteCommandButton->SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, DeleteCommandHdl ) );

    m_pResetTopLevelButton->SetClickHdl  (
        LINK( this, SvxToolbarConfigPage, ResetTopLevelHdl ) );

    m_pMoveUpButton->SetClickHdl ( LINK( this, SvxToolbarConfigPage, MoveHdl) );
    m_pMoveDownButton->SetClickHdl ( LINK( this, SvxToolbarConfigPage, MoveHdl) );
    // Always enable Up and Down buttons
    // added for issue i53677 by shizhoubo
    m_pMoveDownButton->Enable();
    m_pMoveUpButton->Enable();

    m_pMenu->SetMenuFlags(
        m_pMenu->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );

    m_pModifyTopLevelButton->SetPopupMenu( m_pMenu );
    m_pModifyTopLevelButton->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, ToolbarSelectHdl ) );

    m_pEntry->SetMenuFlags(
        m_pEntry->GetMenuFlags() | MenuFlags::AlwaysShowDisabledEntries );

    m_pModifyCommandButton->SetPopupMenu(m_pEntry);
    m_pModifyCommandButton->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, EntrySelectHdl ) );

    // default toolbar to select is standardbar unless a different one
    // has been passed in
    m_aURLToSelect = ITEM_TOOLBAR_URL;
    m_aURLToSelect += "standardbar";

    const SfxPoolItem* pItem =
        rSet.GetItem( rSet.GetPool()->GetWhich( SID_CONFIG ) );

    if ( pItem )
    {
        OUString text = static_cast<const SfxStringItem*>(pItem)->GetValue();
        if (text.startsWith( ITEM_TOOLBAR_URL ))
        {
            m_aURLToSelect = text.copy( 0 );
        }
    }
}

SvxToolbarConfigPage::~SvxToolbarConfigPage()
{
    disposeOnce();
}

void SvxToolbarConfigPage::dispose()
{
    for ( sal_Int32 i = 0 ; i < m_pSaveInListBox->GetEntryCount(); ++i )
    {
        ToolbarSaveInData* pData =
            static_cast<ToolbarSaveInData*>(m_pSaveInListBox->GetEntryData( i ));

        delete pData;
    }
    m_pSaveInListBox->Clear();

    m_pEntry.clear();
    m_pMenu.clear();

    SvxConfigPage::dispose();
}

void SvxToolbarConfigPage::DeleteSelectedTopLevel()
{
    const sal_Int32 nSelectionPos = m_pTopLevelListBox->GetSelectEntryPos();
    ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>( GetSaveInData() );
    pSaveInData->RemoveToolbar( GetTopLevelSelection() );

    if ( m_pTopLevelListBox->GetEntryCount() > 1 )
    {
        // select next entry after the one being deleted
        // selection position is indexed from 0 so need to
        // subtract one from the entry count
        if ( nSelectionPos != m_pTopLevelListBox->GetEntryCount() - 1 )
        {
            m_pTopLevelListBox->SelectEntryPos( nSelectionPos + 1 );
        }
        else
        {
            m_pTopLevelListBox->SelectEntryPos( nSelectionPos - 1 );
        }
        m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );

        // and now remove the entry
        m_pTopLevelListBox->RemoveEntry( nSelectionPos );
    }
    else
    {
        ReloadTopLevelListBox();
    }
}

void SvxToolbarConfigPage::DeleteSelectedContent()
{
    SvTreeListEntry *pActEntry = m_pContentsListBox->FirstSelected();

    if ( pActEntry != nullptr )
    {
        // get currently selected entry
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        SvxConfigEntry* pToolbar = GetTopLevelSelection();

        // remove entry from the list for this toolbar
        SvxConfigPageHelper::RemoveEntry( pToolbar->GetEntries(), pEntry );

        // remove toolbar entry from UI
        m_pContentsListBox->GetModel()->Remove( pActEntry );

        // delete data for toolbar entry
        delete pEntry;

        static_cast<ToolbarSaveInData*>(GetSaveInData())->ApplyToolbar( pToolbar );
        UpdateButtonStates();

        // if this is the last entry in the toolbar and it is a user
        // defined toolbar pop up a dialog asking the user if they
        // want to delete the toolbar
        if ( m_pContentsListBox->GetEntryCount() == 0 &&
             GetTopLevelSelection()->IsDeletable() )
        {
            ScopedVclPtrInstance<MessageDialog> qbox(this,
                CuiResId(RID_SXVSTR_CONFIRM_DELETE_TOOLBAR), VclMessageType::Question, VclButtonsType::YesNo);

            if ( qbox->Execute() == RET_YES )
            {
                DeleteSelectedTopLevel();
            }
        }
    }
}

IMPL_LINK( SvxToolbarConfigPage, MoveHdl, Button *, pButton, void )
{
    MoveEntry(pButton == m_pMoveUpButton);
}

void SvxToolbarConfigPage::MoveEntry( bool bMoveUp )
{
    SvxConfigPage::MoveEntry( bMoveUp );

    // Apply change to currently selected toolbar
    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    if ( pToolbar )
        static_cast<ToolbarSaveInData*>(GetSaveInData())->ApplyToolbar( pToolbar );
    else
    {
        SAL_WARN( "cui.customize", "SvxToolbarConfigPage::MoveEntry(): no entry" );
        UpdateButtonStates();
    }
}

IMPL_LINK( SvxToolbarConfigPage, ToolbarSelectHdl, MenuButton *, pButton, void )
{
    sal_Int32 nSelectionPos = m_pTopLevelListBox->GetSelectEntryPos();

    SvxConfigEntry* pToolbar =
        static_cast<SvxConfigEntry*>(m_pTopLevelListBox->GetEntryData( nSelectionPos ));

    ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>( GetSaveInData() );

    OString sCommand = m_pMenu->GetItemIdent(pButton->GetCurItemId());

    if (sCommand == "modtooldelete")
    {
        DeleteSelectedTopLevel();
        UpdateButtonStates();
    }
    else if (sCommand == "modtoolrename")
    {
        OUString aNewName( SvxConfigPageHelper::stripHotKey( pToolbar->GetName() ) );
        OUString aDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        VclPtrInstance< SvxNameDialog > pNameDialog( this, aNewName, aDesc );
        pNameDialog->SetHelpId( HID_SVX_CONFIG_RENAME_TOOLBAR );
        pNameDialog->SetText( CuiResId( RID_SVXSTR_RENAME_TOOLBAR ) );

        if ( pNameDialog->Execute() == RET_OK )
        {
            pNameDialog->GetName(aNewName);

            pToolbar->SetName( aNewName );
            pSaveInData->ApplyToolbar( pToolbar );

            // have to use remove and insert to change the name
            m_pTopLevelListBox->RemoveEntry( nSelectionPos );
            nSelectionPos =
                m_pTopLevelListBox->InsertEntry( aNewName, nSelectionPos );
            m_pTopLevelListBox->SetEntryData( nSelectionPos, pToolbar );
            m_pTopLevelListBox->SelectEntryPos( nSelectionPos );
        }
    }
}

IMPL_LINK( SvxToolbarConfigPage, EntrySelectHdl, MenuButton *, pButton, void )
{
    bool bNeedsApply = false;

    // get currently selected toolbar
    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    OString sCommand = m_pEntry->GetItemIdent(pButton->GetCurItemId());

    if (sCommand == "toolrename")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        OUString aNewName( SvxConfigPageHelper::stripHotKey( pEntry->GetName() ) );
        OUString aDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        VclPtrInstance< SvxNameDialog > pNameDialog( this, aNewName, aDesc );
        pNameDialog->SetHelpId( HID_SVX_CONFIG_RENAME_TOOLBAR_ITEM );
        pNameDialog->SetText( CuiResId( RID_SVXSTR_RENAME_TOOLBAR ) );

        if ( pNameDialog->Execute() == RET_OK ) {
            pNameDialog->GetName(aNewName);

            if( aNewName.isEmpty() ) //tdf#80758 - Accelerator character ("~") is passed as
                pEntry->SetName( "~" ); // the button name in case of empty values.
            else
                pEntry->SetName( aNewName );

            m_pContentsListBox->SetEntryText( pActEntry, aNewName );
            bNeedsApply = true;
        }
    }
    else if (sCommand == "toolrestore")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        sal_uInt16 nSelectionPos = 0;

        // find position of entry within the list
        for ( sal_uLong i = 0; i < m_pContentsListBox->GetEntryCount(); ++i )
        {
            if ( m_pContentsListBox->GetEntry( nullptr, i ) == pActEntry )
            {
                nSelectionPos = i;
                break;
            }
        }

        ToolbarSaveInData* pSaveInData =
            static_cast<ToolbarSaveInData*>( GetSaveInData() );

        OUString aSystemName =
            pSaveInData->GetSystemUIName( pEntry->GetCommand() );

        if ( !pEntry->GetName().equals( aSystemName ) )
        {
            pEntry->SetName( aSystemName );
            m_pContentsListBox->SetEntryText(
                pActEntry, SvxConfigPageHelper::stripHotKey( aSystemName ) );
            bNeedsApply = true;
        }

        css::uno::Sequence<OUString> aURLSeq { pEntry->GetCommand() };

        try
        {
            GetSaveInData()->GetImageManager()->removeImages(
                SvxConfigPageHelper::GetImageType(), aURLSeq );

            // reset backup in entry
            pEntry->SetBackupGraphic(
                css::uno::Reference< css::graphic::XGraphic >() );

            GetSaveInData()->PersistChanges(
                GetSaveInData()->GetImageManager() );

            m_pContentsListBox->GetModel()->Remove( pActEntry );

            SvTreeListEntry* pNewLBEntry =
                InsertEntryIntoUI( pEntry, nSelectionPos );

            m_pContentsListBox->SetCheckButtonState( pNewLBEntry,
                pEntry->IsVisible() ?
                    SvButtonState::Checked : SvButtonState::Unchecked );

            m_pContentsListBox->Select( pNewLBEntry );
            m_pContentsListBox->MakeVisible( pNewLBEntry );

            bNeedsApply = true;
        }
        catch ( css::uno::Exception& )
        {
            SAL_WARN("cui.customize", "Error restoring image");
        }
    }
    else if (sCommand == "toolchange")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        sal_uInt16 nSelectionPos = 0;

        // find position of entry within the list
        for ( sal_uLong i = 0; i < m_pContentsListBox->GetEntryCount(); ++i )
        {
            if ( m_pContentsListBox->GetEntry( nullptr, i ) == pActEntry )
            {
                nSelectionPos = i;
                break;
            }
        }

        ScopedVclPtr<SvxIconSelectorDialog> pIconDialog(
            VclPtr<SvxIconSelectorDialog>::Create( nullptr,
                GetSaveInData()->GetImageManager(),
                GetSaveInData()->GetParentImageManager() ));

        if ( pIconDialog->Execute() == RET_OK )
        {
            css::uno::Reference< css::graphic::XGraphic > newgraphic =
                pIconDialog->GetSelectedIcon();

            if ( newgraphic.is() )
            {
                css::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > >
                    aGraphicSeq( 1 );

                css::uno::Sequence<OUString> aURLSeq { pEntry->GetCommand() };

                if ( !pEntry->GetBackupGraphic().is() )
                {
                    css::uno::Reference< css::graphic::XGraphic > backup;
                    backup = SvxConfigPageHelper::GetGraphic(
                        GetSaveInData()->GetImageManager(), aURLSeq[ 0 ] );

                    if ( backup.is() )
                    {
                        pEntry->SetBackupGraphic( backup );
                    }
                }

                aGraphicSeq[ 0 ] = newgraphic;
                try
                {
                    GetSaveInData()->GetImageManager()->replaceImages(
                        SvxConfigPageHelper::GetImageType(), aURLSeq, aGraphicSeq );

                    m_pContentsListBox->GetModel()->Remove( pActEntry );
                    SvTreeListEntry* pNewLBEntry =
                        InsertEntryIntoUI( pEntry, nSelectionPos );

                    m_pContentsListBox->SetCheckButtonState( pNewLBEntry,
                        pEntry->IsVisible() ?
                            SvButtonState::Checked : SvButtonState::Unchecked );

                    m_pContentsListBox->Select( pNewLBEntry );
                    m_pContentsListBox->MakeVisible( pNewLBEntry );

                    GetSaveInData()->PersistChanges(
                        GetSaveInData()->GetImageManager() );
                }
                catch ( css::uno::Exception& )
                {
                    SAL_WARN("cui.customize", "Error replacing image");
                }
            }
        }
    }
    else if (sCommand == "toolreset")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        sal_uInt16 nSelectionPos = 0;

        // find position of entry within the list
        for ( sal_uLong i = 0; i < m_pContentsListBox->GetEntryCount(); ++i )
        {
            if ( m_pContentsListBox->GetEntry( nullptr, i ) == pActEntry )
            {
                nSelectionPos = i;
                break;
            }
        }

        css::uno::Reference< css::graphic::XGraphic > backup =
            pEntry->GetBackupGraphic();

        css::uno::Sequence< css::uno::Reference< css::graphic::XGraphic > >
            aGraphicSeq( 1 );
        aGraphicSeq[ 0 ] = backup;

        css::uno::Sequence<OUString> aURLSeq { pEntry->GetCommand() };

        try
        {
            GetSaveInData()->GetImageManager()->replaceImages(
                SvxConfigPageHelper::GetImageType(), aURLSeq, aGraphicSeq );

            m_pContentsListBox->GetModel()->Remove( pActEntry );

            SvTreeListEntry* pNewLBEntry =
                InsertEntryIntoUI( pEntry, nSelectionPos );

            m_pContentsListBox->SetCheckButtonState( pNewLBEntry,
                pEntry->IsVisible() ?
                    SvButtonState::Checked : SvButtonState::Unchecked );

            m_pContentsListBox->Select( pNewLBEntry );
            m_pContentsListBox->MakeVisible( pNewLBEntry );

            // reset backup in entry
            pEntry->SetBackupGraphic(
                css::uno::Reference< css::graphic::XGraphic >() );

            GetSaveInData()->PersistChanges(
                GetSaveInData()->GetImageManager() );
        }
        catch ( css::uno::Exception& )
        {
            SAL_WARN("cui.customize", "Error resetting image");
        }
    }

    if ( bNeedsApply )
    {
        static_cast<ToolbarSaveInData*>( GetSaveInData())->ApplyToolbar( pToolbar );
        UpdateButtonStates();
    }
}

void SvxToolbarConfigPage::Init()
{
    // ensure that the UI is cleared before populating it
    m_pTopLevelListBox->Clear();
    m_pContentsListBox->Clear();

    ReloadTopLevelListBox();

    sal_Int32 nPos = 0;
    if ( !m_aURLToSelect.isEmpty() )
    {
        for ( sal_Int32 i = 0 ; i < m_pTopLevelListBox->GetEntryCount(); ++i )
        {
            SvxConfigEntry* pData =
                static_cast<SvxConfigEntry*>(m_pTopLevelListBox->GetEntryData( i ));

            if ( pData->GetCommand().equals( m_aURLToSelect ) )
            {
                nPos = i;
                break;
            }
        }

        // in future select the default toolbar: Standard
        m_aURLToSelect = ITEM_TOOLBAR_URL;
        m_aURLToSelect += "standardbar";
    }

    m_pTopLevelListBox->SelectEntryPos(nPos);
    m_pTopLevelListBox->GetSelectHdl().Call(*m_pTopLevelListBox);
}

SaveInData* SvxToolbarConfigPage::CreateSaveInData(
    const css::uno::Reference< css::ui::XUIConfigurationManager >& xCfgMgr,
    const css::uno::Reference< css::ui::XUIConfigurationManager >& xParentCfgMgr,
    const OUString& aModuleId,
    bool bDocConfig )
{
    return static_cast< SaveInData* >(
        new ToolbarSaveInData( xCfgMgr, xParentCfgMgr, aModuleId, bDocConfig ));
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, SelectToolbarEntry, SvTreeListBox *, void )
{
    UpdateButtonStates();
}

void SvxToolbarConfigPage::UpdateButtonStates()
{
    PopupMenu* pPopup = m_pModifyCommandButton->GetPopupMenu();
    pPopup->EnableItem(pPopup->GetItemId("toolrename"), false);
    pPopup->EnableItem(pPopup->GetItemId("toolrestore"), false);
    pPopup->EnableItem(pPopup->GetItemId("toolchange"), false);
    pPopup->EnableItem(pPopup->GetItemId("toolreset"), false);

    m_pDeleteCommandButton->Enable( false );
    m_pAddSeparatorButton->Enable( false );

    m_pDescriptionField->SetText("");

    SvTreeListEntry* selection = m_pContentsListBox->GetCurEntry();
    if ( m_pContentsListBox->GetEntryCount() == 0 || selection == nullptr )
    {
        return;
    }

    SvxConfigEntry* pEntryData = static_cast<SvxConfigEntry*>(selection->GetUserData());
    if ( pEntryData->IsSeparator() )
    {
        m_pDeleteCommandButton->Enable();
    }
    else
    {
        pPopup->EnableItem(pPopup->GetItemId("toolrename"));
        pPopup->EnableItem(pPopup->GetItemId("toolchange"));

        m_pDeleteCommandButton->Enable();
        m_pAddSeparatorButton->Enable();

        if ( !pEntryData->IsUserDefined() )
            pPopup->EnableItem(pPopup->GetItemId("toolrestore"));

        if ( pEntryData->IsIconModified() )
            pPopup->EnableItem(pPopup->GetItemId("toolreset"));

        m_pDescriptionField->SetText(pEntryData->GetHelpText());
    }
}

short SvxToolbarConfigPage::QueryReset()
{
    OUString msg = CuiResId( RID_SVXSTR_CONFIRM_TOOLBAR_RESET );

    OUString saveInName = m_pSaveInListBox->GetEntry(
        m_pSaveInListBox->GetSelectEntryPos() );

    OUString label = SvxConfigPageHelper::replaceSaveInName( msg, saveInName );

    ScopedVclPtrInstance< QueryBox > qbox( this, WB_YES_NO, label );

    return qbox->Execute();
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, SelectToolbar, ListBox&, void )
{
    m_pContentsListBox->Clear();

    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    if ( pToolbar == nullptr )
    {
        m_pModifyTopLevelButton->Enable( false );
        m_pModifyCommandButton->Enable( false );
        m_pAddCommandsButton->Enable( false );
        m_pAddSeparatorButton->Enable( false );
        m_pDeleteCommandButton->Enable( false );
        m_pResetTopLevelButton->Enable( false );
        m_pIconsOnlyRB->Enable( false );
        m_pTextOnlyRB->Enable( false );
        m_pIconsAndTextRB->Enable( false );

        return;
    }

    m_pModifyTopLevelButton->Enable();
    m_pModifyCommandButton->Enable();
    m_pAddCommandsButton->Enable();
    m_pResetTopLevelButton->Enable( !pToolbar->IsRenamable() );

    m_pIconsOnlyRB->Enable();
    m_pTextOnlyRB->Enable();
    m_pIconsAndTextRB->Enable();

    PopupMenu* pPopup = m_pModifyTopLevelButton->GetPopupMenu();

    pPopup->EnableItem(m_pMenu->GetItemId("modtooldelete"), pToolbar->IsDeletable());
    pPopup->EnableItem(m_pMenu->GetItemId("modtoolrename"), pToolbar->IsRenamable());

    switch( pToolbar->GetStyle() )
    {
        case 0:
        {
            m_pIconsOnlyRB->Check();
            break;
        }
        case 1:
        {
            m_pTextOnlyRB->Check();
            break;
        }
        case 2:
        {
            m_pIconsAndTextRB->Check();
            break;
        }
    }

    SvxEntries* pEntries = pToolbar->GetEntries();
    SvxEntries::const_iterator iter = pEntries->begin();

    for ( ; iter != pEntries->end(); ++iter )
    {
        SvxConfigEntry* pEntry = *iter;

        SvTreeListEntry* pNewLBEntry = InsertEntryIntoUI( pEntry );

        if(pEntry->IsSeparator())
            m_pContentsListBox->SetCheckButtonInvisible( pNewLBEntry );

        if (pEntry->IsBinding())
        {
            m_pContentsListBox->SetCheckButtonState( pNewLBEntry,
                pEntry->IsVisible() ? SvButtonState::Checked : SvButtonState::Unchecked );
        }
        else
        {
            m_pContentsListBox->SetCheckButtonState(
                pNewLBEntry, SvButtonState::Tristate );
        }
    }

    UpdateButtonStates();
}

IMPL_LINK( SvxToolbarConfigPage, StyleChangeHdl, Button*, pButton, void )
{
    sal_Int32 nSelectionPos = m_pTopLevelListBox->GetSelectEntryPos();

    SvxConfigEntry* pToolbar =
        static_cast<SvxConfigEntry*>(m_pTopLevelListBox->GetEntryData( nSelectionPos ));

    ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>( GetSaveInData() );

    if (pButton == m_pIconsOnlyRB)
    {
        pToolbar->SetStyle( 0 );
        pSaveInData->SetSystemStyle( m_xFrame, pToolbar->GetCommand(), 0 );

        m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );
    }
    else if (pButton == m_pTextOnlyRB)
    {
        pToolbar->SetStyle( 1 );
        pSaveInData->SetSystemStyle( m_xFrame, pToolbar->GetCommand(), 1 );

        m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );
    }
    else if (pButton == m_pIconsAndTextRB)
    {
        pToolbar->SetStyle( 2 );
        pSaveInData->SetSystemStyle( m_xFrame, pToolbar->GetCommand(), 2 );

        m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );
    }
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, NewToolbarHdl, Button *, void )
{
    OUString prefix = CuiResId( RID_SVXSTR_NEW_TOOLBAR );

    OUString aNewName =
        SvxConfigPageHelper::generateCustomName( prefix, GetSaveInData()->GetEntries() );

    OUString aNewURL =
        SvxConfigPageHelper::generateCustomURL( GetSaveInData()->GetEntries() );

    VclPtrInstance< SvxNewToolbarDialog > pNameDialog( nullptr, aNewName );

    for ( sal_Int32 i = 0 ; i < m_pSaveInListBox->GetEntryCount(); ++i )
    {
        SaveInData* pData =
            static_cast<SaveInData*>(m_pSaveInListBox->GetEntryData( i ));

        const sal_Int32 nInsertPos = pNameDialog->m_pSaveInListBox->InsertEntry(
            m_pSaveInListBox->GetEntry( i ) );

        pNameDialog->m_pSaveInListBox->SetEntryData( nInsertPos, pData );
    }

    pNameDialog->m_pSaveInListBox->SelectEntryPos(
        m_pSaveInListBox->GetSelectEntryPos() );

    if ( pNameDialog->Execute() == RET_OK )
    {
        aNewName = pNameDialog->GetName();

        sal_Int32 nInsertPos = pNameDialog->m_pSaveInListBox->GetSelectEntryPos();

        ToolbarSaveInData* pData = static_cast<ToolbarSaveInData*>(
            pNameDialog->m_pSaveInListBox->GetEntryData( nInsertPos ));

        if ( GetSaveInData() != pData )
        {
            m_pSaveInListBox->SelectEntryPos( nInsertPos );
            m_pSaveInListBox->GetSelectHdl().Call(*m_pSaveInListBox);
        }

        SvxConfigEntry* pToolbar =
            new SvxConfigEntry( aNewName, aNewURL, true );

        pToolbar->SetUserDefined();
        pToolbar->SetMain();

        pData->CreateToolbar( pToolbar );

        nInsertPos = m_pTopLevelListBox->InsertEntry( pToolbar->GetName() );
        m_pTopLevelListBox->SetEntryData( nInsertPos, pToolbar );
        m_pTopLevelListBox->SelectEntryPos( nInsertPos );
        m_pTopLevelListBox->GetSelectHdl().Call(*m_pTopLevelListBox);

        pData->SetModified();
    }
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, AddCommandsHdl, Button *, void )
{
    if ( m_pSelectorDlg == nullptr )
    {
        // Create Script Selector which shows slot commands
        m_pSelectorDlg = VclPtr<SvxScriptSelectorDialog>::Create( this, true, m_xFrame );

        // Position the Script Selector over the Add button so it is
        // beside the menu contents list and does not obscure it
        m_pSelectorDlg->SetPosPixel( m_pAddCommandsButton->GetPosPixel() );

        m_pSelectorDlg->SetAddHdl(
            LINK( this, SvxToolbarConfigPage, AddFunctionHdl ) );
    }

    m_pSelectorDlg->SetImageProvider( GetSaveInData() );

    m_pSelectorDlg->Execute();
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, AddSeparatorHdl, Button *, void )
{
    // get currently selected toolbar
    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    SvxConfigEntry* pNewEntryData = new SvxConfigEntry;
    pNewEntryData->SetUserDefined();

    SvTreeListEntry* pNewLBEntry = InsertEntry( pNewEntryData );

    m_pContentsListBox->SetCheckButtonInvisible( pNewLBEntry );
    m_pContentsListBox->SetCheckButtonState(
        pNewLBEntry, SvButtonState::Tristate );

    static_cast<ToolbarSaveInData*>( GetSaveInData())->ApplyToolbar( pToolbar );
    UpdateButtonStates();
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, DeleteCommandHdl, Button *, void )
{
    DeleteSelectedContent();
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, ResetTopLevelHdl, Button *, void )
{
    sal_Int32 nSelectionPos = m_pTopLevelListBox->GetSelectEntryPos();

    SvxConfigEntry* pToolbar =
        static_cast<SvxConfigEntry*>(m_pTopLevelListBox->GetEntryData( nSelectionPos ));

    ScopedVclPtrInstance<MessageDialog> qbox(this,
        CuiResId(RID_SVXSTR_CONFIRM_RESTORE_DEFAULT), VclMessageType::Question, VclButtonsType::YesNo);

    if ( qbox->Execute() == RET_YES )
    {
        ToolbarSaveInData* pSaveInData_ =
        static_cast<ToolbarSaveInData*>(GetSaveInData());

        pSaveInData_->RestoreToolbar( pToolbar );

        m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );
    }
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, AddFunctionHdl, SvxScriptSelectorDialog&, void )
{
    AddFunction();
}

void SvxToolbarConfigPage::AddFunction(
    SvTreeListEntry* pTarget, bool bFront )
{
    SvTreeListEntry* pNewLBEntry =
        SvxConfigPage::AddFunction( pTarget, bFront, true/*bAllowDuplicates*/ );

    SvxConfigEntry* pEntry = static_cast<SvxConfigEntry*>(pNewLBEntry->GetUserData());

    if ( pEntry->IsBinding() )
    {
        pEntry->SetVisible( true );
        m_pContentsListBox->SetCheckButtonState(
            pNewLBEntry, SvButtonState::Checked );
    }
    else
    {
        m_pContentsListBox->SetCheckButtonState(
            pNewLBEntry, SvButtonState::Tristate );
    }

    // get currently selected toolbar and apply change
    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    if ( pToolbar != nullptr )
    {
        static_cast<ToolbarSaveInData*>( GetSaveInData() )->ApplyToolbar( pToolbar );
    }
}

SvxToolbarEntriesListBox::SvxToolbarEntriesListBox(vcl::Window* pParent, SvxToolbarConfigPage* pPg)
    : SvxMenuEntriesListBox(pParent, pPg)
    , pPage(pPg)
{
    m_pButtonData = new SvLBoxButtonData( this );
    BuildCheckBoxButtonImages( m_pButtonData );
    EnableCheckButton( m_pButtonData );
}

SvxToolbarEntriesListBox::~SvxToolbarEntriesListBox()
{
    disposeOnce();
}

void SvxToolbarEntriesListBox::dispose()
{
    delete m_pButtonData;
    m_pButtonData = nullptr;

    pPage.clear();
    SvxMenuEntriesListBox::dispose();
}

void SvxToolbarEntriesListBox::BuildCheckBoxButtonImages( SvLBoxButtonData* pData )
{
    // Build checkbox images according to the current application
    // settings. This is necessary to be able to have correct colors
    // in all color modes, like high contrast.
    const AllSettings& rSettings = Application::GetSettings();

    ScopedVclPtrInstance< VirtualDevice > pVDev;
    Size            aSize( 26, 20 );

    pVDev->SetOutputSizePixel( aSize );

    Image aImage = GetSizedImage( *pVDev.get(), aSize,
        CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Default ));

    // Fill button data struct with new images
    pData->SetImage(SvBmp::UNCHECKED,     aImage);
    pData->SetImage(SvBmp::CHECKED,       GetSizedImage( *pVDev.get(), aSize, CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Checked )) );
    pData->SetImage(SvBmp::HICHECKED,     GetSizedImage( *pVDev.get(), aSize, CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Checked | DrawButtonFlags::Pressed )) );
    pData->SetImage(SvBmp::HIUNCHECKED,   GetSizedImage( *pVDev.get(), aSize, CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Default | DrawButtonFlags::Pressed)) );
    pData->SetImage(SvBmp::TRISTATE,      GetSizedImage( *pVDev.get(), aSize, Image() ) ); // Use tristate bitmaps to have no checkbox for separator entries
    pData->SetImage(SvBmp::HITRISTATE,    GetSizedImage( *pVDev.get(), aSize, Image() ) );

    // Get image size
    m_aCheckBoxImageSizePixel = aImage.GetSizePixel();
}

Image SvxToolbarEntriesListBox::GetSizedImage(
    VirtualDevice& rVDev, const Size& aNewSize, const Image& aImage )
{
    // Create new checkbox images for treelistbox. They must have a
    // decent width to have a clear column for the visibility checkbox.

    // Standard transparent color is light magenta as is won't be
    // used for other things
    Color   aFillColor( COL_LIGHTMAGENTA );

    // Position image at the center of (width-2),(height) rectangle.
    // We need 2 pixels to have a bigger border to the next button image
    sal_uInt16  nPosX = std::max( (sal_uInt16) (((( aNewSize.Width() - 2 ) - aImage.GetSizePixel().Width() ) / 2 ) - 1), (sal_uInt16) 0 );
    sal_uInt16  nPosY = std::max( (sal_uInt16) (((( aNewSize.Height() - 2 ) - aImage.GetSizePixel().Height() ) / 2 ) + 1), (sal_uInt16) 0 );
    Point   aPos( nPosX > 0 ? nPosX : 0, nPosY > 0 ? nPosY : 0 );
    rVDev.SetFillColor( aFillColor );
    rVDev.SetLineColor( aFillColor );
    rVDev.DrawRect( ::tools::Rectangle( Point(), aNewSize ));
    rVDev.DrawImage( aPos, aImage );

    // Draw separator line 2 pixels left from the right border
    Color aLineColor = GetDisplayBackground().GetColor().IsDark() ? Color( COL_WHITE ) : Color( COL_BLACK );
    rVDev.SetLineColor( aLineColor );
    rVDev.DrawLine( Point( aNewSize.Width()-3, 0 ), Point( aNewSize.Width()-3, aNewSize.Height()-1 ));

    // Create new image that uses the fillcolor as transparent
    return Image(BitmapEx(rVDev.GetBitmap(Point(), aNewSize), aFillColor));
}

void SvxToolbarEntriesListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    SvTreeListBox::DataChanged( rDCEvt );

    if (( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) &&
        ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ))
    {
        BuildCheckBoxButtonImages( m_pButtonData );
        Invalidate();
    }
}


void SvxToolbarEntriesListBox::ChangeVisibility( SvTreeListEntry* pEntry )
{
    if ( pEntry != nullptr )
    {
        SvxConfigEntry* pEntryData =
            static_cast<SvxConfigEntry*>(pEntry->GetUserData());

        if ( pEntryData->IsBinding() )
        {
            pEntryData->SetVisible( !pEntryData->IsVisible() );

            SvxConfigEntry* pToolbar = pPage->GetTopLevelSelection();

            ToolbarSaveInData* pToolbarSaveInData = static_cast<ToolbarSaveInData*>(
                pPage->GetSaveInData() );

               pToolbarSaveInData->ApplyToolbar( pToolbar );

            SetCheckButtonState( pEntry, pEntryData->IsVisible() ?
                SvButtonState::Checked : SvButtonState::Unchecked );
        }
    }
}

void SvxToolbarEntriesListBox::CheckButtonHdl()
{
    ChangeVisibility( GetHdlEntry() );
}

void SvxToolbarEntriesListBox::KeyInput( const KeyEvent& rKeyEvent )
{
    // space key will change visibility of toolbar items
    if ( rKeyEvent.GetKeyCode() == KEY_SPACE )
    {
        ChangeVisibility( GetCurEntry() );
    }
    else
    {
        // pass on to superclass
        SvxMenuEntriesListBox::KeyInput( rKeyEvent );
    }
}

TriState SvxToolbarEntriesListBox::NotifyMoving(
    SvTreeListEntry* pTarget, SvTreeListEntry* pSource,
    SvTreeListEntry*& rpNewParent, sal_uLong& rNewChildPos)
{
    TriState result = SvxMenuEntriesListBox::NotifyMoving(
        pTarget, pSource, rpNewParent, rNewChildPos );

    if ( result )
    {
        // Instant Apply changes to UI
        SvxConfigEntry* pToolbar = pPage->GetTopLevelSelection();
        if ( pToolbar != nullptr )
        {
            ToolbarSaveInData* pSaveInData =
                static_cast<ToolbarSaveInData*>( pPage->GetSaveInData() );
            pSaveInData->ApplyToolbar( pToolbar );
        }
    }

    return result;
}

TriState SvxToolbarEntriesListBox::NotifyCopying(
    SvTreeListEntry*  pTarget,
    SvTreeListEntry*  pSource,
    SvTreeListEntry*& rpNewParent,
    sal_uLong&      rNewChildPos)
{
    (void)pSource;
    (void)rpNewParent;
    (void)rNewChildPos;

    if ( !m_bIsInternalDrag )
    {
        // if the target is NULL then add function to the start of the list
        static_cast<SvxToolbarConfigPage*>(pPage.get())->AddFunction( pTarget, pTarget == nullptr );

        // Instant Apply changes to UI
        SvxConfigEntry* pToolbar = pPage->GetTopLevelSelection();
        if ( pToolbar != nullptr )
        {
            ToolbarSaveInData* pSaveInData =
                static_cast<ToolbarSaveInData*>( pPage->GetSaveInData() );
            pSaveInData->ApplyToolbar( pToolbar );
        }

        // AddFunction already adds the listbox entry so return TRISTATE_FALSE
        // to stop another listbox entry being added
        return TRISTATE_FALSE;
    }

    // Copying is only allowed from external controls, not within the listbox
    return TRISTATE_FALSE;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
