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
#include <vcl/taskpanelist.hxx>

// declare system types in sysdata.hxx
#include <vcl/sysdata.hxx>

#include <salframe.hxx>
#include <salobj.hxx>
#include <salgdi.hxx>
#include <svdata.hxx>
#include <window.h>
#include <brdwin.hxx>
#include <helpwin.hxx>

#include <com/sun/star/awt/XTopWindow.hpp>

#include <set>
#include <typeinfo>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::com::sun::star;

using ::com::sun::star::awt::XTopWindow;

struct ImplCalcToTopData
{
    ImplCalcToTopData*       mpNext;
    VclPtr<vcl::Window>      mpWindow;
    vcl::Region*             mpInvalidateRegion;
};

namespace vcl {

vcl::Window* Window::ImplGetTopmostFrameWindow()
{
    vcl::Window *pTopmostParent = this;
    while( pTopmostParent->ImplGetParent() )
        pTopmostParent = pTopmostParent->ImplGetParent();
    return pTopmostParent->mpWindowImpl->mpFrameWindow;
}

void Window::ImplInsertWindow( vcl::Window* pParent )
{
    mpWindowImpl->mpParent            = pParent;
    mpWindowImpl->mpRealParent        = pParent;

    if ( pParent && !mpWindowImpl->mbFrame )
    {
        // search frame window and set window frame data
        vcl::Window* pFrameParent = pParent->mpWindowImpl->mpFrameWindow;
        mpWindowImpl->mpFrameData     = pFrameParent->mpWindowImpl->mpFrameData;
        mpWindowImpl->mpFrame         = pFrameParent->mpWindowImpl->mpFrame;
        mpWindowImpl->mpFrameWindow   = pFrameParent;
        mpWindowImpl->mbFrame         = false;

        // search overlap window and insert window in list
        if ( ImplIsOverlapWindow() )
        {
            vcl::Window* pFirstOverlapParent = pParent;
            while ( !pFirstOverlapParent->ImplIsOverlapWindow() )
                pFirstOverlapParent = pFirstOverlapParent->ImplGetParent();
            mpWindowImpl->mpOverlapWindow = pFirstOverlapParent;

            mpWindowImpl->mpNextOverlap = mpWindowImpl->mpFrameData->mpFirstOverlap;
            mpWindowImpl->mpFrameData->mpFirstOverlap = this;

            // Overlap-Windows are by default the uppermost
            mpWindowImpl->mpNext = pFirstOverlapParent->mpWindowImpl->mpFirstOverlap;
            pFirstOverlapParent->mpWindowImpl->mpFirstOverlap = this;
            if ( !pFirstOverlapParent->mpWindowImpl->mpLastOverlap )
                pFirstOverlapParent->mpWindowImpl->mpLastOverlap = this;
            else
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = this;
        }
        else
        {
            if ( pParent->ImplIsOverlapWindow() )
                mpWindowImpl->mpOverlapWindow = pParent;
            else
                mpWindowImpl->mpOverlapWindow = pParent->mpWindowImpl->mpOverlapWindow;
            mpWindowImpl->mpPrev = pParent->mpWindowImpl->mpLastChild;
            pParent->mpWindowImpl->mpLastChild = this;
            if ( !pParent->mpWindowImpl->mpFirstChild )
                pParent->mpWindowImpl->mpFirstChild = this;
            else
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
        }
    }
}

void Window::ImplRemoveWindow( bool bRemoveFrameData )
{
    // remove window from the lists
    if ( !mpWindowImpl->mbFrame )
    {
        if ( ImplIsOverlapWindow() )
        {
            if ( mpWindowImpl->mpFrameData->mpFirstOverlap.get() == this )
                mpWindowImpl->mpFrameData->mpFirstOverlap = mpWindowImpl->mpNextOverlap;
            else
            {
                vcl::Window* pTempWin = mpWindowImpl->mpFrameData->mpFirstOverlap;
                while ( pTempWin->mpWindowImpl->mpNextOverlap.get() != this )
                    pTempWin = pTempWin->mpWindowImpl->mpNextOverlap;
                pTempWin->mpWindowImpl->mpNextOverlap = mpWindowImpl->mpNextOverlap;
            }

            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = mpWindowImpl->mpPrev;
        }
        else
        {
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else if ( mpWindowImpl->mpParent )
                mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else if ( mpWindowImpl->mpParent )
                mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = mpWindowImpl->mpPrev;
        }

        mpWindowImpl->mpPrev = nullptr;
        mpWindowImpl->mpNext = nullptr;
    }

    if ( bRemoveFrameData )
    {
        // release the graphic
        OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReleaseGraphics();
    }
}

void Window::reorderWithinParent(sal_uInt16 nNewPosition)
{
    sal_uInt16 nChildCount = 0;
    vcl::Window *pSource = mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild;
    while (pSource)
    {
        if (nChildCount == nNewPosition)
            break;
        pSource = pSource->mpWindowImpl->mpNext;
        nChildCount++;
    }

    if (pSource == this) //already at the right place
        return;

    ImplRemoveWindow(false);

    if (pSource)
    {
        mpWindowImpl->mpNext = pSource;
        mpWindowImpl->mpPrev = pSource->mpWindowImpl->mpPrev;
        pSource->mpWindowImpl->mpPrev = this;
    }
    else
        mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = this;

    if (mpWindowImpl->mpPrev)
        mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
    else
        mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = this;
}

void Window::ImplToBottomChild()
{
    if ( !ImplIsOverlapWindow() && !mpWindowImpl->mbReallyVisible && (mpWindowImpl->mpParent->mpWindowImpl->mpLastChild.get() != this) )
    {
        // put the window to the end of the list
        if ( mpWindowImpl->mpPrev )
            mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
        else
            mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = mpWindowImpl->mpNext;
        mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
        mpWindowImpl->mpPrev = mpWindowImpl->mpParent->mpWindowImpl->mpLastChild;
        mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = this;
        mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
        mpWindowImpl->mpNext = nullptr;
    }
}

void Window::ImplCalcToTop( ImplCalcToTopData* pPrevData )
{
    SAL_WARN_IF( !ImplIsOverlapWindow(), "vcl", "Window::ImplCalcToTop(): Is not a OverlapWindow" );

    if ( !mpWindowImpl->mbFrame )
    {
        if ( IsReallyVisible() )
        {
            // calculate region, where the window overlaps with other windows
            Point aPoint( mnOutOffX, mnOutOffY );
            vcl::Region  aRegion( Rectangle( aPoint,
                                        Size( mnOutWidth, mnOutHeight ) ) );
            vcl::Region  aInvalidateRegion;
            ImplCalcOverlapRegionOverlaps( aRegion, aInvalidateRegion );

            if ( !aInvalidateRegion.IsEmpty() )
            {
                ImplCalcToTopData* pData    = new ImplCalcToTopData;
                pPrevData->mpNext           = pData;
                pData->mpNext               = nullptr;
                pData->mpWindow             = this;
                pData->mpInvalidateRegion   = new vcl::Region( aInvalidateRegion );
            }
        }
    }
}

void Window::ImplToTop( ToTopFlags nFlags )
{
    SAL_WARN_IF( !ImplIsOverlapWindow(), "vcl", "Window::ImplToTop(): Is not a OverlapWindow" );

    if ( mpWindowImpl->mbFrame )
    {
        // on a mouse click in the external window, it is the latter's
        // responsibility to assure our frame is put in front
        if ( !mpWindowImpl->mpFrameData->mbHasFocus &&
             !mpWindowImpl->mpFrameData->mbSysObjFocus &&
             !mpWindowImpl->mpFrameData->mbInSysObjFocusHdl &&
             !mpWindowImpl->mpFrameData->mbInSysObjToTopHdl )
        {
            // do not bring floating windows on the client to top
            if( !ImplGetClientWindow() || !(ImplGetClientWindow()->GetStyle() & WB_SYSTEMFLOATWIN) )
            {
                SalFrameToTop nSysFlags = SalFrameToTop::NONE;
                if ( nFlags & ToTopFlags::RestoreWhenMin )
                    nSysFlags |= SalFrameToTop::RestoreWhenMin;
                if ( nFlags & ToTopFlags::ForegroundTask )
                    nSysFlags |= SalFrameToTop::ForegroundTask;
                if ( nFlags & ToTopFlags::GrabFocusOnly )
                    nSysFlags |= SalFrameToTop::GrabFocusOnly;
                mpWindowImpl->mpFrame->ToTop( nSysFlags );
            }
        }
    }
    else
    {
        if ( mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap.get() != this )
        {
            // remove window from the list
            mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = mpWindowImpl->mpPrev;

            // take AlwaysOnTop into account
            bool    bOnTop = IsAlwaysOnTopEnabled();
            vcl::Window* pNextWin = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
            if ( !bOnTop )
            {
                while ( pNextWin )
                {
                    if ( !pNextWin->IsAlwaysOnTopEnabled() )
                        break;
                    pNextWin = pNextWin->mpWindowImpl->mpNext;
                }
            }

            // add the window to the list again
            mpWindowImpl->mpNext = pNextWin;
            if ( pNextWin )
            {
                mpWindowImpl->mpPrev = pNextWin->mpWindowImpl->mpPrev;
                pNextWin->mpWindowImpl->mpPrev = this;
            }
            else
            {
                mpWindowImpl->mpPrev = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap;
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = this;
            }
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = this;

            // recalculate ClipRegion of this and all overlapping windows
            if ( IsReallyVisible() )
            {
                mpWindowImpl->mpOverlapWindow->ImplSetClipFlagOverlapWindows();
            }
        }
    }
}

void Window::ImplStartToTop( ToTopFlags nFlags )
{
    ImplCalcToTopData   aStartData;
    ImplCalcToTopData*  pCurData;
    ImplCalcToTopData*  pNextData;
    vcl::Window* pOverlapWindow;
    if ( ImplIsOverlapWindow() )
        pOverlapWindow = this;
    else
        pOverlapWindow = mpWindowImpl->mpOverlapWindow;

    // first calculate paint areas
    vcl::Window* pTempOverlapWindow = pOverlapWindow;
    aStartData.mpNext = nullptr;
    pCurData = &aStartData;
    do
    {
        pTempOverlapWindow->ImplCalcToTop( pCurData );
        if ( pCurData->mpNext )
            pCurData = pCurData->mpNext;
        pTempOverlapWindow = pTempOverlapWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( !pTempOverlapWindow->mpWindowImpl->mbFrame );
    // next calculate the paint areas of the ChildOverlap windows
    pTempOverlapWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempOverlapWindow )
    {
        pTempOverlapWindow->ImplCalcToTop( pCurData );
        if ( pCurData->mpNext )
            pCurData = pCurData->mpNext;
        pTempOverlapWindow = pTempOverlapWindow->mpWindowImpl->mpNext;
    }

    // and next change the windows list
    pTempOverlapWindow = pOverlapWindow;
    do
    {
        pTempOverlapWindow->ImplToTop( nFlags );
        pTempOverlapWindow = pTempOverlapWindow->mpWindowImpl->mpOverlapWindow;
    }
    while ( !pTempOverlapWindow->mpWindowImpl->mbFrame );
    // as last step invalidate the invalid areas
    pCurData = aStartData.mpNext;
    while ( pCurData )
    {
        pCurData->mpWindow->ImplInvalidateFrameRegion( pCurData->mpInvalidateRegion, InvalidateFlags::Children );
        pNextData = pCurData->mpNext;
        delete pCurData->mpInvalidateRegion;
        delete pCurData;
        pCurData = pNextData;
    }
}

void Window::ImplFocusToTop( ToTopFlags nFlags, bool bReallyVisible )
{
    // do we need to fetch the focus?
    if ( !(nFlags & ToTopFlags::NoGrabFocus) )
    {
        // first window with GrabFocus-Activate gets the focus
        vcl::Window* pFocusWindow = this;
        while ( !pFocusWindow->ImplIsOverlapWindow() )
        {
            // if the window has no BorderWindow, we
            // should always find the belonging BorderWindow
            if ( !pFocusWindow->mpWindowImpl->mpBorderWindow )
            {
                if ( pFocusWindow->mpWindowImpl->mnActivateMode & ActivateModeFlags::GrabFocus )
                    break;
            }
            pFocusWindow = pFocusWindow->ImplGetParent();
        }
        if ( (pFocusWindow->mpWindowImpl->mnActivateMode & ActivateModeFlags::GrabFocus) &&
             !pFocusWindow->HasChildPathFocus( true ) )
            pFocusWindow->GrabFocus();
    }

    if ( bReallyVisible )
        ImplGenerateMouseMove();
}

void Window::ImplShowAllOverlaps()
{
    vcl::Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->mpWindowImpl->mbOverlapVisible )
        {
            pOverlapWindow->Show( true, ShowFlags::NoActivate );
            pOverlapWindow->mpWindowImpl->mbOverlapVisible = false;
        }

        pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplHideAllOverlaps()
{
    vcl::Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
    while ( pOverlapWindow )
    {
        if ( pOverlapWindow->IsVisible() )
        {
            pOverlapWindow->mpWindowImpl->mbOverlapVisible = true;
            pOverlapWindow->Show( false );
        }

        pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
    }
}

void Window::ToTop( ToTopFlags nFlags )
{
    if (!mpWindowImpl)
        return;

    ImplStartToTop( nFlags );
    ImplFocusToTop( nFlags, IsReallyVisible() );
}

void Window::SetZOrder( vcl::Window* pRefWindow, ZOrderFlags nFlags )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->SetZOrder( pRefWindow, nFlags );
        return;
    }

    if ( nFlags & ZOrderFlags::First )
    {
        if ( ImplIsOverlapWindow() )
            pRefWindow = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
        else
            pRefWindow = mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild;
        nFlags |= ZOrderFlags::Before;
    }
    else if ( nFlags & ZOrderFlags::Last )
    {
        if ( ImplIsOverlapWindow() )
            pRefWindow = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap;
        else
            pRefWindow = mpWindowImpl->mpParent->mpWindowImpl->mpLastChild;
        nFlags |= ZOrderFlags::Behind;
    }

    while ( pRefWindow && pRefWindow->mpWindowImpl->mpBorderWindow )
        pRefWindow = pRefWindow->mpWindowImpl->mpBorderWindow;
    if (!pRefWindow || pRefWindow == this || mpWindowImpl->mbFrame)
        return;

    SAL_WARN_IF( pRefWindow->mpWindowImpl->mpParent != mpWindowImpl->mpParent, "vcl", "Window::SetZOrder() - pRefWindow has other parent" );
    if ( nFlags & ZOrderFlags::Before )
    {
        if ( pRefWindow->mpWindowImpl->mpPrev.get() == this )
            return;

        if ( ImplIsOverlapWindow() )
        {
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = mpWindowImpl->mpPrev;
            if ( !pRefWindow->mpWindowImpl->mpPrev )
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = this;
        }
        else
        {
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = mpWindowImpl->mpPrev;
            if ( !pRefWindow->mpWindowImpl->mpPrev )
                mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = this;
        }

        mpWindowImpl->mpPrev = pRefWindow->mpWindowImpl->mpPrev;
        mpWindowImpl->mpNext = pRefWindow;
        if ( mpWindowImpl->mpPrev )
            mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
        mpWindowImpl->mpNext->mpWindowImpl->mpPrev = this;
    }
    else if ( nFlags & ZOrderFlags::Behind )
    {
        if ( pRefWindow->mpWindowImpl->mpNext.get() == this )
            return;

        if ( ImplIsOverlapWindow() )
        {
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = mpWindowImpl->mpPrev;
            if ( !pRefWindow->mpWindowImpl->mpNext )
                mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpLastOverlap = this;
        }
        else
        {
            if ( mpWindowImpl->mpPrev )
                mpWindowImpl->mpPrev->mpWindowImpl->mpNext = mpWindowImpl->mpNext;
            else
                mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild = mpWindowImpl->mpNext;
            if ( mpWindowImpl->mpNext )
                mpWindowImpl->mpNext->mpWindowImpl->mpPrev = mpWindowImpl->mpPrev;
            else
                mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = mpWindowImpl->mpPrev;
            if ( !pRefWindow->mpWindowImpl->mpNext )
                mpWindowImpl->mpParent->mpWindowImpl->mpLastChild = this;
        }

        mpWindowImpl->mpPrev = pRefWindow;
        mpWindowImpl->mpNext = pRefWindow->mpWindowImpl->mpNext;
        if ( mpWindowImpl->mpNext )
            mpWindowImpl->mpNext->mpWindowImpl->mpPrev = this;
        mpWindowImpl->mpPrev->mpWindowImpl->mpNext = this;
    }

    if ( IsReallyVisible() )
    {
        if ( mpWindowImpl->mbInitWinClipRegion || !mpWindowImpl->maWinClipRegion.IsEmpty() )
        {
            bool bInitWinClipRegion = mpWindowImpl->mbInitWinClipRegion;
            ImplSetClipFlag();

            // When ClipRegion was not initialised, assume
            // the window has not been sent, therefore do not
            // trigger any Invalidates. This is an optimization
            // for HTML documents with many controls. If this
            // check gives problems, a flag should be introduced
            // which tracks whether the window has already been
            // emitted after Show
            if ( !bInitWinClipRegion )
            {
                // Invalidate all windows which are next to each other
                // Is INCOMPLETE !!!
                Rectangle   aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                vcl::Window*     pWindow = nullptr;
                if ( ImplIsOverlapWindow() )
                {
                    if ( mpWindowImpl->mpOverlapWindow )
                        pWindow = mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
                }
                else
                    pWindow = ImplGetParent()->mpWindowImpl->mpFirstChild;
                // Invalidate all windows in front of us and which are covered by us
                while ( pWindow )
                {
                    if ( pWindow == this )
                        break;
                    Rectangle aCompRect( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                         Size( pWindow->mnOutWidth, pWindow->mnOutHeight ) );
                    if ( aWinRect.IsOver( aCompRect ) )
                        pWindow->Invalidate( InvalidateFlags::Children | InvalidateFlags::NoTransparent );
                    pWindow = pWindow->mpWindowImpl->mpNext;
                }

                // If we are covered by a window in the background
                // we should redraw it
                while ( pWindow )
                {
                    if ( pWindow != this )
                    {
                        Rectangle aCompRect( Point( pWindow->mnOutOffX, pWindow->mnOutOffY ),
                                             Size( pWindow->mnOutWidth, pWindow->mnOutHeight ) );
                        if ( aWinRect.IsOver( aCompRect ) )
                        {
                            Invalidate( InvalidateFlags::Children | InvalidateFlags::NoTransparent );
                            break;
                        }
                    }
                    pWindow = pWindow->mpWindowImpl->mpNext;
                }
            }
        }
    }
}

