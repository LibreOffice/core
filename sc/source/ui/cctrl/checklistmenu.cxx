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

#include <checklistmenu.hxx>
#include <o3tl/safeint.hxx>
#include <globstr.hrc>
#include <scresid.hxx>

#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <rtl/math.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <tools/json_writer.hxx>
#include <sfx2/viewsh.hxx>
#include <svl/numformat.hxx>
#include <vcl/jsdialog/executor.hxx>

#include <document.hxx>
#include <docsh.hxx>
#include <viewdata.hxx>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;

ScCheckListMenuControl::MenuItemData::MenuItemData()
    : mbEnabled(true)
{
}

ScCheckListMenuControl::SubMenuItemData::SubMenuItemData(ScCheckListMenuControl* pParent)
    : maTimer("sc SubMenuItemData maTimer")
    , mpSubMenu(nullptr)
    , mnMenuPos(MENU_NOT_SELECTED)
    , mpParent(pParent)
{
    maTimer.SetInvokeHandler(LINK(this, ScCheckListMenuControl::SubMenuItemData, TimeoutHdl));
    maTimer.SetTimeout(Application::GetSettings().GetMouseSettings().GetMenuDelay());
}

void ScCheckListMenuControl::SubMenuItemData::reset()
{
    mpSubMenu = nullptr;
    mnMenuPos = MENU_NOT_SELECTED;
    maTimer.Stop();
}

IMPL_LINK_NOARG(ScCheckListMenuControl::SubMenuItemData, TimeoutHdl, Timer *, void)
{
    mpParent->handleMenuTimeout(this);
}

IMPL_LINK_NOARG(ScCheckListMenuControl, RowActivatedHdl, weld::TreeView&, bool)
{
    executeMenuItem(mxMenu->get_selected_index());
    return true;
}

IMPL_LINK(ScCheckListMenuControl, MenuKeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();

    switch (rKeyCode.GetCode())
    {
        case KEY_RIGHT:
        {
            if (mnSelectedMenu >= maMenuItems.size() || mnSelectedMenu == MENU_NOT_SELECTED)
                break;

            const MenuItemData& rMenu = maMenuItems[mnSelectedMenu];
            if (!rMenu.mxSubMenuWin)
                break;

            executeMenuItem(mnSelectedMenu);
        }
    }

    return false;
}

IMPL_LINK_NOARG(ScCheckListMenuControl, SelectHdl, weld::TreeView&, void)
{
    sal_uInt32 nSelectedMenu = MENU_NOT_SELECTED;
    if (!mxMenu->get_selected(mxScratchIter.get()))
    {
        // reselect current item if its submenu is up and the launching item
        // became unselected by mouse moving out of the top level menu
        if (mnSelectedMenu < maMenuItems.size() &&
            maMenuItems[mnSelectedMenu].mxSubMenuWin &&
            maMenuItems[mnSelectedMenu].mxSubMenuWin->IsVisible())
        {
            mxMenu->select(mnSelectedMenu);
            return;
        }
    }
    else
        nSelectedMenu = mxMenu->get_iter_index_in_parent(*mxScratchIter);

    setSelectedMenuItem(nSelectedMenu, true);
}

void ScCheckListMenuControl::addMenuItem(const OUString& rText, Action* pAction)
{
    MenuItemData aItem;
    aItem.mbEnabled = true;
    aItem.mxAction.reset(pAction);
    maMenuItems.emplace_back(std::move(aItem));

    mxMenu->show();
    mxMenu->append_text(rText);
    mxMenu->set_image(mxMenu->n_children() - 1, css::uno::Reference<css::graphic::XGraphic>(), 1);
}

void ScCheckListMenuControl::addSeparator()
{
    MenuItemData aItem;
    maMenuItems.emplace_back(std::move(aItem));

    mxMenu->append_separator("separator" + OUString::number(maMenuItems.size()));
}

IMPL_LINK(ScCheckListMenuControl, TreeSizeAllocHdl, const Size&, rSize, void)
{
    if (maAllocatedSize == rSize)
        return;
    maAllocatedSize = rSize;
    SetDropdownPos();
    if (!mnAsyncSetDropdownPosId && Application::GetToolkitName().startsWith("gtk"))
    {
        // for gtk retry again later in case it didn't work (wayland)
        mnAsyncSetDropdownPosId  = Application::PostUserEvent(LINK(this, ScCheckListMenuControl, SetDropdownPosHdl));
    }
}

void ScCheckListMenuControl::SetDropdownPos()
{
    std::vector<int> aWidths
    {
        o3tl::narrowing<int>(maAllocatedSize.Width() - (mxMenu->get_text_height() * 3) / 4 - 6)
    };
    mxMenu->set_column_fixed_widths(aWidths);
}

IMPL_LINK_NOARG(ScCheckListMenuControl, SetDropdownPosHdl, void*, void)
{
    mnAsyncSetDropdownPosId = nullptr;
    SetDropdownPos();
    mxMenu->queue_resize();
}

void ScCheckListMenuControl::CreateDropDown()
{
    int nWidth = (mxMenu->get_text_height() * 3) / 4;
    mxDropDown->SetOutputSizePixel(Size(nWidth, nWidth));
    DecorationView aDecoView(mxDropDown.get());
    aDecoView.DrawSymbol(tools::Rectangle(Point(0, 0), Size(nWidth, nWidth)),
                         SymbolType::SPIN_RIGHT, mxDropDown->GetTextColor(),
                         DrawSymbolFlags::NONE);
}

ScListSubMenuControl* ScCheckListMenuControl::addSubMenuItem(const OUString& rText, bool bEnabled, bool bColorMenu)
{
    MenuItemData aItem;
    aItem.mbEnabled = bEnabled;

    aItem.mxSubMenuWin.reset(new ScListSubMenuControl(mxMenu.get(), *this, bColorMenu, mpNotifier));
    maMenuItems.emplace_back(std::move(aItem));

    mxMenu->show();
    mxMenu->append_text(rText);
    mxMenu->set_image(mxMenu->n_children() - 1, *mxDropDown, 1);
    return maMenuItems.back().mxSubMenuWin.get();
}

void ScCheckListMenuControl::executeMenuItem(size_t nPos)
{
    if (nPos >= maMenuItems.size())
        return;

    const MenuItemData& rMenu = maMenuItems[nPos];
    if (rMenu.mxSubMenuWin)
    {
        if (rMenu.mbEnabled)
        {
            maOpenTimer.mnMenuPos = nPos;
            maOpenTimer.mpSubMenu = rMenu.mxSubMenuWin.get();
            launchSubMenu();
        }
        return;
    }

    if (!maMenuItems[nPos].mxAction)
        // no action is defined.
        return;

    const bool bClosePopup = maMenuItems[nPos].mxAction->execute();
    if (bClosePopup)
        terminateAllPopupMenus();
}

void ScCheckListMenuControl::setSelectedMenuItem(size_t nPos, bool bSubMenuTimer)
{
    if (mnSelectedMenu == nPos)
        // nothing to do.
        return;

    selectMenuItem(nPos, bSubMenuTimer);
}

void ScCheckListMenuControl::handleMenuTimeout(const SubMenuItemData* pTimer)
{
    if (pTimer == &maOpenTimer)
    {
        // Close any open submenu immediately.
        if (maCloseTimer.mpSubMenu)
        {
            maCloseTimer.mpSubMenu->EndPopupMode();
            maCloseTimer.mpSubMenu = nullptr;
            maCloseTimer.maTimer.Stop();
        }

        launchSubMenu();
    }
    else if (pTimer == &maCloseTimer)
    {
        // end submenu.
        if (maCloseTimer.mpSubMenu)
        {
            maCloseTimer.mpSubMenu->EndPopupMode();
            maCloseTimer.mpSubMenu = nullptr;

            // EndPopup sends a user event, and we want this focus to be set after that has done its conflicting focus-setting work
            if (!mnAsyncPostPopdownId)
                mnAsyncPostPopdownId = Application::PostUserEvent(LINK(this, ScCheckListMenuControl, PostPopdownHdl));
        }
    }
}

