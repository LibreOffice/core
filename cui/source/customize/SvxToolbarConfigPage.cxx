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
#include <helpids.h>
#include <strings.hrc>

#include <acccfg.hxx>
#include <cfg.hxx>
#include <SvxToolbarConfigPage.hxx>
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

SvxToolbarConfigPage::SvxToolbarConfigPage(vcl::Window *pParent, const SfxItemSet& rSet)
    : SvxConfigPage(pParent, rSet)
{
    SetHelpId( HID_SVX_CONFIG_TOOLBAR );

    m_pContentsListBox = VclPtr<SvxToolbarEntriesListBox>::Create(m_pEntries, this);
    m_pContentsListBox->set_grid_left_attach(0);
    m_pContentsListBox->set_grid_top_attach(0);
    m_pContentsListBox->set_hexpand(true);
    m_pContentsListBox->set_vexpand(true);
    m_pContentsListBox->Show();

    m_pTopLevelListBox->SetHelpId ( HID_SVX_TOPLEVELLISTBOX );
    m_pContentsListBox->SetHelpId( HID_SVX_CONFIG_TOOLBAR_CONTENTS );
    m_pSaveInListBox->SetHelpId( HID_SVX_SAVE_IN );
    m_pMoveUpButton->SetHelpId( HID_SVX_UP_TOOLBAR_ITEM );
    m_pMoveDownButton->SetHelpId( HID_SVX_DOWN_TOOLBAR_ITEM );
    m_pDescriptionField->SetHelpId ( HID_SVX_DESCFIELD );

    m_pTopLevelListBox->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, SelectToolbar ) );
    m_pContentsListBox->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, SelectToolbarEntry ) );
    m_pCommandCategoryListBox->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, SelectCategory ) );

    m_pGearBtn->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, GearHdl ) );

    m_pMoveUpButton->SetClickHdl ( LINK( this, SvxToolbarConfigPage, MoveHdl) );
    m_pMoveDownButton->SetClickHdl ( LINK( this, SvxToolbarConfigPage, MoveHdl) );
    // Always enable Up and Down buttons
    // added for issue i53677 by shizhoubo
    m_pMoveDownButton->Enable();
    m_pMoveUpButton->Enable();

    m_pAddCommandButton->SetClickHdl( LINK( this, SvxToolbarConfigPage, AddCommandHdl ) );
    m_pRemoveCommandButton->SetClickHdl( LINK( this, SvxToolbarConfigPage, RemoveCommandHdl ) );

    m_pInsertBtn->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, InsertHdl ) );
    m_pModifyBtn->SetSelectHdl(
        LINK( this, SvxToolbarConfigPage, ModifyItemHdl ) );
    m_pResetBtn->SetClickHdl(
        LINK( this, SvxToolbarConfigPage, ResetToolbarHdl ) );

    // "Insert Submenu" is irrelevant to the toolbars
    PopupMenu* pPopup = m_pInsertBtn->GetPopupMenu();
    pPopup->EnableItem(OString( "insertsubmenu"), false );
    pPopup->RemoveDisabledEntries();

    // Gear menu's "Move" action is irrelevant to the toolbars
    pPopup = m_pGearBtn->GetPopupMenu();
    pPopup->EnableItem("gear_move", false);

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

    SvxConfigPage::dispose();
}

void SvxToolbarConfigPage::DeleteSelectedTopLevel()
{
    const sal_Int32 nSelectionPos = m_pTopLevelListBox->GetSelectedEntryPos();
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
            std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                           VclMessageType::Question, VclButtonsType::YesNo,
                                                           CuiResId(RID_SXVSTR_CONFIRM_DELETE_TOOLBAR)));
            if (xQueryBox->run() == RET_YES)
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

    m_pCommandCategoryListBox->Init(
        comphelper::getProcessComponentContext(),
        m_xFrame,
        vcl::CommandInfoProvider::GetModuleIdentifier(m_xFrame));
    m_pCommandCategoryListBox->categorySelected( m_pFunctions, OUString(), GetSaveInData() );
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

