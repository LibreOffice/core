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

#include "menufloatingwindow.hxx"
#include "menuitemlist.hxx"
#include "menubarwindow.hxx"

#include <svdata.hxx>
#include <vcl/decoview.hxx>
#include <vcl/settings.hxx>
#include <window.h>

MenuFloatingWindow::MenuFloatingWindow( Menu* pMen, vcl::Window* pParent, WinBits nStyle ) :
    FloatingWindow( pParent, nStyle )
{
    mpWindowImpl->mbMenuFloatingWindow= true;
    pMenu               = pMen;
    pActivePopup        = nullptr;
    bInExecute          = false;
    bScrollMenu         = false;
    nHighlightedItem    = ITEMPOS_INVALID;
    nMBDownPos          = ITEMPOS_INVALID;
    nPosInParent        = ITEMPOS_INVALID;
    nScrollerHeight     = 0;
    nBorder             = EXTRASPACEY;
    nFirstEntry         = 0;
    bScrollUp           = false;
    bScrollDown         = false;
    bIgnoreFirstMove    = true;
    bKeyInput           = false;

    ApplySettings(*this);

    SetPopupModeEndHdl( LINK( this, MenuFloatingWindow, PopupEnd ) );

    aHighlightChangedTimer.SetTimeoutHdl( LINK( this, MenuFloatingWindow, HighlightChanged ) );
    aHighlightChangedTimer.SetTimeout( GetSettings().GetMouseSettings().GetMenuDelay() );
    aHighlightChangedTimer.SetDebugName( "vcl::MenuFloatingWindow aHighlightChangedTimer" );

    aSubmenuCloseTimer.SetTimeout( GetSettings().GetMouseSettings().GetMenuDelay() );
    aSubmenuCloseTimer.SetTimeoutHdl( LINK( this, MenuFloatingWindow, SubmenuClose ) );
    aSubmenuCloseTimer.SetDebugName( "vcl::MenuFloatingWindow aSubmenuCloseTimer" );

    aScrollTimer.SetTimeoutHdl( LINK( this, MenuFloatingWindow, AutoScroll ) );
    aScrollTimer.SetDebugName( "vcl::MenuFloatingWindow aScrollTimer" );

    AddEventListener( LINK( this, MenuFloatingWindow, ShowHideListener ) );
}

void MenuFloatingWindow::doShutdown()
{
    if( pMenu )
    {
        // #105373# notify toolkit that highlight was removed
        // otherwise the entry will not be read when the menu is opened again
        if( nHighlightedItem != ITEMPOS_INVALID )
            pMenu->ImplCallEventListeners( VCLEVENT_MENU_DEHIGHLIGHT, nHighlightedItem );
        if (!bKeyInput && pMenu && pMenu->pStartedFrom && !pMenu->pStartedFrom->IsMenuBar())
        {
            // #102461# remove highlight in parent
            size_t i, nCount = pMenu->pStartedFrom->pItemList->size();
            for(i = 0; i < nCount; i++)
            {
                MenuItemData* pData = pMenu->pStartedFrom->pItemList->GetDataFromPos( i );
                if( pData && ( pData->pSubMenu == pMenu ) )
                    break;
            }
            if( i < nCount )
            {
                MenuFloatingWindow* pPWin = static_cast<MenuFloatingWindow*>(pMenu->pStartedFrom->ImplGetWindow());
                if (pPWin)
                    pPWin->InvalidateItem(i);
            }
        }

        // free the reference to the accessible component
        SetAccessible( css::uno::Reference< css::accessibility::XAccessible >() );

        aHighlightChangedTimer.Stop();

        // #95056# invalidate screen area covered by system window
        // so this can be taken into account if the commandhandler performs a scroll operation
        if( GetParent() )
        {
            Rectangle aInvRect( GetWindowExtentsRelative( GetParent() ) );
            GetParent()->Invalidate( aInvRect );
        }
        pMenu = nullptr;
        RemoveEventListener( LINK( this, MenuFloatingWindow, ShowHideListener ) );

        aScrollTimer.Stop();
        aSubmenuCloseTimer.Stop();
        aSubmenuCloseTimer.Stop();
        aHighlightChangedTimer.Stop();
        aHighlightChangedTimer.Stop();
    }
}

MenuFloatingWindow::~MenuFloatingWindow()
{
    disposeOnce();
}

void MenuFloatingWindow::dispose()
{
    doShutdown();
    pMenu.clear();
    pActivePopup.clear();
    xSaveFocusId.clear();
    FloatingWindow::dispose();
}

void MenuFloatingWindow::Resize()
{
    InitMenuClipRegion(*this); // FIXME
}

void MenuFloatingWindow::ApplySettings(vcl::RenderContext& rRenderContext)
{
    FloatingWindow::ApplySettings(rRenderContext);

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();

    SetPointFont(rRenderContext, rStyleSettings.GetMenuFont());

    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::Entire))
    {
        rRenderContext.SetBackground(); // background will be drawn by NWF
    }
    else
        rRenderContext.SetBackground(Wallpaper(rStyleSettings.GetMenuColor()));

    rRenderContext.SetTextColor(rStyleSettings.GetMenuTextColor());
    rRenderContext.SetTextFillColor();
    rRenderContext.SetLineColor();
}

/// Get a negative pixel offset for an offset menu
long MenuFloatingWindow::ImplGetStartY() const
{
    long nY = 0;
    if( pMenu )
    {
        for ( sal_uInt16 n = 0; n < nFirstEntry; n++ )
            nY += pMenu->GetItemList()->GetDataFromPos( n )->aSz.Height();
        nY -= pMenu->GetTitleHeight();
    }
    return -nY;
}

