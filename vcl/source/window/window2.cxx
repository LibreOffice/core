/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <limits.h>
#include <tools/debug.hxx>
#include <tools/poly.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/dialog.hxx>
#include <vcl/event.hxx>
#include <vcl/layout.hxx>
#include <vcl/timer.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/tabctrl.hxx>

#include <window.h>
#include <outfont.hxx>
#include <outdev.h>
#include <svdata.hxx>
#include <impbmp.hxx>
#include <salbmp.hxx>
#include <salgdi.hxx>
#include <salframe.hxx>
#include <scrwnd.hxx>

using namespace com::sun::star;

// =======================================================================

DBG_NAMEEX( Window )

// =======================================================================

#define IMPL_MAXSAVEBACKSIZE    (640*480)
#define IMPL_MAXALLSAVEBACKSIZE (800*600*2)

// =======================================================================

struct ImplFocusDelData : public ImplDelData
{
    Window*         mpFocusWin;
};

// =======================================================================

sal_Bool Window::ImplIsWindowInFront( const Window* pTestWindow ) const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );
    DBG_CHKOBJ( pTestWindow, Window, ImplDbgCheckWindow );

    // check for overlapping window
    pTestWindow = pTestWindow->ImplGetFirstOverlapWindow();
    const Window* pTempWindow = pTestWindow;
    const Window* pThisWindow = ImplGetFirstOverlapWindow();
    if ( pTempWindow == pThisWindow )
        return sal_False;
    do
    {
        if ( pTempWindow == pThisWindow )
            return sal_True;
        if ( pTempWindow->mpWindowImpl->mbFrame )
            break;
        pTempWindow = pTempWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( pTempWindow );
    pTempWindow = pThisWindow;
    do
    {
        if ( pTempWindow == pTestWindow )
            return sal_False;
        if ( pTempWindow->mpWindowImpl->mbFrame )
            break;
        pTempWindow = pTempWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( pTempWindow );

    // move window to same level
    if ( pThisWindow->mpWindowImpl->mpOverlapWindow != pTestWindow->mpWindowImpl->mpOverlapWindow )
    {
        sal_uInt16 nThisLevel = 0;
        sal_uInt16 nTestLevel = 0;
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

    // if TestWindow is before ThisWindow, it is in front
    pTempWindow = pTestWindow;
    do
    {
        if ( pTempWindow == pThisWindow )
            return sal_True;
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }
    while ( pTempWindow );

    return sal_False;
}

// =======================================================================

void Window::ImplSaveOverlapBackground()
{
    DBG_ASSERT( !mpWindowImpl->mpOverlapData->mpSaveBackDev, "Window::ImplSaveOverlapBackground() - Background already saved" );

    if ( !mpWindowImpl->mbFrame )
    {
        sal_uLong nSaveBackSize = mnOutWidth*mnOutHeight;
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

sal_Bool Window::ImplRestoreOverlapBackground( Region& rInvRegion )
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

        return sal_True;
    }

    return sal_False;
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

        // remove window from the list
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
        // remember next window here already, as this window could
        // be removed within the next if clause from the list
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

// -----------------------------------------------------------------------

void Window::ShowFocus( const Rectangle& rRect )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpWindowImpl->mbInShowFocus )
        return;
    mpWindowImpl->mbInShowFocus = sal_True;

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
                    mpWindowImpl->mbInShowFocus = sal_False;
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
        mpWindowImpl->mbFocusVisible = sal_True;
    }
    else
    {
        if( ! mpWindowImpl->mbNativeFocusVisible )
        {
            mpWindowImpl->mbNativeFocusVisible = sal_True;
            if ( !mpWindowImpl->mbInPaint )
                Invalidate();
        }
    }
    mpWindowImpl->mbInShowFocus = sal_False;
}

// -----------------------------------------------------------------------

void Window::HideFocus()
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if( mpWindowImpl->mbInHideFocus )
        return;
    mpWindowImpl->mbInHideFocus = sal_True;

    // native themeing can suggest not to use focus rects
    if( ! ( mpWindowImpl->mbUseNativeFocus &&
            IsNativeWidgetEnabled() ) )
    {
        if ( !mpWindowImpl->mbFocusVisible )
        {
            mpWindowImpl->mbInHideFocus = sal_False;
            return;
        }

        if ( !mpWindowImpl->mbInPaint )
            ImplInvertFocus( *(ImplGetWinData()->mpFocusRect) );
        mpWindowImpl->mbFocusVisible = sal_False;
    }
    else
    {
        if( mpWindowImpl->mbNativeFocusVisible )
        {
            mpWindowImpl->mbNativeFocusVisible = sal_False;
            if ( !mpWindowImpl->mbInPaint )
                Invalidate();
        }
    }
    mpWindowImpl->mbInHideFocus = sal_False;
}

// -----------------------------------------------------------------------

void Window::Invert( const Rectangle& rRect, sal_uInt16 nFlags )
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