IMPL_LINK( SvxToolbarConfigPage, GearHdl, MenuButton *, pButton, void )
{
    OString sIdent = pButton->GetCurItemIdent();
    SvxConfigEntry* pCurrentToolbar = GetTopLevelSelection();

    if (sIdent == "gear_add")
    {
        OUString prefix = CuiResId( RID_SVXSTR_NEW_TOOLBAR );

        OUString aNewName =
            SvxConfigPageHelper::generateCustomName( prefix, GetSaveInData()->GetEntries() );

        OUString aNewURL =
            SvxConfigPageHelper::generateCustomURL( GetSaveInData()->GetEntries() );

        SvxNewToolbarDialog aNameDialog(GetFrameWeld(), aNewName);

        // Reflect the actual m_pSaveInListBox into the new toolbar dialog
        for (sal_Int32 i = 0; i < m_pSaveInListBox->GetEntryCount(); ++i)
            aNameDialog.m_xSaveInListBox->append_text(m_pSaveInListBox->GetEntry(i));

        aNameDialog.m_xSaveInListBox->set_active(m_pSaveInListBox->GetSelectedEntryPos());

        if (aNameDialog.run() == RET_OK)
        {
            aNewName = aNameDialog.GetName();

            // Where to save the new toolbar? (i.e. Modulewise or documentwise)
            int nInsertPos = aNameDialog.m_xSaveInListBox->get_active();

            ToolbarSaveInData* pData =
                static_cast<ToolbarSaveInData*>(
                    m_pSaveInListBox->GetEntryData( nInsertPos ) );

            if ( GetSaveInData() != pData )
            {
                m_pSaveInListBox->SelectEntryPos( nInsertPos );
                m_pSaveInListBox->GetSelectHdl().Call(*m_pSaveInListBox);
            }

            SvxConfigEntry* pToolbar =
                new SvxConfigEntry( aNewName, aNewURL, true, false );

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
    else if (sIdent == "gear_delete")
    {
        if ( pCurrentToolbar && pCurrentToolbar->IsDeletable() )
        {
            DeleteSelectedTopLevel();
            UpdateButtonStates();
        }
    }
    else if (sIdent == "gear_rename")
    {
        sal_Int32 nSelectionPos = m_pTopLevelListBox->GetSelectedEntryPos();
        SvxConfigEntry* pToolbar =
            static_cast<SvxConfigEntry*>(m_pTopLevelListBox->GetEntryData( nSelectionPos ));
        ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>( GetSaveInData() );

        //Rename the toolbar
        OUString sCurrentName( SvxConfigPageHelper::stripHotKey( pToolbar->GetName() ) );
        OUString sDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        SvxNameDialog aNameDialog( GetFrameWeld(), sCurrentName, sDesc );
        aNameDialog.set_help_id( HID_SVX_CONFIG_RENAME_TOOLBAR );
        aNameDialog.set_title( CuiResId( RID_SVXSTR_RENAME_TOOLBAR ) );

        if ( aNameDialog.run() == RET_OK )
        {
            OUString sNewName = aNameDialog.GetName();

            if (sCurrentName == sNewName)
                return;

            pToolbar->SetName( sNewName );
            pSaveInData->ApplyToolbar( pToolbar );

            // have to use remove and insert to change the name
            m_pTopLevelListBox->RemoveEntry( nSelectionPos );
            nSelectionPos =
                    m_pTopLevelListBox->InsertEntry( sNewName, nSelectionPos );
            m_pTopLevelListBox->SetEntryData( nSelectionPos, pToolbar );
            m_pTopLevelListBox->SelectEntryPos( nSelectionPos );
        }
    }
    else if (sIdent == "gear_iconOnly" || sIdent == "gear_textOnly" || sIdent == "gear_iconAndText")
    {
        ToolbarSaveInData* pSaveInData = static_cast<ToolbarSaveInData*>( GetSaveInData() );

        if (pCurrentToolbar == nullptr || pSaveInData == nullptr)
        {
            SAL_WARN("cui.customize", "NULL toolbar or savein data");
            return;
        }

        sal_Int32 nStyle = 0;
        if (sIdent == "gear_iconOnly")
            nStyle = 0;
        else if (sIdent == "gear_textOnly")
            nStyle = 1;
        else if (sIdent == "gear_iconAndText")
            nStyle = 2;

        pCurrentToolbar->SetStyle( nStyle );
        pSaveInData->SetSystemStyle( m_xFrame, pCurrentToolbar->GetCommand(), nStyle );

        m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );
    }
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown gear menu option: " << sIdent);
        return;
    }
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, SelectCategory, ListBox&, void )
{
    OUString aSearchTerm( m_pSearchEdit->GetText() );

    m_pCommandCategoryListBox->categorySelected( m_pFunctions, aSearchTerm, GetSaveInData() );
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, AddCommandHdl, Button *, void )
{
    AddFunction();
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, RemoveCommandHdl, Button *, void )
{
    DeleteSelectedContent();
}