void ScCheckListMenuControl::queueLaunchSubMenu(size_t nPos, ScListSubMenuControl* pMenu)
{
    if (!pMenu)
        return;

    // Set the submenu on launch queue.
    if (maOpenTimer.mpSubMenu)
    {
        if (maOpenTimer.mpSubMenu != pMenu)
        {
            // new submenu is being requested.
            queueCloseSubMenu();
        }
        else
        {
            if (pMenu == maCloseTimer.mpSubMenu)
                maCloseTimer.reset();
        }
    }

    maOpenTimer.mpSubMenu = pMenu;
    maOpenTimer.mnMenuPos = nPos;
    maOpenTimer.maTimer.Start();
}

void ScCheckListMenuControl::queueCloseSubMenu()
{
    if (!maOpenTimer.mpSubMenu)
        // There is no submenu to close.
        return;

    // Stop any submenu on queue for opening.
    maOpenTimer.maTimer.Stop();

    // Flush any pending close so it doesn't get skipped
    if (maCloseTimer.mpSubMenu)
    {
        maCloseTimer.mpSubMenu->EndPopupMode();
    }

    maCloseTimer.mpSubMenu = maOpenTimer.mpSubMenu;
    maCloseTimer.mnMenuPos = maOpenTimer.mnMenuPos;
    maCloseTimer.maTimer.Start();
}

tools::Rectangle ScCheckListMenuControl::GetSubMenuParentRect()
{
    if (!mxMenu->get_selected(mxScratchIter.get()))
        return tools::Rectangle();
    return mxMenu->get_row_area(*mxScratchIter);
}

void ScCheckListMenuControl::launchSubMenu()
{
    ScListSubMenuControl* pSubMenu = maOpenTimer.mpSubMenu;
    if (!pSubMenu)
        return;

    if (!mxMenu->get_selected(mxScratchIter.get()))
        return;

    tools::Rectangle aRect = GetSubMenuParentRect();
    pSubMenu->StartPopupMode(mxMenu.get(), aRect);

    mxMenu->select(*mxScratchIter);
    pSubMenu->GrabFocus();
}

IMPL_LINK_NOARG(ScCheckListMenuControl, PostPopdownHdl, void*, void)
{
    mnAsyncPostPopdownId = nullptr;
    mxMenu->grab_focus();
}

IMPL_LINK(ScCheckListMenuControl, MouseEnterHdl, const MouseEvent&, rMEvt, bool)
{
    if (!rMEvt.IsEnterWindow())
        return false;
    selectMenuItem(MENU_NOT_SELECTED, true);
    return false;
}

void ScCheckListMenuControl::endSubMenu(ScListSubMenuControl& rSubMenu)
{
    rSubMenu.EndPopupMode();
    maOpenTimer.reset();

    // EndPopup sends a user event, and we want this focus to be set after that has done its conflicting focus-setting work
    if (!mnAsyncPostPopdownId)
        mnAsyncPostPopdownId = Application::PostUserEvent(LINK(this, ScCheckListMenuControl, PostPopdownHdl));

    size_t nMenuPos = getSubMenuPos(&rSubMenu);
    if (nMenuPos != MENU_NOT_SELECTED)
    {
        mnSelectedMenu = nMenuPos;
        mxMenu->select(mnSelectedMenu);
    }
}

void ScCheckListMenuControl::selectMenuItem(size_t nPos, bool bSubMenuTimer)
{
    mxMenu->select(nPos == MENU_NOT_SELECTED ? -1 : nPos);
    mnSelectedMenu = nPos;

    if (nPos >= maMenuItems.size() || nPos == MENU_NOT_SELECTED)
    {
        queueCloseSubMenu();
        return;
    }

    if (!maMenuItems[nPos].mbEnabled)
    {
        queueCloseSubMenu();
        return;
    }

    if (bSubMenuTimer)
    {
        if (maMenuItems[nPos].mxSubMenuWin && mxMenu->changed_by_hover())
        {
            ScListSubMenuControl* pSubMenu = maMenuItems[nPos].mxSubMenuWin.get();
            queueLaunchSubMenu(nPos, pSubMenu);
        }
        else
            queueCloseSubMenu();
    }
}

void ScCheckListMenuControl::clearSelectedMenuItem()
{
    selectMenuItem(MENU_NOT_SELECTED, false);
}

size_t ScCheckListMenuControl::getSubMenuPos(const ScListSubMenuControl* pSubMenu)
{
    size_t n = maMenuItems.size();
    for (size_t i = 0; i < n; ++i)
    {
        if (maMenuItems[i].mxSubMenuWin.get() == pSubMenu)
            return i;
    }
    return MENU_NOT_SELECTED;
}

void ScCheckListMenuControl::setSubMenuFocused(const ScListSubMenuControl* pSubMenu)
{
    maCloseTimer.reset();
    size_t nMenuPos = getSubMenuPos(pSubMenu);
    if (mnSelectedMenu != nMenuPos)
    {
        mnSelectedMenu = nMenuPos;
        mxMenu->select(mnSelectedMenu);
    }
}

void ScCheckListMenuControl::EndPopupMode()
{
    if (!mbIsPoppedUp)
        return;
    mxPopover->connect_closed(Link<weld::Popover&, void>());
    mxPopover->popdown();
    PopupModeEndHdl(*mxPopover);
    assert(mbIsPoppedUp == false);
}

void ScCheckListMenuControl::StartPopupMode(weld::Widget* pParent, const tools::Rectangle& rRect)
{
    mxPopover->connect_closed(LINK(this, ScCheckListMenuControl, PopupModeEndHdl));
    mbIsPoppedUp = true;
    mxPopover->popup_at_rect(pParent, rRect);
    GrabFocus();
}

void ScCheckListMenuControl::terminateAllPopupMenus()
{
    if (comphelper::LibreOfficeKit::isActive())
        NotifyCloseLOK();

    EndPopupMode();
}

ScCheckListMenuControl::Config::Config() :
    mbAllowEmptySet(true), mbRTL(false)
{
}

ScCheckListMember::ScCheckListMember()
    : mnValue(0.0)
    , mbVisible(true)
    , mbDate(false)
    , mbLeaf(false)
    , mbValue(false)
    , meDatePartType(YEAR)
{
}

// the value of border-width of FilterDropDown
constexpr int nBorderWidth = 4;
// number of rows visible in checklist
constexpr int nCheckListVisibleRows = 8;

