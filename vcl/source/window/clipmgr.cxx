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
#include <vcl/outdev.hxx>
#include <vcl/clipmgr.hxx>

#include <sal/types.h>

#include <salobj.hxx>
#include <window.h>

#define IMPL_MAXSAVEBACKSIZE    (640*480)
#define IMPL_MAXALLSAVEBACKSIZE (800*600*2)


bool ClipManager::instanceFlag = false;
ClipManager* ClipManager::single = NULL;

ClipManager* ClipManager::GetInstance()
{
    if (!instanceFlag)
    {
        single = new ClipManager();
        instanceFlag = true;
        return single;
    }
    else
    {
        return single;
    }
}

void ClipManager::SetParentClipMode( Window* pWindow, sal_uInt16 nMode )
{
    if ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow->mpWindowImpl->mpBorderWindow->SetParentClipMode( nMode );
    else
    {
        if ( IsOverlapWindow( pWindow ) )
        {
            pWindow->mpWindowImpl->mnParentClipMode = nMode;
            if ( nMode & PARENTCLIPMODE_CLIP )
                pWindow->mpWindowImpl->mpParent->mpWindowImpl->mbClipChildren = true;
        }
    }
}

sal_uInt16 ClipManager::GetParentClipMode( Window* pWindow ) const
{

    if ( pWindow->mpWindowImpl->mpBorderWindow )
        return GetParentClipMode( pWindow->mpWindowImpl->mpBorderWindow );
    else
        return pWindow->mpWindowImpl->mnParentClipMode;
}

bool ClipManager::SetClipFlagChildren( Window *pWindow, bool bSysObjOnlySmaller )
{
    bool bUpdate = true;
    if ( pWindow->mpWindowImpl->mpSysObj )
    {
        Region* pOldRegion = NULL;
        if ( bSysObjOnlySmaller && !pWindow->mpWindowImpl->mbInitWinClipRegion )
            pOldRegion = new Region( pWindow->mpWindowImpl->maWinClipRegion );

        pWindow->mbInitClipRegion = true;
        pWindow->mpWindowImpl->mbInitWinClipRegion = true;

        Window* pChildWindow = pWindow->mpWindowImpl->mpFirstChild;
        while ( pChildWindow )
        {
            if ( !SetClipFlagChildren( pChildWindow, bSysObjOnlySmaller ) )
                bUpdate = false;
            pChildWindow = pChildWindow->mpWindowImpl->mpNext;
        }

        if ( !clipSysObject( pWindow, pOldRegion ) )
        {
            pWindow->mbInitClipRegion = true;
            pWindow->mpWindowImpl->mbInitWinClipRegion = true;
            bUpdate = false;
        }

        delete pOldRegion;
    }
    else
    {
        pWindow->mbInitClipRegion = true;
        pWindow->mpWindowImpl->mbInitWinClipRegion = true;

        Window* pChildWindow = pWindow->mpWindowImpl->mpFirstChild;
        while ( pChildWindow )
        {
            if ( !SetClipFlagChildren( pChildWindow, bSysObjOnlySmaller ) )
                bUpdate = false;
            pChildWindow = pChildWindow->mpWindowImpl->mpNext;
        }
    }
    return bUpdate;
}

bool ClipManager::SetClipFlagOverlapWindows( Window *pWindow, bool bSysObjOnlySmaller )
{
    bool bUpdate = SetClipFlagChildren( pWindow, bSysObjOnlySmaller );

    Window* pOverlapWindow = pWindow->mpWindowImpl->mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( !SetClipFlagOverlapWindows( pOverlapWindow, bSysObjOnlySmaller ) )
            bUpdate = false;
        pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
    }

    return bUpdate;
}