void Window::Invert( const Polygon& rPoly, sal_uInt16 nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    if ( !IsDeviceOutputNecessary() )
        return;

    sal_uInt16 nPoints = rPoly.GetSize();

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

void Window::ShowTracking( const Rectangle& rRect, sal_uInt16 nFlags )
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
    mpWindowImpl->mbTrackVisible              = sal_True;
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
        mpWindowImpl->mbTrackVisible = sal_False;
    }
}

// -----------------------------------------------------------------------

void Window::InvertTracking( const Rectangle& rRect, sal_uInt16 nFlags )
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
            ImplClipBoundaries( aRegion, sal_False, sal_False );
            ImplSelectClipRegion( aRegion, pGraphics );
        }
    }

    sal_uInt16 nStyle = nFlags & SHOWTRACK_STYLE;
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

void Window::InvertTracking( const Polygon& rPoly, sal_uInt16 nFlags )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    sal_uInt16 nPoints = rPoly.GetSize();

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
            ImplClipBoundaries( aRegion, sal_False, sal_False );
            ImplSelectClipRegion( aRegion, pGraphics );
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
    if( ImplIsAntiparallel() )
    {
        // - RTL - re-mirror frame pos at pChild
        ImplReMirror( aMousePos );
    }
    MouseEvent      aMEvt( ImplFrameToOutput( aMousePos ),
                           mpWindowImpl->mpFrameData->mnClickCount, 0,
                           mpWindowImpl->mpFrameData->mnMouseCode, mpWindowImpl->mpFrameData->mnMouseCode );
    TrackingEvent   aTEvt( aMEvt, TRACKING_REPEAT );
    Tracking( aTEvt );

    return 0;
}

// -----------------------------------------------------------------------

void Window::StartTracking( sal_uInt16 nFlags )
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

