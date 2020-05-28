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
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <rtl/math.hxx>
#include <tools/wintypes.hxx>
#include <unotools/charclass.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <document.hxx>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::accessibility::XAccessibleContext;

ScCheckListMenuControl::MenuItemData::MenuItemData()
    : mbEnabled(true)
    , mbSeparator(false)
{
}

#if 0
ScMenuFloatingWindow::SubMenuItemData::SubMenuItemData(ScMenuFloatingWindow* pParent) :
    mpSubMenu(nullptr),
    mnMenuPos(MENU_NOT_SELECTED),
    mpParent(pParent)
{
    maTimer.SetInvokeHandler( LINK(this, ScMenuFloatingWindow::SubMenuItemData, TimeoutHdl) );
    maTimer.SetTimeout(mpParent->GetSettings().GetMouseSettings().GetMenuDelay());
}

void ScMenuFloatingWindow::SubMenuItemData::reset()
{
    mpSubMenu = nullptr;
    mnMenuPos = MENU_NOT_SELECTED;
    maTimer.Stop();
}

IMPL_LINK_NOARG(ScMenuFloatingWindow::SubMenuItemData, TimeoutHdl, Timer *, void)
{
    mpParent->handleMenuTimeout(this);
}

ScMenuFloatingWindow::ScMenuFloatingWindow(vcl::Window* pParent, ScDocument* pDoc, sal_uInt16 nMenuStackLevel) :
    PopupMenuFloatingWindow(pParent),
    maOpenTimer(this),
    maCloseTimer(this),
    maName("ScMenuFloatingWindow"),
    mpParentMenu(dynamic_cast<ScMenuFloatingWindow*>(pParent))
{
    SetMenuStackLevel(nMenuStackLevel);
    SetText("ScMenuFloatingWindow");

    const StyleSettings& rStyle = GetSettings().GetStyleSettings();

    const sal_uInt16 nPopupFontHeight = 12 * GetDPIScaleFactor();
    maLabelFont = rStyle.GetLabelFont();
    maLabelFont.SetFontHeight(nPopupFontHeight);
}

ScMenuFloatingWindow::~ScMenuFloatingWindow()
{
    disposeOnce();
}

void ScMenuFloatingWindow::dispose()
{
    EndPopupMode();
    for (auto& rMenuItem : maMenuItems)
        rMenuItem.mxSubMenuWin.disposeAndClear();
    mpParentMenu.clear();
    PopupMenuFloatingWindow::dispose();
}

void ScMenuFloatingWindow::PopupModeEnd()
{
    handlePopupEnd();
}

#endif

IMPL_LINK_NOARG(ScCheckListMenuControl, RowActivatedHdl, weld::TreeView&, bool)
{
    executeMenuItem(mxMenu->get_selected_index());
    return true;
}

IMPL_LINK_NOARG(ScCheckListMenuControl, SelectHdl, weld::TreeView&, void)
{
    if (!mxMenu->get_selected(mxScratchIter.get()))
        return;

    sal_uInt32 nPos = mxMenu->get_iter_index_in_parent(*mxScratchIter);
    if (nPos >= maMenuItems.size())
        return;

    if (!maMenuItems[nPos].mxSubMenuWin)
        return;

    tools::Rectangle aArea = mxMenu->get_row_area(*mxScratchIter);

    Point aPos = !maConfig.mbRTL ? aArea.TopRight() : aArea.TopLeft();
    maMenuItems[nPos].mxSubMenuWin->popup_at_rect(mxMenu.get(), tools::Rectangle(aPos, Size(1, 1)));
}

void ScCheckListMenuControl::SubMenuEntry::addMenuItem(const OUString& rText, Action* pAction)
{
    MenuItemData aItem;
    aItem.maText = rText;
    aItem.mbEnabled = true;
    aItem.mxAction.reset(pAction);
    maMenuItems.emplace_back(std::move(aItem));

    mxSubMenuWin->append(OUString::number(maMenuItems.size() - 1), rText);
}

void ScCheckListMenuControl::addMenuItem(const OUString& rText, Action* pAction)
{
    MenuItemData aItem;
    aItem.maText = rText;
    aItem.mbEnabled = true;
    aItem.mxAction.reset(pAction);
    maMenuItems.emplace_back(std::move(aItem));

    mxMenu->append_text(rText);
    mxMenu->set_image(mxMenu->n_children() - 1, css::uno::Reference<css::graphic::XGraphic>(), 1);
}

