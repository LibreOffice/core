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


#include <limits.h>
#include <tools/debug.hxx>
#include <tools/poly.hxx>

#include <vcl/bitmap.hxx>
#include <vcl/dialog.hxx>
#include <vcl/event.hxx>
#include <vcl/fixed.hxx>
#include <vcl/layout.hxx>
#include <vcl/timer.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/settings.hxx>

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



#define IMPL_MAXSAVEBACKSIZE    (640*480)
#define IMPL_MAXALLSAVEBACKSIZE (800*600*2)



struct ImplFocusDelData : public ImplDelData
{
    Window*         mpFocusWin;
};



bool Window::ImplIsWindowInFront( const Window* pTestWindow ) const
{
    // check for overlapping window
    pTestWindow = pTestWindow->ImplGetFirstOverlapWindow();
    const Window* pTempWindow = pTestWindow;
    const Window* pThisWindow = ImplGetFirstOverlapWindow();
    if ( pTempWindow == pThisWindow )
        return false;
    do
    {
        if ( pTempWindow == pThisWindow )
            return true;
        if ( pTempWindow->mpWindowImpl->mbFrame )
            break;
        pTempWindow = pTempWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( pTempWindow );
    pTempWindow = pThisWindow;
    do
    {
        if ( pTempWindow == pTestWindow )
            return false;
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
    while ( pTempWindow )
    {
        if ( pTempWindow == pThisWindow )
            return true;
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    return false;
}



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

                    OutputDevice *pOutDev = mpWindowImpl->mpFrameWindow->GetOutDev();
                    pOutDev->ImplGetFrameDev( Point( mnOutOffX, mnOutOffY ),
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



bool Window::ImplRestoreOverlapBackground( Region& rInvRegion )
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

            OutputDevice *pOutDev = mpWindowImpl->mpFrameWindow->GetOutDev();

            if ( mpWindowImpl->mpOverlapData->mpSaveBackRgn )
            {
                mpWindowImpl->mpOverlapData->mpSaveBackRgn->Intersect( mpWindowImpl->maWinClipRegion );
                rInvRegion = mpWindowImpl->maWinClipRegion;
                rInvRegion.Exclude( *mpWindowImpl->mpOverlapData->mpSaveBackRgn );
                pOutDev->ImplDrawFrameDev( aDestPt, aDevPt, aDevSize,
                                           *(mpWindowImpl->mpOverlapData->mpSaveBackDev),
                                           *mpWindowImpl->mpOverlapData->mpSaveBackRgn );
            }
            else
            {
                pOutDev->ImplDrawFrameDev( aDestPt, aDevPt, aDevSize,
                                           *(mpWindowImpl->mpOverlapData->mpSaveBackDev),
                                           mpWindowImpl->maWinClipRegion );
            }
            ImplDeleteOverlapBackground();
        }

        return true;
    }

    return false;
}



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



void Window::ImplInvalidateAllOverlapBackgrounds()
{
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



void Window::ShowFocus( const Rectangle& rRect )
{
    if( mpWindowImpl->mbInShowFocus )
        return;
    mpWindowImpl->mbInShowFocus = true;

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
                    mpWindowImpl->mbInShowFocus = false;
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
        mpWindowImpl->mbFocusVisible = true;
    }
    else
    {
        if( ! mpWindowImpl->mbNativeFocusVisible )
        {
            mpWindowImpl->mbNativeFocusVisible = true;
            if ( !mpWindowImpl->mbInPaint )
                Invalidate();
        }
    }
    mpWindowImpl->mbInShowFocus = false;
}



void Window::HideFocus()
{

    if( mpWindowImpl->mbInHideFocus )
        return;
    mpWindowImpl->mbInHideFocus = true;

    // native themeing can suggest not to use focus rects
    if( ! ( mpWindowImpl->mbUseNativeFocus &&
            IsNativeWidgetEnabled() ) )
    {
        if ( !mpWindowImpl->mbFocusVisible )
        {
            mpWindowImpl->mbInHideFocus = false;
            return;
        }

        if ( !mpWindowImpl->mbInPaint )
            ImplInvertFocus( *(ImplGetWinData()->mpFocusRect) );
        mpWindowImpl->mbFocusVisible = false;
    }
    else
    {
        if( mpWindowImpl->mbNativeFocusVisible )
        {
            mpWindowImpl->mbNativeFocusVisible = false;
            if ( !mpWindowImpl->mbInPaint )
                Invalidate();
        }
    }
    mpWindowImpl->mbInHideFocus = false;
}



void Window::Invert( const Rectangle& rRect, sal_uInt16 nFlags )
{
    if ( !IsDeviceOutputNecessary() )
        return;

    OutputDevice *pOutDev = GetOutDev();
    Rectangle aRect( pOutDev->ImplLogicToDevicePixel( rRect ) );

    if ( aRect.IsEmpty() )
        return;
    aRect.Justify();

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !pOutDev->ImplGetGraphics() )
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



void Window::Invert( const Polygon& rPoly, sal_uInt16 nFlags )
{
    if ( !IsDeviceOutputNecessary() )
        return;

    sal_uInt16 nPoints = rPoly.GetSize();

    if ( nPoints < 2 )
        return;

    OutputDevice *pOutDev = GetOutDev();
    Polygon aPoly( pOutDev->ImplLogicToDevicePixel( rPoly ) );

    // we need a graphics
    if ( !mpGraphics )
    {
        if ( !pOutDev->ImplGetGraphics() )
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



void Window::ShowTracking( const Rectangle& rRect, sal_uInt16 nFlags )
{
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
    mpWindowImpl->mbTrackVisible              = true;
}



void Window::HideTracking()
{
    if ( mpWindowImpl->mbTrackVisible )
    {
        ImplWinData* pWinData = ImplGetWinData();
        if ( !mpWindowImpl->mbInPaint || !(pWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
            InvertTracking( *(pWinData->mpTrackRect), pWinData->mnTrackFlags );
        mpWindowImpl->mbTrackVisible = false;
    }
}



void Window::InvertTracking( const Rectangle& rRect, sal_uInt16 nFlags )
{
    OutputDevice *pOutDev = GetOutDev();
    Rectangle aRect( pOutDev->ImplLogicToDevicePixel( rRect ) );

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
            if ( !pOutDev->ImplGetGraphics() )
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
            ImplClipBoundaries( aRegion, false, false );
            pOutDev->ImplSelectClipRegion( aRegion, pGraphics );
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



void Window::InvertTracking( const Polygon& rPoly, sal_uInt16 nFlags )
{
    sal_uInt16 nPoints = rPoly.GetSize();

    if ( nPoints < 2 )
        return;

    OutputDevice *pOutDev = GetOutDev();

    Polygon aPoly( pOutDev->ImplLogicToDevicePixel( rPoly ) );

    SalGraphics* pGraphics;

    if ( nFlags & SHOWTRACK_WINDOW )
    {
        if ( !IsDeviceOutputNecessary() )
            return;

        // we need a graphics
        if ( !mpGraphics )
        {
            if ( !pOutDev->ImplGetGraphics() )
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
            ImplClipBoundaries( aRegion, false, false );
            pOutDev->ImplSelectClipRegion( aRegion, pGraphics );
        }
    }

    const SalPoint* pPtAry = (const SalPoint*)aPoly.GetConstPointAry();
    pGraphics->Invert( nPoints, pPtAry, SAL_INVERT_TRACKFRAME, this );
}



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
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aMousePos );
    }
    MouseEvent      aMEvt( ImplFrameToOutput( aMousePos ),
                           mpWindowImpl->mpFrameData->mnClickCount, 0,
                           mpWindowImpl->mpFrameData->mnMouseCode, mpWindowImpl->mpFrameData->mnMouseCode );
    TrackingEvent   aTEvt( aMEvt, TRACKING_REPEAT );
    Tracking( aTEvt );

    return 0;
}



void Window::StartTracking( sal_uInt16 nFlags )
{
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



void Window::EndTracking( sal_uInt16 nFlags )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpTrackWin == this )
    {
        // due to DbgChkThis in brackets, as the window could be destroyed
        // in the handler
        {

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
                const OutputDevice *pOutDev = GetOutDev();
                pOutDev->ReMirror( aMousePos );
            }

            MouseEvent      aMEvt( ImplFrameToOutput( aMousePos ),
                                   mpWindowImpl->mpFrameData->mnClickCount, 0,
                                   mpWindowImpl->mpFrameData->mnMouseCode, mpWindowImpl->mpFrameData->mnMouseCode );
            TrackingEvent   aTEvt( aMEvt, nFlags | ENDTRACK_END );
            Tracking( aTEvt );
        }
    }
}



bool Window::IsTracking() const
{
    return (ImplGetSVData()->maWinData.mpTrackWin == this);
}



void Window::StartAutoScroll( sal_uInt16 nFlags )
{
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



void Window::EndAutoScroll()
{
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



void Window::SaveBackground( const Point& rPos, const Size& rSize,
                             const Point& rDestOff, VirtualDevice& rSaveDevice )
{
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
            const bool      bMap = rSaveDevice.IsMapModeEnabled();

            // move clip region to have the same distance to DestOffset
            aClip.Move( aPixOffset.X() - aPixPos.X(), aPixOffset.Y() - aPixPos.Y() );

            // set pixel clip region
            rSaveDevice.EnableMapMode( false );
            rSaveDevice.SetClipRegion( aClip );
            rSaveDevice.EnableMapMode( bMap );
            rSaveDevice.DrawOutDev( rDestOff, rSize, rPos, rSize, *this );
            rSaveDevice.SetClipRegion( aOldClip );
        }
    }
    else
        rSaveDevice.DrawOutDev( rDestOff, rSize, rPos, rSize, *this );
}



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



bool Window::EndSaveFocus( sal_uIntPtr nSaveId, bool bRestore )
{
    if ( !nSaveId )
        return false;
    else
    {
        bool                bOK = true;
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



void Window::SetZoom( const Fraction& rZoom )
{
    if ( mpWindowImpl->maZoom != rZoom )
    {
        mpWindowImpl->maZoom = rZoom;
        StateChanged( STATE_CHANGE_ZOOM );
    }
}



inline long WinFloatRound( double fVal )
{
    return( fVal > 0.0 ? (long) ( fVal + 0.5 ) : -(long) ( -fVal + 0.5 ) );
}



void Window::SetZoomedPointFont( const Font& rFont )
{
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
        long       nFontDiff = std::abs( GetFont().GetSize().Height()-aMetric.GetSize().Height() );
        if ( (aMetric.GetType() == TYPE_RASTER) && (nFontDiff >= 2) )
        {
            sal_uInt16 nType;
            if ( aMetric.GetPitch() == PITCH_FIXED )
                nType = DEFAULTFONT_FIXED;
            else
                nType = DEFAULTFONT_UI_SANS;
            Font aTempFont = GetDefaultFont( nType, GetSettings().GetLanguageTag().getLanguageType(), 0 );
            aFont.SetName( aTempFont.GetName() );
            SetPointFont( aFont );
        }
    }
    else
        SetPointFont( rFont );
}



long Window::CalcZoom( long nCalc ) const
{

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



void Window::SetControlFont()
{
    if ( mpWindowImpl->mpControlFont )
    {
        delete mpWindowImpl->mpControlFont;
        mpWindowImpl->mpControlFont = NULL;
        StateChanged( STATE_CHANGE_CONTROLFONT );
    }
}



void Window::SetControlFont( const Font& rFont )
{
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



Font Window::GetControlFont() const
{
    if ( mpWindowImpl->mpControlFont )
        return *mpWindowImpl->mpControlFont;
    else
    {
        Font aFont;
        return aFont;
    }
}



void Window::SetControlForeground()
{
    if ( mpWindowImpl->mbControlForeground )
    {
        mpWindowImpl->maControlForeground = Color( COL_TRANSPARENT );
        mpWindowImpl->mbControlForeground = false;
        StateChanged( STATE_CHANGE_CONTROLFOREGROUND );
    }
}



void Window::SetControlForeground( const Color& rColor )
{
    if ( rColor.GetTransparency() )
    {
        if ( mpWindowImpl->mbControlForeground )
        {
            mpWindowImpl->maControlForeground = Color( COL_TRANSPARENT );
            mpWindowImpl->mbControlForeground = false;
            StateChanged( STATE_CHANGE_CONTROLFOREGROUND );
        }
    }
    else
    {
        if ( mpWindowImpl->maControlForeground != rColor )
        {
            mpWindowImpl->maControlForeground = rColor;
            mpWindowImpl->mbControlForeground = true;
            StateChanged( STATE_CHANGE_CONTROLFOREGROUND );
        }
    }
}



void Window::SetControlBackground()
{
    if ( mpWindowImpl->mbControlBackground )
    {
        mpWindowImpl->maControlBackground = Color( COL_TRANSPARENT );
        mpWindowImpl->mbControlBackground = false;
        StateChanged( STATE_CHANGE_CONTROLBACKGROUND );
    }
}



void Window::SetControlBackground( const Color& rColor )
{
    if ( rColor.GetTransparency() )
    {
        if ( mpWindowImpl->mbControlBackground )
        {
            mpWindowImpl->maControlBackground = Color( COL_TRANSPARENT );
            mpWindowImpl->mbControlBackground = false;
            StateChanged( STATE_CHANGE_CONTROLBACKGROUND );
        }
    }
    else
    {
        if ( mpWindowImpl->maControlBackground != rColor )
        {
            mpWindowImpl->maControlBackground = rColor;
            mpWindowImpl->mbControlBackground = true;
            StateChanged( STATE_CHANGE_CONTROLBACKGROUND );
        }
    }
}



Size Window::CalcWindowSize( const Size& rOutSz ) const
{
    Size aSz = rOutSz;
    aSz.Width()  += mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder;
    aSz.Height() += mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder;
    return aSz;
}



Size Window::CalcOutputSize( const Size& rWinSz ) const
{
    Size aSz = rWinSz;
    aSz.Width()  -= mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder;
    aSz.Height() -= mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder;
    return aSz;
}



Font Window::GetDrawPixelFont( OutputDevice* pDev ) const
{
    Font    aFont = GetPointFont();
    Size    aFontSize = aFont.GetSize();
    MapMode aPtMapMode( MAP_POINT );
    aFontSize = pDev->LogicToPixel( aFontSize, aPtMapMode );
    aFont.SetSize( aFontSize );
    return aFont;
}



long Window::GetDrawPixel( OutputDevice* pDev, long nPixels ) const
{
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



static void lcl_HandleScrollHelper( ScrollBar* pScrl, long nN, bool isMultiplyByLineSize )
{
    if ( pScrl && nN && pScrl->IsEnabled() && pScrl->IsInputEnabled() && ! pScrl->IsInModalMode() )
    {
        long nNewPos = pScrl->GetThumbPos();

        if ( nN == -LONG_MAX )
            nNewPos += pScrl->GetPageSize();
        else if ( nN == LONG_MAX )
            nNewPos -= pScrl->GetPageSize();
        else
        {
            // allowing both chunked and continuous scrolling
            if(isMultiplyByLineSize){
                nN*=pScrl->GetLineSize();
            }

            const double fVal = (double)(nNewPos - nN);

            if ( fVal < LONG_MIN )
                nNewPos = LONG_MIN;
            else if ( fVal > LONG_MAX )
                nNewPos = LONG_MAX;
            else
                nNewPos = (long)fVal;
        }

        pScrl->DoScroll( nNewPos );
    }

}

bool Window::HandleScrollCommand( const CommandEvent& rCmd,
                                  ScrollBar* pHScrl, ScrollBar* pVScrl )
{
    bool bRet = false;

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
                    bRet = true;
                }
            }
            break;

            case COMMAND_WHEEL:
            {
                const CommandWheelData* pData = rCmd.GetWheelData();

                if ( pData && (COMMAND_WHEEL_SCROLL == pData->GetMode()) )
                {
                    if (!pData->IsDeltaPixel())
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
                            bRet = true;
                        }
                    }
                    else
                    {
                        // Mobile / touch scrolling section
                        const Point & deltaPoint = rCmd.GetMousePosPixel();

                        double deltaXInPixels = double(deltaPoint.X());
                        double deltaYInPixels = double(deltaPoint.Y());
                        Size winSize = this->GetOutputSizePixel();

                        if(pHScrl)
                        {
                            double visSizeX = double(pHScrl->GetVisibleSize());
                            double ratioX = deltaXInPixels / double(winSize.getWidth());
                            long deltaXInLogic = long(visSizeX * ratioX);
                            // Touch need to work by pixels. Did not apply this to
                            // Android, as android code may require adaptations
                            // to work with this scrolling code
#ifndef IOS
                            long lineSizeX = pHScrl->GetLineSize();

                            if(lineSizeX)
                            {
                                deltaXInLogic /= lineSizeX;
                            }
                            else
                            {
                                deltaXInLogic = 0;
                            }
#endif
                            if ( deltaXInLogic)
                            {
#ifndef IOS
                                bool isMultiplyByLineSize = true;
#else
                                bool isMultiplyByLineSize = false;
#endif
                                lcl_HandleScrollHelper( pHScrl, deltaXInLogic, isMultiplyByLineSize );
                                bRet = true;
                            }
                        }
                        if(pVScrl)
                        {
                            double visSizeY = double(pVScrl->GetVisibleSize());
                            double ratioY = deltaYInPixels / double(winSize.getHeight());
                            long deltaYInLogic = long(visSizeY * ratioY);

                            // Touch need to work by pixels. Did not apply this to
                            // Android, as android code may require adaptations
                            // to work with this scrolling code
#ifndef IOS
                            long lineSizeY = pVScrl->GetLineSize();
                            if(lineSizeY)
                            {
                                deltaYInLogic /= lineSizeY;
                            }
                            else
                            {
                                deltaYInLogic = 0;
                            }
#endif
                            if ( deltaYInLogic )
                            {
#ifndef IOS
                                bool isMultiplyByLineSize = true;
#else
                                bool isMultiplyByLineSize = false;
#endif
                                lcl_HandleScrollHelper( pVScrl, deltaYInLogic, isMultiplyByLineSize );

                                bRet = true;
                            }
                        }
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
                    bRet = true;
                }
            }
            break;

            default:
            break;
        }
    }

    return bRet;
}