bool ClipManager::SetClipFlag( Window *pWindow, bool bSysObjOnlySmaller )
{
    if ( !IsOverlapWindow( pWindow ) )
    {
        bool bUpdate = SetClipFlagChildren( pWindow, bSysObjOnlySmaller );

        Window* pParent = pWindow->ImplGetParent();
        if ( pParent &&
             ((pParent->GetStyle() & WB_CLIPCHILDREN) || (pWindow->mpWindowImpl->mnParentClipMode & PARENTCLIPMODE_CLIP)) )
        {
            pParent->mbInitClipRegion = true;
            pParent->mpWindowImpl->mbInitChildRegion = true;
        }

        // siblings should recalculate their clip region
        if ( pWindow->mpWindowImpl->mbClipSiblings )
        {
            Window* pNextWindow = pWindow->mpWindowImpl->mpNext;
            while ( pNextWindow )
            {
                if ( !SetClipFlagChildren( pNextWindow, bSysObjOnlySmaller ) )
                    bUpdate = false;
                pNextWindow = pNextWindow->mpWindowImpl->mpNext;
            }
        }

        return bUpdate;
    }
    else
    {
        return SetClipFlagOverlapWindows( pWindow->mpWindowImpl->mpFrameWindow, bSysObjOnlySmaller );
    }
}

void ClipManager::EnableClipSiblings( Window *pWindow, bool bClipSiblings )
{

    if ( pWindow->mpWindowImpl->mpBorderWindow )
        EnableClipSiblings( pWindow->mpWindowImpl->mpBorderWindow, bClipSiblings );

    pWindow->mpWindowImpl->mbClipSiblings = bClipSiblings;
}

void ClipManager::Init( Window *pWindow )
{
    Region  aRegion;

    // Put back backed up background
    if ( pWindow->mpWindowImpl->mpFrameData->mpFirstBackWin )
        InvalidateAllOverlapBackgrounds( pWindow );
    if ( pWindow->mpWindowImpl->mbInPaint )
        aRegion = *(pWindow->mpWindowImpl->mpPaintRegion);
    else
    {
        aRegion = *(GetChildClipRegion( pWindow ));
        // --- RTL -- only this region is in frame coordinates, so re-mirror it
        // the mpWindowImpl->mpPaintRegion above is already correct (see ImplCallPaint()) !
        if( pWindow->ImplIsAntiparallel() )
            pWindow->ReMirror ( aRegion );
    }
    if ( pWindow->mbClipRegion )
        aRegion.Intersect( pWindow->ImplPixelToDevicePixel( pWindow->maRegion ) );
    if ( aRegion.IsEmpty() )
        pWindow->mbOutputClipped = true;
    else
    {
        pWindow->mbOutputClipped = false;
        pWindow->SelectClipRegion( aRegion );
    }
    pWindow->mbClipRegionSet = true;

    pWindow->mbInitClipRegion = false;
}

void ClipManager::InitClipRegion( Window *pWindow )
{
    // Build Window region
    pWindow->mpWindowImpl->maWinClipRegion = Rectangle( Point( pWindow->GetOutOffXPixel(), pWindow->GetOutOffYPixel() ),
                                                        pWindow->GetOutputSizePixel() );
    if ( pWindow->mpWindowImpl->mbWinRegion )
        pWindow->mpWindowImpl->maWinClipRegion.Intersect( pWindow->ImplPixelToDevicePixel( pWindow->mpWindowImpl->maWinRegion ) );

    if ( pWindow->mpWindowImpl->mbClipSiblings && !IsOverlapWindow( pWindow ) )
        clipSiblings( pWindow, pWindow->mpWindowImpl->maWinClipRegion );

    // Clip Parent Boundaries
    ClipBoundaries( pWindow, pWindow->mpWindowImpl->maWinClipRegion, false, true );

    // Clip Children
    if ( (pWindow->GetStyle() & WB_CLIPCHILDREN) || pWindow->mpWindowImpl->mbClipChildren )
        pWindow->mpWindowImpl->mbInitChildRegion = true;

    pWindow->mpWindowImpl->mbInitWinClipRegion = false;
}

bool ClipManager::IsOverlapWindow( Window* pWindow ) const
{
    return pWindow->mpWindowImpl->mbOverlapWin;
}

