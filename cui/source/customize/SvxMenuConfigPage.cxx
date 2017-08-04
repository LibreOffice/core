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
#include <strings.hrc>
#include "helpids.h"

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

    m_pFunctionsListBox = VclPtr<SvxMenuEntriesListBox>::Create(m_pFunctions, this);
    m_pFunctionsListBox->set_grid_left_attach(0);
    m_pFunctionsListBox->set_grid_top_attach(0);
    m_pFunctionsListBox->set_hexpand(true);
    m_pFunctionsListBox->set_vexpand(true);
    m_pFunctionsListBox->Show();

    m_pTopLevelListBox->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectMenu ) );

    m_pContentsListBox->SetSelectHdl(
        LINK( this, SvxMenuConfigPage, SelectMenuEntry ) );

    m_pMoveUpButton->SetClickHdl ( LINK( this, SvxConfigPage, MoveHdl) );
    m_pMoveDownButton->SetClickHdl ( LINK( this, SvxConfigPage, MoveHdl) );
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

    m_pCommandCategoryListBox->Clear();
    m_pCommandCategoryListBox->FillCategoryEntries(
        comphelper::getProcessComponentContext(),
        m_xFrame,
        vcl::CommandInfoProvider::GetModuleIdentifier(m_xFrame));
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

    if ( m_pContentsListBox->GetEntryCount() == 0 || selection == nullptr )
    {
        m_pMoveUpButton->Enable( false );
        m_pMoveDownButton->Enable( false );

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
        m_pDescriptionField->SetText("");
    }
    else
    {
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

    if ( pMenuData )
    {
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