// Note that when called for COMMAND_WHEEL above, despite its name,
// pVScrl isn't necessarily the vertical scroll bar. Depending on
// whether the scroll is horizontal or vertical, it is either the
// horizontal or vertical scroll bar. nY is correspondingly either
// the horizontal or vertical scroll amount.

void Window::ImplHandleScroll( ScrollBar* pHScrl, long nX,
                               ScrollBar* pVScrl, long nY )
{
    lcl_HandleScrollHelper( pHScrl, nX, true );
    lcl_HandleScrollHelper( pVScrl, nY, true );
}

DockingManager* Window::GetDockingManager()
{
    return ImplGetDockingManager();
}

void Window::EnableDocking( bool bEnable )
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

bool Window::ImplIsDockingWindow() const
{
    return mpWindowImpl->mbDockWin;
}

bool Window::ImplIsFloatingWindow() const
{
    return mpWindowImpl->mbFloatWin;
}

bool Window::ImplIsSplitter() const
{
    return mpWindowImpl->mbSplitter;
}

bool Window::ImplIsPushButton() const
{
    return mpWindowImpl->mbPushButton;
}

bool Window::ImplIsOverlapWindow() const
{
    return mpWindowImpl->mbOverlapWin;
}

void Window::ImplSetMouseTransparent( bool bTransparent )
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

