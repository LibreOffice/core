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
#include <tools/wintypes.hxx>
#include <unotools/charclass.hxx>

#include <AccessibleFilterMenu.hxx>
#include <AccessibleFilterTopWindow.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <vcl/svlbitm.hxx>
#include <vcl/treelistentry.hxx>
#include <document.hxx>

using namespace com::sun::star;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::accessibility::XAccessibleContext;
using ::std::vector;

ScMenuFloatingWindow::MenuItemData::MenuItemData() :
    mbEnabled(true), mbSeparator(false),
    mpAction(static_cast<ScCheckListMenuWindow::Action*>(nullptr)),
    mpSubMenuWin(static_cast<ScMenuFloatingWindow*>(nullptr))
{
}

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
    mnSelectedMenu(MENU_NOT_SELECTED),
    mnClickedMenu(MENU_NOT_SELECTED),
    mpDoc(pDoc),
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
        rMenuItem.mpSubMenuWin.disposeAndClear();
    mpParentMenu.clear();
    PopupMenuFloatingWindow::dispose();
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

void ScMenuFloatingWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
{
    const StyleSettings& rStyle = GetSettings().GetStyleSettings();

    SetFont(maLabelFont);

    Color aBackColor = rStyle.GetMenuColor();
    Color aBorderColor = rStyle.GetShadowColor();

    tools::Rectangle aCtrlRect(Point(0, 0), GetOutputSizePixel());

    // Window background
    bool bNativeDrawn = true;
    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::Entire))
    {
        rRenderContext.SetClipRegion();
        bNativeDrawn = rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::Entire, aCtrlRect,
                                                        ControlState::ENABLED, ImplControlValue(), OUString());
    }
    else
        bNativeDrawn = false;

    if (!bNativeDrawn)
    {
        rRenderContext.SetFillColor(aBackColor);
        rRenderContext.SetLineColor(aBorderColor);
        rRenderContext.DrawRect(aCtrlRect);
    }

    // Menu items
    rRenderContext.SetTextColor(rStyle.GetMenuTextColor());
    drawAllMenuItems(rRenderContext);
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

        size_t nPos = 0;
        for (const auto& rMenuItem : maMenuItems)
        {
            p->appendMenuItem(rMenuItem.maText, nPos);
            ++nPos;
        }
    }

    return mxAccessible;
}

