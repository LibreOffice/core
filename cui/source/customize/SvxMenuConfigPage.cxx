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

#include <vcl/weld.hxx>
#include <vcl/svapp.hxx>
#include <vcl/commandevent.hxx>

#include <strings.hrc>
#include <helpids.h>

#include <cfg.hxx>
#include <SvxMenuConfigPage.hxx>
#include <SvxConfigPageHelper.hxx>
#include <dialmgr.hxx>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/ui/ImageType.hpp>

#include <dlgname.hxx>

SvxMenuConfigPage::SvxMenuConfigPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet, bool bIsMenuBar)
    : SvxConfigPage(pPage, pController, rSet)
    , m_bIsMenuBar(bIsMenuBar)
{
    m_xGearBtn = m_xBuilder->weld_menu_button("menugearbtn");
    m_xGearBtn->show();
    m_xContentsListBox.reset(new SvxMenuEntriesListBox(m_xBuilder->weld_tree_view("menucontents"), this));
    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();
    m_xDropTargetHelper.reset(new SvxConfigPageFunctionDropTarget(*this, rTreeView));
    rTreeView.connect_size_allocate(LINK(this, SvxMenuConfigPage, MenuEntriesSizeAllocHdl));
    Size aSize(m_xFunctions->get_size_request());
    rTreeView.set_size_request(aSize.Width(), aSize.Height());
    MenuEntriesSizeAllocHdl(aSize);
    rTreeView.set_hexpand(true);
    rTreeView.set_vexpand(true);
    rTreeView.show();

    rTreeView.connect_changed(
        LINK( this, SvxMenuConfigPage, SelectMenuEntry ) );
    rTreeView.connect_popup_menu( LINK( this, SvxMenuConfigPage, ContentContextMenuHdl ) );

    m_xFunctions->get_widget().connect_popup_menu(
        LINK( this, SvxMenuConfigPage, FunctionContextMenuHdl ) );

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

void SvxMenuConfigPage::ListModified()
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

    int nExpectedSize = 16;

    int nStandardImageColWidth = rTreeView.get_checkbox_column_width();
    int nMargin = nStandardImageColWidth - nExpectedSize;
    if (nMargin < 16)
        nMargin = 16;

    if (SvxConfigPageHelper::GetImageType() & css::ui::ImageType::SIZE_LARGE)
        nExpectedSize = 24;
    else if (SvxConfigPageHelper::GetImageType() & css::ui::ImageType::SIZE_32)
        nExpectedSize = 32;

    int nImageColWidth = nExpectedSize + nMargin;

    aWidths.push_back(nImageColWidth);
    aWidths.push_back(rSize.Width() - (nImageColWidth + nStandardImageColWidth));
    rTreeView.set_column_fixed_widths(aWidths);
}

SvxMenuConfigPage::~SvxMenuConfigPage()
{
    for (int i = 0, nCount = m_xSaveInListBox->get_count(); i < nCount; ++i)
        delete reinterpret_cast<SaveInData*>(m_xSaveInListBox->get_id(i).toInt64());
    m_xSaveInListBox->clear();
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
        m_xFrame, m_aModuleId);
    m_xCommandCategoryListBox->categorySelected(m_xFunctions.get(), OUString(), GetSaveInData());
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
        (m_xContentsListBox->n_children() != 0 && selection != -1);

    m_xMoveUpButton->set_sensitive(
        bIsValidSelection &&  selection != 0 );
    m_xMoveDownButton->set_sensitive(
        bIsValidSelection && selection != m_xContentsListBox->n_children() - 1);

    m_xRemoveCommandButton->set_sensitive( bIsValidSelection );

    m_xModifyBtn->set_sensitive( bIsValidSelection && !bIsSeparator);

    // If there is no top level selection (menu), then everything working on the right box
    // which contains the functions of the selected menu/toolbar needs to be disabled
    SvxConfigEntry* pMenuData = GetTopLevelSelection();

    m_xInsertBtn->set_sensitive(pMenuData != nullptr);

    m_xAddCommandButton->set_sensitive(pMenuData != nullptr);
    m_xRemoveCommandButton->set_sensitive(pMenuData != nullptr);

    //Handle the gear button
    if (pMenuData && m_bIsMenuBar)
    {
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

    if (nActEntry == -1)
        return;

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

short SvxMenuConfigPage::QueryReset()
{
    OUString msg = CuiResId( RID_SVXSTR_CONFIRM_MENU_RESET );

    OUString saveInName = m_xSaveInListBox->get_active_text();

    OUString label = SvxConfigPageHelper::replaceSaveInName( msg, saveInName );

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   label));
    return xQueryBox->run();
}

void SvxMenuConfigPage::SelectElement()
{
    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();

    SvxConfigEntry* pMenuData = GetTopLevelSelection();
    if (!pMenuData)
        rTreeView.clear();
    else
    {
        SvxEntries* pEntries = pMenuData->GetEntries();

        rTreeView.bulk_insert_for_each(pEntries->size(), [this, &rTreeView, pEntries](weld::TreeIter& rIter, int nIdx) {
            auto const& entry = (*pEntries)[nIdx];
            OUString sId(OUString::number(reinterpret_cast<sal_Int64>(entry)));
            rTreeView.set_id(rIter, sId);
            InsertEntryIntoUI(entry, rTreeView, rIter, 0);
        });
    }

    UpdateButtonStates();
}

