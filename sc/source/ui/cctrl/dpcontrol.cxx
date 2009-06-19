/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: document.hxx,v $
 * $Revision: 1.115.36.9 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include "dpcontrol.hxx"

#include "vcl/outdev.hxx"
#include "vcl/settings.hxx"
#include "vcl/wintypes.hxx"
#include "vcl/decoview.hxx"

#define MENU_NOT_SELECTED 999

using ::rtl::OUString;
using ::rtl::OUStringHash;
using ::std::vector;
using ::std::hash_map;

ScDPFieldButton::ScDPFieldButton(OutputDevice* pOutDev, const StyleSettings* pStyle) :
    mpOutDev(pOutDev),
    mpStyle(pStyle),
    mbPopupButton(false),
    mbHasHiddenMember(false)
{
}

ScDPFieldButton::~ScDPFieldButton()
{
}

void ScDPFieldButton::setText(const OUString& rText)
{
    maText = rText;
}

void ScDPFieldButton::setBoundingBox(const Point& rPos, const Size& rSize)
{
    maPos = rPos;
    maSize = rSize;
}

void ScDPFieldButton::setDrawPopupButton(bool b)
{
    mbPopupButton = b;
}

void ScDPFieldButton::setHasHiddenMember(bool b)
{
    mbHasHiddenMember = b;
}

void ScDPFieldButton::draw()
{
    const long nMargin = 2;

    // Background
    Rectangle aRect(maPos, maSize);
    mpOutDev->SetLineColor(mpStyle->GetFaceColor());
    mpOutDev->SetFillColor(mpStyle->GetFaceColor());
    mpOutDev->DrawRect(aRect);

    // Border lines
    mpOutDev->SetLineColor(mpStyle->GetLightColor());
    mpOutDev->DrawLine(Point(maPos), Point(maPos.X(), maPos.Y()+maSize.Height()-1));
    mpOutDev->DrawLine(Point(maPos), Point(maPos.X()+maSize.Width()-1, maPos.Y()));

    mpOutDev->SetLineColor(mpStyle->GetShadowColor());
    mpOutDev->DrawLine(Point(maPos.X(), maPos.Y()+maSize.Height()-1),
                       Point(maPos.X()+maSize.Width()-1, maPos.Y()+maSize.Height()-1));
    mpOutDev->DrawLine(Point(maPos.X()+maSize.Width()-1, maPos.Y()),
                       Point(maPos.X()+maSize.Width()-1, maPos.Y()+maSize.Height()-1));

    // Field name
    Font aTextFont( mpStyle->GetLabelFont() );
    aTextFont.SetHeight(12);
    mpOutDev->SetFont(aTextFont);

    Point aTextPos = maPos;
    long nTHeight = mpOutDev->GetTextHeight();
    aTextPos.setX(maPos.getX() + nMargin);
    aTextPos.setY(maPos.getY() + (maSize.Height()-nTHeight)/2);
    mpOutDev->DrawText(aTextPos, maText);

    if (mbPopupButton)
        drawPopupButton();
}

void ScDPFieldButton::getPopupBoundingBox(Point& rPos, Size& rSize) const
{
    long nW = maSize.getWidth()*0.5;
    long nH = maSize.getHeight();
    if (nW > 16)
        nW = 16;
    if (nH > 18)
        nH = 18;

    rPos.setX(maPos.getX() + maSize.getWidth() - nW);
    rPos.setY(maPos.getY() + maSize.getHeight() - nH);
    rSize.setWidth(nW);
    rSize.setHeight(nH);
}

bool ScDPFieldButton::isPopupButton() const
{
    return mbPopupButton;
}