void ScMenuFloatingWindow::addMenuItem(const OUString& rText, Action* pAction)
{
    MenuItemData aItem;
    aItem.maText = rText;
    aItem.mbEnabled = true;
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
    aItem.mpSubMenuWin.reset(VclPtr<ScMenuFloatingWindow>::Create(this, mpDoc, GetMenuStackLevel()+1));
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
    rRenderContext. DrawCtrlText(Point(aPos.X()+nXOffset, aPos.Y() + nYOffset), maMenuItems[nPos].maText, 0,
                                 maMenuItems[nPos].maText.getLength(),
                                 maMenuItems[nPos].mbEnabled ? DrawTextFlags::Mnemonic : DrawTextFlags::Disable);

    if (maMenuItems[nPos].mpSubMenuWin)
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

void ScMenuFloatingWindow::fillMenuItemsToAccessible(ScAccessibleFilterMenu* pAccMenu) const
{
    size_t nPos = 0;
    for (const auto& rMenuItem : maMenuItems)
    {
        pAccMenu->appendMenuItem(rMenuItem.maText, nPos);
        ++nPos;
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

    Invalidate();

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
        return nullptr;

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

    VclAccessibleEvent aEvent(VclEventId::MenuHighlight, xAccMenu);
    FireVclEvent(aEvent);
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

ScCheckListMember::ScCheckListMember()
    : mbVisible(true)
    , mbDate(false)
    , mbLeaf(false)
    , meDatePartType(YEAR)
    , mpParent(nullptr)
{
}

ScCheckListMenuWindow::CancelButton::CancelButton(ScCheckListMenuWindow* pParent) :
    ::CancelButton(pParent), mpParent(pParent) {}

ScCheckListMenuWindow::CancelButton::~CancelButton()
{
    disposeOnce();
}

void ScCheckListMenuWindow::CancelButton::dispose()
{
    mpParent.clear();
    ::CancelButton::dispose();
}

void ScCheckListMenuWindow::CancelButton::Click()
{
    mpParent->EndPopupMode();
    ::CancelButton::Click();
}

ScCheckListMenuWindow::ScCheckListMenuWindow(vcl::Window* pParent, ScDocument* pDoc) :
    ScMenuFloatingWindow(pParent, pDoc),
    maEdSearch(VclPtr<ScSearchEdit>::Create(this)),
    maChecks(VclPtr<ScCheckListBox>::Create(this)),
    maChkToggleAll(VclPtr<TriStateBox>::Create(this, 0)),
    maBtnSelectSingle(VclPtr<ImageButton>::Create(this, 0)),
    maBtnUnselectSingle(VclPtr<ImageButton>::Create(this, 0)),
    maBtnOk(VclPtr<OKButton>::Create(this)),
    maBtnCancel(VclPtr<CancelButton>::Create(this)),
    maWndSize(),
    mePrevToggleAllState(TRISTATE_INDET),
    maTabStops(this)
{
    float fScaleFactor = GetDPIScaleFactor();

    maWndSize = Size(200 * fScaleFactor, 330 * fScaleFactor);

    maTabStops.AddTabStop( this );
    maTabStops.AddTabStop( maEdSearch.get() );
    maTabStops.AddTabStop( maChecks.get() );
    maTabStops.AddTabStop( maChkToggleAll.get() );
    maTabStops.AddTabStop( maBtnSelectSingle.get() );
    maTabStops.AddTabStop( maBtnUnselectSingle.get() );
    maTabStops.AddTabStop( maBtnOk.get() );
    maTabStops.AddTabStop( maBtnCancel.get() );

    maEdSearch->SetTabStopsContainer( &maTabStops );
    maChecks->SetTabStopsContainer( &maTabStops );

    set_id("check_list_menu");
    maChkToggleAll->set_id("toggle_all");
    maBtnSelectSingle->set_id("select_current");
    maBtnUnselectSingle->set_id("unselect_current");
}

ScCheckListMenuWindow::~ScCheckListMenuWindow()
{
    disposeOnce();
}

void ScCheckListMenuWindow::dispose()
{
    maTabStops.clear();
    maEdSearch.disposeAndClear();
    maChecks.disposeAndClear();
    maChkToggleAll.disposeAndClear();
    maBtnSelectSingle.disposeAndClear();
    maBtnUnselectSingle.disposeAndClear();
    maBtnOk.disposeAndClear();
    maBtnCancel.disposeAndClear();
    ScMenuFloatingWindow::dispose();
}

void ScCheckListMenuWindow::getSectionPosSize(
    Point& rPos, Size& rSize, SectionType eType) const
{
    float fScaleFactor = GetDPIScaleFactor();

    // constant parameters.
    const long nSearchBoxMargin = 10 *fScaleFactor;
    const long nListBoxMargin = 5 * fScaleFactor;            // horizontal distance from the side of the dialog to the listbox border.
    const long nListBoxInnerPadding = 5 * fScaleFactor;
    const long nTopMargin = 5 * fScaleFactor;
    const long nMenuHeight = maMenuSize.getHeight();
    const long nSingleItemBtnAreaHeight = 32 * fScaleFactor; // height of the middle area below the list box where the single-action buttons are.
    const long nBottomBtnAreaHeight = 50 * fScaleFactor;     // height of the bottom area where the OK and Cancel buttons are.
    const long nBtnWidth = 90 * fScaleFactor;
    const long nLabelHeight = getLabelFont().GetFontHeight();
    const long nBtnHeight = nLabelHeight * 2;
    const long nBottomMargin = 10 * fScaleFactor;
    const long nMenuListMargin = 5 * fScaleFactor;
    const long nSearchBoxHeight = nLabelHeight * 2;

    // parameters calculated from constants.
    const long nListBoxWidth = maWndSize.Width() - nListBoxMargin*2;
    const long nListBoxHeight = maWndSize.Height() - nTopMargin - nMenuHeight -
        nMenuListMargin - nSearchBoxHeight - nSearchBoxMargin - nSingleItemBtnAreaHeight - nBottomBtnAreaHeight;

    const long nSingleBtnAreaY = nTopMargin + nMenuHeight + nMenuListMargin + nSearchBoxHeight + nSearchBoxMargin;

    switch (eType)
    {
        case WHOLE:
        {
            rPos  = Point(0, 0);
            rSize = maWndSize;
        }
        break;
        case EDIT_SEARCH:
        {
            rPos = Point(nSearchBoxMargin, nTopMargin + nMenuHeight + nMenuListMargin);
            rSize = Size(maWndSize.Width() - 2*nSearchBoxMargin, nSearchBoxHeight);
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
            long h = std::min(maChkToggleAll->CalcMinimumSize().Height(), 26L);
            rPos = Point(nListBoxMargin, nSingleBtnAreaY);
            rPos.AdjustX(5 );
            rPos.AdjustY((nSingleItemBtnAreaHeight - h)/2 );
            rSize = Size(70, h);
        }
        break;
        case BTN_SINGLE_SELECT:
        {
            long h = 26 * fScaleFactor;
            rPos = Point(nListBoxMargin, nSingleBtnAreaY);
            rPos.AdjustX(nListBoxWidth - h - 10 - h - 10 );
            rPos.AdjustY((nSingleItemBtnAreaHeight - h)/2 );
            rSize = Size(h, h);
        }
        break;
        case BTN_SINGLE_UNSELECT:
        {
            long h = 26 * fScaleFactor;
            rPos = Point(nListBoxMargin, nSingleBtnAreaY);
            rPos.AdjustX(nListBoxWidth - h - 10 );
            rPos.AdjustY((nSingleItemBtnAreaHeight - h)/2 );
            rSize = Size(h, h);
        }
        break;
        case LISTBOX_AREA_OUTER:
        {
            rPos = Point(nListBoxMargin, nSingleBtnAreaY + nSingleItemBtnAreaHeight-1);
            rSize = Size(nListBoxWidth, nListBoxHeight);
        }
        break;
        case LISTBOX_AREA_INNER:
        {
            rPos = Point(nListBoxMargin, nSingleBtnAreaY + nSingleItemBtnAreaHeight-1);
            rPos.AdjustX(nListBoxInnerPadding );
            rPos.AdjustY(nListBoxInnerPadding );

            rSize = Size(nListBoxWidth, nListBoxHeight);
            rSize.AdjustWidth( -(nListBoxInnerPadding*2) );
            rSize.AdjustHeight( -(nListBoxInnerPadding*2) );
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
        maWndSize.setWidth( maMenuSize.Width() );

    // Set proper window height based on the number of menu items.
    if (maWndSize.Height() < maMenuSize.Height()*2.8)
        maWndSize.setHeight( maMenuSize.Height()*2.8 );

    // TODO: Make sure the window height never exceeds the height of the
    // screen. Also do adjustment based on the number of check box items.

    SetOutputSizePixel(maWndSize);

    const StyleSettings& rStyle = GetSettings().GetStyleSettings();

    Point aPos;
    Size aSize;
    getSectionPosSize(aPos, aSize, WHOLE);
    SetOutputSizePixel(aSize);

    getSectionPosSize(aPos, aSize, BTN_OK);
    maBtnOk->SetPosSizePixel(aPos, aSize);
    maBtnOk->SetFont(getLabelFont());
    maBtnOk->SetClickHdl( LINK(this, ScCheckListMenuWindow, ButtonHdl) );
    maBtnOk->Show();

    getSectionPosSize(aPos, aSize, BTN_CANCEL);
    maBtnCancel->SetPosSizePixel(aPos, aSize);
    maBtnCancel->SetFont(getLabelFont());
    maBtnCancel->Show();

    getSectionPosSize(aPos, aSize, EDIT_SEARCH);
    maEdSearch->SetPosSizePixel(aPos, aSize);
    maEdSearch->SetFont(getLabelFont());
    maEdSearch->SetControlBackground(rStyle.GetFieldColor());
    maEdSearch->SetPlaceholderText(ScResId(STR_EDIT_SEARCH_ITEMS));
    maEdSearch->SetModifyHdl( LINK(this, ScCheckListMenuWindow, EdModifyHdl) );
    maEdSearch->Show();

    getSectionPosSize(aPos, aSize, LISTBOX_AREA_INNER);
    maChecks->SetPosSizePixel(aPos, aSize);
    maChecks->SetFont(getLabelFont());
    maChecks->SetCheckButtonHdl( LINK(this, ScCheckListMenuWindow, CheckHdl) );
    maChecks->Show();

    getSectionPosSize(aPos, aSize, CHECK_TOGGLE_ALL);
    maChkToggleAll->SetPosSizePixel(aPos, aSize);
    maChkToggleAll->SetFont(getLabelFont());
    maChkToggleAll->SetText(ScResId(STR_BTN_TOGGLE_ALL));
    maChkToggleAll->SetTextColor(rStyle.GetMenuTextColor());
    maChkToggleAll->SetControlBackground(rStyle.GetMenuColor());
    maChkToggleAll->SetClickHdl( LINK(this, ScCheckListMenuWindow, TriStateHdl) );
    maChkToggleAll->Show();

    float fScaleFactor = GetDPIScaleFactor();

    ;

    getSectionPosSize(aPos, aSize, BTN_SINGLE_SELECT);
    maBtnSelectSingle->SetPosSizePixel(aPos, aSize);
    maBtnSelectSingle->SetQuickHelpText(ScResId(STR_BTN_SELECT_CURRENT));
    maBtnSelectSingle->SetModeImage(Image(StockImage::Yes, RID_BMP_SELECT_CURRENT));
    maBtnSelectSingle->SetClickHdl( LINK(this, ScCheckListMenuWindow, ButtonHdl) );
    maBtnSelectSingle->Show();

    BitmapEx aSingleUnselectBmp(RID_BMP_UNSELECT_CURRENT);
    if (fScaleFactor > 1)
        aSingleUnselectBmp.Scale(fScaleFactor, fScaleFactor, BmpScaleFlag::Fast);
    Image aSingleUnselect(aSingleUnselectBmp);

    getSectionPosSize(aPos, aSize, BTN_SINGLE_UNSELECT);
    maBtnUnselectSingle->SetPosSizePixel(aPos, aSize);
    maBtnUnselectSingle->SetQuickHelpText(ScResId(STR_BTN_UNSELECT_CURRENT));
    maBtnUnselectSingle->SetModeImage(aSingleUnselect);
    maBtnUnselectSingle->SetClickHdl( LINK(this, ScCheckListMenuWindow, ButtonHdl) );
    maBtnUnselectSingle->Show();
}

void ScCheckListMenuWindow::setAllMemberState(bool bSet)
{
    size_t n = maMembers.size();
    std::set<SvTreeListEntry*> aParents;
    for (size_t i = 0; i < n; ++i)
    {
        aParents.insert(maMembers[i].mpParent);
    }

    for (const auto& pParent : aParents)
    {
        if (!pParent)
        {
            sal_uInt32 nCount = maChecks->GetEntryCount();
            for( sal_uInt32 i = 0; i < nCount; ++i)
            {
                SvTreeListEntry* pEntry = maChecks->GetEntry(i);
                if (!pEntry)
                    continue;

                maChecks->CheckEntry(pEntry, bSet);
            }
        }
        else
        {
            SvTreeListEntries& rEntries = pParent->GetChildEntries();
            for (const auto& rxEntry : rEntries)
            {
                maChecks->CheckEntry(rxEntry.get(), bSet);
            }
        }
    }

    if (!maConfig.mbAllowEmptySet)
        // We need to have at least one member selected.
        maBtnOk->Enable(maChecks->GetCheckedEntryCount() != 0);
}

void ScCheckListMenuWindow::selectCurrentMemberOnly(bool bSet)
{
    setAllMemberState(!bSet);
    SvTreeListEntry* pEntry = maChecks->GetCurEntry();
    if (!pEntry)
        return;
    maChecks->CheckEntry(pEntry, bSet );
}

IMPL_LINK( ScCheckListMenuWindow, ButtonHdl, Button*, pBtn, void )
{
    if (pBtn == maBtnOk.get())
        close(true);
    else if (pBtn == maBtnSelectSingle.get())
    {
        selectCurrentMemberOnly(true);
        CheckHdl(maChecks.get());
    }
    else if (pBtn == maBtnUnselectSingle.get())
    {
        selectCurrentMemberOnly(false);
        CheckHdl(maChecks.get());
    }
}

IMPL_LINK_NOARG(ScCheckListMenuWindow, TriStateHdl, Button*, void)
{
    switch (mePrevToggleAllState)
    {
        case TRISTATE_FALSE:
            maChkToggleAll->SetState(TRISTATE_TRUE);
            setAllMemberState(true);
        break;
        case TRISTATE_TRUE:
            maChkToggleAll->SetState(TRISTATE_FALSE);
            setAllMemberState(false);
        break;
        case TRISTATE_INDET:
        default:
            maChkToggleAll->SetState(TRISTATE_TRUE);
            setAllMemberState(true);
        break;
    }

    mePrevToggleAllState = maChkToggleAll->GetState();
}

IMPL_LINK_NOARG(ScCheckListMenuWindow, EdModifyHdl, Edit&, void)
{
    OUString aSearchText = maEdSearch->GetText();
    aSearchText = ScGlobal::pCharClass->lowercase( aSearchText );
    bool bSearchTextEmpty = aSearchText.isEmpty();
    size_t n = maMembers.size();
    size_t nSelCount = 0;
    OUString aLabelDisp;
    bool bSomeDateDeletes = false;

    for (size_t i = 0; i < n; ++i)
    {
        bool bIsDate = maMembers[i].mbDate;
        bool bPartialMatch = false;

        aLabelDisp = maMembers[i].maName;
        if ( aLabelDisp.isEmpty() )
            aLabelDisp = ScResId( STR_EMPTYDATA );

        if ( !bSearchTextEmpty )
        {
            if ( !bIsDate )
                bPartialMatch = ( ScGlobal::pCharClass->lowercase( aLabelDisp ).indexOf( aSearchText ) != -1 );
            else if ( maMembers[i].meDatePartType == ScCheckListMember::DAY ) // Match with both numerical and text version of month
                bPartialMatch = (ScGlobal::pCharClass->lowercase( OUString(
                                maMembers[i].maRealName + maMembers[i].maDateParts[1] )).indexOf( aSearchText ) != -1);
            else
                continue;
        }
        else if ( bIsDate && maMembers[i].meDatePartType != ScCheckListMember::DAY )
            continue;

        if ( bSearchTextEmpty )
        {
            SvTreeListEntry* pLeaf = maChecks->ShowCheckEntry( aLabelDisp, maMembers[i], true, maMembers[i].mbVisible );
            updateMemberParents( pLeaf, i );
            if ( maMembers[i].mbVisible )
                ++nSelCount;
            continue;
        }

        if ( bPartialMatch )
        {
            SvTreeListEntry* pLeaf = maChecks->ShowCheckEntry( aLabelDisp, maMembers[i] );
            updateMemberParents( pLeaf, i );
            ++nSelCount;
        }
        else
        {
            maChecks->ShowCheckEntry( aLabelDisp, maMembers[i], false, false );
            if( bIsDate )
                bSomeDateDeletes = true;
        }
    }

    if ( bSomeDateDeletes )
    {
        for (size_t i = 0; i < n; ++i)
        {
            if ( !maMembers[i].mbDate ) continue;
            if ( maMembers[i].meDatePartType != ScCheckListMember::DAY ) continue;
            updateMemberParents( nullptr, i );
        }
    }

    if ( nSelCount == n )
        maChkToggleAll->SetState( TRISTATE_TRUE );
    else if ( nSelCount == 0 )
        maChkToggleAll->SetState( TRISTATE_FALSE );
    else
        maChkToggleAll->SetState( TRISTATE_INDET );

    if ( !maConfig.mbAllowEmptySet )
        maBtnOk->Enable( nSelCount != 0);
}

IMPL_LINK( ScCheckListMenuWindow, CheckHdl, SvTreeListBox*, pChecks, void )
{
    if (pChecks != maChecks.get())
        return;
    SvTreeListEntry* pEntry = pChecks->GetHdlEntry();
    if ( pEntry )
        maChecks->CheckEntry( pEntry,  ( pChecks->GetCheckButtonState( pEntry ) == SvButtonState::Checked ) );
    size_t nNumChecked = maChecks->GetCheckedEntryCount();
    if (nNumChecked == maMembers.size())
        // all members visible
        maChkToggleAll->SetState(TRISTATE_TRUE);
    else if (nNumChecked == 0)
        // no members visible
        maChkToggleAll->SetState(TRISTATE_FALSE);
    else
        maChkToggleAll->SetState(TRISTATE_INDET);

    if (!maConfig.mbAllowEmptySet)
        // We need to have at least one member selected.
        maBtnOk->Enable(nNumChecked != 0);

    mePrevToggleAllState = maChkToggleAll->GetState();
}

void ScCheckListMenuWindow::MouseMove(const MouseEvent& rMEvt)
{
    ScMenuFloatingWindow::MouseMove(rMEvt);

    size_t nSelectedMenu = getSelectedMenuItem();
    if (nSelectedMenu == MENU_NOT_SELECTED)
        queueCloseSubMenu();
}

bool ScCheckListMenuWindow::EventNotify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == MouseNotifyEvent::KEYUP)
    {
        const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
        const vcl::KeyCode& rCode = pKeyEvent->GetKeyCode();
        bool bShift = rCode.IsShift();
        if (rCode.GetCode() == KEY_TAB)
        {
            maTabStops.CycleFocus(bShift);
            return true;
        }
    }
    return ScMenuFloatingWindow::EventNotify(rNEvt);
}

void ScCheckListMenuWindow::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    ScMenuFloatingWindow::Paint(rRenderContext, rRect);

    const StyleSettings& rStyle = GetSettings().GetStyleSettings();
    Color aMemberBackColor = rStyle.GetFieldColor();
    Color aBorderColor = rStyle.GetShadowColor();

    Point aPos;
    Size aSize;
    getSectionPosSize(aPos, aSize, LISTBOX_AREA_OUTER);

    // Member list box background
    rRenderContext.SetFillColor(aMemberBackColor);
    rRenderContext.SetLineColor(aBorderColor);
    rRenderContext.DrawRect(tools::Rectangle(aPos,aSize));

    // Single-action button box
    getSectionPosSize(aPos, aSize, SINGLE_BTN_AREA);
    rRenderContext.SetFillColor(rStyle.GetMenuColor());
    rRenderContext.DrawRect(tools::Rectangle(aPos,aSize));
}

void ScCheckListMenuWindow::updateMemberParents( const SvTreeListEntry* pLeaf, size_t nIdx )
{

    if ( !maMembers[nIdx].mbDate || maMembers[nIdx].meDatePartType != ScCheckListMember::DAY )
        return;

    OUString aYearName  = maMembers[nIdx].maDateParts[0];
    OUString aMonthName = maMembers[nIdx].maDateParts[1];
    auto aItr = maYearMonthMap.find(aYearName + aMonthName);

    if ( pLeaf )
    {
        SvTreeListEntry* pMonthEntry = pLeaf->GetParent();
        SvTreeListEntry* pYearEntry = pMonthEntry ? pMonthEntry->GetParent() : nullptr;

        maMembers[nIdx].mpParent = pMonthEntry;
        if ( aItr != maYearMonthMap.end() )
        {
            size_t nMonthIdx = aItr->second;
            maMembers[nMonthIdx].mpParent = pYearEntry;
        }
    }
    else
    {
        SvTreeListEntry* pYearEntry = maChecks->FindEntry( nullptr, aYearName );
        if ( aItr != maYearMonthMap.end() && !pYearEntry )
        {
            size_t nMonthIdx = aItr->second;
            maMembers[nMonthIdx].mpParent = nullptr;
            maMembers[nIdx].mpParent = nullptr;
        }
        else if ( pYearEntry && !maChecks->FindEntry( pYearEntry, aMonthName ) )
            maMembers[nIdx].mpParent = nullptr;
    }
}

Reference<XAccessible> ScCheckListMenuWindow::CreateAccessible()
{
    if (!mxAccessible.is() && maEdSearch)
    {
        mxAccessible.set(new ScAccessibleFilterTopWindow(
            GetAccessibleParentWindow()->GetAccessible(), this, getName()));
        ScAccessibleFilterTopWindow* pAccTop = static_cast<ScAccessibleFilterTopWindow*>(mxAccessible.get());
        fillMenuItemsToAccessible(pAccTop);

        pAccTop->setAccessibleChild(
            maEdSearch->CreateAccessible(), ScAccessibleFilterTopWindow::EDIT_SEARCH_BOX);
        pAccTop->setAccessibleChild(
            maChecks->CreateAccessible(), ScAccessibleFilterTopWindow::LISTBOX);
        pAccTop->setAccessibleChild(
            maChkToggleAll->CreateAccessible(), ScAccessibleFilterTopWindow::TOGGLE_ALL);
        pAccTop->setAccessibleChild(
            maBtnSelectSingle->CreateAccessible(), ScAccessibleFilterTopWindow::SINGLE_ON_BTN);
        pAccTop->setAccessibleChild(
            maBtnUnselectSingle->CreateAccessible(), ScAccessibleFilterTopWindow::SINGLE_OFF_BTN);
        pAccTop->setAccessibleChild(
            maBtnOk->CreateAccessible(), ScAccessibleFilterTopWindow::OK_BTN);
        pAccTop->setAccessibleChild(
            maBtnCancel->CreateAccessible(), ScAccessibleFilterTopWindow::CANCEL_BTN);
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

    maChecks->SetUpdateMode(false);

    SvTreeListEntry* pYearEntry = maChecks->FindEntry(nullptr, aYearName);
    if (!pYearEntry)
    {
        pYearEntry = maChecks->InsertEntry(aYearName, nullptr, true);
        ScCheckListMember aMemYear;
        aMemYear.maName = aYearName;
        aMemYear.maRealName = rsName;
        aMemYear.mbDate = true;
        aMemYear.mbLeaf = false;
        aMemYear.mbVisible = bVisible;
        aMemYear.mpParent = nullptr;
        aMemYear.meDatePartType = ScCheckListMember::YEAR;
        maMembers.push_back(aMemYear);
    }

    SvTreeListEntry* pMonthEntry = maChecks->FindEntry(pYearEntry, aMonthName);
    if (!pMonthEntry)
    {
        pMonthEntry = maChecks->InsertEntry(aMonthName, pYearEntry, true);
        ScCheckListMember aMemMonth;
        aMemMonth.maName = aMonthName;
        aMemMonth.maRealName = rsName;
        aMemMonth.mbDate = true;
        aMemMonth.mbLeaf = false;
        aMemMonth.mbVisible = bVisible;
        aMemMonth.mpParent = pYearEntry;
        aMemMonth.meDatePartType = ScCheckListMember::MONTH;
        maMembers.push_back(aMemMonth);
        maYearMonthMap[aYearName + aMonthName] = maMembers.size() - 1;
    }

    SvTreeListEntry* pDayEntry = maChecks->FindEntry(pMonthEntry, aDayName);
    if (!pDayEntry)
    {
        maChecks->InsertEntry(aDayName, pMonthEntry);
        ScCheckListMember aMemDay;
        aMemDay.maName = aDayName;
        aMemDay.maRealName = rsName;
        aMemDay.maDateParts.resize(2);
        aMemDay.maDateParts[0] = aYearName;
        aMemDay.maDateParts[1] = aMonthName;
        aMemDay.mbDate = true;
        aMemDay.mbLeaf = true;
        aMemDay.mbVisible = bVisible;
        aMemDay.mpParent = pMonthEntry;
        aMemDay.meDatePartType = ScCheckListMember::DAY;
        maMembers.push_back(aMemDay);
    }

    maChecks->SetUpdateMode(true);
}

void ScCheckListMenuWindow::addMember(const OUString& rName, bool bVisible)
{
    ScCheckListMember aMember;
    aMember.maName = rName;
    aMember.mbDate = false;
    aMember.mbLeaf = true;
    aMember.mbVisible = bVisible;
    aMember.mpParent = nullptr;
    maMembers.push_back(aMember);
}

ScTabStops::ScTabStops( ScCheckListMenuWindow* pMenuWin ) :
    mpMenuWindow( pMenuWin ),
    maControlToPos( ControlToPosMap() ),
    mnCurTabStop(0)
{
    maControls.reserve( 8 );
}

ScTabStops::~ScTabStops()
{}

void ScTabStops::AddTabStop( vcl::Window* pWin )
{
    maControls.emplace_back(pWin );
    maControlToPos[pWin] = maControls.size() - 1;
}

void ScTabStops::SetTabStop( vcl::Window* pWin )
{
    if ( maControls.empty() )
        return;
    ControlToPosMap::const_iterator aIter = maControlToPos.find( pWin );
    if ( aIter == maControlToPos.end() )
        return;
    if ( aIter->second == mnCurTabStop )
        return;
    if ( mnCurTabStop < maControls.size() )
    {
        maControls[mnCurTabStop]->SetFakeFocus( false );
        maControls[mnCurTabStop]->LoseFocus();
    }
    mnCurTabStop = aIter->second;
    maControls[mnCurTabStop]->SetFakeFocus( true );
    maControls[mnCurTabStop]->GrabFocus();
}

void ScTabStops::CycleFocus( bool bReverse )
{
    if (maControls.empty())
        return;
    if ( mnCurTabStop < maControls.size() )
    {
        maControls[mnCurTabStop]->SetFakeFocus( false );
        maControls[mnCurTabStop]->LoseFocus();
    }
    else
        mnCurTabStop = 0;

    if ( mpMenuWindow && mnCurTabStop == 0 )
        mpMenuWindow->clearSelectedMenuItem();

    size_t nIterCount = 0;

    if ( bReverse )
    {
        do
        {
            if ( mnCurTabStop > 0 )
                --mnCurTabStop;
            else
                mnCurTabStop = maControls.size() - 1;
            ++nIterCount;
        } while ( nIterCount <= maControls.size() && !maControls[mnCurTabStop]->IsEnabled() );
    }
    else
    {
        do
        {
            ++mnCurTabStop;
            if ( mnCurTabStop >= maControls.size() )
                mnCurTabStop = 0;
            ++nIterCount;
        } while ( nIterCount <= maControls.size() && !maControls[mnCurTabStop]->IsEnabled() );
    }

    if ( nIterCount <= maControls.size() )
    {
        maControls[mnCurTabStop]->SetFakeFocus( true );
        maControls[mnCurTabStop]->GrabFocus();
    }
    // else : all controls are disabled, so can't do anything
}

void ScTabStops::clear()
{
    mnCurTabStop = 0;
    maControlToPos.clear();
    maControls.clear();
}

ScCheckListBox::ScCheckListBox( vcl::Window* pParent )
    :  SvTreeListBox( pParent, 0 ), mbSeenMouseButtonDown( false )
{
    Init();
    set_id("check_list_box");
}

SvTreeListEntry* ScCheckListBox::FindEntry( SvTreeListEntry* pParent, const OUString& sNode )
{
    sal_uInt32 nRootPos = 0;
    SvTreeListEntry* pEntry = pParent ? FirstChild( pParent ) : GetEntry( nRootPos );
    while ( pEntry )
    {
        if ( sNode == GetEntryText( pEntry ) )
            return pEntry;

        pEntry = pParent ? pEntry->NextSibling() : GetEntry( ++nRootPos );
    }
    return nullptr;
}

void ScCheckListBox::Init()
{
    mpCheckButton.reset( new SvLBoxButtonData( this ) );
    EnableCheckButton( mpCheckButton.get() );
    SetNodeDefaultImages();
}

void ScCheckListBox::GetRecursiveChecked( SvTreeListEntry* pEntry, std::unordered_set<OUString>& vOut,
        OUString& rLabel )
{
    if (GetCheckButtonState(pEntry) == SvButtonState::Checked)
    {
        // We have to hash parents and children together.
        // Per convention for easy access in getResult()
        // "child;parent;grandparent" while descending.
        if (rLabel.isEmpty())
            rLabel = GetEntryText(pEntry);
        else
            rLabel = GetEntryText(pEntry) + ";" + rLabel;

        // Prerequisite: the selection mechanism guarantees that if a child is
        // selected then also the parent is selected, so we only have to
        // inspect the children in case the parent is selected.
        if (pEntry->HasChildren())
        {
            const SvTreeListEntries& rChildren = pEntry->GetChildEntries();
            for (auto& rChild : rChildren)
            {
                OUString aLabel = rLabel;
                GetRecursiveChecked( rChild.get(), vOut, aLabel);
                if (!aLabel.isEmpty() && aLabel != rLabel)
                    vOut.insert( aLabel);
            }
            // Let the caller not add the parent alone.
            rLabel.clear();
        }
    }
}

std::unordered_set<OUString> ScCheckListBox::GetAllChecked()
{
    std::unordered_set<OUString> vResults(0);
    sal_uInt32 nRootPos = 0;
    SvTreeListEntry* pEntry = GetEntry(nRootPos);
    while (pEntry)
    {
        OUString aLabel;
        GetRecursiveChecked( pEntry, vResults, aLabel);
        if (!aLabel.isEmpty())
            vResults.insert( aLabel);
        pEntry = GetEntry(++nRootPos);
    }

    return vResults;
}

bool ScCheckListBox::IsChecked( const OUString& sName, SvTreeListEntry* pParent )
{
    SvTreeListEntry* pEntry = FindEntry( pParent, sName );
    return pEntry && GetCheckButtonState( pEntry ) == SvButtonState::Checked;
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
            pParent, bCheck ? SvButtonState::Checked : SvButtonState::Unchecked );
    }
    SvTreeListEntry* pEntry = pParent ? FirstChild( pParent ) : First();
    while ( pEntry )
    {
        CheckAllChildren( pEntry, bCheck );
        pEntry = pEntry->NextSibling();
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
                if ( GetCheckButtonState( pChild ) == SvButtonState::Checked )
                {
                    bChildChecked = true;
                    break;
                }
                pChild = pChild->NextSibling();
            }
            SetCheckButtonState( pAncestor, bChildChecked ? SvButtonState::Checked : SvButtonState::Unchecked );
            pAncestor = GetParent(pAncestor);
        }
    }
}