void Window::EnableAlwaysOnTop( bool bEnable )
{

    mpWindowImpl->mbAlwaysOnTop = bEnable;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->EnableAlwaysOnTop( bEnable );
    else if ( bEnable && IsReallyVisible() )
        ToTop();

    if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetAlwaysOnTop( bEnable );
}

bool Window::IsTopWindow() const
{
    if ( !mpWindowImpl || mpWindowImpl->mbInDispose )
        return false;

    // topwindows must be frames or they must have a borderwindow which is a frame
    if( !mpWindowImpl->mbFrame && (!mpWindowImpl->mpBorderWindow || !mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame ) )
        return false;

    ImplGetWinData();
    if( mpWindowImpl->mpWinData->mnIsTopWindow == (sal_uInt16)~0)    // still uninitialized
    {
        // #113722#, cache result of expensive queryInterface call
        vcl::Window *pThisWin = const_cast<vcl::Window*>(this);
        uno::Reference< XTopWindow > xTopWindow( pThisWin->GetComponentInterface(), UNO_QUERY );
        pThisWin->mpWindowImpl->mpWinData->mnIsTopWindow = xTopWindow.is() ? 1 : 0;
    }
    return mpWindowImpl->mpWinData->mnIsTopWindow == 1;
}

vcl::Window* Window::FindWindow( const Point& rPos ) const
{

    Point aPos = OutputToScreenPixel( rPos );
    return const_cast<vcl::Window*>(this)->ImplFindWindow( aPos );
}