ScCheckListMenuControl::ScCheckListMenuControl(weld::Widget* pParent, ScViewData& rViewData,
                                               bool bHasDates, int nWidth, vcl::ILibreOfficeKitNotifier* pNotifier)
    : mxBuilder(Application::CreateBuilder(pParent, "modules/scalc/ui/filterdropdown.ui"))
    , mxPopover(mxBuilder->weld_popover("FilterDropDown"))
    , mxContainer(mxBuilder->weld_container("container"))
    , mxMenu(mxBuilder->weld_tree_view("menu"))
    , mxScratchIter(mxMenu->make_iterator())
    , mxNonMenu(mxBuilder->weld_widget("nonmenu"))
    , mxEdSearch(mxBuilder->weld_entry("search_edit"))
    , mxBox(mxBuilder->weld_widget("box"))
    , mxListChecks(mxBuilder->weld_tree_view("check_list_box"))
    , mxTreeChecks(mxBuilder->weld_tree_view("check_tree_box"))
    , mxChkToggleAll(mxBuilder->weld_check_button("toggle_all"))
    , mxBtnSelectSingle(mxBuilder->weld_button("select_current"))
    , mxBtnUnselectSingle(mxBuilder->weld_button("unselect_current"))
    , mxButtonBox(mxBuilder->weld_box("buttonbox"))
    , mxBtnOk(mxBuilder->weld_button("ok"))
    , mxBtnCancel(mxBuilder->weld_button("cancel"))
    , mxDropDown(mxMenu->create_virtual_device())
    , mnCheckWidthReq(-1)
    , mnWndWidth(0)
    , mePrevToggleAllState(TRISTATE_INDET)
    , mnSelectedMenu(MENU_NOT_SELECTED)
    , mrViewData(rViewData)
    , mnAsyncPostPopdownId(nullptr)
    , mnAsyncSetDropdownPosId(nullptr)
    , mpNotifier(pNotifier)
    , mbHasDates(bHasDates)
    , mbIsPoppedUp(false)
    , maOpenTimer(this)
    , maCloseTimer(this)
{
    mxTreeChecks->set_clicks_to_toggle(1);
    mxListChecks->set_clicks_to_toggle(1);

    mxNonMenu->connect_mouse_move(LINK(this, ScCheckListMenuControl, MouseEnterHdl));
    mxEdSearch->connect_mouse_move(LINK(this, ScCheckListMenuControl, MouseEnterHdl));
    mxListChecks->connect_mouse_move(LINK(this, ScCheckListMenuControl, MouseEnterHdl));
    mxTreeChecks->connect_mouse_move(LINK(this, ScCheckListMenuControl, MouseEnterHdl));
    mxChkToggleAll->connect_mouse_move(LINK(this, ScCheckListMenuControl, MouseEnterHdl));
    mxBtnSelectSingle->connect_mouse_move(LINK(this, ScCheckListMenuControl, MouseEnterHdl));
    mxBtnUnselectSingle->connect_mouse_move(LINK(this, ScCheckListMenuControl, MouseEnterHdl));
    mxBtnOk->connect_mouse_move(LINK(this, ScCheckListMenuControl, MouseEnterHdl));
    mxBtnCancel->connect_mouse_move(LINK(this, ScCheckListMenuControl, MouseEnterHdl));

    /*
       tdf#136559 If we have no dates we don't need a tree
       structure, just a list. GtkListStore can be then
       used which is much faster than a GtkTreeStore, so
       with no dates switch to the treeview which uses the
       faster GtkListStore
    */
    if (mbHasDates)
        mpChecks = mxTreeChecks.get();
    else
    {
        mxTreeChecks->hide();
        mxListChecks->show();
        mpChecks = mxListChecks.get();
    }

    int nChecksHeight = mxTreeChecks->get_height_rows(nCheckListVisibleRows);
    if (nWidth != -1)
    {
        mnCheckWidthReq = nWidth - nBorderWidth * 2 - 4;
        mxTreeChecks->set_size_request(mnCheckWidthReq, nChecksHeight);
        mxListChecks->set_size_request(mnCheckWidthReq, nChecksHeight);
    }

    // sort ok/cancel into native order, if this was a dialog they would be auto-sorted, but this
    // popup isn't a true dialog
    mxButtonBox->sort_native_button_order();

    mxTreeChecks->enable_toggle_buttons(weld::ColumnToggleType::Check);
    mxListChecks->enable_toggle_buttons(weld::ColumnToggleType::Check);

    mxBox->show();
    mxEdSearch->show();
    mxButtonBox->show();

    mxMenu->connect_row_activated(LINK(this, ScCheckListMenuControl, RowActivatedHdl));
    mxMenu->connect_changed(LINK(this, ScCheckListMenuControl, SelectHdl));
    mxMenu->connect_key_press(LINK(this, ScCheckListMenuControl, MenuKeyInputHdl));

    mxBtnOk->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));
    mxBtnCancel->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));
    mxEdSearch->connect_changed(LINK(this, ScCheckListMenuControl, EdModifyHdl));
    mxEdSearch->connect_activate(LINK(this, ScCheckListMenuControl, EdActivateHdl));
    mxTreeChecks->connect_toggled(LINK(this, ScCheckListMenuControl, CheckHdl));
    mxTreeChecks->connect_key_press(LINK(this, ScCheckListMenuControl, KeyInputHdl));
    mxListChecks->connect_toggled(LINK(this, ScCheckListMenuControl, CheckHdl));
    mxListChecks->connect_key_press(LINK(this, ScCheckListMenuControl, KeyInputHdl));
    mxChkToggleAll->connect_toggled(LINK(this, ScCheckListMenuControl, TriStateHdl));
    mxBtnSelectSingle->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));
    mxBtnUnselectSingle->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));

    CreateDropDown();
    mxMenu->connect_size_allocate(LINK(this, ScCheckListMenuControl, TreeSizeAllocHdl));

    // determine what width the checklist will end up with
    mnCheckWidthReq = mxContainer->get_preferred_size().Width();
    // make that size fixed now, we can now use mnCheckWidthReq to speed up
    // bulk_insert_for_each
    mxTreeChecks->set_size_request(mnCheckWidthReq, nChecksHeight);
    mxListChecks->set_size_request(mnCheckWidthReq, nChecksHeight);
}

void ScCheckListMenuControl::GrabFocus()
{
    if (mxEdSearch->get_visible())
        mxEdSearch->grab_focus();
    else
    {
        mxMenu->set_cursor(0);
        mxMenu->grab_focus();
    }
}

ScCheckListMenuControl::~ScCheckListMenuControl()
{
    EndPopupMode();
    for (auto& rMenuItem : maMenuItems)
        rMenuItem.mxSubMenuWin.reset();
    if (mnAsyncPostPopdownId)
    {
        Application::RemoveUserEvent(mnAsyncPostPopdownId);
        mnAsyncPostPopdownId = nullptr;
    }
    if (mnAsyncSetDropdownPosId)
    {
        Application::RemoveUserEvent(mnAsyncSetDropdownPosId);
        mnAsyncSetDropdownPosId = nullptr;
    }
}

void ScCheckListMenuControl::prepWindow()
{
    mxMenu->set_size_request(-1, mxMenu->get_preferred_size().Height() + 2);
    mnSelectedMenu = 0;
    if (mxMenu->n_children())
    {
        mxMenu->set_cursor(mnSelectedMenu);
        mxMenu->unselect_all();
    }

    mnWndWidth = mxContainer->get_preferred_size().Width() + nBorderWidth * 2 + 4;
}

void ScCheckListMenuControl::setAllMemberState(bool bSet)
{
    mpChecks->all_foreach([this, bSet](weld::TreeIter& rEntry){
        mpChecks->set_toggle(rEntry, bSet ? TRISTATE_TRUE : TRISTATE_FALSE);
        return false;
    });

    if (!maConfig.mbAllowEmptySet)
    {
        // We need to have at least one member selected.
        mxBtnOk->set_sensitive(GetCheckedEntryCount() != 0);
    }
}

void ScCheckListMenuControl::selectCurrentMemberOnly(bool bSet)
{
    setAllMemberState(!bSet);
    std::unique_ptr<weld::TreeIter> xEntry = mpChecks->make_iterator();
    if (!mpChecks->get_cursor(xEntry.get()))
        return;
    mpChecks->set_toggle(*xEntry, bSet ? TRISTATE_TRUE : TRISTATE_FALSE);
}

IMPL_LINK(ScCheckListMenuControl, ButtonHdl, weld::Button&, rBtn, void)
{
    if (&rBtn == mxBtnOk.get())
        close(true);
    else if (&rBtn == mxBtnCancel.get())
        close(false);
    else if (&rBtn == mxBtnSelectSingle.get() || &rBtn == mxBtnUnselectSingle.get())
    {
        selectCurrentMemberOnly(&rBtn == mxBtnSelectSingle.get());
        std::unique_ptr<weld::TreeIter> xEntry = mpChecks->make_iterator();
        if (!mpChecks->get_cursor(xEntry.get()))
            xEntry.reset();
        Check(xEntry.get());
    }
}

IMPL_LINK_NOARG(ScCheckListMenuControl, TriStateHdl, weld::Toggleable&, void)
{
    switch (mePrevToggleAllState)
    {
        case TRISTATE_FALSE:
            mxChkToggleAll->set_state(TRISTATE_TRUE);
            setAllMemberState(true);
        break;
        case TRISTATE_TRUE:
            mxChkToggleAll->set_state(TRISTATE_FALSE);
            setAllMemberState(false);
        break;
        case TRISTATE_INDET:
        default:
            mxChkToggleAll->set_state(TRISTATE_TRUE);
            setAllMemberState(true);
        break;
    }

    mePrevToggleAllState = mxChkToggleAll->get_state();
}