void ScCheckListMenuControl::addSeparator()
{
    MenuItemData aItem;
    aItem.mbSeparator = true;
    maMenuItems.emplace_back(std::move(aItem));

    mxMenu->append_separator("seperator" + OUString::number(maMenuItems.size()));
}

IMPL_LINK(ScCheckListMenuControl, TreeSizeAllocHdl, const Size&, rSize, void)
{
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

ScCheckListMenuControl::SubMenuEntry::SubMenuEntry(weld::Widget* pParent, ScCheckListMenuControl* pControl)
    : mxBuilder(Application::CreateBuilder(pParent, "modules/scalc/ui/listmenu.ui"))
    , mxSubMenuWin(mxBuilder->weld_menu("listmenu"))
    , mpParent(pControl)
{
    mxSubMenuWin->connect_activate(LINK(this, ScCheckListMenuControl::SubMenuEntry, MenuHdl));
}

ScCheckListMenuControl::SubMenuEntry& ScCheckListMenuControl::addSubMenuItem(const OUString& rText, bool bEnabled)
{
    MenuItemData aItem;
    aItem.maText = rText;
    aItem.mbEnabled = bEnabled;
    aItem.mxSubMenuWin.reset(new SubMenuEntry(mxMenu.get(), this));
    maMenuItems.emplace_back(std::move(aItem));

    mxMenu->append_text(rText);
    mxMenu->set_image(mxMenu->n_children() - 1, *mxDropDown, 1);

    return *maMenuItems.back().mxSubMenuWin;
}

#if 0
Size ScMenuFloatingWindow::getMenuSize() const
{
    if (maMenuItems.empty())
        return Size();

    auto itr = std::max_element(maMenuItems.begin(), maMenuItems.end(),
        [this](const MenuItemData& a, const MenuItemData& b) {
            long aTextWidth = a.mbSeparator ? 0 : GetTextWidth(a.maText);
            long bTextWidth = b.mbSeparator ? 0 : GetTextWidth(b.maText);
            return aTextWidth < bTextWidth;
        });
    long nTextWidth = itr->mbSeparator ? 0 : GetTextWidth(itr->maText);

    size_t nLastPos = maMenuItems.size()-1;
    Point aPos;
    Size aSize;
    getMenuItemPosSize(nLastPos, aPos, aSize);
    aPos.AdjustX(nTextWidth + 15 );
    aPos.AdjustY(aSize.Height() + 5 );
    return Size(aPos.X(), aPos.Y());
}

void ScMenuFloatingWindow::drawMenuItem(vcl::RenderContext& rRenderContext, size_t nPos)
{
    if (nPos >= maMenuItems.size())
        return;

    Point aPos;
    Size aSize;
    getMenuItemPosSize(nPos, aPos, aSize);

    DecorationView aDecoView(&rRenderContext);
    long const nXOffset = 5;
    long nYOffset = (aSize.Height() - maLabelFont.GetFontHeight())/2;

    // Make sure the label font is used for the menu item text.
    rRenderContext.Push(PushFlags::FONT);
    rRenderContext.SetFont(maLabelFont);
    rRenderContext. DrawCtrlText(Point(aPos.X()+nXOffset, aPos.Y() + nYOffset), maMenuItems[nPos].maText, 0,
                                 maMenuItems[nPos].maText.getLength(),
                                 maMenuItems[nPos].mbEnabled ? DrawTextFlags::Mnemonic : DrawTextFlags::Disable);
    rRenderContext.Pop();

    if (maMenuItems[nPos].mxSubMenuWin)
    {
        long nFontHeight = maLabelFont.GetFontHeight();
        Point aMarkerPos = aPos;
        aMarkerPos.AdjustY(aSize.Height() / 2 - nFontHeight / 4 + 1 );
        aMarkerPos.AdjustX(aSize.Width() - nFontHeight + nFontHeight / 4 );
        Size aMarkerSize(nFontHeight / 2, nFontHeight / 2);
        aDecoView.DrawSymbol(tools::Rectangle(aMarkerPos, aMarkerSize), SymbolType::SPIN_RIGHT, GetTextColor());
    }
}

void ScMenuFloatingWindow::drawSeparator(vcl::RenderContext& rRenderContext, size_t nPos)
{
    Point aPos;
    Size aSize;
    getMenuItemPosSize(nPos, aPos, aSize);
    tools::Rectangle aRegion(aPos,aSize);

    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::Entire))
    {
        rRenderContext.Push(PushFlags::CLIPREGION);
        rRenderContext.IntersectClipRegion(aRegion);
        tools::Rectangle aCtrlRect(Point(0,0), GetOutputSizePixel());
        rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::Entire, aCtrlRect,
                                         ControlState::ENABLED, ImplControlValue(), OUString());

        rRenderContext.Pop();
    }

    bool bNativeDrawn = false;
    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::Separator))
    {
        ControlState nState = ControlState::NONE;
        const MenuItemData& rData = maMenuItems[nPos];
        if (rData.mbEnabled)
            nState |= ControlState::ENABLED;

        bNativeDrawn = rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::Separator,
                                                        aRegion, nState, ImplControlValue(), OUString());
    }

    if (!bNativeDrawn)
    {
        const StyleSettings& rStyle = rRenderContext.GetSettings().GetStyleSettings();
        Point aTmpPos = aPos;
        aTmpPos.AdjustY(aSize.Height() / 2 );
        rRenderContext.SetLineColor(rStyle.GetShadowColor());
        rRenderContext.DrawLine(aTmpPos, Point(aSize.Width() + aTmpPos.X(), aTmpPos.Y()));
        aTmpPos.AdjustY( 1 );
        rRenderContext.SetLineColor(rStyle.GetLightColor());
        rRenderContext.DrawLine(aTmpPos, Point(aSize.Width() + aTmpPos.X(), aTmpPos.Y()));
        rRenderContext.SetLineColor();
    }
}

