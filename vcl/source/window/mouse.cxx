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


#include <config_features.h>

#include <tools/time.hxx>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <vcl/ITiledRenderable.hxx>
#include <vcl/svapp.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/window.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/cursor.hxx>
#include <vcl/sysdata.hxx>

#include <sal/types.h>

#include <window.h>
#include <outdev.h>
#include <svdata.hxx>
#include <salobj.hxx>
#include <salgdi.hxx>
#include <salframe.hxx>

#include <dndlistenercontainer.hxx>
#include <dndeventdispatcher.hxx>

#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star::uno;

namespace vcl {

WindowHitTest Window::ImplHitTest( const Point& rFramePos )
{
    Point aFramePos( rFramePos );
    if( ImplIsAntiparallel() )
    {
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aFramePos );
    }
    tools::Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
    if ( !aRect.IsInside( aFramePos ) )
        return WindowHitTest::NONE;
    if ( mpWindowImpl->mbWinRegion )
    {
        Point aTempPos = aFramePos;
        aTempPos.AdjustX( -mnOutOffX );
        aTempPos.AdjustY( -mnOutOffY );
        if ( !mpWindowImpl->maWinRegion.IsInside( aTempPos ) )
            return WindowHitTest::NONE;
    }

    WindowHitTest nHitTest = WindowHitTest::Inside;
    if ( mpWindowImpl->mbMouseTransparent )
        nHitTest |= WindowHitTest::Transparent;
    return nHitTest;
}

bool Window::ImplTestMousePointerSet()
{
    // as soon as mouse is captured, switch mouse-pointer
    if ( IsMouseCaptured() )
        return true;

    // if the mouse is over the window, switch it
    tools::Rectangle aClientRect( Point( 0, 0 ), GetOutputSizePixel() );
    return aClientRect.IsInside( GetPointerPosPixel() );
}

