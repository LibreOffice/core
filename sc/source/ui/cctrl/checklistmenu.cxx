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

#include "checklistmenu.hxx"
#include "checklistmenu.hrc"
#include "strload.hxx"

#include <vcl/decoview.hxx>
#include <vcl/settings.hxx>
#include <tools/wintypes.hxx>

#include "AccessibleFilterMenu.hxx"
#include "AccessibleFilterTopWindow.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <svtools/fmtfield.hxx>
#include "document.hxx"

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::accessibility::XAccessibleContext;
using ::std::vector;
using ::boost::unordered_map;

ScMenuFloatingWindow::MenuItemData::MenuItemData() :
    mbEnabled(true), mbSeparator(false),
    mpAction(static_cast<ScCheckListMenuWindow::Action*>(NULL)),
    mpSubMenuWin(static_cast<ScMenuFloatingWindow*>(NULL))
{
}

ScMenuFloatingWindow::SubMenuItemData::SubMenuItemData(ScMenuFloatingWindow* pParent) :
    mpSubMenu(NULL),
    mnMenuPos(MENU_NOT_SELECTED),
    mpParent(pParent)
{
    maTimer.SetTimeoutHdl( LINK(this, ScMenuFloatingWindow::SubMenuItemData, TimeoutHdl) );
    maTimer.SetTimeout(mpParent->GetSettings().GetMouseSettings().GetMenuDelay());
}

void ScMenuFloatingWindow::SubMenuItemData::reset()
{
    mpSubMenu = NULL;
    mnMenuPos = MENU_NOT_SELECTED;
    maTimer.Stop();
}

IMPL_LINK_NOARG(ScMenuFloatingWindow::SubMenuItemData, TimeoutHdl)
{
    mpParent->handleMenuTimeout(this);
    return 0;
}

size_t ScMenuFloatingWindow::MENU_NOT_SELECTED = 999;

ScMenuFloatingWindow::ScMenuFloatingWindow(vcl::Window* pParent, ScDocument* pDoc, sal_uInt16 nMenuStackLevel) :
    PopupMenuFloatingWindow(pParent),
    maOpenTimer(this),
    maCloseTimer(this),
    maName("ScMenuFloatingWindow"),
    mnSelectedMenu(MENU_NOT_SELECTED),
    mnClickedMenu(MENU_NOT_SELECTED),
    mpDoc(pDoc),
    mpParentMenu(dynamic_cast<ScMenuFloatingWindow*>(pParent))
{
    SetMenuStackLevel(nMenuStackLevel);

    // TODO: How do we get the right font to use here ?
    const sal_uInt16 nPopupFontHeight = 12;
    const StyleSettings& rStyle = GetSettings().GetStyleSettings();
    maLabelFont = rStyle.GetLabelFont();
    maLabelFont.SetHeight(nPopupFontHeight);
    SetFont(maLabelFont);

    SetText( OUString("ScMenuFloatingWindow") );
}

ScMenuFloatingWindow::~ScMenuFloatingWindow()
{
    EndPopupMode();
}

void ScMenuFloatingWindow::PopupModeEnd()
{
    handlePopupEnd();
}

void ScMenuFloatingWindow::MouseMove(const MouseEvent& rMEvt)
{
    const Point& rPos = rMEvt.GetPosPixel();
    size_t nSelectedMenu = getEnclosingMenuItem(rPos);
    setSelectedMenuItem(nSelectedMenu, true, false);

    Window::MouseMove(rMEvt);
}

void ScMenuFloatingWindow::MouseButtonDown(const MouseEvent& rMEvt)
{
    const Point& rPos = rMEvt.GetPosPixel();
    mnClickedMenu = getEnclosingMenuItem(rPos);
    Window::MouseButtonDown(rMEvt);
}

void ScMenuFloatingWindow::MouseButtonUp(const MouseEvent& rMEvt)
{
    executeMenuItem(mnClickedMenu);
    mnClickedMenu = MENU_NOT_SELECTED;
    Window::MouseButtonUp(rMEvt);
}

void ScMenuFloatingWindow::KeyInput(const KeyEvent& rKEvt)
{
    if (maMenuItems.empty())
    {
        Window::KeyInput(rKEvt);
        return;
    }

    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();
    bool bHandled = true;
    size_t nSelectedMenu = mnSelectedMenu;
    size_t nLastMenuPos = maMenuItems.size() - 1;
    switch (rKeyCode.GetCode())
    {
        case KEY_UP:
        {
            if (nLastMenuPos == 0)
                // There is only one menu item.  Do nothing.
                break;

            size_t nOldPos = nSelectedMenu;

            if (nSelectedMenu == MENU_NOT_SELECTED || nSelectedMenu == 0)
                nSelectedMenu = nLastMenuPos;
            else
                --nSelectedMenu;

            // Loop until a non-separator menu item is found.
            while (nSelectedMenu != nOldPos)
            {
                if (maMenuItems[nSelectedMenu].mbSeparator)
                {
                    if (nSelectedMenu)
                        --nSelectedMenu;
                    else
                        nSelectedMenu = nLastMenuPos;
                }
                else
                    break;
            }

            setSelectedMenuItem(nSelectedMenu, false, false);
        }
        break;
        case KEY_DOWN:
        {
            if (nLastMenuPos == 0)
                // There is only one menu item.  Do nothing.
                break;

            size_t nOldPos = nSelectedMenu;

            if (nSelectedMenu == MENU_NOT_SELECTED || nSelectedMenu == nLastMenuPos)
                nSelectedMenu = 0;
            else
                ++nSelectedMenu;

            // Loop until a non-separator menu item is found.
            while (nSelectedMenu != nOldPos)
            {
                if (maMenuItems[nSelectedMenu].mbSeparator)
                {
                    if (nSelectedMenu == nLastMenuPos)
                        nSelectedMenu = 0;
                    else
                        ++nSelectedMenu;
                }
                else
                    break;
            }

            setSelectedMenuItem(nSelectedMenu, false, false);
        }
        break;
        case KEY_LEFT:
            if (mpParentMenu)
                mpParentMenu->endSubMenu(this);
        break;
        case KEY_RIGHT:
        {
            if (mnSelectedMenu >= maMenuItems.size() || mnSelectedMenu == MENU_NOT_SELECTED)
                break;

            const MenuItemData& rMenu = maMenuItems[mnSelectedMenu];
            if (!rMenu.mbEnabled || !rMenu.mpSubMenuWin)
                break;

            maOpenTimer.mnMenuPos = mnSelectedMenu;
            maOpenTimer.mpSubMenu = rMenu.mpSubMenuWin.get();
            launchSubMenu(true);
        }
        break;
        case KEY_RETURN:
            if (nSelectedMenu != MENU_NOT_SELECTED)
                executeMenuItem(nSelectedMenu);
        break;
        default:
            bHandled = false;
    }

    if (!bHandled)
        Window::KeyInput(rKEvt);
}

