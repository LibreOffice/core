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
#include <dndlcon.hxx>
#include <dndevdis.hxx>

#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>

#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star::uno;

namespace vcl {

sal_uInt16 Window::ImplHitTest( const Point& rFramePos )
{
    Point aFramePos( rFramePos );
    if( ImplIsAntiparallel() )
    {
        // - RTL - re-mirror frame pos at this window
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aFramePos );
    }
    Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
    if ( !aRect.IsInside( aFramePos ) )
        return 0;
    if ( mpWindowImpl->mbWinRegion )
    {
        Point aTempPos = aFramePos;
        aTempPos.X() -= mnOutOffX;
        aTempPos.Y() -= mnOutOffY;
        if ( !mpWindowImpl->maWinRegion.IsInside( aTempPos ) )
            return 0;
    }

    sal_uInt16 nHitTest = WINDOW_HITTEST_INSIDE;
    if ( mpWindowImpl->mbMouseTransparent )
        nHitTest |= WINDOW_HITTEST_TRANSPARENT;
    return nHitTest;
}

bool Window::ImplTestMousePointerSet()
{
    // as soon as mouse is captured, switch mouse-pointer
    if ( IsMouseCaptured() )
        return true;

    // if the mouse is over the window, switch it
    Rectangle aClientRect( Point( 0, 0 ), GetOutputSizePixel() );
    if ( aClientRect.IsInside( GetPointerPosPixel() ) )
        return true;

    return false;
}