PointerStyle Window::ImplGetMousePointer() const
{
    PointerStyle    ePointerStyle;
    bool            bWait = false;

    if ( IsEnabled() && IsInputEnabled() && ! IsInModalMode() )
        ePointerStyle = GetPointer();
    else
        ePointerStyle = PointerStyle::Arrow;

    const vcl::Window* pWindow = this;
    do
    {
        // when the pointer is not visible stop the search, as
        // this status should not be overwritten
        if ( pWindow->mpWindowImpl->mbNoPtrVisible )
            return PointerStyle::Null;

        if ( !bWait )
        {
            if ( pWindow->mpWindowImpl->mnWaitCount )
            {
                ePointerStyle = PointerStyle::Wait;
                bWait = true;
            }
            else
            {
                if ( pWindow->mpWindowImpl->mbChildPtrOverwrite )
                    ePointerStyle = pWindow->GetPointer();
            }
        }

        if ( pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();
    }
    while ( pWindow );

    return ePointerStyle;
}

void Window::ImplCallMouseMove( sal_uInt16 nMouseCode, bool bModChanged )
{
    if ( mpWindowImpl->mpFrameData->mbMouseIn && mpWindowImpl->mpFrameWindow->mpWindowImpl->mbReallyVisible )
    {
        sal_uInt64 nTime   = tools::Time::GetSystemTicks();
        long    nX      = mpWindowImpl->mpFrameData->mnLastMouseX;
        long    nY      = mpWindowImpl->mpFrameData->mnLastMouseY;
        sal_uInt16  nCode   = nMouseCode;
        MouseEventModifiers nMode = mpWindowImpl->mpFrameData->mnMouseMode;
        bool    bLeave;
        // check for MouseLeave
        bLeave = ((nX < 0) || (nY < 0) ||
                  (nX >= mpWindowImpl->mpFrameWindow->mnOutWidth) ||
                  (nY >= mpWindowImpl->mpFrameWindow->mnOutHeight)) &&
                 !ImplGetSVData()->maWinData.mpCaptureWin;
        nMode |= MouseEventModifiers::SYNTHETIC;
        if ( bModChanged )
            nMode |= MouseEventModifiers::MODIFIERCHANGED;
        ImplHandleMouseEvent( mpWindowImpl->mpFrameWindow, MouseNotifyEvent::MOUSEMOVE, bLeave, nX, nY, nTime, nCode, nMode );
    }
}

void Window::ImplGenerateMouseMove()
{
    if ( mpWindowImpl && mpWindowImpl->mpFrameData &&
         !mpWindowImpl->mpFrameData->mnMouseMoveId )
        mpWindowImpl->mpFrameData->mnMouseMoveId = Application::PostUserEvent( LINK( mpWindowImpl->mpFrameWindow, Window, ImplGenerateMouseMoveHdl ), nullptr, true );
}

IMPL_LINK_NOARG(Window, ImplGenerateMouseMoveHdl, void*, void)
{
    mpWindowImpl->mpFrameData->mnMouseMoveId = nullptr;
    vcl::Window* pCaptureWin = ImplGetSVData()->maWinData.mpCaptureWin;
    if( ! pCaptureWin ||
        (pCaptureWin->mpWindowImpl && pCaptureWin->mpWindowImpl->mpFrame == mpWindowImpl->mpFrame)
    )
    {
        ImplCallMouseMove( mpWindowImpl->mpFrameData->mnMouseCode );
    }
}

void Window::ImplInvertFocus( const tools::Rectangle& rRect )
{
    InvertTracking( rRect, ShowTrackFlags::Small | ShowTrackFlags::TrackWindow );
}

static bool IsWindowFocused(const WindowImpl& rWinImpl)
{
    if (rWinImpl.mpSysObj)
        return true;

    if (rWinImpl.mpFrameData->mbHasFocus)
        return true;

    if (rWinImpl.mbFakeFocusSet)
        return true;

    return false;
}

void Window::ImplGrabFocus( GetFocusFlags nFlags )
{
    // #143570# no focus for destructing windows
    if( !mpWindowImpl || mpWindowImpl->mbInDispose )
        return;

    // some event listeners do really bad stuff
    // => prepare for the worst
    VclPtr<vcl::Window> xWindow( this );

    // Currently the client window should always get the focus
    // Should the border window at some point be focusable
    // we need to change all GrabFocus() instances in VCL,
    // e.g. in ToTop()

    if ( mpWindowImpl->mpClientWindow )
    {
        // For a lack of design we need a little hack here to
        // ensure that dialogs on close pass the focus back to
        // the correct window
        if ( mpWindowImpl->mpLastFocusWindow && (mpWindowImpl->mpLastFocusWindow.get() != this) &&
             !(mpWindowImpl->mnDlgCtrlFlags & DialogControlFlags::WantFocus) &&
             mpWindowImpl->mpLastFocusWindow->IsEnabled() &&
             mpWindowImpl->mpLastFocusWindow->IsInputEnabled() &&
             ! mpWindowImpl->mpLastFocusWindow->IsInModalMode()
             )
            mpWindowImpl->mpLastFocusWindow->GrabFocus();
        else
            mpWindowImpl->mpClientWindow->GrabFocus();
        return;
    }
    else if ( mpWindowImpl->mbFrame )
    {
        // For a lack of design we need a little hack here to
        // ensure that dialogs on close pass the focus back to
        // the correct window
        if ( mpWindowImpl->mpLastFocusWindow && (mpWindowImpl->mpLastFocusWindow.get() != this) &&
             !(mpWindowImpl->mnDlgCtrlFlags & DialogControlFlags::WantFocus) &&
             mpWindowImpl->mpLastFocusWindow->IsEnabled() &&
             mpWindowImpl->mpLastFocusWindow->IsInputEnabled() &&
             ! mpWindowImpl->mpLastFocusWindow->IsInModalMode()
             )
        {
            mpWindowImpl->mpLastFocusWindow->GrabFocus();
            return;
        }
    }

    // If the Window is disabled, then we don't change the focus
    if ( !IsEnabled() || !IsInputEnabled() || IsInModalMode() )
        return;

    // we only need to set the focus if it is not already set
    // note: if some other frame is waiting for an asynchronous focus event
    // we also have to post an asynchronous focus event for this frame
    // which is done using ToTop
    ImplSVData* pSVData = ImplGetSVData();

    bool bAsyncFocusWaiting = false;
    vcl::Window *pFrame = pSVData->maWinData.mpFirstFrame;
    while( pFrame  )
    {
        if( pFrame != mpWindowImpl->mpFrameWindow.get() && pFrame->mpWindowImpl->mpFrameData->mnFocusId )
        {
            bAsyncFocusWaiting = true;
            break;
        }
        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }

    bool bHasFocus = IsWindowFocused(*mpWindowImpl);

    bool bMustNotGrabFocus = false;
    // #100242#, check parent hierarchy if some floater prohibits grab focus

    vcl::Window *pParent = this;
    while( pParent )
    {
        if ((pParent->GetStyle() & WB_SYSTEMFLOATWIN) && !(pParent->GetStyle() & WB_MOVEABLE))
        {
            bMustNotGrabFocus = true;
            break;
        }
        pParent = pParent->mpWindowImpl->mpParent;
    }

    if ( !(( pSVData->maWinData.mpFocusWin.get() != this &&
             !mpWindowImpl->mbInDispose ) ||
           ( bAsyncFocusWaiting && !bHasFocus && !bMustNotGrabFocus )) )
        return;

    // EndExtTextInput if it is not the same window
    if ( pSVData->maWinData.mpExtTextInputWin &&
         (pSVData->maWinData.mpExtTextInputWin.get() != this) )
        pSVData->maWinData.mpExtTextInputWin->EndExtTextInput();

    // mark this windows as the last FocusWindow
    vcl::Window* pOverlapWindow = ImplGetFirstOverlapWindow();
    pOverlapWindow->mpWindowImpl->mpLastFocusWindow = this;
    mpWindowImpl->mpFrameData->mpFocusWin = this;

    if( !bHasFocus )
    {
        // menu windows never get the system focus
        // the application will keep the focus
        if( bMustNotGrabFocus )
            return;
        else
        {
            // here we already switch focus as ToTop()
            // should not give focus to another window
            mpWindowImpl->mpFrame->ToTop( SalFrameToTop::GrabFocus | SalFrameToTop::GrabFocusOnly );
            return;
        }
    }

    VclPtr<vcl::Window> pOldFocusWindow = pSVData->maWinData.mpFocusWin;

    pSVData->maWinData.mpFocusWin = this;

    if ( pOldFocusWindow )
    {
        // Cursor hidden
        if ( pOldFocusWindow->mpWindowImpl->mpCursor )
            pOldFocusWindow->mpWindowImpl->mpCursor->ImplHide();
    }

    // !!!!! due to old SV-Office Activate/Deactivate handling
    // !!!!! first as before
    if ( pOldFocusWindow )
    {
        // remember Focus
        vcl::Window* pOldOverlapWindow = pOldFocusWindow->ImplGetFirstOverlapWindow();
        vcl::Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
        if ( pOldOverlapWindow != pNewOverlapWindow )
            ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
    }
    else
    {
        vcl::Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
        vcl::Window* pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
        pNewOverlapWindow->mpWindowImpl->mbActive = true;
        pNewOverlapWindow->Activate();
        if ( pNewRealWindow != pNewOverlapWindow )
        {
            pNewRealWindow->mpWindowImpl->mbActive = true;
            pNewRealWindow->Activate();
        }
    }

    // call Get- and LoseFocus
    if ( pOldFocusWindow && ! pOldFocusWindow->IsDisposed() )
    {
        NotifyEvent aNEvt( MouseNotifyEvent::LOSEFOCUS, pOldFocusWindow );
        if ( !ImplCallPreNotify( aNEvt ) )
            pOldFocusWindow->CompatLoseFocus();
        pOldFocusWindow->ImplCallDeactivateListeners( this );
    }

    if ( pSVData->maWinData.mpFocusWin.get() == this )
    {
        if ( mpWindowImpl->mpSysObj )
        {
            mpWindowImpl->mpFrameData->mpFocusWin = this;
            if ( !mpWindowImpl->mpFrameData->mbInSysObjFocusHdl )
                mpWindowImpl->mpSysObj->GrabFocus();
        }

        if ( pSVData->maWinData.mpFocusWin.get() == this )
        {
            if ( mpWindowImpl->mpCursor )
                mpWindowImpl->mpCursor->ImplShow();
            mpWindowImpl->mbInFocusHdl = true;
            mpWindowImpl->mnGetFocusFlags = nFlags;
            // if we're changing focus due to closing a popup floating window
            // notify the new focus window so it can restore the inner focus
            // eg, toolboxes can select their recent active item
            if( pOldFocusWindow &&
                ! pOldFocusWindow->IsDisposed() &&
                ( pOldFocusWindow->GetDialogControlFlags() & DialogControlFlags::FloatWinPopupModeEndCancel ) )
                mpWindowImpl->mnGetFocusFlags |= GetFocusFlags::FloatWinPopupModeEndCancel;
            NotifyEvent aNEvt( MouseNotifyEvent::GETFOCUS, this );
            if ( !ImplCallPreNotify( aNEvt ) && !xWindow->IsDisposed() )
                CompatGetFocus();
            if( !xWindow->IsDisposed() )
                ImplCallActivateListeners( (pOldFocusWindow && ! pOldFocusWindow->IsDisposed()) ? pOldFocusWindow : nullptr );
            if( !xWindow->IsDisposed() )
            {
                mpWindowImpl->mnGetFocusFlags = GetFocusFlags::NONE;
                mpWindowImpl->mbInFocusHdl = false;
            }
        }
    }

    ImplNewInputContext();

}

void Window::ImplGrabFocusToDocument( GetFocusFlags nFlags )
{
    vcl::Window *pWin = this;
    while( pWin )
    {
        if( !pWin->GetParent() )
        {
            pWin->ImplGetFrameWindow()->GetWindow( GetWindowType::Client )->ImplGrabFocus(nFlags);
            return;
        }
        pWin = pWin->GetParent();
    }
}

void Window::MouseMove( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( MouseNotifyEvent::MOUSEMOVE, this, &rMEvt );
    EventNotify(aNEvt);
}

void Window::MouseButtonDown( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( MouseNotifyEvent::MOUSEBUTTONDOWN, this, &rMEvt );
    if (!EventNotify(aNEvt))
        mpWindowImpl->mbMouseButtonDown = true;
}

void Window::MouseButtonUp( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( MouseNotifyEvent::MOUSEBUTTONUP, this, &rMEvt );
    if (!EventNotify(aNEvt))
        mpWindowImpl->mbMouseButtonUp = true;
}

void Window::SetMouseTransparent( bool bTransparent )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetMouseTransparent( bTransparent );

    if( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->SetMouseTransparent( bTransparent );

    mpWindowImpl->mbMouseTransparent = bTransparent;
}

void Window::CaptureMouse()
{

    ImplSVData* pSVData = ImplGetSVData();

    // possibly stop tracking
    if ( pSVData->maWinData.mpTrackWin.get() != this )
    {
        if ( pSVData->maWinData.mpTrackWin )
            pSVData->maWinData.mpTrackWin->EndTracking( TrackingEventFlags::Cancel );
    }

    if ( pSVData->maWinData.mpCaptureWin.get() != this )
    {
        pSVData->maWinData.mpCaptureWin = this;
        mpWindowImpl->mpFrame->CaptureMouse( true );
    }
}

void Window::ReleaseMouse()
{
    if (IsMouseCaptured())
    {
        ImplSVData* pSVData = ImplGetSVData();
        pSVData->maWinData.mpCaptureWin = nullptr;
        mpWindowImpl->mpFrame->CaptureMouse( false );
        ImplGenerateMouseMove();
    }
}

bool Window::IsMouseCaptured() const
{
    return (this == ImplGetSVData()->maWinData.mpCaptureWin);
}

void Window::SetPointer( PointerStyle nPointer )
{
    if ( mpWindowImpl->maPointer == nPointer )
        return;

    mpWindowImpl->maPointer   = nPointer;

    // possibly immediately move pointer
    if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );

    if (VclPtr<vcl::Window> pWin = GetParentWithLOKNotifier())
    {
        PointerStyle aPointer = GetPointer();
        // We don't map all possible pointers hence we need a default
        OString aPointerString = "default";
        auto aIt = vcl::gaLOKPointerMap.find(aPointer);
        if (aIt != vcl::gaLOKPointerMap.end())
        {
            aPointerString = aIt->second;
        }

        // issue mouse pointer events only for document windows
        // Doc windows' immediate parent SfxFrameViewWindow_Impl is the one with
        // parent notifier set during initialization
        if (GetParent()->ImplGetWindowImpl()->mbLOKParentNotifier &&
            GetParent()->ImplGetWindowImpl()->mnLOKWindowId == 0)
        {
            pWin->GetLOKNotifier()->libreOfficeKitViewCallback(LOK_CALLBACK_MOUSE_POINTER, aPointerString.getStr());
        }
    }
}

