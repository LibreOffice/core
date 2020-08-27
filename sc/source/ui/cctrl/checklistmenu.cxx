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
#include <globstr.hrc>
#include <scresid.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>

#include <vcl/decoview.hxx>
#include <vcl/event.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <rtl/math.hxx>
#include <tools/wintypes.hxx>
#include <unotools/charclass.hxx>

#include <document.hxx>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;

ScCheckListMenuControl::MenuItemData::MenuItemData()
    : mbEnabled(true)
{
}

ScCheckListMenuControl::SubMenuItemData::SubMenuItemData(ScCheckListMenuControl* pParent)
    : mpSubMenu(nullptr)
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
        case KEY_LEFT:
        {
            ScCheckListMenuWindow* pParentMenu = mxFrame->GetParentMenu();
            if (pParentMenu)
                pParentMenu->get_widget().endSubMenu(*this);
            break;
        }
        case KEY_RIGHT:
        {
            if (mnSelectedMenu >= maMenuItems.size() || mnSelectedMenu == MENU_NOT_SELECTED)
                break;

            const MenuItemData& rMenu = maMenuItems[mnSelectedMenu];
            if (!rMenu.mbEnabled || !rMenu.mxSubMenuWin)
                break;

            maOpenTimer.mnMenuPos = mnSelectedMenu;
            maOpenTimer.mpSubMenu = rMenu.mxSubMenuWin.get();
            launchSubMenu(true);
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
        // became unselected
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

    mxMenu->append_text(rText);
    if (mbCanHaveSubMenu)
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
    assert(mbCanHaveSubMenu);
    std::vector<int> aWidths;
    aWidths.push_back(rSize.Width() - (mxMenu->get_text_height() * 3) / 4 - 6);
    mxMenu->set_column_fixed_widths(aWidths);
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

ScCheckListMenuWindow* ScCheckListMenuControl::addSubMenuItem(const OUString& rText, bool bEnabled)
{
    assert(mbCanHaveSubMenu);

    MenuItemData aItem;
    aItem.mbEnabled = bEnabled;
    vcl::Window *pContainer = mxFrame->GetWindow(GetWindowType::FirstChild);
    aItem.mxSubMenuWin.reset(VclPtr<ScCheckListMenuWindow>::Create(pContainer, mpDoc, false, -1, mxFrame.get()));
    maMenuItems.emplace_back(std::move(aItem));

    mxMenu->append_text(rText);
    if (mbCanHaveSubMenu)
        mxMenu->set_image(mxMenu->n_children() - 1, *mxDropDown, 1);

    return maMenuItems.back().mxSubMenuWin.get();
}

void ScCheckListMenuControl::executeMenuItem(size_t nPos)
{
    if (nPos >= maMenuItems.size())
        return;

    if (!maMenuItems[nPos].mxAction)
        // no action is defined.
        return;

    terminateAllPopupMenus();

    maMenuItems[nPos].mxAction->execute();
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
            vcl::Window::GetDockingManager()->EndPopupMode(maCloseTimer.mpSubMenu);
            maCloseTimer.mpSubMenu = nullptr;
            maCloseTimer.maTimer.Stop();
        }

        launchSubMenu(false);
    }
    else if (pTimer == &maCloseTimer)
    {
        // end submenu.
        if (maCloseTimer.mpSubMenu)
        {
            maOpenTimer.mpSubMenu = nullptr;

            vcl::Window::GetDockingManager()->EndPopupMode(maCloseTimer.mpSubMenu);
            maCloseTimer.mpSubMenu = nullptr;

            maOpenTimer.mnMenuPos = MENU_NOT_SELECTED;
        }
    }
}