void ScMenuFloatingWindow::drawAllMenuItems(vcl::RenderContext& rRenderContext)
{
    size_t n = maMenuItems.size();

    for (size_t i = 0; i < n; ++i)
    {
        if (maMenuItems[i].mbSeparator)
        {
            // Separator
            drawSeparator(rRenderContext, i);
        }
        else
        {
            // Normal menu item
            highlightMenuItem(rRenderContext, i, i == mnSelectedMenu);
        }
    }
}

#endif

IMPL_LINK(ScCheckListMenuControl::SubMenuEntry, MenuHdl, const OString&, rIdent, void)
{
    sal_uInt32 nPos = rIdent.toUInt32();
    if (nPos >= maMenuItems.size())
        return;

    mpParent->terminateAllPopupMenus();

    maMenuItems[nPos].mxAction->execute();
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

void ScCheckListMenuControl::setSelectedMenuItem(size_t nPos, bool bSubMenuTimer, bool bEnsureSubMenu)
{
    if (mnSelectedMenu == nPos)
        // nothing to do.
        return;

    if (bEnsureSubMenu)
    {
#if 0
        // Dismiss any child popup menu windows.
        if (mnSelectedMenu < maMenuItems.size() &&
            maMenuItems[mnSelectedMenu].mxSubMenuWin &&
            maMenuItems[mnSelectedMenu].mxSubMenuWin->IsVisible())
        {
            maMenuItems[mnSelectedMenu].mxSubMenuWin->ensureSubMenuNotVisible();
        }

        // The popup is not visible, yet a menu item is selected.  The request
        // most likely comes from the accessible object.  Make sure this
        // window, as well as all its parent windows are visible.
        if (!IsVisible() && mpParentMenu)
            mpParentMenu->ensureSubMenuVisible(this);
#endif
    }

    selectMenuItem(nPos, bSubMenuTimer);
}

#if 0

void ScMenuFloatingWindow::handleMenuTimeout(const SubMenuItemData* pTimer)
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

        launchSubMenu(false);
    }
    else if (pTimer == &maCloseTimer)
    {
        // end submenu.
        if (maCloseTimer.mpSubMenu)
        {
            maOpenTimer.mpSubMenu = nullptr;

            maCloseTimer.mpSubMenu->EndPopupMode();
            maCloseTimer.mpSubMenu = nullptr;

            Invalidate();
            maOpenTimer.mnMenuPos = MENU_NOT_SELECTED;
        }
    }
}

void ScMenuFloatingWindow::queueLaunchSubMenu(size_t nPos, ScMenuFloatingWindow* pMenu)
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

void ScMenuFloatingWindow::queueCloseSubMenu()
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

