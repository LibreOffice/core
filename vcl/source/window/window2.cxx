/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: window2.cxx,v $
 * $Revision: 1.28 $
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
#include "precompiled_vcl.hxx"

#include <limits.h>
#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif
#include <vcl/salbmp.hxx>
#include <vcl/salgdi.hxx>
#include <vcl/salframe.hxx>
#include <tools/debug.hxx>
#include <vcl/svdata.hxx>
#include <vcl/impbmp.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/event.hxx>
#include <vcl/timer.hxx>
#include <vcl/metric.hxx>
#include <vcl/outfont.hxx>
#include <vcl/outdev.h>
#include <tools/poly.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.h>
#include <vcl/window.hxx>
#include <vcl/scrbar.hxx>
#ifndef _SV_SCRWND_HXX
#include <scrwnd.hxx>
#endif
#include <vcl/dockwin.hxx>



// =======================================================================

DBG_NAMEEX( Window )

// =======================================================================

#define IMPL_MAXSAVEBACKSIZE    (640*480)
#define IMPL_MAXALLSAVEBACKSIZE (800*600*2)

//#define USE_NEW_RTL_IMPLEMENTATION

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
        if ( pTempWindow->mpWindowImpl->mbFrame )
            break;
        pTempWindow = pTempWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( pTempWindow );
    pTempWindow = pThisWindow;
    do
    {
        if ( pTempWindow == pTestWindow )
            return FALSE;
        if ( pTempWindow->mpWindowImpl->mbFrame )
            break;
        pTempWindow = pTempWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( pTempWindow );

    // Fenster auf gleiche Ebene bringen
    if ( pThisWindow->mpWindowImpl->mpOverlapWindow != pTestWindow->mpWindowImpl->mpOverlapWindow )
    {
        USHORT nThisLevel = 0;
        USHORT nTestLevel = 0;
        pTempWindow = pThisWindow;
        do
        {
            nThisLevel++;
            pTempWindow = pTempWindow->mpWindowImpl->mpOverlapWindow;
        }
        while ( !pTempWindow->mpWindowImpl->mbFrame );
        pTempWindow = pTestWindow;
        do
        {
            nTestLevel++;
            pTempWindow = pTempWindow->mpWindowImpl->mpOverlapWindow;
        }
        while ( !pTempWindow->mpWindowImpl->mbFrame );

        if ( nThisLevel < nTestLevel )
        {
            do
            {
                if ( pTestWindow->mpWindowImpl->mpOverlapWindow == pThisWindow->mpWindowImpl->mpOverlapWindow )
                    break;
                if ( pTestWindow->mpWindowImpl->mbFrame )
                    break;
                pTestWindow = pTestWindow->mpWindowImpl->mpOverlapWindow;
            }
            while ( pTestWindow );
        }
        else
        {
            do
            {
                if ( pThisWindow->mpWindowImpl->mpOverlapWindow == pTempWindow->mpWindowImpl->mpOverlapWindow )
                    break;
                if ( pThisWindow->mpWindowImpl->mbFrame )
                    break;
                pThisWindow = pThisWindow->mpWindowImpl->mpOverlapWindow;
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
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }
    while ( pTempWindow );

    return FALSE;
}

// =======================================================================

void Window::ImplSaveOverlapBackground()
{
    DBG_ASSERT( !mpWindowImpl->mpOverlapData->mpSaveBackDev, "Window::ImplSaveOverlapBackground() - Background already saved" );

    if ( !mpWindowImpl->mbFrame )
    {
        ULONG nSaveBackSize = mnOutWidth*mnOutHeight;
        if ( nSaveBackSize <= IMPL_MAXSAVEBACKSIZE )
        {
            if ( nSaveBackSize+mpWindowImpl->mpFrameData->mnAllSaveBackSize <= IMPL_MAXALLSAVEBACKSIZE )
            {
                Size aOutSize( mnOutWidth, mnOutHeight );
                mpWindowImpl->mpOverlapData->mpSaveBackDev = new VirtualDevice( *mpWindowImpl->mpFrameWindow );
                if ( mpWindowImpl->mpOverlapData->mpSaveBackDev->SetOutputSizePixel( aOutSize ) )
                {
                    mpWindowImpl->mpFrameWindow->ImplUpdateAll();

                    if ( mpWindowImpl->mbInitWinClipRegion )
                        ImplInitWinClipRegion();

                    mpWindowImpl->mpOverlapData->mnSaveBackSize = nSaveBackSize;
                    mpWindowImpl->mpFrameData->mnAllSaveBackSize += nSaveBackSize;
                    Point aDevPt;
                    mpWindowImpl->mpFrameWindow->ImplGetFrameDev( Point( mnOutOffX, mnOutOffY ),
                                                    aDevPt, aOutSize,
                                                    *(mpWindowImpl->mpOverlapData->mpSaveBackDev) );
                    mpWindowImpl->mpOverlapData->mpNextBackWin = mpWindowImpl->mpFrameData->mpFirstBackWin;
                    mpWindowImpl->mpFrameData->mpFirstBackWin = this;
                }
                else
                {
                    delete mpWindowImpl->mpOverlapData->mpSaveBackDev;
                    mpWindowImpl->mpOverlapData->mpSaveBackDev = NULL;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------

BOOL Window::ImplRestoreOverlapBackground( Region& rInvRegion )
{
    if ( mpWindowImpl->mpOverlapData->mpSaveBackDev )
    {
        if ( mpWindowImpl->mbInitWinClipRegion )
            ImplInitWinClipRegion();

        if ( mpWindowImpl->mpOverlapData->mpSaveBackDev )
        {
            Point   aDevPt;
            Point   aDestPt( mnOutOffX, mnOutOffY );
            Size    aDevSize = mpWindowImpl->mpOverlapData->mpSaveBackDev->GetOutputSizePixel();
            if ( mpWindowImpl->mpOverlapData->mpSaveBackRgn )
            {
                mpWindowImpl->mpOverlapData->mpSaveBackRgn->Intersect( mpWindowImpl->maWinClipRegion );
                rInvRegion = mpWindowImpl->maWinClipRegion;
                rInvRegion.Exclude( *mpWindowImpl->mpOverlapData->mpSaveBackRgn );
                mpWindowImpl->mpFrameWindow->ImplDrawFrameDev( aDestPt, aDevPt, aDevSize,
                                                 *(mpWindowImpl->mpOverlapData->mpSaveBackDev),
                                                 *mpWindowImpl->mpOverlapData->mpSaveBackRgn );
            }
            else
            {
                mpWindowImpl->mpFrameWindow->ImplDrawFrameDev( aDestPt, aDevPt, aDevSize,
                                                 *(mpWindowImpl->mpOverlapData->mpSaveBackDev),
                                                 mpWindowImpl->maWinClipRegion );
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
    if ( mpWindowImpl->mpOverlapData->mpSaveBackDev )
    {
        mpWindowImpl->mpFrameData->mnAllSaveBackSize -= mpWindowImpl->mpOverlapData->mnSaveBackSize;
        delete mpWindowImpl->mpOverlapData->mpSaveBackDev;
        mpWindowImpl->mpOverlapData->mpSaveBackDev = NULL;
        if ( mpWindowImpl->mpOverlapData->mpSaveBackRgn )
        {
            delete mpWindowImpl->mpOverlapData->mpSaveBackRgn;
            mpWindowImpl->mpOverlapData->mpSaveBackRgn = NULL;
        }

        // Fenster aus der Liste entfernen
        if ( mpWindowImpl->mpFrameData->mpFirstBackWin == this )
            mpWindowImpl->mpFrameData->mpFirstBackWin = mpWindowImpl->mpOverlapData->mpNextBackWin;
        else
        {
            Window* pTemp = mpWindowImpl->mpFrameData->mpFirstBackWin;
            while ( pTemp->mpWindowImpl->mpOverlapData->mpNextBackWin != this )
                pTemp = pTemp->mpWindowImpl->mpOverlapData->mpNextBackWin;
            pTemp->mpWindowImpl->mpOverlapData->mpNextBackWin = mpWindowImpl->mpOverlapData->mpNextBackWin;
        }
        mpWindowImpl->mpOverlapData->mpNextBackWin = NULL;
    }
}

// -----------------------------------------------------------------------

void Window::ImplInvalidateAllOverlapBackgrounds()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Window* pWindow = mpWindowImpl->mpFrameData->mpFirstBackWin;
    while ( pWindow )
    {
        // Naechstes Fenster schon hier merken, da dieses Fenster in
        // der if-Abfrage aus der Liste entfernt werden kann
        Window* pNext = pWindow->mpWindowImpl->mpOverlapData->mpNextBackWin;

        if ( ImplIsWindowInFront( pWindow ) )
        {
            Rectangle aRect1( Point( mnOutOffX, mnOutOffY ),
                              Size( mnOutWidth, mnOutHeight ) );
            Rectangle aRect2( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                              Size( pWindow->mnOutWidth, pWindow->mnOutHeight ) );
            aRect1.Intersection( aRect2 );
            if ( !aRect1.IsEmpty() )
            {
                if ( !pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn )
                    pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn = new Region( aRect2 );
                pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn->Exclude( aRect1 );
                if ( pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn->IsEmpty() )
                    pWindow->ImplDeleteOverlapBackground();
            }

        }

        pWindow = pNext;
    }
}

// =======================================================================

Bitmap Window::SnapShot( BOOL bBorder ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Bitmap aBmp;

    if ( IsReallyVisible() )
    {
        if ( bBorder && mpWindowImpl->mpBorderWindow )
            aBmp = mpWindowImpl->mpBorderWindow->SnapShot();
        else
        {
            ((Window*)this)->Update();

            if ( bBorder && mpWindowImpl->mbFrame )
            {
                SalBitmap* pSalBmp = mpWindowImpl->mpFrame->SnapShot();

                if ( pSalBmp )
                {
                    ImpBitmap* pImpBmp = new ImpBitmap;
                    pImpBmp->ImplSetSalBitmap( pSalBmp );
                    aBmp.ImplSetImpBitmap( pImpBmp );
                    return aBmp;
                }
            }

            mpWindowImpl->mpFrameWindow->ImplGetFrameBitmap( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ), aBmp );
        }
    }

    return aBmp;
}

// -----------------------------------------------------------------------

Bitmap Window::SnapShot() const
{
    // Should be merged in the next top level build !!!
    return SnapShot( TRUE );
}

// -----------------------------------------------------------------------

void Window::ShowFocus( const Rectangle& rRect )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpWindowImpl->mbInShowFocus )
        return;
    mpWindowImpl->mbInShowFocus = TRUE;

    ImplWinData* pWinData = ImplGetWinData();

    // native themeing suggest not to use focus rects
    if( ! ( mpWindowImpl->mbUseNativeFocus &&
            IsNativeWidgetEnabled() ) )
    {
        if ( !mpWindowImpl->mbInPaint )
        {
            if ( mpWindowImpl->mbFocusVisible )
            {
                if ( *(pWinData->mpFocusRect) == rRect )
                {
                    mpWindowImpl->mbInShowFocus = FALSE;
                    return;
                }

                ImplInvertFocus( *(pWinData->mpFocusRect) );
            }

            ImplInvertFocus( rRect );
        }
        if ( !pWinData->mpFocusRect )
            pWinData->mpFocusRect = new Rectangle( rRect );
        else
            *(pWinData->mpFocusRect) = rRect;
        mpWindowImpl->mbFocusVisible = TRUE;
    }
    else
    {
        if( ! mpWindowImpl->mbNativeFocusVisible )
        {
            mpWindowImpl->mbNativeFocusVisible = TRUE;
            if ( !mpWindowImpl->mbInPaint )
                Invalidate();
        }
    }
    mpWindowImpl->mbInShowFocus = FALSE;
}

// -----------------------------------------------------------------------

void Window::HideFocus()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpWindowImpl->mbInHideFocus )
        return;
    mpWindowImpl->mbInHideFocus = TRUE;

    // native themeing can suggest not to use focus rects
    if( ! ( mpWindowImpl->mbUseNativeFocus &&
            IsNativeWidgetEnabled() ) )
    {
        if ( !mpWindowImpl->mbFocusVisible )
        {
            mpWindowImpl->mbInHideFocus = FALSE;
            return;
        }

        if ( !mpWindowImpl->mbInPaint )
            ImplInvertFocus( *(ImplGetWinData()->mpFocusRect) );
        mpWindowImpl->mbFocusVisible = FALSE;
    }
    else
    {
        if( mpWindowImpl->mbNativeFocusVisible )
        {
            mpWindowImpl->mbNativeFocusVisible = FALSE;
            if ( !mpWindowImpl->mbInPaint )
                Invalidate();
        }
    }
    mpWindowImpl->mbInHideFocus = FALSE;
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
    mpGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), nSalFlags, this );
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
    const SalPoint* pPtAry = (const SalPoint*)aPoly.GetConstPointAry();
    mpGraphics->Invert( nPoints, pPtAry, nSalFlags, this );
}

// -----------------------------------------------------------------------

void Window::ShowTracking( const Rectangle& rRect, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    ImplWinData* pWinData = ImplGetWinData();

    if ( !mpWindowImpl->mbInPaint || !(nFlags & SHOWTRACK_WINDOW) )
    {
        if ( mpWindowImpl->mbTrackVisible )
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
    mpWindowImpl->mbTrackVisible              = TRUE;
}

// -----------------------------------------------------------------------

void Window::HideTracking()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mbTrackVisible )
    {
        ImplWinData* pWinData = ImplGetWinData();
        if ( !mpWindowImpl->mbInPaint || !(pWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
            InvertTracking( *(pWinData->mpTrackRect), pWinData->mnTrackFlags );
        mpWindowImpl->mbTrackVisible = FALSE;
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
            ImplSelectClipRegion( pGraphics, aRegion, this );
        }
    }

    USHORT nStyle = nFlags & SHOWTRACK_STYLE;
    if ( nStyle == SHOWTRACK_OBJECT )
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), SAL_INVERT_TRACKFRAME, this );
    else if ( nStyle == SHOWTRACK_SPLIT )
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(), SAL_INVERT_50, this );
    else
    {
        long nBorder = 1;
        if ( nStyle == SHOWTRACK_BIG )
            nBorder = 5;
        pGraphics->Invert( aRect.Left(), aRect.Top(), aRect.GetWidth(), nBorder, SAL_INVERT_50, this );
        pGraphics->Invert( aRect.Left(), aRect.Bottom()-nBorder+1, aRect.GetWidth(), nBorder, SAL_INVERT_50, this );
        pGraphics->Invert( aRect.Left(), aRect.Top()+nBorder, nBorder, aRect.GetHeight()-(nBorder*2), SAL_INVERT_50, this );
        pGraphics->Invert( aRect.Right()-nBorder+1, aRect.Top()+nBorder, nBorder, aRect.GetHeight()-(nBorder*2), SAL_INVERT_50, this );
    }
}