vcl::Region MenuFloatingWindow::ImplCalcClipRegion( bool bIncludeLogo ) const
{
    Size aOutSz = GetOutputSizePixel();
    Point aPos;
    Rectangle aRect( aPos, aOutSz );
    aRect.Top() += nScrollerHeight;
    aRect.Bottom() -= nScrollerHeight;

    if ( pMenu && pMenu->pLogo && !bIncludeLogo )
        aRect.Left() += pMenu->pLogo->aBitmap.GetSizePixel().Width();

    vcl::Region aRegion(aRect);
    if ( pMenu && pMenu->pLogo && bIncludeLogo && nScrollerHeight )
        aRegion.Union( Rectangle( Point(), Size( pMenu->pLogo->aBitmap.GetSizePixel().Width(), aOutSz.Height() ) ) );

    return aRegion;
}

void MenuFloatingWindow::InitMenuClipRegion(vcl::RenderContext& rRenderContext)
{
    if (IsScrollMenu())
    {
        rRenderContext.SetClipRegion(ImplCalcClipRegion());
    }
    else
    {
        rRenderContext.SetClipRegion();
    }
}

void MenuFloatingWindow::ImplHighlightItem( const MouseEvent& rMEvt, bool bMBDown )
{
    if( ! pMenu )
        return;

    long nY = GetInitialItemY();
    long nMouseY = rMEvt.GetPosPixel().Y();
    Size aOutSz = GetOutputSizePixel();
    if ( ( nMouseY >= nY ) && ( nMouseY < ( aOutSz.Height() - nY ) ) )
    {
        bool bHighlighted = false;
        size_t nCount = pMenu->pItemList->size();
        for ( size_t n = 0; !bHighlighted && ( n < nCount ); n++ )
        {
            if ( pMenu->ImplIsVisible( n ) )
            {
                MenuItemData* pItemData = pMenu->pItemList->GetDataFromPos( n );
                long nOldY = nY;
                nY += pItemData->aSz.Height();
                if ( ( nOldY <= nMouseY ) && ( nY > nMouseY ) && pMenu->ImplIsSelectable( n ) )
                {
                    bool bPopupArea = true;
                    if ( pItemData->nBits & MenuItemBits::POPUPSELECT )
                    {
                        // only when clicked over the arrow...
                        Size aSz = GetOutputSizePixel();
                        long nFontHeight = GetTextHeight();
                        bPopupArea = ( rMEvt.GetPosPixel().X() >= ( aSz.Width() - nFontHeight - nFontHeight/4 ) );
                    }

                    if ( bMBDown )
                    {
                        if ( n != nHighlightedItem )
                        {
                            ChangeHighlightItem( (sal_uInt16)n, false );
                        }

                        bool bAllowNewPopup = true;
                        if ( pActivePopup )
                        {
                            MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
                            bAllowNewPopup = pData && ( pData->pSubMenu != pActivePopup );
                            if ( bAllowNewPopup )
                                KillActivePopup();
                        }

                        if ( bPopupArea && bAllowNewPopup )
                        {
                            HighlightChanged( nullptr );
                        }
                    }
                    else
                    {
                        if ( n != nHighlightedItem )
                        {
                            ChangeHighlightItem( (sal_uInt16)n, true );
                        }
                        else if ( pItemData->nBits & MenuItemBits::POPUPSELECT )
                        {
                            if ( bPopupArea && ( pActivePopup != pItemData->pSubMenu ) )
                                HighlightChanged( nullptr );
                        }
                    }
                    bHighlighted = true;
                }
            }
        }
        if ( !bHighlighted )
            ChangeHighlightItem( ITEMPOS_INVALID, true );
    }
    else
    {
        ImplScroll( rMEvt.GetPosPixel() );
        ChangeHighlightItem( ITEMPOS_INVALID, true );
    }
}

IMPL_LINK_NOARG_TYPED(MenuFloatingWindow, PopupEnd, FloatingWindow*, void)
{
    // "this" will be deleted before the end of this method!
    Menu* pM = pMenu;
    if ( bInExecute )
    {
        if ( pActivePopup )
        {
            //SAL_WARN_IF( pActivePopup->ImplGetWindow(), "vcl", "PopupEnd, obwohl pActivePopup MIT Window!" );
            KillActivePopup(); // should be ok to just remove it
            //pActivePopup->bCanceled = true;
        }
        bInExecute = false;
        pMenu->bInCallback = true;
        pMenu->Deactivate();
        pMenu->bInCallback = false;
    }
    else
    {
        if (pMenu && pMenu->pStartedFrom)
            pMenu->pStartedFrom->ClosePopup(pMenu);
    }

    if ( pM )
        pM->pStartedFrom = nullptr;
}

IMPL_LINK_NOARG_TYPED(MenuFloatingWindow, AutoScroll, Timer *, void)
{
    ImplScroll( GetPointerPosPixel() );
}