void ClipManager::ClipBoundaries( Window* pWindow, Region& rRegion, bool bThis, bool bOverlaps )
{
    if ( bThis )
    {
        intersectClipRegion( pWindow, rRegion );
    }
    else if ( IsOverlapWindow( pWindow ) )
    {
        // clip to frame if required
        if ( !pWindow->mpWindowImpl->mbFrame )
            rRegion.Intersect( Rectangle( Point( 0, 0 ), pWindow->GetOutputSizePixel() ) );

        if ( bOverlaps && !rRegion.IsEmpty() )
        {
            // Clip Overlap Siblings
            Window* pStartOverlapWindow = pWindow;
            while ( !pStartOverlapWindow->mpWindowImpl->mbFrame )
            {
                Window* pOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
                while ( pOverlapWindow && (pOverlapWindow != pStartOverlapWindow) )
                {
                    if ( pWindow->mpWindowImpl->mbReallyVisible )
                        Exclude( pWindow, rRegion );

                    excludeOverlapWindows( pWindow, rRegion );
                    pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
                }
                pStartOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow;
            }

            // Clip child overlap windows
            excludeOverlapWindows( pWindow, rRegion );
        }
    }
    else
    {
        intersectClipRegion( pWindow->ImplGetParent(), rRegion );
    }
}

bool ClipManager::ClipChildren( Window *pWindow, Region& rRegion )
{
    bool    bOtherClip = false;
    Window* pChildWindow = pWindow->mpWindowImpl->mpFirstChild;
    while ( pChildWindow )
    {
        if ( pChildWindow->mpWindowImpl->mbReallyVisible )
        {
            // read-out ParentClipMode-Flags
            sal_uInt16 nClipMode = pChildWindow->GetParentClipMode();

            if ( !(nClipMode & PARENTCLIPMODE_NOCLIP) &&
                 ((nClipMode & PARENTCLIPMODE_CLIP) || (pWindow->GetStyle() & WB_CLIPCHILDREN)) )
            {
                Exclude( pChildWindow, rRegion );
            }
            else
            {
                bOtherClip = true;
            }
        }

        pChildWindow = pChildWindow->mpWindowImpl->mpNext;
    }

    return bOtherClip;
}

void ClipManager::ClipAllChildren( Window *pWindow, Region& rRegion )
{
    Window* pChildWindow = pWindow->mpWindowImpl->mpFirstChild;
    while ( pChildWindow )
    {
        if ( pChildWindow->mpWindowImpl->mbReallyVisible )
            Exclude( pWindow, rRegion );
        pChildWindow = pChildWindow->mpWindowImpl->mpNext;
    }
}

Region* ClipManager::GetChildClipRegion( Window* pWindow )
{
    if ( pWindow->mpWindowImpl->mbInitWinClipRegion )
        InitClipRegion( pWindow );
    if ( pWindow->mpWindowImpl->mbInitChildRegion )
        initChildClipRegion( pWindow );
    if ( pWindow->mpWindowImpl->mpChildClipRegion )
        return pWindow->mpWindowImpl->mpChildClipRegion;
    else
        return &pWindow->mpWindowImpl->maWinClipRegion;
}

void ClipManager::SaveBackground( Window *pWindow, const Point& rPos, const Size& rSize,
                                  const Point& rDestOff, VirtualDevice& rSaveDevice )
{
    if ( pWindow->mpWindowImpl->mpPaintRegion )
    {
        Region aClip( *pWindow->mpWindowImpl->mpPaintRegion );
        const Point aPixPos( pWindow->LogicToPixel( rPos ) );

        aClip.Move( -pWindow->mnOutOffX, -pWindow->mnOutOffY );
        aClip.Intersect( Rectangle( aPixPos, pWindow->LogicToPixel( rSize ) ) );

        if ( !aClip.IsEmpty() )
        {
            const Region aOldClip( rSaveDevice.GetClipRegion() );
            const Point aPixOffset( rSaveDevice.LogicToPixel( rDestOff ) );
            const bool bMap = rSaveDevice.IsMapModeEnabled();

            // move clip region to have the same distance to DestOffset
            aClip.Move( aPixOffset.X() - aPixPos.X(), aPixOffset.Y() - aPixPos.Y() );

            // set pixel clip region
            rSaveDevice.EnableMapMode( false );
            rSaveDevice.SetClipRegion( aClip );
            rSaveDevice.EnableMapMode( bMap );
            rSaveDevice.DrawOutDev( rDestOff, rSize, rPos, rSize, *pWindow );
            rSaveDevice.SetClipRegion( aOldClip );
        }
    }
    else
    {
        rSaveDevice.DrawOutDev( rDestOff, rSize, rPos, rSize, *pWindow );
    }
}