void ScMenuFloatingWindow::launchSubMenu(bool bSetMenuPos)
{
    Point aPos;
    Size aSize;
    getMenuItemPosSize(maOpenTimer.mnMenuPos, aPos, aSize);
    ScMenuFloatingWindow* pSubMenu = maOpenTimer.mpSubMenu;

    if (!pSubMenu)
        return;

    FloatWinPopupFlags nOldFlags = GetPopupModeFlags();
    SetPopupModeFlags(nOldFlags | FloatWinPopupFlags::NoAppFocusClose);
    pSubMenu->resizeToFitMenuItems(); // set the size before launching the popup to get it positioned correctly.
    pSubMenu->StartPopupMode(
        tools::Rectangle(aPos,aSize), (FloatWinPopupFlags::Right | FloatWinPopupFlags::GrabFocus));
    pSubMenu->AddPopupModeWindow(this);
    if (bSetMenuPos)
        pSubMenu->setSelectedMenuItem(0, false, false); // select menu item after the popup becomes fully visible.
    SetPopupModeFlags(nOldFlags);
}

void ScMenuFloatingWindow::endSubMenu(ScMenuFloatingWindow* pSubMenu)
{
    if (!pSubMenu)
        return;

    pSubMenu->EndPopupMode();
    maOpenTimer.reset();

    size_t nMenuPos = getSubMenuPos(pSubMenu);
    if (nMenuPos != MENU_NOT_SELECTED)
    {
        mnSelectedMenu = nMenuPos;
        Invalidate();
        fireMenuHighlightedEvent();
    }
}

void ScMenuFloatingWindow::resizeToFitMenuItems()
{
    SetOutputSizePixel(getMenuSize());
}

#endif

void ScCheckListMenuControl::selectMenuItem(size_t nPos, bool /*bSubMenuTimer*/)
{
    mxMenu->select(nPos == MENU_NOT_SELECTED ? -1 : nPos);
    mnSelectedMenu = nPos;
#if 0
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

    if (nPos != MENU_NOT_SELECTED)
    {
        if (mpParentMenu)
            mpParentMenu->setSubMenuFocused(this);

        if (bSubMenuTimer)
        {
            if (maMenuItems[nPos].mxSubMenuWin)
            {
                ScMenuFloatingWindow* pSubMenu = maMenuItems[nPos].mxSubMenuWin.get();
                queueLaunchSubMenu(nPos, pSubMenu);
            }
            else
                queueCloseSubMenu();
        }
    }
#endif
}

void ScCheckListMenuControl::clearSelectedMenuItem()
{
    selectMenuItem(MENU_NOT_SELECTED, false);
}

#if 0
ScMenuFloatingWindow* ScMenuFloatingWindow::getSubMenuWindow(size_t nPos) const
{
    if (maMenuItems.size() <= nPos)
        return nullptr;

    return maMenuItems[nPos].mxSubMenuWin.get();
}

bool ScMenuFloatingWindow::isMenuItemSelected(size_t nPos) const
{
    return nPos == mnSelectedMenu;
}

void ScMenuFloatingWindow::setName(const OUString& rName)
{
    maName = rName;
}

void ScMenuFloatingWindow::highlightMenuItem(vcl::RenderContext& rRenderContext, size_t nPos, bool bSelected)
{
    if (nPos == MENU_NOT_SELECTED)
        return;

    const StyleSettings& rStyle = rRenderContext.GetSettings().GetStyleSettings();
    Color aBackColor = rStyle.GetMenuColor();
    rRenderContext.SetFillColor(aBackColor);
    rRenderContext.SetLineColor(aBackColor);

    Point aPos;
    Size aSize;
    getMenuItemPosSize(nPos, aPos, aSize);
    tools::Rectangle aRegion(aPos,aSize);

    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::Entire))
    {
        rRenderContext.Push(PushFlags::CLIPREGION);
        rRenderContext.IntersectClipRegion(tools::Rectangle(aPos, aSize));
        tools::Rectangle aCtrlRect(Point(0,0), GetOutputSizePixel());
        rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::Entire, aCtrlRect, ControlState::ENABLED,
                                         ImplControlValue(), OUString());
        rRenderContext.Pop();
    }

    bool bNativeDrawn = true;
    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::MenuItem))
    {
        ControlState nState = bSelected ? ControlState::SELECTED : ControlState::NONE;
        if (maMenuItems[nPos].mbEnabled)
            nState |= ControlState::ENABLED;
        bNativeDrawn = rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::MenuItem,
                                                        aRegion, nState, ImplControlValue(), OUString());
    }
    else
        bNativeDrawn = false;

    if (!bNativeDrawn)
    {
        if (bSelected)
        {
            aBackColor = rStyle.GetMenuHighlightColor();
            rRenderContext.SetFillColor(aBackColor);
            rRenderContext.SetLineColor(aBackColor);
        }
        rRenderContext.DrawRect(tools::Rectangle(aPos,aSize));
    }

    Color aTextColor = bSelected ? rStyle.GetMenuHighlightTextColor() : rStyle.GetMenuTextColor();
    rRenderContext.SetTextColor(aTextColor);
    drawMenuItem(rRenderContext, nPos);
}