IMPL_LINK_TYPED( MenuFloatingWindow, HighlightChanged, Timer*, pTimer, void )
{
    if( ! pMenu )
        return;

    MenuItemData* pItemData = pMenu->pItemList->GetDataFromPos( nHighlightedItem );
    if ( pItemData )
    {
        if ( pActivePopup && ( pActivePopup != pItemData->pSubMenu ) )
        {
            FloatWinPopupFlags nOldFlags = GetPopupModeFlags();
            SetPopupModeFlags( GetPopupModeFlags() | FloatWinPopupFlags::NoAppFocusClose );
            KillActivePopup();
            SetPopupModeFlags( nOldFlags );
        }
        if ( pItemData->bEnabled && pItemData->pSubMenu && pItemData->pSubMenu->GetItemCount() && ( pItemData->pSubMenu != pActivePopup ) )
        {
            pActivePopup = static_cast<PopupMenu*>(pItemData->pSubMenu.get());
            long nY = nScrollerHeight+ImplGetStartY();
            MenuItemData* pData = nullptr;
            for ( sal_uLong n = 0; n < nHighlightedItem; n++ )
            {
                pData = pMenu->pItemList->GetDataFromPos( n );
                nY += pData->aSz.Height();
            }
            pData = pMenu->pItemList->GetDataFromPos( nHighlightedItem );
            Size MySize = GetOutputSizePixel();
            Point aItemTopLeft( 0, nY );
            Point aItemBottomRight( aItemTopLeft );
            aItemBottomRight.X() += MySize.Width();
            aItemBottomRight.Y() += pData->aSz.Height();

            // shift the popups a little
            aItemTopLeft.X() += 2;
            aItemBottomRight.X() -= 2;
            if ( nHighlightedItem )
                aItemTopLeft.Y() -= 2;
            else
            {
                sal_Int32 nL, nT, nR, nB;
                GetBorder( nL, nT, nR, nB );
                aItemTopLeft.Y() -= nT;
            }

            // pTest: crash due to Reschedule() in call of Activate()
            // Also it is prevented that submenus are displayed which
            // were for long in Activate Rescheduled and which should not be
            // displayed now.
            Menu* pTest = pActivePopup;
            FloatWinPopupFlags nOldFlags = GetPopupModeFlags();
            SetPopupModeFlags( GetPopupModeFlags() | FloatWinPopupFlags::NoAppFocusClose );
            sal_uInt16 nRet = pActivePopup->ImplExecute( this, Rectangle( aItemTopLeft, aItemBottomRight ), FloatWinPopupFlags::Right, pMenu, pTimer == nullptr );
            SetPopupModeFlags( nOldFlags );

            // nRet != 0, wenn es waerend Activate() abgeschossen wurde...
            if ( !nRet && ( pActivePopup == pTest ) && pActivePopup->ImplGetWindow() )
                pActivePopup->ImplGetFloatingWindow()->AddPopupModeWindow( this );
        }
    }
}

IMPL_LINK_NOARG_TYPED(MenuFloatingWindow, SubmenuClose, Timer *, void)
{
    if( pMenu && pMenu->pStartedFrom )
    {
        MenuFloatingWindow* pWin = static_cast<MenuFloatingWindow*>(pMenu->pStartedFrom->GetWindow());
        if( pWin )
            pWin->KillActivePopup();
    }
}

IMPL_LINK_TYPED( MenuFloatingWindow, ShowHideListener, VclWindowEvent&, rEvent, void )
{
    if( ! pMenu )
        return;

    if( rEvent.GetId() == VCLEVENT_WINDOW_SHOW )
        pMenu->ImplCallEventListeners( VCLEVENT_MENU_SHOW, ITEMPOS_INVALID );
    else if( rEvent.GetId() == VCLEVENT_WINDOW_HIDE )
        pMenu->ImplCallEventListeners( VCLEVENT_MENU_HIDE, ITEMPOS_INVALID );
}

void MenuFloatingWindow::EnableScrollMenu( bool b )
{
    bScrollMenu = b;
    nScrollerHeight = b ? (sal_uInt16) GetSettings().GetStyleSettings().GetScrollBarSize() /2 : 0;
    bScrollDown = true;
    InitMenuClipRegion(*this);
}

void MenuFloatingWindow::Execute()
{
    ImplSVData* pSVData = ImplGetSVData();

    pSVData->maAppData.mpActivePopupMenu = static_cast<PopupMenu*>(pMenu.get());

    bInExecute = true;
//  bCallingSelect = false;

    while ( bInExecute )
        Application::Yield();

    pSVData->maAppData.mpActivePopupMenu = nullptr;
}

void MenuFloatingWindow::StopExecute( VclPtr<vcl::Window> xFocusId )
{
    // restore focus
    // (could have been restored in Select)
    if ( xSaveFocusId != nullptr )
    {
        xFocusId = xSaveFocusId;
        if ( xFocusId != nullptr )
        {
            xSaveFocusId = nullptr;
            ImplGetSVData()->maWinData.mbNoDeactivate = false;
        }
    }
    ImplEndPopupMode( FloatWinPopupEndFlags::NONE, xFocusId );

    aHighlightChangedTimer.Stop();
    bInExecute = false;
    if ( pActivePopup )
    {
        KillActivePopup();
    }
    // notify parent, needed for accessibility
    if( pMenu && pMenu->pStartedFrom )
        pMenu->pStartedFrom->ImplCallEventListeners( VCLEVENT_MENU_SUBMENUDEACTIVATE, nPosInParent );
}

void MenuFloatingWindow::KillActivePopup( PopupMenu* pThisOnly )
{
    if ( pActivePopup && ( !pThisOnly || ( pThisOnly == pActivePopup ) ) )
    {
        if( pActivePopup->pWindow )
            if( static_cast<FloatingWindow *>(pActivePopup->pWindow.get())->IsInCleanUp() )
                return; // kill it later
        if ( pActivePopup->bInCallback )
            pActivePopup->bCanceled = true;

        // For all actions pActivePopup = 0, if e.g.
        // PopupModeEndHdl the popups to destroy were called synchronous
        PopupMenu* pPopup = pActivePopup;
        pActivePopup = nullptr;
        pPopup->bInCallback = true;
        pPopup->Deactivate();
        pPopup->bInCallback = false;
        if ( pPopup->ImplGetWindow() )
        {
            pPopup->ImplGetFloatingWindow()->StopExecute();
            pPopup->ImplGetFloatingWindow()->doShutdown();
            pPopup->pWindow->doLazyDelete();
            pPopup->pWindow = nullptr;

            Update();
        }
    }
}

