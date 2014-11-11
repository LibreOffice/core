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

#include <svids.hrc>
#include <svdata.hxx>
#include <brdwin.hxx>
#include <window.h>

#include <vcl/event.hxx>
#include <vcl/decoview.hxx>
#include <vcl/syswin.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/gradient.hxx>
#include <vcl/image.hxx>
#include <vcl/virdev.hxx>
#include <vcl/help.hxx>
#include <vcl/edit.hxx>
#include <vcl/metric.hxx>
#include <vcl/settings.hxx>

using namespace ::com::sun::star::uno;

// useful caption height for title bar buttons
#define MIN_CAPTION_HEIGHT 18

static void ImplGetPinImage( sal_uInt16 nStyle, bool bPinIn, Image& rImage )
{
    // load ImageList if not available yet
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maCtrlData.mpPinImgList )
    {
        ResMgr* pResMgr = ImplGetResMgr();
        pSVData->maCtrlData.mpPinImgList = new ImageList();
        if( pResMgr )
        {
            Color aMaskColor( 0x00, 0x00, 0xFF );
            pSVData->maCtrlData.mpPinImgList->InsertFromHorizontalBitmap
                ( ResId( SV_RESID_BITMAP_PIN, *pResMgr ), 4,
                  &aMaskColor, NULL, NULL, 0);
        }
    }

    // get and return Image
    sal_uInt16 nId;
    if ( nStyle & BUTTON_DRAW_PRESSED )
    {
        if ( bPinIn )
            nId = 4;
        else
            nId = 3;
    }
    else
    {
        if ( bPinIn )
            nId = 2;
        else
            nId = 1;
    }
    rImage = pSVData->maCtrlData.mpPinImgList->GetImage( nId );
}

namespace vcl {

void Window::ImplCalcSymbolRect( Rectangle& rRect )
{
    // Add border, not shown in the non-default representation,
    // as we want to use it for small buttons
    rRect.Left()--;
    rRect.Top()--;
    rRect.Right()++;
    rRect.Bottom()++;

    // we leave 5% room between the symbol and the button border
    long nExtraWidth = ((rRect.GetWidth()*50)+500)/1000;
    long nExtraHeight = ((rRect.GetHeight()*50)+500)/1000;
    rRect.Left()    += nExtraWidth;
    rRect.Right()   -= nExtraWidth;
    rRect.Top()     += nExtraHeight;
    rRect.Bottom()  -= nExtraHeight;
}

} /* namespace vcl */

static void ImplDrawBrdWinSymbol( OutputDevice* pDev,
                                  const Rectangle& rRect, SymbolType eSymbol )
{
    // we leave 5% room between the symbol and the button border
    DecorationView  aDecoView( pDev );
    Rectangle       aTempRect = rRect;
    vcl::Window::ImplCalcSymbolRect( aTempRect );
    aDecoView.DrawSymbol( aTempRect, eSymbol,
                          pDev->GetSettings().GetStyleSettings().GetButtonTextColor(), 0 );
}

static void ImplDrawBrdWinSymbolButton( OutputDevice* pDev,
                                        const Rectangle& rRect,
                                        SymbolType eSymbol, sal_uInt16 nState )
{
    bool bMouseOver = (nState & BUTTON_DRAW_HIGHLIGHT) != 0;
    nState &= ~BUTTON_DRAW_HIGHLIGHT;

    Rectangle aTempRect;
    vcl::Window *pWin = dynamic_cast< vcl::Window* >(pDev);
    if( pWin )
    {
        if( bMouseOver )
        {
            // provide a bright background for selection effect
            pWin->SetFillColor( pDev->GetSettings().GetStyleSettings().GetWindowColor() );
            pWin->SetLineColor();
            pWin->DrawRect( rRect );
            pWin->DrawSelectionBackground( rRect, 2, (nState & BUTTON_DRAW_PRESSED) ? true : false,
                                            true, false );
        }
        aTempRect = rRect;
        aTempRect.Left()+=3;
        aTempRect.Right()-=4;
        aTempRect.Top()+=3;
        aTempRect.Bottom()-=4;
    }
    else
    {
        DecorationView aDecoView( pDev );
        aTempRect = aDecoView.DrawButton( rRect, nState|BUTTON_DRAW_FLAT );
    }
    ImplDrawBrdWinSymbol( pDev, aTempRect, eSymbol );
}

// - ImplBorderWindowView -

ImplBorderWindowView::~ImplBorderWindowView()
{
}

bool ImplBorderWindowView::MouseMove( const MouseEvent& )
{
    return false;
}

bool ImplBorderWindowView::MouseButtonDown( const MouseEvent& )
{
    return false;
}

bool ImplBorderWindowView::Tracking( const TrackingEvent& )
{
    return false;
}

OUString ImplBorderWindowView::RequestHelp( const Point&, Rectangle& )
{
    return OUString();
}

Rectangle ImplBorderWindowView::GetMenuRect() const
{
    return Rectangle();
}

void ImplBorderWindowView::ImplInitTitle( ImplBorderFrameData* pData )
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( !(pBorderWindow->GetStyle() & (WB_MOVEABLE | WB_POPUP)) ||
          (pData->mnTitleType == BORDERWINDOW_TITLE_NONE) )
    {
        pData->mnTitleType   = BORDERWINDOW_TITLE_NONE;
        pData->mnTitleHeight = 0;
    }
    else
    {
        const StyleSettings& rStyleSettings = pData->mpOutDev->GetSettings().GetStyleSettings();
        if ( pData->mnTitleType == BORDERWINDOW_TITLE_TEAROFF )
            pData->mnTitleHeight = rStyleSettings.GetTearOffTitleHeight();
        else
        {
            if ( pData->mnTitleType == BORDERWINDOW_TITLE_SMALL )
            {
                pBorderWindow->SetPointFont( rStyleSettings.GetFloatTitleFont() );
                pData->mnTitleHeight = rStyleSettings.GetFloatTitleHeight();
            }
            else // pData->mnTitleType == BORDERWINDOW_TITLE_NORMAL
            {
                pBorderWindow->SetPointFont( rStyleSettings.GetTitleFont() );
                pData->mnTitleHeight = rStyleSettings.GetTitleHeight();
            }
            long nTextHeight = pBorderWindow->GetTextHeight();
            if ( nTextHeight > pData->mnTitleHeight )
                pData->mnTitleHeight = nTextHeight;
        }
    }
}