void ScMenuFloatingWindow::getMenuItemPosSize(size_t nPos, Point& rPos, Size& rSize) const
{
    size_t nCount = maMenuItems.size();
    if (nPos >= nCount)
        return;

    const sal_uInt16 nLeftMargin = 5;
    const sal_uInt16 nTopMargin = 5;
    const sal_uInt16 nMenuItemHeight = static_cast<sal_uInt16>(maLabelFont.GetFontHeight()*1.8);
    const sal_uInt16 nSepHeight = static_cast<sal_uInt16>(maLabelFont.GetFontHeight()*0.8);

    Point aPos1(nLeftMargin, nTopMargin);
    rPos = aPos1;
    for (size_t i = 0; i < nPos; ++i)
        rPos.AdjustY(maMenuItems[i].mbSeparator ? nSepHeight : nMenuItemHeight );

    Size aWndSize = GetSizePixel();
    sal_uInt16 nH = maMenuItems[nPos].mbSeparator ? nSepHeight : nMenuItemHeight;
    rSize = Size(aWndSize.Width() - nLeftMargin*2, nH);
}

size_t ScMenuFloatingWindow::getEnclosingMenuItem(const Point& rPos) const
{
    size_t n = maMenuItems.size();
    for (size_t i = 0; i < n; ++i)
    {
        Point aPos;
        Size aSize;
        getMenuItemPosSize(i, aPos, aSize);
        tools::Rectangle aRect(aPos, aSize);
        if (aRect.IsInside(rPos))
            return maMenuItems[i].mbSeparator ? MENU_NOT_SELECTED : i;
    }
    return MENU_NOT_SELECTED;
}

size_t ScMenuFloatingWindow::getSubMenuPos(const ScMenuFloatingWindow* pSubMenu)
{
    size_t n = maMenuItems.size();
    for (size_t i = 0; i < n; ++i)
    {
        if (maMenuItems[i].mxSubMenuWin.get() == pSubMenu)
            return i;
    }
    return MENU_NOT_SELECTED;
}

void ScMenuFloatingWindow::setSubMenuFocused(const ScMenuFloatingWindow* pSubMenu)
{
    maCloseTimer.reset();
    size_t nMenuPos = getSubMenuPos(pSubMenu);
    if (mnSelectedMenu != nMenuPos)
    {
        mnSelectedMenu = nMenuPos;
        Invalidate();
    }
}

void ScMenuFloatingWindow::ensureSubMenuVisible(ScMenuFloatingWindow* pSubMenu)
{
    if (mpParentMenu)
        mpParentMenu->ensureSubMenuVisible(this);

    if (pSubMenu->IsVisible())
        return;

    // Find the menu position of the submenu.
    size_t nMenuPos = getSubMenuPos(pSubMenu);
    if (nMenuPos != MENU_NOT_SELECTED)
    {
        setSelectedMenuItem(nMenuPos, false, false);

        Point aPos;
        Size aSize;
        getMenuItemPosSize(nMenuPos, aPos, aSize);

        FloatWinPopupFlags nOldFlags = GetPopupModeFlags();
        SetPopupModeFlags(nOldFlags | FloatWinPopupFlags::NoAppFocusClose);
        pSubMenu->resizeToFitMenuItems(); // set the size before launching the popup to get it positioned correctly.
        pSubMenu->StartPopupMode(
            tools::Rectangle(aPos,aSize), (FloatWinPopupFlags::Right | FloatWinPopupFlags::GrabFocus));
        pSubMenu->AddPopupModeWindow(this);
        SetPopupModeFlags(nOldFlags);
    }
}