IMPL_LINK(SvxMenuConfigPage, GearHdl, const OString&, rIdent, void)
{
    if (rIdent == "menu_gear_add")
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
    else if (rIdent == "menu_gear_delete")
    {
        DeleteSelectedTopLevel();
    }
    else if (rIdent == "menu_gear_rename")
    {
        SvxConfigEntry* pMenuData = GetTopLevelSelection();

        OUString sCurrentName( SvxConfigPageHelper::stripHotKey( pMenuData->GetName() ) );
        OUString sDesc = CuiResId( RID_SVXSTR_LABEL_NEW_NAME );

        SvxNameDialog aNameDialog(GetFrameWeld(), sCurrentName, sDesc);
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
    int nPos = AddFunction(-1, /*bAllowDuplicates*/false);
    if (nPos == -1)
        return;
    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();
    SvxConfigEntry* pEntry =
        reinterpret_cast<SvxConfigEntry*>(rTreeView.get_id(nPos).toInt64());
    InsertEntryIntoUI(pEntry, rTreeView, nPos, 0);
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
    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();
    if (rIdent == "insertseparator")
    {
        SvxConfigEntry* pNewEntryData = new SvxConfigEntry;
        pNewEntryData->SetUserDefined();
        int nPos = AppendEntry(pNewEntryData, -1);
        InsertEntryIntoUI(pNewEntryData, rTreeView, nPos, 0);
    }
    else if (rIdent == "insertsubmenu")
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

            int nPos = AppendEntry(pNewEntryData, -1);
            InsertEntryIntoUI(pNewEntryData, rTreeView, nPos, 0);

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

        SvxNameDialog aNameDialog(GetFrameWeld(), aNewName, aDesc);
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

    std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                   VclMessageType::Question, VclButtonsType::YesNo,
                                                   CuiResId(RID_SVXSTR_CONFIRM_RESTORE_DEFAULT_MENU)));

    // Resetting individual top-level menus is not possible at the moment.
    // So we are resetting only if it is a context menu
    if (m_bIsMenuBar || xQueryBox->run() != RET_YES)
        return;

    sal_Int32 nPos = m_xTopLevelListBox->get_active();
    ContextMenuSaveInData* pSaveInData = static_cast< ContextMenuSaveInData* >(GetSaveInData());

    pSaveInData->ResetContextMenu(pMenuData);

    // ensure that the UI is cleared before populating it
    m_xTopLevelListBox->clear();
    m_xContentsListBox->clear();

    ReloadTopLevelListBox();

    // Reselect the reset menu
    m_xTopLevelListBox->set_active(nPos);
    SelectElement();
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

IMPL_LINK( SvxMenuConfigPage, ContentContextMenuHdl, const CommandEvent&, rCEvt, bool )
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    weld::TreeView& rTreeView = m_xContentsListBox->get_widget();

    // Select clicked entry
    std::unique_ptr<weld::TreeIter> rIter(rTreeView.make_iterator());
    rTreeView.get_dest_row_at_pos( rCEvt.GetMousePosPixel(), &*rIter );
    rTreeView.select(*rIter);
    SelectMenuEntry( rTreeView );

    int nSelectIndex = m_xContentsListBox->get_selected_index();

    bool  bIsSeparator =
        nSelectIndex != -1 && reinterpret_cast<SvxConfigEntry*>(m_xContentsListBox->get_id(nSelectIndex).toInt64())->IsSeparator();
    bool bIsValidSelection =
        ( m_xContentsListBox->n_children() != 0 && nSelectIndex != -1 );

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder( &rTreeView, "cui/ui/entrycontextmenu.ui" ) );
    auto xContextMenu = xBuilder->weld_menu("menu");
    xContextMenu->set_visible("add", false);
    xContextMenu->set_visible("remove", bIsValidSelection);
    xContextMenu->set_visible("rename", bIsValidSelection && !bIsSeparator);
    xContextMenu->set_visible("changeIcon", false);
    xContextMenu->set_visible("resetIcon", false);
    xContextMenu->set_visible("restoreDefault", false);
    OString sCommand(xContextMenu->popup_at_rect( &rTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1) ) ) );

    if ( sCommand == "remove")
    {
        RemoveCommandHdl( *m_xRemoveCommandButton );
    }
    else if ( sCommand == "rename" )
    {
        ModifyItemHdl( "renameItem" );
    }
    else if ( !sCommand.isEmpty() )
        SAL_WARN("cui.customize", "Unknown context menu action: " << sCommand );
    return true;
}

IMPL_LINK( SvxMenuConfigPage, FunctionContextMenuHdl, const CommandEvent&, rCEvt, bool )
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return false;

    weld::TreeView& rTreeView = m_xFunctions->get_widget();

    // Select clicked entry
    std::unique_ptr<weld::TreeIter> rIter(rTreeView.make_iterator());
    rTreeView.get_dest_row_at_pos( rCEvt.GetMousePosPixel(), &*rIter );
    rTreeView.select(*rIter);
    SelectFunctionHdl( rTreeView );

    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder( &rTreeView, "cui/ui/entrycontextmenu.ui" ) );
    auto xContextMenu = xBuilder->weld_menu("menu");
    xContextMenu->set_visible("add", true);
    xContextMenu->set_visible("remove", false);
    xContextMenu->set_visible("rename", false);
    xContextMenu->set_visible("changeIcon", false);
    xContextMenu->set_visible("resetIcon", false);
    xContextMenu->set_visible("restoreDefault", false);
    OString sCommand(xContextMenu->popup_at_rect( &rTreeView, tools::Rectangle(rCEvt.GetMousePosPixel(), Size(1,1) ) ) );

    if ( sCommand == "add")
    {
        AddCommandHdl( *m_xAddCommandButton );
    }
    else if ( !sCommand.isEmpty() )
        SAL_WARN("cui.customize", "Unknown context menu action: " << sCommand );
    return true;
}

 /* vim:set shiftwidth=4 softtabstop=4 expandtab: */
