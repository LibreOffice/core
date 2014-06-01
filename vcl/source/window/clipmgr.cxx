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

#include <sal/types.h>

#include <salobj.hxx>
#include <clipmgr.hxx>
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

void ClipManager::InitClipRegion( Window *pWindow )
{
    // Build Window region
    pWindow->mpWindowImpl->maWinClipRegion = Rectangle( Point( pWindow->GetOutOffXPixel(), pWindow->GetOutOffYPixel() ),
                                                        pWindow->GetOutputSizePixel() );
    if ( pWindow->mpWindowImpl->mbWinRegion )
        pWindow->mpWindowImpl->maWinClipRegion.Intersect( pWindow->ImplPixelToDevicePixel( pWindow->mpWindowImpl->maWinRegion ) );

    if ( pWindow->mpWindowImpl->mbClipSiblings && !pWindow->ImplIsOverlapWindow() )
        clipSiblings( pWindow, pWindow->mpWindowImpl->maWinClipRegion );

    // Clip Parent Boundaries
    pWindow->ImplClipBoundaries( pWindow->mpWindowImpl->maWinClipRegion, false, true );

    // Clip Children
    if ( (pWindow->GetStyle() & WB_CLIPCHILDREN) || pWindow->mpWindowImpl->mbClipChildren )
        pWindow->mpWindowImpl->mbInitChildRegion = true;

    pWindow->mpWindowImpl->mbInitWinClipRegion = false;
}

void ClipManager::clipSiblings( Window* pWindow, Region& rRegion )
{
    Window* pChildWindow = pWindow->ImplGetParent()->mpWindowImpl->mpFirstChild;

    while ( pChildWindow )
    {
        if ( pChildWindow == pWindow )
            break;

        if ( pChildWindow->mpWindowImpl->mbReallyVisible )
            pChildWindow->ImplExcludeWindowRegion( rRegion );

        pChildWindow = pChildWindow->mpWindowImpl->mpNext;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