void Window::EndTracking( sal_uInt16 nFlags )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpTrackWin == this )
    {
        // due to DbgChkThis in brackets, as the window could be destroyed
        // in the handler
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

        // call EndTracking if required
        if ( !(nFlags & ENDTRACK_DONTCALLHDL) )
        {
            Point           aMousePos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
            if( ImplIsAntiparallel() )
            {
                // - RTL - re-mirror frame pos at pChild
                ImplReMirror( aMousePos );
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

sal_Bool Window::IsTracking() const
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    return (ImplGetSVData()->maWinData.mpTrackWin == this);
}

// -----------------------------------------------------------------------

void Window::StartAutoScroll( sal_uInt16 nFlags )
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
            const sal_Bool      bMap = rSaveDevice.IsMapModeEnabled();

            // move clip region to have the same distance to DestOffset
            aClip.Move( aPixOffset.X() - aPixPos.X(), aPixOffset.Y() - aPixPos.Y() );

            // set pixel clip region
            rSaveDevice.EnableMapMode( sal_False );
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

sal_Bool Window::EndSaveFocus( sal_uIntPtr nSaveId, sal_Bool bRestore )
{
    if ( !nSaveId )
        return sal_False;
    else
    {
        sal_Bool                bOK = sal_True;
        ImplFocusDelData*   pDelData = (ImplFocusDelData*)(void*)nSaveId;
        if ( !pDelData->IsDead() )
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

        // Use another font if the representation is to be scaled,
        // and the actual font is not scalable
        FontMetric aMetric = GetFontMetric();
        long       nFontDiff = Abs( GetFont().GetSize().Height()-aMetric.GetSize().Height() );
        if ( (aMetric.GetType() == TYPE_RASTER) && (nFontDiff >= 2) )
        {
            sal_uInt16 nType;
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
        mpWindowImpl->mbControlForeground = sal_False;
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
            mpWindowImpl->mbControlForeground = sal_False;
            StateChanged( STATE_CHANGE_CONTROLFOREGROUND );
        }
    }
    else
    {
        if ( mpWindowImpl->maControlForeground != rColor )
        {
            mpWindowImpl->maControlForeground = rColor;
            mpWindowImpl->mbControlForeground = sal_True;
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
        mpWindowImpl->mbControlBackground = sal_False;
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
            mpWindowImpl->mbControlBackground = sal_False;
            StateChanged( STATE_CHANGE_CONTROLBACKGROUND );
        }
    }
    else
    {
        if ( mpWindowImpl->maControlBackground != rColor )
        {
            mpWindowImpl->maControlBackground = rColor;
            mpWindowImpl->mbControlBackground = sal_True;
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

sal_Bool Window::HandleScrollCommand( const CommandEvent& rCmd,
                                  ScrollBar* pHScrl, ScrollBar* pVScrl )
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    sal_Bool bRet = sal_False;

    if ( pHScrl || pVScrl )
    {
        switch( rCmd.GetCommand() )
        {
            case COMMAND_STARTAUTOSCROLL:
            {
                sal_uInt16 nFlags = 0;
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
                    bRet = sal_True;
                }
            }
            break;

            case COMMAND_WHEEL:
            {
                const CommandWheelData* pData = rCmd.GetWheelData();

                if ( pData && (COMMAND_WHEEL_SCROLL == pData->GetMode()) )
                {
                    sal_uLong nScrollLines = pData->GetScrollLines();
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
                        bRet = sal_True;
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
                    bRet = sal_True;
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

DockingManager* Window::GetDockingManager()
{
    return ImplGetDockingManager();
}

void Window::EnableDocking( sal_Bool bEnable )
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

void Window::SetHelpId( const OString& rHelpId )
{
    mpWindowImpl->maHelpId = rHelpId;
}

const OString& Window::GetHelpId() const
{
    return mpWindowImpl->maHelpId;
}

void Window::SetUniqueId( const OString& rUniqueId )
{
    mpWindowImpl->maUniqId = rUniqueId;
}

const OString& Window::GetUniqueId() const
{
    return mpWindowImpl->maUniqId;
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

sal_Bool Window::ImplIsDockingWindow() const
{
    return mpWindowImpl->mbDockWin;
}

sal_Bool Window::ImplIsFloatingWindow() const
{
    return mpWindowImpl->mbFloatWin;
}

sal_Bool Window::ImplIsSplitter() const
{
    return mpWindowImpl->mbSplitter;
}

sal_Bool Window::ImplIsPushButton() const
{
    return mpWindowImpl->mbPushButton;
}

sal_Bool Window::ImplIsOverlapWindow() const
{
    return mpWindowImpl->mbOverlapWin;
}

void Window::ImplSetMouseTransparent( sal_Bool bTransparent )
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

void Window::SetCompoundControl( sal_Bool bCompound )
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

void Window::SetType( WindowType nType )
{
    mpWindowImpl->mnType = nType;
}

WindowType Window::GetType() const
{
    return mpWindowImpl->mnType;
}

Dialog* Window::GetParentDialog() const
{
    const Window *pWindow = this;

    while( pWindow )
    {
        if( pWindow->IsDialog() )
            break;

        pWindow = pWindow->GetParent();
    }

    return const_cast<Dialog *>(dynamic_cast<const Dialog*>(pWindow));
}

sal_Bool Window::IsSystemWindow() const
{
    return mpWindowImpl->mbSysWin;
}

sal_Bool Window::IsDialog() const
{
    return mpWindowImpl->mbDialog;
}

sal_Bool Window::IsMenuFloatingWindow() const
{
    return mpWindowImpl->mbMenuFloatingWindow;
}

sal_Bool Window::IsToolbarFloatingWindow() const
{
    return mpWindowImpl->mbToolbarFloatingWindow;
}

void Window::EnableAllResize( sal_Bool bEnable )
{
    mpWindowImpl->mbAllResize = bEnable;
}

void Window::EnableChildTransparentMode( sal_Bool bEnable )
{
    mpWindowImpl->mbChildTransparent = bEnable;
}

sal_Bool Window::IsChildTransparentModeEnabled() const
{
    return mpWindowImpl->mbChildTransparent;
}

sal_Bool Window::IsMouseTransparent() const
{
    return mpWindowImpl->mbMouseTransparent;
}

sal_Bool Window::IsPaintTransparent() const
{
    return mpWindowImpl->mbPaintTransparent;
}

void Window::SetDialogControlStart( sal_Bool bStart )
{
    mpWindowImpl->mbDlgCtrlStart = bStart;
}

sal_Bool Window::IsDialogControlStart() const
{
    return mpWindowImpl->mbDlgCtrlStart;
}

void Window::SetDialogControlFlags( sal_uInt16 nFlags )
{
    mpWindowImpl->mnDlgCtrlFlags = nFlags;
}

sal_uInt16 Window::GetDialogControlFlags() const
{
    return mpWindowImpl->mnDlgCtrlFlags;
}

const InputContext& Window::GetInputContext() const
{
    return mpWindowImpl->maInputContext;
}

sal_Bool Window::IsControlFont() const
{
    return (mpWindowImpl->mpControlFont != 0);
}

Color Window::GetControlForeground() const
{
    return mpWindowImpl->maControlForeground;
}

sal_Bool Window::IsControlForeground() const
{
    return mpWindowImpl->mbControlForeground;
}

Color Window::GetControlBackground() const
{
    return mpWindowImpl->maControlBackground;
}

sal_Bool Window::IsControlBackground() const
{
    return mpWindowImpl->mbControlBackground;
}

sal_Bool Window::IsInPaint() const
{
    return mpWindowImpl->mbInPaint;
}

Window* Window::GetParent() const
{
    return mpWindowImpl->mpRealParent;
}

sal_Bool Window::IsVisible() const
{
    return mpWindowImpl->mbVisible;
}

sal_Bool Window::IsReallyVisible() const
{
    return mpWindowImpl->mbReallyVisible;
}

sal_Bool Window::IsReallyShown() const
{
    return mpWindowImpl->mbReallyShown;
}

sal_Bool Window::IsInInitShow() const
{
    return mpWindowImpl->mbInInitShow;
}

sal_Bool Window::IsEnabled() const
{
    return !mpWindowImpl->mbDisabled;
}

sal_Bool Window::IsInputEnabled() const
{
    return !mpWindowImpl->mbInputDisabled;
}

sal_Bool Window::IsAlwaysEnableInput() const
{
    return mpWindowImpl->meAlwaysInputMode == AlwaysInputEnabled;
}

sal_uInt16 Window::GetActivateMode() const
{
    return mpWindowImpl->mnActivateMode;

}

sal_Bool Window::IsAlwaysOnTopEnabled() const
{
    return mpWindowImpl->mbAlwaysOnTop;
}

sal_Bool Window::IsDefaultPos() const
{
    return mpWindowImpl->mbDefPos;
}

sal_Bool Window::IsDefaultSize() const
{
    return mpWindowImpl->mbDefSize;
}

void Window::EnablePaint( sal_Bool bEnable )
{
    mpWindowImpl->mbPaintDisabled = !bEnable;
}

sal_Bool Window::IsPaintEnabled() const
{
    return !mpWindowImpl->mbPaintDisabled;
}

sal_Bool Window::IsUpdateMode() const
{
    return !mpWindowImpl->mbNoUpdate;
}

void Window::SetParentUpdateMode( sal_Bool bUpdate )
{
    mpWindowImpl->mbNoParentUpdate = !bUpdate;
}

sal_Bool Window::IsActive() const
{
    return mpWindowImpl->mbActive;
}

sal_uInt16 Window::GetGetFocusFlags() const
{
    return mpWindowImpl->mnGetFocusFlags;
}

sal_Bool Window::IsCompoundControl() const
{
    return mpWindowImpl->mbCompoundControl;
}

sal_Bool Window::IsWait() const
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

sal_Bool Window::IsZoom() const
{
    return mpWindowImpl->maZoom.GetNumerator() != mpWindowImpl->maZoom.GetDenominator();
}

void Window::SetHelpText( const XubString& rHelpText )
{
    mpWindowImpl->maHelpText = rHelpText;
    mpWindowImpl->mbHelpTextDynamic = sal_True;
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

sal_Bool Window::IsCreatedWithToolkit() const
{
    return mpWindowImpl->mbCreatedWithToolkit;
}

void Window::SetCreatedWithToolkit( sal_Bool b )
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
    setPosSizePixel( rNewPos.X(), rNewPos.Y(), 0, 0, WINDOW_POSSIZE_POS );
}

void Window::SetSizePixel( const Size& rNewSize )
{
    setPosSizePixel( 0, 0, rNewSize.Width(), rNewSize.Height(),
                     WINDOW_POSSIZE_SIZE );
}

void Window::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    setPosSizePixel( rNewPos.X(), rNewPos.Y(),
                     rNewSize.Width(), rNewSize.Height(),
                     WINDOW_POSSIZE_POSSIZE );
}

void Window::SetOutputSizePixel( const Size& rNewSize )
{
    SetSizePixel( Size( rNewSize.Width()+mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder,
                        rNewSize.Height()+mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder ) );
}

void Window::SetBackgroundBitmap( const BitmapEx& rBitmapEx )
{
    if( !rBitmapEx.IsEmpty() )
    {
        SalBitmap* pBmp = rBitmapEx.ImplGetBitmapImpBitmap()->ImplGetSalBitmap();
        ImplGetFrame()->SetBackgroundBitmap( pBmp );
    }
}

//When a widget wants to renegotiate layout, get toplevel parent dialog and call
//resize on it. Mark all intermediate containers (or container-alike) widgets
//as dirty for the size remains unchanged, but layout changed circumstances
void Window::queue_resize()
{
    Dialog *pDialog = NULL;

    Window *pWindow = this;

    while( pWindow )
    {
        if (pWindow->GetType() == WINDOW_CONTAINER)
        {
            VclContainer *pContainer = static_cast<VclContainer*>(pWindow);
            pContainer->markLayoutDirty();
        }
        else if (pWindow->GetType() == WINDOW_TABCONTROL)
        {
            TabControl *pTabControl = static_cast<TabControl*>(pWindow);
            pTabControl->markLayoutDirty();
        }
        else if (pWindow->IsDialog())
        {
            pDialog = dynamic_cast<Dialog*>(pWindow);
            break;
        }
        pWindow = pWindow->GetParent();
    }

    if (!pDialog || pDialog == this)
        return;
    pDialog->queue_layout();
}

//We deliberately do not overwrite our maHelpId here
//so that the .res loaded id is not overwritten
//by the .ui loaded one
void Window::take_properties(Window &rOther)
{
    if (!mpWindowImpl)
    {
        ImplInitWindowData(WINDOW_WINDOW);
        ImplInit(rOther.GetParent(), rOther.GetStyle(), NULL);
    }

    WindowImpl *pWindowImpl = rOther.mpWindowImpl;
    if (!mpWindowImpl->mpRealParent)
        ImplInit(pWindowImpl->mpRealParent, rOther.GetStyle(), NULL);
    std::swap(mpWindowImpl->mpUserData, pWindowImpl->mpUserData);
    std::swap(mpWindowImpl->mpCursor, pWindowImpl->mpCursor);
    std::swap(mpWindowImpl->maPointer, pWindowImpl->maPointer);
    mpWindowImpl->maZoom = pWindowImpl->maZoom;
    mpWindowImpl->maText = pWindowImpl->maText;
    std::swap(mpWindowImpl->mpControlFont, pWindowImpl->mpControlFont);
    mpWindowImpl->maControlForeground = pWindowImpl->maControlForeground;
    mpWindowImpl->maControlBackground = pWindowImpl->maControlBackground;
    mpWindowImpl->mnLeftBorder = pWindowImpl->mnLeftBorder;
    mpWindowImpl->mnTopBorder = pWindowImpl->mnTopBorder;
    mpWindowImpl->mnRightBorder = pWindowImpl->mnRightBorder;
    mpWindowImpl->mnBottomBorder = pWindowImpl->mnBottomBorder;
    mpWindowImpl->mnWidthRequest = pWindowImpl->mnWidthRequest;
    mpWindowImpl->mnHeightRequest = pWindowImpl->mnHeightRequest;
    mpWindowImpl->mnX = pWindowImpl->mnX;
    mpWindowImpl->mnY = pWindowImpl->mnY;
    mpWindowImpl->mnAbsScreenX = pWindowImpl->mnAbsScreenX;
    mpWindowImpl->maPos = pWindowImpl->maPos;
    mpWindowImpl->maUniqId = pWindowImpl->maUniqId;
    mpWindowImpl->maHelpText = pWindowImpl->maHelpText;
    mpWindowImpl->maQuickHelpText = pWindowImpl->maQuickHelpText;
    std::swap(mpWindowImpl->maInputContext, pWindowImpl->maInputContext);
    mpWindowImpl->mnStyle = pWindowImpl->mnStyle;
    mpWindowImpl->mnPrevStyle = pWindowImpl->mnPrevStyle;
    mpWindowImpl->mnExtendedStyle = pWindowImpl->mnExtendedStyle;
    mpWindowImpl->mnPrevExtendedStyle = pWindowImpl->mnPrevExtendedStyle;
    mpWindowImpl->mnType = pWindowImpl->mnType;
    mpWindowImpl->mnNativeBackground = pWindowImpl->mnNativeBackground;
    mpWindowImpl->mnWaitCount = pWindowImpl->mnWaitCount;
    mpWindowImpl->mnPaintFlags = pWindowImpl->mnPaintFlags;
    mpWindowImpl->mnGetFocusFlags = pWindowImpl->mnGetFocusFlags;
    mpWindowImpl->mnParentClipMode = pWindowImpl->mnParentClipMode;
    mpWindowImpl->mnActivateMode = pWindowImpl->mnActivateMode;
    mpWindowImpl->mnDlgCtrlFlags = pWindowImpl->mnDlgCtrlFlags;
    mpWindowImpl->mnLockCount = pWindowImpl->mnLockCount;
    mpWindowImpl->meAlwaysInputMode = pWindowImpl->meAlwaysInputMode;
    mpWindowImpl->meHalign = pWindowImpl->meHalign;
    mpWindowImpl->meValign = pWindowImpl->meValign;
    mpWindowImpl->mePackType = pWindowImpl->mePackType;
    mpWindowImpl->mnPadding = pWindowImpl->mnPadding;
    mpWindowImpl->mnGridHeight = pWindowImpl->mnGridHeight;
    mpWindowImpl->mnGridLeftAttach = pWindowImpl->mnGridLeftAttach;
    mpWindowImpl->mnGridTopAttach = pWindowImpl->mnGridTopAttach;
    mpWindowImpl->mnGridWidth = pWindowImpl->mnGridWidth;
    mpWindowImpl->mnBorderWidth = pWindowImpl->mnBorderWidth;
    mpWindowImpl->mnMarginLeft = pWindowImpl->mnMarginLeft;
    mpWindowImpl->mnMarginRight = pWindowImpl->mnMarginRight;
    mpWindowImpl->mnMarginTop = pWindowImpl->mnMarginTop;
    mpWindowImpl->mnMarginBottom = pWindowImpl->mnMarginBottom;
    mpWindowImpl->mbFrame = pWindowImpl->mbFrame;
    mpWindowImpl->mbBorderWin = pWindowImpl->mbBorderWin;
    mpWindowImpl->mbOverlapWin = pWindowImpl->mbOverlapWin;
    mpWindowImpl->mbSysWin = pWindowImpl->mbSysWin;
    mpWindowImpl->mbDialog = pWindowImpl->mbDialog;
    mpWindowImpl->mbDockWin = pWindowImpl->mbDockWin;
    mpWindowImpl->mbFloatWin = pWindowImpl->mbFloatWin;
    mpWindowImpl->mbPushButton = pWindowImpl->mbPushButton;
    mpWindowImpl->mbVisible = pWindowImpl->mbVisible;
    mpWindowImpl->mbDisabled = pWindowImpl->mbDisabled;
    mpWindowImpl->mbInputDisabled = pWindowImpl->mbInputDisabled;
    mpWindowImpl->mbDropDisabled = pWindowImpl->mbDropDisabled;
    mpWindowImpl->mbNoUpdate = pWindowImpl->mbNoUpdate;
    mpWindowImpl->mbNoParentUpdate = pWindowImpl->mbNoParentUpdate;
    mpWindowImpl->mbActive = pWindowImpl->mbActive;
    mpWindowImpl->mbParentActive = pWindowImpl->mbParentActive;
    mpWindowImpl->mbReallyVisible = pWindowImpl->mbReallyVisible;
    mpWindowImpl->mbReallyShown = pWindowImpl->mbReallyShown;
    mpWindowImpl->mbInInitShow = pWindowImpl->mbInInitShow;
    mpWindowImpl->mbChildNotify = pWindowImpl->mbChildNotify;
    mpWindowImpl->mbChildPtrOverwrite = pWindowImpl->mbChildPtrOverwrite;
    mpWindowImpl->mbNoPtrVisible = pWindowImpl->mbNoPtrVisible;
    mpWindowImpl->mbPaintFrame = pWindowImpl->mbPaintFrame;
    mpWindowImpl->mbInPaint = pWindowImpl->mbInPaint;
    mpWindowImpl->mbMouseMove = pWindowImpl->mbMouseMove;
    mpWindowImpl->mbMouseButtonDown = pWindowImpl->mbMouseButtonDown;
    mpWindowImpl->mbMouseButtonUp = pWindowImpl->mbMouseButtonUp;
    mpWindowImpl->mbKeyInput = pWindowImpl->mbKeyInput;
    mpWindowImpl->mbKeyUp = pWindowImpl->mbKeyUp;
    mpWindowImpl->mbCommand = pWindowImpl->mbCommand;
    mpWindowImpl->mbDefPos = pWindowImpl->mbDefPos;
    mpWindowImpl->mbDefSize = pWindowImpl->mbDefSize;
    mpWindowImpl->mbCallMove = pWindowImpl->mbCallMove;
    mpWindowImpl->mbCallResize = pWindowImpl->mbCallResize;
    mpWindowImpl->mbWaitSystemResize = pWindowImpl->mbWaitSystemResize;
    mpWindowImpl->mbInitWinClipRegion = pWindowImpl->mbInitWinClipRegion;
    mpWindowImpl->mbInitChildRegion = pWindowImpl->mbInitChildRegion;
    mpWindowImpl->mbWinRegion = pWindowImpl->mbWinRegion;
    mpWindowImpl->mbClipChildren = pWindowImpl->mbClipChildren;
    mpWindowImpl->mbClipSiblings = pWindowImpl->mbClipSiblings;
    mpWindowImpl->mbChildTransparent = pWindowImpl->mbChildTransparent;
    mpWindowImpl->mbPaintTransparent = pWindowImpl->mbPaintTransparent;
    mpWindowImpl->mbMouseTransparent = pWindowImpl->mbMouseTransparent;
    mpWindowImpl->mbDlgCtrlStart = pWindowImpl->mbDlgCtrlStart;
    mpWindowImpl->mbFocusVisible = pWindowImpl->mbFocusVisible;
    mpWindowImpl->mbTrackVisible = pWindowImpl->mbTrackVisible;
    mpWindowImpl->mbUseNativeFocus = pWindowImpl->mbUseNativeFocus;
    mpWindowImpl->mbNativeFocusVisible = pWindowImpl->mbNativeFocusVisible;
    mpWindowImpl->mbInShowFocus = pWindowImpl->mbInShowFocus;
    mpWindowImpl->mbInHideFocus = pWindowImpl->mbInHideFocus;
    mpWindowImpl->mbControlForeground = pWindowImpl->mbControlForeground;
    mpWindowImpl->mbControlBackground = pWindowImpl->mbControlBackground;
    mpWindowImpl->mbAlwaysOnTop = pWindowImpl->mbAlwaysOnTop;
    mpWindowImpl->mbCompoundControl = pWindowImpl->mbCompoundControl;
    mpWindowImpl->mbCompoundControlHasFocus = pWindowImpl->mbCompoundControlHasFocus;
    mpWindowImpl->mbPaintDisabled = pWindowImpl->mbPaintDisabled;
    mpWindowImpl->mbAllResize = pWindowImpl->mbAllResize;
    mpWindowImpl->mbInDtor = pWindowImpl->mbInDtor;
    mpWindowImpl->mbExtTextInput = pWindowImpl->mbExtTextInput;
    mpWindowImpl->mbInFocusHdl = pWindowImpl->mbInFocusHdl;
    mpWindowImpl->mbOverlapVisible = pWindowImpl->mbOverlapVisible;
    mpWindowImpl->mbCreatedWithToolkit = pWindowImpl->mbCreatedWithToolkit;
    mpWindowImpl->mbToolBox = pWindowImpl->mbToolBox;
    mpWindowImpl->mbSplitter = pWindowImpl->mbSplitter;
    mpWindowImpl->mbSuppressAccessibilityEvents = pWindowImpl->mbSuppressAccessibilityEvents;
    mpWindowImpl->mbMenuFloatingWindow = pWindowImpl->mbMenuFloatingWindow;
    mpWindowImpl->mbDrawSelectionBackground = pWindowImpl->mbDrawSelectionBackground;
    mpWindowImpl->mbIsInTaskPaneList = pWindowImpl->mbIsInTaskPaneList;
    mpWindowImpl->mbToolbarFloatingWindow = pWindowImpl->mbToolbarFloatingWindow;
    mpWindowImpl->mbCallHandlersDuringInputDisabled = pWindowImpl->mbCallHandlersDuringInputDisabled;
    mpWindowImpl->mbDisableAccessibleLabelForRelation = pWindowImpl->mbDisableAccessibleLabelForRelation;
    mpWindowImpl->mbDisableAccessibleLabeledByRelation = pWindowImpl->mbDisableAccessibleLabeledByRelation;
    mpWindowImpl->mbHelpTextDynamic = pWindowImpl->mbHelpTextDynamic;
    mpWindowImpl->mbFakeFocusSet = pWindowImpl->mbFakeFocusSet;
    mpWindowImpl->mbHexpand = pWindowImpl->mbHexpand;
    mpWindowImpl->mbVexpand = pWindowImpl->mbVexpand;
    mpWindowImpl->mbExpand = pWindowImpl->mbExpand;
    mpWindowImpl->mbFill = pWindowImpl->mbFill;
    mpWindowImpl->mbSecondary = pWindowImpl->mbSecondary;

    bool bHasBorderWindow = mpWindowImpl->mpBorderWindow;
    bool bOtherHasBorderWindow = pWindowImpl->mpBorderWindow;

    assert(bHasBorderWindow == bOtherHasBorderWindow);

    if (bHasBorderWindow && bOtherHasBorderWindow)
        mpWindowImpl->mpBorderWindow->take_properties(*pWindowImpl->mpBorderWindow);
}

namespace
{
    VclAlign toAlign(const OString &rValue)
    {
        VclAlign eRet = VCL_ALIGN_FILL;

        if (rValue.equalsL(RTL_CONSTASCII_STRINGPARAM("fill")))
            eRet = VCL_ALIGN_FILL;
        else if (rValue.equalsL(RTL_CONSTASCII_STRINGPARAM("start")))
            eRet = VCL_ALIGN_START;
        else if (rValue.equalsL(RTL_CONSTASCII_STRINGPARAM("end")))
            eRet = VCL_ALIGN_END;
        else if (rValue.equalsL(RTL_CONSTASCII_STRINGPARAM("center")))
            eRet = VCL_ALIGN_CENTER;
        return eRet;
    }
}

namespace
{
    OString convertMnemonicMarkup(const OString &rIn)
    {
        OStringBuffer aRet(rIn);
        for (sal_Int32 nI = 0; nI < aRet.getLength(); ++nI)
        {
            if (aRet[nI] == '_')
            {
                if (aRet[nI+1] != '_')
                    aRet[nI] = '~';
                else
                    aRet.remove(nI, 1);
                ++nI;
            }
        }
        return aRet.makeStringAndClear();
    }
}

bool Window::set_property(const OString &rKey, const OString &rValue)
{
    if (
         (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("label"))) ||
         (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("title"))) ||
         (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("text")))
       )
    {
        SetText(OStringToOUString(convertMnemonicMarkup(rValue), RTL_TEXTENCODING_UTF8));
    }
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("visible")))
        Show(toBool(rValue));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("sensitive")))
        Enable(toBool(rValue));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("resizable")))
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_SIZEABLE);
        if (toBool(rValue))
            nBits |= WB_SIZEABLE;
        SetStyle(nBits);
    }
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("xalign")))
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_LEFT | WB_CENTER | WB_RIGHT);

        float f = rValue.toFloat();
        if (f == 0.0)
            nBits |= WB_LEFT;
        else if (f == 1.0)
            nBits |= WB_RIGHT;
        else if (f == 0.5)
            nBits |= WB_CENTER;

        SetStyle(nBits);
    }
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("yalign")))
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_TOP | WB_VCENTER | WB_BOTTOM);

        float f = rValue.toFloat();
        if (f == 0.0)
            nBits |= WB_TOP;
        else if (f == 1.0)
            nBits |= WB_BOTTOM;
        else if (f == 0.5)
            nBits |= WB_CENTER;

        SetStyle(nBits);
    }
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("wrap")))
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_WORDBREAK);
        if (toBool(rValue))
            nBits |= WB_WORDBREAK;
        SetStyle(nBits);
    }
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("height-request")))
        set_height_request(rValue.toInt32());
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("width-request")))
        set_width_request(rValue.toInt32());
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("hexpand")))
        set_hexpand(toBool(rValue));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("vexpand")))
        set_vexpand(toBool(rValue));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("halign")))
        set_halign(toAlign(rValue));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("valign")))
        set_valign(toAlign(rValue));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("tooltip-markup")))
        SetQuickHelpText(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("tooltip-text")))
        SetQuickHelpText(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("border-width")))
        set_border_width(rValue.toInt32());
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("margin-left")))
        set_margin_left(rValue.toInt32());
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("margin-right")))
        set_margin_right(rValue.toInt32());
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("margin-top")))
        set_margin_top(rValue.toInt32());
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("margin-bottom")))
        set_margin_bottom(rValue.toInt32());
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("hscrollbar-policy")))
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_AUTOHSCROLL|WB_HSCROLL);
        if (rValue == "always")
            nBits |= WB_HSCROLL;
        else if (rValue == "automatic")
            nBits |= WB_AUTOHSCROLL;
        SetStyle(nBits);
    }
    else if (rKey.equalsL(RTL_CONSTASCII_STRINGPARAM("vscrollbar-policy")))
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_AUTOVSCROLL|WB_VSCROLL);
        if (rValue == "always")
            nBits |= WB_VSCROLL;
        else if (rValue == "automatic")
            nBits |= WB_AUTOVSCROLL;
        SetStyle(nBits);
    }
    else
    {
        SAL_INFO("vcl.layout", "unhandled property: " << rKey.getStr());
        return false;
    }
    return true;
}