void Window::SetCompoundControl( bool bCompound )
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

bool Window::IsSystemWindow() const
{
    return mpWindowImpl->mbSysWin;
}

bool Window::IsDialog() const
{
    return mpWindowImpl->mbDialog;
}

bool Window::IsMenuFloatingWindow() const
{
    return mpWindowImpl->mbMenuFloatingWindow;
}

bool Window::IsToolbarFloatingWindow() const
{
    return mpWindowImpl->mbToolbarFloatingWindow;
}

void Window::EnableAllResize( bool bEnable )
{
    mpWindowImpl->mbAllResize = bEnable;
}

void Window::EnableChildTransparentMode( bool bEnable )
{
    mpWindowImpl->mbChildTransparent = bEnable;
}

bool Window::IsChildTransparentModeEnabled() const
{
    return mpWindowImpl->mbChildTransparent;
}

bool Window::IsMouseTransparent() const
{
    return mpWindowImpl->mbMouseTransparent;
}

bool Window::IsPaintTransparent() const
{
    return mpWindowImpl->mbPaintTransparent;
}

void Window::SetDialogControlStart( bool bStart )
{
    mpWindowImpl->mbDlgCtrlStart = bStart;
}

bool Window::IsDialogControlStart() const
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

bool Window::IsControlFont() const
{
    return (mpWindowImpl->mpControlFont != 0);
}