SvTreeListEntry* ScCheckListBox::ShowCheckEntry( const OUString& sName, ScCheckListMember& rMember, bool bShow, bool bCheck )
{
    SvTreeListEntry* pEntry = nullptr;
    if (!rMember.mbDate || rMember.mpParent)
        pEntry = FindEntry( rMember.mpParent, sName );

    if ( bShow )
    {
        if ( !pEntry )
        {
            if (rMember.mbDate)
            {
                if (rMember.maDateParts.empty())
                    return nullptr;

                SvTreeListEntry* pYearEntry = FindEntry( nullptr, rMember.maDateParts[0] );
                if ( !pYearEntry )
                    pYearEntry = InsertEntry( rMember.maDateParts[0], nullptr, true );
                SvTreeListEntry* pMonthEntry = FindEntry( pYearEntry, rMember.maDateParts[1] );
                if ( !pMonthEntry )
                    pMonthEntry = InsertEntry( rMember.maDateParts[1], pYearEntry, true );
                SvTreeListEntry* pDayEntry = FindEntry( pMonthEntry, rMember.maName );
                if ( !pDayEntry )
                    pDayEntry = InsertEntry( rMember.maName, pMonthEntry );

                return pDayEntry; // Return leaf node
            }

            pEntry = InsertEntry(
                sName);

            SetCheckButtonState(
                pEntry, bCheck ? SvButtonState::Checked : SvButtonState::Unchecked);
        }
        else
            CheckEntry( pEntry, bCheck );
    }
    else if ( pEntry )
    {
        GetModel()->Remove( pEntry );
        SvTreeListEntry* pParent = rMember.mpParent;
        while ( pParent && !pParent->HasChildren() )
        {
            SvTreeListEntry* pTmp = pParent;
            pParent = pTmp->GetParent();
            GetModel()->Remove( pTmp );
        }
    }
    return nullptr;
}