PointerStyle Window::ImplGetMousePointer() const
{
    PointerStyle    ePointerStyle;
    bool            bWait = false;

    if ( IsEnabled() && IsInputEnabled() && ! IsInModalMode() )
        ePointerStyle = GetPointer().GetStyle();
    else
        ePointerStyle = POINTER_ARROW;

    const vcl::Window* pWindow = this;
    do
    {
        // when the pointer is not visible stop the search, as
        // this status should not be overwritten
        if ( pWindow->mpWindowImpl->mbNoPtrVisible )
            return POINTER_NULL;

        if ( !bWait )
        {
            if ( pWindow->mpWindowImpl->mnWaitCount )
            {
                ePointerStyle = POINTER_WAIT;
                bWait = true;
            }
            else
            {
                if ( pWindow->mpWindowImpl->mbChildPtrOverwrite )
                    ePointerStyle = pWindow->GetPointer().GetStyle();
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
        sal_uLong   nTime   = tools::Time::GetSystemTicks();
        long    nX      = mpWindowImpl->mpFrameData->mnLastMouseX;
        long    nY      = mpWindowImpl->mpFrameData->mnLastMouseY;
        sal_uInt16  nCode   = nMouseCode;
        MouseEventModifiers nMode = mpWindowImpl->mpFrameData->mnMouseMode;
        bool    bLeave;
        // check for MouseLeave
        if ( ((nX < 0) || (nY < 0) ||
              (nX >= mpWindowImpl->mpFrameWindow->mnOutWidth) ||
              (nY >= mpWindowImpl->mpFrameWindow->mnOutHeight)) &&
             !ImplGetSVData()->maWinData.mpCaptureWin )
            bLeave = true;
        else
            bLeave = false;
        nMode |= MouseEventModifiers::SYNTHETIC;
        if ( bModChanged )
            nMode |= MouseEventModifiers::MODIFIERCHANGED;
        ImplHandleMouseEvent( mpWindowImpl->mpFrameWindow, EVENT_MOUSEMOVE, bLeave, nX, nY, nTime, nCode, nMode );
    }
}

void Window::ImplGenerateMouseMove()
{
    if ( !mpWindowImpl->mpFrameData->mnMouseMoveId )
        mpWindowImpl->mpFrameData->mnMouseMoveId = Application::PostUserEvent( LINK( mpWindowImpl->mpFrameWindow, Window, ImplGenerateMouseMoveHdl ) );
}

IMPL_LINK_NOARG(Window, ImplGenerateMouseMoveHdl)
{
    mpWindowImpl->mpFrameData->mnMouseMoveId = 0;
    vcl::Window* pCaptureWin = ImplGetSVData()->maWinData.mpCaptureWin;
    if( ! pCaptureWin ||
        (pCaptureWin->mpWindowImpl && pCaptureWin->mpWindowImpl->mpFrame == mpWindowImpl->mpFrame)
    )
    {
        ImplCallMouseMove( mpWindowImpl->mpFrameData->mnMouseCode );
    }
    return 0;
}

void Window::ImplInvertFocus( const Rectangle& rRect )
{
    InvertTracking( rRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
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

void Window::ImplGrabFocus( sal_uInt16 nFlags )
{
    // #143570# no focus for destructing windows
    if( mpWindowImpl->mbInDtor )
        return;

    // some event listeners do really bad stuff
    // => prepare for the worst
    ImplDelData aDogTag( this );

    // Currently the client window should always get the focus
    // Should the border window at some point be focusable
    // we need to change all GrabFocus() instances in VCL,
    // e.g. in ToTop()

    if ( mpWindowImpl->mpClientWindow )
    {
        // For a lack of design we need a little hack here to
        // ensure that dialogs on close pass the focus back to
        // the correct window
        if ( mpWindowImpl->mpLastFocusWindow && (mpWindowImpl->mpLastFocusWindow != this) &&
             !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) &&
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
        if ( mpWindowImpl->mpLastFocusWindow && (mpWindowImpl->mpLastFocusWindow != this) &&
             !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) &&
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
    if ( !IsEnabled() || !IsInputEnabled() || IsInModalNonRefMode() )
        return;

    // we only need to set the focus if it is not already set
    // note: if some other frame is waiting for an asynchrounous focus event
    // we also have to post an asynchronous focus event for this frame
    // which is done using ToTop
    ImplSVData* pSVData = ImplGetSVData();

    bool bAsyncFocusWaiting = false;
    vcl::Window *pFrame = pSVData->maWinData.mpFirstFrame;
    while( pFrame  )
    {
        if( pFrame != mpWindowImpl->mpFrameWindow && pFrame->mpWindowImpl->mpFrameData->mnFocusId )
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
        // #102158#, ignore grabfocus only if the floating parent grabs keyboard focus by itself (GrabsFocus())
        // otherwise we cannot set the focus in a floating toolbox
        if( ( (pParent->mpWindowImpl->mbFloatWin && static_cast<FloatingWindow*>(pParent)->GrabsFocus()) || ( pParent->GetStyle() & WB_SYSTEMFLOATWIN ) ) && !( pParent->GetStyle() & WB_MOVEABLE ) )
        {
            bMustNotGrabFocus = true;
            break;
        }
        pParent = pParent->mpWindowImpl->mpParent;
    }

    if ( ( pSVData->maWinData.mpFocusWin != this && ! mpWindowImpl->mbInDtor ) || ( bAsyncFocusWaiting && !bHasFocus && !bMustNotGrabFocus ) )
    {
        // EndExtTextInput if it is not the same window
        if ( pSVData->maWinData.mpExtTextInputWin &&
             (pSVData->maWinData.mpExtTextInputWin != this) )
            pSVData->maWinData.mpExtTextInputWin->EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );

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
                //DBG_WARNING( "Window::GrabFocus() - Frame doesn't have the focus" );
                mpWindowImpl->mpFrame->ToTop( SAL_FRAME_TOTOP_GRABFOCUS | SAL_FRAME_TOTOP_GRABFOCUS_ONLY );
                return;
            }
        }

        vcl::Window* pOldFocusWindow = pSVData->maWinData.mpFocusWin;
        ImplDelData aOldFocusDel( pOldFocusWindow );

        pSVData->maWinData.mpFocusWin = this;

        if ( pOldFocusWindow )
        {
            // Cursor hidden
            if ( pOldFocusWindow->mpWindowImpl->mpCursor )
                pOldFocusWindow->mpWindowImpl->mpCursor->ImplHide( true );
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
        if ( pOldFocusWindow && ! aOldFocusDel.IsDead() )
        {
            if ( pOldFocusWindow->IsTracking() &&
                 (pSVData->maWinData.mnTrackFlags & STARTTRACK_FOCUSCANCEL) )
                pOldFocusWindow->EndTracking( ENDTRACK_CANCEL | ENDTRACK_FOCUS );
            NotifyEvent aNEvt( EVENT_LOSEFOCUS, pOldFocusWindow );
            if ( !ImplCallPreNotify( aNEvt ) )
                pOldFocusWindow->LoseFocus();
            pOldFocusWindow->ImplCallDeactivateListeners( this );
        }

        if ( pSVData->maWinData.mpFocusWin == this )
        {
            if ( mpWindowImpl->mpSysObj )
            {
                mpWindowImpl->mpFrameData->mpFocusWin = this;
                if ( !mpWindowImpl->mpFrameData->mbInSysObjFocusHdl )
                    mpWindowImpl->mpSysObj->GrabFocus();
            }

            if ( pSVData->maWinData.mpFocusWin == this )
            {
                if ( mpWindowImpl->mpCursor )
                    mpWindowImpl->mpCursor->ImplShow();
                mpWindowImpl->mbInFocusHdl = true;
                mpWindowImpl->mnGetFocusFlags = nFlags;
                // if we're changing focus due to closing a popup floating window
                // notify the new focus window so it can restore the inner focus
                // eg, toolboxes can select their recent active item
                if( pOldFocusWindow &&
                    ! aOldFocusDel.IsDead() &&
                    ( pOldFocusWindow->GetDialogControlFlags() & WINDOW_DLGCTRL_FLOATWIN_POPUPMODEEND_CANCEL ) )
                    mpWindowImpl->mnGetFocusFlags |= GETFOCUS_FLOATWIN_POPUPMODEEND_CANCEL;
                NotifyEvent aNEvt( EVENT_GETFOCUS, this );
                if ( !ImplCallPreNotify( aNEvt ) && !aDogTag.IsDead() )
                    GetFocus();
                if( !aDogTag.IsDead() )
                    ImplCallActivateListeners( (pOldFocusWindow && ! aOldFocusDel.IsDead()) ? pOldFocusWindow : NULL );
                if( !aDogTag.IsDead() )
                {
                    mpWindowImpl->mnGetFocusFlags = 0;
                    mpWindowImpl->mbInFocusHdl = false;
                }
            }
        }

        GetpApp()->FocusChanged();
        ImplNewInputContext();
    }
}

void Window::ImplGrabFocusToDocument( sal_uInt16 nFlags )
{
    vcl::Window *pWin = this;
    while( pWin )
    {
        if( !pWin->GetParent() )
        {
            pWin->ImplGetFrameWindow()->GetWindow( WINDOW_CLIENT )->ImplGrabFocus(nFlags);
            return;
        }
        pWin = pWin->GetParent();
    }
}

void Window::MouseMove( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( EVENT_MOUSEMOVE, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseMove = true;
}

void Window::MouseButtonDown( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( EVENT_MOUSEBUTTONDOWN, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseButtonDown = true;
}

void Window::MouseButtonUp( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( EVENT_MOUSEBUTTONUP, this, &rMEvt );
    if ( !Notify( aNEvt ) )
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
    if ( pSVData->maWinData.mpTrackWin != this )
    {
        if ( pSVData->maWinData.mpTrackWin )
            pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    }

    if ( pSVData->maWinData.mpCaptureWin != this )
    {
        pSVData->maWinData.mpCaptureWin = this;
        mpWindowImpl->mpFrame->CaptureMouse( true );
    }
}

void Window::ReleaseMouse()
{

    ImplSVData* pSVData = ImplGetSVData();

    DBG_ASSERTWARNING( pSVData->maWinData.mpCaptureWin == this,
                       "Window::ReleaseMouse(): window doesn't have the mouse capture" );

    if ( pSVData->maWinData.mpCaptureWin == this )
    {
        pSVData->maWinData.mpCaptureWin = NULL;
        mpWindowImpl->mpFrame->CaptureMouse( false );
        ImplGenerateMouseMove();
    }
}

bool Window::IsMouseCaptured() const
{

    return (this == ImplGetSVData()->maWinData.mpCaptureWin);
}

void Window::SetPointer( const Pointer& rPointer )
{

    if ( mpWindowImpl->maPointer == rPointer )
        return;

    mpWindowImpl->maPointer   = rPointer;

    // possibly immediately move pointer
    if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
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
            // --- RTL --- (re-mirror mouse pos at this window)
            pOutDev->ReMirror( aPos );
        }
        // mirroring is required here, SetPointerPos bypasses SalGraphics
        mpGraphics->mirror( aPos.X(), this );
    }
    else if( ImplIsAntiparallel() )
    {
        pOutDev->ReMirror( aPos );
    }
    mpWindowImpl->mpFrame->SetPointerPos( aPos.X(), aPos.Y() );
}

Point Window::GetPointerPosPixel()
{

    Point aPos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
    if( ImplIsAntiparallel() )
    {
        // --- RTL --- (re-mirror mouse pos at this window)
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
        // --- RTL --- (re-mirror mouse pos at this window)
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
            // --- RTL --- (re-mirror mouse pos at this window)
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

    if( ! mpWindowImpl->mxDNDListenerContainer.is() )
    {
        sal_Int8 nDefaultActions = 0;

        if( mpWindowImpl->mpFrameData )
        {
            if( ! mpWindowImpl->mpFrameData->mxDropTarget.is() )
            {
                // initialization is done in GetDragSource
                Reference< css::datatransfer::dnd::XDragSource > xDragSource = GetDragSource();
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
#if defined WNT
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.OleDragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.OleDropTarget";
                    aDragSourceAL[ 1 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->hWnd) ) );
                    aDropTargetAL[ 0 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->hWnd) ) );
#elif defined MACOSX
            /* FIXME: Mac OS X specific dnd interface does not exist! *
             * Using Windows based dnd as a temporary solution        */
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.OleDragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.OleDropTarget";
                    aDragSourceAL[ 1 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->mpNSView) ) );
                    aDropTargetAL[ 0 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->mpNSView) ) );
#elif HAVE_FEATURE_X11
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.X11DragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.X11DropTarget";

                    aDragSourceAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                    aDropTargetAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                    aDropTargetAL[ 1 ] = makeAny( (sal_Size)(pEnvData->aShellWindow) );
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