IMPL_LINK( SvxToolbarConfigPage, InsertHdl, MenuButton *, pButton, void )
{
    OString sIdent = pButton->GetCurItemIdent();

    if (sIdent == "insertseparator")
    {
        // Get the currently selected toolbar
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
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown insert option: " << sIdent);
        return;
    }
}

IMPL_LINK( SvxToolbarConfigPage, ModifyItemHdl, MenuButton *, pButton, void )
{
    bool bNeedsApply = false;

    // get currently selected toolbar
    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    OString sIdent = pButton->GetCurItemIdent();

    if (sIdent.isEmpty() || pToolbar == nullptr)
    {
        SAL_WARN("cui.customize", "No toolbar selected, or empty sIdent!");
        return;
    }

    if (sIdent == "renameItem")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        OUString aNewName( SvxConfigPageHelper::stripHotKey( pEntry->GetName() ) );
        OUString aDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        SvxNameDialog aNameDialog(GetFrameWeld(), aNewName, aDesc);
        aNameDialog.set_help_id(HID_SVX_CONFIG_RENAME_TOOLBAR_ITEM);
        aNameDialog.set_title(CuiResId(RID_SVXSTR_RENAME_TOOLBAR));

        if (aNameDialog.run() == RET_OK)
        {
            aNewName = aNameDialog.GetName();

            if( aNewName.isEmpty() )    // tdf#80758 - Accelerator character ("~") is passed as
                pEntry->SetName( "~" ); // the button name in case of empty values.
            else
                pEntry->SetName( aNewName );

            m_pContentsListBox->SetEntryText( pActEntry, aNewName );
            bNeedsApply = true;
        }
    }
    else if (sIdent == "changeIcon")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        // Position of entry within the list
        // TODO: Add a GetSelectionPos() method to the SvTreeListBox class
        sal_uInt16 nSelectionPos = m_pContentsListBox->GetModel()->GetAbsPos( pActEntry );

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
                catch ( css::uno::Exception& e)
                {
                    SAL_WARN("cui.customize", "Error replacing image: " << e);
                }
            }
        }
    }
    else if (sIdent == "resetIcon")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        // Position of entry within the list
        // TODO: Add a GetSelectionPos() method to the SvTreeListBox class
        sal_uInt16 nSelectionPos = m_pContentsListBox->GetModel()->GetAbsPos( pActEntry );

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
        catch ( css::uno::Exception& e )
        {
            SAL_WARN("cui.customize", "Error resetting image: " << e);
        }
    }
    else if (sIdent == "restoreItem")
    {
        SvTreeListEntry* pActEntry = m_pContentsListBox->GetCurEntry();
        SvxConfigEntry* pEntry =
            static_cast<SvxConfigEntry*>(pActEntry->GetUserData());

        // Position of entry within the list
        // TODO: Add a GetSelectionPos() method to the SvTreeListBox class
        sal_uInt16 nSelectionPos = m_pContentsListBox->GetModel()->GetAbsPos( pActEntry );

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

            m_pContentsListBox->RemoveEntry( pActEntry );

            SvTreeListEntry* pNewLBEntry =
                InsertEntryIntoUI( pEntry, nSelectionPos );

            m_pContentsListBox->SetCheckButtonState( pNewLBEntry,
                pEntry->IsVisible() ?
                    SvButtonState::Checked : SvButtonState::Unchecked );

            m_pContentsListBox->Select( pNewLBEntry );
            m_pContentsListBox->MakeVisible( pNewLBEntry );

            bNeedsApply = true;
        }
        catch ( css::uno::Exception& e )
        {
            SAL_WARN("cui.customize", "Error restoring image: " << e);
        }
    }
    else
    {
        //This block should never be reached
        SAL_WARN("cui.customize", "Unknown insert option: " << sIdent);
        return;
    }

    if ( bNeedsApply )
    {
        static_cast<ToolbarSaveInData*>( GetSaveInData())->ApplyToolbar( pToolbar );
        UpdateButtonStates();
    }
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, ResetToolbarHdl, Button *, void )
{
    sal_Int32 nSelectionPos = m_pTopLevelListBox->GetSelectedEntryPos();

    SvxConfigEntry* pToolbar =
        static_cast<SvxConfigEntry*>(m_pTopLevelListBox->GetEntryData( nSelectionPos ));

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   CuiResId(RID_SVXSTR_CONFIRM_RESTORE_DEFAULT)));
    if (xQueryBox->run() == RET_YES)
    {
        ToolbarSaveInData* pSaveInData =
            static_cast<ToolbarSaveInData*>(GetSaveInData());

        pSaveInData->RestoreToolbar( pToolbar );

        m_pTopLevelListBox->GetSelectHdl().Call( *m_pTopLevelListBox );
    }
}