vcl::Window* Window::ImplFindWindow( const Point& rFramePos )
{
    vcl::Window* pTempWindow;
    vcl::Window* pFindWindow;

    // first check all overlapping windows
    pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
        if ( pFindWindow )
            return pFindWindow;
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    // then we check our window
    if ( !mpWindowImpl->mbVisible )
        return nullptr;

    WindowHitTest nHitTest = ImplHitTest( rFramePos );
    if ( nHitTest & WindowHitTest::Inside )
    {
        // and then we check all child windows
        pTempWindow = mpWindowImpl->mpFirstChild;
        while ( pTempWindow )
        {
            pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
            if ( pFindWindow )
                return pFindWindow;
            pTempWindow = pTempWindow->mpWindowImpl->mpNext;
        }

        if ( nHitTest & WindowHitTest::Transparent )
            return nullptr;
        else
            return this;
    }

    return nullptr;
}

bool Window::ImplIsRealParentPath( const vcl::Window* pWindow ) const
{
    pWindow = pWindow->GetParent();
    while ( pWindow )
    {
        if ( pWindow == this )
            return true;
        pWindow = pWindow->GetParent();
    }

    return false;
}

bool Window::ImplIsChild( const vcl::Window* pWindow, bool bSystemWindow ) const
{
    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return true;
    }
    while ( pWindow );

    return false;
}

