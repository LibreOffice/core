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

void ClipManager::EnableClipSiblings( Window *pWindow, bool bClipSiblings )
{

    if ( pWindow->mpWindowImpl->mpBorderWindow )
        EnableClipSiblings( pWindow->mpWindowImpl->mpBorderWindow, bClipSiblings );

    pWindow->mpWindowImpl->mbClipSiblings = bClipSiblings;
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
        intersectClipRegion( pWindow, rRegion );
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

bool ClipManager::ClipCoversWholeWindow( Window *pWindow )
{
    ClipManager *clipMgr = ClipManager::GetInstance();

    bool bCoversWholeWindow = false;

    if ( pWindow->mpWindowImpl->mbInitWinClipRegion )
        clipMgr->InitClipRegion( pWindow );

    Region aWinClipRegion = pWindow->mpWindowImpl->maWinClipRegion;
    Region aWinRegion( Rectangle ( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ), pWindow->GetOutputSizePixel() ) );

    if ( aWinRegion == aWinClipRegion )
        bCoversWholeWindow = true;

    return bCoversWholeWindow;
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
    ClipManager *clipMgr = ClipManager::GetInstance();

    if ( pWindow->mpWindowImpl->mbInitWinClipRegion )
        clipMgr->InitClipRegion(pWindow);

    rRegion.Intersect( pWindow->mpWindowImpl->maWinClipRegion );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