void SvxToolbarConfigPage::UpdateButtonStates()
{
    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    SvTreeListEntry* selection = m_pContentsListBox->GetCurEntry();

    bool  bIsSeparator =
        selection && static_cast<SvxConfigEntry*>(selection->GetUserData())->IsSeparator();
    bool bIsValidSelection =
        !(m_pContentsListBox->GetEntryCount() == 0 || selection == nullptr);

    m_pMoveUpButton->Enable( bIsValidSelection );
    m_pMoveDownButton->Enable( bIsValidSelection );

    m_pRemoveCommandButton->Enable( bIsValidSelection );

    m_pModifyBtn->Enable( bIsValidSelection && !bIsSeparator );

    // Handle the gear button
    PopupMenu* pPopup = m_pGearBtn->GetPopupMenu();
    // "gear_add" option is always enabled
    pPopup->EnableItem( "gear_delete", pToolbar && pToolbar->IsDeletable() );
    pPopup->EnableItem( "gear_rename", pToolbar && pToolbar->IsRenamable() );
}

short SvxToolbarConfigPage::QueryReset()
{
    OUString msg = CuiResId( RID_SVXSTR_CONFIRM_TOOLBAR_RESET );

    OUString saveInName = m_pSaveInListBox->GetEntry(
        m_pSaveInListBox->GetSelectedEntryPos() );

    OUString label = SvxConfigPageHelper::replaceSaveInName( msg, saveInName );

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   label));
    return xQueryBox->run();
}

IMPL_LINK_NOARG( SvxToolbarConfigPage, SelectToolbar, ListBox&, void )
{
    m_pContentsListBox->Clear();

    SvxConfigEntry* pToolbar = GetTopLevelSelection();
    if ( pToolbar == nullptr )
    {
        //TODO: Disable related buttons
        m_pInsertBtn->Enable( false );
        m_pResetBtn->Enable( false );
        m_pGearBtn->Enable( false );

        return;
    }
    else
    {
        m_pInsertBtn->Enable();
        m_pResetBtn->Enable();
        m_pGearBtn->Enable();
    }

    PopupMenu* pGearMenu = m_pGearBtn->GetPopupMenu();
    switch( pToolbar->GetStyle() )
    {
        case 0:
        {
            pGearMenu->CheckItem( "gear_iconOnly" );
            break;
        }
        case 1:
        {
            pGearMenu->CheckItem( "gear_textOnly" );
            break;
        }
        case 2:
        {
            pGearMenu->CheckItem( "gear_iconAndText" );
            break;
        }
    }

    SvxEntries* pEntries = pToolbar->GetEntries();
    for (auto const& entry : *pEntries)
    {
        SvTreeListEntry* pNewLBEntry = InsertEntryIntoUI(entry);

        if(entry->IsSeparator())
            m_pContentsListBox->SetCheckButtonInvisible( pNewLBEntry );

        if (entry->IsBinding())
        {
            m_pContentsListBox->SetCheckButtonState( pNewLBEntry,
                entry->IsVisible() ? SvButtonState::Checked : SvButtonState::Unchecked );
        }
        else
        {
            m_pContentsListBox->SetCheckButtonState(
                pNewLBEntry, SvButtonState::Tristate );
        }
    }

    UpdateButtonStates();
}