void ScCheckListBox::CountCheckedEntries( SvTreeListEntry* pParent, sal_uLong& nCount ) const
{
    if ( pParent && GetCheckButtonState( pParent ) == SvButtonState::Checked  )
        nCount++;
    // Iterate over the children
    SvTreeListEntry* pEntry = pParent ? FirstChild( pParent ) : First();
    while ( pEntry )
    {
        CountCheckedEntries( pEntry, nCount );
        pEntry = pEntry->NextSibling();
    }
}

sal_uInt16 ScCheckListBox::GetCheckedEntryCount() const
{
    sal_uLong nCount = 0;
    CountCheckedEntries( nullptr,  nCount );
    return nCount;
}

void ScCheckListBox::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& rKey = rKEvt.GetKeyCode();

    if ( rKey.GetCode() == KEY_RETURN || rKey.GetCode() == KEY_SPACE )
    {
        SvTreeListEntry* pEntry = GetCurEntry();
        if ( pEntry )
        {
            bool bCheck = ( GetCheckButtonState( pEntry ) == SvButtonState::Checked );
            CheckEntry( pEntry, !bCheck );
            if ( bCheck != ( GetCheckButtonState( pEntry ) == SvButtonState::Checked ) )
                CheckButtonHdl();
        }
    }
    else if ( GetEntryCount() )
        SvTreeListBox::KeyInput( rKEvt );
}

