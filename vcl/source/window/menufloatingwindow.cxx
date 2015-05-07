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

#include <svdata.hxx>
#include <vcl/decoview.hxx>
#include <vcl/settings.hxx>
#include <window.h>

MenuFloatingWindow::MenuFloatingWindow( Menu* pMen, vcl::Window* pParent, WinBits nStyle ) :
    FloatingWindow( pParent, nStyle )
{
    mpWindowImpl->mbMenuFloatingWindow= true;
    pMenu               = pMen;
    pActivePopup        = 0;
    nSaveFocusId        = 0;
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

    EnableSaveBackground();
    ImplInitMenuWindow( this, true, false );

    SetPopupModeEndHdl( LINK( this, MenuFloatingWindow, PopupEnd ) );

    aHighlightChangedTimer.SetTimeoutHdl( LINK( this, MenuFloatingWindow, HighlightChanged ) );
    aHighlightChangedTimer.SetTimeout( GetSettings().GetMouseSettings().GetMenuDelay() );
    aSubmenuCloseTimer.SetTimeout( GetSettings().GetMouseSettings().GetMenuDelay() );
    aSubmenuCloseTimer.SetTimeoutHdl( LINK( this, MenuFloatingWindow, SubmenuClose ) );
    aScrollTimer.SetTimeoutHdl( LINK( this, MenuFloatingWindow, AutoScroll ) );

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
                if( pPWin )
                    pPWin->HighlightItem( i, false );
            }
        }

        // free the reference to the accessible component
        SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >() );

        aHighlightChangedTimer.Stop();

        // #95056# invalidate screen area covered by system window
        // so this can be taken into account if the commandhandler performs a scroll operation
        if( GetParent() )
        {
            Rectangle aInvRect( GetWindowExtentsRelative( GetParent() ) );
            GetParent()->Invalidate( aInvRect );
        }
        pMenu = NULL;
        RemoveEventListener( LINK( this, MenuFloatingWindow, ShowHideListener ) );
    }
}

MenuFloatingWindow::~MenuFloatingWindow()
{
    disposeOnce();
}

void MenuFloatingWindow::dispose()
{
    doShutdown();

    FloatingWindow::dispose();
}

void MenuFloatingWindow::Resize()
{
    InitMenuClipRegion();
}