// -----------------------------------------------------------------------

void Window::InvertTracking( const Polygon& rPoly, USHORT nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    USHORT nPoints = rPoly.GetSize();

    if ( nPoints < 2 )
        return;

    Polygon aPoly( ImplLogicToDevicePixel( rPoly ) );

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
            ImplSelectClipRegion( pGraphics, aRegion, this );
        }
    }

    const SalPoint* pPtAry = (const SalPoint*)aPoly.GetConstPointAry();
    pGraphics->Invert( nPoints, pPtAry, SAL_INVERT_TRACKFRAME, this );
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplTrackTimerHdl, Timer*, pTimer )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Bei Button-Repeat muessen wir den Timeout umsetzen
    if ( pSVData->maWinData.mnTrackFlags & STARTTRACK_BUTTONREPEAT )
        pTimer->SetTimeout( GetSettings().GetMouseSettings().GetButtonRepeat() );

    // Tracking-Event erzeugen
    Point           aMousePos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
    if( ImplHasMirroredGraphics() && !IsRTLEnabled() )
    {
        // - RTL - re-mirror frame pos at pChild
#ifdef USE_NEW_RTL_IMPLEMENTATION
        Window *pRefWindow = (Window*) mpDummy4;
        pRefWindow->ImplReMirror( aMousePos );
#else
        ImplReMirror( aMousePos );
#endif
    }
    MouseEvent      aMEvt( ImplFrameToOutput( aMousePos ),
                           mpWindowImpl->mpFrameData->mnClickCount, 0,
                           mpWindowImpl->mpFrameData->mnMouseCode, mpWindowImpl->mpFrameData->mnMouseCode );
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
            Point           aMousePos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
            if( ImplHasMirroredGraphics() && !IsRTLEnabled() )
            {
                // - RTL - re-mirror frame pos at pChild
#ifdef USE_NEW_RTL_IMPLEMENTATION
                Window *pRefWindow = (Window*) mpDummy4;
                pRefWindow->ImplReMirror( aMousePos );
#else
                ImplReMirror( aMousePos );
#endif
            }

            MouseEvent      aMEvt( ImplFrameToOutput( aMousePos ),
                                   mpWindowImpl->mpFrameData->mnClickCount, 0,
                                   mpWindowImpl->mpFrameData->mnMouseCode, mpWindowImpl->mpFrameData->mnMouseCode );
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
        pSVData->maAppData.mpWheelWindow->ImplStop();
        pSVData->maAppData.mpWheelWindow->doLazyDelete();
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

    if ( mpWindowImpl->mpPaintRegion )
    {
        Region      aClip( *mpWindowImpl->mpPaintRegion );
        const Point aPixPos( LogicToPixel( rPos ) );

        aClip.Move( -mnOutOffX, -mnOutOffY );
        aClip.Intersect( Rectangle( aPixPos, LogicToPixel( rSize ) ) );

        if ( !aClip.IsEmpty() )
        {
            const Region    aOldClip( rSaveDevice.GetClipRegion() );
            const Point     aPixOffset( rSaveDevice.LogicToPixel( rDestOff ) );
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

sal_uIntPtr Window::SaveFocus()
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maWinData.mpFocusWin )
    {
        ImplFocusDelData* pDelData = new ImplFocusDelData;
        pSVData->maWinData.mpFocusWin->ImplAddDel( pDelData );
        pDelData->mpFocusWin = pSVData->maWinData.mpFocusWin;
        return (sal_uIntPtr)(void*)pDelData;
    }
    else
        return 0;
}

