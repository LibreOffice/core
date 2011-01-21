/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "dpcontrol.hrc"

#include <vcl/outdev.hxx>
#include <vcl/settings.hxx>
#include <tools/wintypes.hxx>
#include <vcl/decoview.hxx>
#include "strload.hxx"
#include "global.hxx"
#include "scitems.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "patattr.hxx"

#include "AccessibleFilterMenu.hxx"
#include "AccessibleFilterTopWindow.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::accessibility::XAccessible;
using ::com::sun::star::accessibility::XAccessibleContext;
using ::rtl::OUString;
using ::rtl::OUStringHash;
using ::std::vector;
using ::std::hash_map;
using ::std::auto_ptr;

ScDPFieldButton::ScDPFieldButton(OutputDevice* pOutDev, const StyleSettings* pStyle, const Fraction* pZoomX, const Fraction* pZoomY, ScDocument* pDoc) :
    mpDoc(pDoc),
    mpOutDev(pOutDev),
    mpStyle(pStyle),
    mbBaseButton(true),
    mbPopupButton(false),
    mbHasHiddenMember(false),
    mbPopupPressed(false),
    mbPopupLeft(false)
{
    if (pZoomX)
        maZoomX = *pZoomX;
    else
        maZoomX = Fraction(1, 1);

    if (pZoomY)
        maZoomY = *pZoomY;
    else
        maZoomY = Fraction(1, 1);
}

ScDPFieldButton::~ScDPFieldButton()
{
}

void ScDPFieldButton::setText(const OUString& rText)
{
    maText = rText;
}

void ScDPFieldButton::setBoundingBox(const Point& rPos, const Size& rSize, bool bLayoutRTL)
{
    maPos = rPos;
    maSize = rSize;
    if (bLayoutRTL)
    {
        // rPos is the logical-left position, adjust maPos to visual-left (inside the cell border)
        maPos.X() -= maSize.Width() - 1;
    }
}

void ScDPFieldButton::setDrawBaseButton(bool b)
{
    mbBaseButton = b;
}

void ScDPFieldButton::setDrawPopupButton(bool b)
{
    mbPopupButton = b;
}

void ScDPFieldButton::setHasHiddenMember(bool b)
{
    mbHasHiddenMember = b;
}

void ScDPFieldButton::setPopupPressed(bool b)
{
    mbPopupPressed = b;
}

void ScDPFieldButton::setPopupLeft(bool b)
{
    mbPopupLeft = b;
}

void ScDPFieldButton::draw()
{
    const long nMargin = 2;
    bool bOldMapEnablaed = mpOutDev->IsMapModeEnabled();
    mpOutDev->EnableMapMode(false);

    if (mbBaseButton)
    {
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

        // Field name.
        // Get the font and size the same way as in scenario selection (lcl_DrawOneFrame in gridwin4.cxx)
        Font aTextFont( mpStyle->GetAppFont() );
        if ( mpDoc )
        {
            //  use ScPatternAttr::GetFont only for font size
            Font aAttrFont;
            static_cast<const ScPatternAttr&>(mpDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).
                GetFont( aAttrFont, SC_AUTOCOL_BLACK, mpOutDev, &maZoomY );
            aTextFont.SetSize( aAttrFont.GetSize() );
        }
        mpOutDev->SetFont(aTextFont);
        mpOutDev->SetTextColor(mpStyle->GetButtonTextColor());

        Point aTextPos = maPos;
        long nTHeight = mpOutDev->GetTextHeight();
        aTextPos.setX(maPos.getX() + nMargin);
        aTextPos.setY(maPos.getY() + (maSize.Height()-nTHeight)/2);

        mpOutDev->Push(PUSH_CLIPREGION);
        mpOutDev->IntersectClipRegion(aRect);
        mpOutDev->DrawText(aTextPos, maText);
        mpOutDev->Pop();
    }

    if (mbPopupButton)
        drawPopupButton();

    mpOutDev->EnableMapMode(bOldMapEnablaed);
}

void ScDPFieldButton::getPopupBoundingBox(Point& rPos, Size& rSize) const
{
    long nW = maSize.getWidth() / 2;
    long nH = maSize.getHeight();
    if (nW > 18)
        nW = 18;
    if (nH > 18)
        nH = 18;

    // #i114944# AutoFilter button is left-aligned in RTL.
    // DataPilot button is always right-aligned for now, so text output isn't affected.
    if (mbPopupLeft)
        rPos.setX(maPos.getX());
    else
        rPos.setX(maPos.getX() + maSize.getWidth() - nW);
    rPos.setY(maPos.getY() + maSize.getHeight() - nH);
    rSize.setWidth(nW);
    rSize.setHeight(nH);
}