void Window::EnableChildPointerOverwrite( bool bOverwrite )
{

    if ( mpWindowImpl->mbChildPtrOverwrite == bOverwrite )
        return;

    mpWindowImpl->mbChildPtrOverwrite  = bOverwrite;

    // possibly immediately move pointer
    if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
}

void Window::SetPointerPosPixel( const Point& rPos )
{
    Point aPos = ImplOutputToFrame( rPos );
    const OutputDevice *pOutDev = GetOutDev();
    if( pOutDev->HasMirroredGraphics() )
    {
        if( !IsRTLEnabled() )
        {
            pOutDev->ReMirror( aPos );
        }
        // mirroring is required here, SetPointerPos bypasses SalGraphics
        aPos.setX( mpGraphics->mirror2( aPos.X(), this ) );
    }
    else if( ImplIsAntiparallel() )
    {
        pOutDev->ReMirror( aPos );
    }
    mpWindowImpl->mpFrame->SetPointerPos( aPos.X(), aPos.Y() );
}

void Window::SetLastMousePos(const Point& rPos)
{
    // Do this conversion, so when GetPointerPosPixel() calls
    // ImplFrameToOutput(), we get back the original position.
    Point aPos = ImplOutputToFrame(rPos);
    mpWindowImpl->mpFrameData->mnLastMouseX = aPos.X();
    mpWindowImpl->mpFrameData->mnLastMouseY = aPos.Y();
}