void Window::set_height_request(sal_Int32 nHeightRequest)
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;

    if ( pWindowImpl->mnHeightRequest != nHeightRequest )
    {
        pWindowImpl->mnHeightRequest = nHeightRequest;
        queue_resize();
    }
}

void Window::set_width_request(sal_Int32 nWidthRequest)
{
    DBG_CHKTHIS( Window, ImplDbgCheckWindow );

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;

    if ( pWindowImpl->mnWidthRequest != nWidthRequest )
    {
        pWindowImpl->mnWidthRequest = nWidthRequest;
        queue_resize();
    }
}

Size Window::get_preferred_size() const
{
    Size aRet(get_width_request(), get_height_request());
    if (aRet.Width() == -1 || aRet.Height() == -1)
    {
        Size aOptimal = GetOptimalSize(WINDOWSIZE_PREFERRED);
        if (aRet.Width() == -1)
            aRet.Width() = aOptimal.Width();
        if (aRet.Height() == -1)
            aRet.Height() = aOptimal.Height();
    }
    return aRet;
}

VclAlign Window::get_halign() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->meHalign;
}

void Window::set_halign(VclAlign eAlign)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->meHalign = eAlign;
}

VclAlign Window::get_valign() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->meValign;
}

void Window::set_valign(VclAlign eAlign)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->meValign = eAlign;
}