Color Window::GetControlForeground() const
{
    return mpWindowImpl->maControlForeground;
}

bool Window::IsControlForeground() const
{
    return mpWindowImpl->mbControlForeground;
}

Color Window::GetControlBackground() const
{
    return mpWindowImpl->maControlBackground;
}

bool Window::IsControlBackground() const
{
    return mpWindowImpl->mbControlBackground;
}

bool Window::IsInPaint() const
{
    return mpWindowImpl->mbInPaint;
}

Window* Window::GetParent() const
{
    return mpWindowImpl->mpRealParent;
}

bool Window::IsVisible() const
{
    return mpWindowImpl->mbVisible;
}

bool Window::IsReallyVisible() const
{
    return mpWindowImpl->mbReallyVisible;
}

bool Window::IsReallyShown() const
{
    return mpWindowImpl->mbReallyShown;
}

bool Window::IsInInitShow() const
{
    return mpWindowImpl->mbInInitShow;
}

bool Window::IsEnabled() const
{
    return !mpWindowImpl->mbDisabled;
}

bool Window::IsInputEnabled() const
{
    return !mpWindowImpl->mbInputDisabled;
}

bool Window::IsAlwaysEnableInput() const
{
    return mpWindowImpl->meAlwaysInputMode == AlwaysInputEnabled;
}