void ScDPFieldButton::drawPopupButton()
{
    Point aPos;
    Size aSize;
    getPopupBoundingBox(aPos, aSize);

    // border lines
    mpOutDev->SetLineColor(mpStyle->GetLightColor());
    mpOutDev->DrawLine(aPos, Point(aPos.X(), aPos.Y()+aSize.Height()-1));
    mpOutDev->DrawLine(aPos, Point(aPos.X()+aSize.Width()-1, aPos.Y()));

    mpOutDev->SetLineColor(mpStyle->GetShadowColor());
    mpOutDev->DrawLine(Point(aPos.X(), aPos.Y()+aSize.Height()-1),
                       Point(aPos.X()+aSize.Width()-1, aPos.Y()+aSize.Height()-1));
    mpOutDev->DrawLine(Point(aPos.X()+aSize.Width()-1, aPos.Y()),
                       Point(aPos.X()+aSize.Width()-1, aPos.Y()+aSize.Height()-1));

    // the arrowhead
    Color aArrowColor = mbHasHiddenMember ? mpStyle->GetHighlightLinkColor() : mpStyle->GetButtonTextColor();
    mpOutDev->SetLineColor(aArrowColor);
    mpOutDev->SetFillColor(aArrowColor);
    Point aCenter(aPos.X() + (aSize.Width() >> 1), aPos.Y() + (aSize.Height() >> 1));
    Point aPos1, aPos2;
    aPos1.X() = aCenter.X() - 4;
    aPos2.X() = aCenter.X() + 4;
    aPos1.Y() = aCenter.Y() - 3;
    aPos2.Y() = aCenter.Y() - 3;

    do
    {
        ++aPos1.X();
        --aPos2.X();
        ++aPos1.Y();
        ++aPos2.Y();
        mpOutDev->DrawLine(aPos1, aPos2);
    }
    while (aPos1 != aPos2);

    if (mbHasHiddenMember)
    {
        // tiny little box to display in presence of hidden member(s).
        Point aBoxPos(aPos.X() + aSize.Width() - 5, aPos.Y() + aSize.Height() - 5);
        Size aBoxSize(3, 3);
        mpOutDev->DrawRect(Rectangle(aBoxPos, aBoxSize));
    }
}

// ============================================================================

ScMenuFloatingWindow::MenuItem::MenuItem() :
    mbEnabled(true),
    mpAction(static_cast<ScDPFieldPopupWindow::Action*>(NULL)),
    mpSubMenuWin(static_cast<ScMenuFloatingWindow*>(NULL))
{
}

// ----------------------------------------------------------------------------

ScMenuFloatingWindow::SubMenuItem::SubMenuItem(ScMenuFloatingWindow* pParent) :
    mpSubMenu(NULL),
    mnMenuPos(MENU_NOT_SELECTED),
    mpParent(pParent)
{
    maTimer.SetTimeoutHdl( LINK(this, ScMenuFloatingWindow::SubMenuItem, TimeoutHdl) );
    maTimer.SetTimeout(mpParent->GetSettings().GetMouseSettings().GetMenuDelay());
}

void ScMenuFloatingWindow::SubMenuItem::reset()
{
    mpSubMenu = NULL;
    mnMenuPos = MENU_NOT_SELECTED;
    maTimer.Stop();
}

IMPL_LINK( ScMenuFloatingWindow::SubMenuItem, TimeoutHdl, void*, EMPTYARG )
{
    mpParent->handleMenuTimeout(this);
    return 0;
}

// ----------------------------------------------------------------------------

ScMenuFloatingWindow::ScMenuFloatingWindow(Window* pParent) :
    FloatingWindow(pParent, (WB_SYSTEMFLOATWIN|WB_SYSTEMWINDOW|WB_NOBORDER)),
    maOpenTimer(this),
    maCloseTimer(this),
    mnSelectedMenu(MENU_NOT_SELECTED),
    mnClickedMenu(MENU_NOT_SELECTED),
    mpParentMenu(dynamic_cast<ScMenuFloatingWindow*>(pParent)),
    mpActiveSubMenu(NULL),
    mbActionFired(false)
{
    // TODO: How do we get the right font to use here ?
    const sal_uInt16 nPopupFontHeight = 12;
    const StyleSettings& rStyle = GetSettings().GetStyleSettings();
    maLabelFont = rStyle.GetLabelFont();
    maLabelFont.SetHeight(nPopupFontHeight);
    SetFont(maLabelFont);

    SetPopupModeEndHdl( LINK(this, ScMenuFloatingWindow, EndPopupHdl) );
}

ScMenuFloatingWindow::~ScMenuFloatingWindow()
{
    EndPopupMode();
}

void ScMenuFloatingWindow::MouseMove(const MouseEvent& rMEvt)
{
    const Point& rPos = rMEvt.GetPosPixel();
    size_t nSelectedMenu = getEnclosingMenuItem(rPos);
    setSelectedMenuItem(nSelectedMenu);

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
    executeMenu(mnClickedMenu);
    mnClickedMenu = MENU_NOT_SELECTED;
    Window::MouseButtonUp(rMEvt);
}

