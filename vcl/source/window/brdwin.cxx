/*************************************************************************
 *
 *  $RCSfile: brdwin.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2001-02-16 11:10:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_BRDWIN_CXX

#ifdef REMOTE_APPSERVER
#include <rmwindow.hxx>
#endif

#ifndef _SV_SVIDS_HRC
#include <svids.hrc>
#endif
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_DECOVIEW_HXX
#include <decoview.hxx>
#endif
#ifndef _SV_SYSWIN_HXX
#include <syswin.hxx>
#endif
#ifndef _SV_DOCKWIN_HXX
#include <dockwin.hxx>
#endif
#ifndef _SV_FLOATWIN_HXX
#include <floatwin.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _SV_GRADIENT_HXX
#include <gradient.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <image.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif

#include <tools/debug.hxx>

#include <rvp.hxx>

#pragma hdrstop

using namespace ::com::sun::star::uno;

// =======================================================================

static void ImplGetPinImage( USHORT nStyle, BOOL bPinIn, Image& rImage )
{
    // ImageListe laden, wenn noch nicht vorhanden
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maCtrlData.mpPinImgList )
    {
        Bitmap aBmp( ResId( SV_RESID_BITMAP_PIN, ImplGetResMgr() ) );
        pSVData->maCtrlData.mpPinImgList = new ImageList( aBmp, Color( 0x00, 0x00, 0xFF ), 4 );
    }

    // Image ermitteln und zurueckgeben
    USHORT nId;
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

// -----------------------------------------------------------------------

void Window::ImplCalcSymbolRect( Rectangle& rRect )
{
    // Den Rand den der Button in der nicht Default-Darstellung freilaesst,
    // dazuaddieren, da wir diesen bei kleinen Buttons mit ausnutzen wollen
    rRect.Left()--;
    rRect.Top()--;
    rRect.Right()++;
    rRect.Bottom()++;

    // Zwischen dem Symbol und dem Button-Rand lassen wir 5% Platz
    long nExtraWidth = ((rRect.GetWidth()*50)+500)/1000;
    long nExtraHeight = ((rRect.GetHeight()*50)+500)/1000;
    rRect.Left()    += nExtraWidth;
    rRect.Right()   -= nExtraWidth;
    rRect.Top()     += nExtraHeight;
    rRect.Bottom()  -= nExtraHeight;
}

// -----------------------------------------------------------------------

static void ImplDrawBrdWinSymbol( OutputDevice* pDev,
                                  const Rectangle& rRect, SymbolType eSymbol )
{
    // Zwischen dem Symbol und dem Button lassen wir 5% Platz
    DecorationView  aDecoView( pDev );
    Rectangle       aTempRect = rRect;
    Window::ImplCalcSymbolRect( aTempRect );
    aDecoView.DrawSymbol( aTempRect, eSymbol,
                          pDev->GetSettings().GetStyleSettings().GetButtonTextColor(), 0 );
}

// -----------------------------------------------------------------------

static void ImplDrawBrdWinSymbolButton( OutputDevice* pDev,
                                        const Rectangle& rRect,
                                        SymbolType eSymbol, USHORT nState )
{
    DecorationView aDecoView( pDev );
    Rectangle aTempRect = aDecoView.DrawButton( rRect, nState );
    ImplDrawBrdWinSymbol( pDev, aTempRect, eSymbol );
}


// =======================================================================

// ------------------------
// - ImplBorderWindowView -
// ------------------------

ImplBorderWindowView::~ImplBorderWindowView()
{
}

// -----------------------------------------------------------------------

BOOL ImplBorderWindowView::MouseMove( const MouseEvent& rMEvt )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ImplBorderWindowView::MouseButtonDown( const MouseEvent& rMEvt )
{
    return FALSE;
}

// -----------------------------------------------------------------------

BOOL ImplBorderWindowView::Tracking( const TrackingEvent& rTEvt )
{
    return FALSE;
}

// -----------------------------------------------------------------------

USHORT ImplBorderWindowView::RequestHelp( const Point& rPos, Rectangle& rHelpRect )
{
    return 0;
}

// -----------------------------------------------------------------------

void ImplBorderWindowView::ImplInitTitle( ImplBorderFrameData* pData )
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( !(pBorderWindow->GetStyle() & WB_MOVEABLE) ||
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

// -----------------------------------------------------------------------

USHORT ImplBorderWindowView::ImplHitTest( ImplBorderFrameData* pData, const Point& rPos )
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( pData->maTitleRect.IsInside( rPos ) )
    {
        if ( pData->maCloseRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_CLOSE;
        else if ( pData->maRollRect.IsInside( rPos ) )
            return BORDERWINDOW_HITTEST_ROLL;
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

// -----------------------------------------------------------------------

BOOL ImplBorderWindowView::ImplMouseMove( ImplBorderFrameData* pData, const MouseEvent& rMEvt )
{
    Point aMousePos = rMEvt.GetPosPixel();
    USHORT nHitTest = ImplHitTest( pData, aMousePos );
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
    pData->mpBorderWindow->SetPointer( Pointer( ePtrStyle ) );
    return TRUE;
}

// -----------------------------------------------------------------------

BOOL ImplBorderWindowView::ImplMouseButtonDown( ImplBorderFrameData* pData, const MouseEvent& rMEvt )
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( rMEvt.IsLeft() || rMEvt.IsRight() )
    {
        pData->maMouseOff = rMEvt.GetPosPixel();
        pData->mnHitTest = ImplHitTest( pData, pData->maMouseOff );
        USHORT nDragFullTest = 0;
        if ( pData->mnHitTest )
        {
            BOOL bTracking = TRUE;
            BOOL bHitTest = TRUE;

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
                    // Bei DockingWindows wollen wir Docking
                    if ( (pData->mnHitTest & BORDERWINDOW_HITTEST_TITLE) &&
                         pBorderWindow->ImplGetClientWindow()->ImplGetClientWindow() &&
                         pBorderWindow->ImplGetClientWindow()->ImplGetClientWindow()->ImplIsDockingWindow() )
                    {
                        if ( !pBorderWindow->mbRollUp )
                        {
                            Point aMousePos = pData->maMouseOff;
                            aMousePos.X() -= pData->mnLeftBorder;
                            aMousePos.Y() -= pData->mnTopBorder;
                            bTracking = !(((DockingWindow*)(pBorderWindow->ImplGetClientWindow()->ImplGetClientWindow()))->ImplStartDocking( aMousePos ));
                        }
                    }

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
                    bTracking = FALSE;

                    if ( (pData->mnHitTest & BORDERWINDOW_DRAW_TITLE) &&
                         ((rMEvt.GetClicks() % 2) == 0) )
                    {
                        pData->mnHitTest = 0;
                        bHitTest = FALSE;

                        if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                        {
                            SystemWindow* pClientWindow = (SystemWindow*)(pBorderWindow->ImplGetClientWindow());
                            if ( pBorderWindow->mbDockBtn )
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
                pData->mbDragFull = FALSE;
                if ( nDragFullTest )
                {
                    if ( pBorderWindow->GetSettings().GetStyleSettings().GetDragFullOptions() & nDragFullTest )
                        pData->mbDragFull = TRUE;
                    else
                    {
                        pBorderWindow->ImplUpdateAll();
                        pBorderWindow->ImplGetFrameWindow()->ImplUpdateAll();
                    }
                }
                pBorderWindow->StartTracking();
            }
            else if ( bHitTest )
                pData->mnHitTest = 0;
        }
    }

    return TRUE;
}

// -----------------------------------------------------------------------

BOOL ImplBorderWindowView::ImplTracking( ImplBorderFrameData* pData, const TrackingEvent& rTEvt )
{
    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;

    if ( rTEvt.IsTrackingEnded() )
    {
        USHORT nHitTest = pData->mnHitTest;
        pData->mnHitTest = 0;

        if ( nHitTest & BORDERWINDOW_HITTEST_CLOSE )
        {
            if ( pData->mnCloseState & BUTTON_DRAW_PRESSED )
            {
                pData->mnCloseState &= ~BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_CLOSE );

                // Bei Abbruch kein Click-Handler rufen
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                        ((SystemWindow*)pBorderWindow->ImplGetClientWindow())->Close();
                }
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_ROLL )
        {
            if ( pData->mnRollState & BUTTON_DRAW_PRESSED )
            {
                pData->mnRollState &= ~BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_ROLL );

                // Bei Abbruch kein Click-Handler rufen
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = (SystemWindow*)(pBorderWindow->ImplGetClientWindow());
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

                // Bei Abbruch kein Click-Handler rufen
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = (SystemWindow*)(pBorderWindow->ImplGetClientWindow());
                        pClientWindow->TitleButtonClick( TITLE_BUTTON_DOCKING );
                    }
                }
            }
        }
        else if ( nHitTest & BORDERWINDOW_HITTEST_HIDE )
        {
            if ( pData->mnHideState & BUTTON_DRAW_PRESSED )
            {
                pData->mnHideState &= ~BUTTON_DRAW_PRESSED;
                DrawWindow( BORDERWINDOW_DRAW_HIDE );

                // Bei Abbruch kein Click-Handler rufen
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = (SystemWindow*)(pBorderWindow->ImplGetClientWindow());
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

                // Bei Abbruch kein Click-Handler rufen
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

                // Bei Abbruch kein Click-Handler rufen
                if ( !rTEvt.IsTrackingCanceled() )
                {
                    if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                    {
                        SystemWindow* pClientWindow = (SystemWindow*)(pBorderWindow->ImplGetClientWindow());
                        pClientWindow->SetPin( !pClientWindow->IsPined() );
                        pClientWindow->Pin();
                    }
                }
            }
        }
        else
        {
            if ( pData->mbDragFull )
            {
                // Bei Abbruch alten Zustand wieder herstellen
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
                    if ( ((FloatingWindow*)pBorderWindow->ImplGetClientWindow())->IsInPopupMode() )
                        ((FloatingWindow*)pBorderWindow->ImplGetClientWindow())->EndPopupMode( FLOATWIN_POPUPMODEEND_TEAROFF );
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
            Point   aFrameMousePos = pBorderWindow->ImplOutputToFrame( aMousePos );
            Size    aFrameSize = pBorderWindow->ImplGetFrameWindow()->GetOutputSizePixel();
            if ( aFrameMousePos.X() < 0 )
                aFrameMousePos.X() = 0;
            if ( aFrameMousePos.Y() < 0 )
                aFrameMousePos.Y() = 0;
            if ( aFrameMousePos.X() > aFrameSize.Width()-1 )
                aFrameMousePos.X() = aFrameSize.Width()-1;
            if ( aFrameMousePos.Y() > aFrameSize.Height()-1 )
                aFrameMousePos.Y() = aFrameSize.Height()-1;
            aMousePos = pBorderWindow->ImplFrameToOutput( aFrameMousePos );

            aMousePos.X()    -= pData->maMouseOff.X();
            aMousePos.Y()    -= pData->maMouseOff.Y();

            if ( pData->mnHitTest & BORDERWINDOW_HITTEST_TITLE )
            {
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
                Point       aPos            = pBorderWindow->GetPosPixel();
                Size        aSize           = pBorderWindow->GetSizePixel();
                Rectangle   aNewRect( aPos, aSize );
                long        nOldWidth       = aSize.Width();
                long        nOldHeight      = aSize.Height();
                long        nBorderWidth    = pData->mnLeftBorder+pData->mnRightBorder;
                long        nBorderHeight   = pData->mnTopBorder+pData->mnBottomBorder;
                long        nMinWidth       = pBorderWindow->mnMinWidth+nBorderWidth;
                long        nMinHeight      = pBorderWindow->mnMinHeight+nBorderHeight;
                long        nMinWidth2      = nBorderWidth;

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
                }
                else if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_RIGHT | BORDERWINDOW_HITTEST_TOPRIGHT | BORDERWINDOW_HITTEST_BOTTOMRIGHT) )
                {
                    aNewRect.Right() += aMousePos.X();
                    if ( aNewRect.GetWidth() < nMinWidth )
                        aNewRect.Right() = aNewRect.Left()+nMinWidth+1;
                }
                if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_TOP | BORDERWINDOW_HITTEST_TOPLEFT | BORDERWINDOW_HITTEST_TOPRIGHT) )
                {
                    aNewRect.Top() += aMousePos.Y();
                    if ( aNewRect.GetHeight() < nMinHeight )
                        aNewRect.Top() = aNewRect.Bottom()-nMinHeight+1;
                }
                else if ( pData->mnHitTest & (BORDERWINDOW_HITTEST_BOTTOM | BORDERWINDOW_HITTEST_BOTTOMLEFT | BORDERWINDOW_HITTEST_BOTTOMRIGHT) )
                {
                    aNewRect.Bottom() += aMousePos.Y();
                    if ( aNewRect.GetHeight() < nMinHeight )
                        aNewRect.Bottom() = aNewRect.Top()+nMinHeight+1;
                }

                // Bei SystemWindows rufen wir den Resizing-Handler
                if ( pBorderWindow->ImplGetClientWindow()->IsSystemWindow() )
                {
                    // Groesse fuer Resizing-Aufruf anpassen
                    aSize = aNewRect.GetSize();
                    aSize.Width()   -= nBorderWidth;
                    aSize.Height()  -= nBorderHeight;
                    ((SystemWindow*)pBorderWindow->ImplGetClientWindow())->Resizing( aSize );
                    aSize.Width()   += nBorderWidth;
                    aSize.Height()  += nBorderHeight;
                    if ( aSize.Width() < nMinWidth )
                        aSize.Width() = nMinWidth;
                    if ( aSize.Height() < nMinHeight )
                        aSize.Height() = nMinHeight;
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
                    pBorderWindow->SetPosSizePixel( aNewRect.Left(), aNewRect.Top(),
                                                    aNewRect.GetWidth(), aNewRect.GetHeight(), WINDOW_POSSIZE_POSSIZE );
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

    return TRUE;
}

// -----------------------------------------------------------------------

USHORT ImplBorderWindowView::ImplRequestHelp( ImplBorderFrameData* pData,
                                              const Point& rPos,
                                              Rectangle& rHelpRect )
{
    USHORT nHelpId = 0;
    USHORT nHitTest = ImplHitTest( pData, rPos );
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
    }

    return nHelpId;
}

// -----------------------------------------------------------------------

long ImplBorderWindowView::ImplCalcTitleWidth( const ImplBorderFrameData* pData ) const
{
    // kein sichtbarer Title, dann auch keine Breite
    if ( !pData->mnTitleHeight )
        return 0;

    ImplBorderWindow* pBorderWindow = pData->mpBorderWindow;
    long nTitleWidth = pBorderWindow->GetTextWidth( pBorderWindow->GetText() )+6;
    nTitleWidth += pData->maPinRect.GetWidth();
    nTitleWidth += pData->maCloseRect.GetWidth();
    nTitleWidth += pData->maRollRect.GetWidth();
    nTitleWidth += pData->maDockRect.GetWidth();
    nTitleWidth += pData->maHideRect.GetWidth();
    nTitleWidth += pData->maHelpRect.GetWidth();
    nTitleWidth += pData->mnLeftBorder+pData->mnRightBorder;
    return nTitleWidth;
}

// =======================================================================

// --------------------------
// - ImplNoBorderWindowView -
// --------------------------

ImplNoBorderWindowView::ImplNoBorderWindowView( ImplBorderWindow* pBorderWindow )
{
}

// -----------------------------------------------------------------------

void ImplNoBorderWindowView::Init( OutputDevice* pDev, long nWidth, long nHeight )
{
}

// -----------------------------------------------------------------------

void ImplNoBorderWindowView::GetBorder( long& rLeftBorder, long& rTopBorder,
                                        long& rRightBorder, long& rBottomBorder ) const
{
    rLeftBorder     = 0;
    rTopBorder      = 0;
    rRightBorder    = 0;
    rBottomBorder   = 0;
}

// -----------------------------------------------------------------------

long ImplNoBorderWindowView::CalcTitleWidth() const
{
    return 0;
}

// -----------------------------------------------------------------------

void ImplNoBorderWindowView::DrawWindow( USHORT nDrawFlags, OutputDevice*, const Point* )
{
}

// =======================================================================

// -----------------------------
// - ImplSmallBorderWindowView -
// -----------------------------

// =======================================================================

ImplSmallBorderWindowView::ImplSmallBorderWindowView( ImplBorderWindow* pBorderWindow )
{
    mpBorderWindow = pBorderWindow;
}

// -----------------------------------------------------------------------

void ImplSmallBorderWindowView::Init( OutputDevice* pDev, long nWidth, long nHeight )
{
    mpOutDev    = pDev;
    mnWidth     = nWidth;
    mnHeight    = nHeight;

    USHORT nBorderStyle = mpBorderWindow->GetBorderStyle();
    if ( nBorderStyle & WINDOW_BORDER_NOBORDER )
    {
        mnLeftBorder    = 0;
        mnTopBorder     = 0;
        mnRightBorder   = 0;
        mnBottomBorder  = 0;
    }
    else
    {
        USHORT nStyle = FRAME_DRAW_NODRAW;
        // Wenn Border umgesetzt wurde oder BorderWindow ein Frame-Fenster
        // ist, dann Border nach aussen
        if ( (nBorderStyle & WINDOW_BORDER_DOUBLEOUT) || mpBorderWindow->mbSmallOutBorder )
            nStyle |= FRAME_DRAW_DOUBLEOUT;
        else
            nStyle |= FRAME_DRAW_DOUBLEIN;
        if ( nBorderStyle & WINDOW_BORDER_MONO )
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

// -----------------------------------------------------------------------

void ImplSmallBorderWindowView::GetBorder( long& rLeftBorder, long& rTopBorder,
                                           long& rRightBorder, long& rBottomBorder ) const
{
    rLeftBorder     = mnLeftBorder;
    rTopBorder      = mnTopBorder;
    rRightBorder    = mnRightBorder;
    rBottomBorder   = mnBottomBorder;
}

// -----------------------------------------------------------------------

long ImplSmallBorderWindowView::CalcTitleWidth() const
{
    return 0;
}

// -----------------------------------------------------------------------

void ImplSmallBorderWindowView::DrawWindow( USHORT nDrawFlags, OutputDevice*, const Point* )
{
    USHORT nBorderStyle = mpBorderWindow->GetBorderStyle();
    if ( nBorderStyle & WINDOW_BORDER_NOBORDER )
        return;

    if ( nDrawFlags & BORDERWINDOW_DRAW_FRAME )
    {
        if ( nBorderStyle & WINDOW_BORDER_ACTIVE )
        {
            Color aColor = mpOutDev->GetSettings().GetStyleSettings().GetHighlightColor();
            mpOutDev->SetLineColor();
            mpOutDev->SetFillColor( aColor );
            mpOutDev->DrawRect( Rectangle( 0, 0, mnWidth-1, mnTopBorder ) );
            mpOutDev->DrawRect( Rectangle( 0, mnHeight-mnBottomBorder, mnWidth-1, mnHeight-1 ) );
            mpOutDev->DrawRect( Rectangle( 0, 0, mnLeftBorder, mnHeight-1 ) );
            mpOutDev->DrawRect( Rectangle( mnWidth-mnRightBorder, 0, mnWidth-1, mnHeight-1 ) );
        }
        else
        {
            USHORT nStyle = 0;
            // Wenn Border umgesetzt wurde oder BorderWindow ein Frame-Fenster
            // ist, dann Border nach aussen
            if ( (nBorderStyle & WINDOW_BORDER_DOUBLEOUT) || mpBorderWindow->mbSmallOutBorder )
                nStyle |= FRAME_DRAW_DOUBLEOUT;
            else
                nStyle |= FRAME_DRAW_DOUBLEIN;
            if ( nBorderStyle & WINDOW_BORDER_MONO )
                nStyle |= FRAME_DRAW_MONO;

            DecorationView  aDecoView( mpOutDev );
            Point           aTmpPoint;
            Rectangle       aInRect( aTmpPoint, Size( mnWidth, mnHeight ) );
            aDecoView.DrawFrame( aInRect, nStyle );
        }
    }
}

// =======================================================================

// ---------------------------
// - ImplStdBorderWindowView -
// ---------------------------

ImplStdBorderWindowView::ImplStdBorderWindowView( ImplBorderWindow* pBorderWindow )
{
    maFrameData.mpBorderWindow  = pBorderWindow;
    maFrameData.mbDragFull      = FALSE;
    maFrameData.mnHitTest       = 0;
    maFrameData.mnPinState      = 0;
    maFrameData.mnCloseState    = 0;
    maFrameData.mnRollState     = 0;
    maFrameData.mnDockState     = 0;
    maFrameData.mnHideState     = 0;
    maFrameData.mnHelpState     = 0;

    mpATitleVirDev              = NULL;
    mpDTitleVirDev              = NULL;
}

// -----------------------------------------------------------------------

ImplStdBorderWindowView::~ImplStdBorderWindowView()
{
    if ( mpATitleVirDev )
        delete mpATitleVirDev;
    if ( mpDTitleVirDev )
        delete mpDTitleVirDev;
}

// -----------------------------------------------------------------------

BOOL ImplStdBorderWindowView::MouseMove( const MouseEvent& rMEvt )
{
    return ImplMouseMove( &maFrameData, rMEvt );
}

// -----------------------------------------------------------------------

BOOL ImplStdBorderWindowView::MouseButtonDown( const MouseEvent& rMEvt )
{
    return ImplMouseButtonDown( &maFrameData, rMEvt );
}

// -----------------------------------------------------------------------

BOOL ImplStdBorderWindowView::Tracking( const TrackingEvent& rTEvt )
{
    return ImplTracking( &maFrameData, rTEvt );
}

// -----------------------------------------------------------------------

USHORT ImplStdBorderWindowView::RequestHelp( const Point& rPos, Rectangle& rHelpRect )
{
    return ImplRequestHelp( &maFrameData, rPos, rHelpRect );
}

// -----------------------------------------------------------------------

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

    if ( !(pBorderWindow->GetStyle() & WB_MOVEABLE) || (pData->mnTitleType == BORDERWINDOW_TITLE_NONE) )
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
                ImplGetPinImage( 0, 0, aImage );
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

            if ( pBorderWindow->mbHelpBtn )
            {
                pData->maHelpRect.Top()    = nItemTop;
                pData->maHelpRect.Bottom() = nItemBottom;
                pData->maHelpRect.Right()  = nRight;
                pData->maHelpRect.Left()   = pData->maHelpRect.Right()-pData->maHelpRect.GetHeight()+1;
                nRight -= pData->maHelpRect.GetWidth()+3;
            }
        }
        else
        {
            pData->maPinRect.SetEmpty();
            pData->maCloseRect.SetEmpty();
            pData->maDockRect.SetEmpty();
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
        pData->maHideRect.SetEmpty();
        pData->maRollRect.SetEmpty();
        pData->maHelpRect.SetEmpty();
    }
}

// -----------------------------------------------------------------------

void ImplStdBorderWindowView::GetBorder( long& rLeftBorder, long& rTopBorder,
                                         long& rRightBorder, long& rBottomBorder ) const
{
    rLeftBorder     = maFrameData.mnLeftBorder;
    rTopBorder      = maFrameData.mnTopBorder;
    rRightBorder    = maFrameData.mnRightBorder;
    rBottomBorder   = maFrameData.mnBottomBorder;
}

// -----------------------------------------------------------------------

long ImplStdBorderWindowView::CalcTitleWidth() const
{
    return ImplCalcTitleWidth( &maFrameData );
}

// -----------------------------------------------------------------------

void ImplStdBorderWindowView::DrawWindow( USHORT nDrawFlags, OutputDevice* pOutDev, const Point* pOffset )
{
    ImplBorderFrameData*    pData = &maFrameData;
    OutputDevice*           pDev = pOutDev ? pOutDev : pData->mpOutDev;
    ImplBorderWindow*       pBorderWindow = pData->mpBorderWindow;
    Point                   aTmpPoint = pOffset ? Point(*pOffset) : Point();
    Rectangle               aInRect( aTmpPoint, Size( pData->mnWidth, pData->mnHeight ) );
    const StyleSettings&    rStyleSettings = pData->mpOutDev->GetSettings().GetStyleSettings();
    DecorationView          aDecoView( pDev );
    USHORT                  nStyle;
    BOOL                    bActive = pBorderWindow->IsDisplayActive();

    // Draw Frame
    if ( nDrawFlags & BORDERWINDOW_DRAW_FRAME )
        nStyle = 0;
    else
        nStyle = FRAME_DRAW_NODRAW;
    aInRect = aDecoView.DrawFrame( aInRect, FRAME_DRAW_DOUBLEOUT | nStyle );

    // Draw Border
    pDev->SetLineColor();
    long nBorderSize = pData->mnBorderSize;
    if ( (nDrawFlags & BORDERWINDOW_DRAW_BORDER) && nBorderSize )
    {
        if ( bActive )
            pDev->SetFillColor( rStyleSettings.GetActiveBorderColor() );
        else
            pDev->SetFillColor( rStyleSettings.GetDeactiveBorderColor() );
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

        Color aColor2;
        if ( bActive )
        {
            pDev->SetFillColor( rStyleSettings.GetActiveColor() );
            pDev->SetTextColor( rStyleSettings.GetActiveTextColor() );
            aColor2 = rStyleSettings.GetActiveColor2();
        }
        else
        {
            pDev->SetFillColor( rStyleSettings.GetDeactiveColor() );
            pDev->SetTextColor( rStyleSettings.GetDeactiveTextColor() );
            aColor2 = rStyleSettings.GetDeactiveColor2();
        }
#ifndef REMOTE_APPSERVER
        BOOL bDrawRect;
        if ( pDev->GetColorCount() >= 256 )
        {
            Point           aTempPoint;
            Size            aRectSize = aInRect.GetSize();
            VirtualDevice*  pVirDev;
            if ( bActive )
                pVirDev = mpATitleVirDev;
            else
                pVirDev = mpDTitleVirDev;
            bDrawRect = FALSE;
            if ( !pVirDev || (aRectSize != pVirDev->GetOutputSizePixel()) )
            {
                if ( !pVirDev )
                {
                    pVirDev = new VirtualDevice( *pDev );
                    if ( bActive )
                        mpATitleVirDev = pVirDev;
                    else
                        mpDTitleVirDev = pVirDev;
                }

                Gradient aGradient( GRADIENT_LINEAR, pDev->GetFillColor(), aColor2 );
                aGradient.SetAngle( 900 );
                aGradient.SetBorder( 50 );
                if ( pVirDev->SetOutputSizePixel( aRectSize ) )
                    pVirDev->DrawGradient( Rectangle( aTempPoint, aRectSize ), aGradient );
                else
                    bDrawRect = TRUE;
            }

            if ( !bDrawRect )
            {
                Point aDest( aInRect.TopLeft() );
                if ( pOffset )
                    aDest.Move( pOffset->X(), pOffset->Y() );
                pDev->DrawOutDev( aDest, aRectSize,
                                  aTempPoint, aRectSize, *pVirDev );
            }
        }
        else
            bDrawRect = TRUE;

        if ( bDrawRect )
#endif
            pDev->DrawRect( aInRect );

        if ( pData->mnTitleType != BORDERWINDOW_TITLE_TEAROFF )
        {
            if ( pOffset )
                aInRect.Move( pOffset->X(), pOffset->Y() );

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
            else if ( !pData->maCloseRect.IsEmpty() )
                aInRect.Right() = pData->maCloseRect.Left()-2;
            pDev->DrawText( aInRect, pBorderWindow->GetText(),
                            TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER |
                            TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_CLIP );
        }
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_CLOSE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maCloseRect.IsEmpty() )
    {
        Rectangle aSymbolRect( pData->maCloseRect );
        if ( pOffset )
            aSymbolRect.Move( pOffset->X(), pOffset->Y() );
        ImplDrawBrdWinSymbolButton( pDev, aSymbolRect, SYMBOL_CLOSE, pData->mnCloseState );
    }
    if ( ((nDrawFlags & BORDERWINDOW_DRAW_DOCK) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maDockRect.IsEmpty() )
    {
        Rectangle aSymbolRect( pData->maDockRect );
        if ( pOffset )
            aSymbolRect.Move( pOffset->X(), pOffset->Y() );
        ImplDrawBrdWinSymbolButton( pDev, aSymbolRect, SYMBOL_DOCK, pData->mnDockState );
    }
    if ( ((nDrawFlags & BORDERWINDOW_DRAW_HIDE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maHideRect.IsEmpty() )
    {
        Rectangle aSymbolRect( pData->maHideRect );
        if ( pOffset )
            aSymbolRect.Move( pOffset->X(), pOffset->Y() );
        ImplDrawBrdWinSymbolButton( pDev, aSymbolRect, SYMBOL_HIDE, pData->mnHideState );
    }
    if ( ((nDrawFlags & BORDERWINDOW_DRAW_ROLL) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maRollRect.IsEmpty() )
    {
        SymbolType eType;
        if ( pBorderWindow->mbRollUp )
            eType = SYMBOL_ROLLDOWN;
        else
            eType = SYMBOL_ROLLUP;
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
        ImplDrawBrdWinSymbolButton( pDev, aSymbolRect, SYMBOL_HELP, pData->mnHelpState );
    }
    if ( ((nDrawFlags & BORDERWINDOW_DRAW_PIN) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maPinRect.IsEmpty() )
    {
        Image aImage;
        ImplGetPinImage( pData->mnPinState, pBorderWindow->mbPined, aImage );
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

// =======================================================================

// ---------------------------
// - ImplOS2BorderWindowView -
// ---------------------------

class ImplOS2BorderWindowView : public ImplBorderWindowView
{
    ImplBorderFrameData     maFrameData;

public:
                            ImplOS2BorderWindowView( ImplBorderWindow* pBorderWindow );
                            ~ImplOS2BorderWindowView();

    virtual BOOL            MouseMove( const MouseEvent& rMEvt );
    virtual BOOL            MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL            Tracking( const TrackingEvent& rTEvt );
    virtual USHORT          RequestHelp( const Point& rPos, Rectangle& rHelpRect );

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight );
    virtual void            GetBorder( long& rLeftBorder, long& rTopBorder,
                                       long& rRightBorder, long& rBottomBorder ) const;
    virtual long            CalcTitleWidth() const;
    virtual void            DrawWindow( USHORT nDrawFlags, OutputDevice* pOutDev, const Point* pOffset );

    Rectangle               DrawOS2TitleButton( const Rectangle& rRect, USHORT nStyle );
};

// Aus decoview.cxx
void ImplDrawOS2Symbol( OutputDevice* pDev, const Rectangle& rRect,
                        USHORT nStyle, BOOL bClose );

// =======================================================================

ImplOS2BorderWindowView::ImplOS2BorderWindowView( ImplBorderWindow* pBorderWindow )
{
    maFrameData.mpBorderWindow  = pBorderWindow;
    maFrameData.mbDragFull      = FALSE;
    maFrameData.mnHitTest       = 0;
    maFrameData.mnPinState      = 0;
    maFrameData.mnCloseState    = 0;
    maFrameData.mnRollState     = 0;
    maFrameData.mnDockState     = 0;
    maFrameData.mnHideState     = 0;
    maFrameData.mnHelpState     = 0;
}

// -----------------------------------------------------------------------

ImplOS2BorderWindowView::~ImplOS2BorderWindowView()
{
}

// -----------------------------------------------------------------------

BOOL ImplOS2BorderWindowView::MouseMove( const MouseEvent& rMEvt )
{
    return ImplMouseMove( &maFrameData, rMEvt );
}

// -----------------------------------------------------------------------

BOOL ImplOS2BorderWindowView::MouseButtonDown( const MouseEvent& rMEvt )
{
    return ImplMouseButtonDown( &maFrameData, rMEvt );
}

// -----------------------------------------------------------------------

BOOL ImplOS2BorderWindowView::Tracking( const TrackingEvent& rTEvt )
{
    return ImplTracking( &maFrameData, rTEvt );
}

// -----------------------------------------------------------------------

USHORT ImplOS2BorderWindowView::RequestHelp( const Point& rPos, Rectangle& rHelpRect )
{
    return ImplRequestHelp( &maFrameData, rPos, rHelpRect );
}

// -----------------------------------------------------------------------

void ImplOS2BorderWindowView::Init( OutputDevice* pDev, long nWidth, long nHeight )
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

    if ( !(pBorderWindow->GetStyle() & WB_MOVEABLE) || (pData->mnTitleType == BORDERWINDOW_TITLE_NONE) )
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
        // Wegen 3D-Border bei aktiver Darstellung
        if ( pData->mnTitleType != BORDERWINDOW_TITLE_TEAROFF )
        {
            long nTextHeight = pBorderWindow->GetTextHeight();
            nTextHeight += 2;
            if ( nTextHeight > pData->mnTitleHeight )
                pData->mnTitleHeight = nTextHeight;
        }

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

            if ( pBorderWindow->mbHelpBtn )
            {
                pData->maHelpRect.Top()    = nItemTop;
                pData->maHelpRect.Bottom() = nItemBottom;
                pData->maHelpRect.Left()   = nLeft;
                pData->maHelpRect.Right()  = pData->maHelpRect.Left()+pData->maHelpRect.GetHeight()-1;
                nLeft += pData->maHelpRect.GetWidth();
            }

            if ( pBorderWindow->GetStyle() & WB_PINABLE )
            {
                nLeft += 2;
                Image aImage;
                ImplGetPinImage( 0, 0, aImage );
                pData->maPinRect.Top()    = nItemTop;
                pData->maPinRect.Bottom() = nItemBottom;
                pData->maPinRect.Left()   = nLeft;
                pData->maPinRect.Right()  = pData->maPinRect.Left()+aImage.GetSizePixel().Width();
                nLeft += pData->maPinRect.GetWidth();
            }

            if ( pBorderWindow->mbDockBtn )
            {
                pData->maDockRect.Top()    = nItemTop;
                pData->maDockRect.Bottom() = nItemBottom;
                pData->maDockRect.Right()  = nRight;
                pData->maDockRect.Left()   = pData->maDockRect.Right()-pData->maDockRect.GetHeight()+1;
                nRight -= pData->maDockRect.GetWidth();
            }

            if ( pBorderWindow->mbHideBtn )
            {
                pData->maHideRect.Top()    = nItemTop;
                pData->maHideRect.Bottom() = nItemBottom;
                pData->maHideRect.Right()  = nRight;
                pData->maHideRect.Left()   = pData->maHideRect.Right()-pData->maHideRect.GetHeight()+1;
                nRight -= pData->maHideRect.GetWidth();
            }

            if ( pBorderWindow->GetStyle() & WB_ROLLABLE )
            {
                pData->maRollRect.Top()    = nItemTop;
                pData->maRollRect.Bottom() = nItemBottom;
                pData->maRollRect.Right()  = nRight;
                pData->maRollRect.Left()   = pData->maRollRect.Right()-pData->maRollRect.GetHeight()+1;
                nRight -= pData->maRollRect.GetWidth();
            }

            if ( pBorderWindow->GetStyle() & WB_CLOSEABLE )
            {
                pData->maCloseRect.Top()    = nItemTop;
                pData->maCloseRect.Bottom() = nItemBottom;
                pData->maCloseRect.Right()  = nRight;
                pData->maCloseRect.Left()   = pData->maCloseRect.Right()-pData->maCloseRect.GetHeight()+1;
                nRight -= pData->maCloseRect.GetWidth();
            }
        }
        else
        {
            pData->maPinRect.SetEmpty();
            pData->maCloseRect.SetEmpty();
            pData->maDockRect.SetEmpty();
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
        pData->maHideRect.SetEmpty();
        pData->maRollRect.SetEmpty();
        pData->maHelpRect.SetEmpty();
    }
}

// -----------------------------------------------------------------------

void ImplOS2BorderWindowView::GetBorder( long& rLeftBorder, long& rTopBorder,
                                         long& rRightBorder, long& rBottomBorder ) const
{
    rLeftBorder     = maFrameData.mnLeftBorder;
    rTopBorder      = maFrameData.mnTopBorder;
    rRightBorder    = maFrameData.mnRightBorder;
    rBottomBorder   = maFrameData.mnBottomBorder;
}

// -----------------------------------------------------------------------

long ImplOS2BorderWindowView::CalcTitleWidth() const
{
    return ImplCalcTitleWidth( &maFrameData );
}

// -----------------------------------------------------------------------

void ImplOS2BorderWindowView::DrawWindow( USHORT nDrawFlags, OutputDevice*, const Point* )
{
    ImplBorderFrameData*    pData = &maFrameData;
    OutputDevice*           pDev = pData->mpOutDev;
    ImplBorderWindow*       pBorderWindow = pData->mpBorderWindow;
    Point                   aTmpPoint;
    Rectangle               aInRect( aTmpPoint, Size( pData->mnWidth, pData->mnHeight ) );
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();
    DecorationView          aDecoView( pDev );
    USHORT                  nStyle;
    BOOL                    bActive = pBorderWindow->IsDisplayActive();

    // Draw Frame
    if ( nDrawFlags & BORDERWINDOW_DRAW_FRAME )
        nStyle = 0;
    else
        nStyle = FRAME_DRAW_NODRAW;
    aInRect = aDecoView.DrawFrame( aInRect, FRAME_DRAW_DOUBLEOUT | nStyle );

    // Draw Border
    pDev->SetLineColor();
    long nBorderSize = pData->mnBorderSize;
    if ( (nDrawFlags & BORDERWINDOW_DRAW_BORDER) && nBorderSize )
    {
        if ( bActive )
            pDev->SetFillColor( rStyleSettings.GetActiveBorderColor() );
        else
            pDev->SetFillColor( rStyleSettings.GetDeactiveBorderColor() );
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

        if ( !pData->maHelpRect.IsEmpty() )
            aInRect.Left() = pData->maHelpRect.Right()+1;

        if ( !pData->maCloseRect.IsEmpty() )
            aInRect.Right() = pData->maCloseRect.Left()-1;
        else if ( !pData->maRollRect.IsEmpty() )
            aInRect.Right() = pData->maRollRect.Left()-1;
        else if ( !pData->maHideRect.IsEmpty() )
            aInRect.Right() = pData->maHideRect.Left()-1;
        else if ( !pData->maDockRect.IsEmpty() )
            aInRect.Right() = pData->maDockRect.Left()-1;

        if ( bActive )
        {
            pDev->SetFillColor( rStyleSettings.GetActiveColor() );
            pDev->SetTextColor( rStyleSettings.GetActiveTextColor() );
        }
        else
        {
            pDev->SetFillColor( rStyleSettings.GetDeactiveColor() );
            pDev->SetTextColor( rStyleSettings.GetDeactiveTextColor() );
        }
        pDev->DrawRect( aInRect );

        if ( pData->mnTitleType != BORDERWINDOW_TITLE_TEAROFF )
        {
            Rectangle aOrgInRect = aInRect;

            if ( !pData->maPinRect.IsEmpty() )
                aInRect.Left() = pData->maPinRect.Right();

            aInRect.Left()  += 2;
            aInRect.Right() -= 2;

            pDev->DrawText( aInRect, pBorderWindow->GetText(),
                            TEXT_DRAW_LEFT | TEXT_DRAW_VCENTER |
                            TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_CLIP );

            if ( bActive )
                aDecoView.DrawFrame( aOrgInRect, rStyleSettings.GetShadowColor(), rStyleSettings.GetLightColor() );
        }
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_CLOSE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maCloseRect.IsEmpty() )
    {
        Rectangle aRect = DrawOS2TitleButton( pData->maCloseRect, pData->mnCloseState );
        ImplDrawOS2Symbol( pDev, aRect, pData->mnCloseState, TRUE );
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_DOCK) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maDockRect.IsEmpty() )
    {
        Rectangle aRect = DrawOS2TitleButton( pData->maDockRect, pData->mnDockState );
        ImplDrawOS2Symbol( pDev, aRect, pData->mnDockState, FALSE );
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_HIDE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maHideRect.IsEmpty() )
    {
        Rectangle aRect = DrawOS2TitleButton( pData->maHideRect, pData->mnHideState );
        aRect.Left()   += 2;
        aRect.Top()    += 2;
        aRect.Right()  -= 2;
        aRect.Bottom() -= 2;
        ImplDrawOS2Symbol( pDev, aRect, pData->mnHideState, FALSE );
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_ROLL) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maRollRect.IsEmpty() )
    {
        Rectangle aRect = DrawOS2TitleButton( pData->maRollRect, pData->mnRollState );
        if ( !pBorderWindow->mbRollUp )
            aRect.Bottom() = aRect.Top()+6;
        ImplDrawOS2Symbol( pDev, aRect, pData->mnRollState, FALSE );
        if ( pBorderWindow->mbRollUp )
        {
            aRect.Left()   += 3;
            aRect.Top()    += 3;
            aRect.Right()  -= 3;
            aRect.Bottom() -= 3;
            pDev->SetFillColor();
            if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
                pDev->SetLineColor( rStyleSettings.GetShadowColor() );
            else
                pDev->SetLineColor( rStyleSettings.GetLightColor() );
            pDev->DrawLine( aRect.TopLeft(), aRect.TopRight() );
            aRect.Top()++;
            if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
                pDev->SetLineColor( rStyleSettings.GetLightColor() );
            else
                pDev->SetLineColor( rStyleSettings.GetShadowColor() );
            pDev->DrawLine( aRect.TopLeft(), aRect.TopRight() );
        }
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_HELP) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maHelpRect.IsEmpty() )
    {
        Rectangle aRect = DrawOS2TitleButton( pData->maHelpRect, pData->mnHelpState );
        ImplDrawBrdWinSymbol( pDev, aRect, SYMBOL_HELP );
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_PIN) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maPinRect.IsEmpty() )
    {
        Image aImage;
        ImplGetPinImage( pData->mnPinState, pBorderWindow->mbPined, aImage );
        Size  aImageSize = aImage.GetSizePixel();
        long  nRectHeight = pData->maPinRect.GetHeight();
        if ( nRectHeight < aImageSize.Height() )
        {
            pDev->DrawImage( Point( pData->maPinRect.Left(), pData->maPinRect.Top() ),
                             Size( aImageSize.Width(), nRectHeight ),
                             aImage );
        }
        else
        {
            pDev->DrawImage( Point( pData->maPinRect.Left(),
                                    pData->maPinRect.Top()+(nRectHeight-aImageSize.Height())/2 ),
                             aImage );
        }
    }
}

// -----------------------------------------------------------------------

Rectangle ImplOS2BorderWindowView::DrawOS2TitleButton( const Rectangle& rRect, USHORT )
{
    OutputDevice*           pDev = maFrameData.mpOutDev;
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();
    Rectangle               aRect = rRect;

    pDev->SetLineColor();
    pDev->SetFillColor( rStyleSettings.GetFaceColor() );
    pDev->DrawRect( aRect );

    long nExtraWidth  = ((aRect.GetWidth()*150)+500)/1000;
    long nExtraHeight = ((aRect.GetHeight()*150)+500)/1000;
    if ( !nExtraWidth )
        nExtraWidth = 1;
    if ( !nExtraHeight )
        nExtraHeight = 1;
    aRect.Left()    += nExtraWidth;
    aRect.Right()   -= nExtraWidth;
    aRect.Top()     += nExtraHeight;
    aRect.Bottom()  -= nExtraHeight;
    return aRect;
}

// =======================================================================

// ---------------------------
// - ImplUnxBorderWindowView -
// ---------------------------

class ImplUnxBorderWindowView : public ImplBorderWindowView
{
    ImplBorderFrameData     maFrameData;

public:
                            ImplUnxBorderWindowView( ImplBorderWindow* pBorderWindow );
                            ~ImplUnxBorderWindowView();

    virtual BOOL            MouseMove( const MouseEvent& rMEvt );
    virtual BOOL            MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL            Tracking( const TrackingEvent& rTEvt );
    virtual USHORT          RequestHelp( const Point& rPos, Rectangle& rHelpRect );

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight );
    virtual void            GetBorder( long& rLeftBorder, long& rTopBorder,
                                       long& rRightBorder, long& rBottomBorder ) const;
    virtual long            CalcTitleWidth() const;
    virtual void            DrawWindow( USHORT nDrawFlags, OutputDevice* pOutDev, const Point* pOffset );

    Rectangle               DrawUnxTitleButton( const Rectangle& rRect, USHORT nStyle );
};

// =======================================================================

ImplUnxBorderWindowView::ImplUnxBorderWindowView( ImplBorderWindow* pBorderWindow )
{
    maFrameData.mpBorderWindow  = pBorderWindow;
    maFrameData.mbDragFull      = FALSE;
    maFrameData.mnHitTest       = 0;
    maFrameData.mnPinState      = 0;
    maFrameData.mnCloseState    = 0;
    maFrameData.mnRollState     = 0;
    maFrameData.mnDockState     = 0;
    maFrameData.mnHideState     = 0;
    maFrameData.mnHelpState     = 0;
}

// -----------------------------------------------------------------------

ImplUnxBorderWindowView::~ImplUnxBorderWindowView()
{
}

// -----------------------------------------------------------------------

BOOL ImplUnxBorderWindowView::MouseMove( const MouseEvent& rMEvt )
{
    return ImplMouseMove( &maFrameData, rMEvt );
}

// -----------------------------------------------------------------------

BOOL ImplUnxBorderWindowView::MouseButtonDown( const MouseEvent& rMEvt )
{
    return ImplMouseButtonDown( &maFrameData, rMEvt );
}

// -----------------------------------------------------------------------

BOOL ImplUnxBorderWindowView::Tracking( const TrackingEvent& rTEvt )
{
    return ImplTracking( &maFrameData, rTEvt );
}

// -----------------------------------------------------------------------

USHORT ImplUnxBorderWindowView::RequestHelp( const Point& rPos, Rectangle& rHelpRect )
{
    return ImplRequestHelp( &maFrameData, rPos, rHelpRect );
}

// -----------------------------------------------------------------------

void ImplUnxBorderWindowView::Init( OutputDevice* pDev, long nWidth, long nHeight )
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

    if ( !(pBorderWindow->GetStyle() & WB_MOVEABLE) ||
         (pData->mnTitleType == BORDERWINDOW_TITLE_NONE) )
        pData->mnBorderSize = 0;
    else if ( pData->mnTitleType == BORDERWINDOW_TITLE_TEAROFF )
        pData->mnBorderSize = 0;
    else
    {
        pData->mnBorderSize = rStyleSettings.GetBorderSize();
        if ( pData->mnBorderSize < 3 )
            pData->mnBorderSize = 3;
        aCalcRect = aRect;
    }
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
        pData->maTitleRect.Left()    = pData->mnLeftBorder;
        pData->maTitleRect.Right()   = nWidth-pData->mnRightBorder-1;
        pData->maTitleRect.Top()     = pData->mnTopBorder;
        pData->maTitleRect.Bottom()  = pData->maTitleRect.Top() + pData->mnTitleHeight - 1;

        if ( pData->mnTitleType & (BORDERWINDOW_TITLE_NORMAL | BORDERWINDOW_TITLE_SMALL) )
        {
            long nLeft          = pData->maTitleRect.Left();
            long nRight         = pData->maTitleRect.Right();
            long nItemTop       = pData->maTitleRect.Top();
            long nItemBottom    = pData->maTitleRect.Bottom();

            if ( pBorderWindow->GetStyle() & WB_CLOSEABLE )
            {
                pData->maCloseRect.Top()    = nItemTop;
                pData->maCloseRect.Bottom() = nItemBottom;
                pData->maCloseRect.Left()   = nLeft;
                pData->maCloseRect.Right()  = nLeft + pData->maCloseRect.GetHeight();
                nLeft += pData->maCloseRect.GetWidth() + 2;
            }

            if ( pBorderWindow->GetStyle() & WB_PINABLE )
            {
                Image aImage;
                ImplGetPinImage( 0, 0, aImage );
                pData->maPinRect.Top()    = nItemTop;
                pData->maPinRect.Bottom() = nItemBottom;
                pData->maPinRect.Left()   = nLeft;
                pData->maPinRect.Right()  = pData->maPinRect.Left() + aImage.GetSizePixel().Width();
                nLeft += pData->maPinRect.GetWidth();
            }

            if ( pBorderWindow->mbDockBtn )
            {
                pData->maDockRect.Top()    = nItemTop;
                pData->maDockRect.Bottom() = nItemBottom;
                pData->maDockRect.Right()  = nRight;
                pData->maDockRect.Left()   = pData->maDockRect.Right()-pData->maDockRect.GetHeight()+1;
                nRight -= pData->maDockRect.GetWidth();
            }

            if ( pBorderWindow->mbHideBtn )
            {
                pData->maHideRect.Top()    = nItemTop;
                pData->maHideRect.Bottom() = nItemBottom;
                pData->maHideRect.Right()  = nRight;
                pData->maHideRect.Left()   = pData->maHideRect.Right()-pData->maHideRect.GetHeight()+1;
                nRight -= pData->maHideRect.GetWidth();
            }

            if ( pBorderWindow->GetStyle() & WB_ROLLABLE )
            {
                pData->maRollRect.Top()    = nItemTop;
                pData->maRollRect.Bottom() = nItemBottom;
                pData->maRollRect.Right()  = nRight;
                pData->maRollRect.Left()   = pData->maRollRect.Right()-pData->maRollRect.GetHeight()+1;
                nRight -= pData->maRollRect.GetWidth();
            }

            if ( pBorderWindow->mbHelpBtn )
            {
                pData->maHelpRect.Top()    = nItemTop;
                pData->maHelpRect.Bottom() = nItemBottom;
                pData->maHelpRect.Right()  = nRight;
                pData->maHelpRect.Left()   = pData->maHelpRect.Right()-pData->maHelpRect.GetHeight()+1;
                nRight -= pData->maHelpRect.GetWidth();
            }
        }
        else
        {
            pData->maPinRect.SetEmpty();
            pData->maCloseRect.SetEmpty();
            pData->maDockRect.SetEmpty();
            pData->maHideRect.SetEmpty();
            pData->maRollRect.SetEmpty();
            pData->maHelpRect.SetEmpty();
        }

        pData->mnTopBorder += pData->mnTitleHeight;
    }
    else
    {
        pData->maTitleRect.SetEmpty();
        pData->maPinRect.SetEmpty();
        pData->maCloseRect.SetEmpty();
        pData->maDockRect.SetEmpty();
        pData->maHideRect.SetEmpty();
        pData->maRollRect.SetEmpty();
        pData->maHelpRect.SetEmpty();
    }
}

// -----------------------------------------------------------------------

void ImplUnxBorderWindowView::GetBorder( long& rLeftBorder, long& rTopBorder,
                                         long& rRightBorder, long& rBottomBorder ) const
{
    rLeftBorder     = maFrameData.mnLeftBorder;
    rTopBorder      = maFrameData.mnTopBorder;
    rRightBorder    = maFrameData.mnRightBorder;
    rBottomBorder   = maFrameData.mnBottomBorder;
}

// -----------------------------------------------------------------------

long ImplUnxBorderWindowView::CalcTitleWidth() const
{
    return ImplCalcTitleWidth( &maFrameData );
}

// -----------------------------------------------------------------------

void ImplUnxBorderWindowView::DrawWindow( USHORT nDrawFlags, OutputDevice*, const Point* )
{
    ImplBorderFrameData*    pData = &maFrameData;
    OutputDevice*           pDev = pData->mpOutDev;
    ImplBorderWindow*       pBorderWindow = pData->mpBorderWindow;
    Point                   aTmpPoint;
    Rectangle               aInRect( aTmpPoint, Size( pData->mnWidth, pData->mnHeight ) );
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();
    DecorationView          aDecoView( pDev );
    USHORT                  nStyle;
    BOOL                    bActive = pBorderWindow->IsDisplayActive();

    // Color-Management for 3D-like Border
    Color aLineColor;
    Color aFillColor;
    Color aLightColor;
    Color aDarkColor;
    Color aTextColor;
    if ( bActive )
    {
        aLineColor = rStyleSettings.GetActiveBorderColor();
        aFillColor = rStyleSettings.GetActiveColor();
        aTextColor = rStyleSettings.GetActiveTextColor();
    }
    else
    {
        aLineColor = rStyleSettings.GetDeactiveBorderColor();
        aFillColor = rStyleSettings.GetDeactiveColor();
        aTextColor = rStyleSettings.GetDeactiveTextColor();
    }
    aLightColor = aLineColor;
    aDarkColor  = aLineColor;
    aLightColor.IncreaseLuminance( 64 );
    aDarkColor.DecreaseLuminance(  64 );

    // Draw Frame
    long nBorderSize = pData->mnBorderSize;
    if ( !nBorderSize )
    {
        if ( nDrawFlags & BORDERWINDOW_DRAW_FRAME )
            nStyle = 0;
        else
            nStyle = FRAME_DRAW_NODRAW;
        aInRect = aDecoView.DrawFrame( aInRect, FRAME_DRAW_DOUBLEOUT | nStyle );
    }
    else
    {
        // Draw Border
        if ( (nDrawFlags & BORDERWINDOW_DRAW_BORDER) && nBorderSize )
        {
            // Geometry Presettings
            long    nOffset = 0;
            long    nEffBorderSize;
            long    nLeft   = aInRect.Left();
            long    nRight  = aInRect.Right();
            long    nTop    = aInRect.Top();
            long    nBottom = aInRect.Bottom();

            nEffBorderSize = nBorderSize > 2 ? nBorderSize - 2 : 1;

            // outer border
            pDev->SetLineColor( aLightColor );
            pDev->DrawLine( Point( nLeft,  nTop    ), Point( nLeft,  nBottom ) );
            pDev->DrawLine( Point( nLeft,  nTop    ), Point( nRight, nTop    ) );

            pDev->SetLineColor( aDarkColor );
            nTop++; nLeft++;
            pDev->DrawLine( Point( nRight, nTop    ), Point( nRight, nBottom ) );
            pDev->DrawLine( Point( nLeft,  nBottom ), Point( nRight, nBottom ) );

            // middle border
            pDev->SetLineColor();
            pDev->SetFillColor( aLineColor );
            nRight--; nBottom--;
            pDev->DrawRect( Rectangle( Point( nLeft, nTop ),
                                       Size(  nRight - nLeft, nEffBorderSize ) ) );
            pDev->DrawRect( Rectangle( Point( nLeft, nTop ),
                                       Size(  nEffBorderSize, nBottom - nTop ) ) );
            nRight  -= nEffBorderSize;
            nBottom -= nEffBorderSize;
            pDev->DrawRect( Rectangle( Point( nRight + 1, nTop ),
                                       Size(  nEffBorderSize, nBottom - nTop + 1) ) );
            pDev->DrawRect( Rectangle( Point( nLeft, nBottom + 1),
                                       Size(  nRight - nLeft + 1, nEffBorderSize ) ) );

            // inner Border
            pDev->SetLineColor( aDarkColor );
            nTop += nEffBorderSize; nLeft += nEffBorderSize;
            pDev->DrawLine( Point( nLeft,  nTop    ), Point( nLeft,  nBottom ) );
            pDev->DrawLine( Point( nLeft,  nTop    ), Point( nRight, nTop    ) );

            pDev->SetLineColor( aLightColor );
            nTop++; nLeft++;
            pDev->DrawLine( Point( nRight, nTop    ), Point( nRight, nBottom ) );
            pDev->DrawLine( Point( nLeft,  nBottom ), Point( nRight, nBottom ) );

            // edge intersections
            if ( pBorderWindow->GetStyle() & WB_SIZEABLE )
            {
                nLeft   = aInRect.Left();
                nRight  = aInRect.Right();
                nTop    = aInRect.Top();
                nBottom = aInRect.Bottom();

                if ( !pData->maTitleRect.IsEmpty() )
                    nOffset = pData->maTitleRect.GetHeight() + nEffBorderSize + 2;

                pDev->SetLineColor( aDarkColor );
                pDev->DrawLine( Point( nLeft + 1,              nTop + nOffset ),
                                Point( nLeft + nEffBorderSize, nTop + nOffset ));
                pDev->DrawLine( Point( nLeft + nOffset,        nTop + 1       ),
                                Point( nLeft + nOffset,        nTop + nEffBorderSize));
                pDev->DrawLine( Point( nRight - 1,             nTop + nOffset ),
                                Point( nRight - nEffBorderSize,nTop + nOffset ));
                pDev->DrawLine( Point( nRight - nOffset,       nTop + 1       ),
                                Point( nRight - nOffset,       nTop + nEffBorderSize));

                pDev->DrawLine( Point( nLeft + 1,              nBottom - nOffset ),
                                Point( nLeft + nEffBorderSize, nBottom - nOffset ));
                pDev->DrawLine( Point( nLeft + nOffset,        nBottom - 1       ),
                                Point( nLeft + nOffset,        nBottom - nEffBorderSize));
                pDev->DrawLine( Point( nRight - 1,             nBottom - nOffset ),
                                Point( nRight - nEffBorderSize,nBottom - nOffset ));
                pDev->DrawLine( Point( nRight - nOffset,       nBottom - 1       ),
                                Point( nRight - nOffset,       nBottom - nEffBorderSize));
                pDev->SetLineColor( aLightColor );
                nEffBorderSize = nBorderSize > 1 ? nBorderSize - 1 : 1;
                nOffset += 1;

                pDev->DrawLine( Point( nLeft,                  nTop + nOffset    ),
                                Point( nLeft + nEffBorderSize, nTop + nOffset    ));
                pDev->DrawLine( Point( nLeft + nOffset,        nTop              ),
                                Point( nLeft + nOffset,        nTop + nEffBorderSize));
                pDev->DrawLine( Point( nRight,                 nTop + nOffset    ),
                                Point( nRight - nEffBorderSize,nTop + nOffset    ));
                pDev->DrawLine( Point( nLeft + nOffset,        nBottom           ),
                                Point( nLeft + nOffset,        nBottom - nEffBorderSize));
                nOffset -= 2;
                pDev->DrawLine( Point( nRight - nOffset,       nTop              ),
                                Point( nRight - nOffset,       nTop + nEffBorderSize));
                pDev->DrawLine( Point( nLeft,                  nBottom - nOffset ),
                                Point( nLeft + nEffBorderSize, nBottom - nOffset ));
                pDev->DrawLine( Point( nRight,                 nBottom - nOffset ),
                                Point( nRight - nEffBorderSize,nBottom - nOffset ));
                pDev->DrawLine( Point( nRight - nOffset,       nBottom           ),
                                Point( nRight - nOffset,       nBottom - nEffBorderSize));
            }

            // Reset Linecolor
            pDev->SetLineColor();
        }
    }

    // Draw Title
    if ( (nDrawFlags & BORDERWINDOW_DRAW_TITLE) && !pData->maTitleRect.IsEmpty() )
    {
        // Title Rect
        aInRect = pData->maTitleRect;

        pDev->SetFillColor( aFillColor );
        pDev->SetTextColor( aTextColor );
        pDev->DrawRect( aInRect );

        // Title Rect Decoration
        pDev->SetLineColor( aDarkColor );
        pDev->DrawLine( Point( aInRect.Left(),  aInRect.Bottom() ),
                        Point( aInRect.Right(), aInRect.Bottom() ) );
        pDev->DrawLine( Point( aInRect.Right(), aInRect.Top()    ),
                        Point( aInRect.Right(), aInRect.Bottom() ) );
        pDev->SetLineColor( aLightColor );
        pDev->DrawLine( Point( aInRect.Left(),  aInRect.Top()    ),
                        Point( aInRect.Right(), aInRect.Top()    ) );
        pDev->DrawLine( Point( aInRect.Left(),  aInRect.Top() ),
                        Point( aInRect.Left(),  aInRect.Bottom() ) );

        long nLeft;
        if ( !pData->maCloseRect.IsEmpty() )
            nLeft = pData->maCloseRect.Right()+1;
        else
            nLeft = aInRect.Left()+1;

        // Title Text
        if ( pData->mnTitleType != BORDERWINDOW_TITLE_TEAROFF )
        {
            if ( !pData->maPinRect.IsEmpty() )
                aInRect.Left() = pData->maPinRect.Right()+1;
            else if ( !pData->maCloseRect.IsEmpty() )
                aInRect.Left() = pData->maCloseRect.Right()+1;

            if ( !pData->maHelpRect.IsEmpty() )
                aInRect.Right() = pData->maHelpRect.Left()-1;
            else if ( !pData->maRollRect.IsEmpty() )
                aInRect.Right() = pData->maRollRect.Left()-1;
            else if ( !pData->maHideRect.IsEmpty() )
                aInRect.Right() = pData->maHideRect.Left()-1;
            else if ( !pData->maDockRect.IsEmpty() )
                aInRect.Right() = pData->maDockRect.Left()-1;

            pDev->DrawText( aInRect, pBorderWindow->GetText(),
                            TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER |
                            TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_CLIP );
        }

        // more Title Rect Decoration
        pDev->SetLineColor( aLightColor );
        pDev->DrawLine( Point( nLeft, aInRect.Bottom() ),
                        Point( nLeft, aInRect.Top()    ));
        pDev->SetLineColor( aDarkColor );
        pDev->DrawLine( Point( aInRect.Right(), aInRect.Top() ),
                        Point( aInRect.Right(), aInRect.Bottom() ) );
    }

    // Draw the buttons
    if ( ((nDrawFlags & BORDERWINDOW_DRAW_CLOSE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maCloseRect.IsEmpty() )
    {
        Rectangle aInCloseRect = DrawUnxTitleButton( pData->maCloseRect,
                                                     pData->mnCloseState );
        aDecoView.DrawSymbol( aInCloseRect, SYMBOL_CLOSE, aTextColor, 0 );
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_DOCK) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maDockRect.IsEmpty() )
    {
        Rectangle aInDockRect = DrawUnxTitleButton( pData->maDockRect,
                                                    pData->mnDockState );
        aDecoView.DrawSymbol( aInDockRect, SYMBOL_DOCK, aTextColor, 0 );
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_HIDE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maHideRect.IsEmpty() )
    {
        Rectangle aInHideRect = DrawUnxTitleButton( pData->maHideRect,
                                                    pData->mnHideState );
        aDecoView.DrawSymbol( aInHideRect, SYMBOL_HIDE, aTextColor, 0 );
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_ROLL) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maRollRect.IsEmpty() )
    {
        Rectangle aInRollRect = DrawUnxTitleButton( pData->maRollRect,
                                                    pData->mnRollState );
        SymbolType eType;
        if ( pBorderWindow->mbRollUp )
            eType = SYMBOL_ROLLDOWN;
        else
            eType = SYMBOL_ROLLUP;
        aDecoView.DrawSymbol( aInRollRect, eType, aTextColor, 0 );
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_HELP) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maHelpRect.IsEmpty() )
    {
        Rectangle aInHelpRect = DrawUnxTitleButton( pData->maHelpRect,
                                                    pData->mnHelpState );
        aDecoView.DrawSymbol( aInHelpRect, SYMBOL_HELP, aTextColor, 0 );
    }

    if ( ((nDrawFlags & BORDERWINDOW_DRAW_PIN) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
         !pData->maPinRect.IsEmpty() )
    {
        Image aImage;
        ImplGetPinImage( pData->mnPinState, pBorderWindow->mbPined, aImage );
        Size  aImageSize = aImage.GetSizePixel();
        long  nRectHeight = pData->maPinRect.GetHeight();
        if ( nRectHeight < aImageSize.Height() )
        {
            pDev->DrawImage( Point( pData->maPinRect.Left(),
                                    pData->maPinRect.Top() ),
                             Size( aImageSize.Width(), nRectHeight ),
                             aImage );
        }
        else
        {
            pDev->DrawImage( Point( pData->maPinRect.Left(),
                                    pData->maPinRect.Top()+(nRectHeight-aImageSize.Height())/2 ),
                             aImage );
        }
    }
}

// -----------------------------------------------------------------------

Rectangle ImplUnxBorderWindowView::DrawUnxTitleButton( const Rectangle& rRect, USHORT nStyle )
{
    Rectangle               aFillRect = rRect;
    OutputDevice*           pDev = maFrameData.mpOutDev;
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();
    ImplBorderFrameData*    pData = &maFrameData;
    ImplBorderWindow*       pBorderWindow = pData->mpBorderWindow;
    BOOL                    bActive = pBorderWindow->IsDisplayActive();

    // Color-Management for 3D-like Border
    Color aLineColor;
    Color aLightColor;
    Color aDarkColor;
    if ( bActive )
        aLineColor = rStyleSettings.GetActiveBorderColor();
    else
        aLineColor = rStyleSettings.GetDeactiveBorderColor();
    aLightColor = aLineColor;
    aDarkColor  = aLineColor;
    aLightColor.IncreaseLuminance( 64 );
    aDarkColor.DecreaseLuminance(  64 );

    // Draw Button
    if ( !(nStyle & BUTTON_DRAW_NODRAW) )
    {
        // left and upper button-border
        if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
            pDev->SetLineColor( aDarkColor );
        else
            pDev->SetLineColor( aLightColor );
        pDev->DrawLine( Point( aFillRect.Left(),  aFillRect.Top()    ),
                        Point( aFillRect.Right(), aFillRect.Top()    ) );
        pDev->DrawLine( Point( aFillRect.Left(),  aFillRect.Top()    ),
                        Point( aFillRect.Left(),  aFillRect.Bottom() ) );

        aFillRect.Left()++;
        aFillRect.Top()++;

        // right and bottom button-border
        if ( nStyle & (BUTTON_DRAW_PRESSED | BUTTON_DRAW_CHECKED) )
            pDev->SetLineColor( aLightColor );
        else
            pDev->SetLineColor( aDarkColor );
        pDev->DrawLine( Point( aFillRect.Right(), aFillRect.Top()    ),
                        Point( aFillRect.Right(), aFillRect.Bottom()    ) );
        pDev->DrawLine( Point( aFillRect.Left(),  aFillRect.Bottom()    ),
                        Point( aFillRect.Right(), aFillRect.Bottom() ) );

        aFillRect.Right()--;
        aFillRect.Bottom()--;

        // button area
        pDev->SetFillColor( aLineColor );
        pDev->SetLineColor();

        pDev->DrawRect( Rectangle( aFillRect.Left(),  aFillRect.Top(),
                                   aFillRect.Right(), aFillRect.Bottom() ) );
    }

    aFillRect.Left()    += 3;
    aFillRect.Top()     += 3;
    aFillRect.Right()   -= 2;
    aFillRect.Bottom()  -= 2;
    return aFillRect;
}

// =======================================================================

// ---------------------------
// - ImplMacBorderWindowView -
// ---------------------------

class ImplMacBorderWindowView : public ImplBorderWindowView
{
    ImplBorderFrameData     maFrameData;
    VirtualDevice           maVirDev;
    BOOL                    mbPressed;

public:
                            ImplMacBorderWindowView( ImplBorderWindow* pBorderWindow );
                            ~ImplMacBorderWindowView();

    virtual BOOL            MouseMove( const MouseEvent& rMEvt );
    virtual BOOL            MouseButtonDown( const MouseEvent& rMEvt );
    virtual BOOL            Tracking( const TrackingEvent& rTEvt );
    virtual USHORT          RequestHelp( const Point& rPos, Rectangle& rHelpRect );

    virtual void            Init( OutputDevice* pDev, long nWidth, long nHeight );
    virtual void            GetBorder( long& rLeftBorder, long& rTopBorder,
                                       long& rRightBorder, long& rBottomBorder ) const;
    virtual long            CalcTitleWidth() const;
    virtual void            DrawWindow( USHORT nDrawFlags, OutputDevice* pOutDev, const Point* pOffset );

    Rectangle               DrawMacTitleButton( const Rectangle& rRect, USHORT nStyle );
};

// =======================================================================

ImplMacBorderWindowView::ImplMacBorderWindowView( ImplBorderWindow* pBorderWindow )
{
    maFrameData.mpBorderWindow  = pBorderWindow;
    maFrameData.mbDragFull      = FALSE;
    maFrameData.mnHitTest       = 0;
    maFrameData.mnPinState      = 0;
    maFrameData.mnCloseState    = 0;
    maFrameData.mnRollState     = 0;
    maFrameData.mnDockState     = 0;
    maFrameData.mnHideState     = 0;
    maFrameData.mnHelpState     = 0;

    mbPressed                   = FALSE;
}

// -----------------------------------------------------------------------

ImplMacBorderWindowView::~ImplMacBorderWindowView()
{
}

// -----------------------------------------------------------------------

BOOL ImplMacBorderWindowView::MouseMove( const MouseEvent& rMEvt )
{
    return ImplMouseMove( &maFrameData, rMEvt );
}

// -----------------------------------------------------------------------

BOOL ImplMacBorderWindowView::MouseButtonDown( const MouseEvent& rMEvt )
{
    return ImplMouseButtonDown( &maFrameData, rMEvt );
}

// -----------------------------------------------------------------------

BOOL ImplMacBorderWindowView::Tracking( const TrackingEvent& rTEvt )
{
    return ImplTracking( &maFrameData, rTEvt );
}

// -----------------------------------------------------------------------

USHORT ImplMacBorderWindowView::RequestHelp( const Point& rPos, Rectangle& rHelpRect )
{
    return ImplRequestHelp( &maFrameData, rPos, rHelpRect );
}

// -----------------------------------------------------------------------

void ImplMacBorderWindowView::Init( OutputDevice* pDev, long nWidth, long nHeight )
{
    ImplBorderFrameData*    pData = &maFrameData;
    ImplBorderWindow*       pBorderWindow = maFrameData.mpBorderWindow;
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();

    pData->mpOutDev         = pDev;
    pData->mnWidth          = nWidth;
    pData->mnHeight         = nHeight;

    pData->mnTitleType      = pBorderWindow->mnTitleType;
    pData->mbFloatWindow    = pBorderWindow->mbFloatWindow;

    if ( !(pBorderWindow->GetStyle() & WB_MOVEABLE) || (pData->mnTitleType == BORDERWINDOW_TITLE_NONE) )
        pData->mnBorderSize = 0;
    else if ( pData->mnTitleType == BORDERWINDOW_TITLE_TEAROFF )
        pData->mnBorderSize = 1;
    else
        pData->mnBorderSize = rStyleSettings.GetBorderSize();
    pData->mnLeftBorder     = 2;
    pData->mnTopBorder      = 2;
    pData->mnRightBorder    = 2;
    pData->mnBottomBorder   = 2;
    pData->mnLeftBorder    += pData->mnBorderSize;
    pData->mnTopBorder     += pData->mnBorderSize;
    pData->mnRightBorder   += pData->mnBorderSize;
    pData->mnBottomBorder  += pData->mnBorderSize;
    pData->mnNoTitleTop     = pData->mnTopBorder;
    pData->mnTitleOff       = 0;

    ImplInitTitle( &maFrameData );
    if ( pData->mnTitleHeight )
    {
        if ( (pData->mnTitleType & (BORDERWINDOW_TITLE_NORMAL | BORDERWINDOW_TITLE_SMALL)) &&
             pData->mnBorderSize )
            pData->mnTitleOff = 3;  // 3 damit Hoehe auf dem MAC stimmt, ansonsten waere 2 richtig
        pData->mnTitleHeight        -= pData->mnTitleOff;
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

            if ( pBorderWindow->GetStyle() & WB_CLOSEABLE )
            {
                pData->maCloseRect.Top()    = nItemTop;
                pData->maCloseRect.Bottom() = nItemBottom;
                pData->maCloseRect.Left()   = nLeft;
                pData->maCloseRect.Right()  = pData->maCloseRect.Left()+pData->maCloseRect.GetHeight()-1;
                nLeft += pData->maCloseRect.GetWidth()+3;
            }

            if ( pBorderWindow->GetStyle() & WB_PINABLE )
            {
                Image aImage;
                ImplGetPinImage( 0, 0, aImage );
                pData->maPinRect.Top()    = nItemTop;
                pData->maPinRect.Bottom() = nItemBottom;
                pData->maPinRect.Left()   = nLeft;
                pData->maPinRect.Right()  = pData->maPinRect.Left()+aImage.GetSizePixel().Width();
                nLeft += pData->maPinRect.GetWidth()+3;
            }

            if ( pBorderWindow->mbDockBtn )
            {
                pData->maDockRect.Top()    = nItemTop;
                pData->maDockRect.Bottom() = nItemBottom;
                pData->maDockRect.Right()  = nRight;
                pData->maDockRect.Left()   = pData->maDockRect.Right()-pData->maDockRect.GetHeight()+1;
                nRight -= pData->maDockRect.GetWidth()+3;
            }

            if ( pBorderWindow->mbHideBtn )
            {
                pData->maHideRect.Top()    = nItemTop;
                pData->maHideRect.Bottom() = nItemBottom;
                pData->maHideRect.Right()  = nRight;
                pData->maHideRect.Left()   = pData->maHideRect.Right()-pData->maHideRect.GetHeight()+1;
                nRight -= pData->maHideRect.GetWidth()+3;
            }

            if ( pBorderWindow->GetStyle() & WB_ROLLABLE )
            {
                pData->maRollRect.Top()    = nItemTop;
                pData->maRollRect.Bottom() = nItemBottom;
                pData->maRollRect.Right()  = nRight;
                pData->maRollRect.Left()   = pData->maRollRect.Right()-pData->maRollRect.GetHeight()+1;
                nRight -= pData->maRollRect.GetWidth()+3;
            }

            if ( pBorderWindow->mbHelpBtn )
            {
                pData->maHelpRect.Top()    = nItemTop;
                pData->maHelpRect.Bottom() = nItemBottom;
                pData->maHelpRect.Right()  = nRight;
                pData->maHelpRect.Left()   = pData->maHelpRect.Right()-pData->maHelpRect.GetHeight()+1;
                nRight -= pData->maHelpRect.GetWidth()+3;
            }
        }
        else
        {
            pData->maCloseRect.SetEmpty();
            pData->maDockRect.SetEmpty();
            pData->maHideRect.SetEmpty();
            pData->maRollRect.SetEmpty();
            pData->maHelpRect.SetEmpty();
        }

        pData->mnTopBorder  += pData->mnTitleHeight;

        // Innerer Border nur, wenn wir auch eine TitleBar haben
        if ( !pBorderWindow->mbRollUp || pBorderWindow->mnRollHeight )
        {
            pData->mnTopBorder      += 3+pData->mnBorderSize;
            pData->mnBottomBorder   += 3;
        }
        pData->mnLeftBorder     += 3;
        pData->mnRightBorder    += 3;
    }
    else
    {
        pData->maTitleRect.SetEmpty();
        pData->maPinRect.SetEmpty();
        pData->maCloseRect.SetEmpty();
        pData->maDockRect.SetEmpty();
        pData->maHideRect.SetEmpty();
        pData->maRollRect.SetEmpty();
        pData->maHelpRect.SetEmpty();
    }
}

// -----------------------------------------------------------------------

void ImplMacBorderWindowView::GetBorder( long& rLeftBorder, long& rTopBorder,
                                         long& rRightBorder, long& rBottomBorder ) const
{
    rLeftBorder     = maFrameData.mnLeftBorder;
    rTopBorder      = maFrameData.mnTopBorder;
    rRightBorder    = maFrameData.mnRightBorder;
    rBottomBorder   = maFrameData.mnBottomBorder;
}

// -----------------------------------------------------------------------

long ImplMacBorderWindowView::CalcTitleWidth() const
{
    return ImplCalcTitleWidth( &maFrameData );
}

// -----------------------------------------------------------------------

void ImplMacBorderWindowView::DrawWindow( USHORT nDrawFlags, OutputDevice*, const Point* )
{
    ImplBorderFrameData*    pData = &maFrameData;
    OutputDevice*           pDev = pData->mpOutDev;
    ImplBorderWindow*       pBorderWindow = pData->mpBorderWindow;
    Point                   aTmpPoint;
    Rectangle               aInRect( aTmpPoint, Size( pData->mnWidth, pData->mnHeight ) );
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();
    BOOL                    bActive = pBorderWindow->IsDisplayActive();

    // DrawFrame
    pBorderWindow->SetFillColor();
    if ( nDrawFlags & BORDERWINDOW_DRAW_FRAME )
    {
        if ( bActive )
            pDev->SetLineColor( rStyleSettings.GetDarkShadowColor() );
        else
            pDev->SetLineColor( rStyleSettings.GetShadowColor() );
        pDev->DrawRect( aInRect );
        aInRect.Left()++;
        aInRect.Top()++;
        aInRect.Right()--;
        aInRect.Bottom()--;
        if ( bActive )
        {
            pDev->SetLineColor( rStyleSettings.GetLightColor() );
            pDev->DrawLine( aInRect.TopLeft(), Point( aInRect.Left(), aInRect.Bottom() ) );
            pDev->DrawLine( aInRect.TopLeft(), Point( aInRect.Right(), aInRect.Top() ) );
            pDev->SetLineColor( rStyleSettings.GetShadowColor() );
            pDev->DrawLine( Point( aInRect.Left()+1, aInRect.Bottom() ), aInRect.BottomRight() );
            pDev->DrawLine( Point( aInRect.Right(), aInRect.Top()+1 ), aInRect.BottomRight() );
        }
        else
        {
            pDev->SetLineColor( rStyleSettings.GetDeactiveBorderColor() );
            pDev->DrawRect( aInRect );
        }
        aInRect.Left()++;
        aInRect.Top()++;
        aInRect.Right()--;
        aInRect.Bottom()--;
    }
    else
    {
        aInRect.Left()   += 2;
        aInRect.Top()    += 2;
        aInRect.Right()  -= 2;
        aInRect.Bottom() -= 2;
    }

    pBorderWindow->SetLineColor();
    long nBorderSize = pData->mnBorderSize;
    if ( (nDrawFlags & BORDERWINDOW_DRAW_BORDER) && nBorderSize )
    {
        if ( bActive )
            pDev->SetFillColor( rStyleSettings.GetActiveBorderColor() );
        else
            pDev->SetFillColor( rStyleSettings.GetDeactiveBorderColor() );
        pDev->DrawRect( Rectangle( Point( aInRect.Left(), aInRect.Top() ),
                                   Size( aInRect.GetWidth(), nBorderSize ) ) );
        pDev->DrawRect( Rectangle( Point( aInRect.Left(), aInRect.Top()+nBorderSize ),
                                   Size( nBorderSize, aInRect.GetHeight()-nBorderSize ) ) );
        pDev->DrawRect( Rectangle( Point( aInRect.Left(), aInRect.Bottom()-nBorderSize+1 ),
                                   Size( aInRect.GetWidth(), nBorderSize ) ) );
        pDev->DrawRect( Rectangle( Point( aInRect.Right()-nBorderSize+1, aInRect.Top()+nBorderSize ),
                                   Size( nBorderSize, aInRect.GetHeight()-nBorderSize ) ) );
    }
    aInRect.Left()   += nBorderSize;
    aInRect.Top()    += nBorderSize;
    aInRect.Right()  -= nBorderSize;
    aInRect.Bottom() -= nBorderSize;

    Rectangle   aTitleRect = pData->maTitleRect;
    XubString   aText = pBorderWindow->GetText();
    BOOL        bDrawText = FALSE;
    if ( (nDrawFlags & BORDERWINDOW_DRAW_TITLE) && !pData->maTitleRect.IsEmpty() )
    {
        Rectangle aTextRect;
        if ( bActive )
        {
            pDev->SetFillColor( rStyleSettings.GetActiveColor() );
            pDev->SetTextColor( rStyleSettings.GetActiveTextColor() );
        }
        else
        {
            pDev->SetFillColor( rStyleSettings.GetDeactiveColor() );
            pDev->SetTextColor( rStyleSettings.GetDeactiveTextColor() );
        }
        pDev->DrawRect( aTitleRect );

        if ( pData->mnTitleType != BORDERWINDOW_TITLE_TEAROFF )
        {
            if ( !pData->maPinRect.IsEmpty() )
                aTitleRect.Left() = pData->maPinRect.Right()+3;
            else if ( !pData->maCloseRect.IsEmpty() )
                aTitleRect.Left() = pData->maCloseRect.Right()+3;

            if ( !pData->maHelpRect.IsEmpty() )
                aTitleRect.Right() = pData->maHelpRect.Left()-3;
            else if ( !pData->maHideRect.IsEmpty() )
                aTitleRect.Right() = pData->maHideRect.Left()-3;
            else if ( !pData->maDockRect.IsEmpty() )
                aTitleRect.Right() = pData->maDockRect.Left()-3;
            else if ( !pData->maRollRect.IsEmpty() )
                aTitleRect.Right() = pData->maRollRect.Left()-3;

            if ( aText.Len() )
            {
                aTextRect = pDev->GetTextRect( aTitleRect, aText,
                                               TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER |
                                               TEXT_DRAW_ENDELLIPSIS );
                bDrawText = TRUE;
            }
        }

        if ( bActive )
        {
            long nY         = aTitleRect.Top();
            long nYMax      = nY+pData->mnTitleHeight-2+(pData->mnTitleHeight%2);
            long nX1        = aTitleRect.Left();
            long nX2;
            long nX3;
            long nX4;
            BOOL bLines;
            BOOL b2Lines;
            if ( aTextRect.IsEmpty() )
            {
                nX2     = aTitleRect.Right();
                bLines  = nX2-2 > nX1;
                b2Lines = FALSE;
            }
            else
            {
                aTextRect.Left()  -= 4;
                aTextRect.Right() += 4;
                if ( (aTextRect.Left() > aTitleRect.Left()) &&
                     (aTextRect.Right() < aTitleRect.Right()) )
                {
                    nX2     = aTextRect.Left();
                    nX3     = aTextRect.Right();
                    nX4     = aTitleRect.Right();
                    bLines  = TRUE;
                    b2Lines = TRUE;
                }
                else
                    bLines = FALSE;
            }

            if ( bLines )
            {
                for ( ; nY < nYMax; nY += 2 )
                {
                    if ( rStyleSettings.GetOptions() & STYLE_OPTION_MONO )
                        pDev->SetLineColor( Color( COL_BLACK ) );
                    else
                        pDev->SetLineColor( rStyleSettings.GetLightColor() );
                    pDev->DrawLine( Point( nX1, nY ), Point( nX2-1, nY ) );
                    if ( b2Lines )
                        pDev->DrawLine( Point( nX3, nY ), Point( nX4-1, nY ) );
                    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
                    {
                        pDev->SetLineColor( rStyleSettings.GetShadowColor() );
                        pDev->DrawLine( Point( nX1+1, nY+1 ), Point( nX2, nY+1 ) );
                        if ( b2Lines )
                            pDev->DrawLine( Point( nX3+1, nY+1 ), Point( nX4, nY+1 ) );
                    }
                }
            }
        }
    }
    aInRect.Top() += pData->mnTitleHeight;

    if ( bActive )
    {
        if ( ((nDrawFlags & BORDERWINDOW_DRAW_CLOSE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
             !pData->maCloseRect.IsEmpty() )
            DrawMacTitleButton( pData->maCloseRect, pData->mnCloseState );

        if ( ((nDrawFlags & BORDERWINDOW_DRAW_PIN) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
             !pData->maPinRect.IsEmpty() )
        {
            Image aImage;
            ImplGetPinImage( pData->mnPinState, pBorderWindow->mbPined, aImage );
            Size  aImageSize = aImage.GetSizePixel();
            long  nRectHeight = pData->maPinRect.GetHeight();
            if ( nRectHeight < aImageSize.Height() )
            {
                pDev->DrawImage( Point( pData->maPinRect.Left(), pData->maPinRect.Top() ),
                                 Size( aImageSize.Width(), nRectHeight ),
                                 aImage );
            }
            else
            {
                pDev->DrawImage( Point( pData->maPinRect.Left(),
                                        pData->maPinRect.Top()+(nRectHeight-aImageSize.Height())/2 ),
                                 aImage );
            }
        }

        if ( ((nDrawFlags & BORDERWINDOW_DRAW_DOCK) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
             !pData->maDockRect.IsEmpty() )
        {
            Rectangle aInDockRect = DrawMacTitleButton( pData->maDockRect, pData->mnDockState );
            pDev->SetLineColor( rStyleSettings.GetDarkShadowColor() );
            pDev->SetFillColor();
            aInDockRect.Left()++;
            aInDockRect.Top()++;
            aInDockRect.Right()--;
            aInDockRect.Bottom()--;
            pDev->DrawRect( aInDockRect );
        }

        if ( ((nDrawFlags & BORDERWINDOW_DRAW_HIDE) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
             !pData->maHideRect.IsEmpty() )
        {
            Rectangle aInHideRect = DrawMacTitleButton( pData->maHideRect, pData->mnHideState );
            pDev->SetLineColor( rStyleSettings.GetDarkShadowColor() );
            pDev->DrawLine( Point( aInHideRect.Left(), aInHideRect.Bottom()-1 ), Point( aInHideRect.Right(), aInHideRect.Bottom()-1 ) );
            pDev->DrawLine( Point( aInHideRect.Left(), aInHideRect.Bottom() ), Point( aInHideRect.Right(), aInHideRect.Bottom() ) );
        }

        if ( ((nDrawFlags & BORDERWINDOW_DRAW_ROLL) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
             !pData->maRollRect.IsEmpty() )
        {
            Rectangle aInRollRect = DrawMacTitleButton( pData->maRollRect, pData->mnRollState );
            pDev->SetLineColor( rStyleSettings.GetDarkShadowColor() );
            long nY = aInRollRect.Center().Y();
            pDev->DrawLine( Point( aInRollRect.Left(), nY-1 ), Point( aInRollRect.Right(), nY-1 ) );
            pDev->DrawLine( Point( aInRollRect.Left(), nY+1 ), Point( aInRollRect.Right(), nY+1 ) );
        }

        if ( ((nDrawFlags & BORDERWINDOW_DRAW_HELP) || (nDrawFlags & BORDERWINDOW_DRAW_TITLE)) &&
             !pData->maHelpRect.IsEmpty() )
        {
            Rectangle aInHelpRect = DrawMacTitleButton( pData->maHelpRect, pData->mnHelpState );
            // ...
        }
    }

    if ( (nDrawFlags & BORDERWINDOW_DRAW_BORDER) && nBorderSize && !pData->maTitleRect.IsEmpty() )
    {
        pBorderWindow->SetLineColor();
        if ( bActive )
            pDev->SetFillColor( rStyleSettings.GetActiveColor() );
        else
            pDev->SetFillColor( rStyleSettings.GetDeactiveColor() );
        pDev->DrawRect( Rectangle( Point( aInRect.Left(), aInRect.Top() ),
                                   Size( aInRect.GetWidth(), nBorderSize ) ) );
    }
    aInRect.Top() += nBorderSize;

    if ( (nDrawFlags & BORDERWINDOW_DRAW_FRAME) && !pData->maTitleRect.IsEmpty() &&
         (!pBorderWindow->mbRollUp || pBorderWindow->mnRollHeight) )
    {
        pBorderWindow->SetFillColor();
        if ( bActive )
        {
            pDev->SetLineColor( rStyleSettings.GetShadowColor() );
            pDev->DrawLine( aInRect.TopLeft(), Point( aInRect.Left(), aInRect.Bottom() ) );
            pDev->DrawLine( aInRect.TopLeft(), Point( aInRect.Right(), aInRect.Top() ) );
            pDev->SetLineColor( rStyleSettings.GetLightColor() );
            pDev->DrawLine( Point( aInRect.Left()+1, aInRect.Bottom() ), aInRect.BottomRight() );
            pDev->DrawLine( Point( aInRect.Right(), aInRect.Top()+1 ), aInRect.BottomRight() );
        }
        else
        {
            pDev->SetLineColor( rStyleSettings.GetDeactiveBorderColor() );
            pDev->DrawRect( aInRect );
        }
        aInRect.Left()++;
        aInRect.Top()++;
        aInRect.Right()--;
        aInRect.Bottom()--;
        if ( bActive )
            pDev->SetLineColor( rStyleSettings.GetDarkShadowColor() );
        else
            pDev->SetLineColor( rStyleSettings.GetShadowColor() );
        pDev->DrawRect( aInRect );
        aInRect.Left()++;
        aInRect.Top()++;
        aInRect.Right()--;
        aInRect.Bottom()--;
        if ( bActive )
        {
            pDev->SetLineColor( rStyleSettings.GetLightColor() );
            pDev->DrawLine( aInRect.TopLeft(), Point( aInRect.Left(), aInRect.Bottom() ) );
            pDev->DrawLine( aInRect.TopLeft(), Point( aInRect.Right(), aInRect.Top() ) );
            pDev->SetLineColor( rStyleSettings.GetShadowColor() );
            pDev->DrawLine( Point( aInRect.Left()+1, aInRect.Bottom() ), aInRect.BottomRight() );
            pDev->DrawLine( Point( aInRect.Right(), aInRect.Top()+1 ), aInRect.BottomRight() );
        }
        else
        {
            pDev->SetLineColor( rStyleSettings.GetDeactiveBorderColor() );
            pDev->DrawRect( aInRect );
        }
    }

    // Text als letztes zeichen, da auf dem MAC unter/ober-Laengen in
    // den Border gezeichnet werden
    if ( bDrawText )
    {
        pDev->DrawText( aTitleRect, aText,
                         TEXT_DRAW_CENTER | TEXT_DRAW_VCENTER |
                         TEXT_DRAW_ENDELLIPSIS );
    }
}

//fuer WIN16 Borland
#ifdef WIN
#pragma codeseg BRWDIN_SEG1
#endif

// -----------------------------------------------------------------------

Rectangle ImplMacBorderWindowView::DrawMacTitleButton( const Rectangle& rRect, USHORT nStyle )
{
    OutputDevice*           pDev = maFrameData.mpOutDev;
    const StyleSettings&    rStyleSettings = pDev->GetSettings().GetStyleSettings();
    Rectangle               aRect = rRect;

    if ( !(rStyleSettings.GetOptions() & STYLE_OPTION_MONO) )
    {
        pDev->SetLineColor( rStyleSettings.GetShadowColor() );
        pDev->DrawLine( aRect.TopLeft(), Point( aRect.Left(), aRect.Bottom() ) );
        pDev->DrawLine( aRect.TopLeft(), Point( aRect.Right(), aRect.Top() ) );
        pDev->SetLineColor( rStyleSettings.GetLightColor() );
        pDev->DrawLine( Point( aRect.Left()+1, aRect.Bottom() ), aRect.BottomRight() );
        pDev->DrawLine( Point( aRect.Right(), aRect.Top()+1 ), aRect.BottomRight() );
        aRect.Left()++;
        aRect.Top()++;
        aRect.Right()--;
        aRect.Bottom()--;
        pDev->SetLineColor( rStyleSettings.GetDarkShadowColor() );
        BOOL bDrawRect;
        if ( pDev->GetColorCount() >= 256 )
        {
            Point   aTempPoint;
            Size    aRectSize = aRect.GetSize();
            BOOL    bNewPressed = (nStyle & BUTTON_DRAW_PRESSED) != 0;
            bDrawRect = FALSE;
            if ( (aRectSize != maVirDev.GetOutputSizePixel()) ||
                 (bNewPressed != mbPressed) )
            {
                Gradient aGradient( GRADIENT_LINEAR,
                                    rStyleSettings.GetShadowColor(),
                                    rStyleSettings.GetLightColor() );
                if ( bNewPressed )
                {
                    aGradient.SetStartColor( rStyleSettings.GetDarkShadowColor() );
                    aGradient.SetEndColor( rStyleSettings.GetFaceColor() );
                }
                aGradient.SetAngle( 450 );
                if ( maVirDev.SetOutputSizePixel( aRectSize ) )
                    maVirDev.DrawGradient( Rectangle( aTempPoint, aRectSize ), aGradient );
                else
                    bDrawRect = TRUE;
                mbPressed = bNewPressed;
            }
            if ( !bDrawRect )
            {
                pDev->DrawOutDev( aRect.TopLeft(), aRectSize,
                                  aTempPoint, aRectSize, maVirDev );
                pDev->SetFillColor();
            }
        }
        else
            bDrawRect = TRUE;
        if ( bDrawRect )
        {
            if ( nStyle & BUTTON_DRAW_PRESSED )
                pDev->SetFillColor( rStyleSettings.GetShadowColor() );
            else
                pDev->SetFillColor( rStyleSettings.GetActiveColor() );
        }
        pDev->DrawRect( aRect );
        aRect.Left()++;
        aRect.Top()++;
        aRect.Right()--;
        aRect.Bottom()--;
        if ( nStyle & BUTTON_DRAW_PRESSED )
            pDev->SetLineColor( rStyleSettings.GetShadowColor() );
        else
            pDev->SetLineColor( rStyleSettings.GetLightColor() );
        pDev->DrawLine( aRect.TopLeft(), Point( aRect.Left(), aRect.Bottom() ) );
        pDev->DrawLine( aRect.TopLeft(), Point( aRect.Right(), aRect.Top() ) );
        pDev->SetLineColor( rStyleSettings.GetShadowColor() );
        pDev->DrawLine( Point( aRect.Left()+1, aRect.Bottom() ), aRect.BottomRight() );
        pDev->DrawLine( Point( aRect.Right(), aRect.Top()+1 ), aRect.BottomRight() );
    }
    else
    {
        pDev->SetLineColor( Color( COL_BLACK ) );
        if ( nStyle & BUTTON_DRAW_PRESSED )
            pDev->SetFillColor( Color( COL_BLACK ) );
        else
            pDev->SetFillColor( rStyleSettings.GetActiveColor() );
        pDev->DrawRect( aRect );
        aRect.Left()++;
        aRect.Top()++;
        aRect.Right()--;
        aRect.Bottom()--;
    }

    return aRect;
}

// =======================================================================
#ifdef REMOTE_APPSERVER
void ImplBorderWindow::ImplInit( Window* pParent,
                                 WinBits nStyle, USHORT nTypeStyle,
                                 SystemParentData* pSystemParentData
                                 )
{
    static ::com::sun::star::uno::Any aVoid;

    DBG_ASSERT( pSystemParentData, "remote and non remote confusion, please clarify" );
    ImplInit( pParent, nStyle, nTypeStyle, aVoid );
}
#else
void ImplBorderWindow::ImplInit( Window* pParent,
                                 WinBits nStyle, USHORT nTypeStyle,
                                 const ::com::sun::star::uno::Any& aSystemToken )
{
    ImplInit( pParent, nStyle, nTypeStyle, NULL );
}
#endif

#ifndef REMOTE_APPSERVER
void ImplBorderWindow::ImplInit( Window* pParent,
                                 WinBits nStyle, USHORT nTypeStyle,
                                 SystemParentData* pSystemParentData
                                 )
#else
void ImplBorderWindow::ImplInit( Window* pParent,
                                 WinBits nStyle,
                                 USHORT nTypeStyle,
                                 const ::com::sun::star::uno::Any& aSystemToken
                                 )
#endif
{
    // Alle WindowBits entfernen, die wir nicht haben wollen
    WinBits nOrgStyle = nStyle;
    WinBits nTestStyle = (WB_MOVEABLE | WB_SIZEABLE | WB_ROLLABLE | WB_PINABLE | WB_CLOSEABLE | WB_STANDALONE | WB_DIALOGCONTROL | WB_NODIALOGCONTROL);
    if ( nTypeStyle & BORDERWINDOW_STYLE_APP )
        nTestStyle |= WB_APP;
    nStyle &= nTestStyle;

    mbBorderWin         = TRUE;
    mbSmallOutBorder    = FALSE;
    if ( nTypeStyle & BORDERWINDOW_STYLE_FRAME )
    {
        mbOverlapWin    = TRUE;
        mbFrame         = TRUE;
        mbFrameBorder   = FALSE;
        if ( (nOrgStyle & (WB_BORDER | WB_NOBORDER | WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE)) == WB_BORDER )
            mbSmallOutBorder = TRUE;
    }
    else if ( nTypeStyle & BORDERWINDOW_STYLE_OVERLAP )
    {
        mbOverlapWin    = TRUE;
        mbFrameBorder   = TRUE;
    }
    else
        mbFrameBorder   = FALSE;

    if ( nTypeStyle & BORDERWINDOW_STYLE_FLOAT )
        mbFloatWindow = TRUE;
    else
        mbFloatWindow = FALSE;

#ifndef REMOTE_APPSERVER
    Window::ImplInit( pParent, nStyle, pSystemParentData );
#else
    Window::ImplInit( pParent, nStyle, aSystemToken );
#endif
    SetBackground();
    SetTextFillColor();

    mpMenuBarWindow = NULL;
    mnMinWidth      = 0;
    mnMinHeight     = 0;
    mnRollHeight    = 0;
    mnOrgMenuHeight = 0;
    mbPined         = FALSE;
    mbRollUp        = FALSE;
    mbMenuHide      = FALSE;
    mbDockBtn       = FALSE;
    mbHideBtn       = FALSE;
    mbHelpBtn       = FALSE;
    mbDisplayActive = IsActive();

    if ( nTypeStyle & BORDERWINDOW_STYLE_FLOAT )
        mnTitleType = BORDERWINDOW_TITLE_SMALL;
    else
        mnTitleType = BORDERWINDOW_TITLE_NORMAL;
    mnBorderStyle   = WINDOW_BORDER_NORMAL;
    InitView();
}

// =======================================================================

ImplBorderWindow::ImplBorderWindow( Window* pParent,
                                    SystemParentData* pSystemParentData,
                                    WinBits nStyle, USHORT nTypeStyle
                                    ) : Window( WINDOW_BORDERWINDOW )
{
    ImplInit( pParent, nStyle, nTypeStyle, pSystemParentData );
}

// -----------------------------------------------------------------------

ImplBorderWindow::ImplBorderWindow( Window* pParent, WinBits nStyle ,
                                    USHORT nTypeStyle ) :
    Window( WINDOW_BORDERWINDOW )
{
    ImplInit( pParent, nStyle, nTypeStyle, ::com::sun::star::uno::Any() );
}

ImplBorderWindow::ImplBorderWindow( Window* pParent,
                                    WinBits nStyle, USHORT nTypeStyle,
                                    const ::com::sun::star::uno::Any& aSystemToken ) :
    Window( WINDOW_BORDERWINDOW )
{
    ImplInit( pParent, nStyle, nTypeStyle, aSystemToken );
}

// -----------------------------------------------------------------------

ImplBorderWindow::~ImplBorderWindow()
{
    delete mpBorderView;
}

// -----------------------------------------------------------------------

void ImplBorderWindow::MouseMove( const MouseEvent& rMEvt )
{
    mpBorderView->MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void ImplBorderWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    mpBorderView->MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void ImplBorderWindow::Tracking( const TrackingEvent& rTEvt )
{
    mpBorderView->Tracking( rTEvt );
}

// -----------------------------------------------------------------------

void ImplBorderWindow::Paint( const Rectangle& rRect )
{
    mpBorderView->DrawWindow( BORDERWINDOW_DRAW_ALL );
}

void ImplBorderWindow::Draw( const Rectangle& rRect, OutputDevice* pOutDev, const Point& rPos )
{
    mpBorderView->DrawWindow( BORDERWINDOW_DRAW_ALL, pOutDev, &rPos );
}

// -----------------------------------------------------------------------

void ImplBorderWindow::Activate()
{
    SetDisplayActive( TRUE );
    Window::Activate();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::Deactivate()
{
    // Fenster die immer Active sind, nehmen wir von dieser Regel aus,
    // genauso, wenn ein Menu aktiv wird, ignorieren wir das Deactivate
    if ( GetActivateMode() && !ImplGetSVData()->maWinData.mbNoDeactivate )
        SetDisplayActive( FALSE );
    Window::Deactivate();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::RequestHelp( const HelpEvent& rHEvt )
{
    if ( rHEvt.GetMode() & (HELPMODE_BALLOON | HELPMODE_QUICK) )
    {
        Point       aMousePosPixel = ScreenToOutputPixel( rHEvt.GetMousePosPixel() );
        Rectangle   aHelpRect;
        USHORT      nHelpResId = mpBorderView->RequestHelp( aMousePosPixel, aHelpRect );

        // Rechteck ermitteln
        if ( nHelpResId )
        {
            Point aPt = OutputToScreenPixel( aHelpRect.TopLeft() );
            aHelpRect.Left()   = aPt.X();
            aHelpRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aHelpRect.BottomRight() );
            aHelpRect.Right()  = aPt.X();
            aHelpRect.Bottom() = aPt.Y();

            // Text ermitteln und anzeigen
            XubString aStr( ResId( nHelpResId, ImplGetResMgr() ) );
            if ( rHEvt.GetMode() & HELPMODE_BALLOON )
                Help::ShowBalloon( this, aHelpRect.Center(), aHelpRect, aStr );
            else
                Help::ShowQuickHelp( this, aHelpRect, aStr );
            return;
        }
    }

    Window::RequestHelp( rHEvt );
}

// -----------------------------------------------------------------------

void ImplBorderWindow::Resize()
{
    Size aSize = GetOutputSizePixel();

    if ( !mbRollUp )
    {
        Window* pClientWindow = ImplGetClientWindow();

        if ( mpMenuBarWindow )
        {
            long nLeftBorder;
            long nTopBorder;
            long nRightBorder;
            long nBottomBorder;
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
            mpMenuBarWindow->SetPosSizePixel( nLeftBorder,
                                              nTopBorder,
                                              aSize.Width()-nLeftBorder-nRightBorder,
                                              nMenuHeight,
                                              WINDOW_POSSIZE_POS |
                                              WINDOW_POSSIZE_WIDTH | WINDOW_POSSIZE_HEIGHT );
        }

        GetBorder( pClientWindow->mnLeftBorder, pClientWindow->mnTopBorder,
                   pClientWindow->mnRightBorder, pClientWindow->mnBottomBorder );
        pClientWindow->ImplPosSizeWindow( pClientWindow->mnLeftBorder,
                                          pClientWindow->mnTopBorder,
                                          aSize.Width()-pClientWindow->mnLeftBorder-pClientWindow->mnRightBorder,
                                          aSize.Height()-pClientWindow->mnTopBorder-pClientWindow->mnBottomBorder,
                                          WINDOW_POSSIZE_X | WINDOW_POSSIZE_Y |
                                          WINDOW_POSSIZE_WIDTH | WINDOW_POSSIZE_HEIGHT );
    }

    // UpdateView
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();

    Window::Resize();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::StateChanged( StateChangedType nType )
{
    if ( (nType == STATE_CHANGE_TEXT) ||
         (nType == STATE_CHANGE_IMAGE) ||
         (nType == STATE_CHANGE_DATA) )
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

// -----------------------------------------------------------------------

void ImplBorderWindow::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_FONTS) ||
         (rDCEvt.GetType() == DATACHANGED_FONTSUBSTITUTION) ||
         ((rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
          (rDCEvt.GetFlags() & SETTINGS_STYLE)) )
    {
        if ( !mbFrame )
            UpdateView( TRUE, ImplGetWindow()->GetOutputSizePixel() );
    }

    Window::DataChanged( rDCEvt );
}

// -----------------------------------------------------------------------

void ImplBorderWindow::InitView()
{
    if ( mbSmallOutBorder )
        mpBorderView = new ImplSmallBorderWindowView( this );
    else if ( mbFrame )
        mpBorderView = new ImplNoBorderWindowView( this );
    else if ( !mbFrameBorder )
        mpBorderView = new ImplSmallBorderWindowView( this );
    else if ( GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_MACSTYLE )
        mpBorderView = new ImplMacBorderWindowView( this );
    else if ( GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_OS2STYLE )
        mpBorderView = new ImplOS2BorderWindowView( this );
    else if ( GetSettings().GetStyleSettings().GetOptions() & STYLE_OPTION_UNIXSTYLE )
        mpBorderView = new ImplUnxBorderWindowView( this );
    else
        mpBorderView = new ImplStdBorderWindowView( this );
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
}

// -----------------------------------------------------------------------

void ImplBorderWindow::UpdateView( BOOL bNewView, const Size& rNewOutSize )
{
    long nLeftBorder;
    long nTopBorder;
    long nRightBorder;
    long nBottomBorder;
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

    Window* pClientWindow = ImplGetClientWindow();
    if ( pClientWindow )
    {
        GetBorder( pClientWindow->mnLeftBorder, pClientWindow->mnTopBorder,
                   pClientWindow->mnRightBorder, pClientWindow->mnBottomBorder );
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

// -----------------------------------------------------------------------

void ImplBorderWindow::InvalidateBorder()
{
    if ( IsReallyVisible() )
    {
        // Nur wenn wir einen Border haben, muessen wir auch invalidieren
        long nLeftBorder;
        long nTopBorder;
        long nRightBorder;
        long nBottomBorder;
        mpBorderView->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
        if ( nLeftBorder || nTopBorder || nRightBorder || nBottomBorder )
        {
            Rectangle   aWinRect( Point( 0, 0 ), GetOutputSizePixel() );
            Region      aRegion( aWinRect );
            aWinRect.Left()   += nLeftBorder;
            aWinRect.Top()    += nTopBorder;
            aWinRect.Right()  -= nRightBorder;
            aWinRect.Bottom() -= nBottomBorder;
            // kein Output-Bereich mehr, dann alles invalidieren
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

// -----------------------------------------------------------------------

void ImplBorderWindow::SetDisplayActive( BOOL bActive )
{
    if ( mbDisplayActive != bActive )
    {
        mbDisplayActive = bActive;
        if ( mbFrameBorder )
            InvalidateBorder();
    }
}

// -----------------------------------------------------------------------

void ImplBorderWindow::SetTitleType( USHORT nTitleType, const Size& rSize )
{
    mnTitleType = nTitleType;
    UpdateView( FALSE, rSize );
}

// -----------------------------------------------------------------------

void ImplBorderWindow::SetBorderStyle( USHORT nStyle )
{
    if ( !mbFrameBorder && (mnBorderStyle != nStyle) )
    {
        mnBorderStyle = nStyle;
        UpdateView( FALSE, ImplGetWindow()->GetOutputSizePixel() );
    }
}

// -----------------------------------------------------------------------

void ImplBorderWindow::SetPin( BOOL bPin )
{
    mbPined = bPin;
    InvalidateBorder();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::SetRollUp( BOOL bRollUp, const Size& rSize )
{
    mbRollUp = bRollUp;
    mnRollHeight = rSize.Height();
    UpdateView( FALSE, rSize );
}

// -----------------------------------------------------------------------

void ImplBorderWindow::SetCloser()
{
    SetStyle( GetStyle() | WB_CLOSEABLE );
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::SetDockButton( BOOL bDockButton )
{
    mbDockBtn = bDockButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::SetHideButton( BOOL bHideButton )
{
    mbHideBtn = bHideButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::SetHelpButton( BOOL bHelpButton )
{
    mbHelpBtn = bHelpButton;
    Size aSize = GetOutputSizePixel();
    mpBorderView->Init( this, aSize.Width(), aSize.Height() );
    InvalidateBorder();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::UpdateMenuHeight()
{
    Resize();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::SetMenuBarWindow( Window* pWindow )
{
    mpMenuBarWindow = pWindow;
    UpdateMenuHeight();
    if ( pWindow )
        pWindow->Show();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::SetMenuBarMode( BOOL bHide )
{
    mbMenuHide = bHide;
    UpdateMenuHeight();
}

// -----------------------------------------------------------------------

void ImplBorderWindow::GetBorder( long& rLeftBorder, long& rTopBorder,
                                  long& rRightBorder, long& rBottomBorder ) const
{
    mpBorderView->GetBorder( rLeftBorder, rTopBorder, rRightBorder, rBottomBorder );
    if ( mpMenuBarWindow && !mbMenuHide )
        rTopBorder += mpMenuBarWindow->GetSizePixel().Height();
}

// -----------------------------------------------------------------------

long ImplBorderWindow::CalcTitleWidth() const
{
    return mpBorderView->CalcTitleWidth();
}
