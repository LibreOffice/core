/*************************************************************************
 *
 *  $RCSfile: window2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: th $ $Date: 2001-04-06 12:48:42 $
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

#define _SV_WINDOW_CXX

#include <limits.h>
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#ifndef REMOTE_APPSERVER
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#else
#include <rmoutdev.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <bitmap.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_METRIC_HXX
#include <metric.hxx>
#endif
#ifndef _SV_OUTFONT_HXX
#include <outfont.hxx>
#endif
#ifndef _SV_OUTDEV_H
#include <outdev.h>
#endif
#ifndef _SV_ACCESS_HXX
#include <access.hxx>
#endif
#ifndef _SV_POLY_H
#include <poly.h>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_VIRDEV_HXX
#include <virdev.hxx>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_SCRBAR_HXX
#include <scrbar.hxx>
#endif
#ifndef _SV_SCRWND_HXX
#include <scrwnd.hxx>
#endif

#pragma hdrstop

// =======================================================================

DBG_NAMEEX( Window );

// =======================================================================

#define IMPL_MAXSAVEBACKSIZE    (640*480)
#define IMPL_MAXALLSAVEBACKSIZE (800*600*2)

// =======================================================================

struct ImplFocusDelData : public ImplDelData
{
    Window*         mpFocusWin;
};

// =======================================================================

BOOL Window::ImplIsWindowInFront( const Window* pTestWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_CHKOBJ( pTestWindow, Window, ImplDbgCheckWindow );

    // Testen, ob es Fenster untereinander liegen
    pTestWindow = pTestWindow->ImplGetFirstOverlapWindow();
    const Window* pTempWindow = pTestWindow;
    const Window* pThisWindow = ImplGetFirstOverlapWindow();
    if ( pTempWindow == pThisWindow )
        return FALSE;
    do
    {
        if ( pTempWindow == pThisWindow )
            return TRUE;
        if ( pTempWindow->mbFrame )
            break;
        pTempWindow = pTempWindow->mpOverlapWindow;
    }
    while ( pTempWindow );
    pTempWindow = pThisWindow;
    do
    {
        if ( pTempWindow == pTestWindow )
            return FALSE;
        if ( pTempWindow->mbFrame )
            break;
        pTempWindow = pTempWindow->mpOverlapWindow;
    }
    while ( pTempWindow );

    // Fenster auf gleiche Ebene bringen
    if ( pThisWindow->mpOverlapWindow != pTestWindow->mpOverlapWindow )
    {
        USHORT nThisLevel = 0;
        USHORT nTestLevel = 0;
        pTempWindow = pThisWindow;
        do
        {
            nThisLevel++;
            pTempWindow = pTempWindow->mpOverlapWindow;
        }
        while ( !pTempWindow->mbFrame );
        pTempWindow = pTestWindow;
        do
        {
            nTestLevel++;
            pTempWindow = pTempWindow->mpOverlapWindow;
        }
        while ( !pTempWindow->mbFrame );

        if ( nThisLevel < nTestLevel )
        {
            do
            {
                if ( pTestWindow->mpOverlapWindow == pThisWindow->mpOverlapWindow )
                    break;
                if ( pTestWindow->mbFrame )
                    break;
                pTestWindow = pTestWindow->mpOverlapWindow;
            }
            while ( pTestWindow );
        }
        else
        {
            do
            {
                if ( pThisWindow->mpOverlapWindow == pTempWindow->mpOverlapWindow )
                    break;
                if ( pThisWindow->mbFrame )
                    break;
                pThisWindow = pThisWindow->mpOverlapWindow;
            }
            while ( pThisWindow );
        }
    }

    // Wenn TestWindow vor ThisWindow kommt, liegt es vorne
    pTempWindow = pTestWindow;
    do
    {
        if ( pTempWindow == pThisWindow )
            return TRUE;
        pTempWindow = pTempWindow->mpNext;
    }
    while ( pTempWindow );

    return FALSE;
}

// =======================================================================

void Window::ImplSaveOverlapBackground()
{
    DBG_ASSERT( !mpOverlapData->mpSaveBackDev, "Window::ImplSaveOverlapBackground() - Background already saved" );

    if ( !mbFrame )
    {
        ULONG nSaveBackSize = mnOutWidth*mnOutHeight;
        if ( nSaveBackSize <= IMPL_MAXSAVEBACKSIZE )
        {
            if ( nSaveBackSize+mpFrameData->mnAllSaveBackSize <= IMPL_MAXALLSAVEBACKSIZE )
            {
                Size aOutSize( mnOutWidth, mnOutHeight );
                mpOverlapData->mpSaveBackDev = new VirtualDevice( *mpFrameWindow );
                if ( mpOverlapData->mpSaveBackDev->SetOutputSizePixel( aOutSize ) )
                {
                    mpFrameWindow->ImplUpdateAll();

                    if ( mbInitWinClipRegion )
                        ImplInitWinClipRegion();

                    mpOverlapData->mnSaveBackSize = nSaveBackSize;
                    mpFrameData->mnAllSaveBackSize += nSaveBackSize;
                    Point aDevPt;
                    mpFrameWindow->ImplGetFrameDev( Point( mnOutOffX, mnOutOffY ),
                                                    aDevPt, aOutSize,
                                                    *(mpOverlapData->mpSaveBackDev) );
                    mpOverlapData->mpNextBackWin = mpFrameData->mpFirstBackWin;
                    mpFrameData->mpFirstBackWin = this;
                }
                else
                {
                    delete mpOverlapData->mpSaveBackDev;
                    mpOverlapData->mpSaveBackDev = NULL;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

BOOL Window::ImplRestoreOverlapBackground( Region& rInvRegion )
{
    if ( mpOverlapData->mpSaveBackDev )
    {
        if ( mbInitWinClipRegion )
            ImplInitWinClipRegion();

        if ( mpOverlapData->mpSaveBackDev )
        {
            Point   aDevPt;
            Point   aDestPt( mnOutOffX, mnOutOffY );
            Size    aDevSize = mpOverlapData->mpSaveBackDev->GetOutputSizePixel();
            if ( mpOverlapData->mpSaveBackRgn )
            {
                mpOverlapData->mpSaveBackRgn->Intersect( maWinClipRegion );
                rInvRegion = maWinClipRegion;
                rInvRegion.Exclude( *mpOverlapData->mpSaveBackRgn );
                mpFrameWindow->ImplDrawFrameDev( aDestPt, aDevPt, aDevSize,
                                                 *(mpOverlapData->mpSaveBackDev),
                                                 *mpOverlapData->mpSaveBackRgn );
            }
            else
            {
                mpFrameWindow->ImplDrawFrameDev( aDestPt, aDevPt, aDevSize,
                                                 *(mpOverlapData->mpSaveBackDev),
                                                 maWinClipRegion );
            }
            ImplDeleteOverlapBackground();
        }

        return TRUE;
    }

    return FALSE;
}

// -----------------------------------------------------------------------

void Window::ImplDeleteOverlapBackground()
{
    if ( mpOverlapData->mpSaveBackDev )
    {
        mpFrameData->mnAllSaveBackSize -= mpOverlapData->mnSaveBackSize;
        delete mpOverlapData->mpSaveBackDev;
        mpOverlapData->mpSaveBackDev = NULL;
        if ( mpOverlapData->mpSaveBackRgn )
        {
            delete mpOverlapData->mpSaveBackRgn;
            mpOverlapData->mpSaveBackRgn = NULL;
        }

        // Fenster aus der Liste entfernen
        if ( mpFrameData->mpFirstBackWin == this )
            mpFrameData->mpFirstBackWin = mpOverlapData->mpNextBackWin;
        else
        {
            Window* pTemp = mpFrameData->mpFirstBackWin;
            while ( pTemp->mpOverlapData->mpNextBackWin != this )
                pTemp = pTemp->mpOverlapData->mpNextBackWin;
            pTemp->mpOverlapData->mpNextBackWin = mpOverlapData->mpNextBackWin;
        }
        mpOverlapData->mpNextBackWin = NULL;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInvalidateAllOverlapBackgrounds()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Window* pWindow = mpFrameData->mpFirstBackWin;
    while ( pWindow )
    {
        // Naechstes Fenster schon hier merken, da dieses Fenster in
        // der if-Abfrage aus der Liste entfernt werden kann
        Window* pNext = pWindow->mpOverlapData->mpNextBackWin;

        if ( ImplIsWindowInFront( pWindow ) )
        {
            Rectangle aRect1( Point( mnOutOffX, mnOutOffY ),
                              Size( mnOutWidth, mnOutHeight ) );
            Rectangle aRect2( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                              Size( pWindow->mnOutWidth, pWindow->mnOutHeight ) );
            aRect1.Intersection( aRect2 );
            if ( !aRect1.IsEmpty() )
            {
                if ( !pWindow->mpOverlapData->mpSaveBackRgn )
                    pWindow->mpOverlapData->mpSaveBackRgn = new Region( aRect2 );
                pWindow->mpOverlapData->mpSaveBackRgn->Exclude( aRect1 );
                if ( pWindow->mpOverlapData->mpSaveBackRgn->IsEmpty() )
                    pWindow->ImplDeleteOverlapBackground();
            }

        }

        pWindow = pNext;
    }
}

// =======================================================================

Bitmap Window::SnapShot() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Bitmap aBmp;

    if ( mpBorderWindow )
        aBmp = mpBorderWindow->SnapShot();
    else if ( IsReallyVisible() )
        mpFrameWindow->ImplGetFrameBitmap( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ), aBmp );

    return aBmp;
}

// -----------------------------------------------------------------------

void Window::ShowFocus( const Rectangle& rRect )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplWinData* pWinData = ImplGetWinData();

    if ( !mbInPaint )
    {
        if ( mbFocusVisible )
        {
            if ( *(pWinData->mpFocusRect) == rRect )
                return;

            ImplInvertFocus( *(pWinData->mpFocusRect) );
        }

        ImplInvertFocus( rRect );
    }

    if ( !pWinData->mpFocusRect )
        pWinData->mpFocusRect = new Rectangle( rRect );
    else
        *(pWinData->mpFocusRect) = rRect;
    mbFocusVisible = TRUE;
}

// -----------------------------------------------------------------------

void Window::HideFocus()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !mbFocusVisible )
        return;

    if ( !mbInPaint )
        ImplInvertFocus( *(ImplGetWinData()->mpFocusRect) );
    mbFocusVisible = FALSE;
}

// -----------------------------------------------------------------------

void Window::Invert( const Rectangle& rRect, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() )
        return;

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;
    aRect.Justify();

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();

    if ( mbOutputClipped )
        return;

    SalInvert nSalFlags = 0;
    if ( nFlags & INVERT_HIGHLIGHT )
        nSalFlags |= SAL_INVERT_HIGHLIGHT;
    if ( nFlags & INVERT_50 )
        nSalFlags |= SAL_INVERT_50;
    mpGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), nSalFlags );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
        pGraphics->Invert( aRect, nFlags );
#endif
}

// -----------------------------------------------------------------------

void Window::Invert( const Polygon& rPoly, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() )
        return;

    USHORT nPoints = rPoly.GetSize();

    if ( nPoints < 2 )
        return;

    Polygon aPoly( ImplLogicToDevicePixel( rPoly ) );

#ifndef REMOTE_APPSERVER
    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !ImplGetGraphics() )
            return;
    }

    if ( mbInitClipRegion )
        ImplInitClipRegion();

    if ( mbOutputClipped )
        return;

    SalInvert nSalFlags = 0;
    if ( nFlags & INVERT_HIGHLIGHT )
        nSalFlags |= SAL_INVERT_HIGHLIGHT;
    if ( nFlags & INVERT_50 )
        nSalFlags |= SAL_INVERT_50;
    const SalPoint* pPtAry = (const SalPoint*)aPoly.ImplGetConstPointAry();
    mpGraphics->Invert( nPoints, pPtAry, nSalFlags );
#else
    ImplServerGraphics* pGraphics = ImplGetServerGraphics();
    if ( pGraphics )
        pGraphics->Invert( aPoly, nFlags );
#endif
}

// -----------------------------------------------------------------------

void Window::ShowTracking( const Rectangle& rRect, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplWinData* pWinData = ImplGetWinData();

    if ( !mbInPaint || !(nFlags & SHOWTRACK_WINDOW) )
    {
        if ( mbTrackVisible )
        {
            if ( (*(pWinData->mpTrackRect)  == rRect) &&
                 (pWinData->mnTrackFlags    == nFlags) )
                return;

            InvertTracking( *(pWinData->mpTrackRect), pWinData->mnTrackFlags );
        }

        InvertTracking( rRect, nFlags );
    }

    if ( !pWinData->mpTrackRect )
        pWinData->mpTrackRect = new Rectangle( rRect );
    else
        *(pWinData->mpTrackRect) = rRect;
    pWinData->mnTrackFlags      = nFlags;
    mbTrackVisible              = TRUE;
}

// -----------------------------------------------------------------------

void Window::HideTracking()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mbTrackVisible )
    {
        ImplWinData* pWinData = ImplGetWinData();
        if ( !mbInPaint || !(pWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
            InvertTracking( *(pWinData->mpTrackRect), pWinData->mnTrackFlags );
        mbTrackVisible = FALSE;
    }
}

// -----------------------------------------------------------------------

void Window::InvertTracking( const Rectangle& rRect, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Rectangle aRect( ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;
    aRect.Justify();

#ifndef REMOTE_APPSERVER
    SalGraphics* pGraphics;

    if ( nFlags & SHOWTRACK_WINDOW )
    {
        if ( !IsDeviceOutputNecessary() )
            return;

        // we need a graphics
        if ( !mpGraphics )
        {
            if ( !ImplGetGraphics() )
                return;
        }

        if ( mbInitClipRegion )
            ImplInitClipRegion();

        if ( mbOutputClipped )
            return;

        pGraphics = mpGraphics;
    }
    else
    {
        pGraphics = ImplGetFrameGraphics();

        if ( nFlags & SHOWTRACK_CLIP )
        {
            Point aPoint( mnOutOffX, mnOutOffY );
            Region aRegion( Rectangle( aPoint,
                                       Size( mnOutWidth, mnOutHeight ) ) );
            ImplClipBoundaries( aRegion, FALSE, FALSE );
            ImplSelectClipRegion( pGraphics, aRegion );
        }
    }

    USHORT nStyle = nFlags & SHOWTRACK_STYLE;
    if ( nStyle == SHOWTRACK_OBJECT )
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), SAL_INVERT_TRACKFRAME );
    else if ( nStyle == SHOWTRACK_SPLIT )
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), SAL_INVERT_50 );
    else
    {
        long nBorder = 1;
        if ( nStyle == SHOWTRACK_BIG )
            nBorder = 3;
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), nBorder, SAL_INVERT_50 );
        pGraphics->Invert( aRect.Left(), aRect.Bottom()-nBorder+1, aRect.GetWidth(), nBorder, SAL_INVERT_50 );
        pGraphics->Invert( aRect.Left(), aRect.Top()+nBorder, nBorder, aRect.GetHeight()-(nBorder*2), SAL_INVERT_50 );
        pGraphics->Invert( aRect.Right()-nBorder+1, aRect.Top()+nBorder, nBorder, aRect.GetHeight()-(nBorder*2), SAL_INVERT_50 );
    }
#else
    ImplServerGraphics* pGraphics;
    if ( nFlags & SHOWTRACK_WINDOW )
    {
        if ( !IsDeviceOutputNecessary() )
            return;

        pGraphics = ImplGetServerGraphics();
    }
    else
    {
        pGraphics = ImplGetServerGraphics( TRUE );

        if ( nFlags & SHOWTRACK_CLIP )
        {
            Point aTmpPoint( mnOutOffX, mnOutOffY );
            Size aTmpSize( mnOutWidth, mnOutHeight );
            Rectangle aTmpRect( aTmpPoint, aTmpSize );
            Region aRegion( aTmpRect );
            ImplClipBoundaries( aRegion, FALSE, FALSE );
            pGraphics->SetClipRegion( aRegion );
        }
    }

    if ( pGraphics )
        pGraphics->InvertTracking( aRect, nFlags );
#endif
}

// -----------------------------------------------------------------------

void Window::InvertTracking( const Polygon& rPoly, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    USHORT nPoints = rPoly.GetSize();

    if ( nPoints < 2 )
        return;

    Polygon aPoly( ImplLogicToDevicePixel( rPoly ) );

#ifndef REMOTE_APPSERVER
    SalGraphics* pGraphics;

    if ( nFlags & SHOWTRACK_WINDOW )
    {
        if ( !IsDeviceOutputNecessary() )
            return;

        // we need a graphics
        if ( !mpGraphics )
        {
            if ( !ImplGetGraphics() )
                return;
        }

        if ( mbInitClipRegion )
            ImplInitClipRegion();

        if ( mbOutputClipped )
            return;

        pGraphics = mpGraphics;
    }
    else
    {
        pGraphics = ImplGetFrameGraphics();

        if ( nFlags & SHOWTRACK_CLIP )
        {
            Point aPoint( mnOutOffX, mnOutOffY );
            Region aRegion( Rectangle( aPoint,
                                       Size( mnOutWidth, mnOutHeight ) ) );
            ImplClipBoundaries( aRegion, FALSE, FALSE );
            ImplSelectClipRegion( pGraphics, aRegion );
        }
    }

    const SalPoint* pPtAry = (const SalPoint*)aPoly.ImplGetConstPointAry();
    pGraphics->Invert( nPoints, pPtAry, SAL_INVERT_TRACKFRAME );
#else
    ImplServerGraphics* pGraphics;
    if ( nFlags & SHOWTRACK_WINDOW )
    {
        if ( !IsDeviceOutputNecessary() )
            return;

        pGraphics = ImplGetServerGraphics();
    }
    else
    {
        pGraphics = ImplGetServerGraphics( TRUE );

        if ( nFlags & SHOWTRACK_CLIP )
        {
            Point aTmpPoint( mnOutOffX, mnOutOffY );
            Size aTmpSize( mnOutWidth, mnOutHeight );
            Rectangle aTmpRect( aTmpPoint, aTmpSize );
            Region aRegion( aTmpRect );
            ImplClipBoundaries( aRegion, FALSE, FALSE );
            pGraphics->SetClipRegion( aRegion );
        }
    }

    if ( pGraphics )
        pGraphics->InvertTracking( aPoly, nFlags );
#endif
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplTrackTimerHdl, Timer*, pTimer )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Bei Button-Repeat muessen wir den Timeout umsetzen
    if ( pSVData->maWinData.mnTrackFlags & STARTTRACK_BUTTONREPEAT )
        pTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonRepeat() );

    // Tracking-Event erzeugen
    Point           aMousePos( mpFrameData->mnLastMouseX, mpFrameData->mnLastMouseY );
    MouseEvent      aMEvt( ImplFrameToOutput( aMousePos ),
                           mpFrameData->mnClickCount, 0,
                           mpFrameData->mnMouseCode, mpFrameData->mnMouseCode );
    TrackingEvent   aTEvt( aMEvt, TRACKING_REPEAT );
    Tracking( aTEvt );

    return 0;
}

// -----------------------------------------------------------------------

void Window::StartTracking( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpTrackWin != this )
    {
        if ( pSVData->maWinData.mpTrackWin )
            pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    }

    if ( nFlags & (STARTTRACK_SCROLLREPEAT | STARTTRACK_BUTTONREPEAT) )
    {
        pSVData->maWinData.mpTrackTimer = new AutoTimer;

        if ( nFlags & STARTTRACK_SCROLLREPEAT )
            pSVData->maWinData.mpTrackTimer->SetTimeout( GetSettings().GetMouseSettings().GetScrollRepeat() );
        else
            pSVData->maWinData.mpTrackTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonStartRepeat() );
        pSVData->maWinData.mpTrackTimer->SetTimeoutHdl( LINK( this, Window, ImplTrackTimerHdl ) );
        pSVData->maWinData.mpTrackTimer->Start();
    }

    pSVData->maWinData.mpTrackWin   = this;
    pSVData->maWinData.mnTrackFlags = nFlags;
    CaptureMouse();
}

// -----------------------------------------------------------------------

void Window::EndTracking( USHORT nFlags )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpTrackWin == this )
    {
        // Hier wegen DbgChkThis geklammert, da Window im Handler zerstoert
        // werden kann
        {
        DBG_CHKTHIS( Window, ImplDbgCheckWindow );

        if ( pSVData->maWinData.mpTrackTimer )
        {
            delete pSVData->maWinData.mpTrackTimer;
            pSVData->maWinData.mpTrackTimer = NULL;
        }

        pSVData->maWinData.mpTrackWin    = NULL;
        pSVData->maWinData.mnTrackFlags  = 0;
        ReleaseMouse();
        }

        // EndTracking rufen, wenn es gerufen werden soll
        if ( !(nFlags & ENDTRACK_DONTCALLHDL) )
        {
            Point           aMousePos( mpFrameData->mnLastMouseX, mpFrameData->mnLastMouseY );
            MouseEvent      aMEvt( ImplFrameToOutput( aMousePos ),
                                   mpFrameData->mnClickCount, 0,
                                   mpFrameData->mnMouseCode, mpFrameData->mnMouseCode );
            TrackingEvent   aTEvt( aMEvt, nFlags | ENDTRACK_END );
            Tracking( aTEvt );
        }
    }
}

// -----------------------------------------------------------------------

BOOL Window::IsTracking() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return (ImplGetSVData()->maWinData.mpTrackWin == this);
}

// -----------------------------------------------------------------------

void Window::StartAutoScroll( USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpAutoScrollWin != this )
    {
        if ( pSVData->maWinData.mpAutoScrollWin )
            pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();
    }

    pSVData->maWinData.mpAutoScrollWin = this;
    pSVData->maWinData.mnAutoScrollFlags = nFlags;
    pSVData->maAppData.mpWheelWindow = new ImplWheelWindow( this );
}

// -----------------------------------------------------------------------

void Window::EndAutoScroll()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpAutoScrollWin == this )
    {
        pSVData->maWinData.mpAutoScrollWin = NULL;
        pSVData->maWinData.mnAutoScrollFlags = 0;
        delete pSVData->maAppData.mpWheelWindow;
        pSVData->maAppData.mpWheelWindow = NULL;
    }
}

// -----------------------------------------------------------------------

BOOL Window::IsAutoScroll() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return (ImplGetSVData()->maWinData.mpAutoScrollWin == this);
}

// -----------------------------------------------------------------------

void Window::SaveBackground( const Point& rPos, const Size& rSize,
                             const Point& rDestOff, VirtualDevice& rSaveDevice )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpPaintRegion )
    {
        Region      aClip( *mpPaintRegion );
        const Point aPixPos( LogicToPixel( rPos ) );

        aClip.Move( -mnOutOffX, -mnOutOffY );
        aClip.Intersect( Rectangle( aPixPos, LogicToPixel( rSize ) ) );

        if ( !aClip.IsEmpty() )
        {
            const Region    aOldClip( rSaveDevice.GetClipRegion() );
            const Point     aPixOffset( rSaveDevice.LogicToPixel( rDestOff ) );
            const Point     aPixTopLeft( aClip.GetBoundRect().TopLeft() );
            const BOOL      bMap = rSaveDevice.IsMapModeEnabled();

            // move clip region to have the same distance to DestOffset
            aClip.Move( aPixOffset.X() - aPixPos.X(), aPixOffset.Y() - aPixPos.Y() );

            // set pixel clip region
            rSaveDevice.EnableMapMode( FALSE );
            rSaveDevice.SetClipRegion( aClip );
            rSaveDevice.EnableMapMode( bMap );
            rSaveDevice.DrawOutDev( rDestOff, rSize, rPos, rSize, *this );
            rSaveDevice.SetClipRegion( aOldClip );
        }
    }
    else
        rSaveDevice.DrawOutDev( rDestOff, rSize, rPos, rSize, *this );
}

// -----------------------------------------------------------------------

ULONG Window::SaveFocus()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpFocusWin )
    {
        ImplFocusDelData* pDelData = new ImplFocusDelData;
        pSVData->maWinData.mpFocusWin->ImplAddDel( pDelData );
        pDelData->mpFocusWin = pSVData->maWinData.mpFocusWin;
        return (ULONG)(void*)pDelData;
    }
    else
        return 0;
}

// -----------------------------------------------------------------------

BOOL Window::EndSaveFocus( ULONG nSaveId, BOOL bRestore )
{
    if ( !nSaveId )
        return FALSE;
    else
    {
        BOOL                bOK = TRUE;
        ImplFocusDelData*   pDelData = (ImplFocusDelData*)(void*)nSaveId;
        if ( !pDelData->IsDelete() )
        {
            pDelData->mpFocusWin->ImplRemoveDel( pDelData );
            if ( bRestore )
                pDelData->mpFocusWin->GrabFocus();
        }
        else
            bOK = !bRestore;
        delete pDelData;
        return bOK;
    }
}

// -----------------------------------------------------------------------

void Window::SetZoom( const Fraction& rZoom )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( maZoom != rZoom )
    {
        maZoom = rZoom;
        StateChanged( STATE_CHANGE_ZOOM );
    }
}

// -----------------------------------------------------------------------

inline long WinFloatRound( double fVal )
{
    return( fVal > 0.0 ? (long) ( fVal + 0.5 ) : -(long) ( -fVal + 0.5 ) );
}

// -----------------------------------------------------------------------

void Window::SetZoomedPointFont( const Font& rFont )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    const Fraction& rZoom = GetZoom();
    if ( rZoom.GetNumerator() != rZoom.GetDenominator() )
    {
        Font aFont( rFont );
        Size aSize = aFont.GetSize();
        double n = (double)aSize.Width();
        n *= (double)rZoom.GetNumerator();
        n /= (double)rZoom.GetDenominator();
        aSize.Width() = WinFloatRound( n );
        n = (double)aSize.Height();
        n *= (double)rZoom.GetNumerator();
        n /= (double)rZoom.GetDenominator();
        aSize.Height() = WinFloatRound( n );
        aFont.SetSize( aSize );
        SetPointFont( aFont );

        // Wenn Darstellung skaliert wird, nehmen wir gegebenenfalls
        // einen anderen Font, wenn der aktuelle nicht skalierbar ist
        FontMetric aMetric = GetFontMetric();
        long       nFontDiff = Abs( GetFont().GetSize().Height()-aMetric.GetSize().Height() );
        if ( (aMetric.GetType() == TYPE_RASTER) && (nFontDiff >= 2) )
        {
            USHORT nType;
            if ( aMetric.GetPitch() == PITCH_FIXED )
                nType = FONT_DEFAULT_FIXED;
            else
                nType = FONT_DEFAULT_UI_SANS;
            Font aTempFont = GetDefaultFont( nType, GetSettings().GetInternational().GetLanguage(), FALSE );
            aFont.SetName( aTempFont.GetName() );
            SetPointFont( aFont );
        }
    }
    else
        SetPointFont( rFont );
}

// -----------------------------------------------------------------------

long Window::CalcZoom( long nCalc ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    const Fraction& rZoom = GetZoom();
    if ( rZoom.GetNumerator() != rZoom.GetDenominator() )
    {
        double n = (double)nCalc;
        n *= (double)rZoom.GetNumerator();
        n /= (double)rZoom.GetDenominator();
        nCalc = WinFloatRound( n );
    }
    return nCalc;
}

// -----------------------------------------------------------------------

void Window::SetControlFont()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpControlFont )
    {
        delete mpControlFont;
        mpControlFont = NULL;
        StateChanged( STATE_CHANGE_CONTROLFONT );
    }
}

// -----------------------------------------------------------------------

void Window::SetControlFont( const Font& rFont )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( rFont == Font() )
    {
        SetControlFont();
        return;
    }

    if ( mpControlFont )
    {
        if ( *mpControlFont == rFont )
            return;
        *mpControlFont = rFont;
    }
    else
        mpControlFont = new Font( rFont );

    StateChanged( STATE_CHANGE_CONTROLFONT );
}

// -----------------------------------------------------------------------

Font Window::GetControlFont() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpControlFont )
        return *mpControlFont;
    else
    {
        Font aFont;
        return aFont;
    }
}

// -----------------------------------------------------------------------

void Window::SetControlForeground()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mbControlForeground )
    {
        maControlForeground = Color( COL_TRANSPARENT );
        mbControlForeground = FALSE;
        StateChanged( STATE_CHANGE_CONTROLFOREGROUND );
    }
}

// -----------------------------------------------------------------------

void Window::SetControlForeground( const Color& rColor )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( rColor.GetTransparency() )
    {
        if ( mbControlForeground )
        {
            maControlForeground = Color( COL_TRANSPARENT );
            mbControlForeground = FALSE;
            StateChanged( STATE_CHANGE_CONTROLFOREGROUND );
        }
    }
    else
    {
        if ( maControlForeground != rColor )
        {
            maControlForeground = rColor;
            mbControlForeground = TRUE;
            StateChanged( STATE_CHANGE_CONTROLFOREGROUND );
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetControlBackground()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mbControlBackground )
    {
        maControlBackground = Color( COL_TRANSPARENT );
        mbControlBackground = FALSE;
        StateChanged( STATE_CHANGE_CONTROLBACKGROUND );
    }
}

// -----------------------------------------------------------------------

void Window::SetControlBackground( const Color& rColor )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( rColor.GetTransparency() )
    {
        if ( mbControlBackground )
        {
            maControlBackground = Color( COL_TRANSPARENT );
            mbControlBackground = FALSE;
            StateChanged( STATE_CHANGE_CONTROLBACKGROUND );
        }
    }
    else
    {
        if ( maControlBackground != rColor )
        {
            maControlBackground = rColor;
            mbControlBackground = TRUE;
            StateChanged( STATE_CHANGE_CONTROLBACKGROUND );
        }
    }
}

// -----------------------------------------------------------------------

Size Window::CalcWindowSize( const Size& rOutSz ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Size aSz = rOutSz;
    aSz.Width()  += mnLeftBorder+mnRightBorder;
    aSz.Height() += mnTopBorder+mnBottomBorder;
    return aSz;
}

// -----------------------------------------------------------------------

Size Window::CalcOutputSize( const Size& rWinSz ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Size aSz = rWinSz;
    aSz.Width()  -= mnLeftBorder+mnRightBorder;
    aSz.Height() -= mnTopBorder+mnBottomBorder;
    return aSz;
}

// -----------------------------------------------------------------------

Font Window::GetDrawPixelFont( OutputDevice* pDev ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Font    aFont = GetPointFont();
    Size    aFontSize = aFont.GetSize();
    MapMode aPtMapMode( MAP_POINT );
    aFontSize = pDev->LogicToPixel( aFontSize, aPtMapMode );
    aFont.SetSize( aFontSize );
    return aFont;
}

// -----------------------------------------------------------------------

long Window::GetDrawPixel( OutputDevice* pDev, long nPixels ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    long nP = nPixels;
    if ( pDev->GetOutDevType() != OUTDEV_WINDOW )
    {
        MapMode aMap( MAP_100TH_MM );
        Size aSz( nP, 0 );
        aSz = PixelToLogic( aSz, aMap );
        aSz = pDev->LogicToPixel( aSz, aMap );
        nP = aSz.Width();
    }
    return nP;
}

// -----------------------------------------------------------------------

BOOL Window::HandleScrollCommand( const CommandEvent& rCmd,
                                  ScrollBar* pHScrl, ScrollBar* pVScrl )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    BOOL bRet = FALSE;

    if ( pHScrl || pVScrl )
    {
        switch( rCmd.GetCommand() )
        {
            case COMMAND_STARTAUTOSCROLL:
            {
                USHORT nFlags = 0;
                if ( pHScrl )
                {
                    if ( (pHScrl->GetVisibleSize() < pHScrl->GetRangeMax()) &&
                         pHScrl->IsEnabled() && pHScrl->IsInputEnabled() )
                        nFlags |= AUTOSCROLL_HORZ;
                }
                if ( pVScrl )
                {
                    if ( (pVScrl->GetVisibleSize() < pVScrl->GetRangeMax()) &&
                         pVScrl->IsEnabled() && pVScrl->IsInputEnabled() )
                        nFlags |= AUTOSCROLL_VERT;
                }

                if ( nFlags )
                {
                    StartAutoScroll( nFlags );
                    bRet = TRUE;
                }
            }
            break;

            case COMMAND_WHEEL:
            {
                const CommandWheelData* pData = rCmd.GetWheelData();

                if ( pData && (COMMAND_WHEEL_SCROLL == pData->GetMode()) && !pData->IsHorz() )
                {
                    ULONG nScrollLines = pData->GetScrollLines();
                    long nLines;
                    if ( nScrollLines == COMMAND_WHEEL_PAGESCROLL )
                    {
                        if ( pData->GetDelta() < 0 )
                            nLines = -LONG_MAX;
                        else
                            nLines = LONG_MAX;
                    }
                    else
                        nLines = pData->GetNotchDelta() * (long)nScrollLines;
                    if ( nLines )
                    {
                        ImplHandleScroll( NULL, 0L, pVScrl, nLines );
                        bRet = TRUE;
                    }
                }
            }
            break;

            case COMMAND_AUTOSCROLL:
            {
                const CommandScrollData* pData = rCmd.GetAutoScrollData();
                if ( pData && (pData->GetDeltaX() || pData->GetDeltaY()) )
                {
                    ImplHandleScroll( pHScrl, pData->GetDeltaX(),
                                      pVScrl, pData->GetDeltaY() );
                    bRet = TRUE;
                }
            }
            break;

            default:
            break;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

void Window::ImplHandleScroll( ScrollBar* pHScrl, long nX,
                               ScrollBar* pVScrl, long nY )
{
    if ( pHScrl && nX && pHScrl->IsEnabled() && pHScrl->IsInputEnabled() )
    {
        long nNewPos = pHScrl->GetThumbPos();

        if ( nX == -LONG_MAX )
            nNewPos += pHScrl->GetPageSize();
        else if ( nX == LONG_MAX )
            nNewPos -= pHScrl->GetPageSize();
        else
        {
            const double fVal = (double)nNewPos - ((double)nX * pHScrl->GetLineSize());

            if ( fVal < LONG_MIN )
                nNewPos = LONG_MIN;
            else if ( fVal > LONG_MAX )
                nNewPos = LONG_MAX;
            else
                nNewPos = (long)fVal;
        }

        pHScrl->DoScroll( nNewPos );
    }

    if ( pVScrl && nY && pVScrl->IsEnabled() && pVScrl->IsInputEnabled() )
    {
        long nNewPos = pVScrl->GetThumbPos();

        if ( nY == -LONG_MAX )
            nNewPos += pVScrl->GetPageSize();
        else if ( nY == LONG_MAX )
            nNewPos -= pVScrl->GetPageSize();
        else
        {
            const double fVal = (double)nNewPos - ((double)nY * pVScrl->GetLineSize());

            if ( fVal < LONG_MIN )
                nNewPos = LONG_MIN;
            else if ( fVal > LONG_MAX )
                nNewPos = LONG_MAX;
            else
                nNewPos = (long)fVal;
        }

        pVScrl->DoScroll( nNewPos );
    }
}

// -----------------------------------------------------------------------

void Window::GetAccessObject( AccessObjectRef& rAcc ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    rAcc = new AccessObject( (void*) this, ACCESS_TYPE_WND );
}
