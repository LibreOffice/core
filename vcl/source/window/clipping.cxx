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

#include <vcl/window.hxx>
#include <vcl/virdev.hxx>

#include <sal/types.h>

#include <salobj.hxx>
#include <window.h>
#include <clipmgr.hxx>

#define IMPL_MAXSAVEBACKSIZE    (640*480)
#define IMPL_MAXALLSAVEBACKSIZE (800*600*2)

void Window::InitClipRegion()
{
    DBG_TESTSOLARMUTEX();

    ClipManager *clipMgr = ClipManager::GetInstance();

    Region  aRegion;

    // Put back backed up background
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();
    if ( mpWindowImpl->mbInPaint )
        aRegion = *(mpWindowImpl->mpPaintRegion);
    else
    {
        aRegion = *(clipMgr->GetChildClipRegion(this));
        // --- RTL -- only this region is in frame coordinates, so re-mirror it
        // the mpWindowImpl->mpPaintRegion above is already correct (see ImplCallPaint()) !
        if( ImplIsAntiparallel() )
            ReMirror ( aRegion );
    }
    if ( mbClipRegion )
        aRegion.Intersect( ImplPixelToDevicePixel( maRegion ) );
    if ( aRegion.IsEmpty() )
        mbOutputClipped = true;
    else
    {
        mbOutputClipped = false;
        SelectClipRegion( aRegion );
    }
    mbClipRegionSet = true;

    mbInitClipRegion = false;
}

void Window::SetParentClipMode( sal_uInt16 nMode )
{
    ClipManager *clipMgr = ClipManager::GetInstance();

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetParentClipMode( nMode );
    else
    {
        if ( !clipMgr->IsOverlapWindow( this ) )
        {
            mpWindowImpl->mnParentClipMode = nMode;
            if ( nMode & PARENTCLIPMODE_CLIP )
                mpWindowImpl->mpParent->mpWindowImpl->mbClipChildren = true;
        }
    }
}

sal_uInt16 Window::GetParentClipMode() const
{

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->GetParentClipMode();
    else
        return mpWindowImpl->mnParentClipMode;
}

void Window::ExpandPaintClipRegion( const Region& rRegion )
{
    ClipManager *clipMgr = ClipManager::GetInstance();

    if( mpWindowImpl->mpPaintRegion )
    {
        Region aPixRegion = LogicToPixel( rRegion );
        Region aDevPixRegion = ImplPixelToDevicePixel( aPixRegion );

        Region aWinChildRegion = *clipMgr->GetChildClipRegion(this);
        // --- RTL -- only this region is in frame coordinates, so re-mirror it
        if( ImplIsAntiparallel() )
        {
            const OutputDevice *pOutDev = GetOutDev();
            pOutDev->ReMirror( aWinChildRegion );
        }

        aDevPixRegion.Intersect( aWinChildRegion );
        if( ! aDevPixRegion.IsEmpty() )
        {
            mpWindowImpl->mpPaintRegion->Union( aDevPixRegion );
            mbInitClipRegion = true;
        }
    }
}

Region Window::GetWindowClipRegionPixel( sal_uInt16 nFlags ) const
{
    ClipManager *clipMgr = ClipManager::GetInstance();

    Region aWinClipRegion;

    if ( nFlags & WINDOW_GETCLIPREGION_NOCHILDREN )
    {
        if ( mpWindowImpl->mbInitWinClipRegion )
            clipMgr->InitClipRegion(const_cast<Window *>(this));
        aWinClipRegion = mpWindowImpl->maWinClipRegion;
    }
    else
    {
        Region* pWinChildClipRegion = clipMgr->GetChildClipRegion(const_cast<Window*>(this));
        aWinClipRegion = *pWinChildClipRegion;
        // --- RTL --- remirror clip region before passing it to somebody
        if( ImplIsAntiparallel() )
        {
            const OutputDevice *pOutDev = GetOutDev();
            pOutDev->ReMirror( aWinClipRegion );
        }
    }

    if ( nFlags & WINDOW_GETCLIPREGION_NULL )
    {
        Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aWinRegion( aWinRect );

        if ( aWinRegion == aWinClipRegion )
            aWinClipRegion.SetNull();
    }

    aWinClipRegion.Move( -mnOutOffX, -mnOutOffY );

    return aWinClipRegion;
}