bool Window::ImplIsWindowOrChild( const vcl::Window* pWindow, bool bSystemWindow ) const
{
    if ( this == pWindow )
        return true;
    return ImplIsChild( pWindow, bSystemWindow );
}

void Window::ImplResetReallyVisible()
{
    bool bBecameReallyInvisible = mpWindowImpl->mbReallyVisible;

    mbDevOutput     = false;
    mpWindowImpl->mbReallyVisible = false;
    mpWindowImpl->mbReallyShown   = false;

    // the SHOW/HIDE events serve as indicators to send child creation/destroy events to the access bridge.
    // For this, the data member of the event must not be NULL.
    // Previously, we did this in Window::Show, but there some events got lost in certain situations.
    if( bBecameReallyInvisible && ImplIsAccessibleCandidate() )
        CallEventListeners( VCLEVENT_WINDOW_HIDE, this );
        // TODO. It's kind of a hack that we're re-using the VCLEVENT_WINDOW_HIDE. Normally, we should
        // introduce another event which explicitly triggers the Accessibility implementations.

    vcl::Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
            pWindow->ImplResetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
            pWindow->ImplResetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplUpdateWindowPtr( vcl::Window* pWindow )
{
    if ( mpWindowImpl->mpFrameWindow != pWindow->mpWindowImpl->mpFrameWindow )
    {
        // release graphic
        OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReleaseGraphics();
    }

    mpWindowImpl->mpFrameData     = pWindow->mpWindowImpl->mpFrameData;
    mpWindowImpl->mpFrame         = pWindow->mpWindowImpl->mpFrame;
    mpWindowImpl->mpFrameWindow   = pWindow->mpWindowImpl->mpFrameWindow;
    if ( pWindow->ImplIsOverlapWindow() )
        mpWindowImpl->mpOverlapWindow = pWindow;
    else
        mpWindowImpl->mpOverlapWindow = pWindow->mpWindowImpl->mpOverlapWindow;

    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( pWindow );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

void Window::ImplUpdateWindowPtr()
{
    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( this );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

void Window::ImplUpdateOverlapWindowPtr( bool bNewFrame )
{
    bool bVisible = IsVisible();
    Show( false );
    ImplRemoveWindow( bNewFrame );
    vcl::Window* pRealParent = mpWindowImpl->mpRealParent;
    ImplInsertWindow( ImplGetParent() );
    mpWindowImpl->mpRealParent = pRealParent;
    ImplUpdateWindowPtr();
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();

    if ( bNewFrame )
    {
        vcl::Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
        while ( pOverlapWindow )
        {
            vcl::Window* pNextOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
            pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
            pOverlapWindow = pNextOverlapWindow;
        }
    }

    if ( bVisible )
        Show();
}

SystemWindow* Window::GetSystemWindow() const
{

    const vcl::Window* pWin = this;
    while ( pWin && !pWin->IsSystemWindow() )
        pWin  = pWin->GetParent();
    return static_cast<SystemWindow*>(const_cast<Window*>(pWin));
}

static SystemWindow *ImplGetLastSystemWindow( vcl::Window *pWin )
{
    // get the most top-level system window, the one that contains the taskpanelist
    SystemWindow *pSysWin = nullptr;
    if( !pWin )
        return pSysWin;
    vcl::Window *pMyParent = pWin;
    while ( pMyParent )
    {
        if ( pMyParent->IsSystemWindow() )
            pSysWin = static_cast<SystemWindow*>(pMyParent);
        pMyParent = pMyParent->GetParent();
    }
    return pSysWin;
}

void Window::SetParent( vcl::Window* pNewParent )
{
    SAL_WARN_IF( !pNewParent, "vcl", "Window::SetParent(): pParent == NULL" );
    SAL_WARN_IF( pNewParent == this, "vcl", "someone tried to reparent a window to itself" );

    if( !pNewParent || pNewParent == this )
        return;

    // check if the taskpanelist would change and move the window pointer accordingly
    SystemWindow *pSysWin = ImplGetLastSystemWindow(this);
    SystemWindow *pNewSysWin = nullptr;
    bool bChangeTaskPaneList = false;
    if( pSysWin && pSysWin->ImplIsInTaskPaneList( this ) )
    {
        pNewSysWin = ImplGetLastSystemWindow( pNewParent );
        if( pNewSysWin && pNewSysWin != pSysWin )
        {
            bChangeTaskPaneList = true;
            pSysWin->GetTaskPaneList()->RemoveWindow( this );
        }
    }
    // remove ownerdraw decorated windows from list in the top-most frame window
    if( (GetStyle() & WB_OWNERDRAWDECORATION) && mpWindowImpl->mbFrame )
    {
        ::std::vector< VclPtr<vcl::Window> >& rList = ImplGetOwnerDrawList();
        auto p = ::std::find( rList.begin(), rList.end(), VclPtr<vcl::Window>(this) );
        if( p != rList.end() )
            rList.erase( p );
    }

    ImplSetFrameParent( pNewParent );

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpRealParent = pNewParent;
        mpWindowImpl->mpBorderWindow->SetParent( pNewParent );
        return;
    }

    if ( mpWindowImpl->mpParent.get() == pNewParent )
        return;

    if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetParent( pNewParent->mpWindowImpl->mpFrame );

    bool bVisible = IsVisible();
    Show( false, ShowFlags::NoFocusChange );

    // check if the overlap window changes
    vcl::Window* pOldOverlapWindow;
    vcl::Window* pNewOverlapWindow = nullptr;
    if ( ImplIsOverlapWindow() )
        pOldOverlapWindow = nullptr;
    else
    {
        pNewOverlapWindow = pNewParent->ImplGetFirstOverlapWindow();
        if ( mpWindowImpl->mpOverlapWindow.get() != pNewOverlapWindow )
            pOldOverlapWindow = mpWindowImpl->mpOverlapWindow;
        else
            pOldOverlapWindow = nullptr;
    }

    // convert windows in the hierarchy
    bool bFocusOverlapWin = HasChildPathFocus( true );
    bool bFocusWin = HasChildPathFocus();
    bool bNewFrame = pNewParent->mpWindowImpl->mpFrameWindow != mpWindowImpl->mpFrameWindow;
    if ( bNewFrame )
    {
        if ( mpWindowImpl->mpFrameData->mpFocusWin )
        {
            if ( IsWindowOrChild( mpWindowImpl->mpFrameData->mpFocusWin ) )
                mpWindowImpl->mpFrameData->mpFocusWin = nullptr;
        }
        if ( mpWindowImpl->mpFrameData->mpMouseMoveWin )
        {
            if ( IsWindowOrChild( mpWindowImpl->mpFrameData->mpMouseMoveWin ) )
                mpWindowImpl->mpFrameData->mpMouseMoveWin = nullptr;
        }
        if ( mpWindowImpl->mpFrameData->mpMouseDownWin )
        {
            if ( IsWindowOrChild( mpWindowImpl->mpFrameData->mpMouseDownWin ) )
                mpWindowImpl->mpFrameData->mpMouseDownWin = nullptr;
        }
    }
    ImplRemoveWindow( bNewFrame );
    ImplInsertWindow( pNewParent );
    if ( mpWindowImpl->mnParentClipMode & ParentClipMode::Clip )
        pNewParent->mpWindowImpl->mbClipChildren = true;
    ImplUpdateWindowPtr();
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();

    // If the Overlap-Window has changed, we need to test whether
    // OverlapWindows that had the Child window as their parent
    // need to be put into the window hierarchy.
    if ( ImplIsOverlapWindow() )
    {
        if ( bNewFrame )
        {
            vcl::Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
            while ( pOverlapWindow )
            {
                vcl::Window* pNextOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
                pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
                pOverlapWindow = pNextOverlapWindow;
            }
        }
    }
    else if ( pOldOverlapWindow )
    {
        // reset Focus-Save
        if ( bFocusWin ||
             (pOldOverlapWindow->mpWindowImpl->mpLastFocusWindow &&
              IsWindowOrChild( pOldOverlapWindow->mpWindowImpl->mpLastFocusWindow )) )
            pOldOverlapWindow->mpWindowImpl->mpLastFocusWindow = nullptr;

        vcl::Window* pOverlapWindow = pOldOverlapWindow->mpWindowImpl->mpFirstOverlap;
        while ( pOverlapWindow )
        {
            vcl::Window* pNextOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
            if ( ImplIsRealParentPath( pOverlapWindow->ImplGetWindow() ) )
                pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
            pOverlapWindow = pNextOverlapWindow;
        }

        // update activate-status at next overlap window
        if ( HasChildPathFocus( true ) )
            ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
    }

    // also convert Activate-Status
    if ( bNewFrame )
    {
        if ( (GetType() == WINDOW_BORDERWINDOW) &&
             (ImplGetWindow()->GetType() == WINDOW_FLOATINGWINDOW) )
            static_cast<ImplBorderWindow*>(this)->SetDisplayActive( mpWindowImpl->mpFrameData->mbHasFocus );
    }

    // when required give focus to new frame if
    // FocusWindow is changed with SetParent()
    if ( bFocusOverlapWin )
    {
        mpWindowImpl->mpFrameData->mpFocusWin = Application::GetFocusWindow();
        if ( !mpWindowImpl->mpFrameData->mbHasFocus )
        {
            mpWindowImpl->mpFrame->ToTop( SalFrameToTop::NONE );
        }
    }

    // Assure DragSource and DropTarget members are created
    if ( bNewFrame )
    {
            GetDropTarget();
    }

    if( bChangeTaskPaneList )
        pNewSysWin->GetTaskPaneList()->AddWindow( this );

    if( (GetStyle() & WB_OWNERDRAWDECORATION) && mpWindowImpl->mbFrame )
        ImplGetOwnerDrawList().push_back( this );

    if ( bVisible )
        Show( true, ShowFlags::NoFocusChange | ShowFlags::NoActivate );
}

sal_uInt16 Window::GetChildCount() const
{
    if (!mpWindowImpl)
        return 0;

    sal_uInt16  nChildCount = 0;
    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        nChildCount++;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    return nChildCount;
}

vcl::Window* Window::GetChild( sal_uInt16 nChild ) const
{
    if (!mpWindowImpl)
        return nullptr;

    sal_uInt16  nChildCount = 0;
    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        if ( nChild == nChildCount )
            return pChild;
        pChild = pChild->mpWindowImpl->mpNext;
        nChildCount++;
    }

    return nullptr;
}

vcl::Window* Window::GetWindow( GetWindowType nType ) const
{
    if (!mpWindowImpl)
        return nullptr;

    switch ( nType )
    {
        case GetWindowType::Parent:
            return mpWindowImpl->mpRealParent;

        case GetWindowType::FirstChild:
            return mpWindowImpl->mpFirstChild;

        case GetWindowType::LastChild:
            return mpWindowImpl->mpLastChild;

        case GetWindowType::Prev:
            return mpWindowImpl->mpPrev;

        case GetWindowType::Next:
            return mpWindowImpl->mpNext;

        case GetWindowType::FirstOverlap:
            return mpWindowImpl->mpFirstOverlap;

        case GetWindowType::LastOverlap:
            return mpWindowImpl->mpLastOverlap;

        case GetWindowType::Overlap:
            if ( ImplIsOverlapWindow() )
                return const_cast<vcl::Window*>(this);
            else
                return mpWindowImpl->mpOverlapWindow;

        case GetWindowType::ParentOverlap:
            if ( ImplIsOverlapWindow() )
                return mpWindowImpl->mpOverlapWindow;
            else
                return mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpOverlapWindow;

        case GetWindowType::Client:
            return const_cast<vcl::Window*>(this)->ImplGetWindow();

        case GetWindowType::RealParent:
            return ImplGetParent();

        case GetWindowType::Frame:
            return mpWindowImpl->mpFrameWindow;

        case GetWindowType::Border:
            if ( mpWindowImpl->mpBorderWindow )
                return mpWindowImpl->mpBorderWindow->GetWindow( GetWindowType::Border );
            return const_cast<vcl::Window*>(this);

        case GetWindowType::FirstTopWindowChild:
            return ImplGetWinData()->maTopWindowChildren.empty() ? nullptr : (*ImplGetWinData()->maTopWindowChildren.begin()).get();

        case GetWindowType::LastTopWindowChild:
            return ImplGetWinData()->maTopWindowChildren.empty() ? nullptr : (*ImplGetWinData()->maTopWindowChildren.rbegin()).get();

        case GetWindowType::PrevTopWindowSibling:
        {
            if ( !mpWindowImpl->mpRealParent )
                return nullptr;
            const ::std::list< VclPtr<vcl::Window> >& rTopWindows( mpWindowImpl->mpRealParent->ImplGetWinData()->maTopWindowChildren );
            ::std::list< VclPtr<vcl::Window> >::const_iterator myPos =
                ::std::find( rTopWindows.begin(), rTopWindows.end(), this );
            if ( myPos == rTopWindows.end() )
                return nullptr;
            if ( myPos == rTopWindows.begin() )
                return nullptr;
            return *--myPos;
        }

        case GetWindowType::NextTopWindowSibling:
        {
            if ( !mpWindowImpl->mpRealParent )
                return nullptr;
            const ::std::list< VclPtr<vcl::Window> >& rTopWindows( mpWindowImpl->mpRealParent->ImplGetWinData()->maTopWindowChildren );
            ::std::list< VclPtr<vcl::Window> >::const_iterator myPos =
                ::std::find( rTopWindows.begin(), rTopWindows.end(), this );
            if ( ( myPos == rTopWindows.end() ) || ( ++myPos == rTopWindows.end() ) )
                return nullptr;
            return *myPos;
        }

    }

    return nullptr;
}

bool Window::IsChild( const vcl::Window* pWindow, bool bSystemWindow ) const
{
    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return true;
    }
    while ( pWindow );

    return false;
}

bool Window::IsWindowOrChild( const vcl::Window* pWindow, bool bSystemWindow ) const
{

    if ( this == pWindow )
        return true;
    return ImplIsChild( pWindow, bSystemWindow );
}

void Window::ImplSetFrameParent( const vcl::Window* pParent )
{
    vcl::Window* pFrameWindow = ImplGetSVData()->maWinData.mpFirstFrame;
    while( pFrameWindow )
    {
        // search all frames that are children of this window
        // and reparent them
        if( ImplIsRealParentPath( pFrameWindow ) )
        {
            SAL_WARN_IF( mpWindowImpl->mpFrame == pFrameWindow->mpWindowImpl->mpFrame, "vcl", "SetFrameParent to own" );
            SAL_WARN_IF( !mpWindowImpl->mpFrame, "vcl", "no frame" );
            SalFrame* pParentFrame = pParent ? pParent->mpWindowImpl->mpFrame : nullptr;
            pFrameWindow->mpWindowImpl->mpFrame->SetParent( pParentFrame );
        }
        pFrameWindow = pFrameWindow->mpWindowImpl->mpFrameData->mpNextFrame;
    }
}

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