sal_uInt16 ImplBorderWindowView::ImplHitTest( ImplBorderFrameData* pData, const Point& rPos )
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( pData->maTitleRect.IsInside( rPos ) )
    {
        if ( pData->maCloseRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_CLOSE;
        else if ( pData->maRollRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_ROLL;
        else if ( pData->maMenuRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_MENU;
        else if ( pData->maDockRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_DOCK;
        else if ( pData->maHideRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_HIDE;
        else if ( pData->maHelpRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_HELP;
        else if ( pData->maPinRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_PIN;
        else
            return BORDERWINDOW_HITTEST_TITLE;
    }

    if ( (pBorderWindow->GetStyle() & WB_SIZEABLE) &&
         !pBorderWindow->mbRollUp )
    {
        long nSizeWidth = pData->mnNoTitleTop+pData->mnTitleHeight;
        if ( nSizeWidth < 16 )
            nSizeWidth = 16;

        // no corner resize for floating toolbars, which would lead to jumps while formatting
        // setting nSizeWidth = 0 will only return pure left,top,right,bottom
        if( pBorderWindow->GetStyle() & (WB_OWNERDRAWDECORATION | WB_POPUP) )
            nSizeWidth = 0;

        if ( rPos.X() < pData->mnLeftBorder )
        {
            if ( rPos.Y() < nSizeWidth )
                return BORDERWINDOW_HITTEST_TOPLEFT;
            else if ( rPos.Y() >= pData->mnHeight-nSizeWidth )
                return BORDERWINDOW_HITTEST_BOTTOMLEFT;
            else
                return BORDERWINDOW_HITTEST_LEFT;
        }
        else if ( rPos.X() >= pData->mnWidth-pData->mnRightBorder )
        {
            if ( rPos.Y() < nSizeWidth )
                return BORDERWINDOW_HITTEST_TOPRIGHT;
            else if ( rPos.Y() >= pData->mnHeight-nSizeWidth )
                return BORDERWINDOW_HITTEST_BOTTOMRIGHT;
            else
                return BORDERWINDOW_HITTEST_RIGHT;
        }
        else if ( rPos.Y() < pData->mnNoTitleTop )
        {
            if ( rPos.X() < nSizeWidth )
                return BORDERWINDOW_HITTEST_TOPLEFT;
            else if ( rPos.X() >= pData->mnWidth-nSizeWidth )
                return BORDERWINDOW_HITTEST_TOPRIGHT;
            else
                return BORDERWINDOW_HITTEST_TOP;
        }
        else if ( rPos.Y() >= pData->mnHeight-pData->mnBottomBorder )
        {
            if ( rPos.X() < nSizeWidth )
                return BORDERWINDOW_HITTEST_BOTTOMLEFT;
            else if ( rPos.X() >= pData->mnWidth-nSizeWidth )
                return BORDERWINDOW_HITTEST_BOTTOMRIGHT;
            else
                return BORDERWINDOW_HITTEST_BOTTOM;
        }
    }

    return 0;
}

bool ImplBorderWindowView::ImplMouseMove( ImplBorderFrameData* pData, const MouseEvent& rMEvt )
{
    sal_uInt16 oldCloseState = pData->mnCloseState;
    sal_uInt16 oldMenuState = pData->mnMenuState;
    pData->mnCloseState &= ~BUTTON_DRAW_HIGHLIGHT;
    pData->mnMenuState &= ~BUTTON_DRAW_HIGHLIGHT;

    Point aMousePos = rMEvt.GetPosPixel();
    sal_uInt16 nHitTest = ImplHitTest( pData, aMousePos );
    PointerStyle ePtrStyle = POINTER_ARROW;
    if ( nHitTest & BORDERWINDOW_HITTEST_LEFT )
        ePtrStyle = POINTER_WINDOW_WSIZE;
    else if ( nHitTest & BORDERWINDOW_HITTEST_RIGHT )
        ePtrStyle = POINTER_WINDOW_ESIZE;
    else if ( nHitTest & BORDERWINDOW_HITTEST_TOP )
        ePtrStyle = POINTER_WINDOW_NSIZE;
    else if ( nHitTest & BORDERWINDOW_HITTEST_BOTTOM )
        ePtrStyle = POINTER_WINDOW_SSIZE;
    else if ( nHitTest & BORDERWINDOW_HITTEST_TOPLEFT )
        ePtrStyle = POINTER_WINDOW_NWSIZE;
    else if ( nHitTest & BORDERWINDOW_HITTEST_BOTTOMRIGHT )
        ePtrStyle = POINTER_WINDOW_SESIZE;
    else if ( nHitTest & BORDERWINDOW_HITTEST_TOPRIGHT )
        ePtrStyle = POINTER_WINDOW_NESIZE;
    else if ( nHitTest & BORDERWINDOW_HITTEST_BOTTOMLEFT )
        ePtrStyle = POINTER_WINDOW_SWSIZE;
    else if ( nHitTest & BORDERWINDOW_HITTEST_CLOSE )
        pData->mnCloseState |= BUTTON_DRAW_HIGHLIGHT;
    else if ( nHitTest & BORDERWINDOW_HITTEST_MENU )
        pData->mnMenuState |= BUTTON_DRAW_HIGHLIGHT;
    pData->mpBorderWindow->SetPointer( Pointer( ePtrStyle ) );

    if( pData->mnCloseState != oldCloseState )
        pData->mpBorderWindow->Invalidate( pData->maCloseRect );
    if( pData->mnMenuState != oldMenuState )
        pData->mpBorderWindow->Invalidate( pData->maMenuRect );

    return true;
}

bool ImplBorderWindowView::ImplMouseButtonDown( ImplBorderFrameData* pData, const MouseEvent& rMEvt )
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( rMEvt.IsLeft() || rMEvt.IsRight() )
    {
        pData->maMouseOff = rMEvt.GetPosPixel();
        pData->mnHitTest = ImplHitTest( pData, pData->maMouseOff );
        if ( pData->mnHitTest )
        {
            sal_uInt16 nDragFullTest = 0;
            bool bTracking = true;
            bool bHitTest = true;

            if ( pData->mnHitTest & BORDERWINDOW_HITTEST_CLOSE )
            {
                pData->mnCloseState |= BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_CLOSE );
            }
            else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_ROLL )
            {
                pData->mnRollState |= BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_ROLL );
            }
            else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_DOCK )
            {
                pData->mnDockState |= BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_DOCK );
            }
            else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_MENU )
            {
                pData->mnMenuState |= BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_MENU );

                // call handler already on mouse down
                if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                {
                    SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                    pClientWindow->TitleButtonClick( TITLE_BUTTON_MENU );
                }
            }
            else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_HIDE )
            {
                pData->mnHideState |= BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_HIDE );
            }
            else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_HELP )
            {
                pData->mnHelpState |= BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_HELP );
            }
            else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_PIN )
            {
                pData->mnPinState |= BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_PIN );
            }
            else
            {
                if ( rMEvt.GetClicks() == 1 )
                {
                    if ( bTracking )
                    {
                        Point   aPos         = pBorderWindow->GetPosPixel();
                        Size    aSize        = pBorderWindow->GetOutputSizePixel();
                        pData->mnTrackX      = aPos.X();
                        pData->mnTrackY      = aPos.Y();
                        pData->mnTrackWidth  = aSize.Width();
                        pData->mnTrackHeight = aSize.Height();

                        if ( pData->mnHitTest & BORDERWINDOW_HITTEST_TITLE )
                            nDragFullTest = DRAGFULL_OPTION_WINDOWMOVE;
                        else
                            nDragFullTest = DRAGFULL_OPTION_WINDOWSIZE;
                    }
                }
                else
                {
                    bTracking = false;

                    if ( (pData->mnHitTest & BORDERWINDOW_DRAW_TITLE) &&
                         ((rMEvt.GetClicks() % 2) == 0) )
                    {
                        pData->mnHitTest = 0;
                        bHitTest = false;

                        if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                        {
                            SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                            if ( true /*pBorderWindow->mbDockBtn*/ )   // always perform docking on double click, no button required
                                pClientWindow->TitleButtonClick( TITLE_BUTTON_DOCKING );
                            else if ( pBorderWindow->GetStyle() & WB_ROLLABLE )
                            {
                                if ( pClientWindow->IsRollUp() )
                                    pClientWindow->RollDown();
                                else
                                    pClientWindow->RollUp();
                                pClientWindow->Roll();
                            }
                        }
                    }
                }
            }

            if ( bTracking )
            {
                pData->mbDragFull = false;
                if ( nDragFullTest )
                    pData->mbDragFull = true;   // always fulldrag for proper docking, ignore system settings
                pBorderWindow->StartTracking();
            }
            else if ( bHitTest )
                pData->mnHitTest = 0;
        }
    }

    return true;
}