void ScMenuFloatingWindow::Paint(const Rectangle& /*rRect*/)
{
    const StyleSettings& rStyle = GetSettings().GetStyleSettings();
    Color aBackColor = rStyle.GetMenuColor();
    Color aBorderColor = rStyle.GetShadowColor();

    Rectangle aCtrlRect(Point(0, 0), GetOutputSizePixel());

    // Window background
    bool bNativeDrawn = true;
    if (IsNativeControlSupported(CTRL_MENU_POPUP, PART_ENTIRE_CONTROL))
    {
        SetClipRegion();
        bNativeDrawn = DrawNativeControl(
            CTRL_MENU_POPUP, PART_ENTIRE_CONTROL, aCtrlRect, CTRL_STATE_ENABLED,
            ImplControlValue(), OUString());
    }
    else
        bNativeDrawn = false;

    if (!bNativeDrawn)
    {
        SetFillColor(aBackColor);
        SetLineColor(aBorderColor);
        DrawRect(aCtrlRect);
    }

    // Menu items
    SetTextColor(rStyle.GetMenuTextColor());
    drawAllMenuItems();
}

Reference<XAccessible> ScMenuFloatingWindow::CreateAccessible()
{
    if (!mxAccessible.is())
    {
        Reference<XAccessible> xAccParent = mpParentMenu ?
            mpParentMenu->GetAccessible() : GetAccessibleParentWindow()->GetAccessible();

        mxAccessible.set(new ScAccessibleFilterMenu(xAccParent, this, maName, 999));
        ScAccessibleFilterMenu* p = static_cast<ScAccessibleFilterMenu*>(
            mxAccessible.get());

        vector<MenuItemData>::const_iterator itr, itrBeg = maMenuItems.begin(), itrEnd = maMenuItems.end();
        for (itr = itrBeg; itr != itrEnd; ++itr)
        {
            size_t nPos = ::std::distance(itrBeg, itr);
            p->appendMenuItem(itr->maText, itr->mbEnabled, nPos);
        }
    }

    return mxAccessible;
}

void ScMenuFloatingWindow::addMenuItem(const OUString& rText, bool bEnabled, Action* pAction)
{
    MenuItemData aItem;
    aItem.maText = rText;
    aItem.mbEnabled = bEnabled;
    aItem.mpAction.reset(pAction);
    maMenuItems.push_back(aItem);
}

void ScMenuFloatingWindow::addSeparator()
{
    MenuItemData aItem;
    aItem.mbSeparator = true;
    maMenuItems.push_back(aItem);
}

ScMenuFloatingWindow* ScMenuFloatingWindow::addSubMenuItem(const OUString& rText, bool bEnabled)
{
    MenuItemData aItem;
    aItem.maText = rText;
    aItem.mbEnabled = bEnabled;
    aItem.mpSubMenuWin.reset(new ScMenuFloatingWindow(this, mpDoc, GetMenuStackLevel()+1));
    aItem.mpSubMenuWin->setName(rText);
    maMenuItems.push_back(aItem);
    return aItem.mpSubMenuWin.get();
}

void ScMenuFloatingWindow::handlePopupEnd()
{
    clearSelectedMenuItem();
}

Size ScMenuFloatingWindow::getMenuSize() const
{
    if (maMenuItems.empty())
        return Size();

    vector<MenuItemData>::const_iterator itr = maMenuItems.begin(), itrEnd = maMenuItems.end();
    long nTextWidth = 0;
    for (; itr != itrEnd; ++itr)
    {
        if (itr->mbSeparator)
            continue;

        nTextWidth = ::std::max(GetTextWidth(itr->maText), nTextWidth);
    }

    size_t nLastPos = maMenuItems.size()-1;
    Point aPos;
    Size aSize;
    getMenuItemPosSize(nLastPos, aPos, aSize);
    aPos.X() += nTextWidth + 15;
    aPos.Y() += aSize.Height() + 5;
    return Size(aPos.X(), aPos.Y());
}

void ScMenuFloatingWindow::drawMenuItem(size_t nPos)
{
    if (nPos >= maMenuItems.size())
        return;

    Point aPos;
    Size aSize;
    getMenuItemPosSize(nPos, aPos, aSize);

    DecorationView aDecoView(this);
    long nXOffset = 5;
    long nYOffset = (aSize.Height() - maLabelFont.GetHeight())/2;
    DrawCtrlText(Point(aPos.X()+nXOffset, aPos.Y() + nYOffset), maMenuItems[nPos].maText, 0,
                 maMenuItems[nPos].maText.getLength(),
                 maMenuItems[nPos].mbEnabled ? TEXT_DRAW_MNEMONIC : TEXT_DRAW_DISABLE);

    if (maMenuItems[nPos].mpSubMenuWin)
    {
        long nFontHeight = maLabelFont.GetHeight();
        Point aMarkerPos = aPos;
        aMarkerPos.Y() += aSize.Height()/2 - nFontHeight/4 + 1;
        aMarkerPos.X() += aSize.Width() - nFontHeight + nFontHeight/4;
        Size aMarkerSize(nFontHeight/2, nFontHeight/2);
        aDecoView.DrawSymbol(Rectangle(aMarkerPos, aMarkerSize),
                             SymbolType::SPIN_RIGHT, GetTextColor(), 0);
    }
}

void ScMenuFloatingWindow::drawSeparator(size_t nPos)
{
    Point aPos;
    Size aSize;
    getMenuItemPosSize(nPos, aPos, aSize);
    Rectangle aRegion(aPos,aSize);

    if (IsNativeControlSupported(CTRL_MENU_POPUP, PART_ENTIRE_CONTROL))
    {
        Push(PushFlags::CLIPREGION);
        IntersectClipRegion(aRegion);
        Rectangle aCtrlRect(Point(0,0), GetOutputSizePixel());
        DrawNativeControl(
            CTRL_MENU_POPUP, PART_ENTIRE_CONTROL, aCtrlRect, CTRL_STATE_ENABLED,
            ImplControlValue(), OUString());

        Pop();
    }

    bool bNativeDrawn = false;
    if (IsNativeControlSupported(CTRL_MENU_POPUP, PART_MENU_SEPARATOR))
    {
        ControlState nState = 0;
        const MenuItemData& rData = maMenuItems[nPos];
        if (rData.mbEnabled)
            nState |= CTRL_STATE_ENABLED;

        bNativeDrawn = DrawNativeControl(
            CTRL_MENU_POPUP, PART_MENU_SEPARATOR,
            aRegion, nState, ImplControlValue(), OUString());
    }

    if (!bNativeDrawn)
    {
        const StyleSettings& rStyle = GetSettings().GetStyleSettings();
        Point aTmpPos = aPos;
        aTmpPos.Y() += aSize.Height()/2;
        SetLineColor(rStyle.GetShadowColor());
        DrawLine(aTmpPos, Point(aSize.Width()+aTmpPos.X(), aTmpPos.Y()));
        ++aTmpPos.Y();
        SetLineColor(rStyle.GetLightColor());
        DrawLine(aTmpPos, Point(aSize.Width()+aTmpPos.X(), aTmpPos.Y()));
        SetLineColor();
    }
}