namespace
{
    void insertMember(weld::TreeView& rView, const weld::TreeIter& rIter, const ScCheckListMember& rMember, bool bChecked)
    {
        OUString aLabel = rMember.maName;
        if (aLabel.isEmpty())
            aLabel = ScResId(STR_EMPTYDATA);
        rView.set_toggle(rIter, bChecked ? TRISTATE_TRUE : TRISTATE_FALSE);
        rView.set_text(rIter, aLabel, 0);
    }
}

IMPL_LINK_NOARG(ScCheckListMenuControl, EdModifyHdl, weld::Entry&, void)
{
    OUString aSearchText = mxEdSearch->get_text();
    aSearchText = ScGlobal::getCharClass().lowercase( aSearchText );
    bool bSearchTextEmpty = aSearchText.isEmpty();
    size_t n = maMembers.size();
    size_t nSelCount = 0;

    // This branch is the general case, the other is an optimized variant of
    // this one where we can take advantage of knowing we have no hierarchy
    if (mbHasDates)
    {
        mpChecks->freeze();

        bool bSomeDateDeletes = false;

        for (size_t i = 0; i < n; ++i)
        {
            bool bIsDate = maMembers[i].mbDate;
            bool bPartialMatch = false;

            OUString aLabelDisp = maMembers[i].maName;
            if ( aLabelDisp.isEmpty() )
                aLabelDisp = ScResId( STR_EMPTYDATA );

            if ( !bSearchTextEmpty )
            {
                if ( !bIsDate )
                    bPartialMatch = ( ScGlobal::getCharClass().lowercase( aLabelDisp ).indexOf( aSearchText ) != -1 );
                else if ( maMembers[i].meDatePartType == ScCheckListMember::DAY ) // Match with both numerical and text version of month
                    bPartialMatch = (ScGlobal::getCharClass().lowercase( OUString(
                                    maMembers[i].maRealName + maMembers[i].maDateParts[1] )).indexOf( aSearchText ) != -1);
                else
                    continue;
            }
            else if ( bIsDate && maMembers[i].meDatePartType != ScCheckListMember::DAY )
                continue;

            if ( bSearchTextEmpty )
            {
                auto xLeaf = ShowCheckEntry(aLabelDisp, maMembers[i], true, maMembers[i].mbVisible);
                updateMemberParents(xLeaf.get(), i);
                if ( maMembers[i].mbVisible )
                    ++nSelCount;
                continue;
            }

            if ( bPartialMatch )
            {
                auto xLeaf = ShowCheckEntry(aLabelDisp, maMembers[i]);
                updateMemberParents(xLeaf.get(), i);
                ++nSelCount;
            }
            else
            {
                ShowCheckEntry(aLabelDisp, maMembers[i], false, false);
                if( bIsDate )
                    bSomeDateDeletes = true;
            }
        }

        if ( bSomeDateDeletes )
        {
            for (size_t i = 0; i < n; ++i)
            {
                if (!maMembers[i].mbDate)
                    continue;
                if (maMembers[i].meDatePartType != ScCheckListMember::DAY)
                    continue;
                updateMemberParents(nullptr, i);
            }
        }

        mpChecks->thaw();
    }
    else
    {
        mpChecks->freeze();

        // when there are a lot of rows, it is cheaper to simply clear the tree and either
        // re-initialise or just insert the filtered lines
        mpChecks->clear();

        mpChecks->thaw();

        if (bSearchTextEmpty)
            nSelCount = initMembers();
        else
        {
            std::vector<size_t> aShownIndexes;

            for (size_t i = 0; i < n; ++i)
            {
                assert(!maMembers[i].mbDate);

                OUString aLabelDisp = maMembers[i].maName;
                if ( aLabelDisp.isEmpty() )
                    aLabelDisp = ScResId( STR_EMPTYDATA );

                bool bPartialMatch = ScGlobal::getCharClass().lowercase( aLabelDisp ).indexOf( aSearchText ) != -1;

                if (!bPartialMatch)
                    continue;

                aShownIndexes.push_back(i);
            }

            std::vector<int> aFixedWidths { mnCheckWidthReq };
            // tdf#122419 insert in the fastest order, this might be backwards.
            mpChecks->bulk_insert_for_each(aShownIndexes.size(), [this, &aShownIndexes, &nSelCount](weld::TreeIter& rIter, int i) {
                size_t nIndex = aShownIndexes[i];
                insertMember(*mpChecks, rIter, maMembers[nIndex], true);
                ++nSelCount;
            }, nullptr, &aFixedWidths);
        }
    }

    if ( nSelCount == n )
        mxChkToggleAll->set_state( TRISTATE_TRUE );
    else if ( nSelCount == 0 )
        mxChkToggleAll->set_state( TRISTATE_FALSE );
    else
        mxChkToggleAll->set_state( TRISTATE_INDET );

    if ( !maConfig.mbAllowEmptySet )
    {
        const bool bEmptySet( nSelCount == 0 );
        mpChecks->set_sensitive(!bEmptySet);
        mxChkToggleAll->set_sensitive(!bEmptySet);
        mxBtnSelectSingle->set_sensitive(!bEmptySet);
        mxBtnUnselectSingle->set_sensitive(!bEmptySet);
        mxBtnOk->set_sensitive(!bEmptySet);
    }
}

IMPL_LINK_NOARG(ScCheckListMenuControl, EdActivateHdl, weld::Entry&, bool)
{
    if (mxBtnOk->get_sensitive())
        close(true);
    return true;
}

IMPL_LINK( ScCheckListMenuControl, CheckHdl, const weld::TreeView::iter_col&, rRowCol, void )
{
    Check(&rRowCol.first);
}

void ScCheckListMenuControl::Check(const weld::TreeIter* pEntry)
{
    if (pEntry)
        CheckEntry(*pEntry, mpChecks->get_toggle(*pEntry) == TRISTATE_TRUE);
    size_t nNumChecked = GetCheckedEntryCount();
    if (nNumChecked == maMembers.size())
        // all members visible
        mxChkToggleAll->set_state(TRISTATE_TRUE);
    else if (nNumChecked == 0)
        // no members visible
        mxChkToggleAll->set_state(TRISTATE_FALSE);
    else
        mxChkToggleAll->set_state(TRISTATE_INDET);

    if (!maConfig.mbAllowEmptySet)
        // We need to have at least one member selected.
        mxBtnOk->set_sensitive(nNumChecked != 0);

    mePrevToggleAllState = mxChkToggleAll->get_state();
}

void ScCheckListMenuControl::updateMemberParents(const weld::TreeIter* pLeaf, size_t nIdx)
{
    if ( !maMembers[nIdx].mbDate || maMembers[nIdx].meDatePartType != ScCheckListMember::DAY )
        return;

    OUString aYearName  = maMembers[nIdx].maDateParts[0];
    OUString aMonthName = maMembers[nIdx].maDateParts[1];
    auto aItr = maYearMonthMap.find(aYearName + aMonthName);

    if ( pLeaf )
    {
        std::unique_ptr<weld::TreeIter> xYearEntry;
        std::unique_ptr<weld::TreeIter> xMonthEntry = mpChecks->make_iterator(pLeaf);
        if (!mpChecks->iter_parent(*xMonthEntry))
            xMonthEntry.reset();
        else
        {
            xYearEntry = mpChecks->make_iterator(xMonthEntry.get());
            if (!mpChecks->iter_parent(*xYearEntry))
                xYearEntry.reset();
        }

        maMembers[nIdx].mxParent = std::move(xMonthEntry);
        if ( aItr != maYearMonthMap.end() )
        {
            size_t nMonthIdx = aItr->second;
            maMembers[nMonthIdx].mxParent = std::move(xYearEntry);
        }
    }
    else
    {
        std::unique_ptr<weld::TreeIter> xYearEntry = FindEntry(nullptr, aYearName);
        if (aItr != maYearMonthMap.end() && !xYearEntry)
        {
            size_t nMonthIdx = aItr->second;
            maMembers[nMonthIdx].mxParent.reset();
            maMembers[nIdx].mxParent.reset();
        }
        else if (xYearEntry && !FindEntry(xYearEntry.get(), aMonthName))
            maMembers[nIdx].mxParent.reset();
    }
}