// -----------------------------------------------------------------------

BOOL Window::EndSaveFocus( sal_uIntPtr nSaveId, BOOL bRestore )
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

    if ( mpWindowImpl->maZoom != rZoom )
    {
        mpWindowImpl->maZoom = rZoom;
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
                nType = DEFAULTFONT_FIXED;
            else
                nType = DEFAULTFONT_UI_SANS;
            Font aTempFont = GetDefaultFont( nType, GetSettings().GetLanguage(), 0 );
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

    if ( mpWindowImpl->mpControlFont )
    {
        delete mpWindowImpl->mpControlFont;
        mpWindowImpl->mpControlFont = NULL;
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

    if ( mpWindowImpl->mpControlFont )
    {
        if ( *mpWindowImpl->mpControlFont == rFont )
            return;
        *mpWindowImpl->mpControlFont = rFont;
    }
    else
        mpWindowImpl->mpControlFont = new Font( rFont );

    StateChanged( STATE_CHANGE_CONTROLFONT );
}

// -----------------------------------------------------------------------

Font Window::GetControlFont() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mpControlFont )
        return *mpWindowImpl->mpControlFont;
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

    if ( mpWindowImpl->mbControlForeground )
    {
        mpWindowImpl->maControlForeground = Color( COL_TRANSPARENT );
        mpWindowImpl->mbControlForeground = FALSE;
        StateChanged( STATE_CHANGE_CONTROLFOREGROUND );
    }
}