Point Window::GetPointerPosPixel()
{

    Point aPos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
    if( ImplIsAntiparallel() )
    {
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aPos );
    }
    return ImplFrameToOutput( aPos );
}

Point Window::GetLastPointerPosPixel()
{

    Point aPos( mpWindowImpl->mpFrameData->mnBeforeLastMouseX, mpWindowImpl->mpFrameData->mnBeforeLastMouseY );
    if( ImplIsAntiparallel() )
    {
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aPos );
    }
    return ImplFrameToOutput( aPos );
}

void Window::ShowPointer( bool bVisible )
{

    if ( mpWindowImpl->mbNoPtrVisible != !bVisible )
    {
        mpWindowImpl->mbNoPtrVisible = !bVisible;

        // possibly immediately move pointer
        if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
    }
}

Window::PointerState Window::GetPointerState()
{
    PointerState aState;
    aState.mnState = 0;

    if (mpWindowImpl->mpFrame)
    {
        SalFrame::SalPointerState aSalPointerState;

        aSalPointerState = mpWindowImpl->mpFrame->GetPointerState();
        if( ImplIsAntiparallel() )
        {
            const OutputDevice *pOutDev = GetOutDev();
            pOutDev->ReMirror( aSalPointerState.maPos );
        }
        aState.maPos = ImplFrameToOutput( aSalPointerState.maPos );
        aState.mnState = aSalPointerState.mnState;
    }
    return aState;
}