void ScMenuFloatingWindow::drawAllMenuItems()
{
    size_t n = maMenuItems.size();
    for (size_t i = 0; i < n; ++i)
    {
        if (maMenuItems[i].mbSeparator)
            // Separator
            drawSeparator(i);
        else
            // Normal menu item
            highlightMenuItem(i, i == mnSelectedMenu);
    }
}

void ScMenuFloatingWindow::executeMenuItem(size_t nPos)
{
    if (nPos >= maMenuItems.size())
        return;

    if (!maMenuItems[nPos].mpAction)
        // no action is defined.
        return;

    maMenuItems[nPos].mpAction->execute();
    terminateAllPopupMenus();
}

void ScMenuFloatingWindow::setSelectedMenuItem(size_t nPos, bool bSubMenuTimer, bool bEnsureSubMenu)
{
    if (mnSelectedMenu == nPos)
        // nothing to do.
        return;

    if (bEnsureSubMenu)
    {
        // Dismiss any child popup menu windows.
        if (mnSelectedMenu < maMenuItems.size() &&
            maMenuItems[mnSelectedMenu].mpSubMenuWin &&
            maMenuItems[mnSelectedMenu].mpSubMenuWin->IsVisible())
        {
            maMenuItems[mnSelectedMenu].mpSubMenuWin->ensureSubMenuNotVisible();
        }

        // The popup is not visible, yet a menu item is selected.  The request
        // most likely comes from the accessible object.  Make sure this
        // window, as well as all its parent windows are visible.
        if (!IsVisible() && mpParentMenu)
            mpParentMenu->ensureSubMenuVisible(this);
    }

    selectMenuItem(mnSelectedMenu, false, bSubMenuTimer);
    selectMenuItem(nPos, true, bSubMenuTimer);
    mnSelectedMenu = nPos;

    fireMenuHighlightedEvent();
}

void ScMenuFloatingWindow::handleMenuTimeout(SubMenuItemData* pTimer)
{
    if (pTimer == &maOpenTimer)
    {
        // Close any open submenu immediately.
        if (maCloseTimer.mpSubMenu)
        {
            maCloseTimer.mpSubMenu->EndPopupMode();
            maCloseTimer.mpSubMenu = NULL;
            maCloseTimer.maTimer.Stop();
        }

        launchSubMenu(false);
    }
    else if (pTimer == &maCloseTimer)
    {
        // end submenu.
        if (maCloseTimer.mpSubMenu)
        {
            maOpenTimer.mpSubMenu = NULL;

            maCloseTimer.mpSubMenu->EndPopupMode();
            maCloseTimer.mpSubMenu = NULL;

            highlightMenuItem(maOpenTimer.mnMenuPos, false);
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

    sal_uInt32 nOldFlags = GetPopupModeFlags();
    SetPopupModeFlags(nOldFlags | FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE);
    pSubMenu->resizeToFitMenuItems(); // set the size before launching the popup to get it positioned correctly.
    pSubMenu->StartPopupMode(
        Rectangle(aPos,aSize), (FLOATWIN_POPUPMODE_RIGHT | FLOATWIN_POPUPMODE_GRABFOCUS));
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
        highlightMenuItem(nMenuPos, true);
        mnSelectedMenu = nMenuPos;
        fireMenuHighlightedEvent();
    }
}

void ScMenuFloatingWindow::fillMenuItemsToAccessible(ScAccessibleFilterMenu* pAccMenu) const
{
    vector<MenuItemData>::const_iterator itr, itrBeg = maMenuItems.begin(), itrEnd = maMenuItems.end();
    for (itr = itrBeg; itr != itrEnd; ++itr)
    {
        size_t nPos = ::std::distance(itrBeg, itr);
        pAccMenu->appendMenuItem(itr->maText, itr->mbEnabled, nPos);
    }
}

void ScMenuFloatingWindow::resizeToFitMenuItems()
{
    SetOutputSizePixel(getMenuSize());
}

void ScMenuFloatingWindow::selectMenuItem(size_t nPos, bool bSelected, bool bSubMenuTimer)
{
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

    highlightMenuItem(nPos, bSelected);

    if (bSelected)
    {
        if (mpParentMenu)
            mpParentMenu->setSubMenuFocused(this);

        if (bSubMenuTimer)
        {
            if (maMenuItems[nPos].mpSubMenuWin)
            {
                ScMenuFloatingWindow* pSubMenu = maMenuItems[nPos].mpSubMenuWin.get();
                queueLaunchSubMenu(nPos, pSubMenu);
            }
            else
                queueCloseSubMenu();
        }
    }
}

void ScMenuFloatingWindow::clearSelectedMenuItem()
{
    selectMenuItem(mnSelectedMenu, false, false);
    mnSelectedMenu = MENU_NOT_SELECTED;
}

ScMenuFloatingWindow* ScMenuFloatingWindow::getSubMenuWindow(size_t nPos) const
{
    if (maMenuItems.size() <= nPos)
        return NULL;

    return maMenuItems[nPos].mpSubMenuWin.get();
}

bool ScMenuFloatingWindow::isMenuItemSelected(size_t nPos) const
{
    return nPos == mnSelectedMenu;
}

void ScMenuFloatingWindow::setName(const OUString& rName)
{
    maName = rName;
}

void ScMenuFloatingWindow::highlightMenuItem(size_t nPos, bool bSelected)
{
    if (nPos == MENU_NOT_SELECTED)
        return;

    const StyleSettings& rStyle = GetSettings().GetStyleSettings();
    Color aBackColor = rStyle.GetMenuColor();
    SetFillColor(aBackColor);
    SetLineColor(aBackColor);

    Point aPos;
    Size aSize;
    getMenuItemPosSize(nPos, aPos, aSize);
    Rectangle aRegion(aPos,aSize);

    if (IsNativeControlSupported(CTRL_MENU_POPUP, PART_ENTIRE_CONTROL))
    {
        Push(PushFlags::CLIPREGION);
        IntersectClipRegion(Rectangle(aPos, aSize));
        Rectangle aCtrlRect(Point(0,0), GetOutputSizePixel());
        DrawNativeControl(
            CTRL_MENU_POPUP, PART_ENTIRE_CONTROL, aCtrlRect, CTRL_STATE_ENABLED,
            ImplControlValue(), OUString());

        Pop();
    }

    bool bNativeDrawn = true;
    if (IsNativeControlSupported(CTRL_MENU_POPUP, PART_MENU_ITEM))
    {
        ControlState nState = bSelected ? CTRL_STATE_SELECTED : 0;
        if (maMenuItems[nPos].mbEnabled)
            nState |= CTRL_STATE_ENABLED;
        bNativeDrawn = DrawNativeControl(
            CTRL_MENU_POPUP, PART_MENU_ITEM, aRegion, nState, ImplControlValue(), OUString());
    }
    else
        bNativeDrawn = false;

    if (!bNativeDrawn)
    {
        if (bSelected)
        {
            aBackColor = rStyle.GetMenuHighlightColor();
            SetFillColor(aBackColor);
            SetLineColor(aBackColor);
        }
        DrawRect(Rectangle(aPos,aSize));
    }

    Color aTextColor = bSelected ? rStyle.GetMenuHighlightTextColor() : rStyle.GetMenuTextColor();
    SetTextColor(aTextColor);
    drawMenuItem(nPos);
}

void ScMenuFloatingWindow::getMenuItemPosSize(size_t nPos, Point& rPos, Size& rSize) const
{
    size_t nCount = maMenuItems.size();
    if (nPos >= nCount)
        return;

    const sal_uInt16 nLeftMargin = 5;
    const sal_uInt16 nTopMargin = 5;
    const sal_uInt16 nMenuItemHeight = static_cast<sal_uInt16>(maLabelFont.GetHeight()*1.8);
    const sal_uInt16 nSepHeight = static_cast<sal_uInt16>(maLabelFont.GetHeight()*0.8);

    Point aPos1(nLeftMargin, nTopMargin);
    rPos = aPos1;
    for (size_t i = 0; i < nPos; ++i)
        rPos.Y() += maMenuItems[i].mbSeparator ? nSepHeight : nMenuItemHeight;

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
        Rectangle aRect(aPos, aSize);
        if (aRect.IsInside(rPos))
            return maMenuItems[i].mbSeparator ? MENU_NOT_SELECTED : i;
    }
    return MENU_NOT_SELECTED;
}