void ScDPFieldButton::drawPopupButton()
{
    Point aPos;
    Size aSize;
    getPopupBoundingBox(aPos, aSize);

    // Background & outer black border
    mpOutDev->SetLineColor(COL_BLACK);
    mpOutDev->SetFillColor(mpStyle->GetFaceColor());
    mpOutDev->DrawRect(Rectangle(aPos, aSize));

    if (!mbPopupPressed)
    {
        // border lines
        mpOutDev->SetLineColor(mpStyle->GetLightColor());
        mpOutDev->DrawLine(Point(aPos.X()+1, aPos.Y()+1), Point(aPos.X()+1, aPos.Y()+aSize.Height()-2));
        mpOutDev->DrawLine(Point(aPos.X()+1, aPos.Y()+1), Point(aPos.X()+aSize.Width()-2, aPos.Y()+1));

        mpOutDev->SetLineColor(mpStyle->GetShadowColor());
        mpOutDev->DrawLine(Point(aPos.X()+1, aPos.Y()+aSize.Height()-2),
                           Point(aPos.X()+aSize.Width()-2, aPos.Y()+aSize.Height()-2));
        mpOutDev->DrawLine(Point(aPos.X()+aSize.Width()-2, aPos.Y()+1),
                           Point(aPos.X()+aSize.Width()-2, aPos.Y()+aSize.Height()-2));
    }

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

    if (mbPopupPressed)
    {
        aPos1.X() += 1;
        aPos2.X() += 1;
        aPos1.Y() += 1;
        aPos2.Y() += 1;
    }

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
        if (mbPopupPressed)
        {
            aBoxPos.X() += 1;
            aBoxPos.Y() += 1;
        }
        Size aBoxSize(3, 3);
        mpOutDev->DrawRect(Rectangle(aBoxPos, aBoxSize));
    }
}

// ============================================================================

ScMenuFloatingWindow::MenuItemData::MenuItemData() :
    mbEnabled(true),
    mpAction(static_cast<ScDPFieldPopupWindow::Action*>(NULL)),
    mpSubMenuWin(static_cast<ScMenuFloatingWindow*>(NULL))
{
}

// ----------------------------------------------------------------------------

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

IMPL_LINK( ScMenuFloatingWindow::SubMenuItemData, TimeoutHdl, void*, EMPTYARG )
{
    mpParent->handleMenuTimeout(this);
    return 0;
}

// ----------------------------------------------------------------------------

size_t ScMenuFloatingWindow::MENU_NOT_SELECTED = 999;

ScMenuFloatingWindow::ScMenuFloatingWindow(Window* pParent, ScDocument* pDoc, sal_uInt16 nMenuStackLevel) :
    PopupMenuFloatingWindow(pParent),
    maOpenTimer(this),
    maCloseTimer(this),
    maName(OUString::createFromAscii("ScMenuFloatingWindow")),
    mnSelectedMenu(MENU_NOT_SELECTED),
    mnClickedMenu(MENU_NOT_SELECTED),
    mpDoc(pDoc),
    mpParentMenu(dynamic_cast<ScMenuFloatingWindow*>(pParent)),
    mpActiveSubMenu(NULL)
{
    SetMenuStackLevel(nMenuStackLevel);

    // TODO: How do we get the right font to use here ?
    const sal_uInt16 nPopupFontHeight = 12;
    const StyleSettings& rStyle = GetSettings().GetStyleSettings();
    maLabelFont = rStyle.GetLabelFont();
    maLabelFont.SetHeight(nPopupFontHeight);
    SetFont(maLabelFont);

    SetText(OUString::createFromAscii("ScMenuFloatingWindow"));
    SetPopupModeEndHdl( LINK(this, ScMenuFloatingWindow, PopupEndHdl) );
}