void ClipManager::SaveOverlapBackground( Window *pWindow )
{
    DBG_ASSERT( !pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev, "ClipManger::SaveOverlapBackground() - Background already saved" );

    if ( !pWindow->mpWindowImpl->mbFrame )
    {
        sal_uLong nSaveBackSize = pWindow->mnOutWidth * pWindow->mnOutHeight;
        if ( nSaveBackSize <= IMPL_MAXSAVEBACKSIZE )
        {
            if ( nSaveBackSize + pWindow->mpWindowImpl->mpFrameData->mnAllSaveBackSize <= IMPL_MAXALLSAVEBACKSIZE )
            {
                Size aOutSize = pWindow->GetOutputSizePixel();

                pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev = new VirtualDevice( *pWindow->mpWindowImpl->mpFrameWindow );
                if ( pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev->SetOutputSizePixel( aOutSize ) )
                {
                    pWindow->mpWindowImpl->mpFrameWindow->ImplUpdateAll();

                    if ( pWindow->mpWindowImpl->mbInitWinClipRegion )
                        InitClipRegion( pWindow );

                    pWindow->mpWindowImpl->mpOverlapData->mnSaveBackSize = nSaveBackSize;
                    pWindow->mpWindowImpl->mpFrameData->mnAllSaveBackSize += nSaveBackSize;

                    Point aDevPt;
                    OutputDevice *pOutDev = pWindow->mpWindowImpl->mpFrameWindow->GetOutDev();
                    pOutDev->ImplGetFrameDev( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                              aDevPt, aOutSize,
                                              *(pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev) );
                    pWindow->mpWindowImpl->mpOverlapData->mpNextBackWin = pWindow->mpWindowImpl->mpFrameData->mpFirstBackWin;
                    pWindow->mpWindowImpl->mpFrameData->mpFirstBackWin = pWindow;
                }
                else
                {
                    delete pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev;
                    pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev = NULL;
                }
            }
        }
    }
}

bool ClipManager::RestoreOverlapBackground( Window *pWindow, Region& rInvRegion )
{
    if ( pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev )
    {
        if ( pWindow->mpWindowImpl->mbInitWinClipRegion )
            InitClipRegion( pWindow );

        if ( pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev )
        {
            Point aDevPt;
            Point aDestPt( pWindow->mnOutOffX, pWindow->mnOutOffY );
            Size aDevSize = pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev->GetOutputSizePixel();

            OutputDevice *pOutDev = pWindow->mpWindowImpl->mpFrameWindow->GetOutDev();

            if ( pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn )
            {
                pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn->Intersect( pWindow->mpWindowImpl->maWinClipRegion );
                rInvRegion = pWindow->mpWindowImpl->maWinClipRegion;
                rInvRegion.Exclude( *pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn );
                pOutDev->ImplDrawFrameDev( aDestPt, aDevPt, aDevSize,
                                           *(pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev),
                                           *pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn );
            }
            else
            {
                pOutDev->ImplDrawFrameDev( aDestPt, aDevPt, aDevSize,
                                           *(pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev),
                                           pWindow->mpWindowImpl->maWinClipRegion );
            }
            DeleteOverlapBackground( pWindow );
        }

        return true;
    }

    return false;
}