void MenuFloatingWindow::EndExecute()
{
    Menu* pStart = pMenu ? pMenu->ImplGetStartMenu() : nullptr;
    VclPtr<vcl::Window> xFocusId;

    // if started elsewhere, cleanup there as well
    MenuFloatingWindow* pCleanUpFrom = this;
    MenuFloatingWindow* pWin = this;
    while (pWin && !pWin->bInExecute &&
        pWin->pMenu->pStartedFrom && !pWin->pMenu->pStartedFrom->IsMenuBar())
    {
        pWin = static_cast<PopupMenu*>(pWin->pMenu->pStartedFrom.get())->ImplGetFloatingWindow();
    }
    if ( pWin )
        pCleanUpFrom = pWin;

    // this window will be destroyed => store date locally...
    Menu* pM = pMenu;
    sal_uInt16 nItem = nHighlightedItem;

    pCleanUpFrom->StopExecute( xFocusId );

    if ( nItem != ITEMPOS_INVALID && pM )
    {
        MenuItemData* pItemData = pM->GetItemList()->GetDataFromPos( nItem );
        if ( pItemData && !pItemData->bIsTemporary )
        {
            pM->nSelectedId = pItemData->nId;
            if ( pStart )
                pStart->nSelectedId = pItemData->nId;

            pM->ImplSelect();
        }
    }
}

void MenuFloatingWindow::EndExecute( sal_uInt16 nId )
{
    size_t nPos;
    if ( pMenu && pMenu->GetItemList()->GetData( nId, nPos ) )
        nHighlightedItem = nPos;
    else
        nHighlightedItem = ITEMPOS_INVALID;

    EndExecute();
}

void MenuFloatingWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    // TH creates a ToTop on this window, but the active popup
    // should stay on top...
    // due to focus change this would close all menus -> don't do it (#94123)
    //if ( pActivePopup && pActivePopup->ImplGetWindow() && !pActivePopup->ImplGetFloatingWindow()->pActivePopup )
    //    pActivePopup->ImplGetFloatingWindow()->ToTop( ToTopFlags::NoGrabFocus );

    ImplHighlightItem( rMEvt, true );

    nMBDownPos = nHighlightedItem;
}

void MenuFloatingWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    MenuItemData* pData = pMenu ? pMenu->GetItemList()->GetDataFromPos( nHighlightedItem ) : nullptr;
    // nMBDownPos store in local variable and reset immediately,
    // as it will be too late after EndExecute
    sal_uInt16 _nMBDownPos = nMBDownPos;
    nMBDownPos = ITEMPOS_INVALID;
    if ( pData && pData->bEnabled && ( pData->eType != MenuItemType::SEPARATOR ) )
    {
        if ( !pData->pSubMenu )
        {
            EndExecute();
        }
        else if ( ( pData->nBits & MenuItemBits::POPUPSELECT ) && ( nHighlightedItem == _nMBDownPos ) && ( rMEvt.GetClicks() == 2 ) )
        {
            // not when clicked over the arrow...
            Size aSz = GetOutputSizePixel();
            long nFontHeight = GetTextHeight();
            if ( rMEvt.GetPosPixel().X() < ( aSz.Width() - nFontHeight - nFontHeight/4 ) )
                EndExecute();
        }
    }

}

void MenuFloatingWindow::MouseMove( const MouseEvent& rMEvt )
{
    if ( !IsVisible() || rMEvt.IsSynthetic() || rMEvt.IsEnterWindow() )
        return;

    if ( rMEvt.IsLeaveWindow() )
    {
        // #102461# do not remove highlight if a popup menu is open at this position
        MenuItemData* pData = pMenu ? pMenu->pItemList->GetDataFromPos( nHighlightedItem ) : nullptr;
        // close popup with some delayed if we leave somewhere else
        if( pActivePopup && pData && pData->pSubMenu != pActivePopup )
            pActivePopup->ImplGetFloatingWindow()->aSubmenuCloseTimer.Start();

        if( !pActivePopup || (pData && pData->pSubMenu != pActivePopup ) )
            ChangeHighlightItem( ITEMPOS_INVALID, false );

        if ( IsScrollMenu() )
            ImplScroll( rMEvt.GetPosPixel() );
    }
    else
    {
        aSubmenuCloseTimer.Stop();
        if( bIgnoreFirstMove )
            bIgnoreFirstMove = false;
        else
            ImplHighlightItem( rMEvt, false );
    }
}

void MenuFloatingWindow::ImplScroll( bool bUp )
{
    KillActivePopup();
    Update();

    if (!pMenu)
        return;

    Invalidate();

    pMenu->ImplKillLayoutData();

    if ( bScrollUp && bUp )
    {
        nFirstEntry = pMenu->ImplGetPrevVisible( nFirstEntry );
        SAL_WARN_IF( nFirstEntry == ITEMPOS_INVALID, "vcl", "Scroll?!" );

        long nScrollEntryHeight = pMenu->GetItemList()->GetDataFromPos( nFirstEntry )->aSz.Height();

        if ( !bScrollDown )
        {
            bScrollDown = true;
            Invalidate();
        }

        if ( pMenu->ImplGetPrevVisible( nFirstEntry ) == ITEMPOS_INVALID )
        {
            bScrollUp = false;
            Invalidate();
        }

        Scroll( 0, nScrollEntryHeight, ImplCalcClipRegion( false ).GetBoundRect(), ScrollFlags::Clip );
    }
    else if ( bScrollDown && !bUp )
    {
        long nScrollEntryHeight = pMenu->GetItemList()->GetDataFromPos( nFirstEntry )->aSz.Height();

        nFirstEntry = pMenu->ImplGetNextVisible( nFirstEntry );
        SAL_WARN_IF( nFirstEntry == ITEMPOS_INVALID, "vcl", "Scroll?!" );

        if ( !bScrollUp )
        {
            bScrollUp = true;
            Invalidate();
        }

        long nHeight = GetOutputSizePixel().Height();
        sal_uInt16 nLastVisible;
        static_cast<PopupMenu*>(pMenu.get())->ImplCalcVisEntries( nHeight, nFirstEntry, &nLastVisible );
        if ( pMenu->ImplGetNextVisible( nLastVisible ) == ITEMPOS_INVALID )
        {
            bScrollDown = false;
            Invalidate();
        }

        Scroll( 0, -nScrollEntryHeight, ImplCalcClipRegion( false ).GetBoundRect(), ScrollFlags::Clip );
    }

    Invalidate();
}