size_t ScMenuFloatingWindow::getSubMenuPos(ScMenuFloatingWindow* pSubMenu)
{
    size_t n = maMenuItems.size();
    for (size_t i = 0; i < n; ++i)
    {
        if (maMenuItems[i].mpSubMenuWin.get() == pSubMenu)
            return i;
    }
    return MENU_NOT_SELECTED;
}

void ScMenuFloatingWindow::fireMenuHighlightedEvent()
{
    if (mnSelectedMenu == MENU_NOT_SELECTED)
        return;

    if (!mxAccessible.is())
        return;

    Reference<XAccessibleContext> xAccCxt = mxAccessible->getAccessibleContext();
    if (!xAccCxt.is())
        return;

    Reference<XAccessible> xAccMenu = xAccCxt->getAccessibleChild(mnSelectedMenu);
    if (!xAccMenu.is())
        return;

    VclAccessibleEvent aEvent(VCLEVENT_MENU_HIGHLIGHT, xAccMenu);
    FireVclEvent(&aEvent);
}

void ScMenuFloatingWindow::setSubMenuFocused(ScMenuFloatingWindow* pSubMenu)
{
    maCloseTimer.reset();
    size_t nMenuPos = getSubMenuPos(pSubMenu);
    if (mnSelectedMenu != nMenuPos)
    {
        highlightMenuItem(nMenuPos, true);
        mnSelectedMenu = nMenuPos;
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

        sal_uInt32 nOldFlags = GetPopupModeFlags();
        SetPopupModeFlags(nOldFlags | FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE);
        pSubMenu->resizeToFitMenuItems(); // set the size before launching the popup to get it positioned correctly.
        pSubMenu->StartPopupMode(
            Rectangle(aPos,aSize), (FLOATWIN_POPUPMODE_RIGHT | FLOATWIN_POPUPMODE_GRABFOCUS));
        pSubMenu->AddPopupModeWindow(this);
        SetPopupModeFlags(nOldFlags);
    }
}

void ScMenuFloatingWindow::ensureSubMenuNotVisible()
{
    if (mnSelectedMenu <= maMenuItems.size() &&
        maMenuItems[mnSelectedMenu].mpSubMenuWin &&
        maMenuItems[mnSelectedMenu].mpSubMenuWin->IsVisible())
    {
        maMenuItems[mnSelectedMenu].mpSubMenuWin->ensureSubMenuNotVisible();
    }

    EndPopupMode();
}

void ScMenuFloatingWindow::terminateAllPopupMenus()
{
    EndPopupMode();
    if (mpParentMenu)
        mpParentMenu->terminateAllPopupMenus();
}

ScCheckListMenuWindow::Config::Config() :
    mbAllowEmptySet(true), mbRTL(false)
{
}

ScCheckListMenuWindow::Member::Member()
    : mbVisible(true)
    , mbDate(false)
    , mbLeaf(false)
    , mpParent(NULL)
{
}

ScCheckListMenuWindow::CancelButton::CancelButton(ScCheckListMenuWindow* pParent) :
    ::CancelButton(pParent), mpParent(pParent) {}

void ScCheckListMenuWindow::CancelButton::Click()
{
    mpParent->EndPopupMode();
    ::CancelButton::Click();
}

ScCheckListMenuWindow::ScCheckListMenuWindow(vcl::Window* pParent, ScDocument* pDoc) :
    ScMenuFloatingWindow(pParent, pDoc),
    maChecks(this,  WB_HASBUTTONS | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONSATROOT ),
    maChkToggleAll(this, 0),
    maBtnSelectSingle  (this, 0),
    maBtnUnselectSingle(this, 0),
    maBtnOk(this),
    maBtnCancel(this),
    mnCurTabStop(0),
    mpExtendedData(NULL),
    mpOKAction(NULL),
    mpPopupEndAction(NULL),
    maWndSize(200, 330),
    mePrevToggleAllState(TRISTATE_INDET)
{
    maTabStopCtrls.reserve(7);
    maTabStopCtrls.push_back(this);
    maTabStopCtrls.push_back(&maChecks);
    maTabStopCtrls.push_back(&maChkToggleAll);
    maTabStopCtrls.push_back(&maBtnSelectSingle);
    maTabStopCtrls.push_back(&maBtnUnselectSingle);
    maTabStopCtrls.push_back(&maBtnOk);
    maTabStopCtrls.push_back(&maBtnCancel);

    // Enable type-ahead search in the check list box.
    maChecks.SetStyle(maChecks.GetStyle() | WB_QUICK_SEARCH);
}