bool ImplBorderWindowView::ImplTracking( ImplBorderFrameData* pData, const TrackingEvent& rTEvt )
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( rTEvt.IsTrackingEnded() )
    {
        sal_uInt16 nHitTest = pData->mnHitTest;
        pData->mnHitTest = 0;

        if ( nHitTest & BORDERWINDOW_HITTEST_CLOSE )
        {
            if ( pData->mnCloseState & BUTTON_DRAW_PRESSED )
            {
                pData->mnCloseState &= ~BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_CLOSE );

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    // dispatch to correct window type (why is Close() not virtual ??? )
                    // TODO: make Close() virtual
                    vcl::Window *pWin = pBorderWindow->ImplGetClientWindow()->ImplGetWindow();
                    SystemWindow  *pSysWin  = dynamic_cast<SystemWindow* >(pWin);
                    DockingWindow *pDockWin = dynamic_cast<DockingWindow*>(pWin);
                    if ( pSysWin )
                        pSysWin->Close();
                    else if ( pDockWin )
                        pDockWin->Close();
                }
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_ROLL )
        {
            if ( pData->mnRollState & BUTTON_DRAW_PRESSED )
            {
                pData->mnRollState &= ~BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_ROLL );

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                        if ( pClientWindow->IsRollUp() )
                            pClientWindow->RollDown();
                        else
                            pClientWindow->RollUp();
                        pClientWindow->Roll();
                    }
                }
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_DOCK )
        {
            if ( pData->mnDockState & BUTTON_DRAW_PRESSED )
            {
                pData->mnDockState &= ~BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_DOCK );

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                        pClientWindow->TitleButtonClick( TITLE_BUTTON_DOCKING );
                    }
                }
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_MENU )
        {
            if ( pData->mnMenuState & BUTTON_DRAW_PRESSED )
            {
                pData->mnMenuState &= ~BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_MENU );

                // handler already called on mouse down
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_HIDE )
        {
            if ( pData->mnHideState & BUTTON_DRAW_PRESSED )
            {
                pData->mnHideState &= ~BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_HIDE );

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                        pClientWindow->TitleButtonClick( TITLE_BUTTON_HIDE );
                    }
                }
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_HELP )
        {
            if ( pData->mnHelpState & BUTTON_DRAW_PRESSED )
            {
                pData->mnHelpState &= ~BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_HELP );

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                }
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_PIN )
        {
            if ( pData->mnPinState & BUTTON_DRAW_PRESSED )
            {
                pData->mnPinState &= ~BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_PIN );

                // do not call a Click-Handler when aborting
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow());
                        pClientWindow->SetPin( !pClientWindow->IsPinned() );
                        pClientWindow->Pin();
                    }
                }
            }
        }
        else
        {
            if ( pData->mbDragFull )
            {
                // restore old state when aborting
                if ( rTEvt.IsTrackingCanceled() )
                    pBorderWindow->SetPosSizePixel( Point( pData->mnTrackX, pData->mnTrackY ), Size( pData->mnTrackWidth, pData->mnTrackHeight ) );
            }
            else
            {
                pBorderWindow->HideTracking();
                if ( !rTEvt.IsTrackingCanceled() )
                    pBorderWindow->SetPosSizePixel( Point( pData->mnTrackX, pData->mnTrackY ), Size( pData->mnTrackWidth, pData->mnTrackHeight ) );
            }

            if ( !rTEvt.IsTrackingCanceled() )
            {
                if ( pBorderWindow->ImplGetClientWindow()->ImplIsFloatingWindow() )
                {
                    if ( static_cast<FloatingWindow*>(pBorderWindow->ImplGetClientWindow())->IsInPopupMode() )
                        static_cast<FloatingWindow*>(pBorderWindow->ImplGetClientWindow())->EndPopupMode( FLOATWIN_POPUPMODEEND_TEAROFF );
                }
            }
        }
    }
    else if ( !rTEvt.GetMouseEvent().IsSynthetic() )
    {
        Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();

        if ( pData->mnHitTest & BORDERWINDOW_HITTEST_CLOSE )
        {
            if ( pData->maCloseRect.IsInside( aMousePos ) )
            {
                if ( !(pData->mnCloseState & BUTTON_DRAW_PRESSED) )
                {
                    pData->mnCloseState |= BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_CLOSE );
                }
            }
            else
            {
                if ( pData->mnCloseState & BUTTON_DRAW_PRESSED )
                {
                    pData->mnCloseState &= ~BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_CLOSE );
                }
            }
        }
        else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_ROLL )
        {
            if ( pData->maRollRect.IsInside( aMousePos ) )
            {
                if ( !(pData->mnRollState & BUTTON_DRAW_PRESSED) )
                {
                    pData->mnRollState |= BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_ROLL );
                }
            }
            else
            {
                if ( pData->mnRollState & BUTTON_DRAW_PRESSED )
                {
                    pData->mnRollState &= ~BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_ROLL );
                }
            }
        }
        else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_DOCK )
        {
            if ( pData->maDockRect.IsInside( aMousePos ) )
            {
                if ( !(pData->mnDockState & BUTTON_DRAW_PRESSED) )
                {
                    pData->mnDockState |= BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_DOCK );
                }
            }
            else
            {
                if ( pData->mnDockState & BUTTON_DRAW_PRESSED )
                {
                    pData->mnDockState &= ~BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_DOCK );
                }
            }
        }
        else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_MENU )
        {
            if ( pData->maMenuRect.IsInside( aMousePos ) )
            {
                if ( !(pData->mnMenuState & BUTTON_DRAW_PRESSED) )
                {
                    pData->mnMenuState |= BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_MENU );

                }
            }
            else
            {
                if ( pData->mnMenuState & BUTTON_DRAW_PRESSED )
                {
                    pData->mnMenuState &= ~BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_MENU );
                }
            }
        }
        else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_HIDE )
        {
            if ( pData->maHideRect.IsInside( aMousePos ) )
            {
                if ( !(pData->mnHideState & BUTTON_DRAW_PRESSED) )
                {
                    pData->mnHideState |= BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_HIDE );
                }
            }
            else
            {
                if ( pData->mnHideState & BUTTON_DRAW_PRESSED )
                {
                    pData->mnHideState &= ~BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_HIDE );
                }
            }
        }
        else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_HELP )
        {
            if ( pData->maHelpRect.IsInside( aMousePos ) )
            {
                if ( !(pData->mnHelpState & BUTTON_DRAW_PRESSED) )
                {
                    pData->mnHelpState |= BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_HELP );
                }
            }
            else
            {
                if ( pData->mnHelpState & BUTTON_DRAW_PRESSED )
                {
                    pData->mnHelpState &= ~BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_HELP );
                }
            }
        }
        else if ( pData->mnHitTest & BORDERWINDOW_HITTEST_PIN )
        {
            if ( pData->maPinRect.IsInside( aMousePos ) )
            {
                if ( !(pData->mnPinState & BUTTON_DRAW_PRESSED) )
                {
                    pData->mnPinState |= BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_PIN );
                }
            }
            else
            {
                if ( pData->mnPinState & BUTTON_DRAW_PRESSED )
                {
                    pData->mnPinState &= ~BUTTON_DRAW_PRESSED;
                    DrawWindow( BORDERWINDOW_DRAW_PIN );
                }
            }
        }
        else
        {
            aMousePos.X()    -= pData->maMouseOff.X();
            aMousePos.Y()    -= pData->maMouseOff.Y();

            if ( pData->mnHitTest & BORDERWINDOW_HITTEST_TITLE )
            {
                pData->mpBorderWindow->SetPointer( Pointer( POINTER_MOVE ) );

                Point aPos = pBorderWindow->GetPosPixel();
                aPos.X() += aMousePos.X();
                aPos.Y() += aMousePos.Y();
                if ( pData->mbDragFull )
                {
                    pBorderWindow->SetPosPixel( aPos );
                    pBorderWindow->ImplUpdateAll();
                    pBorderWindow->ImplGetFrameWindow()->ImplUpdateAll();
                }
                else
                {
                    pData->mnTrackX = aPos.X();
                    pData->mnTrackY = aPos.Y();
                    pBorderWindow->ShowTracking( Rectangle( pBorderWindow->ScreenToOutputPixel( aPos ), pBorderWindow->GetOutputSizePixel() ), SHOWTRACK_BIG );
                }
            }
            else
            {
                Point       aOldPos         = pBorderWindow->GetPosPixel();
                Size        aSize           = pBorderWindow->GetSizePixel();
                Rectangle   aNewRect( aOldPos, aSize );
                long        nOldWidth       = aSize.Width();
                long        nOldHeight      = aSize.Height();
                long        nBorderWidth    = pData->mnLeftBorder+pData->mnRightBorder;
                long        nBorderHeight   = pData->mnTopBorder+pData->mnBottomBorder;
                long        nMinWidth       = pBorderWindow->mnMinWidth+nBorderWidth;
                long        nMinHeight      = pBorderWindow->mnMinHeight+nBorderHeight;
                long        nMinWidth2      = nBorderWidth;
                long        nMaxWidth       = pBorderWindow->mnMaxWidth+nBorderWidth;
                long        nMaxHeight      = pBorderWindow->mnMaxHeight+nBorderHeight;

                if ( pData->mnTitleHeight )
                {
                    nMinWidth2 += 4;

                    if ( pBorderWindow->GetStyle() & WB_CLOSEABLE )
                        nMinWidth2 += pData->maCloseRect.GetWidth();
                }
                if ( nMinWidth2 > nMinWidth )
                    nMinWidth = nMinWidth2;
                if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_LEFT | BORDERWINDOW_HITTEST_TOPLEFT | BORDERWINDOW_HITTEST_BOTTOMLEFT) )
                {
                    aNewRect.Left() += aMousePos.X();
                    if ( aNewRect.GetWidth() < nMinWidth )
                        aNewRect.Left() = aNewRect.Right()-nMinWidth+1;
                    else if ( aNewRect.GetWidth() > nMaxWidth )
                        aNewRect.Left() = aNewRect.Right()-nMaxWidth+1;
                }
                else if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_RIGHT | BORDERWINDOW_HITTEST_TOPRIGHT | BORDERWINDOW_HITTEST_BOTTOMRIGHT) )
                {
                    aNewRect.Right() += aMousePos.X();
                    if ( aNewRect.GetWidth() < nMinWidth )
                        aNewRect.Right() = aNewRect.Left()+nMinWidth+1;
                    else if ( aNewRect.GetWidth() > nMaxWidth )
                        aNewRect.Right() = aNewRect.Left()+nMaxWidth+1;
                }
                if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_TOP | BORDERWINDOW_HITTEST_TOPLEFT | BORDERWINDOW_HITTEST_TOPRIGHT) )
                {
                    aNewRect.Top() += aMousePos.Y();
                    if ( aNewRect.GetHeight() < nMinHeight )
                        aNewRect.Top() = aNewRect.Bottom()-nMinHeight+1;
                    else if ( aNewRect.GetHeight() > nMaxHeight )
                        aNewRect.Top() = aNewRect.Bottom()-nMaxHeight+1;
                }
                else if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_BOTTOM | BORDERWINDOW_HITTEST_BOTTOMLEFT | BORDERWINDOW_HITTEST_BOTTOMRIGHT) )
                {
                    aNewRect.Bottom() += aMousePos.Y();
                    if ( aNewRect.GetHeight() < nMinHeight )
                        aNewRect.Bottom() = aNewRect.Top()+nMinHeight+1;
                    else if ( aNewRect.GetHeight() > nMaxHeight )
                        aNewRect.Bottom() = aNewRect.Top()+nMaxHeight+1;
                }

                // call Resizing-Handler for SystemWindows
                if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                {
                    // adjust size for Resizing-call
                    aSize = aNewRect.GetSize();
                    aSize.Width()   -= nBorderWidth;
                    aSize.Height()  -= nBorderHeight;
                    static_cast<SystemWindow*>(pBorderWindow->ImplGetClientWindow())->Resizing( aSize );
                    aSize.Width()   += nBorderWidth;
                    aSize.Height()  += nBorderHeight;
                    if ( aSize.Width() < nMinWidth )
                        aSize.Width() = nMinWidth;
                    if ( aSize.Height() < nMinHeight )
                        aSize.Height() = nMinHeight;
                    if ( aSize.Width() > nMaxWidth )
                        aSize.Width() = nMaxWidth;
                    if ( aSize.Height() > nMaxHeight )
                        aSize.Height() = nMaxHeight;
                    if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_LEFT | BORDERWINDOW_HITTEST_TOPLEFT | BORDERWINDOW_HITTEST_BOTTOMLEFT) )
                        aNewRect.Left() = aNewRect.Right()-aSize.Width()+1;
                    else
                        aNewRect.Right() = aNewRect.Left()+aSize.Width()-1;
                    if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_TOP | BORDERWINDOW_HITTEST_TOPLEFT | BORDERWINDOW_HITTEST_TOPRIGHT) )
                        aNewRect.Top() = aNewRect.Bottom()-aSize.Height()+1;
                    else
                        aNewRect.Bottom() = aNewRect.Top()+aSize.Height()-1;
                }

                if ( pData->mbDragFull )
                {
                    // no move (only resize) if position did not change
                    if( aOldPos != aNewRect.TopLeft() )
                        pBorderWindow->setPosSizePixel( aNewRect.Left(), aNewRect.Top(),
                                                    aNewRect.GetWidth(), aNewRect.GetHeight(), WINDOW_POSSIZE_POSSIZE );
                    else
                        pBorderWindow->setPosSizePixel( aNewRect.Left(), aNewRect.Top(),
                                                    aNewRect.GetWidth(), aNewRect.GetHeight(), WINDOW_POSSIZE_SIZE );

                    pBorderWindow->ImplUpdateAll();
                    pBorderWindow->ImplGetFrameWindow()->ImplUpdateAll();
                    if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_RIGHT | BORDERWINDOW_HITTEST_TOPRIGHT | BORDERWINDOW_HITTEST_BOTTOMRIGHT) )
                        pData->maMouseOff.X() += aNewRect.GetWidth()-nOldWidth;
                    if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_BOTTOM | BORDERWINDOW_HITTEST_BOTTOMLEFT | BORDERWINDOW_HITTEST_BOTTOMRIGHT) )
                        pData->maMouseOff.Y() += aNewRect.GetHeight()-nOldHeight;
                }
                else
                {
                    pData->mnTrackX        = aNewRect.Left();
                    pData->mnTrackY        = aNewRect.Top();
                    pData->mnTrackWidth    = aNewRect.GetWidth();
                    pData->mnTrackHeight   = aNewRect.GetHeight();
                    pBorderWindow->ShowTracking( Rectangle( pBorderWindow->ScreenToOutputPixel( aNewRect.TopLeft() ), aNewRect.GetSize() ), SHOWTRACK_BIG );
                }
            }
        }
    }

    return true;
}