Region Window::GetActiveClipRegion() const
{
    Region aRegion(true);

    if ( mpWindowImpl->mbInPaint )
    {
        aRegion = *(mpWindowImpl->mpPaintRegion);
        aRegion.Move( -mnOutOffX, -mnOutOffY );
    }

    if ( mbClipRegion )
        aRegion.Intersect( maRegion );

    return PixelToLogic( aRegion );
}

void Window::ClipToPaintRegion(Rectangle& rDstRect)
{
    const Region aPaintRgn(GetPaintRegion());

    if (!aPaintRgn.IsNull())
        rDstRect.Intersection(LogicToPixel(aPaintRgn.GetBoundRect()));
}

void Window::EnableClipSiblings( bool bClipSiblings )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->EnableClipSiblings( bClipSiblings );

    mpWindowImpl->mbClipSiblings = bClipSiblings;
}

void Window::ImplClipAllChildren( Region& rRegion )
{
    ClipManager *clipMgr = ClipManager::GetInstance();

    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
            clipMgr->Exclude( pWindow, rRegion );
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

bool Window::ImplSysObjClip( const Region* pOldRegion )
{
    ClipManager *clipMgr = ClipManager::GetInstance();

    bool bUpdate = true;

    if ( mpWindowImpl->mpSysObj )
    {
        bool bVisibleState = mpWindowImpl->mbReallyVisible;

        if ( bVisibleState )
        {
            Region* pWinChildClipRegion = clipMgr->GetChildClipRegion(this);

            if ( !pWinChildClipRegion->IsEmpty() )
            {
                if ( pOldRegion )
                {
                    Region aNewRegion = *pWinChildClipRegion;
                    pWinChildClipRegion->Intersect( *pOldRegion );
                    bUpdate = aNewRegion == *pWinChildClipRegion;
                }

                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();

                Region      aRegion = *pWinChildClipRegion;
                Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Region      aWinRectRegion( aWinRect );
                sal_uInt16      nClipFlags = mpWindowImpl->mpSysObj->GetClipRegionType();

                if ( aRegion == aWinRectRegion )
                    mpWindowImpl->mpSysObj->ResetClipRegion();
                else
                {
                    if ( nClipFlags & SAL_OBJECT_CLIP_EXCLUDERECTS )
                    {
                        aWinRectRegion.Exclude( aRegion );
                        aRegion = aWinRectRegion;
                    }
                    if ( !(nClipFlags & SAL_OBJECT_CLIP_ABSOLUTE) )
                        aRegion.Move( -mnOutOffX, -mnOutOffY );

                    // set/update clip region
                    RectangleVector aRectangles;
                    aRegion.GetRegionRectangles(aRectangles);
                    mpWindowImpl->mpSysObj->BeginSetClipRegion(aRectangles.size());

                    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
                    {
                        mpWindowImpl->mpSysObj->UnionClipRegion(
                            aRectIter->Left(),
                            aRectIter->Top(),
                            aRectIter->GetWidth(),   // orig nWidth was ((R - L) + 1), same as GetWidth does
                            aRectIter->GetHeight()); // same for height
                    }

                    mpWindowImpl->mpSysObj->EndSetClipRegion();
                }
            }
            else
                bVisibleState = false;
        }

        // update visible status
        mpWindowImpl->mpSysObj->Show( bVisibleState );
    }

    return bUpdate;
}

void Window::ImplUpdateSysObjChildrenClip()
{
    if ( mpWindowImpl->mpSysObj && mpWindowImpl->mbInitWinClipRegion )
        ImplSysObjClip( NULL );

    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjChildrenClip();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplUpdateSysObjOverlapsClip()
{
    ImplUpdateSysObjChildrenClip();

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        pWindow->ImplUpdateSysObjOverlapsClip();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplUpdateSysObjClip()
{
    ClipManager *clipMgr = ClipManager::GetInstance();

    if ( !clipMgr->IsOverlapWindow( this ) )
    {
        ImplUpdateSysObjChildrenClip();

        // siblings should recalculate their clip region
        if ( mpWindowImpl->mbClipSiblings )
        {
            Window* pWindow = mpWindowImpl->mpNext;
            while ( pWindow )
            {
                pWindow->ImplUpdateSysObjChildrenClip();
                pWindow = pWindow->mpWindowImpl->mpNext;
            }
        }
    }
    else
        mpWindowImpl->mpFrameWindow->ImplUpdateSysObjOverlapsClip();
}

bool Window::ImplSetClipFlagChildren( bool bSysObjOnlySmaller )
{
    bool bUpdate = true;
    if ( mpWindowImpl->mpSysObj )
    {
        Region* pOldRegion = NULL;
        if ( bSysObjOnlySmaller && !mpWindowImpl->mbInitWinClipRegion )
            pOldRegion = new Region( mpWindowImpl->maWinClipRegion );

        mbInitClipRegion = true;
        mpWindowImpl->mbInitWinClipRegion = true;

        Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
                bUpdate = false;
            pWindow = pWindow->mpWindowImpl->mpNext;
        }

        if ( !ImplSysObjClip( pOldRegion ) )
        {
            mbInitClipRegion = true;
            mpWindowImpl->mbInitWinClipRegion = true;
            bUpdate = false;
        }

        delete pOldRegion;
    }
    else
    {
    mbInitClipRegion = true;
    mpWindowImpl->mbInitWinClipRegion = true;

    Window* pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
            bUpdate = false;
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
    }
    return bUpdate;
}

bool Window::ImplSetClipFlagOverlapWindows( bool bSysObjOnlySmaller )
{
    bool bUpdate = ImplSetClipFlagChildren( bSysObjOnlySmaller );

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( !pWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller ) )
            bUpdate = false;
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    return bUpdate;
}

bool Window::ImplSetClipFlag( bool bSysObjOnlySmaller )
{
    ClipManager *clipMgr = ClipManager::GetInstance();

    if ( !clipMgr->IsOverlapWindow( this ) )
    {
        bool bUpdate = ImplSetClipFlagChildren( bSysObjOnlySmaller );

        Window* pParent = ImplGetParent();
        if ( pParent &&
             ((pParent->GetStyle() & WB_CLIPCHILDREN) || (mpWindowImpl->mnParentClipMode & PARENTCLIPMODE_CLIP)) )
        {
            pParent->mbInitClipRegion = true;
            pParent->mpWindowImpl->mbInitChildRegion = true;
        }

        // siblings should recalculate their clip region
        if ( mpWindowImpl->mbClipSiblings )
        {
            Window* pWindow = mpWindowImpl->mpNext;
            while ( pWindow )
            {
                if ( !pWindow->ImplSetClipFlagChildren( bSysObjOnlySmaller ) )
                    bUpdate = false;
                pWindow = pWindow->mpWindowImpl->mpNext;
            }
        }

        return bUpdate;
    }
    else
        return mpWindowImpl->mpFrameWindow->ImplSetClipFlagOverlapWindows( bSysObjOnlySmaller );
}

void Window::ImplIntersectWindowRegion( Region& rRegion )
{
    rRegion.Intersect( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                  Size( mnOutWidth, mnOutHeight ) ) );
    if ( mpWindowImpl->mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
}

void Window::ImplIntersectAndUnionOverlapWindows( const Region& rInterRegion, Region& rRegion )
{
    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
        {
            Region aTempRegion( rInterRegion );
            pWindow->ImplIntersectWindowRegion( aTempRegion );
            rRegion.Union( aTempRegion );
            pWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
        }

        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplIntersectAndUnionOverlapWindows2( const Region& rInterRegion, Region& rRegion )
{
    if ( mpWindowImpl->mbReallyVisible )
    {
        Region aTempRegion( rInterRegion );
        ImplIntersectWindowRegion( aTempRegion );
        rRegion.Union( aTempRegion );
    }

    ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
}

void Window::ImplCalcOverlapRegionOverlaps( const Region& rInterRegion, Region& rRegion )
{
    ClipManager *clipMgr = ClipManager::GetInstance();

    // Clip Overlap Siblings
    Window* pStartOverlapWindow;
    if ( !clipMgr->IsOverlapWindow( this ) )
        pStartOverlapWindow = mpWindowImpl->mpOverlapWindow;
    else
        pStartOverlapWindow = this;
    while ( !pStartOverlapWindow->mpWindowImpl->mbFrame )
    {
        Window* pOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
        while ( pOverlapWindow && (pOverlapWindow != pStartOverlapWindow) )
        {
            pOverlapWindow->ImplIntersectAndUnionOverlapWindows2( rInterRegion, rRegion );
            pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
        }
        pStartOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow;
    }

    // Clip Child Overlap Windows
    if ( !clipMgr->IsOverlapWindow( this ) )
        mpWindowImpl->mpOverlapWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
    else
        ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
}

void Window::ImplCalcOverlapRegion( const Rectangle& rSourceRect, Region& rRegion,
                                    bool bChildren, bool bParent, bool bSiblings )
{
    ClipManager *clipMgr = ClipManager::GetInstance();

    Region  aRegion( rSourceRect );
    if ( mpWindowImpl->mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
    Region  aTempRegion;
    Window* pWindow;

    ImplCalcOverlapRegionOverlaps( aRegion, rRegion );

    // Parent-Boundaries
    if ( bParent )
    {
        pWindow = this;
        if ( !clipMgr->IsOverlapWindow( this ) )
        {
            pWindow = ImplGetParent();
            do
            {
                aTempRegion = aRegion;
                clipMgr->Exclude( pWindow, aTempRegion );
                rRegion.Union( aTempRegion );
                if ( clipMgr->IsOverlapWindow( pWindow ) )
                    break;
                pWindow = pWindow->ImplGetParent();
            }
            while ( pWindow );
        }
        if ( pWindow && !pWindow->mpWindowImpl->mbFrame )
        {
            aTempRegion = aRegion;
            aTempRegion.Exclude( Rectangle( Point( 0, 0 ), Size( mpWindowImpl->mpFrameWindow->mnOutWidth, mpWindowImpl->mpFrameWindow->mnOutHeight ) ) );
            rRegion.Union( aTempRegion );
        }
    }

    // Siblings
    if ( bSiblings && !clipMgr->IsOverlapWindow( this ) )
    {
        pWindow = mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild;
        do
        {
            if ( pWindow->mpWindowImpl->mbReallyVisible && (pWindow != this) )
            {
                aTempRegion = aRegion;
                pWindow->ImplIntersectWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
            }
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
        while ( pWindow );
    }

    if ( bChildren )
    {
        pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            if ( pWindow->mpWindowImpl->mbReallyVisible )
            {
                aTempRegion = aRegion;
                pWindow->ImplIntersectWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
            }
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }
}

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

void Window::ImplSaveOverlapBackground()
{
    DBG_ASSERT( !mpWindowImpl->mpOverlapData->mpSaveBackDev, "Window::ImplSaveOverlapBackground() - Background already saved" );

    ClipManager *clipMgr = ClipManager::GetInstance();

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
                        clipMgr->InitClipRegion(this);

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
    ClipManager *clipMgr = ClipManager::GetInstance();

    if ( mpWindowImpl->mpOverlapData->mpSaveBackDev )
    {
        if ( mpWindowImpl->mbInitWinClipRegion )
            clipMgr->InitClipRegion(this);

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