void ClipManager::DeleteOverlapBackground( Window *pWindow )
{
    if ( pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev )
    {
        pWindow->mpWindowImpl->mpFrameData->mnAllSaveBackSize -= pWindow->mpWindowImpl->mpOverlapData->mnSaveBackSize;
        delete pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev;
        pWindow->mpWindowImpl->mpOverlapData->mpSaveBackDev = NULL;
        if ( pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn )
        {
            delete pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn;
            pWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn = NULL;
        }

        // remove window from the list
        if ( pWindow->mpWindowImpl->mpFrameData->mpFirstBackWin == pWindow )
            pWindow->mpWindowImpl->mpFrameData->mpFirstBackWin = pWindow->mpWindowImpl->mpOverlapData->mpNextBackWin;
        else
        {
            Window* pTemp = pWindow->mpWindowImpl->mpFrameData->mpFirstBackWin;
            while ( pTemp->mpWindowImpl->mpOverlapData->mpNextBackWin != pWindow )
                pTemp = pTemp->mpWindowImpl->mpOverlapData->mpNextBackWin;
            pTemp->mpWindowImpl->mpOverlapData->mpNextBackWin = pWindow->mpWindowImpl->mpOverlapData->mpNextBackWin;
        }
        pWindow->mpWindowImpl->mpOverlapData->mpNextBackWin = NULL;
    }
}

void ClipManager::InvalidateAllOverlapBackgrounds( Window *pWindow )
{
    Window* pBackWindow = pWindow->mpWindowImpl->mpFrameData->mpFirstBackWin;
    while ( pBackWindow )
    {
        // remember next window here already, as this window could
        // be removed within the next if clause from the list
        Window* pNext = pBackWindow->mpWindowImpl->mpOverlapData->mpNextBackWin;

        if ( pWindow->ImplIsWindowInFront( pBackWindow ) )
        {
            Rectangle aRect1( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                              pWindow->GetOutputSizePixel() );
            Rectangle aRect2( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                              pBackWindow->GetOutputSizePixel() );
            aRect1.Intersection( aRect2 );
            if ( !aRect1.IsEmpty() )
            {
                if ( !pBackWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn )
                    pBackWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn = new Region( aRect2 );
                pBackWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn->Exclude( aRect1 );
                if ( pBackWindow->mpWindowImpl->mpOverlapData->mpSaveBackRgn->IsEmpty() )
                    DeleteOverlapBackground( pBackWindow );
            }

        }

        pBackWindow = pNext;
    }
}


void ClipManager::CalcOverlapRegion( Window *pWindow,
                                     const Rectangle& rSourceRect, Region& rRegion,
                                     bool bChildren, bool bParent, bool bSiblings )
{
    Region aRegion( rSourceRect );

    if ( pWindow->mpWindowImpl->mbWinRegion )
        rRegion.Intersect( pWindow->ImplPixelToDevicePixel( pWindow->mpWindowImpl->maWinRegion ) );

    CalcOverlapRegionOverlaps( pWindow, aRegion, rRegion );

    // Parent-Boundaries
    if ( bParent )
    {
        Window *pParentWindow = pWindow;
        if ( !IsOverlapWindow( pWindow ) )
        {
            pParentWindow = pWindow->ImplGetParent();
            do
            {
                Region aExcludedRegion = aRegion;
                Exclude( pParentWindow, aExcludedRegion );
                rRegion.Union( aExcludedRegion );
                if ( IsOverlapWindow( pParentWindow ) )
                    break;
                pParentWindow = pParentWindow->ImplGetParent();
            }
            while ( pParentWindow );
        }
        if ( pParentWindow && !pParentWindow->mpWindowImpl->mbFrame )
        {
            Region aExcludedRegion( aRegion );
            aExcludedRegion.Exclude( Rectangle( Point( 0, 0 ), pWindow->GetOutputSizePixel() ) );
            rRegion.Union( aExcludedRegion );
        }
    }

    // Siblings
    if ( bSiblings && !IsOverlapWindow( pWindow ) )
    {
        Window *pNextWindow = pWindow->mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild;
        do
        {
            if ( pNextWindow->mpWindowImpl->mbReallyVisible && (pNextWindow != pWindow) )
            {
                Region aIntersectRegion( aRegion );
                Intersect( pNextWindow, aIntersectRegion );
                rRegion.Union( aIntersectRegion );
            }
            pNextWindow = pNextWindow->mpWindowImpl->mpNext;
        }
        while ( pNextWindow );
    }

    if ( bChildren )
    {
        Window *pChildWindow = pWindow->mpWindowImpl->mpFirstChild;
        while ( pChildWindow )
        {
            if ( pChildWindow->mpWindowImpl->mbReallyVisible )
            {
                Region aIntersectRegion( aRegion );
                Intersect( pChildWindow, aIntersectRegion );
                rRegion.Union( aIntersectRegion );
            }
            pChildWindow = pChildWindow->mpWindowImpl->mpNext;
        }
    }
}