void MenuFloatingWindow::ImplScroll( const Point& rMousePos )
{
    Size aOutSz = GetOutputSizePixel();

    long nY = nScrollerHeight;
    long nMouseY = rMousePos.Y();
    long nDelta = 0;

    if ( bScrollUp && ( nMouseY < nY ) )
    {
        ImplScroll( true );
        nDelta = nY - nMouseY;
    }
    else if ( bScrollDown && ( nMouseY > ( aOutSz.Height() - nY ) ) )
    {
        ImplScroll( false );
        nDelta = nMouseY - ( aOutSz.Height() - nY );
    }

    if ( nDelta )
    {
        aScrollTimer.Stop();    // if scrolled through MouseMove.
        long nTimeout;
        if ( nDelta < 3 )
            nTimeout = 200;
        else if ( nDelta < 5 )
            nTimeout = 100;
        else if ( nDelta < 8 )
            nTimeout = 70;
        else if ( nDelta < 12 )
            nTimeout = 40;
        else
            nTimeout = 20;
        aScrollTimer.SetTimeout( nTimeout );
        aScrollTimer.Start();
    }
}
void MenuFloatingWindow::ChangeHighlightItem( sal_uInt16 n, bool bStartPopupTimer )
{
    // #57934# ggf. immediately close the active, as TH's backgroundstorage works.
    // #65750# we prefer to refrain from the background storage of small lines.
    //         otherwise the menus are difficult to operate.
    //  MenuItemData* pNextData = pMenu->pItemList->GetDataFromPos( n );
    //  if ( pActivePopup && pNextData && ( pActivePopup != pNextData->pSubMenu ) )
    //      KillActivePopup();

    aSubmenuCloseTimer.Stop();
    if( ! pMenu )
        return;

    if ( nHighlightedItem != ITEMPOS_INVALID )
    {
        InvalidateItem(nHighlightedItem);
        pMenu->ImplCallEventListeners( VCLEVENT_MENU_DEHIGHLIGHT, nHighlightedItem );
    }

    nHighlightedItem = (sal_uInt16)n;
    SAL_WARN_IF( !pMenu->ImplIsVisible( nHighlightedItem ) && nHighlightedItem != ITEMPOS_INVALID, "vcl", "ChangeHighlightItem: Not visible!" );
    if( nHighlightedItem != ITEMPOS_INVALID )
    {
        if (pMenu->pStartedFrom && !pMenu->pStartedFrom->IsMenuBar())
        {
            // #102461# make sure parent entry is highlighted as well
            size_t i, nCount = pMenu->pStartedFrom->pItemList->size();
            for(i = 0; i < nCount; i++)
            {
                MenuItemData* pData = pMenu->pStartedFrom->pItemList->GetDataFromPos( i );
                if( pData && ( pData->pSubMenu == pMenu ) )
                    break;
            }
            if( i < nCount )
            {
                MenuFloatingWindow* pPWin = static_cast<MenuFloatingWindow*>(pMenu->pStartedFrom->ImplGetWindow());
                if( pPWin && pPWin->nHighlightedItem != i )
                {
                    pPWin->InvalidateItem(i);
                    pPWin->nHighlightedItem = i;
                }
            }
        }
        InvalidateItem(nHighlightedItem);
        pMenu->ImplCallHighlight( nHighlightedItem );
    }
    else
        pMenu->nSelectedId = 0;

    if ( bStartPopupTimer )
    {
        // #102438# Menu items are not selectable
        // If a menu item is selected by an AT-tool via the XAccessibleAction, XAccessibleValue
        // or XAccessibleSelection interface, and the parent popup menus are not executed yet,
        // the parent popup menus must be executed SYNCHRONOUSLY, before the menu item is selected.
        if ( GetSettings().GetMouseSettings().GetMenuDelay() )
            aHighlightChangedTimer.Start();
        else
            HighlightChanged( &aHighlightChangedTimer );
    }
}

/// Calculate the initial vertical pixel offset of the first item.
/// May be negative for scrolled windows.
long MenuFloatingWindow::GetInitialItemY(long *pStartY) const
{
    long nStartY = ImplGetStartY();
    if (pStartY)
        *pStartY = nStartY;
    return nScrollerHeight + nStartY +
        ImplGetSVData()->maNWFData.mnMenuFormatBorderY;
}

/// Emit an Invalidate just for this item's area
void MenuFloatingWindow::InvalidateItem(sal_uInt16 nPos)
{
    if (!pMenu)
        return;

    long nY = GetInitialItemY();
    size_t nCount = pMenu->pItemList->size();
    for (size_t n = 0; n < nCount; n++)
    {
        MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
        long nHeight = pData->aSz.Height();
        if (n == nPos)
        {
            Size aWidth( GetSizePixel() );
            Rectangle aRect(Point(0, nY), Size(aWidth.Width(), nHeight));
            Invalidate( aRect );
        }
        nY += nHeight;
    }
}