void ScCheckListMenuControl::setMemberSize(size_t n)
{
    maMembers.reserve(n);
}

void ScCheckListMenuControl::addDateMember(const OUString& rsName, double nVal, bool bVisible)
{
    SvNumberFormatter* pFormatter = mrViewData.GetDocument().GetFormatTable();

    // Convert the numeric date value to a date object.
    Date aDate = pFormatter->GetNullDate();
    aDate.AddDays(rtl::math::approxFloor(nVal));

    sal_Int16 nYear = aDate.GetYear();
    sal_uInt16 nMonth = aDate.GetMonth();
    sal_uInt16 nDay = aDate.GetDay();

    // Get the localized month name list.
    CalendarWrapper& rCalendar = ScGlobal::GetCalendar();
    uno::Sequence<i18n::CalendarItem2> aMonths = rCalendar.getMonths();
    if (aMonths.getLength() < nMonth)
        return;

    OUString aYearName = OUString::number(nYear);
    OUString aMonthName = aMonths[nMonth-1].FullName;
    OUString aDayName = OUString::number(nDay);

    if ( aDayName.getLength() == 1 )
        aDayName = "0" + aDayName;

    mpChecks->freeze();

    std::unique_ptr<weld::TreeIter> xYearEntry = FindEntry(nullptr, aYearName);
    if (!xYearEntry)
    {
        xYearEntry = mpChecks->make_iterator();
        mpChecks->insert(nullptr, -1, nullptr, nullptr, nullptr, nullptr, false, xYearEntry.get());
        mpChecks->set_toggle(*xYearEntry, TRISTATE_FALSE);
        mpChecks->set_text(*xYearEntry, aYearName, 0);
        ScCheckListMember aMemYear;
        aMemYear.maName = aYearName;
        aMemYear.maRealName = rsName;
        aMemYear.mbDate = true;
        aMemYear.mbLeaf = false;
        aMemYear.mbVisible = bVisible;
        aMemYear.mxParent.reset();
        aMemYear.meDatePartType = ScCheckListMember::YEAR;
        maMembers.emplace_back(std::move(aMemYear));
    }

    std::unique_ptr<weld::TreeIter> xMonthEntry = FindEntry(xYearEntry.get(), aMonthName);
    if (!xMonthEntry)
    {
        xMonthEntry = mpChecks->make_iterator();
        mpChecks->insert(xYearEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xMonthEntry.get());
        mpChecks->set_toggle(*xMonthEntry, TRISTATE_FALSE);
        mpChecks->set_text(*xMonthEntry, aMonthName, 0);
        ScCheckListMember aMemMonth;
        aMemMonth.maName = aMonthName;
        aMemMonth.maRealName = rsName;
        aMemMonth.mbDate = true;
        aMemMonth.mbLeaf = false;
        aMemMonth.mbVisible = bVisible;
        aMemMonth.mxParent = std::move(xYearEntry);
        aMemMonth.meDatePartType = ScCheckListMember::MONTH;
        maMembers.emplace_back(std::move(aMemMonth));
        maYearMonthMap[aYearName + aMonthName] = maMembers.size() - 1;
    }

    std::unique_ptr<weld::TreeIter> xDayEntry = FindEntry(xMonthEntry.get(), aDayName);
    if (!xDayEntry)
    {
        xDayEntry = mpChecks->make_iterator();
        mpChecks->insert(xMonthEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xDayEntry.get());
        mpChecks->set_toggle(*xDayEntry, TRISTATE_FALSE);
        mpChecks->set_text(*xDayEntry, aDayName, 0);
        ScCheckListMember aMemDay;
        aMemDay.maName = aDayName;
        aMemDay.maRealName = rsName;
        aMemDay.maDateParts.resize(2);
        aMemDay.maDateParts[0] = aYearName;
        aMemDay.maDateParts[1] = aMonthName;
        aMemDay.mbDate = true;
        aMemDay.mbLeaf = true;
        aMemDay.mbVisible = bVisible;
        aMemDay.mxParent = std::move(xMonthEntry);
        aMemDay.meDatePartType = ScCheckListMember::DAY;
        maMembers.emplace_back(std::move(aMemDay));
    }

    mpChecks->thaw();
}

void ScCheckListMenuControl::addMember(const OUString& rName, const double nVal, bool bVisible, bool bValue)
{
    ScCheckListMember aMember;
    // tdf#46062 - indicate hidden whitespaces using quotes
    aMember.maName = rName.trim() != rName ? "\"" + rName + "\"" : rName;
    aMember.maRealName = rName;
    aMember.mnValue = nVal;
    aMember.mbDate = false;
    aMember.mbLeaf = true;
    aMember.mbValue = bValue;
    aMember.mbVisible = bVisible;
    aMember.mxParent.reset();
    maMembers.emplace_back(std::move(aMember));
}

std::unique_ptr<weld::TreeIter> ScCheckListMenuControl::FindEntry(const weld::TreeIter* pParent, std::u16string_view sNode)
{
    std::unique_ptr<weld::TreeIter> xEntry = mpChecks->make_iterator(pParent);
    bool bEntry = pParent ? mpChecks->iter_children(*xEntry) : mpChecks->get_iter_first(*xEntry);
    while (bEntry)
    {
        if (sNode == mpChecks->get_text(*xEntry, 0))
            return xEntry;
        bEntry = mpChecks->iter_next_sibling(*xEntry);
    }
    return nullptr;
}

void ScCheckListMenuControl::GetRecursiveChecked(const weld::TreeIter* pEntry, std::unordered_set<OUString>& vOut,
                                                 OUString& rLabel)
{
    if (mpChecks->get_toggle(*pEntry) != TRISTATE_TRUE)
        return;

    // We have to hash parents and children together.
    // Per convention for easy access in getResult()
    // "child;parent;grandparent" while descending.
    if (rLabel.isEmpty())
        rLabel = mpChecks->get_text(*pEntry, 0);
    else
        rLabel = mpChecks->get_text(*pEntry, 0) + ";" + rLabel;

    // Prerequisite: the selection mechanism guarantees that if a child is
    // selected then also the parent is selected, so we only have to
    // inspect the children in case the parent is selected.
    if (!mpChecks->iter_has_child(*pEntry))
        return;

    std::unique_ptr<weld::TreeIter> xChild(mpChecks->make_iterator(pEntry));
    bool bChild = mpChecks->iter_children(*xChild);
    while (bChild)
    {
        OUString aLabel = rLabel;
        GetRecursiveChecked(xChild.get(), vOut, aLabel);
        if (!aLabel.isEmpty() && aLabel != rLabel)
            vOut.insert(aLabel);
        bChild = mpChecks->iter_next_sibling(*xChild);
    }
    // Let the caller not add the parent alone.
    rLabel.clear();
}

std::unordered_set<OUString> ScCheckListMenuControl::GetAllChecked()
{
    std::unordered_set<OUString> vResults(0);

    std::unique_ptr<weld::TreeIter> xEntry = mpChecks->make_iterator();
    bool bEntry = mpChecks->get_iter_first(*xEntry);
    while (bEntry)
    {
        OUString aLabel;
        GetRecursiveChecked(xEntry.get(), vResults, aLabel);
        if (!aLabel.isEmpty())
            vResults.insert(aLabel);
        bEntry = mpChecks->iter_next_sibling(*xEntry);
    }

    return vResults;
}