void ClipManager::CalcOverlapRegionOverlaps( Window* pWindow, const Region& rInterRegion, Region& rRegion )
{
    // Clip Overlap Siblings
    Window* pStartOverlapWindow;
    if ( !IsOverlapWindow( pWindow ) )
        pStartOverlapWindow = pWindow->mpWindowImpl->mpOverlapWindow;
    else
        pStartOverlapWindow = pWindow;
    while ( !pStartOverlapWindow->mpWindowImpl->mbFrame )
    {
        Window* pOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
        while ( pOverlapWindow && (pOverlapWindow != pStartOverlapWindow) )
        {
            if ( pOverlapWindow->mpWindowImpl->mbReallyVisible )
            {
                Region aIntersectRegion( rInterRegion );
                Intersect( pWindow, aIntersectRegion );
                rRegion.Union( aIntersectRegion );
            }

            intersectAndUnionOverlapWindows( pOverlapWindow, rInterRegion, rRegion );
            pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
        }
        pStartOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow;
    }

    // Clip Child Overlap Windows
    if ( !IsOverlapWindow( pWindow ) )
        intersectAndUnionOverlapWindows( pWindow->mpWindowImpl->mpOverlapWindow, rInterRegion, rRegion );
    else
        intersectAndUnionOverlapWindows( pWindow, rInterRegion, rRegion );
}


void ClipManager::Intersect( Window* pWindow, Region& rRegion )
{
    rRegion.Intersect( Rectangle( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                  pWindow->GetOutputSizePixel() ) );

    if ( pWindow->mpWindowImpl->mbWinRegion )
        rRegion.Intersect( pWindow->ImplPixelToDevicePixel( pWindow->mpWindowImpl->maWinRegion ) );
}