void ScCheckListMenuControl::queueLaunchSubMenu(size_t nPos, ScCheckListMenuWindow* pMenu)
{
    if (!pMenu)
        return;

    // Set the submenu on launch queue.
    if (maOpenTimer.mpSubMenu)
    {
        if (maOpenTimer.mpSubMenu == pMenu)
        {
            if (pMenu == maCloseTimer.mpSubMenu)
                maCloseTimer.reset();
            return;
        }

        // new submenu is being requested.
        queueCloseSubMenu();
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

    maCloseTimer.mpSubMenu = maOpenTimer.mpSubMenu;
    maCloseTimer.mnMenuPos = maOpenTimer.mnMenuPos;
    maCloseTimer.maTimer.Start();
}

void ScCheckListMenuControl::launchSubMenu(bool bSetMenuPos)
{
    ScCheckListMenuWindow* pSubMenu = maOpenTimer.mpSubMenu;
    if (!pSubMenu)
        return;

    if (!mxMenu->get_selected(mxScratchIter.get()))
        return;

    tools::Rectangle aRect = mxMenu->get_row_area(*mxScratchIter);
    ScCheckListMenuControl& rSubMenuControl = pSubMenu->get_widget();
    rSubMenuControl.StartPopupMode(aRect, FloatWinPopupFlags::Right);
    if (bSetMenuPos)
        rSubMenuControl.setSelectedMenuItem(0, false); // select menu item after the popup becomes fully visible.

    mxMenu->select(*mxScratchIter);
    rSubMenuControl.GrabFocus();
}

IMPL_LINK_NOARG(ScCheckListMenuControl, PostPopdownHdl, void*, void)
{
    mnAsyncPostPopdownId = nullptr;
    mxMenu->grab_focus();
}

void ScCheckListMenuControl::endSubMenu(ScCheckListMenuControl& rSubMenu)
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

void ScCheckListMenuControl::resizeToFitMenuItems()
{
    mxMenu->set_size_request(-1, mxMenu->get_preferred_size().Height() + 2);
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

    ScCheckListMenuWindow* pParentMenu = mxFrame->GetParentMenu();
    if (pParentMenu)
        pParentMenu->get_widget().setSubMenuFocused(this);

    if (bSubMenuTimer)
    {
        if (maMenuItems[nPos].mxSubMenuWin)
        {
            ScCheckListMenuWindow* pSubMenu = maMenuItems[nPos].mxSubMenuWin.get();
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

size_t ScCheckListMenuControl::getSubMenuPos(const ScCheckListMenuControl* pSubMenu)
{
    size_t n = maMenuItems.size();
    for (size_t i = 0; i < n; ++i)
    {
        if (!maMenuItems[i].mxSubMenuWin)
            continue;
        if (&maMenuItems[i].mxSubMenuWin->get_widget() == pSubMenu)
            return i;
    }
    return MENU_NOT_SELECTED;
}

void ScCheckListMenuControl::setSubMenuFocused(const ScCheckListMenuControl* pSubMenu)
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
    vcl::Window::GetDockingManager()->EndPopupMode(mxFrame);
    mxFrame->EnableDocking(false);
}

void ScCheckListMenuControl::StartPopupMode(const tools::Rectangle& rRect, FloatWinPopupFlags eFlags)
{
    mxFrame->EnableDocking(true);
    DockingManager* pDockingManager = vcl::Window::GetDockingManager();
    pDockingManager->SetPopupModeEndHdl(mxFrame, LINK(this, ScCheckListMenuControl, PopupModeEndHdl));
    pDockingManager->StartPopupMode(mxFrame, rRect, (eFlags | FloatWinPopupFlags::GrabFocus));
}

void ScCheckListMenuControl::terminateAllPopupMenus()
{
    EndPopupMode();
    ScCheckListMenuWindow* pParentMenu = mxFrame->GetParentMenu();
    if (pParentMenu)
        pParentMenu->get_widget().terminateAllPopupMenus();
}

ScCheckListMenuControl::Config::Config() :
    mbAllowEmptySet(true), mbRTL(false)
{
}

ScCheckListMember::ScCheckListMember()
    : mbVisible(true)
    , mbDate(false)
    , mbLeaf(false)
    , meDatePartType(YEAR)
{
}

ScCheckListMenuControl::ScCheckListMenuControl(ScCheckListMenuWindow* pParent, vcl::Window* pContainer,
                                               ScDocument* pDoc, bool bCanHaveSubMenu, int nWidth)
    : mxFrame(pParent)
    , mxBuilder(Application::CreateInterimBuilder(pContainer, "modules/scalc/ui/filterdropdown.ui", false))
    , mxContainer(mxBuilder->weld_container("FilterDropDown"))
    , mxMenu(mxBuilder->weld_tree_view("menu"))
    , mxScratchIter(mxMenu->make_iterator())
    , mxEdSearch(mxBuilder->weld_entry("search_edit"))
    , mxBox(mxBuilder->weld_widget("box"))
    , mxChecks(mxBuilder->weld_tree_view("check_list_box"))
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
    , mpDoc(pDoc)
    , mnAsyncPostPopdownId(nullptr)
    , mbHasDates(false)
    , mbCanHaveSubMenu(bCanHaveSubMenu)
    , maOpenTimer(this)
    , maCloseTimer(this)
{
    bool bIsSubMenu = pParent->GetParentMenu();

    int nChecksHeight = mxChecks->get_height_rows(9);
    if (!bIsSubMenu && nWidth != -1)
    {
        mnCheckWidthReq = nWidth - mxFrame->get_border_width() * 2 - 4;
        mxChecks->set_size_request(mnCheckWidthReq, nChecksHeight);
    }

    // sort ok/cancel into native order, if this was a dialog they would be auto-sorted, but this
    // popup isn't a true dialog
    mxButtonBox->sort_native_button_order();

    if (!bIsSubMenu)
    {
        mxChecks->enable_toggle_buttons(weld::ColumnToggleType::Check);

        mxBox->show();
        mxEdSearch->show();
        mxButtonBox->show();
    }

    mxContainer->connect_focus_in(LINK(this, ScCheckListMenuControl, FocusHdl));
    mxMenu->connect_row_activated(LINK(this, ScCheckListMenuControl, RowActivatedHdl));
    mxMenu->connect_changed(LINK(this, ScCheckListMenuControl, SelectHdl));
    mxMenu->connect_key_press(LINK(this, ScCheckListMenuControl, MenuKeyInputHdl));

    if (!bIsSubMenu)
    {
        mxBtnOk->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));
        mxBtnCancel->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));
        mxEdSearch->connect_changed(LINK(this, ScCheckListMenuControl, EdModifyHdl));
        mxEdSearch->connect_activate(LINK(this, ScCheckListMenuControl, EdActivateHdl));
        mxChecks->connect_toggled(LINK(this, ScCheckListMenuControl, CheckHdl));
        mxChecks->connect_key_press(LINK(this, ScCheckListMenuControl, KeyInputHdl));
        mxChkToggleAll->connect_toggled(LINK(this, ScCheckListMenuControl, TriStateHdl));
        mxBtnSelectSingle->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));
        mxBtnUnselectSingle->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));
    }

    if (mbCanHaveSubMenu)
    {
        CreateDropDown();
        mxMenu->connect_size_allocate(LINK(this, ScCheckListMenuControl, TreeSizeAllocHdl));
    }

    if (!bIsSubMenu)
    {
        // determine what width the checklist will end up with
        mnCheckWidthReq = mxContainer->get_preferred_size().Width();
        // make that size fixed now, we can now use mnCheckWidthReq to speed up
        // bulk_insert_for_each
        mxChecks->set_size_request(mnCheckWidthReq, nChecksHeight);
    }
}