bool ScCheckListMenuControl::IsChecked(std::u16string_view sName, const weld::TreeIter* pParent)
{
    std::unique_ptr<weld::TreeIter> xEntry = FindEntry(pParent, sName);
    return xEntry && mpChecks->get_toggle(*xEntry) == TRISTATE_TRUE;
}

void ScCheckListMenuControl::CheckEntry(std::u16string_view sName, const weld::TreeIter* pParent, bool bCheck)
{
    std::unique_ptr<weld::TreeIter> xEntry = FindEntry(pParent, sName);
    if (xEntry)
        CheckEntry(*xEntry, bCheck);
}

// Recursively check all children of rParent
void ScCheckListMenuControl::CheckAllChildren(const weld::TreeIter& rParent, bool bCheck)
{
    mpChecks->set_toggle(rParent, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE);
    std::unique_ptr<weld::TreeIter> xEntry = mpChecks->make_iterator(&rParent);
    bool bEntry = mpChecks->iter_children(*xEntry);
    while (bEntry)
    {
        CheckAllChildren(*xEntry, bCheck);
        bEntry = mpChecks->iter_next_sibling(*xEntry);
    }
}

void ScCheckListMenuControl::CheckEntry(const weld::TreeIter& rParent, bool bCheck)
{
    // recursively check all items below rParent
    CheckAllChildren(rParent, bCheck);
    // checking rParent can affect ancestors, e.g. if ancestor is unchecked and rParent is
    // now checked then the ancestor needs to be checked also
    if (!mpChecks->get_iter_depth(rParent))
        return;

    std::unique_ptr<weld::TreeIter> xAncestor(mpChecks->make_iterator(&rParent));
    bool bAncestor = mpChecks->iter_parent(*xAncestor);
    while (bAncestor)
    {
        // if any first level children checked then ancestor
        // needs to be checked, similarly if no first level children
        // checked then ancestor needs to be unchecked
        std::unique_ptr<weld::TreeIter> xChild(mpChecks->make_iterator(xAncestor.get()));
        bool bChild = mpChecks->iter_children(*xChild);
        bool bChildChecked = false;

        while (bChild)
        {
            if (mpChecks->get_toggle(*xChild) == TRISTATE_TRUE)
            {
                bChildChecked = true;
                break;
            }
            bChild = mpChecks->iter_next_sibling(*xChild);
        }
        mpChecks->set_toggle(*xAncestor, bChildChecked ? TRISTATE_TRUE : TRISTATE_FALSE);
        bAncestor = mpChecks->iter_parent(*xAncestor);
    }
}

std::unique_ptr<weld::TreeIter> ScCheckListMenuControl::ShowCheckEntry(const OUString& sName, ScCheckListMember& rMember, bool bShow, bool bCheck)
{
    std::unique_ptr<weld::TreeIter> xEntry;
    if (!rMember.mbDate || rMember.mxParent)
        xEntry = FindEntry(rMember.mxParent.get(), sName);

    if ( bShow )
    {
        if (!xEntry)
        {
            if (rMember.mbDate)
            {
                if (rMember.maDateParts.empty())
                    return nullptr;

                std::unique_ptr<weld::TreeIter> xYearEntry = FindEntry(nullptr, rMember.maDateParts[0]);
                if (!xYearEntry)
                {
                    xYearEntry = mpChecks->make_iterator();
                    mpChecks->insert(nullptr, -1, nullptr, nullptr, nullptr, nullptr, false, xYearEntry.get());
                    mpChecks->set_toggle(*xYearEntry, TRISTATE_FALSE);
                    mpChecks->set_text(*xYearEntry, rMember.maDateParts[0], 0);
                }
                std::unique_ptr<weld::TreeIter> xMonthEntry = FindEntry(xYearEntry.get(), rMember.maDateParts[1]);
                if (!xMonthEntry)
                {
                    xMonthEntry = mpChecks->make_iterator();
                    mpChecks->insert(xYearEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xMonthEntry.get());
                    mpChecks->set_toggle(*xMonthEntry, TRISTATE_FALSE);
                    mpChecks->set_text(*xMonthEntry, rMember.maDateParts[1], 0);
                }
                std::unique_ptr<weld::TreeIter> xDayEntry = FindEntry(xMonthEntry.get(), rMember.maName);
                if (!xDayEntry)
                {
                    xDayEntry = mpChecks->make_iterator();
                    mpChecks->insert(xMonthEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xDayEntry.get());
                    mpChecks->set_toggle(*xDayEntry, TRISTATE_FALSE);
                    mpChecks->set_text(*xDayEntry, rMember.maName, 0);
                }
                return xDayEntry; // Return leaf node
            }

            xEntry = mpChecks->make_iterator();
            mpChecks->append(xEntry.get());
            mpChecks->set_toggle(*xEntry, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE);
            mpChecks->set_text(*xEntry, sName, 0);
        }
        else
            CheckEntry(*xEntry, bCheck);
    }
    else if (xEntry)
    {
        mpChecks->remove(*xEntry);
        if (rMember.mxParent)
        {
            std::unique_ptr<weld::TreeIter> xParent(mpChecks->make_iterator(rMember.mxParent.get()));
            while (xParent && !mpChecks->iter_has_child(*xParent))
            {
                std::unique_ptr<weld::TreeIter> xTmp(mpChecks->make_iterator(xParent.get()));
                if (!mpChecks->iter_parent(*xParent))
                    xParent.reset();
                mpChecks->remove(*xTmp);
            }
        }
    }
    return nullptr;
}

int ScCheckListMenuControl::GetCheckedEntryCount() const
{
    int nRet = 0;

    mpChecks->all_foreach([this, &nRet](weld::TreeIter& rEntry){
        if (mpChecks->get_toggle(rEntry) == TRISTATE_TRUE)
            ++nRet;
        return false;
    });

    return nRet;
}

IMPL_LINK(ScCheckListMenuControl, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    const vcl::KeyCode& rKey = rKEvt.GetKeyCode();

    if ( rKey.GetCode() == KEY_RETURN || rKey.GetCode() == KEY_SPACE )
    {
        std::unique_ptr<weld::TreeIter> xEntry = mpChecks->make_iterator();
        bool bEntry = mpChecks->get_cursor(xEntry.get());
        if (bEntry)
        {
            bool bOldCheck = mpChecks->get_toggle(*xEntry) == TRISTATE_TRUE;
            CheckEntry(*xEntry, !bOldCheck);
            bool bNewCheck = mpChecks->get_toggle(*xEntry) == TRISTATE_TRUE;
            if (bOldCheck != bNewCheck)
                Check(xEntry.get());
        }
        return true;
    }

    return false;
}

size_t ScCheckListMenuControl::initMembers(int nMaxMemberWidth)
{
    size_t n = maMembers.size();
    size_t nVisMemCount = 0;

    if (nMaxMemberWidth == -1)
        nMaxMemberWidth = mnCheckWidthReq;

    if (!mpChecks->n_children() && !mbHasDates)
    {
        std::vector<int> aFixedWidths { nMaxMemberWidth };
        // tdf#134038 insert in the fastest order, this might be backwards so only do it for
        // the !mbHasDates case where no entry depends on another to exist before getting
        // inserted. We cannot retain pre-existing treeview content, only clear and fill it.
        mpChecks->bulk_insert_for_each(n, [this, &nVisMemCount](weld::TreeIter& rIter, int i) {
            assert(!maMembers[i].mbDate);
            insertMember(*mpChecks, rIter, maMembers[i], maMembers[i].mbVisible);
            if (maMembers[i].mbVisible)
                ++nVisMemCount;
        }, nullptr, &aFixedWidths);
    }
    else
    {
        mpChecks->freeze();

        std::unique_ptr<weld::TreeIter> xEntry = mpChecks->make_iterator();
        std::vector<std::unique_ptr<weld::TreeIter>> aExpandRows;

        for (size_t i = 0; i < n; ++i)
        {
            if (maMembers[i].mbDate)
            {
                CheckEntry(maMembers[i].maName, maMembers[i].mxParent.get(), maMembers[i].mbVisible);
                // Expand first node of checked dates
                if (!maMembers[i].mxParent && IsChecked(maMembers[i].maName, maMembers[i].mxParent.get()))
                {
                    std::unique_ptr<weld::TreeIter> xDateEntry = FindEntry(nullptr, maMembers[i].maName);
                    if (xDateEntry)
                        aExpandRows.emplace_back(std::move(xDateEntry));
                }
            }
            else
            {
                mpChecks->append(xEntry.get());
                insertMember(*mpChecks, *xEntry, maMembers[i], maMembers[i].mbVisible);
            }

            if (maMembers[i].mbVisible)
                ++nVisMemCount;
        }

        mpChecks->thaw();

        for (const auto& rRow : aExpandRows)
            mpChecks->expand_row(*rRow);
    }

    if (nVisMemCount == n)
    {
        // all members visible
        mxChkToggleAll->set_state(TRISTATE_TRUE);
        mePrevToggleAllState = TRISTATE_TRUE;
    }
    else if (nVisMemCount == 0)
    {
        // no members visible
        mxChkToggleAll->set_state(TRISTATE_FALSE);
        mePrevToggleAllState = TRISTATE_FALSE;
    }
    else
    {
        mxChkToggleAll->set_state(TRISTATE_INDET);
        mePrevToggleAllState = TRISTATE_INDET;
    }

    if (nVisMemCount)
        mpChecks->select(0);

    return nVisMemCount;
}