ScMenuFloatingWindow::~ScMenuFloatingWindow()
{
    EndPopupMode();
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
            setSelectedMenuItem(nSelectedMenu, false, false);
        break;
        case KEY_DOWN:
            if (nSelectedMenu == MENU_NOT_SELECTED || nSelectedMenu == nLastMenuPos)
                nSelectedMenu = 0;
            else
                ++nSelectedMenu;
            setSelectedMenuItem(nSelectedMenu, false, false);
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

        mxAccessible.set(new ScAccessibleFilterMenu(xAccParent, this, maName, 999, getDoc()));
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

size_t ScMenuFloatingWindow::getSelectedMenuItem() const
{
    return mnSelectedMenu;
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

ScDocument* ScMenuFloatingWindow::getDoc()
{
    return mpDoc;
}

void ScMenuFloatingWindow::resizeToFitMenuItems()
{
    if (maMenuItems.empty())
        return;

    vector<MenuItemData>::const_iterator itr = maMenuItems.begin(), itrEnd = maMenuItems.end();
    long nTextWidth = 0;
    for (; itr != itrEnd; ++itr)
        nTextWidth = ::std::max(GetTextWidth(itr->maText), nTextWidth);

    size_t nLastPos = maMenuItems.size()-1;
    Point aPos;
    Size aSize;
    getMenuItemPosSize(nLastPos, aPos, aSize);
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

const OUString& ScMenuFloatingWindow::getName() const
{
    return maName;
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
        Push(PUSH_CLIPREGION);
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
    const sal_uInt16 nLeftMargin = 5;
    const sal_uInt16 nTopMargin = 5;
    const sal_uInt16 nMenuItemHeight = static_cast< sal_uInt16 >( maLabelFont.GetHeight()*1.8 );

    Size aWndSize = GetSizePixel();

    Point aPos1(nLeftMargin, nTopMargin);
    Size aSize1(aWndSize.Width() - nLeftMargin*2, nMenuItemHeight);

    rPos = aPos1;
    rPos.Y() += aSize1.Height()*nPos;
    rSize = aSize1;
}

ScMenuFloatingWindow* ScMenuFloatingWindow::getParentMenuWindow() const
{
    return mpParentMenu;
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
            return i;
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

IMPL_LINK( ScMenuFloatingWindow, PopupEndHdl, void*, EMPTYARG )
{
    clearSelectedMenuItem();
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

ScDPFieldPopupWindow::ScDPFieldPopupWindow(Window* pParent, ScDocument* pDoc) :
    ScMenuFloatingWindow(pParent, pDoc),
    maChecks(this, 0),
    maChkToggleAll(this, 0),
    maBtnSelectSingle  (this, 0),
    maBtnUnselectSingle(this, 0),
    maBtnOk(this),
    maBtnCancel(this),
    mnCurTabStop(0),
    mpExtendedData(NULL),
    mpOKAction(NULL),
    maWndSize(240, 330),
    mePrevToggleAllState(STATE_DONTKNOW)
{
    maTabStopCtrls.reserve(7);
    maTabStopCtrls.push_back(this);
    maTabStopCtrls.push_back(&maChecks);
    maTabStopCtrls.push_back(&maChkToggleAll);
    maTabStopCtrls.push_back(&maBtnSelectSingle);
    maTabStopCtrls.push_back(&maBtnUnselectSingle);
    maTabStopCtrls.push_back(&maBtnOk);
    maTabStopCtrls.push_back(&maBtnCancel);

    const StyleSettings& rStyle = GetSettings().GetStyleSettings();

    Point aPos;
    Size aSize;
    getSectionPosSize(aPos, aSize, WHOLE);
    SetOutputSizePixel(aSize);
    Size aOutSize = GetOutputSizePixel();

    getSectionPosSize(aPos, aSize, BTN_OK);
    maBtnOk.SetPosSizePixel(aPos, aSize);
    maBtnOk.SetFont(getLabelFont());
    maBtnOk.SetClickHdl( LINK(this, ScDPFieldPopupWindow, ButtonHdl) );
    maBtnOk.Show();

    getSectionPosSize(aPos, aSize, BTN_CANCEL);
    maBtnCancel.SetPosSizePixel(aPos, aSize);
    maBtnCancel.SetFont(getLabelFont());
    maBtnCancel.Show();

    getSectionPosSize(aPos, aSize, LISTBOX_AREA_INNER);
    maChecks.SetPosSizePixel(aPos, aSize);
    maChecks.SetFont(getLabelFont());
    maChecks.SetCheckButtonHdl( LINK(this, ScDPFieldPopupWindow, CheckHdl) );
    maChecks.Show();

    getSectionPosSize(aPos, aSize, CHECK_TOGGLE_ALL);
    maChkToggleAll.SetPosSizePixel(aPos, aSize);
    maChkToggleAll.SetFont(getLabelFont());
    maChkToggleAll.SetText(ScRscStrLoader(RID_POPUP_FILTER, STR_BTN_TOGGLE_ALL).GetString());
    maChkToggleAll.SetControlBackground(rStyle.GetMenuColor());
    maChkToggleAll.SetClickHdl( LINK(this, ScDPFieldPopupWindow, TriStateHdl) );
    maChkToggleAll.Show();

    getSectionPosSize(aPos, aSize, BTN_SINGLE_SELECT);
    maBtnSelectSingle.SetPosSizePixel(aPos, aSize);
    maBtnSelectSingle.SetQuickHelpText(ScRscStrLoader(RID_POPUP_FILTER, STR_BTN_SELECT_CURRENT).GetString());
    maBtnSelectSingle.SetModeImage(Image(ScResId(RID_IMG_SELECT_CURRENT)), BMP_COLOR_NORMAL);
    maBtnSelectSingle.SetClickHdl( LINK(this, ScDPFieldPopupWindow, ButtonHdl) );
    maBtnSelectSingle.Show();

    getSectionPosSize(aPos, aSize, BTN_SINGLE_UNSELECT);
    maBtnUnselectSingle.SetPosSizePixel(aPos, aSize);
    maBtnUnselectSingle.SetQuickHelpText(ScRscStrLoader(RID_POPUP_FILTER, STR_BTN_UNSELECT_CURRENT).GetString());
    maBtnUnselectSingle.SetModeImage(Image(ScResId(RID_IMG_UNSELECT_CURRENT)), BMP_COLOR_NORMAL);
    maBtnUnselectSingle.SetClickHdl( LINK(this, ScDPFieldPopupWindow, ButtonHdl) );
    maBtnUnselectSingle.Show();
}

ScDPFieldPopupWindow::~ScDPFieldPopupWindow()
{
}

void ScDPFieldPopupWindow::getSectionPosSize(Point& rPos, Size& rSize, SectionType eType) const
{
    // constant parameters.
    const sal_uInt16 nListBoxMargin = 5;            // horizontal distance from the side of the dialog to the listbox border.
    const sal_uInt16 nListBoxInnerPadding = 5;
    const sal_uInt16 nTopMargin = 5;
    const sal_uInt16 nMenuHeight = 60;
    const sal_uInt16 nSingleItemBtnAreaHeight = 32; // height of the middle area below the list box where the single-action buttons are.
    const sal_uInt16 nBottomBtnAreaHeight = 50;     // height of the bottom area where the OK and Cancel buttons are.
    const sal_uInt16 nBtnWidth = 90;
    const sal_uInt16 nLabelHeight = static_cast< sal_uInt16 >( getLabelFont().GetHeight() );
    const sal_uInt16 nBtnHeight = nLabelHeight*2;
    const sal_uInt16 nBottomMargin = 10;
    const sal_uInt16 nMenuListMargin = 20;

    // parameters calculated from constants.
    const sal_uInt16 nListBoxWidth = static_cast< sal_uInt16 >( maWndSize.Width() - nListBoxMargin*2 );
    const sal_uInt16 nListBoxHeight = static_cast< sal_uInt16 >( maWndSize.Height() - nTopMargin - nMenuHeight -
        nMenuListMargin - nSingleItemBtnAreaHeight - nBottomBtnAreaHeight );

    const sal_uInt16 nSingleBtnAreaY = nTopMargin + nMenuHeight + nListBoxHeight + nMenuListMargin - 1;

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
            long h = nLabelHeight*3/2; // check box height is heuristically 150% of the text height.
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
            rPos.X() += 150;
            rPos.Y() += (nSingleItemBtnAreaHeight - h)/2;
            rSize = Size(h, h);
        }
        break;
        case BTN_SINGLE_UNSELECT:
        {
            long h = 26;
            rPos = Point(nListBoxMargin, nSingleBtnAreaY);
            rPos.X() += 150 + h + 10;
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

void ScDPFieldPopupWindow::setAllMemberState(bool bSet)
{
    size_t n = maMembers.size();
    for (size_t i = 0; i < n; ++i)
        maChecks.CheckEntryPos(static_cast< sal_uInt16 >( i ), bSet);
}

void ScDPFieldPopupWindow::selectCurrentMemberOnly(bool bSet)
{
    setAllMemberState(!bSet);
    sal_uInt16 nSelected = maChecks.GetSelectEntryPos();
    maChecks.CheckEntryPos(nSelected, bSet);
}

void ScDPFieldPopupWindow::cycleFocus(bool bReverse)
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

IMPL_LINK( ScDPFieldPopupWindow, ButtonHdl, Button*, pBtn )
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

IMPL_LINK( ScDPFieldPopupWindow, TriStateHdl, TriStateBox*, EMPTYARG )
{
    switch (mePrevToggleAllState)
    {
        case STATE_NOCHECK:
            maChkToggleAll.SetState(STATE_CHECK);
            setAllMemberState(true);
        break;
        case STATE_CHECK:
            maChkToggleAll.SetState(STATE_NOCHECK);
            setAllMemberState(false);
        break;
        case STATE_DONTKNOW:
        default:
            maChkToggleAll.SetState(STATE_CHECK);
            setAllMemberState(true);
        break;
    }

    mePrevToggleAllState = maChkToggleAll.GetState();
    return 0;
}

IMPL_LINK( ScDPFieldPopupWindow, CheckHdl, SvTreeListBox*, pChecks )
{
    if (pChecks != &maChecks)
        return 0;

    size_t nNumChecked = maChecks.GetCheckedEntryCount();
    if (nNumChecked == maMembers.size())
        // all members visible
        maChkToggleAll.SetState(STATE_CHECK);
    else if (nNumChecked == 0)
        // no members visible
        maChkToggleAll.SetState(STATE_NOCHECK);
    else
        maChkToggleAll.SetState(STATE_DONTKNOW);

    mePrevToggleAllState = maChkToggleAll.GetState();
    return 0;
}

void ScDPFieldPopupWindow::MouseMove(const MouseEvent& rMEvt)
{
    ScMenuFloatingWindow::MouseMove(rMEvt);

    size_t nSelectedMenu = getSelectedMenuItem();
    if (nSelectedMenu == MENU_NOT_SELECTED)
        queueCloseSubMenu();
}

long ScDPFieldPopupWindow::Notify(NotifyEvent& rNEvt)
{
    switch (rNEvt.GetType())
    {
        case EVENT_KEYUP:
        {
            const KeyEvent* pKeyEvent = rNEvt.GetKeyEvent();
            const KeyCode& rCode = pKeyEvent->GetKeyCode();
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

void ScDPFieldPopupWindow::Paint(const Rectangle& rRect)
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

Window* ScDPFieldPopupWindow::GetPreferredKeyInputWindow()
{
    return maTabStopCtrls[mnCurTabStop];
}

Reference<XAccessible> ScDPFieldPopupWindow::CreateAccessible()
{
    if (!mxAccessible.is())
    {
        mxAccessible.set(new ScAccessibleFilterTopWindow(
            GetAccessibleParentWindow()->GetAccessible(), this, getName(), getDoc()));
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
    size_t n = maMembers.size();
    size_t nVisMemCount = 0;
    for (size_t i = 0; i < n; ++i)
    {
        maChecks.InsertEntry(maMembers[i].maName);
        maChecks.CheckEntryPos(static_cast< sal_uInt16 >( i ), maMembers[i].mbVisible);
        if (maMembers[i].mbVisible)
            ++nVisMemCount;
    }
    if (nVisMemCount == n)
    {
        // all members visible
        maChkToggleAll.SetState(STATE_CHECK);
        mePrevToggleAllState = STATE_CHECK;
    }
    else if (nVisMemCount == 0)
    {
        // no members visible
        maChkToggleAll.SetState(STATE_NOCHECK);
        mePrevToggleAllState = STATE_NOCHECK;
    }
    else
    {
        maChkToggleAll.SetState(STATE_DONTKNOW);
        mePrevToggleAllState = STATE_DONTKNOW;
    }
}

const Size& ScDPFieldPopupWindow::getWindowSize() const
{
    return maWndSize;
}

void ScDPFieldPopupWindow::getResult(hash_map<OUString, bool, OUStringHash>& rResult)
{
    typedef hash_map<OUString, bool, OUStringHash> ResultMap;
    ResultMap aResult;
    size_t n = maMembers.size();
    for (size_t i = 0; i < n; ++i)
    {
        bool bState = maChecks.IsChecked(static_cast< sal_uInt16 >( i ));
        aResult.insert(ResultMap::value_type(maMembers[i].maName, bState));
    }
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