void MenuFloatingWindow::RenderHighlightItem(vcl::RenderContext& rRenderContext, sal_uInt16 nPos)
{
    if (!pMenu)
        return;

    Size aSz(GetOutputSizePixel());

    long nX = 0;
    long nStartY;
    long nY = GetInitialItemY(&nStartY);

    if (pMenu->pLogo)
        nX = pMenu->pLogo->aBitmap.GetSizePixel().Width();

    int nOuterSpaceX = ImplGetSVData()->maNWFData.mnMenuFormatBorderX;

    size_t nCount = pMenu->pItemList->size();
    for (size_t n = 0; n < nCount; n++)
    {
        MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
        if (n == nPos)
        {
            SAL_WARN_IF(!pMenu->ImplIsVisible(n), "vcl", "Highlight: Item not visible!");
            if (pData->eType != MenuItemType::SEPARATOR)
            {
                bool bRestoreLineColor = false;
                Color oldLineColor;
                bool bDrawItemRect = true;

                Rectangle aItemRect(Point(nX + nOuterSpaceX, nY), Size(aSz.Width() - 2 * nOuterSpaceX, pData->aSz.Height()));
                if (pData->nBits & MenuItemBits::POPUPSELECT)
                {
                    long nFontHeight = GetTextHeight();
                    aItemRect.Right() -= nFontHeight + nFontHeight / 4;
                }

                if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::Entire))
                {
                    Size aPxSize(GetOutputSizePixel());
                    rRenderContext.Push(PushFlags::CLIPREGION);
                    rRenderContext.IntersectClipRegion(Rectangle(Point(nX, nY), Size(aSz.Width(), pData->aSz.Height())));
                    Rectangle aCtrlRect(Point(nX, 0), Size(aPxSize.Width()-nX, aPxSize.Height()));
                    MenupopupValue aVal(pMenu->nTextPos-GUTTERBORDER, aItemRect);
                    rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::Entire,
                                                     aCtrlRect, ControlState::ENABLED, aVal, OUString());
                    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::MenuItem))
                    {
                        bDrawItemRect = false;
                        if (!rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::MenuItem, aItemRect,
                                                              ControlState::SELECTED | (pData->bEnabled
                                                                                            ? ControlState::ENABLED
                                                                                            : ControlState::NONE),
                                                              aVal, OUString()))
                        {
                            bDrawItemRect = true;
                        }
                    }
                    else
                        bDrawItemRect = true;
                    rRenderContext.Pop();
                }
                if (bDrawItemRect)
                {
                    if (pData->bEnabled)
                        rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetMenuHighlightColor());
                    else
                    {
                        rRenderContext.SetFillColor();
                        oldLineColor = rRenderContext.GetLineColor();
                        rRenderContext.SetLineColor(rRenderContext.GetSettings().GetStyleSettings().GetMenuHighlightColor());
                        bRestoreLineColor = true;
                    }

                    rRenderContext.DrawRect(aItemRect);
                }
                pMenu->ImplPaint(rRenderContext, nScrollerHeight, nStartY, pData, true/*bHighlight*/);
                if (bRestoreLineColor)
                    rRenderContext.SetLineColor(oldLineColor);
            }
            return;
        }

        nY += pData->aSz.Height();
    }
}

Rectangle MenuFloatingWindow::ImplGetItemRect( sal_uInt16 nPos )
{
    if( ! pMenu )
        return Rectangle();

    Rectangle aRect;
    Size    aSz = GetOutputSizePixel();
    long    nStartY = ImplGetStartY();
    long    nY = nScrollerHeight+nStartY;
    long    nX = 0;

    if ( pMenu->pLogo )
        nX = pMenu->pLogo->aBitmap.GetSizePixel().Width();

    size_t nCount = pMenu->pItemList->size();
    for ( size_t n = 0; n < nCount; n++ )
    {
        MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
        if ( n == nPos )
        {
            SAL_WARN_IF( !pMenu->ImplIsVisible( n ), "vcl", "ImplGetItemRect: Item not visible!" );
            if ( pData->eType != MenuItemType::SEPARATOR )
            {
                aRect = Rectangle( Point( nX, nY ), Size( aSz.Width(), pData->aSz.Height() ) );
                if ( pData->nBits & MenuItemBits::POPUPSELECT )
                {
                    long nFontHeight = GetTextHeight();
                    aRect.Right() -= nFontHeight + nFontHeight/4;
                }
            }
            break;
        }
        nY += pData->aSz.Height();
    }
    return aRect;
}

void MenuFloatingWindow::ImplCursorUpDown( bool bUp, bool bHomeEnd )
{
    if( ! pMenu )
        return;

    const StyleSettings& rSettings = GetSettings().GetStyleSettings();

    sal_uInt16 n = nHighlightedItem;
    if ( n == ITEMPOS_INVALID )
    {
        if ( bUp )
            n = 0;
        else
            n = pMenu->GetItemCount()-1;
    }

    sal_uInt16 nLoop = n;

    if( bHomeEnd )
    {
        // absolute positioning
        if( bUp )
        {
            n = pMenu->GetItemCount();
            nLoop = n-1;
        }
        else
        {
            n = (sal_uInt16)-1;
            nLoop = n+1;
        }
    }

    do
    {
        if ( bUp )
        {
            if ( n )
                n--;
            else
                if ( !IsScrollMenu() || ( nHighlightedItem == ITEMPOS_INVALID ) )
                    n = pMenu->GetItemCount()-1;
                else
                    break;
        }
        else
        {
            n++;
            if ( n >= pMenu->GetItemCount() )
            {
                if ( !IsScrollMenu() || ( nHighlightedItem == ITEMPOS_INVALID ) )
                    n = 0;
                else
                    break;
            }
        }

        MenuItemData* pData = pMenu->GetItemList()->GetDataFromPos( n );
        if ( ( pData->bEnabled || !rSettings.GetSkipDisabledInMenus() )
              && ( pData->eType != MenuItemType::SEPARATOR ) && pMenu->ImplIsVisible( n ) && pMenu->ImplIsSelectable( n ) )
        {
            // Is selection in visible area?
            if ( IsScrollMenu() )
            {
                ChangeHighlightItem( ITEMPOS_INVALID, false );

                while ( n < nFirstEntry )
                    ImplScroll( true );

                Size aOutSz = GetOutputSizePixel();
                sal_uInt16 nLastVisible;
                static_cast<PopupMenu*>(pMenu.get())->ImplCalcVisEntries( aOutSz.Height(), nFirstEntry, &nLastVisible );
                while ( n > nLastVisible )
                {
                    ImplScroll( false );
                    static_cast<PopupMenu*>(pMenu.get())->ImplCalcVisEntries( aOutSz.Height(), nFirstEntry, &nLastVisible );
                }
            }
            ChangeHighlightItem( n, false );
            break;
        }
    } while ( n != nLoop );
}