void ClipManager::Exclude( Window *pWindow, Region& rRegion )
{
    if ( pWindow->mpWindowImpl->mbWinRegion )
    {
        Region aRegion( Rectangle( Point ( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                   pWindow->GetOutputSizePixel() ) );
        aRegion.Intersect( pWindow->ImplPixelToDevicePixel( pWindow->mpWindowImpl->maWinRegion ) );
        rRegion.Exclude( aRegion );
    }
    else
    {
        rRegion.Exclude( Rectangle( Point ( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                    pWindow->GetOutputSizePixel() ) );
    }
}

Region ClipManager::GetActiveClipRegion( const Window *pWindow ) const
{
    Region aRegion(true);

    if ( pWindow->mpWindowImpl->mbInPaint )
    {
        aRegion = *(pWindow->mpWindowImpl->mpPaintRegion);
        aRegion.Move( -pWindow->mnOutOffX, -pWindow->mnOutOffY );
    }

    if ( pWindow->mbClipRegion )
        aRegion.Intersect( pWindow->maRegion );

    return pWindow->PixelToLogic( aRegion );
}

void ClipManager::ClipToPaintRegion( Window *pWindow, Rectangle& rDstRect )
{
    const Region aPaintRgn( pWindow->GetPaintRegion() );

    if (!aPaintRgn.IsNull())
        rDstRect.Intersection( pWindow->LogicToPixel( aPaintRgn.GetBoundRect() ) );
}

void ClipManager::ExpandPaintClipRegion( Window *pWindow, const Region& rRegion )
{
    if( pWindow->mpWindowImpl->mpPaintRegion )
    {
        Region aPixRegion = pWindow->LogicToPixel( rRegion );
        Region aDevPixRegion = pWindow->ImplPixelToDevicePixel( aPixRegion );

        Region aWinChildRegion = *GetChildClipRegion( pWindow );
        // --- RTL -- only this region is in frame coordinates, so re-mirror it
        if( pWindow->ImplIsAntiparallel() )
        {
            pWindow->ReMirror( aWinChildRegion );
        }

        aDevPixRegion.Intersect( aWinChildRegion );
        if( ! aDevPixRegion.IsEmpty() )
        {
            pWindow->mpWindowImpl->mpPaintRegion->Union( aDevPixRegion );
            pWindow->mbInitClipRegion = true;
        }
    }
}

bool ClipManager::ClipCoversWholeWindow( Window *pWindow )
{
    bool bCoversWholeWindow = false;

    if ( pWindow->mpWindowImpl->mbInitWinClipRegion )
        InitClipRegion( pWindow );

    Region aWinClipRegion = pWindow->mpWindowImpl->maWinClipRegion;
    Region aWinRegion( Rectangle ( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ), pWindow->GetOutputSizePixel() ) );

    if ( aWinRegion == aWinClipRegion )
        bCoversWholeWindow = true;

    return bCoversWholeWindow;
}

void ClipManager::UpdateSysObjClip( Window *pWindow )
{
    if ( !IsOverlapWindow( pWindow ) )
    {
        updateSysObjChildrenClip( pWindow );

        // siblings should recalculate their clip region
        if ( pWindow->mpWindowImpl->mbClipSiblings )
        {
            Window* pNextWindow = pWindow->mpWindowImpl->mpNext;
            while ( pNextWindow )
            {
                updateSysObjChildrenClip( pNextWindow );
                pNextWindow = pNextWindow->mpWindowImpl->mpNext;
            }
        }
    }
    else
    {
        updateSysObjOverlapsClip( pWindow->mpWindowImpl->mpFrameWindow );
    }
}

void ClipManager::initChildClipRegion( Window *pWindow )
{
    if ( !pWindow->mpWindowImpl->mpFirstChild )
    {
        if ( pWindow->mpWindowImpl->mpChildClipRegion )
        {
            delete pWindow->mpWindowImpl->mpChildClipRegion;
            pWindow->mpWindowImpl->mpChildClipRegion = NULL;
        }
    }
    else
    {
        if ( !pWindow->mpWindowImpl->mpChildClipRegion )
            pWindow->mpWindowImpl->mpChildClipRegion = new Region( pWindow->mpWindowImpl->maWinClipRegion );
        else
            *pWindow->mpWindowImpl->mpChildClipRegion = pWindow->mpWindowImpl->maWinClipRegion;

        ClipChildren( pWindow, *pWindow->mpWindowImpl->mpChildClipRegion );
    }

    pWindow->mpWindowImpl->mbInitChildRegion = false;
}

void ClipManager::excludeOverlapWindows( Window *pWindow, Region& rRegion )
{
    Window* pOverlapWindow = pWindow->mpWindowImpl->mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->mpWindowImpl->mbReallyVisible )
        {
            Exclude( pOverlapWindow, rRegion );
            excludeOverlapWindows( pOverlapWindow, rRegion );
        }

        pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
    }
}

void ClipManager::intersectClipRegion( Window* pWindow, Region& rRegion )
{
    if ( pWindow->mpWindowImpl->mbInitWinClipRegion )
        InitClipRegion(pWindow);

    rRegion.Intersect( pWindow->mpWindowImpl->maWinClipRegion );
}

void ClipManager::intersectAndUnionOverlapWindows( Window *pWindow, const Region& rInterRegion, Region& rRegion )
{
    Window* pChildWindow = pWindow->mpWindowImpl->mpFirstOverlap;
    while ( pChildWindow )
    {
        if ( pChildWindow->mpWindowImpl->mbReallyVisible )
        {
            Region aIntersectRegion( rInterRegion );
            Intersect( pChildWindow, aIntersectRegion );
            rRegion.Union( aIntersectRegion );
            intersectAndUnionOverlapWindows( pChildWindow, rInterRegion, rRegion );
        }

        pChildWindow = pChildWindow->mpWindowImpl->mpNext;
    }
}