ScCheckListMenuWindow::~ScCheckListMenuWindow()
{
}

void ScCheckListMenuWindow::getSectionPosSize(
    Point& rPos, Size& rSize, SectionType eType) const
{
    // constant parameters.
    const long nListBoxMargin = 5;            // horizontal distance from the side of the dialog to the listbox border.
    const long nListBoxInnerPadding = 5;
    const long nTopMargin = 5;
    const long nMenuHeight = maMenuSize.getHeight();
    const long nSingleItemBtnAreaHeight = 32; // height of the middle area below the list box where the single-action buttons are.
    const long nBottomBtnAreaHeight = 50;     // height of the bottom area where the OK and Cancel buttons are.
    const long nBtnWidth = 90;
    const long nLabelHeight = getLabelFont().GetHeight();
    const long nBtnHeight = nLabelHeight*2;
    const long nBottomMargin = 10;
    const long nMenuListMargin = 5;

    // parameters calculated from constants.
    const long nListBoxWidth = maWndSize.Width() - nListBoxMargin*2;
    const long nListBoxHeight = maWndSize.Height() - nTopMargin - nMenuHeight -
        nMenuListMargin - nSingleItemBtnAreaHeight - nBottomBtnAreaHeight;

    const long nSingleBtnAreaY = nTopMargin + nMenuHeight + nListBoxHeight + nMenuListMargin - 1;

    switch (eType)
    {
        case WHOLE:
        {
            rPos  = Point(0, 0);
            rSize = maWndSize;
        }
        break;
        case LISTBOX_AREA_OUTER:
        {
            rPos = Point(nListBoxMargin, nTopMargin + nMenuHeight + nMenuListMargin);
            rSize = Size(nListBoxWidth, nListBoxHeight);
        }
        break;
        case LISTBOX_AREA_INNER:
        {
            rPos = Point(nListBoxMargin, nTopMargin + nMenuHeight + nMenuListMargin);
            rPos.X() += nListBoxInnerPadding;
            rPos.Y() += nListBoxInnerPadding;

            rSize = Size(nListBoxWidth, nListBoxHeight);
            rSize.Width()  -= nListBoxInnerPadding*2;
            rSize.Height() -= nListBoxInnerPadding*2;
        }
        break;
        case SINGLE_BTN_AREA:
        {
            rPos = Point(nListBoxMargin, nSingleBtnAreaY);
            rSize = Size(nListBoxWidth, nSingleItemBtnAreaHeight);
        }
        break;
        case CHECK_TOGGLE_ALL:
        {
            long h = std::min(maChkToggleAll.CalcMinimumSize().Height(), 26L);
            rPos = Point(nListBoxMargin, nSingleBtnAreaY);
            rPos.X() += 5;
            rPos.Y() += (nSingleItemBtnAreaHeight - h)/2;
            rSize = Size(70, h);
        }
        break;
        case BTN_SINGLE_SELECT:
        {
            long h = 26;
            rPos = Point(nListBoxMargin, nSingleBtnAreaY);
            rPos.X() += nListBoxWidth - h - 10 - h - 10;
            rPos.Y() += (nSingleItemBtnAreaHeight - h)/2;
            rSize = Size(h, h);
        }
        break;
        case BTN_SINGLE_UNSELECT:
        {
            long h = 26;
            rPos = Point(nListBoxMargin, nSingleBtnAreaY);
            rPos.X() += nListBoxWidth - h - 10;
            rPos.Y() += (nSingleItemBtnAreaHeight - h)/2;
            rSize = Size(h, h);
        }
        break;
        case BTN_OK:
        {
            long x = (maWndSize.Width() - nBtnWidth*2)/3;
            long y = maWndSize.Height() - nBottomMargin - nBtnHeight;
            rPos = Point(x, y);
            rSize = Size(nBtnWidth, nBtnHeight);
        }
        break;
        case BTN_CANCEL:
        {
            long x = (maWndSize.Width() - nBtnWidth*2)/3*2 + nBtnWidth;
            long y = maWndSize.Height() - nBottomMargin - nBtnHeight;
            rPos = Point(x, y);
            rSize = Size(nBtnWidth, nBtnHeight);
        }
        break;
        default:
            ;
    }
}

void ScCheckListMenuWindow::packWindow()
{
    maMenuSize = getMenuSize();

    if (maWndSize.Width() < maMenuSize.Width())
        // Widen the window to fit the menu items.
        maWndSize.Width() = maMenuSize.Width();

    // Set proper window height based on the number of menu items.
    if (maWndSize.Height() < maMenuSize.Height()*2.8)
        maWndSize.Height() = maMenuSize.Height()*2.8;

    // TODO: Make sure the window height never exceeds the height of the
    // screen. Also do adjustment based on the number of check box items.

    SetOutputSizePixel(maWndSize);

    const StyleSettings& rStyle = GetSettings().GetStyleSettings();

    Point aPos;
    Size aSize;
    getSectionPosSize(aPos, aSize, WHOLE);
    SetOutputSizePixel(aSize);

    getSectionPosSize(aPos, aSize, BTN_OK);
    maBtnOk.SetPosSizePixel(aPos, aSize);
    maBtnOk.SetFont(getLabelFont());
    maBtnOk.SetClickHdl( LINK(this, ScCheckListMenuWindow, ButtonHdl) );
    maBtnOk.Show();

    getSectionPosSize(aPos, aSize, BTN_CANCEL);
    maBtnCancel.SetPosSizePixel(aPos, aSize);
    maBtnCancel.SetFont(getLabelFont());
    maBtnCancel.Show();

    getSectionPosSize(aPos, aSize, LISTBOX_AREA_INNER);
    maChecks.SetPosSizePixel(aPos, aSize);
    maChecks.SetFont(getLabelFont());
    maChecks.SetCheckButtonHdl( LINK(this, ScCheckListMenuWindow, CheckHdl) );
    maChecks.Show();

    getSectionPosSize(aPos, aSize, CHECK_TOGGLE_ALL);
    maChkToggleAll.SetPosSizePixel(aPos, aSize);
    maChkToggleAll.SetFont(getLabelFont());
    maChkToggleAll.SetText(SC_STRLOAD(RID_POPUP_FILTER, STR_BTN_TOGGLE_ALL));
    maChkToggleAll.SetTextColor(rStyle.GetMenuTextColor());
    maChkToggleAll.SetControlBackground(rStyle.GetMenuColor());
    maChkToggleAll.SetClickHdl( LINK(this, ScCheckListMenuWindow, TriStateHdl) );
    maChkToggleAll.Show();

    getSectionPosSize(aPos, aSize, BTN_SINGLE_SELECT);
    maBtnSelectSingle.SetPosSizePixel(aPos, aSize);
    maBtnSelectSingle.SetQuickHelpText(SC_STRLOAD(RID_POPUP_FILTER, STR_BTN_SELECT_CURRENT));
    maBtnSelectSingle.SetModeImage(Image(ScResId(RID_IMG_SELECT_CURRENT)));
    maBtnSelectSingle.SetClickHdl( LINK(this, ScCheckListMenuWindow, ButtonHdl) );
    maBtnSelectSingle.Show();

    getSectionPosSize(aPos, aSize, BTN_SINGLE_UNSELECT);
    maBtnUnselectSingle.SetPosSizePixel(aPos, aSize);
    maBtnUnselectSingle.SetQuickHelpText(SC_STRLOAD(RID_POPUP_FILTER, STR_BTN_UNSELECT_CURRENT));
    maBtnUnselectSingle.SetModeImage(Image(ScResId(RID_IMG_UNSELECT_CURRENT)));
    maBtnUnselectSingle.SetClickHdl( LINK(this, ScCheckListMenuWindow, ButtonHdl) );
    maBtnUnselectSingle.Show();
}