// -----------------------------------------------------------------------

void Window::SetControlForeground( const Color& rColor )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( rColor.GetTransparency() )
    {
        if ( mpWindowImpl->mbControlForeground )
        {
            mpWindowImpl->maControlForeground = Color( COL_TRANSPARENT );
            mpWindowImpl->mbControlForeground = FALSE;
            StateChanged( STATE_CHANGE_CONTROLFOREGROUND );
        }
    }
    else
    {
        if ( mpWindowImpl->maControlForeground != rColor )
        {
            mpWindowImpl->maControlForeground = rColor;
            mpWindowImpl->mbControlForeground = TRUE;
            StateChanged( STATE_CHANGE_CONTROLFOREGROUND );
        }
    }
}

// -----------------------------------------------------------------------

void Window::SetControlBackground()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( mpWindowImpl->mbControlBackground )
    {
        mpWindowImpl->maControlBackground = Color( COL_TRANSPARENT );
        mpWindowImpl->mbControlBackground = FALSE;
        StateChanged( STATE_CHANGE_CONTROLBACKGROUND );
    }
}

// -----------------------------------------------------------------------

void Window::SetControlBackground( const Color& rColor )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( rColor.GetTransparency() )
    {
        if ( mpWindowImpl->mbControlBackground )
        {
            mpWindowImpl->maControlBackground = Color( COL_TRANSPARENT );
            mpWindowImpl->mbControlBackground = FALSE;
            StateChanged( STATE_CHANGE_CONTROLBACKGROUND );
        }
    }
    else
    {
        if ( mpWindowImpl->maControlBackground != rColor )
        {
            mpWindowImpl->maControlBackground = rColor;
            mpWindowImpl->mbControlBackground = TRUE;
            StateChanged( STATE_CHANGE_CONTROLBACKGROUND );
        }
    }
}

// -----------------------------------------------------------------------

Size Window::CalcWindowSize( const Size& rOutSz ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Size aSz = rOutSz;
    aSz.Width()  += mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder;
    aSz.Height() += mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder;
    return aSz;
}

// -----------------------------------------------------------------------