sal_uInt16 Window::GetActivateMode() const
{
    return mpWindowImpl->mnActivateMode;

}

bool Window::IsAlwaysOnTopEnabled() const
{
    return mpWindowImpl->mbAlwaysOnTop;
}

bool Window::IsDefaultPos() const
{
    return mpWindowImpl->mbDefPos;
}

bool Window::IsDefaultSize() const
{
    return mpWindowImpl->mbDefSize;
}

void Window::EnablePaint( bool bEnable )
{
    mpWindowImpl->mbPaintDisabled = !bEnable;
}

bool Window::IsPaintEnabled() const
{
    return !mpWindowImpl->mbPaintDisabled;
}

bool Window::IsUpdateMode() const
{
    return !mpWindowImpl->mbNoUpdate;
}

void Window::SetParentUpdateMode( bool bUpdate )
{
    mpWindowImpl->mbNoParentUpdate = !bUpdate;
}

bool Window::IsActive() const
{
    return mpWindowImpl->mbActive;
}

sal_uInt16 Window::GetGetFocusFlags() const
{
    return mpWindowImpl->mnGetFocusFlags;
}

bool Window::IsCompoundControl() const
{
    return mpWindowImpl->mbCompoundControl;
}

bool Window::IsWait() const
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

bool Window::IsZoom() const
{
    return mpWindowImpl->maZoom.GetNumerator() != mpWindowImpl->maZoom.GetDenominator();
}