void ScCheckListBox::MouseButtonDown(const MouseEvent& rMEvt)
{
    SvTreeListBox::MouseButtonDown( rMEvt );
    if ( rMEvt.IsLeft() )
        mbSeenMouseButtonDown = true;
}

void ScCheckListBox::MouseButtonUp(const MouseEvent& rMEvt)
{
    SvTreeListBox::MouseButtonUp( rMEvt );
    if ( mpTabStops && mbSeenMouseButtonDown && rMEvt.IsLeft() )
    {
        mpTabStops->SetTabStop( this );
        mbSeenMouseButtonDown = false;
    }
}

void ScSearchEdit::MouseButtonDown(const MouseEvent& rMEvt)
{
    Edit::MouseButtonDown( rMEvt );
    if ( mpTabStops && rMEvt.IsLeft() && rMEvt.GetClicks() >= 1 )
        mpTabStops->SetTabStop( this );
}

void ScCheckListMenuWindow::setHasDates(bool bHasDates)
{
    // Enables type-ahead search in the check list box.
    maChecks->SetQuickSearch(true);
    if (bHasDates)
        maChecks->SetStyle(WB_HASBUTTONS | WB_HASLINES | WB_HASLINESATROOT | WB_HASBUTTONSATROOT);
    else
        maChecks->SetStyle(WB_HASBUTTONS);
}