void ClipManager::clipSiblings( Window* pWindow, Region& rRegion )
{
    Window* pChildWindow = pWindow->ImplGetParent()->mpWindowImpl->mpFirstChild;

    while ( pChildWindow )
    {
        if ( pChildWindow == pWindow )
            break;

        if ( pChildWindow->mpWindowImpl->mbReallyVisible )
            Exclude( pChildWindow, rRegion );

        pChildWindow = pChildWindow->mpWindowImpl->mpNext;
    }
}

bool ClipManager::clipSysObject( Window *pWindow, const Region* pOldRegion )
{
    bool bUpdate = true;

    if ( pWindow->mpWindowImpl->mpSysObj )
    {
        bool bVisibleState = pWindow->mpWindowImpl->mbReallyVisible;

        if ( bVisibleState )
        {
            Region* pWinChildClipRegion = GetChildClipRegion( pWindow );

            if ( !pWinChildClipRegion->IsEmpty() )
            {
                if ( pOldRegion )
                {
                    Region aNewRegion = *pWinChildClipRegion;
                    pWinChildClipRegion->Intersect( *pOldRegion );
                    bUpdate = (aNewRegion == *pWinChildClipRegion);
                }

                if ( pWindow->mpWindowImpl->mpFrameData->mpFirstBackWin )
                    InvalidateAllOverlapBackgrounds( pWindow );

                Region      aRegion = *pWinChildClipRegion;
                Rectangle   aWinRect( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ), pWindow->GetOutputSizePixel() );
                Region      aWinRectRegion( aWinRect );
                sal_uInt16      nClipFlags = pWindow->mpWindowImpl->mpSysObj->GetClipRegionType();

                if ( aRegion == aWinRectRegion )
                    pWindow->mpWindowImpl->mpSysObj->ResetClipRegion();
                else
                {
                    if ( nClipFlags & SAL_OBJECT_CLIP_EXCLUDERECTS )
                    {
                        aWinRectRegion.Exclude( aRegion );
                        aRegion = aWinRectRegion;
                    }
                    if ( !(nClipFlags & SAL_OBJECT_CLIP_ABSOLUTE) )
                        aRegion.Move( -pWindow->mnOutOffX, -pWindow->mnOutOffY );

                    // set/update clip region
                    RectangleVector aRectangles;
                    aRegion.GetRegionRectangles(aRectangles);
                    pWindow->mpWindowImpl->mpSysObj->BeginSetClipRegion(aRectangles.size());

                    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
                    {
                        pWindow->mpWindowImpl->mpSysObj->UnionClipRegion(
                            aRectIter->Left(),
                            aRectIter->Top(),
                            aRectIter->GetWidth(),   // orig nWidth was ((R - L) + 1), same as GetWidth does
                            aRectIter->GetHeight()); // same for height
                    }

                    pWindow->mpWindowImpl->mpSysObj->EndSetClipRegion();
                }
            }
            else
                bVisibleState = false;
        }

        // update visible status
        pWindow->mpWindowImpl->mpSysObj->Show( bVisibleState );
    }

    return bUpdate;
}

void ClipManager::updateSysObjChildrenClip( Window *pWindow )
{
    if ( pWindow->mpWindowImpl->mpSysObj && pWindow->mpWindowImpl->mbInitWinClipRegion )
        clipSysObject( pWindow, NULL );

    Window* pChildWindow = pWindow->mpWindowImpl->mpFirstChild;
    while ( pChildWindow )
    {
        updateSysObjChildrenClip( pChildWindow );
        pChildWindow = pChildWindow->mpWindowImpl->mpNext;
    }
}

void ClipManager::updateSysObjOverlapsClip( Window *pWindow )
{
    updateSysObjChildrenClip( pWindow );

    Window* pNextWindow = pWindow->mpWindowImpl->mpFirstOverlap;
    while ( pNextWindow )
    {
        updateSysObjOverlapsClip( pNextWindow );
        pNextWindow = pNextWindow->mpWindowImpl->mpNext;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