bool Window::get_hexpand() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbHexpand;
}

void Window::set_hexpand(bool bExpand)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbHexpand = bExpand;
}

bool Window::get_vexpand() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbVexpand;
}

void Window::set_vexpand(bool bExpand)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbVexpand = bExpand;
}

bool Window::get_expand() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbExpand;
}

void Window::set_expand(bool bExpand)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbExpand = bExpand;
}

VclPackType Window::get_pack_type() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mePackType;
}

void Window::set_pack_type(VclPackType ePackType)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mePackType = ePackType;
}

sal_Int32 Window::get_padding() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnPadding;
}

void Window::set_padding(sal_Int32 nPadding)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnPadding = nPadding;
}

bool Window::get_fill() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbFill;
}

void Window::set_fill(bool bFill)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbFill = bFill;
}

sal_Int32 Window::get_grid_width() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnGridWidth;
}

void Window::set_grid_width(sal_Int32 nCols)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnGridWidth = nCols;
}

sal_Int32 Window::get_grid_left_attach() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnGridLeftAttach;
}

void Window::set_grid_left_attach(sal_Int32 nAttach)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnGridLeftAttach = nAttach;
}

sal_Int32 Window::get_grid_height() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnGridHeight;
}

void Window::set_grid_height(sal_Int32 nRows)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnGridHeight = nRows;
}

sal_Int32 Window::get_grid_top_attach() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnGridTopAttach;
}

void Window::set_grid_top_attach(sal_Int32 nAttach)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnGridTopAttach = nAttach;
}

void Window::set_border_width(sal_Int32 nBorderWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnBorderWidth = nBorderWidth;
}

sal_Int32 Window::get_border_width() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnBorderWidth;
}

void Window::set_margin_left(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnMarginLeft = nWidth;
}

sal_Int32 Window::get_margin_left() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnMarginLeft;
}

void Window::set_margin_right(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnMarginRight = nWidth;
}

sal_Int32 Window::get_margin_right() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnMarginRight;
}

void Window::set_margin_top(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnMarginTop = nWidth;
}

sal_Int32 Window::get_margin_top() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnMarginTop;
}

void Window::set_margin_bottom(sal_Int32 nWidth)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mnMarginBottom = nWidth;
}

sal_Int32 Window::get_margin_bottom() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnMarginBottom;
}

sal_Int32 Window::get_height_request() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnHeightRequest;
}

sal_Int32 Window::get_width_request() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mnWidthRequest;
}

bool Window::get_secondary() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbSecondary;
}

void Window::set_secondary(bool bSecondary)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbSecondary = bSecondary;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