IMPL_LINK_NOARG(ScCheckListMenuControl, FocusHdl, weld::Widget&, void)
{
    GrabFocus();
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
        rMenuItem.mxSubMenuWin.disposeAndClear();
    if (mnAsyncPostPopdownId)
    {
        Application::RemoveUserEvent(mnAsyncPostPopdownId);
        mnAsyncPostPopdownId = nullptr;
    }
}

ScCheckListMenuWindow::ScCheckListMenuWindow(vcl::Window* pParent, ScDocument* pDoc, bool bCanHaveSubMenu,
                                             int nWidth, ScCheckListMenuWindow* pParentMenu)
    : DockingWindow(pParent, "InterimDockParent", "svx/ui/interimdockparent.ui")
    , mxParentMenu(pParentMenu)
    , mxBox(get("box"))
{
    setDeferredProperties();
    mxControl.reset(new ScCheckListMenuControl(this, mxBox.get(), pDoc, bCanHaveSubMenu, nWidth));
    SetBackground(Application::GetSettings().GetStyleSettings().GetMenuColor());
    set_id("check_list_menu");
}

bool ScCheckListMenuWindow::EventNotify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE)
    {
        ScCheckListMenuControl& rMenuControl = get_widget();
        rMenuControl.queueCloseSubMenu();
        rMenuControl.clearSelectedMenuItem();
    }
    return DockingWindow::EventNotify(rNEvt);
}