void SvxToolbarConfigPage::AddFunction(
    SvTreeListEntry* pTarget, bool bFront )
{
    SvxConfigEntry* pToolbar = GetTopLevelSelection();

    if (pToolbar == nullptr)
        return;

    // Add the command to the contents listbox of the selected toolbar
    SvTreeListEntry* pNewLBEntry =
        SvxConfigPage::AddFunction( pTarget, bFront, true/*bAllowDuplicates*/ );

    if (pNewLBEntry == nullptr)
        return;

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

    // Changes are not visible on the toolbar until this point
    // TODO: Figure out a way to show the changes on the toolbar, but revert if
    //       the dialog is closed by pressing "Cancel"
    // get currently selected toolbar and apply change
    if ( pToolbar != nullptr )
    {
        static_cast<ToolbarSaveInData*>( GetSaveInData() )->ApplyToolbar( pToolbar );
    }
}

SvxToolbarEntriesListBox::SvxToolbarEntriesListBox(vcl::Window* pParent, SvxToolbarConfigPage* pPg)
    : SvxMenuEntriesListBox(pParent, pPg)
    , pPage(pPg)
{
    m_pButtonData.reset(new SvLBoxButtonData( this ));
    BuildCheckBoxButtonImages( m_pButtonData.get() );
    EnableCheckButton( m_pButtonData.get() );
}

SvxToolbarEntriesListBox::~SvxToolbarEntriesListBox()
{
    disposeOnce();
}

void SvxToolbarEntriesListBox::dispose()
{
    m_pButtonData.reset();

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

    Image aImage = GetSizedImage( *pVDev, aSize,
        CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Default ));

    // Fill button data struct with new images
    pData->SetImage(SvBmp::UNCHECKED,     aImage);
    pData->SetImage(SvBmp::CHECKED,       GetSizedImage( *pVDev, aSize, CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Checked )) );
    pData->SetImage(SvBmp::HICHECKED,     GetSizedImage( *pVDev, aSize, CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Checked | DrawButtonFlags::Pressed )) );
    pData->SetImage(SvBmp::HIUNCHECKED,   GetSizedImage( *pVDev, aSize, CheckBox::GetCheckImage( rSettings, DrawButtonFlags::Default | DrawButtonFlags::Pressed)) );
    pData->SetImage(SvBmp::TRISTATE,      GetSizedImage( *pVDev, aSize, Image() ) ); // Use tristate bitmaps to have no checkbox for separator entries
    pData->SetImage(SvBmp::HITRISTATE,    GetSizedImage( *pVDev, aSize, Image() ) );

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
    sal_uInt16  nPosX = std::max( static_cast<sal_uInt16>(((( aNewSize.Width() - 2 ) - aImage.GetSizePixel().Width() ) / 2 ) - 1), sal_uInt16(0) );
    sal_uInt16  nPosY = std::max( static_cast<sal_uInt16>(((( aNewSize.Height() - 2 ) - aImage.GetSizePixel().Height() ) / 2 ) + 1), sal_uInt16(0) );
    Point   aPos( std::max<sal_uInt16>(nPosX, 0), std::max<sal_uInt16>(nPosY, 0) );
    rVDev.SetFillColor( aFillColor );
    rVDev.SetLineColor( aFillColor );
    rVDev.DrawRect( ::tools::Rectangle( Point(), aNewSize ));
    rVDev.DrawImage( aPos, aImage );

    // Draw separator line 2 pixels left from the right border
    Color aLineColor = GetDisplayBackground().GetColor().IsDark() ? COL_WHITE : COL_BLACK;
    rVDev.SetLineColor( aLineColor );
    rVDev.DrawLine( Point( aNewSize.Width()-3, 0 ), Point( aNewSize.Width()-3, aNewSize.Height()-1 ));

    // Create new image that uses the fillcolor as transparent
    return Image(BitmapEx(rVDev.GetBitmapEx(Point(), aNewSize).GetBitmap(), aFillColor));
}

void SvxToolbarEntriesListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    SvTreeListBox::DataChanged( rDCEvt );

    if (( rDCEvt.GetType() == DataChangedEventType::SETTINGS ) &&
        ( rDCEvt.GetFlags() & AllSettingsFlags::STYLE ))
    {
        BuildCheckBoxButtonImages( m_pButtonData.get() );
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
    SvTreeListEntry*,
    SvTreeListEntry*&,
    sal_uLong&)
{

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