void ScCheckListMenuWindow::initMembers()
{
    size_t n = maMembers.size();
    size_t nVisMemCount = 0;

    maChecks->SetUpdateMode(false);
    maChecks->GetModel()->EnableInvalidate(false);

    for (size_t i = 0; i < n; ++i)
    {
        if (maMembers[i].mbDate)
        {
            maChecks->CheckEntry(maMembers[i].maName, maMembers[i].mpParent, maMembers[i].mbVisible);
            // Expand first node of checked dates
            if (!maMembers[i].mpParent && maChecks->IsChecked(maMembers[i].maName,  maMembers[i].mpParent))
            {
                SvTreeListEntry* pEntry = maChecks->FindEntry(nullptr, maMembers[i].maName);
                if (pEntry)
                    maChecks->Expand(pEntry);
            }
        }
        else
        {
            OUString aLabel = maMembers[i].maName;
            if (aLabel.isEmpty())
                aLabel = ScResId(STR_EMPTYDATA);
            SvTreeListEntry* pEntry = maChecks->InsertEntry(
                aLabel);

            maChecks->SetCheckButtonState(
                pEntry, maMembers[i].mbVisible ? SvButtonState::Checked : SvButtonState::Unchecked);
        }

        if (maMembers[i].mbVisible)
            ++nVisMemCount;
    }
    if (nVisMemCount == n)
    {
        // all members visible
        maChkToggleAll->SetState(TRISTATE_TRUE);
        mePrevToggleAllState = TRISTATE_TRUE;
    }
    else if (nVisMemCount == 0)
    {
        // no members visible
        maChkToggleAll->SetState(TRISTATE_FALSE);
        mePrevToggleAllState = TRISTATE_FALSE;
    }
    else
    {
        maChkToggleAll->SetState(TRISTATE_INDET);
        mePrevToggleAllState = TRISTATE_INDET;
    }

    maChecks->GetModel()->EnableInvalidate(true);
    maChecks->SetUpdateMode(true);
}