void ScCheckListMenuWindow::setAllMemberState(bool bSet)
{
    size_t n = maMembers.size();
    for (size_t i = 0; i < n; ++i)
        maChecks.CheckEntry( maMembers[i].maName, maMembers[i].mpParent, bSet);

    if (!maConfig.mbAllowEmptySet)
        // We need to have at least one member selected.
        maBtnOk.Enable(maChecks.GetCheckedEntryCount() != 0);
}

void ScCheckListMenuWindow::selectCurrentMemberOnly(bool bSet)
{
    setAllMemberState(!bSet);
    SvTreeListEntry* pEntry = maChecks.GetCurEntry();
    if (!pEntry)
        return;
    maChecks.CheckEntry(pEntry, bSet );
}

void ScCheckListMenuWindow::cycleFocus(bool bReverse)
{
    maTabStopCtrls[mnCurTabStop]->SetFakeFocus(false);
    maTabStopCtrls[mnCurTabStop]->LoseFocus();
    if (mnCurTabStop == 0)
        clearSelectedMenuItem();

    if (bReverse)
    {
        if (mnCurTabStop > 0)
            --mnCurTabStop;
        else
            mnCurTabStop = maTabStopCtrls.size() - 1;
    }
    else
    {
        ++mnCurTabStop;
        if (mnCurTabStop >= maTabStopCtrls.size())
            mnCurTabStop = 0;
    }
    maTabStopCtrls[mnCurTabStop]->SetFakeFocus(true);
    maTabStopCtrls[mnCurTabStop]->GrabFocus();
}

IMPL_LINK( ScCheckListMenuWindow, ButtonHdl, Button*, pBtn )
{
    if (pBtn == &maBtnOk)
        close(true);
    else if (pBtn == &maBtnSelectSingle)
    {
        selectCurrentMemberOnly(true);
        CheckHdl(&maChecks);
    }
    else if (pBtn == &maBtnUnselectSingle)
    {
        selectCurrentMemberOnly(false);
        CheckHdl(&maChecks);
    }
    return 0;
}

IMPL_LINK_NOARG(ScCheckListMenuWindow, TriStateHdl)
{
    switch (mePrevToggleAllState)
    {
        case TRISTATE_FALSE:
            maChkToggleAll.SetState(TRISTATE_TRUE);
            setAllMemberState(true);
        break;
        case TRISTATE_TRUE:
            maChkToggleAll.SetState(TRISTATE_FALSE);
            setAllMemberState(false);
        break;
        case TRISTATE_INDET:
        default:
            maChkToggleAll.SetState(TRISTATE_TRUE);
            setAllMemberState(true);
        break;
    }

    mePrevToggleAllState = maChkToggleAll.GetState();
    return 0;
}

IMPL_LINK( ScCheckListMenuWindow, CheckHdl, SvTreeListBox*, pChecks )
{
    if (pChecks != &maChecks)
        return 0;
    SvTreeListEntry* pEntry = pChecks->GetHdlEntry();
    if ( pEntry )
        maChecks.CheckEntry( pEntry,  ( pChecks->GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED ) );
    size_t nNumChecked = maChecks.GetCheckedEntryCount();
    if (nNumChecked == maMembers.size())
        // all members visible
        maChkToggleAll.SetState(TRISTATE_TRUE);
    else if (nNumChecked == 0)
        // no members visible
        maChkToggleAll.SetState(TRISTATE_FALSE);
    else
        maChkToggleAll.SetState(TRISTATE_INDET);

    if (!maConfig.mbAllowEmptySet)
        // We need to have at least one member selected.
        maBtnOk.Enable(nNumChecked != 0);

    mePrevToggleAllState = maChkToggleAll.GetState();
    return 0;
}

void ScCheckListMenuWindow::MouseMove(const MouseEvent& rMEvt)
{
    ScMenuFloatingWindow::MouseMove(rMEvt);

    size_t nSelectedMenu = getSelectedMenuItem();
    if (nSelectedMenu == MENU_NOT_SELECTED)
        queueCloseSubMenu();
}

bool ScCheckListMenuWindow::Notify(NotifyEvent& rNEvt)
{
    switch (rNEvt.GetType())
    {
        case EVENT_KEYUP:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            const vcl::KeyCode& rCode = pKeyEvent->GetKeyCode();
            bool bShift = rCode.IsShift();
            if (rCode.GetCode() == KEY_TAB)
            {
                cycleFocus(bShift);
                return true;
            }
        }
        break;
    }
    return ScMenuFloatingWindow::Notify(rNEvt);
}

void ScCheckListMenuWindow::Paint(const Rectangle& rRect)
{
    ScMenuFloatingWindow::Paint(rRect);

    const StyleSettings& rStyle = GetSettings().GetStyleSettings();
    Color aMemberBackColor = rStyle.GetFieldColor();
    Color aBorderColor = rStyle.GetShadowColor();

    Point aPos;
    Size aSize;
    getSectionPosSize(aPos, aSize, LISTBOX_AREA_OUTER);

    // Member list box background
    SetFillColor(aMemberBackColor);
    SetLineColor(aBorderColor);
    DrawRect(Rectangle(aPos,aSize));

    // Single-action button box
    getSectionPosSize(aPos, aSize, SINGLE_BTN_AREA);
    SetFillColor(rStyle.GetMenuColor());
    DrawRect(Rectangle(aPos,aSize));
}

vcl::Window* ScCheckListMenuWindow::GetPreferredKeyInputWindow()
{
    return maTabStopCtrls[mnCurTabStop];
}