OUString ImplBorderWindowView::ImplRequestHelp( ImplBorderFrameData* pData,
                                              const Point& rPos,
                                              Rectangle& rHelpRect )
{
    sal_uInt16 nHelpId = 0;
    OUString aHelpStr;
    sal_uInt16 nHitTest = ImplHitTest( pData, rPos );
    if ( nHitTest )
    {
        if ( nHitTest & BORDERWINDOW_HITTEST_CLOSE )
        {
            nHelpId     = SV_HELPTEXT_CLOSE;
            rHelpRect   = pData->maCloseRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_ROLL )
        {
            if ( pData->mpBorderWindow->mbRollUp )
                nHelpId = SV_HELPTEXT_ROLLDOWN;
            else
                nHelpId = SV_HELPTEXT_ROLLUP;
            rHelpRect   = pData->maRollRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_DOCK )
        {
            nHelpId     = SV_HELPTEXT_MAXIMIZE;
            rHelpRect   = pData->maDockRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_HIDE )
        {
            nHelpId     = SV_HELPTEXT_MINIMIZE;
            rHelpRect   = pData->maHideRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_HELP )
        {
            nHelpId     = SV_HELPTEXT_HELP;
            rHelpRect   = pData->maHelpRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_PIN )
        {
            nHelpId     = SV_HELPTEXT_ALWAYSVISIBLE;
            rHelpRect   = pData->maPinRect;
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_TITLE )
        {
            if( !pData->maTitleRect.IsEmpty() )
            {
                // tooltip only if title truncated
                if( pData->mbTitleClipped )
                {
                    rHelpRect   = pData->maTitleRect;
                    // no help id, use window title as help string
                    aHelpStr    = pData->mpBorderWindow->GetText();
                }
            }
        }
    }

    if( nHelpId && ImplGetResMgr() )
        aHelpStr = ResId(nHelpId, *ImplGetResMgr()).toString();

    return aHelpStr;
}

long ImplBorderWindowView::ImplCalcTitleWidth( const ImplBorderFrameData* pData ) const
{
    // title is not visible therefore no width
    if ( !pData->mnTitleHeight )
        return 0;

    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;
    long nTitleWidth = pBorderWindow->GetTextWidth( pBorderWindow->GetText() )+6;
    nTitleWidth += pData->maPinRect.GetWidth();
    nTitleWidth += pData->maCloseRect.GetWidth();
    nTitleWidth += pData->maRollRect.GetWidth();
    nTitleWidth += pData->maDockRect.GetWidth();
    nTitleWidth += pData->maMenuRect.GetWidth();
    nTitleWidth += pData->maHideRect.GetWidth();
    nTitleWidth += pData->maHelpRect.GetWidth();
    nTitleWidth += pData->mnLeftBorder+pData->mnRightBorder;
    return nTitleWidth;
}

// - ImplNoBorderWindowView -

ImplNoBorderWindowView::ImplNoBorderWindowView( ImplBorderWindow* )
{
}

void ImplNoBorderWindowView::Init( OutputDevice*, long, long )
{
}

void ImplNoBorderWindowView::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                        sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    rLeftBorder     = 0;
    rTopBorder      = 0;
    rRightBorder    = 0;
    rBottomBorder   = 0;
}

long ImplNoBorderWindowView::CalcTitleWidth() const
{
    return 0;
}

void ImplNoBorderWindowView::DrawWindow( sal_uInt16, OutputDevice*, const Point* )
{
}

// - ImplSmallBorderWindowView -
ImplSmallBorderWindowView::ImplSmallBorderWindowView( ImplBorderWindow* pBorderWindow )
    : mpBorderWindow(pBorderWindow)
    , mpOutDev(NULL)
    , mnWidth(0)
    , mnHeight(0)
    , mnLeftBorder(0)
    , mnTopBorder(0)
    , mnRightBorder(0)
    , mnBottomBorder(0)
    , mbNWFBorder(false)
{
}

void ImplSmallBorderWindowView::Init( OutputDevice* pDev, long nWidth, long nHeight )
{
    mpOutDev    = pDev;
    mnWidth     = nWidth;
    mnHeight    = nHeight;
    mbNWFBorder = false;

    vcl::Window *pWin = NULL, *pCtrl = NULL;
    if (mpOutDev->GetOutDevType() == OUTDEV_WINDOW)
        pWin = static_cast<vcl::Window*>(mpOutDev);

    if (pWin)
        pCtrl = mpBorderWindow->GetWindow(WINDOW_CLIENT);

    long nOrigLeftBorder = mnLeftBorder;
    long nOrigTopBorder = mnTopBorder;
    long nOrigRightBorder = mnRightBorder;
    long nOrigBottomBorder = mnBottomBorder;

    WindowBorderStyle nBorderStyle = mpBorderWindow->GetBorderStyle();
    if ( nBorderStyle & WindowBorderStyle::NOBORDER )
    {
        mnLeftBorder    = 0;
        mnTopBorder     = 0;
        mnRightBorder   = 0;
        mnBottomBorder  = 0;
    }
    else
    {
        // FIXME: this is currently only on OS X, check with other
        // platforms
        if( ImplGetSVData()->maNWFData.mbNoFocusRects && !( nBorderStyle & WindowBorderStyle::NWF ) )
        {
            // for native widget drawing we must find out what
            // control this border belongs to
            ControlType aCtrlType = 0;
            if (pCtrl)
            {
                switch( pCtrl->GetType() )
                {
                    case WINDOW_LISTBOX:
                        if( pCtrl->GetStyle() & WB_DROPDOWN )
                        {
                            aCtrlType = CTRL_LISTBOX;
                            mbNWFBorder = true;
                        }
                        break;
                    case WINDOW_COMBOBOX:
                        if( pCtrl->GetStyle() & WB_DROPDOWN )
                        {
                            aCtrlType = CTRL_COMBOBOX;
                            mbNWFBorder = true;
                        }
                        break;
                    case WINDOW_MULTILINEEDIT:
                        aCtrlType = CTRL_MULTILINE_EDITBOX;
                        mbNWFBorder = true;
                        break;
                    case WINDOW_EDIT:
                    case WINDOW_PATTERNFIELD:
                    case WINDOW_METRICFIELD:
                    case WINDOW_CURRENCYFIELD:
                    case WINDOW_DATEFIELD:
                    case WINDOW_TIMEFIELD:
                    case WINDOW_LONGCURRENCYFIELD:
                    case WINDOW_NUMERICFIELD:
                    case WINDOW_SPINFIELD:
                    case WINDOW_CALCINPUTLINE:
                        mbNWFBorder = true;
                        aCtrlType = (pCtrl->GetStyle() & WB_SPIN) ? CTRL_SPINBOX : CTRL_EDITBOX;
                        break;
                    default:
                        break;
                }
            }
            if( mbNWFBorder )
            {
                ImplControlValue aControlValue;
                Rectangle aCtrlRegion( (const Point&)Point(), Size( mnWidth < 10 ? 10 : mnWidth, mnHeight < 10 ? 10 : mnHeight ) );
                Rectangle aBounds( aCtrlRegion );
                Rectangle aContent( aCtrlRegion );
                if( pWin->GetNativeControlRegion( aCtrlType, PART_ENTIRE_CONTROL, aCtrlRegion,
                                                  CTRL_STATE_ENABLED, aControlValue, OUString(),
                                                  aBounds, aContent ) )
                {
                    mnLeftBorder    = aContent.Left() - aBounds.Left();
                    mnRightBorder   = aBounds.Right() - aContent.Right();
                    mnTopBorder     = aContent.Top() - aBounds.Top();
                    mnBottomBorder  = aBounds.Bottom() - aContent.Bottom();
                    if( mnWidth && mnHeight )
                    {

                        mpBorderWindow->SetPaintTransparent( true );
                        mpBorderWindow->SetBackground();
                        pCtrl->SetPaintTransparent( true );

                        vcl::Window* pCompoundParent = NULL;
                        if( pWin->GetParent() && pWin->GetParent()->IsCompoundControl() )
                            pCompoundParent = pWin->GetParent();

                        if( pCompoundParent )
                            pCompoundParent->SetPaintTransparent( true );

                        if( mnWidth < aBounds.GetWidth() || mnHeight < aBounds.GetHeight() )
                        {
                            if( ! pCompoundParent ) // compound controls have to fix themselves
                            {
                                Point aPos( mpBorderWindow->GetPosPixel() );
                                if( mnWidth < aBounds.GetWidth() )
                                    aPos.X() -= (aBounds.GetWidth() - mnWidth) / 2;
                                if( mnHeight < aBounds.GetHeight() )
                                    aPos.Y() -= (aBounds.GetHeight() - mnHeight) / 2;
                                mpBorderWindow->SetPosSizePixel( aPos, aBounds.GetSize() );
                            }
                        }
                    }
                }
                else
                    mbNWFBorder = false;
            }
        }

        if( ! mbNWFBorder )
        {
            sal_uInt16 nStyle = FRAME_DRAW_NODRAW;
            // move border outside if border was converted or if the BorderWindow is a frame window,
            if ( mpBorderWindow->mbSmallOutBorder )
                nStyle |= FRAME_DRAW_DOUBLEOUT;
            else if ( nBorderStyle & WindowBorderStyle::NWF )
                nStyle |= FRAME_DRAW_NWF;
            else
                nStyle |= FRAME_DRAW_DOUBLEIN;
            if ( nBorderStyle & WindowBorderStyle::MONO )
                nStyle |= FRAME_DRAW_MONO;

            DecorationView  aDecoView( mpOutDev );
            Rectangle       aRect( 0, 0, 10, 10 );
            Rectangle       aCalcRect = aDecoView.DrawFrame( aRect, nStyle );
            mnLeftBorder    = aCalcRect.Left();
            mnTopBorder     = aCalcRect.Top();
            mnRightBorder   = aRect.Right()-aCalcRect.Right();
            mnBottomBorder  = aRect.Bottom()-aCalcRect.Bottom();
        }
    }

    if (pCtrl)
    {
        //fdo#57090 If the borders have changed, then trigger a queue_resize on
        //the bordered window, which will resync its borders at that point
        if (nOrigLeftBorder != mnLeftBorder ||
            nOrigTopBorder != mnTopBorder ||
            nOrigRightBorder != mnRightBorder ||
            nOrigBottomBorder != mnBottomBorder)
        {
            pCtrl->queue_resize();
        }
    }
}

void ImplSmallBorderWindowView::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                           sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    rLeftBorder     = mnLeftBorder;
    rTopBorder      = mnTopBorder;
    rRightBorder    = mnRightBorder;
    rBottomBorder   = mnBottomBorder;
}