void ScMenuFloatingWindow::KeyInput(const KeyEvent& rKEvt)
{
    const KeyCode& rKeyCode = rKEvt.GetKeyCode();
    bool bHandled = true;
    size_t nSelectedMenu = mnSelectedMenu;
    size_t nLastMenuPos = maMenuItems.size() - 1;
    switch (rKeyCode.GetCode())
    {
        case KEY_UP:
            if (nSelectedMenu == MENU_NOT_SELECTED || nSelectedMenu == 0)
                nSelectedMenu = nLastMenuPos;
            else
                --nSelectedMenu;
            setSelectedMenuItem(nSelectedMenu, false);
        break;
        case KEY_DOWN:
            if (nSelectedMenu == MENU_NOT_SELECTED || nSelectedMenu == nLastMenuPos)
                nSelectedMenu = 0;
            else
                ++nSelectedMenu;
            setSelectedMenuItem(nSelectedMenu, false);
        break;
        case KEY_LEFT:
            if (mpParentMenu)
                mpParentMenu->endSubMenu();
        break;
        case KEY_RIGHT:
        {
            if (mnSelectedMenu >= maMenuItems.size() || mnSelectedMenu == MENU_NOT_SELECTED)
                break;

            const MenuItem& rMenu = maMenuItems[mnSelectedMenu];
            if (!rMenu.mbEnabled || !rMenu.mpSubMenuWin)
                break;

            maOpenTimer.mnMenuPos = mnSelectedMenu;
            maOpenTimer.mpSubMenu = rMenu.mpSubMenuWin.get();
            launchSubMenu(true);
        }
        break;
        case KEY_RETURN:
            if (nSelectedMenu != MENU_NOT_SELECTED)
                executeMenu(nSelectedMenu);
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
            CTRL_MENU_POPUP, PART_ENTIRE_CONTROL, Region(aCtrlRect), CTRL_STATE_ENABLED,
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

void ScMenuFloatingWindow::addMenuItem(const OUString& rText, bool bEnabled, Action* pAction)
{
    MenuItem aItem;
    aItem.maText = rText;
    aItem.mbEnabled = bEnabled;
    aItem.mpAction.reset(pAction);
    maMenuItems.push_back(aItem);
}

ScMenuFloatingWindow* ScMenuFloatingWindow::addSubMenuItem(const OUString& rText, bool bEnabled)
{
    MenuItem aItem;
    aItem.maText = rText;
    aItem.mbEnabled = bEnabled;
    aItem.mpSubMenuWin.reset(new ScMenuFloatingWindow(this));
    maMenuItems.push_back(aItem);
    return aItem.mpSubMenuWin.get();
}

void ScMenuFloatingWindow::drawMenuItem(size_t nPos)
{
    if (nPos >= maMenuItems.size())
        return;

    Point aPos;
    Size aSize;
    getMenuItemPosSize(aPos, aSize, nPos);

    DecorationView aDecoView(this);
    long nXOffset = 5;
    long nYOffset = (aSize.Height() - maLabelFont.GetHeight())/2;
    DrawCtrlText(Point(aPos.X()+nXOffset, aPos.Y() + nYOffset), maMenuItems[nPos].maText, 0, STRING_LEN,
                 maMenuItems[nPos].mbEnabled ? TEXT_DRAW_MNEMONIC : TEXT_DRAW_DISABLE);

    if (maMenuItems[nPos].mpSubMenuWin)
    {
        long nFontHeight = maLabelFont.GetHeight();
        Point aMarkerPos = aPos;
        aMarkerPos.Y() += aSize.Height()/2 - nFontHeight/4 + 1;
        aMarkerPos.X() += aSize.Width() - nFontHeight + nFontHeight/4;
        Size aMarkerSize(nFontHeight/2, nFontHeight/2);
        aDecoView.DrawSymbol(Rectangle(aMarkerPos, aMarkerSize),
                             SYMBOL_SPIN_RIGHT, GetTextColor(), 0);
    }
}

void ScMenuFloatingWindow::drawAllMenuItems()
{
    size_t n = maMenuItems.size();
    for (size_t i = 0; i < n; ++i)
        highlightMenuItem(i, i == mnSelectedMenu);
}

const Font& ScMenuFloatingWindow::getLabelFont() const
{
    return maLabelFont;
}

void ScMenuFloatingWindow::executeMenu(size_t nPos)
{
    if (nPos >= maMenuItems.size())
        return;

    if (!maMenuItems[nPos].mpAction)
        // no action is defined.
        return;

    maMenuItems[nPos].mpAction->execute();
    mbActionFired = true;
    EndPopupMode();
}

void ScMenuFloatingWindow::setSelectedMenuItem(size_t nPos, bool bSubMenuTimer)
{
    if (mnSelectedMenu != nPos)
    {
        selectMenuItem(mnSelectedMenu, false, bSubMenuTimer);
        selectMenuItem(nPos, true, bSubMenuTimer);
        mnSelectedMenu = nPos;
    }
}

size_t ScMenuFloatingWindow::getSelectedMenuItem() const
{
    return mnSelectedMenu;
}

void ScMenuFloatingWindow::handleMenuTimeout(SubMenuItem* pTimer)
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
    getMenuItemPosSize(aPos, aSize, maOpenTimer.mnMenuPos);
    ScMenuFloatingWindow* pSubMenu = maOpenTimer.mpSubMenu;

    if (!pSubMenu)
        return;

    sal_uInt32 nOldFlags = GetPopupModeFlags();
    SetPopupModeFlags(nOldFlags | FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE);
    pSubMenu->resetMenu(bSetMenuPos);
    pSubMenu->StartPopupMode(
        Rectangle(aPos,aSize), (FLOATWIN_POPUPMODE_RIGHT | FLOATWIN_POPUPMODE_GRABFOCUS));
    pSubMenu->AddPopupModeWindow(this);
    SetPopupModeFlags(nOldFlags);
}

void ScMenuFloatingWindow::endSubMenu()
{
    if (maOpenTimer.mpSubMenu)
    {
        maOpenTimer.mpSubMenu->EndPopupMode();
        maOpenTimer.mpSubMenu = NULL;
        highlightMenuItem(maOpenTimer.mnMenuPos, true);
    }
}

void ScMenuFloatingWindow::notify(NotificationType eType)
{
    switch (eType)
    {
        case SUBMENU_FOCUSED:
            // Cancel any request for ending submenu.
            maCloseTimer.reset();
            if (mnSelectedMenu != maOpenTimer.mnMenuPos)
            {
                highlightMenuItem(maOpenTimer.mnMenuPos, true);
                mnSelectedMenu = maOpenTimer.mnMenuPos;
            }
        break;
        default:
            ;
    }
}

void ScMenuFloatingWindow::resetMenu(bool bSetMenuPos)
{
    mnSelectedMenu = bSetMenuPos ? 0 : MENU_NOT_SELECTED;
    resizeToFitMenuItems();
}

void ScMenuFloatingWindow::resizeToFitMenuItems()
{
    if (maMenuItems.empty())
        return;

    vector<MenuItem>::const_iterator itr = maMenuItems.begin(), itrEnd = maMenuItems.end();
    long nTextWidth = 0;
    for (; itr != itrEnd; ++itr)
        nTextWidth = ::std::max(GetTextWidth(itr->maText), nTextWidth);

    size_t nLastPos = maMenuItems.size()-1;
    Point aPos;
    Size aSize;
    getMenuItemPosSize(aPos, aSize, nLastPos);
    aPos.X() += nTextWidth + 15;
    aPos.Y() += aSize.Height() + 5;
    SetOutputSizePixel(Size(aPos.X(), aPos.Y()));
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
            mpParentMenu->notify(SUBMENU_FOCUSED);

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

void ScMenuFloatingWindow::highlightMenuItem(size_t nPos, bool bSelected)
{
    const StyleSettings& rStyle = GetSettings().GetStyleSettings();
    Color aBackColor = rStyle.GetMenuColor();
    SetFillColor(aBackColor);
    SetLineColor(aBackColor);

    Point aPos;
    Size aSize;
    getMenuItemPosSize(aPos, aSize, nPos);
    Region aRegion(Rectangle(aPos,aSize));

    if (IsNativeControlSupported(CTRL_MENU_POPUP, PART_ENTIRE_CONTROL))
    {
        Push(PUSH_CLIPREGION);
        IntersectClipRegion(Rectangle(aPos, aSize));
        Rectangle aCtrlRect(Point(0,0), GetOutputSizePixel());
        DrawNativeControl(
            CTRL_MENU_POPUP, PART_ENTIRE_CONTROL, Region(aCtrlRect), CTRL_STATE_ENABLED,
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

void ScMenuFloatingWindow::getMenuItemPosSize(Point& rPos, Size& rSize, size_t nPos) const
{
    const sal_uInt16 nLeftMargin = 5;
    const sal_uInt16 nTopMargin = 5;
    const sal_uInt16 nMenuItemHeight = maLabelFont.GetHeight()*1.8;

    Size aWndSize = GetSizePixel();

    Point aPos1(nLeftMargin, nTopMargin);
    Size aSize1(aWndSize.Width() - nLeftMargin*2, nMenuItemHeight);

    rPos = aPos1;
    rPos.Y() += aSize1.Height()*nPos;
    rSize = aSize1;
}

size_t ScMenuFloatingWindow::getEnclosingMenuItem(const Point& rPos) const
{
    size_t n = maMenuItems.size();
    for (size_t i = 0; i < n; ++i)
    {
        Point aPos;
        Size aSize;
        getMenuItemPosSize(aPos, aSize, i);
        Rectangle aRect(aPos, aSize);
        if (aRect.IsInside(rPos))
            return i;
    }
    return MENU_NOT_SELECTED;
}

IMPL_LINK( ScMenuFloatingWindow, EndPopupHdl, void*, EMPTYARG )
{
    if (mbActionFired && mpParentMenu)
        mpParentMenu->EndPopupMode();

    return 0;
}

// ============================================================================

ScDPFieldPopupWindow::Member::Member() :
    mbVisible(true)
{
}

// ----------------------------------------------------------------------------

ScDPFieldPopupWindow::CancelButton::CancelButton(ScDPFieldPopupWindow* pParent) :
    ::CancelButton(pParent), mpParent(pParent) {}

void ScDPFieldPopupWindow::CancelButton::Click()
{
    mpParent->EndPopupMode();
    ::CancelButton::Click();
}

// ----------------------------------------------------------------------------

ScDPFieldPopupWindow::ScDPFieldPopupWindow(Window* pParent) :
    ScMenuFloatingWindow(pParent),
    maCheck0(this, 0),
    maCheck1(this, 0),
    maCheck2(this, 0),
    maCheck3(this, 0),
    maCheck4(this, 0),
    maCheck5(this, 0),
    maCheck6(this, 0),
    maCheck7(this, 0),
    maCheck8(this, 0),
    maCheck9(this, 0),
    maScrollBar(this, WB_VERT),
    maBtnOk(this),
    maBtnCancel(this),
    mpExtendedData(NULL),
    mpOKAction(NULL),
    mnScrollPos(0)
{
    Point aPos;
    Size aSize;
    getSectionPosSize(aPos, aSize, WHOLE);
    SetOutputSizePixel(aSize);
    Size aOutSize = GetOutputSizePixel();

    mpCheckPtr.reserve(10);
    mpCheckPtr.push_back(&maCheck0);
    mpCheckPtr.push_back(&maCheck1);
    mpCheckPtr.push_back(&maCheck2);
    mpCheckPtr.push_back(&maCheck3);
    mpCheckPtr.push_back(&maCheck4);
    mpCheckPtr.push_back(&maCheck5);
    mpCheckPtr.push_back(&maCheck6);
    mpCheckPtr.push_back(&maCheck7);
    mpCheckPtr.push_back(&maCheck8);
    mpCheckPtr.push_back(&maCheck9);

    getSectionPosSize(aPos, aSize, FIRST_LISTITEM);
    for (vector<CheckBox*>::iterator itr = mpCheckPtr.begin(), itrEnd = mpCheckPtr.end();
          itr != itrEnd; ++itr)
    {
        CheckBox* p = *itr;
        p->SetPosSizePixel(aPos, aSize);
        p->SetFont(getLabelFont());
        p->SetClickHdl( LINK(this, ScDPFieldPopupWindow, CheckBoxHdl) );
        aPos.Y() += aSize.Height() + 1;
    }

    getSectionPosSize(aPos, aSize, BTN_OK);
    maBtnOk.SetPosSizePixel(aPos, aSize);
    maBtnOk.SetFont(getLabelFont());
    maBtnOk.SetClickHdl( LINK(this, ScDPFieldPopupWindow, OKButtonHdl) );
    maBtnOk.Show();

    getSectionPosSize(aPos, aSize, BTN_CANCEL);
    maBtnCancel.SetPosSizePixel(aPos, aSize);
    maBtnCancel.SetFont(getLabelFont());
    maBtnCancel.Show();

    getSectionPosSize(aPos, aSize, SCROLL_BAR_V);
    maScrollBar.SetPosSizePixel(aPos, aSize);
    maScrollBar.SetPageSize(mpCheckPtr.size());
    maScrollBar.SetVisibleSize(mpCheckPtr.size());
    maScrollBar.SetLineSize(1);
    maScrollBar.SetScrollHdl( LINK(this, ScDPFieldPopupWindow, ScrollHdl) );
    maScrollBar.EnableDrag(true);
}

ScDPFieldPopupWindow::~ScDPFieldPopupWindow()
{
}

vector<ScDPFieldPopupWindow::Member>& ScDPFieldPopupWindow::getMembers()
{
    return maMembers;
}

void ScDPFieldPopupWindow::getSectionPosSize(Point& rPos, Size& rSize, SectionType eType) const
{
    const sal_uInt16 nListBoxMargin = 5;
    const sal_uInt16 nTopMargin = 5;
    const sal_uInt16 nMenuHeight = 60;
    const sal_uInt16 nBottomBtnAreaHeight = 50;
    const sal_uInt16 nInnerItemMargin = 5;
    const sal_uInt16 nScrollBarWidth = 17;
    const sal_uInt16 nBtnWidth = 60;
    const sal_uInt16 nBtnHeight = getLabelFont().GetHeight()*2;
    const sal_uInt16 nBottomMargin = 10;
    const sal_uInt16 nMenuListMargin = 20;

    Size aWndSize = Size(160, 330);

    switch (eType)
    {
        case WHOLE:
        {
            rPos  = Point(0, 0);
            rSize = aWndSize;
        }
        break;
        case LISTBOX_AREA:
        {
            rPos = Point(nListBoxMargin, nTopMargin + nMenuHeight + nMenuListMargin);
            rSize = Size(
                aWndSize.Width() - nListBoxMargin*2,
                aWndSize.Height() - nTopMargin - nMenuHeight - nMenuListMargin - nBottomBtnAreaHeight);
        }
        break;
        case FIRST_LISTITEM:
        {
            rPos = Point(nListBoxMargin + nInnerItemMargin,
                         nTopMargin + nMenuHeight + nMenuListMargin + nInnerItemMargin);
            rSize = Size(
                aWndSize.Width() - nListBoxMargin*2 - nInnerItemMargin - nScrollBarWidth - 10,
                17);
        }
        break;
        case BTN_OK:
        {
            long x = (aWndSize.Width() - nBtnWidth*2)/3;
            long y = aWndSize.Height() - nBottomMargin - nBtnHeight;
            rPos = Point(x, y);
            rSize = Size(nBtnWidth, nBtnHeight);
        }
        break;
        case BTN_CANCEL:
        {
            long x = (aWndSize.Width() - nBtnWidth*2)/3*2 + nBtnWidth;
            long y = aWndSize.Height() - nBottomMargin - nBtnHeight;
            rPos = Point(x, y);
            rSize = Size(nBtnWidth, nBtnHeight);
        }
        break;
        case SCROLL_BAR_V:
        {
            long x = aWndSize.Width() - nListBoxMargin - nInnerItemMargin - nScrollBarWidth;
            long y = nTopMargin + nMenuHeight + nMenuListMargin + nInnerItemMargin;
            rPos = Point(x, y);
            long h = aWndSize.Height() - nTopMargin - nMenuHeight - nMenuListMargin - nBottomBtnAreaHeight - nInnerItemMargin*2;
            rSize = Size(nScrollBarWidth, h);
        }
        break;
        default:
            ;
    }
}

void ScDPFieldPopupWindow::resetDisplayedItems()
{
    long nScrollPos = maScrollBar.GetThumbPos();
    if (nScrollPos < 0)
        return;

    mnScrollPos = static_cast<size_t>(nScrollPos);
    size_t nCheckCount = mpCheckPtr.size();
    for (size_t i = 0; i < nCheckCount; ++i)
    {
        CheckBox* p = mpCheckPtr[i];
        p->SetText(maMembers[i+mnScrollPos].maName);
        TriState nNewState = maMembers[i+mnScrollPos].mbVisible ? STATE_CHECK : STATE_NOCHECK;
        p->SetState(nNewState);
    }
}

IMPL_LINK( ScDPFieldPopupWindow, CheckBoxHdl, CheckBox*, pCheck )
{
    vector<CheckBox*>::const_iterator itr, itrBeg = mpCheckPtr.begin(), itrEnd = mpCheckPtr.end();
    for (itr = itrBeg; itr != itrEnd; ++itr)
    {
        if (*itr == pCheck)
        {
            size_t nIndex = ::std::distance(itrBeg, itr);
            maMembers[nIndex+mnScrollPos].mbVisible = !maMembers[nIndex+mnScrollPos].mbVisible;
        }
    }
    return 0;
}

IMPL_LINK( ScDPFieldPopupWindow, OKButtonHdl, OKButton*, EMPTYARG )
{
    close(true);
    return 0;
}

IMPL_LINK( ScDPFieldPopupWindow, ScrollHdl, ScrollBar*, EMPTYARG )
{
    resetDisplayedItems();
    return 0;
}

void ScDPFieldPopupWindow::MouseMove(const MouseEvent& rMEvt)
{
    ScMenuFloatingWindow::MouseMove(rMEvt);

    size_t nSelectedMenu = getSelectedMenuItem();
    if (nSelectedMenu == MENU_NOT_SELECTED)
        queueCloseSubMenu();
}

void ScDPFieldPopupWindow::Paint(const Rectangle& rRect)
{
    ScMenuFloatingWindow::Paint(rRect);

    const StyleSettings& rStyle = GetSettings().GetStyleSettings();
    Color aMemberBackColor = rStyle.GetFieldColor();

    // Member list box background
    SetFillColor(aMemberBackColor);
    Point aPos;
    Size aSize;
    getSectionPosSize(aPos, aSize, LISTBOX_AREA);
    DrawRect(Rectangle(aPos,aSize));
}

void ScDPFieldPopupWindow::setMemberSize(size_t n)
{
    maMembers.reserve(n);
}

void ScDPFieldPopupWindow::addMember(const OUString& rName, bool bVisible)
{
    Member aMember;
    aMember.maName = rName;
    aMember.mbVisible = bVisible;
    maMembers.push_back(aMember);
}

void ScDPFieldPopupWindow::initMembers()
{
    size_t nMemCount = maMembers.size();
    size_t nCheckCount = mpCheckPtr.size();
    bool bNeedsScroll = false;
    if (nMemCount > nCheckCount)
    {
        nMemCount = nCheckCount;
        bNeedsScroll = true;
    }

    for (size_t i = 0; i < nMemCount; ++i)
    {
        CheckBox* p = mpCheckPtr[i];
        p->SetText(maMembers[i].maName);
        p->Show();
        p->SetState(maMembers[i].mbVisible ? STATE_CHECK : STATE_NOCHECK);
    }
    if (bNeedsScroll)
    {
        maScrollBar.SetRange(Range(0, maMembers.size()));
        maScrollBar.Show();
    }
}

void ScDPFieldPopupWindow::getResult(hash_map<OUString, bool, OUStringHash>& rResult)
{
    typedef hash_map<OUString, bool, OUStringHash> ResultMap;
    ResultMap aResult;
    vector<Member>::const_iterator itr = maMembers.begin(), itrEnd = maMembers.end();
    for (; itr != itrEnd; ++itr)
        aResult.insert(ResultMap::value_type(itr->maName, itr->mbVisible));
    rResult.swap(aResult);
}

void ScDPFieldPopupWindow::close(bool bOK)
{
    if (bOK && mpOKAction.get())
        mpOKAction->execute();

    EndPopupMode();
}

void ScDPFieldPopupWindow::setExtendedData(ExtendedData* p)
{
    mpExtendedData.reset(p);
}

ScDPFieldPopupWindow::ExtendedData* ScDPFieldPopupWindow::getExtendedData()
{
    return mpExtendedData.get();
}

void ScDPFieldPopupWindow::setOKAction(Action* p)
{
    mpOKAction.reset(p);
}