void ScCheckListMenuControl::setConfig(const Config& rConfig)
{
    maConfig = rConfig;
}

bool ScCheckListMenuControl::isAllSelected() const
{
    return mxChkToggleAll->get_state() == TRISTATE_TRUE;
}

void ScCheckListMenuControl::getResult(ResultType& rResult)
{
    ResultType aResult;
    std::unordered_set<OUString> vCheckeds = GetAllChecked();
    size_t n = maMembers.size();
    for (size_t i = 0; i < n; ++i)
    {
        if ( maMembers[i].mbLeaf )
        {
            OUStringBuffer aLabel(maMembers[i].maName);
            if (aLabel.isEmpty())
                aLabel = ScResId(STR_EMPTYDATA);

            /* TODO: performance-wise this looks suspicious, concatenating to
             * do the lookup for each leaf item seems wasteful. */
            // Checked labels are in the form "child;parent;grandparent".
            if (maMembers[i].mxParent)
            {
                std::unique_ptr<weld::TreeIter> xIter(mpChecks->make_iterator(maMembers[i].mxParent.get()));
                do
                {
                    aLabel.append(";" + mpChecks->get_text(*xIter));
                }
                while (mpChecks->iter_parent(*xIter));
            }

            bool bState = vCheckeds.find(aLabel.makeStringAndClear()) != vCheckeds.end();

            ResultEntry aResultEntry;
            aResultEntry.bValid = bState;
            aResultEntry.aName = maMembers[i].maRealName;
            aResultEntry.nValue = maMembers[i].mnValue;
            aResultEntry.bDate = maMembers[i].mbDate;
            aResultEntry.bValue = maMembers[i].mbValue;
            aResult.insert(aResultEntry);
        }
    }
    rResult.swap(aResult);
}

void ScCheckListMenuControl::launch(weld::Widget* pWidget, const tools::Rectangle& rRect)
{
    prepWindow();
    if (!maConfig.mbAllowEmptySet)
        // We need to have at least one member selected.
        mxBtnOk->set_sensitive(GetCheckedEntryCount() != 0);

    tools::Rectangle aRect(rRect);
    if (maConfig.mbRTL)
    {
        // In RTL mode, the logical "left" is visual "right".
        tools::Long nLeft = aRect.Left() - aRect.GetWidth();
        aRect.SetLeft( nLeft );
    }
    else if (mnWndWidth < aRect.GetWidth())
    {
        // Target rectangle (i.e. cell width) is wider than the window.
        // Simulate right-aligned launch by modifying the target rectangle
        // size.
        tools::Long nDiff = aRect.GetWidth() - mnWndWidth;
        aRect.AdjustLeft(nDiff );
    }

    StartPopupMode(pWidget, aRect);
}

void ScCheckListMenuControl::NotifyCloseLOK()
{
    if (mpNotifier)
    {
        tools::JsonWriter aJsonWriter;
        aJsonWriter.put("jsontype", "autofilter");
        aJsonWriter.put("action", "close");

        const std::string message = aJsonWriter.extractAsStdString();
        mpNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.c_str());
    }
}

void ScCheckListMenuControl::close(bool bOK)
{
    if (bOK && mxOKAction)
        mxOKAction->execute();
    EndPopupMode();

    if (comphelper::LibreOfficeKit::isActive())
        NotifyCloseLOK();
}

void ScCheckListMenuControl::setExtendedData(std::unique_ptr<ExtendedData> p)
{
    mxExtendedData = std::move(p);
}

ScCheckListMenuControl::ExtendedData* ScCheckListMenuControl::getExtendedData()
{
    return mxExtendedData.get();
}

void ScCheckListMenuControl::setOKAction(Action* p)
{
    mxOKAction.reset(p);
}

void ScCheckListMenuControl::setPopupEndAction(Action* p)
{
    mxPopupEndAction.reset(p);
}

IMPL_LINK_NOARG(ScCheckListMenuControl, PopupModeEndHdl, weld::Popover&, void)
{
    mbIsPoppedUp = false;
    clearSelectedMenuItem();
    if (mxPopupEndAction)
        mxPopupEndAction->execute();

    if (comphelper::LibreOfficeKit::isActive())
        NotifyCloseLOK();
}

int ScCheckListMenuControl::GetTextWidth(const OUString& rsName) const
{
    return mxDropDown->GetTextWidth(rsName);
}

int ScCheckListMenuControl::IncreaseWindowWidthToFitText(int nMaxTextWidth)
{
    int nBorder = nBorderWidth * 2 + 4;
    int nNewWidth = nMaxTextWidth - nBorder;
    if (nNewWidth > mnCheckWidthReq)
    {
        mnCheckWidthReq = nNewWidth;
        int nChecksHeight = mpChecks->get_height_rows(nCheckListVisibleRows);
        mpChecks->set_size_request(mnCheckWidthReq, nChecksHeight);
    }
    return mnCheckWidthReq + nBorder;
}

ScListSubMenuControl::ScListSubMenuControl(weld::Widget* pParent, ScCheckListMenuControl& rParentControl, bool bColorMenu, vcl::ILibreOfficeKitNotifier* pNotifier)
    : mxBuilder(Application::CreateBuilder(pParent, "modules/scalc/ui/filtersubdropdown.ui"))
    , mxPopover(mxBuilder->weld_popover("FilterSubDropDown"))
    , mxContainer(mxBuilder->weld_container("container"))
    , mxMenu(mxBuilder->weld_tree_view("menu"))
    , mxBackColorMenu(mxBuilder->weld_tree_view("background"))
    , mxTextColorMenu(mxBuilder->weld_tree_view("textcolor"))
    , mxScratchIter(mxMenu->make_iterator())
    , mrParentControl(rParentControl)
    , mpNotifier(pNotifier)
    , mnBackColorMenuPrefHeight(-1)
    , mnTextColorMenuPrefHeight(-1)
    , mbColorMenu(bColorMenu)
{
    mxMenu->hide();
    mxBackColorMenu->hide();
    mxTextColorMenu->hide();

    if (!bColorMenu)
    {
        SetupMenu(*mxMenu);
        mxMenu->show();
    }
    else
    {
        mxBackColorMenu->set_clicks_to_toggle(1);
        mxBackColorMenu->enable_toggle_buttons(weld::ColumnToggleType::Radio);
        mxBackColorMenu->connect_changed(LINK(this, ScListSubMenuControl, ColorSelChangedHdl));
        mxTextColorMenu->set_clicks_to_toggle(1);
        mxTextColorMenu->enable_toggle_buttons(weld::ColumnToggleType::Radio);
        mxTextColorMenu->connect_changed(LINK(this, ScListSubMenuControl, ColorSelChangedHdl));
        SetupMenu(*mxBackColorMenu);
        SetupMenu(*mxTextColorMenu);
    }
}