bool Window::IsMouseOver()
{
    return ImplGetWinData()->mbMouseOver;
}

void Window::EnterWait()
{

    mpWindowImpl->mnWaitCount++;

    if ( mpWindowImpl->mnWaitCount == 1 )
    {
        // possibly immediately move pointer
        if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
    }
}

void Window::LeaveWait()
{

    if ( mpWindowImpl->mnWaitCount )
    {
        mpWindowImpl->mnWaitCount--;

        if ( !mpWindowImpl->mnWaitCount )
        {
            // possibly immediately move pointer
            if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
                mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
        }
    }
}

bool Window::ImplStopDnd()
{
    bool bRet = false;
    if( mpWindowImpl->mpFrameData && mpWindowImpl->mpFrameData->mxDropTargetListener.is() )
    {
        bRet = true;
        mpWindowImpl->mpFrameData->mxDropTarget.clear();
        mpWindowImpl->mpFrameData->mxDragSource.clear();
        mpWindowImpl->mpFrameData->mxDropTargetListener.clear();
    }

    return bRet;
}

void Window::ImplStartDnd()
{
    GetDropTarget();
}

Reference< css::datatransfer::dnd::XDropTarget > Window::GetDropTarget()
{
    if( !mpWindowImpl )
        return Reference< css::datatransfer::dnd::XDropTarget >();

    if( ! mpWindowImpl->mxDNDListenerContainer.is() )
    {
        sal_Int8 nDefaultActions = 0;

        if( mpWindowImpl->mpFrameData )
        {
            if( ! mpWindowImpl->mpFrameData->mxDropTarget.is() )
            {
                // initialization is done in GetDragSource
                GetDragSource();
            }

            if( mpWindowImpl->mpFrameData->mxDropTarget.is() )
            {
                nDefaultActions = mpWindowImpl->mpFrameData->mxDropTarget->getDefaultActions();

                if( ! mpWindowImpl->mpFrameData->mxDropTargetListener.is() )
                {
                    mpWindowImpl->mpFrameData->mxDropTargetListener = new DNDEventDispatcher( mpWindowImpl->mpFrameWindow );

                    try
                    {
                        mpWindowImpl->mpFrameData->mxDropTarget->addDropTargetListener( mpWindowImpl->mpFrameData->mxDropTargetListener );

                        // register also as drag gesture listener if directly supported by drag source
                        Reference< css::datatransfer::dnd::XDragGestureRecognizer > xDragGestureRecognizer =
                            Reference< css::datatransfer::dnd::XDragGestureRecognizer > (mpWindowImpl->mpFrameData->mxDragSource, UNO_QUERY);

                        if( xDragGestureRecognizer.is() )
                        {
                            xDragGestureRecognizer->addDragGestureListener(
                                Reference< css::datatransfer::dnd::XDragGestureListener > (mpWindowImpl->mpFrameData->mxDropTargetListener, UNO_QUERY));
                        }
                        else
                            mpWindowImpl->mpFrameData->mbInternalDragGestureRecognizer = true;

                    }
                    catch (const RuntimeException&)
                    {
                        // release all instances
                        mpWindowImpl->mpFrameData->mxDropTarget.clear();
                        mpWindowImpl->mpFrameData->mxDragSource.clear();
                    }
                }
            }

        }

        mpWindowImpl->mxDNDListenerContainer = static_cast < css::datatransfer::dnd::XDropTarget * > ( new DNDListenerContainer( nDefaultActions ) );
    }

    // this object is located in the same process, so there will be no runtime exception
    return Reference< css::datatransfer::dnd::XDropTarget > ( mpWindowImpl->mxDNDListenerContainer, UNO_QUERY );
}