void ScCheckListMenuWindow::setConfig(const Config& rConfig)
{
    maConfig = rConfig;
}

bool ScCheckListMenuWindow::isAllSelected() const
{
    return maChkToggleAll->IsChecked();
}

void ScCheckListMenuWindow::getResult(ResultType& rResult)
{
    ResultType aResult;
    std::unordered_set<OUString> vCheckeds = maChecks->GetAllChecked();
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
            for (SvTreeListEntry* pParent = maMembers[i].mpParent;
                    pParent && pParent->GetFirstItem( SvLBoxItemType::String);
                    pParent = pParent->GetParent())
            {
                aLabel.append(";").append(maChecks->GetEntryText( pParent));
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

void ScCheckListMenuWindow::launch(const tools::Rectangle& rRect)
{
    packWindow();
    if (!maConfig.mbAllowEmptySet)
        // We need to have at least one member selected.
        maBtnOk->Enable(maChecks->GetCheckedEntryCount() != 0);

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

    StartPopupMode(aRect, (FloatWinPopupFlags::Down | FloatWinPopupFlags::GrabFocus));
    maTabStops.CycleFocus(); // Set initial focus to the search box ( index = 1 )
}

void ScCheckListMenuWindow::close(bool bOK)
{
    if (bOK && mpOKAction.get())
        mpOKAction->execute();

    EndPopupMode();
}

void ScCheckListMenuWindow::setExtendedData(std::unique_ptr<ExtendedData> p)
{
    mpExtendedData = std::move(p);
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