void Window::SetHelpText( const OUString& rHelpText )
{
    mpWindowImpl->maHelpText = rHelpText;
    mpWindowImpl->mbHelpTextDynamic = true;
}

void Window::SetQuickHelpText( const OUString& rHelpText )
{
    mpWindowImpl->maQuickHelpText = rHelpText;
}

const OUString& Window::GetQuickHelpText() const
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

bool Window::IsCreatedWithToolkit() const
{
    return mpWindowImpl->mbCreatedWithToolkit;
}

void Window::SetCreatedWithToolkit( bool b )
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

//When a widget wants to renegotiate layout, get toplevel parent dialog and call
//resize on it. Mark all intermediate containers (or container-alike) widgets
//as dirty for the size remains unchanged, but layout changed circumstances
namespace
{
    bool queue_ungrouped_resize(Window *pOrigWindow)
    {
        bool bSomeoneCares = false;

        Window *pWindow = pOrigWindow->GetParent();
        if (pWindow)
        {
            if (isContainerWindow(*pWindow))
            {
                bSomeoneCares = true;
            }
            else if (pWindow->GetType() == WINDOW_TABCONTROL)
            {
                bSomeoneCares = true;
            }
            pWindow->queue_resize();
        }

        return bSomeoneCares;
    }
}

void Window::queue_resize()
{
    bool bSomeoneCares = queue_ungrouped_resize(this);

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    if (pWindowImpl->m_xSizeGroup && pWindowImpl->m_xSizeGroup->get_mode() != VCL_SIZE_GROUP_NONE)
    {
        std::set<Window*> &rWindows = pWindowImpl->m_xSizeGroup->get_widgets();
        for (std::set<Window*>::iterator aI = rWindows.begin(),
            aEnd = rWindows.end(); aI != aEnd; ++aI)
        {
            Window *pOther = *aI;
            if (pOther == this)
                continue;
            queue_ungrouped_resize(pOther);
        }
    }

    if (bSomeoneCares)
    {
        //fdo#57090 force a resync of the borders of the borderwindow onto this
        //window in case they have changed
        Window* pBorderWindow = ImplGetBorderWindow();
        if (pBorderWindow)
            pBorderWindow->Resize();
    }
}

namespace
{
    VclAlign toAlign(const OString &rValue)
    {
        VclAlign eRet = VCL_ALIGN_FILL;

        if (rValue == "fill")
            eRet = VCL_ALIGN_FILL;
        else if (rValue == "start")
            eRet = VCL_ALIGN_START;
        else if (rValue == "end")
            eRet = VCL_ALIGN_END;
        else if (rValue == "center")
            eRet = VCL_ALIGN_CENTER;
        return eRet;
    }
}