long ImplSmallBorderWindowView::CalcTitleWidth() const
{
    return 0;
}

void ImplSmallBorderWindowView::DrawWindow( sal_uInt16 nDrawFlags, OutputDevice*, const Point* )
{
    WindowBorderStyle nBorderStyle = mpBorderWindow->GetBorderStyle();
    if ( nBorderStyle & WindowBorderStyle::NOBORDER )
        return;

    bool bNativeOK = false;
    // for native widget drawing we must find out what
    // control this border belongs to
    vcl::Window *pWin = NULL, *pCtrl = NULL;
    if( mpOutDev->GetOutDevType() == OUTDEV_WINDOW )
        pWin = static_cast<vcl::Window*>(mpOutDev);

    ControlType aCtrlType = 0;
    ControlPart aCtrlPart = PART_ENTIRE_CONTROL;

    if( pWin && (pCtrl = mpBorderWindow->GetWindow( WINDOW_CLIENT )) != NULL )
    {
        switch( pCtrl->GetType() )
        {
            case WINDOW_MULTILINEEDIT:
                aCtrlType = CTRL_MULTILINE_EDITBOX;
                break;
            case WINDOW_EDIT:
            case WINDOW_PATTERNFIELD:
            case WINDOW_METRICFIELD:
            case WINDOW_CURRENCYFIELD:
            case WINDOW_DATEFIELD:
            case WINDOW_TIMEFIELD:
            case WINDOW_LONGCURRENCYFIELD:
            case WINDOW_NUMERICFIELD:
            case WINDOW_SPINFIELD:
            case WINDOW_CALCINPUTLINE:
                if( pCtrl->GetStyle() & WB_SPIN )
                    aCtrlType = CTRL_SPINBOX;
                else
                    aCtrlType = CTRL_EDITBOX;
                break;

            case WINDOW_LISTBOX:
            case WINDOW_MULTILISTBOX:
            case WINDOW_TREELISTBOX:
                aCtrlType = CTRL_LISTBOX;
                if( pCtrl->GetStyle() & WB_DROPDOWN )
                    aCtrlPart = PART_ENTIRE_CONTROL;
                else
                    aCtrlPart = PART_WINDOW;
                break;

            case WINDOW_LISTBOXWINDOW:
                aCtrlType = CTRL_LISTBOX;
                aCtrlPart = PART_WINDOW;
                break;

            case WINDOW_COMBOBOX:
            case WINDOW_PATTERNBOX:
            case WINDOW_NUMERICBOX:
            case WINDOW_METRICBOX:
            case WINDOW_CURRENCYBOX:
            case WINDOW_DATEBOX:
            case WINDOW_TIMEBOX:
            case WINDOW_LONGCURRENCYBOX:
                if( pCtrl->GetStyle() & WB_DROPDOWN )
                {
                    aCtrlType = CTRL_COMBOBOX;
                    aCtrlPart = PART_ENTIRE_CONTROL;
                }
                else
                {
                    aCtrlType = CTRL_LISTBOX;
                    aCtrlPart = PART_WINDOW;
                }
                break;

            default:
                break;
        }
    }

    if ( aCtrlType && pCtrl->IsNativeControlSupported(aCtrlType, aCtrlPart) )
    {
        ImplControlValue aControlValue;
        ControlState     nState = CTRL_STATE_ENABLED;

        if ( !pWin->IsEnabled() )
            nState &= ~CTRL_STATE_ENABLED;
        if ( pWin->HasFocus() )
            nState |= CTRL_STATE_FOCUSED;
        else if( mbNWFBorder )
        {
            // FIXME: this is curently only on OS X, see if other platforms can profit

            // FIXME: for OS X focus rings all controls need to support GetNativeControlRegion
            // for the dropdown style
            if( pCtrl->HasFocus() || pCtrl->HasChildPathFocus() )
                nState |= CTRL_STATE_FOCUSED;
        }

        bool bMouseOver = false;
        vcl::Window *pCtrlChild = pCtrl->GetWindow( WINDOW_FIRSTCHILD );
        while( pCtrlChild && !(bMouseOver = pCtrlChild->IsMouseOver()) )
            pCtrlChild = pCtrlChild->GetWindow( WINDOW_NEXT );

        if( bMouseOver )
            nState |= CTRL_STATE_ROLLOVER;

        Point aPoint;
        Rectangle aCtrlRegion( aPoint, Size( mnWidth, mnHeight ) );

        Rectangle aBoundingRgn( aPoint, Size( mnWidth, mnHeight ) );
        Rectangle aContentRgn( aCtrlRegion );
        if( ! ImplGetSVData()->maNWFData.mbCanDrawWidgetAnySize &&
            pWin->GetNativeControlRegion( aCtrlType, aCtrlPart, aCtrlRegion,
                                          nState, aControlValue, OUString(),
                                          aBoundingRgn, aContentRgn ))
        {
            aCtrlRegion=aContentRgn;
        }

        bNativeOK = pWin->DrawNativeControl( aCtrlType, aCtrlPart, aCtrlRegion, nState,
                aControlValue, OUString() );

        // if the native theme draws the spinbuttons in one call, make sure the proper settings
        // are passed, this might force a redraw though.... (TODO: improve)
        if ( (aCtrlType == CTRL_SPINBOX) && !pCtrl->IsNativeControlSupported( CTRL_SPINBOX, PART_BUTTON_UP ) )
        {
            Edit *pEdit = static_cast<Edit*>(pCtrl)->GetSubEdit();
            if ( pEdit )
                pCtrl->Paint( Rectangle() );  // make sure the buttons are also drawn as they might overwrite the border
        }
    }

    if( bNativeOK )
        return;

    if ( nDrawFlags & BORDERWINDOW_DRAW_FRAME )
    {
        sal_uInt16 nStyle = 0;
        // move border outside if border was converted or if the border window is a frame window,
        if ( mpBorderWindow->mbSmallOutBorder )
            nStyle |= FRAME_DRAW_DOUBLEOUT;
        else if ( nBorderStyle & WindowBorderStyle::NWF )
            nStyle |= FRAME_DRAW_NWF;
        else
            nStyle |= FRAME_DRAW_DOUBLEIN;
        if ( nBorderStyle & WindowBorderStyle::MONO )
            nStyle |= FRAME_DRAW_MONO;
        if ( nBorderStyle & WindowBorderStyle::MENU )
            nStyle |= FRAME_DRAW_MENU;
        // tell DrawFrame that we're drawing a window border of a frame window to avoid round corners
        if( pWin && pWin == pWin->ImplGetFrameWindow() )
            nStyle |= FRAME_DRAW_WINDOWBORDER;

        DecorationView  aDecoView( mpOutDev );
        Point           aTmpPoint;
        Rectangle       aInRect( aTmpPoint, Size( mnWidth, mnHeight ) );
        aDecoView.DrawFrame( aInRect, nStyle );
    }
}

// - ImplStdBorderWindowView -

ImplStdBorderWindowView::ImplStdBorderWindowView( ImplBorderWindow* pBorderWindow )
{
    maFrameData.mpBorderWindow  = pBorderWindow;
    maFrameData.mbDragFull      = false;
    maFrameData.mnHitTest       = 0;
    maFrameData.mnPinState      = 0;
    maFrameData.mnCloseState    = 0;
    maFrameData.mnRollState     = 0;
    maFrameData.mnDockState     = 0;
    maFrameData.mnMenuState     = 0;
    maFrameData.mnHideState     = 0;
    maFrameData.mnHelpState     = 0;
    maFrameData.mbTitleClipped  = false;

    mpATitleVirDev              = NULL;
    mpDTitleVirDev              = NULL;
}

ImplStdBorderWindowView::~ImplStdBorderWindowView()
{
    delete mpATitleVirDev;
    delete mpDTitleVirDev;
}

bool ImplStdBorderWindowView::MouseMove( const MouseEvent& rMEvt )
{
    return ImplMouseMove( &maFrameData, rMEvt );
}

bool ImplStdBorderWindowView::MouseButtonDown( const MouseEvent& rMEvt )
{
    return ImplMouseButtonDown( &maFrameData, rMEvt );
}

bool ImplStdBorderWindowView::Tracking( const TrackingEvent& rTEvt )
{
    return ImplTracking( &maFrameData, rTEvt );
}

OUString ImplStdBorderWindowView::RequestHelp( const Point& rPos, Rectangle& rHelpRect )
{
    return ImplRequestHelp( &maFrameData, rPos, rHelpRect );
}

Rectangle ImplStdBorderWindowView::GetMenuRect() const
{
    return maFrameData.maMenuRect;
}