ScCheckListMenuWindow::~ScCheckListMenuWindow()
{
    disposeOnce();
}

void ScCheckListMenuWindow::dispose()
{
    mxControl.reset();
    mxBox.disposeAndClear();
    mxParentMenu.clear();
    DockingWindow::dispose();
}

void ScCheckListMenuWindow::GetFocus()
{
    DockingWindow::GetFocus();
    if (!mxControl)
        return;
    mxControl->GrabFocus();
}

void ScCheckListMenuControl::prepWindow()
{
    mxMenu->set_size_request(-1, mxMenu->get_preferred_size().Height() + 2);
    mnSelectedMenu = 0;
    mxMenu->set_cursor(mnSelectedMenu);
    mxMenu->unselect_all();

    mnWndWidth = mxContainer->get_preferred_size().Width() + mxFrame->get_border_width() * 2 + 4;
}

void ScCheckListMenuControl::setAllMemberState(bool bSet)
{
    CheckAllChildren(nullptr, bSet);

    if (!maConfig.mbAllowEmptySet)
    {
        // We need to have at least one member selected.
        mxBtnOk->set_sensitive(GetCheckedEntryCount() != 0);
    }
}

void ScCheckListMenuControl::selectCurrentMemberOnly(bool bSet)
{
    setAllMemberState(!bSet);
    std::unique_ptr<weld::TreeIter> xEntry = mxChecks->make_iterator();
    if (!mxChecks->get_cursor(xEntry.get()))
        return;
    mxChecks->set_toggle(*xEntry, bSet ? TRISTATE_TRUE : TRISTATE_FALSE);
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
        std::unique_ptr<weld::TreeIter> xEntry = mxChecks->make_iterator();
        if (!mxChecks->get_cursor(xEntry.get()))
            xEntry.reset();
        Check(xEntry.get());
    }
}

IMPL_LINK_NOARG(ScCheckListMenuControl, TriStateHdl, weld::ToggleButton&, void)
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