bool Window::set_font_attribute(const OString &rKey, const OString &rValue)
{
    if (rKey == "weight")
    {
        Font aFont(GetControlFont());
        if (rValue == "thin")
            aFont.SetWeight(WEIGHT_THIN);
        else if (rValue == "ultralight")
            aFont.SetWeight(WEIGHT_ULTRALIGHT);
        else if (rValue == "light")
            aFont.SetWeight(WEIGHT_LIGHT);
        else if (rValue == "book")
            aFont.SetWeight(WEIGHT_SEMILIGHT);
        else if (rValue == "normal")
            aFont.SetWeight(WEIGHT_NORMAL);
        else if (rValue == "medium")
            aFont.SetWeight(WEIGHT_MEDIUM);
        else if (rValue == "semibold")
            aFont.SetWeight(WEIGHT_SEMIBOLD);
        else if (rValue == "bold")
            aFont.SetWeight(WEIGHT_BOLD);
        else if (rValue == "ultrabold")
            aFont.SetWeight(WEIGHT_ULTRABOLD);
        else
            aFont.SetWeight(WEIGHT_BLACK);
        SetControlFont(aFont);
    }
    else if (rKey == "style")
    {
        Font aFont(GetControlFont());
        if (rValue == "normal")
            aFont.SetItalic(ITALIC_NONE);
        else if (rValue == "oblique")
            aFont.SetItalic(ITALIC_OBLIQUE);
        else if (rValue == "italic")
            aFont.SetItalic(ITALIC_NORMAL);
        SetControlFont(aFont);
    }
    else if (rKey == "underline" && toBool(rValue) == true)
    {
        Font aFont(GetControlFont());
        aFont.SetUnderline(UNDERLINE_SINGLE);
        SetControlFont(aFont);
    }
    else
    {
        SAL_INFO("vcl.layout", "unhandled font attribute: " << rKey.getStr());
        return false;
    }
    return true;
}


bool Window::set_property(const OString &rKey, const OString &rValue)
{
    if ((rKey == "label") || (rKey == "title") || (rKey == "text") )
    {
        SetText(OStringToOUString(VclBuilder::convertMnemonicMarkup(rValue), RTL_TEXTENCODING_UTF8));
    }
    else if (rKey == "visible")
        Show(toBool(rValue));
    else if (rKey == "sensitive")
        Enable(toBool(rValue));
    else if (rKey == "resizable")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_SIZEABLE);
        if (toBool(rValue))
            nBits |= WB_SIZEABLE;
        SetStyle(nBits);
    }
    else if (rKey == "xalign")
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
    else if (rKey == "justification")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_LEFT | WB_CENTER | WB_RIGHT);

        if (rValue == "left")
            nBits |= WB_LEFT;
        else if (rValue == "right")
            nBits |= WB_RIGHT;
        else if (rValue == "center")
            nBits |= WB_CENTER;

        SetStyle(nBits);
    }
    else if (rKey == "yalign")
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
    else if (rKey == "wrap")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_WORDBREAK);
        if (toBool(rValue))
            nBits |= WB_WORDBREAK;
        SetStyle(nBits);
    }
    else if (rKey == "height-request")
        set_height_request(rValue.toInt32());
    else if (rKey == "width-request")
        set_width_request(rValue.toInt32());
    else if (rKey == "hexpand")
        set_hexpand(toBool(rValue));
    else if (rKey == "vexpand")
        set_vexpand(toBool(rValue));
    else if (rKey == "halign")
        set_halign(toAlign(rValue));
    else if (rKey == "valign")
        set_valign(toAlign(rValue));
    else if (rKey == "tooltip-markup")
        SetQuickHelpText(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    else if (rKey == "tooltip-text")
        SetQuickHelpText(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    else if (rKey == "border-width")
        set_border_width(rValue.toInt32());
    else if (rKey == "margin-left")
        set_margin_left(rValue.toInt32());
    else if (rKey == "margin-right")
        set_margin_right(rValue.toInt32());
    else if (rKey == "margin-top")
        set_margin_top(rValue.toInt32());
    else if (rKey == "margin-bottom")
        set_margin_bottom(rValue.toInt32());
    else if (rKey == "hscrollbar-policy")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_AUTOHSCROLL|WB_HSCROLL);
        if (rValue == "always")
            nBits |= WB_HSCROLL;
        else if (rValue == "automatic")
            nBits |= WB_AUTOHSCROLL;
        SetStyle(nBits);
    }
    else if (rKey == "vscrollbar-policy")
    {
        WinBits nBits = GetStyle();
        nBits &= ~(WB_AUTOVSCROLL|WB_VSCROLL);
        if (rValue == "always")
            nBits |= WB_VSCROLL;
        else if (rValue == "automatic")
            nBits |= WB_AUTOVSCROLL;
        SetStyle(nBits);
    }
    else if (rKey == "accessible-name")
    {
        SetAccessibleName(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    }
    else if (rKey == "accessible-description")
    {
        SetAccessibleDescription(OStringToOUString(rValue, RTL_TEXTENCODING_UTF8));
    }
    else if (rKey == "use-markup")
    {
        //https://live.gnome.org/GnomeGoals/RemoveMarkupInMessages
        SAL_WARN_IF(toBool(rValue), "vcl.layout", "Use pango attributes instead of mark-up");
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

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;

    if ( pWindowImpl->mnHeightRequest != nHeightRequest )
    {
        pWindowImpl->mnHeightRequest = nHeightRequest;
        queue_resize();
    }
}

void Window::set_width_request(sal_Int32 nWidthRequest)
{

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;

    if ( pWindowImpl->mnWidthRequest != nWidthRequest )
    {
        pWindowImpl->mnWidthRequest = nWidthRequest;
        queue_resize();
    }
}

namespace
{
    Size get_ungrouped_preferred_size(const Window &rWindow)
    {
        Size aRet(rWindow.get_width_request(), rWindow.get_height_request());
        if (aRet.Width() == -1 || aRet.Height() == -1)
        {
            Size aOptimal = rWindow.GetOptimalSize();
            if (aRet.Width() == -1)
                aRet.Width() = aOptimal.Width();
            if (aRet.Height() == -1)
                aRet.Height() = aOptimal.Height();
        }
        return aRet;
    }
}

Size Window::get_preferred_size() const
{
    Size aRet(get_ungrouped_preferred_size(*this));

    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    if (pWindowImpl->m_xSizeGroup)
    {
        const VclSizeGroupMode eMode = pWindowImpl->m_xSizeGroup->get_mode();
        if (eMode != VCL_SIZE_GROUP_NONE)
        {
            const bool bIgnoreInHidden = pWindowImpl->m_xSizeGroup->get_ignore_hidden();
            const std::set<Window*> &rWindows = pWindowImpl->m_xSizeGroup->get_widgets();
            for (std::set<Window*>::const_iterator aI = rWindows.begin(),
                aEnd = rWindows.end(); aI != aEnd; ++aI)
            {
                const Window *pOther = *aI;
                if (pOther == this)
                    continue;
                if (bIgnoreInHidden && !pOther->IsVisible())
                    continue;
                Size aOtherSize = get_ungrouped_preferred_size(*pOther);
                if (eMode == VCL_SIZE_GROUP_BOTH || eMode == VCL_SIZE_GROUP_HORIZONTAL)
                    aRet.Width() = std::max(aRet.Width(), aOtherSize.Width());
                if (eMode == VCL_SIZE_GROUP_BOTH || eMode == VCL_SIZE_GROUP_VERTICAL)
                    aRet.Height() = std::max(aRet.Height(), aOtherSize.Height());
            }
        }
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

bool Window::get_non_homogeneous() const
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    return pWindowImpl->mbNonHomogeneous;
}

void Window::set_non_homogeneous(bool bNonHomogeneous)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    pWindowImpl->mbNonHomogeneous = bNonHomogeneous;
}

void Window::add_to_size_group(boost::shared_ptr< VclSizeGroup > xGroup)
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    //To-Do, multiple groups
    pWindowImpl->m_xSizeGroup = xGroup;
    pWindowImpl->m_xSizeGroup->insert(this);
    if (VCL_SIZE_GROUP_NONE != pWindowImpl->m_xSizeGroup->get_mode())
        queue_resize();
}

