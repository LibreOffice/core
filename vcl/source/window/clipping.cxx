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
#include <vcl/clipmgr.hxx>

#include <sal/types.h>

#include <salobj.hxx>
#include <window.h>

void Window::InitClipRegion()
{
    DBG_TESTSOLARMUTEX();

    ClipManager *pClipMgr = ClipManager::GetInstance();
    pClipMgr->Init( this );
}

void Window::SetParentClipMode( sal_uInt16 nMode )
{
    ClipManager *pClipMgr = ClipManager::GetInstance();
    pClipMgr->SetParentClipMode( this, nMode );
}

sal_uInt16 Window::GetParentClipMode() const
{
    ClipManager *pClipMgr = ClipManager::GetInstance();
    return pClipMgr->GetParentClipMode( const_cast<Window*>(this) );
}

Region Window::GetActiveClipRegion() const
{
    ClipManager *pClipMgr = ClipManager::GetInstance();
    return pClipMgr->GetActiveClipRegion( this );
}

void Window::ClipToPaintRegion(Rectangle& rDstRect)
{
    const Region aPaintRgn(GetPaintRegion());

    if (!aPaintRgn.IsNull())
        rDstRect.Intersection(LogicToPixel(aPaintRgn.GetBoundRect()));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