Reference< css::datatransfer::dnd::XDragSource > Window::GetDragSource()
{

#if HAVE_FEATURE_DESKTOP

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxDragSource.is() )
        {
            try
            {
                Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
                const SystemEnvData * pEnvData = GetSystemData();

                if( pEnvData )
                {
                    Sequence< Any > aDragSourceAL( 2 ), aDropTargetAL( 2 );
                    OUString aDragSourceSN, aDropTargetSN;
#if defined(_WIN32)
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.OleDragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.OleDropTarget";
                    aDragSourceAL[ 1 ] <<= static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->hWnd) );
                    aDropTargetAL[ 0 ] <<= static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->hWnd) );
#elif defined MACOSX
            /* FIXME: macOS specific dnd interface does not exist! *
             * Using Windows based dnd as a temporary solution        */
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.OleDragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.OleDropTarget";
                    aDragSourceAL[ 1 ] <<= static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->mpNSView) );
                    aDropTargetAL[ 0 ] <<= static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->mpNSView) );
#elif HAVE_FEATURE_X11
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.X11DragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.X11DropTarget";

                    aDragSourceAL[ 0 ] <<= Application::GetDisplayConnection();
                    aDragSourceAL[ 1 ] <<= pEnvData->aShellWindow;
                    aDropTargetAL[ 0 ] <<= Application::GetDisplayConnection();
                    aDropTargetAL[ 1 ] <<= pEnvData->aShellWindow;
#endif
                    if( !aDragSourceSN.isEmpty() )
                        mpWindowImpl->mpFrameData->mxDragSource.set(
                            xContext->getServiceManager()->createInstanceWithArgumentsAndContext( aDragSourceSN, aDragSourceAL, xContext ),
                            UNO_QUERY );

                    if( !aDropTargetSN.isEmpty() )
                        mpWindowImpl->mpFrameData->mxDropTarget.set(
                           xContext->getServiceManager()->createInstanceWithArgumentsAndContext( aDropTargetSN, aDropTargetAL, xContext ),
                           UNO_QUERY );
                }
            }

            // createInstance can throw any exception
            catch (const Exception&)
            {
                // release all instances
                mpWindowImpl->mpFrameData->mxDropTarget.clear();
                mpWindowImpl->mpFrameData->mxDragSource.clear();
            }
        }

        return mpWindowImpl->mpFrameData->mxDragSource;
    }
#endif
    return Reference< css::datatransfer::dnd::XDragSource > ();
}

Reference< css::datatransfer::dnd::XDragGestureRecognizer > Window::GetDragGestureRecognizer()
{
    return Reference< css::datatransfer::dnd::XDragGestureRecognizer > ( GetDropTarget(), UNO_QUERY );
}

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