void ScMenuFloatingWindow::ensureSubMenuNotVisible()
{
    if (mnSelectedMenu < maMenuItems.size() &&
        maMenuItems[mnSelectedMenu].mxSubMenuWin &&
        maMenuItems[mnSelectedMenu].mxSubMenuWin->IsVisible())
    {
        maMenuItems[mnSelectedMenu].mxSubMenuWin->ensureSubMenuNotVisible();
    }

    EndPopupMode();
}
#endif

void ScCheckListMenuControl::terminateAllPopupMenus()
{
    vcl::Window::GetDockingManager()->EndPopupMode(mxParent);
#if 0
    if (mpParentMenu)
        mpParentMenu->terminateAllPopupMenus();
#endif
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

ScCheckListMenuControl::ScCheckListMenuControl(DockingWindow* pParent, vcl::Window* pContainer, ScDocument* pDoc, int nWidth)
    : mxParent(pParent)
    , mxBuilder(Application::CreateInterimBuilder(pContainer, "modules/scalc/ui/filterdropdown.ui"))
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
    , mnWidthHint(nWidth)
    , maWndSize()
    , mePrevToggleAllState(TRISTATE_INDET)
    , mnSelectedMenu(MENU_NOT_SELECTED)
    , mpDoc(pDoc)
{
    // sort ok/cancel into native order, if this was a dialog they would be auto-sorted, but this
    // popup isn't a true dialog
    mxButtonBox->sort_native_button_order();

    std::vector<int> aWidths;
    aWidths.push_back(mxChecks->get_checkbox_column_width());
    mxChecks->set_column_fixed_widths(aWidths);

    CreateDropDown();

    mxContainer->connect_focus_in(LINK(this, ScCheckListMenuControl, FocusHdl));
    mxMenu->connect_row_activated(LINK(this, ScCheckListMenuControl, RowActivatedHdl));
    mxMenu->connect_changed(LINK(this, ScCheckListMenuControl, SelectHdl));
    mxMenu->connect_size_allocate(LINK(this, ScCheckListMenuControl, TreeSizeAllocHdl));
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
        mxMenu->grab_focus();
}

ScCheckListMenuControl::~ScCheckListMenuControl()
{
}

ScCheckListMenuWindow::ScCheckListMenuWindow(vcl::Window* pParent, ScDocument* pDoc, int nWidth)
    : DockingWindow(pParent, "InterimDockParent", "svx/ui/interimdockparent.ui")
    , mxBox(get("box"))
    , mxControl(new ScCheckListMenuControl(this, mxBox.get(), pDoc, nWidth))
{
    SetBackground(Application::GetSettings().GetStyleSettings().GetMenuColor());
    set_id("check_list_menu");
}

ScCheckListMenuWindow::~ScCheckListMenuWindow()
{
    disposeOnce();
}

void ScCheckListMenuWindow::dispose()
{
    mxControl.reset();
    mxBox.disposeAndClear();
    DockingWindow::dispose();
}

void ScCheckListMenuWindow::GetFocus()
{
    DockingWindow::GetFocus();
    if (!mxControl)
        return;
    mxControl->GrabFocus();
}

void ScCheckListMenuControl::packWindow()
{
    mxBox->show();
    mxEdSearch->show();
    mxButtonBox->show();

    mxBtnOk->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));
    mxBtnCancel->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));
    mxEdSearch->connect_changed(LINK(this, ScCheckListMenuControl, EdModifyHdl));
    mxEdSearch->connect_activate(LINK(this, ScCheckListMenuControl, EdActivateHdl));
    mxChecks->connect_toggled(LINK(this, ScCheckListMenuControl, CheckHdl));
    mxChecks->connect_key_press(LINK(this, ScCheckListMenuControl, KeyInputHdl));
    mxChkToggleAll->connect_toggled(LINK(this, ScCheckListMenuControl, TriStateHdl));
    mxBtnSelectSingle->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));
    mxBtnUnselectSingle->connect_clicked(LINK(this, ScCheckListMenuControl, ButtonHdl));

    mxChecks->set_size_request(-1, mxChecks->get_height_rows(9));
    mxMenu->set_size_request(-1, mxMenu->get_preferred_size().Height() + 2);

    maWndSize = mxContainer->get_preferred_size();
    if (maWndSize.Width() < mnWidthHint)
    {
        mxContainer->set_size_request(mnWidthHint, -1);
        maWndSize.setWidth(mnWidthHint);
    }
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
    mxChecks->set_toggle(*xEntry, bSet ? TRISTATE_TRUE : TRISTATE_FALSE, 0);
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

    if (bSearchTextEmpty)
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

            if ( !bIsDate )
                bPartialMatch = ( ScGlobal::getCharClassPtr()->lowercase( aLabelDisp ).indexOf( aSearchText ) != -1 );
            else if ( maMembers[i].meDatePartType == ScCheckListMember::DAY ) // Match with both numerical and text version of month
                bPartialMatch = (ScGlobal::getCharClassPtr()->lowercase( OUString(
                                maMembers[i].maRealName + maMembers[i].maDateParts[1] )).indexOf( aSearchText ) != -1);
            else
                continue;

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
        CheckEntry(pEntry,  mxChecks->get_toggle(*pEntry, 0) == TRISTATE_TRUE);
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