void ImplStdBorderWindowView::Init( OutputDevice* pDev, long nWidth, long nHeight )
{
    ImplBorderFrameData*    pData = &maFrameData;
    ImplBorderWindow*       pBorderWindow = maFrameData.mpBorderWindow;
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();
    DecorationView          aDecoView( pDev );
    Rectangle               aRect( 0, 0, 10, 10 );
    Rectangle               aCalcRect = aDecoView.DrawFrame( aRect, FRAME_DRAW_DOUBLEOUT | FRAME_DRAW_NODRAW );

    pData->mpOutDev         = pDev;
    pData->mnWidth          = nWidth;
    pData->mnHeight         = nHeight;

    pData->mnTitleType      = pBorderWindow->mnTitleType;
    pData->mbFloatWindow    = pBorderWindow->mbFloatWindow;

    if ( !(pBorderWindow->GetStyle() & (WB_MOVEABLE | WB_POPUP)) || (pData->mnTitleType == BORDERWINDOW_TITLE_NONE) )
        pData->mnBorderSize = 0;
    else if ( pData->mnTitleType == BORDERWINDOW_TITLE_TEAROFF )
        pData->mnBorderSize = 0;
    else
        pData->mnBorderSize = rStyleSettings.GetBorderSize();
    pData->mnLeftBorder     = aCalcRect.Left();
    pData->mnTopBorder      = aCalcRect.Top();
    pData->mnRightBorder    = aRect.Right()-aCalcRect.Right();
    pData->mnBottomBorder   = aRect.Bottom()-aCalcRect.Bottom();
    pData->mnLeftBorder    += pData->mnBorderSize;
    pData->mnTopBorder     += pData->mnBorderSize;
    pData->mnRightBorder   += pData->mnBorderSize;
    pData->mnBottomBorder  += pData->mnBorderSize;
    pData->mnNoTitleTop     = pData->mnTopBorder;

    ImplInitTitle( &maFrameData );
    if ( pData->mnTitleHeight )
    {
        // to improve symbol display force a minum title height
        if( pData->mnTitleHeight < MIN_CAPTION_HEIGHT )
            pData->mnTitleHeight = MIN_CAPTION_HEIGHT;

        // set a proper background for drawing
        // highlighted buttons in the title
        pBorderWindow->SetBackground( rStyleSettings.GetFaceColor() );

        pData->maTitleRect.Left()    = pData->mnLeftBorder;
        pData->maTitleRect.Right()   = nWidth-pData->mnRightBorder-1;
        pData->maTitleRect.Top()     = pData->mnTopBorder;
        pData->maTitleRect.Bottom()  = pData->maTitleRect.Top()+pData->mnTitleHeight-1;

        if ( pData->mnTitleType & (BORDERWINDOW_TITLE_NORMAL | BORDERWINDOW_TITLE_SMALL) )
        {
            long nLeft          = pData->maTitleRect.Left();
            long nRight         = pData->maTitleRect.Right();
            long nItemTop       = pData->maTitleRect.Top();
            long nItemBottom    = pData->maTitleRect.Bottom();
            nLeft              += 1;
            nRight             -= 3;
            nItemTop           += 2;
            nItemBottom        -= 2;

            if ( pBorderWindow->GetStyle() & WB_PINABLE )
            {
                Image aImage;
                ImplGetPinImage( 0, false, aImage );
                pData->maPinRect.Top()    = nItemTop;
                pData->maPinRect.Bottom() = nItemBottom;
                pData->maPinRect.Left()   = nLeft;
                pData->maPinRect.Right()  = pData->maPinRect.Left()+aImage.GetSizePixel().Width();
                nLeft += pData->maPinRect.GetWidth()+3;
            }

            if ( pBorderWindow->GetStyle() & WB_CLOSEABLE )
            {
                pData->maCloseRect.Top()    = nItemTop;
                pData->maCloseRect.Bottom() = nItemBottom;
                pData->maCloseRect.Right()  = nRight;
                pData->maCloseRect.Left()   = pData->maCloseRect.Right()-pData->maCloseRect.GetHeight()+1;
                nRight -= pData->maCloseRect.GetWidth()+3;
            }

            if ( pBorderWindow->mbMenuBtn )
            {
                pData->maMenuRect.Top()    = nItemTop;
                pData->maMenuRect.Bottom() = nItemBottom;
                pData->maMenuRect.Right()  = nRight;
                pData->maMenuRect.Left()   = pData->maMenuRect.Right()-pData->maMenuRect.GetHeight()+1;
                nRight -= pData->maMenuRect.GetWidth();
            }

            if ( pBorderWindow->mbDockBtn )
            {
                pData->maDockRect.Top()    = nItemTop;
                pData->maDockRect.Bottom() = nItemBottom;
                pData->maDockRect.Right()  = nRight;
                pData->maDockRect.Left()   = pData->maDockRect.Right()-pData->maDockRect.GetHeight()+1;
                nRight -= pData->maDockRect.GetWidth();
                if ( !pBorderWindow->mbHideBtn &&
                     !(pBorderWindow->GetStyle() & WB_ROLLABLE) )
                    nRight -= 3;
            }

            if ( pBorderWindow->mbHideBtn )
            {
                pData->maHideRect.Top()    = nItemTop;
                pData->maHideRect.Bottom() = nItemBottom;
                pData->maHideRect.Right()  = nRight;
                pData->maHideRect.Left()   = pData->maHideRect.Right()-pData->maHideRect.GetHeight()+1;
                nRight -= pData->maHideRect.GetWidth();
                if ( !(pBorderWindow->GetStyle() & WB_ROLLABLE) )
                    nRight -= 3;
            }

            if ( pBorderWindow->GetStyle() & WB_ROLLABLE )
            {
                pData->maRollRect.Top()    = nItemTop;
                pData->maRollRect.Bottom() = nItemBottom;
                pData->maRollRect.Right()  = nRight;
                pData->maRollRect.Left()   = pData->maRollRect.Right()-pData->maRollRect.GetHeight()+1;
                nRight -= pData->maRollRect.GetWidth();
            }
        }
        else
        {
            pData->maPinRect.SetEmpty();
            pData->maCloseRect.SetEmpty();
            pData->maDockRect.SetEmpty();
            pData->maMenuRect.SetEmpty();
            pData->maHideRect.SetEmpty();
            pData->maRollRect.SetEmpty();
            pData->maHelpRect.SetEmpty();
        }

        pData->mnTopBorder  += pData->mnTitleHeight;
    }
    else
    {
        pData->maTitleRect.SetEmpty();
        pData->maPinRect.SetEmpty();
        pData->maCloseRect.SetEmpty();
        pData->maDockRect.SetEmpty();
        pData->maMenuRect.SetEmpty();
        pData->maHideRect.SetEmpty();
        pData->maRollRect.SetEmpty();
        pData->maHelpRect.SetEmpty();
    }
}

void ImplStdBorderWindowView::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                         sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    rLeftBorder     = maFrameData.mnLeftBorder;
    rTopBorder      = maFrameData.mnTopBorder;
    rRightBorder    = maFrameData.mnRightBorder;
    rBottomBorder   = maFrameData.mnBottomBorder;
}

long ImplStdBorderWindowView::CalcTitleWidth() const
{
    return ImplCalcTitleWidth( &maFrameData );
}