void MenuFloatingWindow::KeyInput( const KeyEvent& rKEvent )
{
    VclPtr<vcl::Window> xWindow = this;

    bool accel = ImplGetSVData()->maNWFData.mbEnableAccel;
    bool autoacc = ImplGetSVData()->maNWFData.mbAutoAccel;
    sal_uInt16 nCode = rKEvent.GetKeyCode().GetCode();
    bKeyInput = true;
    switch ( nCode )
    {
        case KEY_UP:
        case KEY_DOWN:
        {
            ImplCursorUpDown( nCode == KEY_UP );
        }
        break;
        case KEY_END:
        case KEY_HOME:
        {
            ImplCursorUpDown( nCode == KEY_END, true );
        }
        break;
        case KEY_F6:
        case KEY_ESCAPE:
        {
            // Ctrl-F6 acts like ESC here, the menu bar however will then put the focus in the document
            if( nCode == KEY_F6 && !rKEvent.GetKeyCode().IsMod1() )
                break;
            if( pMenu )
            {
                if ( !pMenu->pStartedFrom )
                {
                    StopExecute();
                    KillActivePopup();
                }
                else if (pMenu->pStartedFrom->IsMenuBar())
                {
                    pMenu->pStartedFrom->MenuBarKeyInput(rKEvent);
                }
                else
                {
                    StopExecute();
                    PopupMenu* pPopupMenu = static_cast<PopupMenu*>(pMenu->pStartedFrom.get());
                    MenuFloatingWindow* pFloat = pPopupMenu->ImplGetFloatingWindow();
                    pFloat->GrabFocus();
                    pFloat->KillActivePopup();
                    pPopupMenu->ImplCallHighlight(pFloat->nHighlightedItem);
                }
            }
        }
        break;
        case KEY_LEFT:
        {
            if ( pMenu && pMenu->pStartedFrom )
            {
                StopExecute();
                if (pMenu->pStartedFrom->IsMenuBar())
                {
                    pMenu->pStartedFrom->MenuBarKeyInput(rKEvent);
                }
                else
                {
                    MenuFloatingWindow* pFloat = static_cast<PopupMenu*>(pMenu->pStartedFrom.get())->ImplGetFloatingWindow();
                    pFloat->GrabFocus();
                    pFloat->KillActivePopup();
                    sal_uInt16 highlightItem = pFloat->GetHighlightedItem();
                    pFloat->ChangeHighlightItem(highlightItem, false);
                }
            }
        }
        break;
        case KEY_RIGHT:
        {
            if( pMenu )
            {
                bool bDone = false;
                if ( nHighlightedItem != ITEMPOS_INVALID )
                {
                    MenuItemData* pData = pMenu->GetItemList()->GetDataFromPos( nHighlightedItem );
                    if ( pData && pData->pSubMenu )
                    {
                        HighlightChanged( nullptr );
                        bDone = true;
                    }
                }
                if ( !bDone )
                {
                    Menu* pStart = pMenu->ImplGetStartMenu();
                    if (pStart && pStart->IsMenuBar())
                    {
                        // Forward...
                        pStart->ImplGetWindow()->KeyInput( rKEvent );
                    }
                }
            }
        }
        break;
        case KEY_RETURN:
        {
            if( pMenu )
            {
                MenuItemData* pData = pMenu->GetItemList()->GetDataFromPos( nHighlightedItem );
                if ( pData && pData->bEnabled )
                {
                    if ( pData->pSubMenu )
                        HighlightChanged( nullptr );
                    else
                        EndExecute();
                }
                else
                    StopExecute();
            }
        }
        break;
        case KEY_MENU:
        {
            if( pMenu )
            {
                Menu* pStart = pMenu->ImplGetStartMenu();
                if (pStart && pStart->IsMenuBar())
                {
                    // Forward...
                    pStart->ImplGetWindow()->KeyInput( rKEvent );
                }
            }
        }
        break;
        default:
        {
            sal_Unicode nCharCode = rKEvent.GetCharCode();
            sal_uInt16 nPos = 0;
            sal_uInt16 nDuplicates = 0;
            MenuItemData* pData = (nCharCode && pMenu && accel) ?
                pMenu->GetItemList()->SearchItem(nCharCode, rKEvent.GetKeyCode(), nPos, nDuplicates, nHighlightedItem) : nullptr;
            if (pData)
            {
                if ( pData->pSubMenu || nDuplicates > 1 )
                {
                    ChangeHighlightItem( nPos, false );
                    HighlightChanged( nullptr );
                }
                else
                {
                    nHighlightedItem = nPos;
                    EndExecute();
                }
            }
            else
                FloatingWindow::KeyInput( rKEvent );
        }
    }

    if (pMenu && pMenu->pStartedFrom && pMenu->pStartedFrom->IsMenuBar())
    {
        MenuBar *pMenuBar = static_cast<MenuBar*>(pMenu->pStartedFrom.get());
        const bool bShowAccels = nCode != KEY_ESCAPE;
        if (pMenuBar->getMenuBarWindow()->GetMBWMenuKey() != bShowAccels)
        {
            pMenuBar->getMenuBarWindow()->SetMBWMenuKey(bShowAccels);
            pMenuBar->getMenuBarWindow()->SetMBWHideAccel(!bShowAccels);
            if (accel && autoacc)
                Invalidate(InvalidateFlags::Update);
        }
    }

    // #105474# check if menu window was not destroyed
    if ( !xWindow->IsDisposed() )
    {
        bKeyInput = false;
    }
}