#if 0
bool ScCheckListMenuControl::EventNotify(NotifyEvent& rNEvt)
{
    MouseNotifyEvent nType = rNEvt.GetType();
    if (HasFocus() && nType == MouseNotifyEvent::GETFOCUS)
    {
        setSelectedMenuItem( 0 , false, false );
        return true;
    }
    return InterimItemWindow::EventNotify(rNEvt);
}
#endif

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
        mxChecks->insert(nullptr, -1, nullptr, nullptr, nullptr, nullptr, nullptr, false, xYearEntry.get());
        mxChecks->set_toggle(*xYearEntry, TRISTATE_FALSE, 0);
        mxChecks->set_text(*xYearEntry, aYearName, 1);
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
        mxChecks->insert(xYearEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, nullptr, false, xMonthEntry.get());
        mxChecks->set_toggle(*xMonthEntry, TRISTATE_FALSE, 0);
        mxChecks->set_text(*xMonthEntry, aMonthName, 1);
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
        mxChecks->insert(xMonthEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, nullptr, false, xDayEntry.get());
        mxChecks->set_toggle(*xDayEntry, TRISTATE_FALSE, 0);
        mxChecks->set_text(*xDayEntry, aDayName, 1);
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
        if (sNode == mxChecks->get_text(*xEntry, 1))
            return xEntry;
        bEntry = mxChecks->iter_next_sibling(*xEntry);
    }
    return nullptr;
}

void ScCheckListMenuControl::GetRecursiveChecked(const weld::TreeIter* pEntry, std::unordered_set<OUString>& vOut,
                                                 OUString& rLabel)
{
    if (mxChecks->get_toggle(*pEntry, 0) == TRISTATE_TRUE)
    {
        // We have to hash parents and children together.
        // Per convention for easy access in getResult()
        // "child;parent;grandparent" while descending.
        if (rLabel.isEmpty())
            rLabel = mxChecks->get_text(*pEntry, 1);
        else
            rLabel = mxChecks->get_text(*pEntry, 1) + ";" + rLabel;

        // Prerequisite: the selection mechanism guarantees that if a child is
        // selected then also the parent is selected, so we only have to
        // inspect the children in case the parent is selected.
        if (mxChecks->iter_has_child(*pEntry))
        {
            std::unique_ptr<weld::TreeIter> xChild(mxChecks->make_iterator());
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
    }
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
    return xEntry && mxChecks->get_toggle(*xEntry, 0) == TRISTATE_TRUE;
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
        mxChecks->set_toggle(*pParent, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE, 0);
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
    if (pParent && mxChecks->get_iter_depth(*pParent))
    {
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
                if (mxChecks->get_toggle(*xChild, 0) == TRISTATE_TRUE)
                {
                    bChildChecked = true;
                    break;
                }
                bChild = mxChecks->iter_next_sibling(*xChild);
            }
            mxChecks->set_toggle(*xAncestor, bChildChecked ? TRISTATE_TRUE : TRISTATE_FALSE, 0);
            bAncestor = mxChecks->iter_parent(*xAncestor);
        }
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
                    mxChecks->insert(nullptr, -1, nullptr, nullptr, nullptr, nullptr, nullptr, false, xYearEntry.get());
                    mxChecks->set_toggle(*xYearEntry, TRISTATE_FALSE, 0);
                    mxChecks->set_text(*xYearEntry, rMember.maDateParts[0], 1);
                }
                std::unique_ptr<weld::TreeIter> xMonthEntry = FindEntry(xYearEntry.get(), rMember.maDateParts[1]);
                if (!xMonthEntry)
                {
                    mxChecks->insert(xYearEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, nullptr, false, xMonthEntry.get());
                    mxChecks->set_toggle(*xMonthEntry, TRISTATE_FALSE, 0);
                    mxChecks->set_text(*xMonthEntry, rMember.maDateParts[1], 1);
                }
                std::unique_ptr<weld::TreeIter> xDayEntry = FindEntry(xMonthEntry.get(), rMember.maName);
                if (!xDayEntry)
                {
                    mxChecks->insert(xMonthEntry.get(), -1, nullptr, nullptr, nullptr, nullptr, nullptr, false, xDayEntry.get());
                    mxChecks->set_toggle(*xDayEntry, TRISTATE_FALSE, 0);
                    mxChecks->set_text(*xDayEntry, rMember.maName, 1);
                }
                return xDayEntry; // Return leaf node
            }

            xEntry = mxChecks->make_iterator();
            mxChecks->append(xEntry.get());
            mxChecks->set_toggle(*xEntry, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE, 0);
            mxChecks->set_text(*xEntry, sName, 1);
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
        if (mxChecks->get_toggle(rEntry, 0) == TRISTATE_TRUE)
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
            bool bOldCheck = mxChecks->get_toggle(*xEntry, 0) == TRISTATE_TRUE;
            CheckEntry(xEntry.get(), !bOldCheck);
            bool bNewCheck = mxChecks->get_toggle(*xEntry, 0) == TRISTATE_TRUE;
            if (bOldCheck != bNewCheck)
                Check(xEntry.get());
        }
        return true;
    }

    return false;
}