void ImplStdBorderWindowView::DrawWindow( sal_uInt16 nDrawFlags, OutputDevice* pOutDev, const Point* pOffset )
{
    ImplBorderFrameData*    pData = &maFrameData;
    OutputDevice*           pDev = pOutDev ? pOutDev : pData->mpOutDev;
    ImplBorderWindow*       pBorderWindow = pData->mpBorderWindow;
    Point                   aTmpPoint = pOffset ? Point(*pOffset) : Point();
    Rectangle               aInRect( aTmpPoint, Size( pData->mnWidth, pData->mnHeight ) );
    const StyleSettings&    rStyleSettings = pData->mpOutDev->GetSettings().GetStyleSettings();
    DecorationView          aDecoView( pDev );
    Color                   aFaceColor( rStyleSettings.GetFaceColor() );
    Color                   aFrameColor( aFaceColor );

    aFrameColor.DecreaseContrast( (sal_uInt8) (0.50 * 255));

    // Draw Frame
    if ( nDrawFlags & BORDERWINDOW_DRAW_FRAME )
    {
        vcl::Region oldClipRgn( pDev->GetClipRegion( ) );

        // for popups, don't draw part of the frame
        if ( pData->mnTitleType == BORDERWINDOW_TITLE_POPUP )
        {
            FloatingWindow *pWin = dynamic_cast< FloatingWindow* >( pData->mpBorderWindow->GetWindow( WINDOW_CLIENT ) );
            if ( pWin )
            {
                vcl::Region aClipRgn( aInRect );
                Rectangle aItemClipRect( pWin->ImplGetItemEdgeClipRect() );
                if( !aItemClipRect.IsEmpty() )
                {
                    aItemClipRect.SetPos( pData->mpBorderWindow->AbsoluteScreenToOutputPixel( aItemClipRect.TopLeft() ) );
                    aClipRgn.Exclude( aItemClipRect );
                    pDev->SetClipRegion( aClipRgn );
                }
            }
        }

        // single line frame
        pDev->SetLineColor( aFrameColor );
        pDev->SetFillColor();
        pDev->DrawRect( aInRect );
        ++aInRect.Left();
        --aInRect.Right();
        ++aInRect.Top();
        --aInRect.Bottom();

        // restore
        if ( pData->mnTitleType == BORDERWINDOW_TITLE_POPUP )
            pDev->SetClipRegion( oldClipRgn );
    }
    else
        aInRect = aDecoView.DrawFrame( aInRect, FRAME_DRAW_DOUBLEOUT | FRAME_DRAW_NODRAW);

    // Draw Border
    pDev->SetLineColor();
    long nBorderSize = pData->mnBorderSize;
    if ( (nDrawFlags & BORDERWINDOW_DRAW_BORDER) && nBorderSize )
    {
        pDev->SetFillColor( rStyleSettings.GetFaceColor() );
        pDev->DrawRect( Rectangle( Point( aInRect.Left(), aInRect.Top() ),
                                   Size( aInRect.GetWidth(), nBorderSize ) ) );
        pDev->DrawRect( Rectangle( Point( aInRect.Left(), aInRect.Top()+nBorderSize ),
                                   Size( nBorderSize, aInRect.GetHeight()-nBorderSize ) ) );
        pDev->DrawRect( Rectangle( Point( aInRect.Left(), aInRect.Bottom()-nBorderSize+1 ),
                                   Size( aInRect.GetWidth(), nBorderSize ) ) );
        pDev->DrawRect( Rectangle( Point( aInRect.Right()-nBorderSize+1, aInRect.Top()+nBorderSize ),
                                   Size( nBorderSize, aInRect.GetHeight()-nBorderSize ) ) );
    }

    // Draw Title
    if ( (nDrawFlags & BORDERWINDOW_DRAW_TITLE) && !pData->maTitleRect.IsEmpty() )
    {
        aInRect = pData->maTitleRect;

        // use no gradient anymore, just a static titlecolor
        if ( pData->mnTitleType != BORDERWINDOW_TITLE_POPUP )
            pDev->SetFillColor( aFrameColor );
        else
            pDev->SetFillColor( aFaceColor );

        pDev->SetTextColor( rStyleSettings.GetButtonTextColor() );
        Rectangle aTitleRect( pData->maTitleRect );
        if( pOffset )
            aTitleRect.Move( pOffset->X(), pOffset->Y() );
        pDev->DrawRect( aTitleRect );

        if ( pData->mnTitleType != BORDERWINDOW_TITLE_TEAROFF )
        {
            aInRect.Left()  += 2;
            aInRect.Right() -= 2;

            if ( !pData->maPinRect.IsEmpty() )
                aInRect.Left() = pData->maPinRect.Right()+2;

            if ( !pData->maHelpRect.IsEmpty() )
                aInRect.Right() = pData->maHelpRect.Left()-2;
            else if ( !pData->maRollRect.IsEmpty() )
                aInRect.Right() = pData->maRollRect.Left()-2;
            else if ( !pData->maHideRect.IsEmpty() )
                aInRect.Right() = pData->maHideRect.Left()-2;
            else if ( !pData->maDockRect.IsEmpty() )
                aInRect.Right() = pData->maDockRect.Left()-2;
            else if ( !pData->maMenuRect.IsEmpty() )
                aInRect.Right() = pData->maMenuRect.Left()-2;
            else if ( !pData->maCloseRect.IsEmpty() )
                aInRect.Right() = pData->maCloseRect.Left()-2;

            if ( pOffset )
                aInRect.Move( pOffset->X(), pOffset->Y() );

            sal_uInt16 nTextStyle = TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER | TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_CLIP;

            // must show tooltip ?
            TextRectInfo aInfo;
            pDev->GetTextRect( aInRect, pBorderWindow->GetText(), nTextStyle, &aInfo );
            pData->mbTitleClipped = aInfo.IsEllipses();

            pDev->DrawText( aInRect, pBorderWindow->GetText(), nTextStyle );
        }
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_CLOSE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maCloseRect.IsEmpty() )
    {
        Rectangle aSymbolRect( pData->maCloseRect );
        if ( pOffset )
            aSymbolRect.Move( pOffset->X(), pOffset->Y() );
        ImplDrawBrdWinSymbolButton( pDev, aSymbolRect, SymbolType::CLOSE, pData->mnCloseState );
    }
    if ( ((nDrawFlags & BORDERWINDOW_DRAW_DOCK) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maDockRect.IsEmpty() )
    {
        Rectangle aSymbolRect( pData->maDockRect );
        if ( pOffset )
            aSymbolRect.Move( pOffset->X(), pOffset->Y() );
        ImplDrawBrdWinSymbolButton( pDev, aSymbolRect, SymbolType::DOCK, pData->mnDockState );
    }
    if ( ((nDrawFlags & BORDERWINDOW_DRAW_MENU) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maMenuRect.IsEmpty() )
    {
        Rectangle aSymbolRect( pData->maMenuRect );
        if ( pOffset )
            aSymbolRect.Move( pOffset->X(), pOffset->Y() );
        ImplDrawBrdWinSymbolButton( pDev, aSymbolRect, SymbolType::MENU, pData->mnMenuState );
    }
    if ( ((nDrawFlags & BORDERWINDOW_DRAW_HIDE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maHideRect.IsEmpty() )
    {
        Rectangle aSymbolRect( pData->maHideRect );
        if ( pOffset )
            aSymbolRect.Move( pOffset->X(), pOffset->Y() );
        ImplDrawBrdWinSymbolButton( pDev, aSymbolRect, SymbolType::HIDE, pData->mnHideState );
    }
    if ( ((nDrawFlags & BORDERWINDOW_DRAW_ROLL) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maRollRect.IsEmpty() )
    {
        SymbolType eType;
        if ( pBorderWindow->mbRollUp )
            eType = SymbolType::ROLLDOWN;
        else
            eType = SymbolType::ROLLUP;
        Rectangle aSymbolRect( pData->maRollRect );
        if ( pOffset )
            aSymbolRect.Move( pOffset->X(), pOffset->Y() );
        ImplDrawBrdWinSymbolButton( pDev, aSymbolRect, eType, pData->mnRollState );
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_HELP) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maHelpRect.IsEmpty() )
    {
        Rectangle aSymbolRect( pData->maHelpRect );
        if ( pOffset )
            aSymbolRect.Move( pOffset->X(), pOffset->Y() );
        ImplDrawBrdWinSymbolButton( pDev, aSymbolRect, SymbolType::HELP, pData->mnHelpState );
    }
    if ( ((nDrawFlags & BORDERWINDOW_DRAW_PIN) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maPinRect.IsEmpty() )
    {
        Image aImage;
        ImplGetPinImage( pData->mnPinState, pBorderWindow->mbPinned, aImage );
        Size  aImageSize = aImage.GetSizePixel();
        long  nRectHeight = pData->maPinRect.GetHeight();
        Point aPos( pData->maPinRect.TopLeft() );
        if ( pOffset )
            aPos.Move( pOffset->X(), pOffset->Y() );
        if ( nRectHeight < aImageSize.Height() )
        {
            pDev->DrawImage( aPos, Size( aImageSize.Width(), nRectHeight ), aImage );
        }
        else
        {
            aPos.Y() += (nRectHeight-aImageSize.Height())/2;
            pDev->DrawImage( aPos, aImage );
        }
    }
}

void ImplBorderWindow::ImplInit( vcl::Window* pParent,
                                 WinBits nStyle, sal_uInt16 nTypeStyle,
                                 const ::com::sun::star::uno::Any& )
{
    ImplInit( pParent, nStyle, nTypeStyle, NULL );
}

void ImplBorderWindow::ImplInit( vcl::Window* pParent,
                                 WinBits nStyle, sal_uInt16 nTypeStyle,
                                 SystemParentData* pSystemParentData
                                 )
{
    // remove all unwanted WindowBits
    WinBits nOrgStyle = nStyle;
    WinBits nTestStyle = (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_PINABLE | WB_CLOSEABLE | WB_STANDALONE | WB_DIALOGCONTROL | WB_NODIALOGCONTROL | WB_SYSTEMFLOATWIN | WB_INTROWIN | WB_DEFAULTWIN | WB_TOOLTIPWIN | WB_NOSHADOW | WB_OWNERDRAWDECORATION | WB_SYSTEMCHILDWINDOW  | WB_NEEDSFOCUS | WB_POPUP);
    if ( nTypeStyle & BORDERWINDOW_STYLE_APP )
        nTestStyle |= WB_APP;
    nStyle &= nTestStyle;

    mpWindowImpl->mbBorderWin       = true;
    mbSmallOutBorder    = false;
    if ( nTypeStyle & BORDERWINDOW_STYLE_FRAME )
    {
        if( (nStyle & WB_SYSTEMCHILDWINDOW) )
        {
            mpWindowImpl->mbOverlapWin  = true;
            mpWindowImpl->mbFrame       = true;
            mbFrameBorder               = false;
        }
        else if( (nStyle & (WB_OWNERDRAWDECORATION | WB_POPUP)) )
        {
            mpWindowImpl->mbOverlapWin  = true;
            mpWindowImpl->mbFrame       = true;
            mbFrameBorder   = (nOrgStyle & WB_NOBORDER) ? false : true;
        }
        else
        {
            mpWindowImpl->mbOverlapWin  = true;
            mpWindowImpl->mbFrame       = true;
            mbFrameBorder   = false;
            // closeable windows may have a border as well, eg. system floating windows without caption
            if ( (nOrgStyle & (WB_BORDER | WB_NOBORDER | WB_MOVEABLE | WB_SIZEABLE/* | WB_CLOSEABLE*/)) == WB_BORDER )
                mbSmallOutBorder = true;
        }
    }
    else if ( nTypeStyle & BORDERWINDOW_STYLE_OVERLAP )
    {
        mpWindowImpl->mbOverlapWin  = true;
        mbFrameBorder   = true;
    }
    else
        mbFrameBorder   = false;

    if ( nTypeStyle & BORDERWINDOW_STYLE_FLOAT )
        mbFloatWindow = true;
    else
        mbFloatWindow = false;

    Window::ImplInit( pParent, nStyle, pSystemParentData );
    SetBackground();
    SetTextFillColor();

    mpMenuBarWindow = NULL;
    mnMinWidth      = 0;
    mnMinHeight     = 0;
    mnMaxWidth      = SHRT_MAX;
    mnMaxHeight     = SHRT_MAX;
    mnRollHeight    = 0;
    mnOrgMenuHeight = 0;
    mbPinned        = false;
    mbRollUp        = false;
    mbMenuHide      = false;
    mbDockBtn       = false;
    mbMenuBtn       = false;
    mbHideBtn       = false;
    mbDisplayActive = IsActive();

    if ( nTypeStyle & BORDERWINDOW_STYLE_FLOAT )
        mnTitleType = BORDERWINDOW_TITLE_SMALL;
    else
        mnTitleType = BORDERWINDOW_TITLE_NORMAL;
    mnBorderStyle   = WindowBorderStyle::NORMAL;
    InitView();
}

ImplBorderWindow::ImplBorderWindow( vcl::Window* pParent,
                                    SystemParentData* pSystemParentData,
                                    WinBits nStyle, sal_uInt16 nTypeStyle
                                    ) : Window( WINDOW_BORDERWINDOW )
{
    ImplInit( pParent, nStyle, nTypeStyle, pSystemParentData );
}

ImplBorderWindow::ImplBorderWindow( vcl::Window* pParent, WinBits nStyle ,
                                    sal_uInt16 nTypeStyle ) :
    Window( WINDOW_BORDERWINDOW )
{
    ImplInit( pParent, nStyle, nTypeStyle, ::com::sun::star::uno::Any() );
}

ImplBorderWindow::~ImplBorderWindow()
{
    delete mpBorderView;
}

void ImplBorderWindow::MouseMove( const MouseEvent& rMEvt )
{
    mpBorderView->MouseMove( rMEvt );
}

void ImplBorderWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    mpBorderView->MouseButtonDown( rMEvt );
}

void ImplBorderWindow::Tracking( const TrackingEvent& rTEvt )
{
    mpBorderView->Tracking( rTEvt );
}

void ImplBorderWindow::Paint( const Rectangle& )
{
    mpBorderView->DrawWindow( BORDERWINDOW_DRAW_ALL );
}

void ImplBorderWindow::Draw( const Rectangle&, OutputDevice* pOutDev, const Point& rPos )
{
    mpBorderView->DrawWindow( BORDERWINDOW_DRAW_ALL, pOutDev, &rPos );
}

void ImplBorderWindow::Activate()
{
    SetDisplayActive( true );
    Window::Activate();
}

void ImplBorderWindow::Deactivate()
{
    // remove active windows from the ruler, also ignore the Deactivate
    // if a menu becomes active
    if ( GetActivateMode() && !ImplGetSVData()->maWinData.mbNoDeactivate )
        SetDisplayActive( false );
    Window::Deactivate();
}

void ImplBorderWindow::RequestHelp( const HelpEvent& rHEvt )
{
    // no keyboard help for border window
    if ( rHEvt.GetMode() & (HelpEventMode::BALLOON | HelpEventMode::QUICK) && !rHEvt.KeyboardActivated() )
    {
        Point       aMousePosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        Rectangle   aHelpRect;
        OUString    aHelpStr( mpBorderView->RequestHelp( aMousePosPixel, aHelpRect ) );

        // retrieve rectangle
        if ( !aHelpStr.isEmpty() )
        {
            aHelpRect.SetPos( OutputToScreenPixel( aHelpRect.TopLeft() ) );
            if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
                Help::ShowBalloon( this, aHelpRect.Center(), aHelpRect, aHelpStr );
            else
                Help::ShowQuickHelp( this, aHelpRect, aHelpStr );
            return;
        }
    }

    Window::RequestHelp( rHEvt );
}

void ImplBorderWindow::Resize()
{
    Size aSize = GetOutputSizePixel();

    if ( !mbRollUp )
    {
        vcl::Window* pClientWindow = ImplGetClientWindow();

        if ( mpMenuBarWindow )
        {
            sal_Int32 nLeftBorder;
            sal_Int32 nTopBorder;
            sal_Int32 nRightBorder;
            sal_Int32 nBottomBorder;
            long nMenuHeight = mpMenuBarWindow->GetSizePixel().Height();
            if ( mbMenuHide )
            {
                if ( nMenuHeight )
                    mnOrgMenuHeight = nMenuHeight;
                nMenuHeight = 0;
            }
            else
            {
                if ( !nMenuHeight )
                    nMenuHeight = mnOrgMenuHeight;
            }
            mpBorderView->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
            mpMenuBarWindow->setPosSizePixel( nLeftBorder,
                                              nTopBorder,
                                              aSize.Width()-nLeftBorder-nRightBorder,
                                              nMenuHeight,
                                              WINDOW_POSSIZE_POS |
                                              WINDOW_POSSIZE_WIDTH | WINDOW_POSSIZE_HEIGHT );
        }

        GetBorder( pClientWindow->mpWindowImpl->mnLeftBorder, pClientWindow->mpWindowImpl->mnTopBorder,
                   pClientWindow->mpWindowImpl->mnRightBorder, pClientWindow->mpWindowImpl->mnBottomBorder );
        pClientWindow->ImplPosSizeWindow( pClientWindow->mpWindowImpl->mnLeftBorder,
                                          pClientWindow->mpWindowImpl->mnTopBorder,
                                          aSize.Width()-pClientWindow->mpWindowImpl->mnLeftBorder-pClientWindow->mpWindowImpl->mnRightBorder,
                                          aSize.Height()-pClientWindow->mpWindowImpl->mnTopBorder-pClientWindow->mpWindowImpl->mnBottomBorder,
                                          WINDOW_POSSIZE_X | WINDOW_POSSIZE_Y |
                                          WINDOW_POSSIZE_WIDTH | WINDOW_POSSIZE_HEIGHT );
    }

    // UpdateView
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();

    Window::Resize();
}

void ImplBorderWindow::StateChanged( StateChangedType nType )
{
    if ( (nType == StateChangedType::TEXT) ||
         (nType == StateChangedType::IMAGE) ||
         (nType == StateChangedType::DATA) )
    {
        if ( IsReallyVisible() && mbFrameBorder )
        {
            if ( HasPaintEvent() )
                InvalidateBorder();
            else
                mpBorderView->DrawWindow( BORDERWINDOW_DRAW_TITLE );
        }
    }

    Window::StateChanged( nType );
}

void ImplBorderWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        if ( !mpWindowImpl->mbFrame || (GetStyle() & (WB_OWNERDRAWDECORATION | WB_POPUP)) )
            UpdateView( true, ImplGetWindow()->GetOutputSizePixel() );
    }

    Window::DataChanged( rDCEvt );
}

void ImplBorderWindow::InitView()
{
    if ( mbSmallOutBorder )
        mpBorderView = new ImplSmallBorderWindowView( this );
    else if ( mpWindowImpl->mbFrame )
    {
        if( mbFrameBorder )
            mpBorderView = new ImplStdBorderWindowView( this );
        else
            mpBorderView = new ImplNoBorderWindowView( this );
    }
    else if ( !mbFrameBorder )
        mpBorderView = new ImplSmallBorderWindowView( this );
    else
        mpBorderView = new ImplStdBorderWindowView( this );
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
}

void ImplBorderWindow::UpdateView( bool bNewView, const Size& rNewOutSize )
{
    sal_Int32 nLeftBorder;
    sal_Int32 nTopBorder;
    sal_Int32 nRightBorder;
    sal_Int32 nBottomBorder;
    Size aOldSize = GetSizePixel();
    Size aOutputSize = rNewOutSize;

    if ( bNewView )
    {
        delete mpBorderView;
        InitView();
    }
    else
    {
        Size aSize = aOutputSize;
        mpBorderView->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
        aSize.Width()  += nLeftBorder+nRightBorder;
        aSize.Height() += nTopBorder+nBottomBorder;
        mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    }

    vcl::Window* pClientWindow = ImplGetClientWindow();
    if ( pClientWindow )
    {
        GetBorder( pClientWindow->mpWindowImpl->mnLeftBorder, pClientWindow->mpWindowImpl->mnTopBorder,
                   pClientWindow->mpWindowImpl->mnRightBorder, pClientWindow->mpWindowImpl->mnBottomBorder );
    }
    GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
    if ( aOldSize.Width() || aOldSize.Height() )
    {
        aOutputSize.Width()     += nLeftBorder+nRightBorder;
        aOutputSize.Height()    += nTopBorder+nBottomBorder;
        if ( aOutputSize == GetSizePixel() )
            InvalidateBorder();
        else
            SetSizePixel( aOutputSize );
    }
}

void ImplBorderWindow::InvalidateBorder()
{
    if ( IsReallyVisible() )
    {
        // invalidate only if we have a border
        sal_Int32 nLeftBorder;
        sal_Int32 nTopBorder;
        sal_Int32 nRightBorder;
        sal_Int32 nBottomBorder;
        mpBorderView->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
        if ( nLeftBorder || nTopBorder || nRightBorder || nBottomBorder )
        {
            Rectangle   aWinRect( Point( 0, 0 ), GetOutputSizePixel() );
            vcl::Region      aRegion( aWinRect );
            aWinRect.Left()   += nLeftBorder;
            aWinRect.Top()    += nTopBorder;
            aWinRect.Right()  -= nRightBorder;
            aWinRect.Bottom() -= nBottomBorder;
            // no output area anymore, now invalidate all
            if ( (aWinRect.Right() < aWinRect.Left()) ||
                 (aWinRect.Bottom() < aWinRect.Top()) )
                Invalidate( INVALIDATE_NOCHILDREN );
            else
            {
                aRegion.Exclude( aWinRect );
                Invalidate( aRegion, INVALIDATE_NOCHILDREN );
            }
        }
    }
}

void ImplBorderWindow::SetDisplayActive( bool bActive )
{
    if ( mbDisplayActive != bActive )
    {
        mbDisplayActive = bActive;
        if ( mbFrameBorder )
            InvalidateBorder();
    }
}

void ImplBorderWindow::SetTitleType( sal_uInt16 nTitleType, const Size& rSize )
{
    mnTitleType = nTitleType;
    UpdateView( false, rSize );
}

void ImplBorderWindow::SetBorderStyle( WindowBorderStyle nStyle )
{
    if ( !mbFrameBorder && (mnBorderStyle != nStyle) )
    {
        mnBorderStyle = nStyle;
        UpdateView( false, ImplGetWindow()->GetOutputSizePixel() );
    }
}

void ImplBorderWindow::SetPin( bool bPin )
{
    mbPinned = bPin;
    InvalidateBorder();
}

void ImplBorderWindow::SetRollUp( bool bRollUp, const Size& rSize )
{
    mbRollUp = bRollUp;
    mnRollHeight = rSize.Height();
    UpdateView( false, rSize );
}

void ImplBorderWindow::SetCloseButton()
{
    SetStyle( GetStyle() | WB_CLOSEABLE );
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::SetDockButton( bool bDockButton )
{
    mbDockBtn = bDockButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::SetHideButton( bool bHideButton )
{
    mbHideBtn = bHideButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::SetMenuButton( bool bMenuButton )
{
    mbMenuBtn = bMenuButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

void ImplBorderWindow::UpdateMenuHeight()
{
    Resize();
}

void ImplBorderWindow::SetMenuBarWindow( vcl::Window* pWindow )
{
    mpMenuBarWindow = pWindow;
    UpdateMenuHeight();
    if ( pWindow )
        pWindow->Show();
}

void ImplBorderWindow::SetMenuBarMode( bool bHide )
{
    mbMenuHide = bHide;
    UpdateMenuHeight();
}

void ImplBorderWindow::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                  sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    mpBorderView->GetBorder( rLeftBorder, rTopBorder, rRightBorder, rBottomBorder );
    if ( mpMenuBarWindow && !mbMenuHide )
        rTopBorder += mpMenuBarWindow->GetSizePixel().Height();
}

long ImplBorderWindow::CalcTitleWidth() const
{
    return mpBorderView->CalcTitleWidth();
}

Rectangle ImplBorderWindow::GetMenuRect() const
{
    return mpBorderView->GetMenuRect();
}

Size ImplBorderWindow::GetOptimalSize() const
{
    const vcl::Window* pClientWindow = ImplGetClientWindow();
    if (pClientWindow)
        return pClientWindow->GetOptimalSize();
    return Size(mnMinWidth, mnMinHeight);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