Reference<XAccessible> ScCheckListMenuWindow::CreateAccessible()
{
    if (!mxAccessible.is())
    {
        mxAccessible.set(new ScAccessibleFilterTopWindow(
            GetAccessibleParentWindow()->GetAccessible(), this, getName()));
        ScAccessibleFilterTopWindow* pAccTop = static_cast<ScAccessibleFilterTopWindow*>(mxAccessible.get());
        fillMenuItemsToAccessible(pAccTop);

        pAccTop->setAccessibleChild(
            maChecks.CreateAccessible(), ScAccessibleFilterTopWindow::LISTBOX);
        pAccTop->setAccessibleChild(
            maChkToggleAll.CreateAccessible(), ScAccessibleFilterTopWindow::TOGGLE_ALL);
        pAccTop->setAccessibleChild(
            maBtnSelectSingle.CreateAccessible(), ScAccessibleFilterTopWindow::SINGLE_ON_BTN);
        pAccTop->setAccessibleChild(
            maBtnUnselectSingle.CreateAccessible(), ScAccessibleFilterTopWindow::SINGLE_OFF_BTN);
        pAccTop->setAccessibleChild(
            maBtnOk.CreateAccessible(), ScAccessibleFilterTopWindow::OK_BTN);
        pAccTop->setAccessibleChild(
            maBtnCancel.CreateAccessible(), ScAccessibleFilterTopWindow::CANCEL_BTN);
    }

    return mxAccessible;
}

void ScCheckListMenuWindow::setMemberSize(size_t n)
{
    maMembers.reserve(n);
}

void ScCheckListMenuWindow::addDateMember(const OUString& rsName, double nVal, bool bVisible)
{
    ScDocument* pDoc = getDoc();
    SvNumberFormatter* pFormatter = pDoc->GetFormatTable();

    // Convert the numeric date value to a date object.
    Date aDate = *(pFormatter->GetNullDate());
    aDate += static_cast<long>(rtl::math::approxFloor(nVal));

    sal_uInt16 nYear = aDate.GetYear();
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

    maChecks.SetUpdateMode(false);

    SvTreeListEntry* pYearEntry = maChecks.FindEntry(NULL, aYearName);
    if (!pYearEntry)
    {
        pYearEntry = maChecks.InsertEntry(aYearName, NULL, true);
        Member aMemYear;
        aMemYear.maName = aYearName;
        aMemYear.maRealName = rsName;
        aMemYear.mbDate = true;
        aMemYear.mbLeaf = false;
        aMemYear.mbVisible = bVisible;
        aMemYear.mpParent = NULL;
        maMembers.push_back(aMemYear);
    }

    SvTreeListEntry* pMonthEntry = maChecks.FindEntry(pYearEntry, aMonthName);
    if (!pMonthEntry)
    {
        pMonthEntry = maChecks.InsertEntry(aMonthName, pYearEntry, true);
        Member aMemMonth;
        aMemMonth.maName = aMonthName;
        aMemMonth.maRealName = rsName;
        aMemMonth.mbDate = true;
        aMemMonth.mbLeaf = false;
        aMemMonth.mbVisible = bVisible;
        aMemMonth.mpParent = pYearEntry;
        maMembers.push_back(aMemMonth);
    }

    SvTreeListEntry* pDayEntry = maChecks.FindEntry(pMonthEntry, aDayName);
    if (!pDayEntry)
    {
        maChecks.InsertEntry(aDayName, pMonthEntry, false);
        Member aMemDay;
        aMemDay.maName = aDayName;
        aMemDay.maRealName = rsName;
        aMemDay.mbDate = true;
        aMemDay.mbLeaf = true;
        aMemDay.mbVisible = bVisible;
        aMemDay.mpParent = pMonthEntry;
        maMembers.push_back(aMemDay);
    }

    maChecks.SetUpdateMode(true);
}

void ScCheckListMenuWindow::addMember(const OUString& rName, bool bVisible)
{
    Member aMember;
    aMember.maName = rName;
    aMember.mbDate = false;
    aMember.mbLeaf = true;
    aMember.mbVisible = bVisible;
    aMember.mpParent = NULL;
    maMembers.push_back(aMember);
}

ScCheckListBox::ScCheckListBox( vcl::Window* pParent, WinBits nWinStyle )
   :  SvTreeListBox( pParent, nWinStyle ), mpCheckButton( NULL )
{
    Init();
}

SvTreeListEntry* ScCheckListBox::FindEntry( SvTreeListEntry* pParent, const OUString& sNode )
{
    sal_uInt16 nRootPos = 0;
    SvTreeListEntry* pEntry = pParent ? FirstChild( pParent ) : GetEntry( nRootPos );
    while ( pEntry )
    {
        if (  sNode.equals(GetEntryText( pEntry )) )
            return pEntry;

        pEntry = pParent ? NextSibling( pEntry ) : GetEntry( ++nRootPos );
    }
    return NULL;
}

void ScCheckListBox::Init()
{
    mpCheckButton = new SvLBoxButtonData( this );
    EnableCheckButton( mpCheckButton );
    SetNodeDefaultImages();
}

bool ScCheckListBox::IsChecked( const OUString& sName, SvTreeListEntry* pParent )
{
    SvTreeListEntry* pEntry = FindEntry( pParent, sName );
    if ( pEntry && GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED)
        return true;
    return false;
}

void ScCheckListBox::CheckEntry( const OUString& sName, SvTreeListEntry* pParent, bool bCheck )
{
    SvTreeListEntry* pEntry = FindEntry( pParent, sName );
    if ( pEntry )
        CheckEntry(  pEntry, bCheck );
}

// Recursively check all children of pParent
void ScCheckListBox::CheckAllChildren( SvTreeListEntry* pParent, bool bCheck )
{
    if ( pParent )
    {
        SetCheckButtonState(
            pParent, bCheck ? SvButtonState( SV_BUTTON_CHECKED ) :
                                           SvButtonState( SV_BUTTON_UNCHECKED ) );
    }
    SvTreeListEntry* pEntry = pParent ? FirstChild( pParent ) : First();
    while ( pEntry )
    {
        CheckAllChildren( pEntry, bCheck );
        pEntry = NextSibling( pEntry );
    }
}

void ScCheckListBox::CheckEntry( SvTreeListEntry* pParent, bool bCheck )
{
    // recursively check all items below pParent
    CheckAllChildren( pParent, bCheck );
    // checking pParent can affect ancestors, e.g. if ancestor is unchecked and pParent is
    // now checked then the ancestor needs to be checked also
    SvTreeListEntry* pAncestor = GetParent(pParent);
    if ( pAncestor )
    {
        while ( pAncestor )
        {
            // if any first level children checked then ancestor
            // needs to be checked, similarly if no first level children
            // checked then ancestor needs to be unchecked
            SvTreeListEntry* pChild = FirstChild( pAncestor );
            bool bChildChecked = false;

            while ( pChild )
            {
                if ( GetCheckButtonState( pChild ) == SV_BUTTON_CHECKED )
                {
                    bChildChecked = true;
                    break;
                }
                pChild = NextSibling( pChild );
            }
            SetCheckButtonState(
                pAncestor, bChildChecked ? SvButtonState( SV_BUTTON_CHECKED ) :
                                           SvButtonState( SV_BUTTON_UNCHECKED ) );
            pAncestor = GetParent(pAncestor);
        }
    }
}