#if 0
void ScCheckListMenuControl::setHasDates(bool bHasDates)
{
    // Enables type-ahead search in the check list box.
    mxChecks->SetQuickSearch(true);
    if (bHasDates)
        mxChecks->SetStyle(WB_HASBUTTONS | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONSATROOT);
    else
        mxChecks->SetStyle(WB_HASBUTTONS);
}
#endif

size_t ScCheckListMenuControl::initMembers()
{
    size_t n = maMembers.size();
    size_t nVisMemCount = 0;

    mxChecks->freeze();

    std::unique_ptr<weld::TreeIter> xEntry = mxChecks->make_iterator();

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
                    mxChecks->expand_row(*xDateEntry);
            }
        }
        else
        {
            OUString aLabel = maMembers[i].maName;
            if (aLabel.isEmpty())
                aLabel = ScResId(STR_EMPTYDATA);

            mxChecks->append(xEntry.get());
            mxChecks->set_toggle(*xEntry, maMembers[i].mbVisible ? TRISTATE_TRUE : TRISTATE_FALSE, 0);
            mxChecks->set_text(*xEntry, aLabel, 1);
        }

        if (maMembers[i].mbVisible)
            ++nVisMemCount;
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

    mxChecks->thaw();
    return nVisMemCount;
}

void ScCheckListMenuControl::setConfig(const Config& rConfig)
{
    maConfig = rConfig;
}

bool ScCheckListMenuControl::isAllSelected() const
{
    return mxChkToggleAll->get_active();
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

#if 0 //TODO
            /* TODO: performance-wise this looks suspicious, concatenating to
             * do the lookup for each leaf item seems wasteful. */
            // Checked labels are in the form "child;parent;grandparent".
            for (SvTreeListEntry* pParent = maMembers[i].mpParent;
                    pParent && pParent->GetFirstItem( SvLBoxItemType::String);
                    pParent = pParent->GetParent())
            {
                aLabel.append(";").append(mxChecks->GetEntryText( pParent));
            }
#endif
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
    packWindow();
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
    else if (maWndSize.Width() < aRect.GetWidth())
    {
        // Target rectangle (i.e. cell width) is wider than the window.
        // Simulate right-aligned launch by modifying the target rectangle
        // size.
        long nDiff = aRect.GetWidth() - maWndSize.Width();
        aRect.AdjustLeft(nDiff );
    }

    mxParent->EnableDocking();
    vcl::Window::GetDockingManager()->StartPopupMode(mxParent, aRect, (FloatWinPopupFlags::Down | FloatWinPopupFlags::GrabFocus));
}

void ScCheckListMenuControl::close(bool bOK)
{
    if (bOK && mxOKAction)
        mxOKAction->execute();

    vcl::Window::GetDockingManager()->EndPopupMode(mxParent);
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

void ScCheckListMenuControl::handlePopupEnd()
{
    clearSelectedMenuItem();
    if (mxPopupEndAction)
        mxPopupEndAction->execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