IMPL_LINK_NOARG(ScCheckListMenuControl, EdModifyHdl, weld::Entry&, void)
{
    OUString aSearchText = mxEdSearch->get_text();
    aSearchText = ScGlobal::getCharClassPtr()->lowercase( aSearchText );
    bool bSearchTextEmpty = aSearchText.isEmpty();
    size_t n = maMembers.size();
    size_t nSelCount = 0;
    bool bSomeDateDeletes = false;

    mxChecks->freeze();

    if (bSearchTextEmpty && !mbHasDates)
    {
        // when there are a lot of rows, it is cheaper to simply clear the tree and re-initialise
        mxChecks->clear();
        nSelCount = initMembers();
    }
    else
    {
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
                    bPartialMatch = ( ScGlobal::getCharClassPtr()->lowercase( aLabelDisp ).indexOf( aSearchText ) != -1 );
                else if ( maMembers[i].meDatePartType == ScCheckListMember::DAY ) // Match with both numerical and text version of month
                    bPartialMatch = (ScGlobal::getCharClassPtr()->lowercase( OUString(
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

    mxChecks->thaw();

    if ( nSelCount == n )
        mxChkToggleAll->set_state( TRISTATE_TRUE );
    else if ( nSelCount == 0 )
        mxChkToggleAll->set_state( TRISTATE_FALSE );
    else
        mxChkToggleAll->set_state( TRISTATE_INDET );

    if ( !maConfig.mbAllowEmptySet )
    {
        const bool bEmptySet( nSelCount == 0 );
        mxChecks->set_sensitive(!bEmptySet);
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
        CheckEntry(pEntry,  mxChecks->get_toggle(*pEntry) == TRISTATE_TRUE);
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
        std::unique_ptr<weld::TreeIter> xMonthEntry = mxChecks->make_iterator(pLeaf);
        if (!mxChecks->iter_parent(*xMonthEntry))
            xMonthEntry.reset();
        else
        {
            xYearEntry = mxChecks->make_iterator(xMonthEntry.get());
            if (!mxChecks->iter_parent(*xYearEntry))
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
    SvNumberFormatter* pFormatter = mpDoc->GetFormatTable();

    // Convert the numeric date value to a date object.
    Date aDate = pFormatter->GetNullDate();
    aDate.AddDays(rtl::math::approxFloor(nVal));

    sal_Int16 nYear = aDate.GetYear();
    sal_uInt16 nMonth = aDate.GetMonth();
    sal_uInt16 nDay = aDate.GetDay();

    // Get the localized month name list.
    CalendarWrapper* pCalendar = ScGlobal::GetCalendar();
    uno::Sequence<i18n::CalendarItem2> aMonths = pCalendar->getMonths();
    if (aMonths.getLength() < nMonth)
        return;

    OUString aYearName = OUString::number(nYear);
    OUString aMonthName = aMonths[nMonth-1].FullName;
    OUString aDayName = OUString::number(nDay);

    if ( aDayName.getLength() == 1 )
        aDayName = "0" + aDayName;

    mxChecks->freeze();

    std::unique_ptr<weld::TreeIter> xYearEntry = FindEntry(nullptr, aYearName);
    if (!xYearEntry)
    {
        xYearEntry = mxChecks->make_iterator();
        mxChecks->insert(nullptr, -1, nullptr, nullptr, nullptr, nullptr, false, xYearEntry.get());
        mxChecks->set_toggle(*xYearEntry, TRISTATE_FALSE);
        mxChecks->set_text(*xYearEntry, aYearName, 0);
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
        xMonthEntry = mxChecks->make_iterator();
        mxChecks->insert(xYearEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xMonthEntry.get());
        mxChecks->set_toggle(*xMonthEntry, TRISTATE_FALSE);
        mxChecks->set_text(*xMonthEntry, aMonthName, 0);
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
        xDayEntry = mxChecks->make_iterator();
        mxChecks->insert(xMonthEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xDayEntry.get());
        mxChecks->set_toggle(*xDayEntry, TRISTATE_FALSE);
        mxChecks->set_text(*xDayEntry, aDayName, 0);
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

    mxChecks->thaw();
}

void ScCheckListMenuControl::addMember(const OUString& rName, bool bVisible)
{
    ScCheckListMember aMember;
    aMember.maName = rName;
    aMember.mbDate = false;
    aMember.mbLeaf = true;
    aMember.mbVisible = bVisible;
    aMember.mxParent.reset();
    maMembers.emplace_back(std::move(aMember));
}

std::unique_ptr<weld::TreeIter> ScCheckListMenuControl::FindEntry(const weld::TreeIter* pParent, const OUString& sNode)
{
    std::unique_ptr<weld::TreeIter> xEntry = mxChecks->make_iterator(pParent);
    bool bEntry = pParent ? mxChecks->iter_children(*xEntry) : mxChecks->get_iter_first(*xEntry);
    while (bEntry)
    {
        if (sNode == mxChecks->get_text(*xEntry, 0))
            return xEntry;
        bEntry = mxChecks->iter_next_sibling(*xEntry);
    }
    return nullptr;
}

void ScCheckListMenuControl::GetRecursiveChecked(const weld::TreeIter* pEntry, std::unordered_set<OUString>& vOut,
                                                 OUString& rLabel)
{
    if (mxChecks->get_toggle(*pEntry) != TRISTATE_TRUE)
        return;

    // We have to hash parents and children together.
    // Per convention for easy access in getResult()
    // "child;parent;grandparent" while descending.
    if (rLabel.isEmpty())
        rLabel = mxChecks->get_text(*pEntry, 0);
    else
        rLabel = mxChecks->get_text(*pEntry, 0) + ";" + rLabel;

    // Prerequisite: the selection mechanism guarantees that if a child is
    // selected then also the parent is selected, so we only have to
    // inspect the children in case the parent is selected.
    if (!mxChecks->iter_has_child(*pEntry))
        return;

    std::unique_ptr<weld::TreeIter> xChild(mxChecks->make_iterator(pEntry));
    bool bChild = mxChecks->iter_children(*xChild);
    while (bChild)
    {
        OUString aLabel = rLabel;
        GetRecursiveChecked(xChild.get(), vOut, aLabel);
        if (!aLabel.isEmpty() && aLabel != rLabel)
            vOut.insert(aLabel);
        bChild = mxChecks->iter_next_sibling(*xChild);
    }
    // Let the caller not add the parent alone.
    rLabel.clear();
}

std::unordered_set<OUString> ScCheckListMenuControl::GetAllChecked()
{
    std::unordered_set<OUString> vResults(0);

    std::unique_ptr<weld::TreeIter> xEntry = mxChecks->make_iterator();
    bool bEntry = mxChecks->get_iter_first(*xEntry);
    while (bEntry)
    {
        OUString aLabel;
        GetRecursiveChecked(xEntry.get(), vResults, aLabel);
        if (!aLabel.isEmpty())
            vResults.insert(aLabel);
        bEntry = mxChecks->iter_next_sibling(*xEntry);
    }

    return vResults;
}

bool ScCheckListMenuControl::IsChecked(const OUString& sName, const weld::TreeIter* pParent)
{
    std::unique_ptr<weld::TreeIter> xEntry = FindEntry(pParent, sName);
    return xEntry && mxChecks->get_toggle(*xEntry) == TRISTATE_TRUE;
}

void ScCheckListMenuControl::CheckEntry(const OUString& sName, const weld::TreeIter* pParent, bool bCheck)
{
    std::unique_ptr<weld::TreeIter> xEntry = FindEntry(pParent, sName);
    if (xEntry)
        CheckEntry(xEntry.get(), bCheck);
}

// Recursively check all children of pParent
void ScCheckListMenuControl::CheckAllChildren(const weld::TreeIter* pParent, bool bCheck)
{
    if (pParent)
        mxChecks->set_toggle(*pParent, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE);
    std::unique_ptr<weld::TreeIter> xEntry = mxChecks->make_iterator(pParent);
    bool bEntry = pParent ? mxChecks->iter_children(*xEntry) : mxChecks->get_iter_first(*xEntry);
    while (bEntry)
    {
        CheckAllChildren(xEntry.get(), bCheck);
        bEntry = mxChecks->iter_next_sibling(*xEntry);
    }
}

void ScCheckListMenuControl::CheckEntry(const weld::TreeIter* pParent, bool bCheck)
{
    // recursively check all items below pParent
    CheckAllChildren(pParent, bCheck);
    // checking pParent can affect ancestors, e.g. if ancestor is unchecked and pParent is
    // now checked then the ancestor needs to be checked also
    if (!(pParent && mxChecks->get_iter_depth(*pParent)))
        return;

    std::unique_ptr<weld::TreeIter> xAncestor(mxChecks->make_iterator(pParent));
    bool bAncestor = mxChecks->iter_parent(*xAncestor);
    while (bAncestor)
    {
        // if any first level children checked then ancestor
        // needs to be checked, similarly if no first level children
        // checked then ancestor needs to be unchecked
        std::unique_ptr<weld::TreeIter> xChild(mxChecks->make_iterator(xAncestor.get()));
        bool bChild = mxChecks->iter_children(*xChild);
        bool bChildChecked = false;

        while (bChild)
        {
            if (mxChecks->get_toggle(*xChild) == TRISTATE_TRUE)
            {
                bChildChecked = true;
                break;
            }
            bChild = mxChecks->iter_next_sibling(*xChild);
        }
        mxChecks->set_toggle(*xAncestor, bChildChecked ? TRISTATE_TRUE : TRISTATE_FALSE);
        bAncestor = mxChecks->iter_parent(*xAncestor);
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
                    xYearEntry = mxChecks->make_iterator();
                    mxChecks->insert(nullptr, -1, nullptr, nullptr, nullptr, nullptr, false, xYearEntry.get());
                    mxChecks->set_toggle(*xYearEntry, TRISTATE_FALSE);
                    mxChecks->set_text(*xYearEntry, rMember.maDateParts[0], 0);
                }
                std::unique_ptr<weld::TreeIter> xMonthEntry = FindEntry(xYearEntry.get(), rMember.maDateParts[1]);
                if (!xMonthEntry)
                {
                    xMonthEntry = mxChecks->make_iterator();
                    mxChecks->insert(xYearEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xMonthEntry.get());
                    mxChecks->set_toggle(*xMonthEntry, TRISTATE_FALSE);
                    mxChecks->set_text(*xMonthEntry, rMember.maDateParts[1], 0);
                }
                std::unique_ptr<weld::TreeIter> xDayEntry = FindEntry(xMonthEntry.get(), rMember.maName);
                if (!xDayEntry)
                {
                    xDayEntry = mxChecks->make_iterator();
                    mxChecks->insert(xMonthEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, false, xDayEntry.get());
                    mxChecks->set_toggle(*xDayEntry, TRISTATE_FALSE);
                    mxChecks->set_text(*xDayEntry, rMember.maName, 0);
                }
                return xDayEntry; // Return leaf node
            }

            xEntry = mxChecks->make_iterator();
            mxChecks->append(xEntry.get());
            mxChecks->set_toggle(*xEntry, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE);
            mxChecks->set_text(*xEntry, sName, 0);
        }
        else
            CheckEntry(xEntry.get(), bCheck);
    }
    else if (xEntry)
    {
        mxChecks->remove(*xEntry);
        if (rMember.mxParent)
        {
            std::unique_ptr<weld::TreeIter> xParent(mxChecks->make_iterator(rMember.mxParent.get()));
            while (xParent && !mxChecks->iter_has_child(*xParent))
            {
                std::unique_ptr<weld::TreeIter> xTmp(mxChecks->make_iterator(xParent.get()));
                if (!mxChecks->iter_parent(*xParent))
                    xParent.reset();
                mxChecks->remove(*xTmp);
            }
        }
    }
    return nullptr;
}

int ScCheckListMenuControl::GetCheckedEntryCount() const
{
    int nRet = 0;

    mxChecks->all_foreach([this, &nRet](weld::TreeIter& rEntry){
        if (mxChecks->get_toggle(rEntry) == TRISTATE_TRUE)
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
        std::unique_ptr<weld::TreeIter> xEntry = mxChecks->make_iterator();
        bool bEntry = mxChecks->get_cursor(xEntry.get());
        if (bEntry)
        {
            bool bOldCheck = mxChecks->get_toggle(*xEntry) == TRISTATE_TRUE;
            CheckEntry(xEntry.get(), !bOldCheck);
            bool bNewCheck = mxChecks->get_toggle(*xEntry) == TRISTATE_TRUE;
            if (bOldCheck != bNewCheck)
                Check(xEntry.get());
        }
        return true;
    }

    return false;
}

void ScCheckListMenuControl::setHasDates(bool bHasDates)
{
    mbHasDates = bHasDates;
    mxChecks->set_show_expanders(mbHasDates);
}

namespace
{
    void insertMember(weld::TreeView& rView, weld::TreeIter& rIter, const ScCheckListMember& rMember)
    {
        OUString aLabel = rMember.maName;
        if (aLabel.isEmpty())
            aLabel = ScResId(STR_EMPTYDATA);
        rView.set_toggle(rIter, rMember.mbVisible ? TRISTATE_TRUE : TRISTATE_FALSE);
        rView.set_text(rIter, aLabel, 0);
    }
}

size_t ScCheckListMenuControl::initMembers()
{
    size_t n = maMembers.size();
    size_t nVisMemCount = 0;

    if (!mxChecks->n_children() && !mbHasDates)
    {
        std::vector<int> aFixedWidths { mnCheckWidthReq };
        // tdf#134038 insert in the fastest order, this might be backwards so only do it for
        // the !mbHasDates case where no entry depends on another to exist before getting
        // inserted. We cannot retain pre-existing treeview content, only clear and fill it.
        mxChecks->bulk_insert_for_each(n, [this, &nVisMemCount](weld::TreeIter& rIter, int i) {
            assert(!maMembers[i].mbDate);
            insertMember(*mxChecks, rIter, maMembers[i]);
            if (maMembers[i].mbVisible)
                ++nVisMemCount;
        }, &aFixedWidths);
    }
    else
    {
        mxChecks->freeze();

        std::unique_ptr<weld::TreeIter> xEntry = mxChecks->make_iterator();
        std::vector<std::unique_ptr<weld::TreeIter>> aExpandRows;

        for (size_t i = 0; i < n; ++i)
        {
            if (maMembers[i].mbDate)
            {
                CheckEntry(maMembers[i].maName, maMembers[i].mxParent.get(), maMembers[i].mbVisible);
                // Expand first node of checked dates
                if (!maMembers[i].mxParent && IsChecked(maMembers[i].maName,  maMembers[i].mxParent.get()))
                {
                    std::unique_ptr<weld::TreeIter> xDateEntry = FindEntry(nullptr, maMembers[i].maName);
                    if (xDateEntry)
                        aExpandRows.emplace_back(std::move(xDateEntry));
                }
            }
            else
            {
                mxChecks->append(xEntry.get());
                insertMember(*mxChecks, *xEntry, maMembers[i]);
            }

            if (maMembers[i].mbVisible)
                ++nVisMemCount;
        }

        mxChecks->thaw();

        for (auto& rRow : aExpandRows)
            mxChecks->expand_row(*rRow);
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
        mxChecks->select(0);

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
            OUStringBuffer aLabel = maMembers[i].maName;
            if (aLabel.isEmpty())
                aLabel = ScResId(STR_EMPTYDATA);

            /* TODO: performance-wise this looks suspicious, concatenating to
             * do the lookup for each leaf item seems wasteful. */
            // Checked labels are in the form "child;parent;grandparent".
            if (maMembers[i].mxParent)
            {
                std::unique_ptr<weld::TreeIter> xIter(mxChecks->make_iterator(maMembers[i].mxParent.get()));
                do
                {
                    aLabel.append(";").append(mxChecks->get_text(*xIter));
                }
                while (mxChecks->iter_parent(*xIter));
            }

            bool bState = vCheckeds.find(aLabel.makeStringAndClear()) != vCheckeds.end();

            ResultEntry aResultEntry;
            aResultEntry.bValid = bState;
            if ( maMembers[i].mbDate )
                aResultEntry.aName = maMembers[i].maRealName;
            else
                aResultEntry.aName = maMembers[i].maName;
            aResultEntry.bDate = maMembers[i].mbDate;
            aResult.insert(aResultEntry);
        }
    }
    rResult.swap(aResult);
}

void ScCheckListMenuControl::launch(const tools::Rectangle& rRect)
{
    prepWindow();
    if (!maConfig.mbAllowEmptySet)
        // We need to have at least one member selected.
        mxBtnOk->set_sensitive(GetCheckedEntryCount() != 0);

    tools::Rectangle aRect(rRect);
    if (maConfig.mbRTL)
    {
        // In RTL mode, the logical "left" is visual "right".
        long nLeft = aRect.Left() - aRect.GetWidth();
        aRect.SetLeft( nLeft );
    }
    else if (mnWndWidth < aRect.GetWidth())
    {
        // Target rectangle (i.e. cell width) is wider than the window.
        // Simulate right-aligned launch by modifying the target rectangle
        // size.
        long nDiff = aRect.GetWidth() - mnWndWidth;
        aRect.AdjustLeft(nDiff );
    }

    StartPopupMode(aRect, FloatWinPopupFlags::Down);
}

void ScCheckListMenuControl::close(bool bOK)
{
    if (bOK && mxOKAction)
        mxOKAction->execute();
    EndPopupMode();
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

IMPL_LINK_NOARG(ScCheckListMenuControl, PopupModeEndHdl, FloatingWindow*, void)
{
    clearSelectedMenuItem();
    if (mxPopupEndAction)
        mxPopupEndAction->execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