SvTreeListEntry* ScCheckListBox::CountCheckedEntries( SvTreeListEntry* pParent, sal_uLong& nCount ) const
{
    if ( pParent && GetCheckButtonState( pParent ) == SV_BUTTON_CHECKED  )
        nCount++;
    // Iterate over the children
    SvTreeListEntry* pEntry = pParent ? FirstChild( pParent ) : First();
    while ( pEntry )
    {
        CountCheckedEntries( pEntry, nCount );
        pEntry = NextSibling( pEntry );
    }
    return NULL;
}

sal_uInt16 ScCheckListBox::GetCheckedEntryCount() const
{
    sal_uLong nCount = 0;
    CountCheckedEntries( NULL,  nCount );
    return nCount;
}

void ScCheckListBox::ExpandChildren( SvTreeListEntry* pParent )
{
    if ( pParent )
        Expand( pParent );
    // Iterate over the children
    SvTreeListEntry* pEntry = pParent ? FirstChild( pParent ) : First();
    while ( pEntry )
    {
        ExpandChildren( pEntry );
        pEntry = NextSibling( pEntry );
    }
}

void ScCheckListBox::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& rKey = rKEvt.GetKeyCode();

    if ( rKey.GetCode() == KEY_RETURN || rKey.GetCode() == KEY_SPACE )
    {
        SvTreeListEntry* pEntry = GetCurEntry();
        if ( pEntry )
        {
            bool bCheck = ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED );
            CheckEntry( pEntry, !bCheck );
            if ( bCheck != ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED ) )
                CheckButtonHdl();
        }
    }
    else if ( GetEntryCount() )
        SvTreeListBox::KeyInput( rKEvt );
}

void ScCheckListMenuWindow::initMembers()
{
    size_t n = maMembers.size();
    size_t nVisMemCount = 0;

    maChecks.SetUpdateMode(false);
    maChecks.GetModel()->EnableInvalidate(false);

    for (size_t i = 0; i < n; ++i)
    {
        if (maMembers[i].mbDate)
        {
            maChecks.CheckEntry(maMembers[i].maName, maMembers[i].mpParent, maMembers[i].mbVisible);
            // Expand first node of checked dates
            if (!maMembers[i].mpParent && maChecks.IsChecked(maMembers[i].maName,  maMembers[i].mpParent))
            {
                SvTreeListEntry* pEntry = maChecks.FindEntry(NULL, maMembers[i].maName);
                if (pEntry)
                    maChecks.Expand(pEntry);
            }
        }
        else
        {
            SvTreeListEntry* pEntry = maChecks.InsertEntry(
                maMembers[i].maName, NULL, false, TREELIST_APPEND, NULL,
                SvLBoxButtonKind_enabledCheckbox);

            maChecks.SetCheckButtonState(
                pEntry, maMembers[i].mbVisible ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED);
        }

        if (maMembers[i].mbVisible)
            ++nVisMemCount;
    }
    if (nVisMemCount == n)
    {
        // all members visible
        maChkToggleAll.SetState(TRISTATE_TRUE);
        mePrevToggleAllState = TRISTATE_TRUE;
    }
    else if (nVisMemCount == 0)
    {
        // no members visible
        maChkToggleAll.SetState(TRISTATE_FALSE);
        mePrevToggleAllState = TRISTATE_FALSE;
    }
    else
    {
        maChkToggleAll.SetState(TRISTATE_INDET);
        mePrevToggleAllState = TRISTATE_INDET;
    }

    maChecks.GetModel()->EnableInvalidate(true);
    maChecks.SetUpdateMode(true);
}

void ScCheckListMenuWindow::setConfig(const Config& rConfig)
{
    maConfig = rConfig;
}

bool ScCheckListMenuWindow::isAllSelected() const
{
    return maChkToggleAll.IsChecked();
}

void ScCheckListMenuWindow::getResult(ResultType& rResult)
{
    ResultType aResult;
    size_t n = maMembers.size();
    for (size_t i = 0; i < n; ++i)
    {
        if ( maMembers[i].mbLeaf )
        {
            bool bState =  maChecks.IsChecked( maMembers[i].maName,  maMembers[i].mpParent );
            OUString sName;
            if ( maMembers[i].mbDate )
                sName = maMembers[i].maRealName;
            else
                sName = maMembers[i].maName;
            aResult.insert(ResultType::value_type(sName, bState));
        }
    }
    rResult.swap(aResult);
}

void ScCheckListMenuWindow::launch(const Rectangle& rRect)
{
    packWindow();
    if (!maConfig.mbAllowEmptySet)
        // We need to have at least one member selected.
        maBtnOk.Enable(maChecks.GetCheckedEntryCount() != 0);

    Rectangle aRect(rRect);
    if (maConfig.mbRTL)
    {
        // In RTL mode, the logical "left" is visual "right".
        long nLeft = aRect.Left() - aRect.GetWidth();
        aRect.Left() = nLeft;
    }
    else if (maWndSize.Width() < aRect.GetWidth())
    {
        // Target rectangle (i.e. cell width) is wider than the window.
        // Simulate right-aligned launch by modifying the target rectangle
        // size.
        long nDiff = aRect.GetWidth() - maWndSize.Width();
        aRect.Left() += nDiff;
    }

    StartPopupMode(aRect, (FLOATWIN_POPUPMODE_DOWN | FLOATWIN_POPUPMODE_GRABFOCUS));
    cycleFocus(); // Set initial focus to the check list box.
}

void ScCheckListMenuWindow::close(bool bOK)
{
    if (bOK && mpOKAction.get())
        mpOKAction->execute();

    EndPopupMode();
}

void ScCheckListMenuWindow::setExtendedData(ExtendedData* p)
{
    mpExtendedData.reset(p);
}

ScCheckListMenuWindow::ExtendedData* ScCheckListMenuWindow::getExtendedData()
{
    return mpExtendedData.get();
}

void ScCheckListMenuWindow::setOKAction(Action* p)
{
    mpOKAction.reset(p);
}

void ScCheckListMenuWindow::setPopupEndAction(Action* p)
{
    mpPopupEndAction.reset(p);
}

void ScCheckListMenuWindow::handlePopupEnd()
{
    clearSelectedMenuItem();
    if (mpPopupEndAction)
        mpPopupEndAction->execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