void Window::remove_from_all_size_groups()
{
    WindowImpl *pWindowImpl = mpWindowImpl->mpBorderWindow ? mpWindowImpl->mpBorderWindow->mpWindowImpl : mpWindowImpl;
    //To-Do, multiple groups
    if (pWindowImpl->m_xSizeGroup)
    {
        if (VCL_SIZE_GROUP_NONE != pWindowImpl->m_xSizeGroup->get_mode())
            queue_resize();
        pWindowImpl->m_xSizeGroup->erase(this);
        pWindowImpl->m_xSizeGroup.reset();
    }
}

void Window::add_mnemonic_label(FixedText *pLabel)
{
    std::vector<FixedText*>& v = mpWindowImpl->m_aMnemonicLabels;
    if (std::find(v.begin(), v.end(), pLabel) != v.end())
        return;
    v.push_back(pLabel);
    pLabel->set_mnemonic_widget(this);
}

void Window::remove_mnemonic_label(FixedText *pLabel)
{
    std::vector<FixedText*>& v = mpWindowImpl->m_aMnemonicLabels;
    std::vector<FixedText*>::iterator aFind = std::find(v.begin(), v.end(), pLabel);
    if (aFind == v.end())
        return;
    v.erase(aFind);
    pLabel->set_mnemonic_widget(NULL);
}

std::vector<FixedText*> Window::list_mnemonic_labels() const
{
    return mpWindowImpl->m_aMnemonicLabels;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