void MenuFloatingWindow::Paint(vcl::RenderContext& rRenderContext, const Rectangle &rPaintRect)
{
    if (!pMenu)
        return;

    rRenderContext.Push( PushFlags::CLIPREGION );
    rRenderContext.SetClipRegion(vcl::Region(rPaintRect));

    if (rRenderContext.IsNativeControlSupported(ControlType::MenuPopup, ControlPart::Entire))
    {
        rRenderContext.SetClipRegion();
        long nX = pMenu->pLogo ? pMenu->pLogo->aBitmap.GetSizePixel().Width() : 0;
        Size aPxSize(GetOutputSizePixel());
        aPxSize.Width() -= nX;
        ImplControlValue aVal(pMenu->nTextPos - GUTTERBORDER);
        rRenderContext.DrawNativeControl(ControlType::MenuPopup, ControlPart::Entire,
                                         Rectangle(Point(nX, 0), aPxSize),
                                         ControlState::ENABLED, aVal, OUString());
        InitMenuClipRegion(rRenderContext);
    }
    if (IsScrollMenu())
    {
        ImplDrawScroller(rRenderContext, true);
        ImplDrawScroller(rRenderContext, false);
    }
    rRenderContext.SetFillColor(rRenderContext.GetSettings().GetStyleSettings().GetMenuColor());
    pMenu->ImplPaint(rRenderContext, nScrollerHeight, ImplGetStartY());
    if (nHighlightedItem != ITEMPOS_INVALID)
        RenderHighlightItem(rRenderContext, nHighlightedItem);

    rRenderContext.Pop();
}

void MenuFloatingWindow::ImplDrawScroller(vcl::RenderContext& rRenderContext, bool bUp)
{
    if (!pMenu)
        return;

    rRenderContext.SetClipRegion();

    Size aOutSz(GetOutputSizePixel());
    long nY = bUp ? 0 : (aOutSz.Height() - nScrollerHeight);
    long nX = pMenu->pLogo ? pMenu->pLogo->aBitmap.GetSizePixel().Width() : 0;
    Rectangle aRect(Point(nX, nY), Size(aOutSz.Width() - nX, nScrollerHeight));

    DecorationView aDecoView(&rRenderContext);
    SymbolType eSymbol = bUp ? SymbolType::SPIN_UP : SymbolType::SPIN_DOWN;

    DrawSymbolFlags nStyle = DrawSymbolFlags::NONE;
    if ((bUp && !bScrollUp) || (!bUp && !bScrollDown))
        nStyle |= DrawSymbolFlags::Disable;

    aDecoView.DrawSymbol(aRect, eSymbol, rRenderContext.GetSettings().GetStyleSettings().GetButtonTextColor(), nStyle);

    InitMenuClipRegion(rRenderContext);
}

void MenuFloatingWindow::RequestHelp( const HelpEvent& rHEvt )
{
    sal_uInt16 nId = nHighlightedItem;
    Menu* pM = pMenu;
    vcl::Window* pW = this;

    // #102618# Get item rect before destroying the window in EndExecute() call
    Rectangle aHighlightRect( ImplGetItemRect( nHighlightedItem ) );

    if ( rHEvt.GetMode() & (HelpEventMode::CONTEXT | HelpEventMode::EXTENDED) )
    {
        nHighlightedItem = ITEMPOS_INVALID;
        EndExecute();
        pW = nullptr;
    }

    if( !ImplHandleHelpEvent( pW, pM, nId, rHEvt, aHighlightRect ) )
        Window::RequestHelp( rHEvt );
}

void MenuFloatingWindow::StateChanged( StateChangedType nType )
{
    FloatingWindow::StateChanged( nType );

    if ( ( nType == StateChangedType::ControlForeground ) || ( nType == StateChangedType::ControlBackground ) )
    {
        ApplySettings(*this);
        Invalidate();
    }
}

void MenuFloatingWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    FloatingWindow::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DataChangedEventType::FONTS) ||
         (rDCEvt.GetType() == DataChangedEventType::FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
          (rDCEvt.GetFlags() & AllSettingsFlags::STYLE)) )
    {
        ApplySettings(*this);
        Invalidate();
    }
}

void MenuFloatingWindow::Command( const CommandEvent& rCEvt )
{
    if ( rCEvt.GetCommand() == CommandEventId::Wheel )
    {
        const CommandWheelData* pData = rCEvt.GetWheelData();
        if( !pData->GetModifier() && ( pData->GetMode() == CommandWheelMode::SCROLL ) )
        {
            ImplScroll( pData->GetDelta() > 0L );
            MouseMove( MouseEvent( GetPointerPosPixel(), 0 ) );
        }
    }
}

css::uno::Reference<css::accessibility::XAccessible> MenuFloatingWindow::CreateAccessible()
{
    css::uno::Reference<css::accessibility::XAccessible> xAcc;

    if (pMenu && !pMenu->pStartedFrom)
        xAcc = pMenu->GetAccessible();

    return xAcc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