long MenuFloatingWindow::ImplGetStartY() const
{
    long nY = 0;
    if( pMenu )
    {
        for ( sal_uInt16 n = 0; n < nFirstEntry; n++ )
            nY += pMenu->GetItemList()->GetDataFromPos( n )->aSz.Height();
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

void MenuFloatingWindow::InitMenuClipRegion()
{
    if ( IsScrollMenu() )
    {
        SetClipRegion( ImplCalcClipRegion() );
    }
    else
    {
        SetClipRegion();
    }
}

void MenuFloatingWindow::ImplHighlightItem( const MouseEvent& rMEvt, bool bMBDown )
{
    if( ! pMenu )
        return;

    long nY = nScrollerHeight + ImplGetSVData()->maNWFData.mnMenuFormatBorderY;
    long nMouseY = rMEvt.GetPosPixel().Y();
    Size aOutSz = GetOutputSizePixel();
    if ( ( nMouseY >= nY ) && ( nMouseY < ( aOutSz.Height() - nY ) ) )
    {
        bool bHighlighted = false;
        size_t nCount = pMenu->pItemList->size();
        nY += ImplGetStartY();  // ggf. gescrollt.
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
                            HighlightChanged( NULL );
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
                                HighlightChanged( NULL );
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

IMPL_LINK_NOARG(MenuFloatingWindow, PopupEnd)
{
    // "this" will be deleted before the end of this method!
    Menu* pM = pMenu;
    if ( bInExecute )
    {
        if ( pActivePopup )
        {
            //DBG_ASSERT( !pActivePopup->ImplGetWindow(), "PopupEnd, obwohl pActivePopup MIT Window!" );
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
        pM->pStartedFrom = 0;

    return 0;
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
            sal_uLong nOldFlags = GetPopupModeFlags();
            SetPopupModeFlags( GetPopupModeFlags() | FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE );
            KillActivePopup();
            SetPopupModeFlags( nOldFlags );
        }
        if ( pItemData->bEnabled && pItemData->pSubMenu && pItemData->pSubMenu->GetItemCount() && ( pItemData->pSubMenu != pActivePopup ) )
        {
            pActivePopup = static_cast<PopupMenu*>(pItemData->pSubMenu);
            long nY = nScrollerHeight+ImplGetStartY();
            MenuItemData* pData = 0;
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
            sal_uLong nOldFlags = GetPopupModeFlags();
            SetPopupModeFlags( GetPopupModeFlags() | FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE );
            sal_uInt16 nRet = pActivePopup->ImplExecute( this, Rectangle( aItemTopLeft, aItemBottomRight ), FLOATWIN_POPUPMODE_RIGHT, pMenu, pTimer == nullptr );
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

IMPL_LINK( MenuFloatingWindow, ShowHideListener, VclWindowEvent*, pEvent )
{
    if( ! pMenu )
        return 0;

    if( pEvent->GetId() == VCLEVENT_WINDOW_SHOW )
        pMenu->ImplCallEventListeners( VCLEVENT_MENU_SHOW, ITEMPOS_INVALID );
    else if( pEvent->GetId() == VCLEVENT_WINDOW_HIDE )
        pMenu->ImplCallEventListeners( VCLEVENT_MENU_HIDE, ITEMPOS_INVALID );
    return 0;
}

void MenuFloatingWindow::EnableScrollMenu( bool b )
{
    bScrollMenu = b;
    nScrollerHeight = b ? (sal_uInt16) GetSettings().GetStyleSettings().GetScrollBarSize() /2 : 0;
    bScrollDown = true;
    InitMenuClipRegion();
}

void MenuFloatingWindow::Execute()
{
    ImplSVData* pSVData = ImplGetSVData();

    pSVData->maAppData.mpActivePopupMenu = static_cast<PopupMenu*>(pMenu);

    bInExecute = true;
//  bCallingSelect = false;

    while ( bInExecute )
        Application::Yield();

    pSVData->maAppData.mpActivePopupMenu = NULL;
}

void MenuFloatingWindow::StopExecute( sal_uLong nFocusId )
{
    // restore focus
    // (could have been restored in Select)
    if ( nSaveFocusId )
    {
        Window::EndSaveFocus( nFocusId, false );
        nFocusId = nSaveFocusId;
        if ( nFocusId )
        {
            nSaveFocusId = 0;
            ImplGetSVData()->maWinData.mbNoDeactivate = false;
        }
    }
    ImplEndPopupMode( 0, nFocusId );

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
        pActivePopup = NULL;
        pPopup->bInCallback = true;
        pPopup->Deactivate();
        pPopup->bInCallback = false;
        if ( pPopup->ImplGetWindow() )
        {
            pPopup->ImplGetFloatingWindow()->StopExecute();
            pPopup->ImplGetFloatingWindow()->doShutdown();
            pPopup->pWindow->doLazyDelete();
            pPopup->pWindow = NULL;

            Update();
        }
    }
}

void MenuFloatingWindow::EndExecute()
{
    Menu* pStart = pMenu ? pMenu->ImplGetStartMenu() : NULL;
    sal_uLong nFocusId = 0;
    if (pStart)
        nFocusId = pStart->DeactivateMenuBar(nFocusId);

    // if started elsewhere, cleanup there as well
    MenuFloatingWindow* pCleanUpFrom = this;
    MenuFloatingWindow* pWin = this;
    while (pWin && !pWin->bInExecute &&
        pWin->pMenu->pStartedFrom && !pWin->pMenu->pStartedFrom->IsMenuBar())
    {
        pWin = static_cast<PopupMenu*>(pWin->pMenu->pStartedFrom)->ImplGetFloatingWindow();
    }
    if ( pWin )
        pCleanUpFrom = pWin;

    // this window will be destroyed => store date locally...
    Menu* pM = pMenu;
    sal_uInt16 nItem = nHighlightedItem;

    pCleanUpFrom->StopExecute( nFocusId );

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
    //    pActivePopup->ImplGetFloatingWindow()->ToTop( TOTOP_NOGRABFOCUS );

    ImplHighlightItem( rMEvt, true );

    nMBDownPos = nHighlightedItem;
}

void MenuFloatingWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    MenuItemData* pData = pMenu ? pMenu->GetItemList()->GetDataFromPos( nHighlightedItem ) : NULL;
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
        MenuItemData* pData = pMenu ? pMenu->pItemList->GetDataFromPos( nHighlightedItem ) : NULL;
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

    if( ! pMenu )
        return;

    HighlightItem( nHighlightedItem, false );

    pMenu->ImplKillLayoutData();

    if ( bScrollUp && bUp )
    {
        nFirstEntry = pMenu->ImplGetPrevVisible( nFirstEntry );
        DBG_ASSERT( nFirstEntry != ITEMPOS_INVALID, "Scroll?!" );

        long nScrollEntryHeight = pMenu->GetItemList()->GetDataFromPos( nFirstEntry )->aSz.Height();

        if ( !bScrollDown )
        {
            bScrollDown = true;
            ImplDrawScroller( false );
        }

        if ( pMenu->ImplGetPrevVisible( nFirstEntry ) == ITEMPOS_INVALID )
        {
            bScrollUp = false;
            ImplDrawScroller( true );
        }

        Scroll( 0, nScrollEntryHeight, ImplCalcClipRegion( false ).GetBoundRect(), SCROLL_CLIP );
    }
    else if ( bScrollDown && !bUp )
    {
        long nScrollEntryHeight = pMenu->GetItemList()->GetDataFromPos( nFirstEntry )->aSz.Height();

        nFirstEntry = pMenu->ImplGetNextVisible( nFirstEntry );
        DBG_ASSERT( nFirstEntry != ITEMPOS_INVALID, "Scroll?!" );

        if ( !bScrollUp )
        {
            bScrollUp = true;
            ImplDrawScroller( true );
        }

        long nHeight = GetOutputSizePixel().Height();
        sal_uInt16 nLastVisible;
        static_cast<PopupMenu*>(pMenu)->ImplCalcVisEntries( nHeight, nFirstEntry, &nLastVisible );
        if ( pMenu->ImplGetNextVisible( nLastVisible ) == ITEMPOS_INVALID )
        {
            bScrollDown = false;
            ImplDrawScroller( false );
        }

        Scroll( 0, -nScrollEntryHeight, ImplCalcClipRegion( false ).GetBoundRect(), SCROLL_CLIP );
    }

    HighlightItem( nHighlightedItem, true );
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
        HighlightItem( nHighlightedItem, false );
        pMenu->ImplCallEventListeners( VCLEVENT_MENU_DEHIGHLIGHT, nHighlightedItem );
    }

    nHighlightedItem = (sal_uInt16)n;
    DBG_ASSERT( pMenu->ImplIsVisible( nHighlightedItem ) || nHighlightedItem == ITEMPOS_INVALID, "ChangeHighlightItem: Not visible!" );
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
                    pPWin->HighlightItem( i, true );
                    pPWin->nHighlightedItem = i;
                }
            }
        }
        HighlightItem( nHighlightedItem, true );
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

void MenuFloatingWindow::HighlightItem( sal_uInt16 nPos, bool bHighlight )
{
    if( ! pMenu )
        return;

    Size    aSz = GetOutputSizePixel();
    long    nStartY = ImplGetStartY();
    long    nY = nScrollerHeight + nStartY + ImplGetSVData()->maNWFData.mnMenuFormatBorderY;
    long    nX = 0;

    if ( pMenu->pLogo )
        nX = pMenu->pLogo->aBitmap.GetSizePixel().Width();

    int nOuterSpaceX = ImplGetSVData()->maNWFData.mnMenuFormatBorderX;

    size_t nCount = pMenu->pItemList->size();
    for ( size_t n = 0; n < nCount; n++ )
    {
        MenuItemData* pData = pMenu->pItemList->GetDataFromPos( n );
        if ( n == nPos )
        {
            DBG_ASSERT( pMenu->ImplIsVisible( n ), "Highlight: Item not visible!" );
            if ( pData->eType != MenuItemType::SEPARATOR )
            {
                bool bRestoreLineColor = false;
                Color oldLineColor;
                bool bDrawItemRect = true;

                Rectangle aItemRect( Point( nX+nOuterSpaceX, nY ), Size( aSz.Width()-2*nOuterSpaceX, pData->aSz.Height() ) );
                if ( pData->nBits & MenuItemBits::POPUPSELECT )
                {
                    long nFontHeight = GetTextHeight();
                    aItemRect.Right() -= nFontHeight + nFontHeight/4;
                }

                if( IsNativeControlSupported( CTRL_MENU_POPUP, PART_ENTIRE_CONTROL ) )
                {
                    Size aPxSize( GetOutputSizePixel() );
                    Push( PushFlags::CLIPREGION );
                    IntersectClipRegion( Rectangle( Point( nX, nY ), Size( aSz.Width(), pData->aSz.Height() ) ) );
                    Rectangle aCtrlRect( Point( nX, 0 ), Size( aPxSize.Width()-nX, aPxSize.Height() ) );
                    MenupopupValue aVal( pMenu->nTextPos-GUTTERBORDER, aItemRect );
                    DrawNativeControl( CTRL_MENU_POPUP, PART_ENTIRE_CONTROL,
                                       aCtrlRect,
                                       ControlState::ENABLED,
                                       aVal,
                                       OUString() );
                    if( bHighlight &&
                        IsNativeControlSupported( CTRL_MENU_POPUP, PART_MENU_ITEM ) )
                    {
                        bDrawItemRect = false;
                        if( !DrawNativeControl( CTRL_MENU_POPUP, PART_MENU_ITEM,
                                                        aItemRect,
                                                        ControlState::SELECTED | ( pData->bEnabled? ControlState::ENABLED: ControlState::NONE ),
                                                        aVal,
                                                        OUString() ) )
                        {
                            bDrawItemRect = bHighlight;
                        }
                    }
                    else
                        bDrawItemRect = bHighlight;
                    Pop();
                }
                if( bDrawItemRect )
                {
                    if ( bHighlight )
                    {
                        if( pData->bEnabled )
                            SetFillColor( GetSettings().GetStyleSettings().GetMenuHighlightColor() );
                        else
                        {
                            SetFillColor();
                            oldLineColor = GetLineColor();
                            SetLineColor( GetSettings().GetStyleSettings().GetMenuHighlightColor() );
                            bRestoreLineColor = true;
                        }
                    }
                    else
                        SetFillColor( GetSettings().GetStyleSettings().GetMenuColor() );

                    DrawRect( aItemRect );
                }
                pMenu->ImplPaint( this, nScrollerHeight, nStartY, pData, bHighlight );
                if( bRestoreLineColor )
                    SetLineColor( oldLineColor );
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
            DBG_ASSERT( pMenu->ImplIsVisible( n ), "ImplGetItemRect: Item not visible!" );
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

        MenuItemData* pData = (MenuItemData*)pMenu->GetItemList()->GetDataFromPos( n );
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
                static_cast<PopupMenu*>(pMenu)->ImplCalcVisEntries( aOutSz.Height(), nFirstEntry, &nLastVisible );
                while ( n > nLastVisible )
                {
                    ImplScroll( false );
                    static_cast<PopupMenu*>(pMenu)->ImplCalcVisEntries( aOutSz.Height(), nFirstEntry, &nLastVisible );
                }
            }
            ChangeHighlightItem( n, false );
            break;
        }
    } while ( n != nLoop );
}

void MenuFloatingWindow::KeyInput( const KeyEvent& rKEvent )
{
    ImplDelData aDelData;
    ImplAddDel( &aDelData );

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
                    PopupMenu* pPopupMenu = static_cast<PopupMenu*>(pMenu->pStartedFrom);
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
                    MenuFloatingWindow* pFloat = static_cast<PopupMenu*>(pMenu->pStartedFrom)->ImplGetFloatingWindow();
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
                        HighlightChanged( 0 );
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
                        HighlightChanged( 0 );
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
            MenuItemData* pData = (nCharCode && pMenu) ? pMenu->GetItemList()->SearchItem( nCharCode, rKEvent.GetKeyCode(), nPos, nDuplicates, nHighlightedItem ) : NULL;
            if ( pData )
            {
                if ( pData->pSubMenu || nDuplicates > 1 )
                {
                    ChangeHighlightItem( nPos, false );
                    HighlightChanged( 0 );
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
    // #105474# check if menu window was not destroyed
    if ( !aDelData.IsDead() )
    {
        ImplRemoveDel( &aDelData );
        bKeyInput = false;
    }
}

void MenuFloatingWindow::Paint( vcl::RenderContext& /*rRenderContext*/, const Rectangle& )
{
    if( ! pMenu )
        return;

    if( IsNativeControlSupported( CTRL_MENU_POPUP, PART_ENTIRE_CONTROL ) )
    {
        SetClipRegion();
        long nX = pMenu->pLogo ? pMenu->pLogo->aBitmap.GetSizePixel().Width() : 0;
        Size aPxSize( GetOutputSizePixel() );
        aPxSize.Width() -= nX;
        ImplControlValue aVal( pMenu->nTextPos-GUTTERBORDER );
        DrawNativeControl( CTRL_MENU_POPUP, PART_ENTIRE_CONTROL,
                           Rectangle( Point( nX, 0 ), aPxSize ),
                           ControlState::ENABLED,
                           aVal,
                           OUString() );
        InitMenuClipRegion();
    }
    if ( IsScrollMenu() )
    {
        ImplDrawScroller( true );
        ImplDrawScroller( false );
    }
    SetFillColor( GetSettings().GetStyleSettings().GetMenuColor() );
    pMenu->ImplPaint( this, nScrollerHeight, ImplGetStartY() );
    if ( nHighlightedItem != ITEMPOS_INVALID )
        HighlightItem( nHighlightedItem, true );
}

void MenuFloatingWindow::ImplDrawScroller( bool bUp )
{
    if( ! pMenu )
        return;

    SetClipRegion();

    Size aOutSz = GetOutputSizePixel();
    long nY = bUp ? 0 : ( aOutSz.Height() - nScrollerHeight );
    long nX = pMenu->pLogo ? pMenu->pLogo->aBitmap.GetSizePixel().Width() : 0;
    Rectangle aRect( Point( nX, nY ), Size( aOutSz.Width()-nX, nScrollerHeight ) );

    DecorationView aDecoView( this );
    SymbolType eSymbol = bUp ? SymbolType::SPIN_UP : SymbolType::SPIN_DOWN;

    sal_uInt16 nStyle = 0;
    if ( ( bUp && !bScrollUp ) || ( !bUp && !bScrollDown ) )
        nStyle |= SYMBOL_DRAW_DISABLE;

    aDecoView.DrawSymbol( aRect, eSymbol, GetSettings().GetStyleSettings().GetButtonTextColor(), nStyle );

    InitMenuClipRegion();
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
        pW = NULL;
    }

    if( !ImplHandleHelpEvent( pW, pM, nId, rHEvt, aHighlightRect ) )
        Window::RequestHelp( rHEvt );
}

void MenuFloatingWindow::StateChanged( StateChangedType nType )
{
    FloatingWindow::StateChanged( nType );

    if ( ( nType == StateChangedType::ControlForeground ) || ( nType == StateChangedType::ControlBackground ) )
    {
        ImplInitMenuWindow( this, false, false );
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
        ImplInitMenuWindow( this, false, false );
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
//          ImplCursorUpDown( pData->GetDelta() > 0L );
            ImplScroll( pData->GetDelta() > 0L );
            MouseMove( MouseEvent( GetPointerPosPixel(), 0 ) );
        }
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > MenuFloatingWindow::CreateAccessible()
{
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > xAcc;

    if ( pMenu && !pMenu->pStartedFrom )
        xAcc = pMenu->GetAccessible();

    return xAcc;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