Size Window::CalcOutputSize( const Size& rWinSz ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    Size aSz = rWinSz;
    aSz.Width()  -= mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder;
    aSz.Height() -= mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder;
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
                         pHScrl->IsEnabled() && pHScrl->IsInputEnabled() && ! pHScrl->IsInModalMode() )
                        nFlags |= AUTOSCROLL_HORZ;
                }
                if ( pVScrl )
                {
                    if ( (pVScrl->GetVisibleSize() < pVScrl->GetRangeMax()) &&
                         pVScrl->IsEnabled() && pVScrl->IsInputEnabled() && ! pVScrl->IsInModalMode() )
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

                if ( pData && (COMMAND_WHEEL_SCROLL == pData->GetMode()) )
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
                        ImplHandleScroll( NULL,
                                          0L,
                                          pData->IsHorz() ? pHScrl : pVScrl,
                                          nLines );
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
    if ( pHScrl && nX && pHScrl->IsEnabled() && pHScrl->IsInputEnabled() && ! pHScrl->IsInModalMode() )
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

    if ( pVScrl && nY && pVScrl->IsEnabled() && pVScrl->IsInputEnabled() && ! pVScrl->IsInModalMode() )
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

// support for docking
// this is currently handled in ImplDockingWindowWrapper
/*
void Window::ImplSetFloatingMode( BOOL bFloatMode )
{
    // if the window is docked, put it into a flaoting window
    // if it is floating put it back in the old frame

    ImplDockingWindowWrapper *pWrapper = pDockingMgr->GetDockingWindowWrapper( this );
    if( !pDockingData )
        return;

    if ( pWrapper->IsFloatingMode() != bFloatMode )
    {
        if ( pWrapper->PrepareToggleFloatingMode() )
        {
            BOOL bVisible = IsVisible();

            if ( bFloatMode )
            {
                Show( FALSE, SHOW_NOFOCUSCHANGE );

                pWrapper->maDockPos = GetPosPixel();

                Window* pRealParent = mpWindowImpl->mpRealParent;
                pWrapper->mpOldBorderWin = mpWindowImpl->mpBorderWindow;

                ImplDockFloatWin* pWin =
                    new ImplDockFloatWin2(
                                         mpWindowImpl->mpParent,
                                         mnFloatBits & ( WB_MOVEABLE | WB_SIZEABLE | WB_CLOSEABLE ) ?  mnFloatBits | WB_SYSTEMWINDOW : mnFloatBits,
                                         pWrapper );
                pWrapper->mpFloatWin = pWin;
                mpWindowImpl->mpBorderWindow  = NULL;
                mpWindowImpl->mnLeftBorder    = 0;
                mpWindowImpl->mnTopBorder     = 0;
                mpWindowImpl->mnRightBorder   = 0;
                mpWindowImpl->mnBottomBorder  = 0;
                // Falls Parent zerstoert wird, muessen wir auch vom
                // BorderWindow den Parent umsetzen
                if ( pWrapper->mpOldBorderWin )
                    pWrapper->mpOldBorderWin->SetParent( pWin );
                SetParent( pWin );
                pWin->SetPosPixel( Point() );
                mpWindowImpl->mpBorderWindow = pWin;
                pWin->mpWindowImpl->mpClientWindow = this;
                mpWindowImpl->mpRealParent = pRealParent;
                pWin->SetText( GetText() );
                pWin->SetOutputSizePixel( GetSizePixel() );
                pWin->SetPosPixel( pWrapper->maFloatPos );
                // DockingDaten ans FloatingWindow weiterreichen
                pWin->ShowTitleButton( TITLE_BUTTON_DOCKING, pWrapper->mbDockBtn );
                pWin->ShowTitleButton( TITLE_BUTTON_HIDE, pWrapper->mbHideBtn );
                pWin->SetPin( pWrapper->mbPined );
                if ( pWrapper->mbRollUp )
                    pWin->RollUp();
                else
                    pWin->RollDown();
                pWin->SetRollUpOutputSizePixel( pWrapper->maRollUpOutSize );
                pWin->SetMinOutputSizePixel( pWrapper->maMinOutSize );

                pWrapper->ToggleFloatingMode();

                if ( bVisible )
                    Show();
            }
            else
            {
                Show( FALSE, SHOW_NOFOCUSCHANGE );

                // FloatingDaten wird im FloatingWindow speichern
                pWrapper->maFloatPos      = mpFloatWin->GetPosPixel();
                pWrapper->mbDockBtn       = mpFloatWin->IsTitleButtonVisible( TITLE_BUTTON_DOCKING );
                pWrapper->mbHideBtn       = mpFloatWin->IsTitleButtonVisible( TITLE_BUTTON_HIDE );
                pWrapper->mbPined         = mpFloatWin->IsPined();
                pWrapper->mbRollUp        = mpFloatWin->IsRollUp();
                pWrapper->maRollUpOutSize = mpFloatWin->GetRollUpOutputSizePixel();
                pWrapper->maMinOutSize    = mpFloatWin->GetMinOutputSizePixel();

                Window* pRealParent = mpWindowImpl->mpRealParent;
                mpWindowImpl->mpBorderWindow = NULL;
                if ( pWrapper->mpOldBorderWin )
                {
                    SetParent( pWrapper->mpOldBorderWin );
                    ((ImplBorderWindow*)pWrapper->mpOldBorderWin)->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
                    pWrapper->mpOldBorderWin->Resize();
                }
                mpWindowImpl->mpBorderWindow = pWrapper->mpOldBorderWin;
                SetParent( pRealParent );
                mpWindowImpl->mpRealParent = pRealParent;
                delete static_cast<ImplDockFloatWin*>(mpFloatWin);
                pWrapper->mpFloatWin = NULL;
                SetPosPixel( maDockPos );

                pWrapper->ToggleFloatingMode();

                if ( bVisible )
                    Show();
            }
        }
    }
}
*/

DockingManager* Window::GetDockingManager()
{
    return ImplGetDockingManager();
}

void Window::EnableDocking( BOOL bEnable )
{
    // update list of dockable windows
    if( bEnable )
        ImplGetDockingManager()->AddWindow( this );
    else
        ImplGetDockingManager()->RemoveWindow( this );
}


// retrieves the list of owner draw decorated windows for this window hiearchy
::std::vector<Window *>& Window::ImplGetOwnerDrawList()
{
    return ImplGetTopmostFrameWindow()->mpWindowImpl->mpFrameData->maOwnerDrawList;
}

Window* Window::ImplGetTopmostFrameWindow()
{
    Window *pTopmostParent = this;
    while( pTopmostParent->ImplGetParent() )
        pTopmostParent = pTopmostParent->ImplGetParent();
    return pTopmostParent->mpWindowImpl->mpFrameWindow;
}

// making these Methods out of line to be able to change them lateron without complete rebuild
// TODO: Set the SmartId in here and remove mpWindowImpl->mnHelpId
void Window::SetHelpId( ULONG nHelpId ) { mpWindowImpl->mnHelpId = nHelpId; }
ULONG Window::GetHelpId() const { return mpWindowImpl->mnHelpId; }

void Window::SetSmartHelpId( const SmartId& aId, SmartIdUpdateMode aMode )
{
    // create SmartId if required
    if ( (aMode == SMART_SET_STR) || (aMode == SMART_SET_ALL) || ( (aMode == SMART_SET_SMART) && aId.HasString() ) )
    {
        if ( !ImplGetWinData()->mpSmartHelpId )
            ImplGetWinData()->mpSmartHelpId = new SmartId();
    }

    // if we have a SmartId (eather from earlier call or just created) fill with new values
    if ( mpWindowImpl->mpWinData && mpWindowImpl->mpWinData->mpSmartHelpId )
        ImplGetWinData()->mpSmartHelpId->UpdateId( aId, aMode );

    if ( (aMode == SMART_SET_NUM) || (aMode == SMART_SET_ALL) || ( (aMode == SMART_SET_SMART) && aId.HasNumeric() ) )
        mpWindowImpl->mnHelpId = aId.GetNum();
}

SmartId Window::GetSmartHelpId() const
{
    if ( mpWindowImpl->mpWinData && mpWindowImpl->mpWinData->mpSmartHelpId )
    {
        if ( mpWindowImpl->mnHelpId || mpWindowImpl->mpWinData->mpSmartHelpId->HasNumeric() )
            mpWindowImpl->mpWinData->mpSmartHelpId->UpdateId( SmartId( mpWindowImpl->mnHelpId ), SMART_SET_NUM );
        return *mpWindowImpl->mpWinData->mpSmartHelpId;
    }
    else
    {
        if ( mpWindowImpl->mnHelpId )
            return SmartId( mpWindowImpl->mnHelpId );
        else
            return SmartId();
    }
}


// making these Methods out of line to be able to change them lateron without complete rebuild
// TODO: Set the SmartId in here and remove mpWindowImpl->mnUniqId
void Window::SetUniqueId( ULONG nUniqueId ) { mpWindowImpl->mnUniqId = nUniqueId; }
ULONG Window::GetUniqueId() const { return mpWindowImpl->mnUniqId; }


void Window::SetSmartUniqueId( const SmartId& aId, SmartIdUpdateMode aMode )
{
    // create SmartId if required
    if ( (aMode == SMART_SET_STR) || (aMode == SMART_SET_ALL) || ( (aMode == SMART_SET_SMART) && aId.HasString() ) )
    {
        if ( !ImplGetWinData()->mpSmartUniqueId )
            ImplGetWinData()->mpSmartUniqueId = new SmartId();
    }

    // if we have a SmartId (eather from earlier call or just created) fill with new values
    if ( mpWindowImpl->mpWinData && mpWindowImpl->mpWinData->mpSmartUniqueId )
        ImplGetWinData()->mpSmartUniqueId->UpdateId( aId, aMode );

    if ( (aMode == SMART_SET_NUM) || (aMode == SMART_SET_ALL) || ( (aMode == SMART_SET_SMART) && aId.HasNumeric() ) )
        mpWindowImpl->mnUniqId = aId.GetNum();
}

SmartId Window::GetSmartUniqueId() const
{
    if ( mpWindowImpl->mpWinData && mpWindowImpl->mpWinData->mpSmartUniqueId )
    {
        if ( mpWindowImpl->mnUniqId || mpWindowImpl->mpWinData->mpSmartUniqueId->HasNumeric() )
            mpWindowImpl->mpWinData->mpSmartUniqueId->UpdateId( SmartId( mpWindowImpl->mnUniqId ), SMART_SET_NUM );
        return *mpWindowImpl->mpWinData->mpSmartUniqueId;
    }
    else
    {
        if ( mpWindowImpl->mnUniqId )
            return SmartId( mpWindowImpl->mnUniqId );
        else
            return SmartId();
    }
}

SmartId Window::GetSmartUniqueOrHelpId() const
{
    if ( ( mpWindowImpl->mpWinData && mpWindowImpl->mpWinData->mpSmartHelpId ) || mpWindowImpl->mnHelpId )
    {
        if ( ( mpWindowImpl->mpWinData && mpWindowImpl->mpWinData->mpSmartUniqueId ) || mpWindowImpl->mnUniqId )
        {
            SmartId aTemp = GetSmartHelpId();
            aTemp.UpdateId( GetSmartUniqueId() );
            return aTemp;
        }
        else
            return GetSmartHelpId();
    }
    else
        return GetSmartUniqueId();
}




// --------- old inline methods ---------------

Window* Window::ImplGetWindow()
{
    if ( mpWindowImpl->mpClientWindow )
        return mpWindowImpl->mpClientWindow;
    else
        return this;
}

ImplFrameData* Window::ImplGetFrameData()
{
    return mpWindowImpl->mpFrameData;
}

SalFrame* Window::ImplGetFrame() const
{
    return mpWindowImpl->mpFrame;
}

Window* Window::ImplGetParent() const
{
    return mpWindowImpl->mpParent;
}

Window* Window::ImplGetClientWindow() const
{
    return mpWindowImpl->mpClientWindow;
}

Window* Window::ImplGetBorderWindow() const
{
    return mpWindowImpl->mpBorderWindow;
}

Window* Window::ImplGetFirstOverlapWindow()
{
    if ( mpWindowImpl->mbOverlapWin )
        return this;
    else
        return mpWindowImpl->mpOverlapWindow;
}

const Window* Window::ImplGetFirstOverlapWindow() const
{
    if ( mpWindowImpl->mbOverlapWin )
        return this;
    else
        return mpWindowImpl->mpOverlapWindow;
}

Window* Window::ImplGetFrameWindow() const
{
    return mpWindowImpl->mpFrameWindow;
}

BOOL Window::ImplIsDockingWindow() const
{
    return mpWindowImpl->mbDockWin;
}

BOOL Window::ImplIsFloatingWindow() const
{
    return mpWindowImpl->mbFloatWin;
}

BOOL Window::ImplIsToolbox() const
{
    return mpWindowImpl->mbToolBox;
}

BOOL Window::ImplIsSplitter() const
{
    return mpWindowImpl->mbSplitter;
}

BOOL Window::ImplIsPushButton() const
{
    return mpWindowImpl->mbPushButton;
}

BOOL Window::ImplIsOverlapWindow() const
{
    return mpWindowImpl->mbOverlapWin;
}

void Window::ImplSetActive( BOOL bActive )
{
    mpWindowImpl->mbActive = bActive;
}

BOOL Window::ImplIsMouseTransparent() const
{
    return mpWindowImpl->mbMouseTransparent;
}

void Window::ImplSetMouseTransparent( BOOL bTransparent )
{
    mpWindowImpl->mbMouseTransparent = bTransparent;
}

Point Window::ImplOutputToFrame( const Point& rPos )
{
    return Point( rPos.X()+mnOutOffX, rPos.Y()+mnOutOffY );
}

Point Window::ImplFrameToOutput( const Point& rPos )
{
    return Point( rPos.X()-mnOutOffX, rPos.Y()-mnOutOffY );
}

void Window::ImplOutputToFrame( Rectangle& rRect )
{
    rRect.Left()+=mnOutOffX;
    rRect.Top()+=mnOutOffY;
    rRect.Right()+=mnOutOffX;
    rRect.Bottom()+=mnOutOffY;
}

void Window::ImplFrameToOutput( Rectangle& rRect )
{
    rRect.Left()-=mnOutOffX;
    rRect.Top()-=mnOutOffY;
    rRect.Right()-=mnOutOffX;
    rRect.Bottom()-=mnOutOffY;
}

void Window::SetCompoundControl( BOOL bCompound )
{
    mpWindowImpl->mbCompoundControl = bCompound;
}

void Window::IncrementLockCount()
{
    mpWindowImpl->mnLockCount++;
}

void Window::DecrementLockCount()
{
    mpWindowImpl->mnLockCount--;
}

WinBits Window::GetStyle() const
{
    return mpWindowImpl->mnStyle;
}

WinBits Window::GetPrevStyle() const
{
    return mpWindowImpl->mnPrevStyle;
}

WinBits Window::GetExtendedStyle() const
{
    return mpWindowImpl->mnExtendedStyle;
}

WinBits Window::GetPrevExtendedStyle() const
{
    return mpWindowImpl->mnExtendedStyle;
}

void Window::SetType( WindowType nType )
{
    mpWindowImpl->mnType = nType;
}

WindowType Window::GetType() const
{
    return mpWindowImpl->mnType;
}
BOOL Window::IsSystemWindow() const
{
    return mpWindowImpl->mbSysWin;
}

BOOL Window::IsDialog() const
{
    return mpWindowImpl->mbDialog;
}

BOOL Window::IsMenuFloatingWindow() const
{
    return mpWindowImpl->mbMenuFloatingWindow;
}

BOOL Window::IsToolbarFloatingWindow() const
{
    return mpWindowImpl->mbToolbarFloatingWindow;
}

void Window::EnableAllResize( BOOL bEnable )
{
    mpWindowImpl->mbAllResize = bEnable;
}

BOOL Window::IsAllResizeEnabled() const
{
    return mpWindowImpl->mbAllResize;
}

BOOL Window::IsClipSiblingsEnabled() const
{
    return mpWindowImpl->mbClipSiblings;
}

void Window::EnableChildTransparentMode( BOOL bEnable )
{
    mpWindowImpl->mbChildTransparent = bEnable;
}

BOOL Window::IsChildTransparentModeEnabled() const
{
    return mpWindowImpl->mbChildTransparent;
}

BOOL Window::IsMouseTransparent() const
{
    return mpWindowImpl->mbMouseTransparent;
}

BOOL Window::IsPaintTransparent() const
{
    return mpWindowImpl->mbPaintTransparent;
}

void Window::SetDialogControlStart( BOOL bStart )
{
    mpWindowImpl->mbDlgCtrlStart = bStart;
}

BOOL Window::IsDialogControlStart() const
{
    return mpWindowImpl->mbDlgCtrlStart;
}

void Window::SetDialogControlFlags( USHORT nFlags )
{
    mpWindowImpl->mnDlgCtrlFlags = nFlags;
}

USHORT Window::GetDialogControlFlags() const
{
    return mpWindowImpl->mnDlgCtrlFlags;
}

const InputContext& Window::GetInputContext() const
{
    return mpWindowImpl->maInputContext;
}

BOOL Window::IsExtTextInput() const
{
    return mpWindowImpl->mbExtTextInput;
}

void Window::EnableChildNotify( BOOL bEnable )
{
    mpWindowImpl->mbChildNotify = bEnable;
}

BOOL Window::IsChildNotify() const
{
    return mpWindowImpl->mbChildNotify;
}

BOOL Window::IsControlFont() const
{
    return (mpWindowImpl->mpControlFont != 0);
}

Color Window::GetControlForeground() const
{
    return mpWindowImpl->maControlForeground;
}

BOOL Window::IsControlForeground() const
{
    return mpWindowImpl->mbControlForeground;
}

Color Window::GetControlBackground() const
{
    return mpWindowImpl->maControlBackground;
}

BOOL Window::IsControlBackground() const
{
    return mpWindowImpl->mbControlBackground;
}

BOOL Window::IsInPaint() const
{
    return mpWindowImpl->mbInPaint;
}

Window* Window::GetParent() const
{
    return mpWindowImpl->mpRealParent;
}

BOOL Window::IsVisible() const
{
    return mpWindowImpl->mbVisible;
}

BOOL Window::IsReallyVisible() const
{
    return mpWindowImpl->mbReallyVisible;
}

BOOL Window::IsParentPathVisible() const
{
    return mpWindowImpl->mbReallyVisible;
}

BOOL Window::IsReallyShown() const
{
    return mpWindowImpl->mbReallyShown;
}

BOOL Window::IsInInitShow() const
{
    return mpWindowImpl->mbInInitShow;
}

BOOL Window::IsEnabled() const
{
    return !mpWindowImpl->mbDisabled;
}

BOOL Window::IsInputEnabled() const
{
    return !mpWindowImpl->mbInputDisabled;
}

BOOL Window::IsAlwaysEnableInput() const
{
    return mpWindowImpl->meAlwaysInputMode == AlwaysInputEnabled;
}

BOOL Window::IsAlwaysDisableInput() const
{
    return mpWindowImpl->meAlwaysInputMode == AlwaysInputDisabled;
}

USHORT Window::GetActivateMode() const
{
    return mpWindowImpl->mnActivateMode;

}

BOOL Window::IsAlwaysOnTopEnabled() const
{
    return mpWindowImpl->mbAlwaysOnTop;
}

BOOL Window::IsDefaultPos() const
{
    return mpWindowImpl->mbDefPos;
}

BOOL Window::IsDefaultSize() const
{
    return mpWindowImpl->mbDefSize;
}

void Window::EnablePaint( BOOL bEnable )
{
    mpWindowImpl->mbPaintDisabled = !bEnable;
}

BOOL Window::IsPaintEnabled() const
{
    return !mpWindowImpl->mbPaintDisabled;
}

BOOL Window::IsUpdateMode() const
{
    return !mpWindowImpl->mbNoUpdate;
}

void Window::SetParentUpdateMode( BOOL bUpdate )
{
    mpWindowImpl->mbNoParentUpdate = !bUpdate;
}

BOOL Window::IsParentUpdateMode() const
{
    return !mpWindowImpl->mbNoParentUpdate;
}

BOOL Window::IsActive() const
{
    return mpWindowImpl->mbActive;
}

USHORT Window::GetGetFocusFlags() const
{
    return mpWindowImpl->mnGetFocusFlags;
}

BOOL Window::IsCompoundControl() const
{
    return mpWindowImpl->mbCompoundControl;
}

BOOL Window::HasCompoundControlFocus() const
{
    return mpWindowImpl->mbCompoundControlHasFocus;
}

BOOL Window::IsChildPointerOverwrite() const
{
    return mpWindowImpl->mbChildPtrOverwrite;
}

BOOL Window::IsPointerVisible() const
{
    return !mpWindowImpl->mbNoPtrVisible;
}

BOOL Window::IsWait() const
{
    return (mpWindowImpl->mnWaitCount != 0);
}

Cursor* Window::GetCursor() const
{
    return mpWindowImpl->mpCursor;
}

const Fraction& Window::GetZoom() const
{
    return mpWindowImpl->maZoom;
}

BOOL Window::IsZoom() const
{
    return mpWindowImpl->maZoom.GetNumerator() != mpWindowImpl->maZoom.GetDenominator();
}

void Window::SetHelpText( const XubString& rHelpText )
{
    mpWindowImpl->maHelpText = rHelpText;
}

void Window::SetQuickHelpText( const XubString& rHelpText )
{
    mpWindowImpl->maQuickHelpText = rHelpText;
}

const XubString& Window::GetQuickHelpText() const
{
    return mpWindowImpl->maQuickHelpText;
}

void Window::SetData( void* pNewData )
{
    mpWindowImpl->mpUserData = pNewData;
}

void* Window::GetData() const
{
    return mpWindowImpl->mpUserData;
}

BOOL Window::IsCreatedWithToolkit() const
{
    return mpWindowImpl->mbCreatedWithToolkit;
}

void Window::SetCreatedWithToolkit( BOOL b )
{
    mpWindowImpl->mbCreatedWithToolkit = b;

}
const Pointer& Window::GetPointer() const
{
    return mpWindowImpl->maPointer;
}

VCLXWindow* Window::GetWindowPeer() const
{
    return mpWindowImpl->mpVCLXWindow;
}

void Window::SetPosPixel( const Point& rNewPos )
{
    SetPosSizePixel( rNewPos.X(), rNewPos.Y(), 0, 0, WINDOW_POSSIZE_POS );
}

void Window::SetSizePixel( const Size& rNewSize )
{
    SetPosSizePixel( 0, 0, rNewSize.Width(), rNewSize.Height(),
                     WINDOW_POSSIZE_SIZE );
}

void Window::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    SetPosSizePixel( rNewPos.X(), rNewPos.Y(),
                     rNewSize.Width(), rNewSize.Height(),
                     WINDOW_POSSIZE_POSSIZE );
}

void Window::SetOutputSizePixel( const Size& rNewSize )
{
    SetSizePixel( Size( rNewSize.Width()+mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder,
                        rNewSize.Height()+mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder ) );
}