void ScListSubMenuControl::SetupMenu(weld::TreeView& rMenu)
{
    rMenu.connect_row_activated(LINK(this, ScListSubMenuControl, RowActivatedHdl));
    rMenu.connect_key_press(LINK(this, ScListSubMenuControl, MenuKeyInputHdl));
}

void ScListSubMenuControl::StartPopupMode(weld::Widget* pParent, const tools::Rectangle& rRect)
{
    if (mxPopupStartAction)
        mxPopupStartAction->execute();

    mxPopover->popup_at_rect(pParent, rRect, weld::Placement::End);

    weld::TreeView& rFirstMenu = mbColorMenu ? *mxBackColorMenu : *mxMenu;
    rFirstMenu.set_cursor(0);
    rFirstMenu.select(0);

    mrParentControl.setSubMenuFocused(this);
}

void ScListSubMenuControl::EndPopupMode()
{
    mxPopover->popdown();
}

void ScListSubMenuControl::GrabFocus()
{
    weld::TreeView& rFirstMenu = mbColorMenu ? *mxBackColorMenu : *mxMenu;
    rFirstMenu.grab_focus();
}

bool ScListSubMenuControl::IsVisible() const
{
    return mxPopover->get_visible();
}

void ScListSubMenuControl::resizeToFitMenuItems()
{
    if (!mbColorMenu)
        mxMenu->set_size_request(-1, mxMenu->get_preferred_size().Height());
    else
    {
        int nBackColorMenuPrefHeight = mnBackColorMenuPrefHeight;
        if (nBackColorMenuPrefHeight == -1)
            nBackColorMenuPrefHeight = mxBackColorMenu->get_preferred_size().Height();
        mxBackColorMenu->set_size_request(-1, nBackColorMenuPrefHeight);
        int nTextColorMenuPrefHeight = mnTextColorMenuPrefHeight;
        if (nTextColorMenuPrefHeight == -1)
            nTextColorMenuPrefHeight = mxTextColorMenu->get_preferred_size().Height();
        mxTextColorMenu->set_size_request(-1, nTextColorMenuPrefHeight);
    }
}

void ScListSubMenuControl::addItem(ScCheckListMenuControl::Action* pAction)
{
    ScCheckListMenuControl::MenuItemData aItem;
    aItem.mbEnabled = true;
    aItem.mxAction.reset(pAction);
    maMenuItems.emplace_back(std::move(aItem));
}

void ScListSubMenuControl::addMenuItem(const OUString& rText, ScCheckListMenuControl::Action* pAction)
{
    addItem(pAction);
    mxMenu->append(OUString::number(reinterpret_cast<sal_Int64>(pAction)), rText);
}

void ScListSubMenuControl::addMenuColorItem(const OUString& rText, bool bActive, VirtualDevice& rImage,
                                            int nMenu, ScCheckListMenuControl::Action* pAction)
{
    addItem(pAction);

    weld::TreeView& rColorMenu = nMenu == 0 ? *mxBackColorMenu : *mxTextColorMenu;
    rColorMenu.show();

    OUString sId = OUString::number(reinterpret_cast<sal_Int64>(pAction));
    rColorMenu.insert(nullptr, -1, &rText, &sId, nullptr, nullptr, false, mxScratchIter.get());
    rColorMenu.set_toggle(*mxScratchIter, bActive ? TRISTATE_TRUE : TRISTATE_FALSE);
    rColorMenu.set_image(*mxScratchIter, rImage);

    if (mnTextColorMenuPrefHeight == -1 && &rColorMenu == mxTextColorMenu.get() && mxTextColorMenu->n_children() == 8)
        mnTextColorMenuPrefHeight = mxTextColorMenu->get_preferred_size().Height();

    if (mnBackColorMenuPrefHeight == -1 && &rColorMenu == mxBackColorMenu.get() && mxBackColorMenu->n_children() == 8)
        mnBackColorMenuPrefHeight = mxBackColorMenu->get_preferred_size().Height();
}

void ScListSubMenuControl::addSeparator()
{
    ScCheckListMenuControl::MenuItemData aItem;
    maMenuItems.emplace_back(std::move(aItem));

    mxMenu->append_separator("separator" + OUString::number(maMenuItems.size()));
}

void ScListSubMenuControl::clearMenuItems()
{
    maMenuItems.clear();
    mxMenu->clear();
    mxBackColorMenu->clear();
    mnBackColorMenuPrefHeight = -1;
    mxTextColorMenu->clear();
    mnTextColorMenuPrefHeight = -1;
}

IMPL_LINK(ScListSubMenuControl, MenuKeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bConsumed = false;
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();

    switch (rKeyCode.GetCode())
    {
        case KEY_ESCAPE:
        case KEY_LEFT:
        {
            mrParentControl.endSubMenu(*this);
            bConsumed = true;
            break;
        }
        case KEY_SPACE:
        case KEY_RETURN:
        {
            weld::TreeView& rMenu = !mbColorMenu ? *mxMenu :
                                    (mxBackColorMenu->has_focus() ? *mxBackColorMenu : *mxTextColorMenu);
            // don't toggle checkbutton, go straight to activating entry
            bConsumed = RowActivatedHdl(rMenu);
            break;
        }
        case KEY_DOWN:
        {
            if (mxTextColorMenu->get_visible() &&
                mxBackColorMenu->has_focus() &&
                mxBackColorMenu->get_selected_index() == mxBackColorMenu->n_children() - 1)
            {
                mxBackColorMenu->unselect_all();
                mxTextColorMenu->select(0);
                mxTextColorMenu->set_cursor(0);
                mxTextColorMenu->grab_focus();
                bConsumed = true;
            }
            break;
        }
        case KEY_UP:
        {
            if (mxBackColorMenu->get_visible() &&
                mxTextColorMenu->has_focus() &&
                mxTextColorMenu->get_selected_index() == 0)
            {
                mxTextColorMenu->unselect_all();
                int nIndex = mxBackColorMenu->n_children() - 1;
                mxBackColorMenu->select(nIndex);
                mxBackColorMenu->set_cursor(nIndex);
                mxBackColorMenu->grab_focus();
                bConsumed = true;
            }
            break;
        }
    }

    return bConsumed;
}

IMPL_LINK(ScListSubMenuControl, ColorSelChangedHdl, weld::TreeView&, rMenu, void)
{
    if (rMenu.get_selected_index() == -1)
        return;
    if (&rMenu != mxTextColorMenu.get())
        mxTextColorMenu->unselect_all();
    else
        mxBackColorMenu->unselect_all();
    rMenu.grab_focus();
}

IMPL_LINK(ScListSubMenuControl, RowActivatedHdl, weld::TreeView&, rMenu, bool)
{
    executeMenuItem(reinterpret_cast<ScCheckListMenuControl::Action*>(rMenu.get_selected_id().toInt64()));
    return true;
}

void ScListSubMenuControl::executeMenuItem(ScCheckListMenuControl::Action* pAction)
{
    // if no action is defined.
    if (!pAction)
        return;

    const bool bClosePopup = pAction->execute();
    if (bClosePopup)
        terminateAllPopupMenus();
}

void ScListSubMenuControl::setPopupStartAction(ScCheckListMenuControl::Action* p)
{
    mxPopupStartAction.reset(p);
}

void ScListSubMenuControl::terminateAllPopupMenus()
{
    if (comphelper::LibreOfficeKit::isActive())
        NotifyCloseLOK();

    EndPopupMode();
    mrParentControl.terminateAllPopupMenus();
}

void ScListSubMenuControl::NotifyCloseLOK()
{
    if (mpNotifier)
    {
        tools::JsonWriter aJsonWriter;
        aJsonWriter.put("jsontype", "autofilter");
        aJsonWriter.put("action", "close");

        const std::string message = aJsonWriter.extractAsStdString();
        mpNotifier->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.c_str());
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
