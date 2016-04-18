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

#include <tools/debug.hxx>
#include <tools/time.hxx>

#include <unotools/localedatawrapper.hxx>

#include <vcl/i18nhelp.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/timer.hxx>
#include <vcl/event.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/cursor.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/help.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/menu.hxx>
#include <vcl/virdev.hxx>
#include <vcl/lazydelete.hxx>
#include <touch/touch.h>

#include <svdata.hxx>
#include <dbggui.hxx>
#include <salwtype.hxx>
#include <salframe.hxx>
#include <accmgr.hxx>
#include <print.h>
#include <window.h>
#include <helpwin.hxx>
#include <brdwin.hxx>
#include <salgdi.hxx>
#include <dndlcon.hxx>

#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>

#define IMPL_MIN_NEEDSYSWIN         49

bool ImplCallPreNotify( NotifyEvent& rEvt )
{
    return Application::CallEventHooks( rEvt )
        || rEvt.GetWindow()->CompatPreNotify( rEvt );
}

static bool ImplHandleMouseFloatMode( vcl::Window* pChild, const Point& rMousePos,
                                      sal_uInt16 nCode, MouseNotifyEvent nSVEvent,
                                      bool bMouseLeave )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpFirstFloat && !pSVData->maWinData.mpCaptureWin &&
         !pSVData->maWinData.mpFirstFloat->ImplIsFloatPopupModeWindow( pChild ) )
    {
        /*
         *  #93895# since floats are system windows, coordinates have
         *  to be converted to float relative for the hittest
         */
        HitTest         nHitTest = HITTEST_OUTSIDE;
        FloatingWindow* pFloat = pSVData->maWinData.mpFirstFloat->ImplFloatHitTest( pChild, rMousePos, nHitTest );
        FloatingWindow* pLastLevelFloat;
        FloatWinPopupFlags nPopupFlags;
        if ( nSVEvent == MouseNotifyEvent::MOUSEMOVE )
        {
            if ( bMouseLeave )
                return true;

            if ( !pFloat || (nHitTest == HITTEST_RECT) )
            {
                if ( pSVData->maHelpData.mpHelpWin && !pSVData->maHelpData.mbKeyboardHelp )
                    ImplDestroyHelpWindow( true );
                pChild->ImplGetFrame()->SetPointer( PointerStyle::Arrow );
                return true;
            }
        }
        else
        {
            if ( nCode & MOUSE_LEFT )
            {
                if ( nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN )
                {
                    if ( !pFloat )
                    {
                        pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
                        pLastLevelFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
                        return true;
                    }
                    else if ( nHitTest == HITTEST_RECT )
                    {
                        if ( !(pFloat->GetPopupModeFlags() & FloatWinPopupFlags::NoMouseRectClose) )
                            pFloat->ImplSetMouseDown();
                        return true;
                    }
                }
                else
                {
                    if ( pFloat )
                    {
                        if ( nHitTest == HITTEST_RECT )
                        {
                            if ( pFloat->ImplIsMouseDown() )
                                pFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel );
                            return true;
                        }
                    }
                    else
                    {
                        pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
                        nPopupFlags = pLastLevelFloat->GetPopupModeFlags();
                        if ( !(nPopupFlags & FloatWinPopupFlags::NoMouseUpClose) )
                        {
                            pLastLevelFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
                            return true;
                        }
                    }
                }
            }
            else
            {
                if ( !pFloat )
                {
                    pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
                    nPopupFlags = pLastLevelFloat->GetPopupModeFlags();
                    if ( nPopupFlags & FloatWinPopupFlags::AllMouseButtonClose )
                    {
                        if ( (nPopupFlags & FloatWinPopupFlags::NoMouseUpClose) &&
                             (nSVEvent == MouseNotifyEvent::MOUSEBUTTONUP) )
                            return true;
                        pLastLevelFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
                        return true;
                    }
                    else
                        return true;
                }
            }
        }
    }

    return false;
}

static void ImplHandleMouseHelpRequest( vcl::Window* pChild, const Point& rMousePos )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maHelpData.mpHelpWin ||
         !( pSVData->maHelpData.mpHelpWin->IsWindowOrChild( pChild ) ||
           pChild->IsWindowOrChild( pSVData->maHelpData.mpHelpWin ) ) )
    {
        HelpEventMode nHelpMode = HelpEventMode::NONE;
        if ( pSVData->maHelpData.mbQuickHelp )
            nHelpMode = HelpEventMode::QUICK;
        if ( pSVData->maHelpData.mbBalloonHelp )
            nHelpMode |= HelpEventMode::BALLOON;
        if ( bool(nHelpMode) )
        {
            if ( pChild->IsInputEnabled() && !pChild->IsInModalMode() )
            {
                HelpEvent aHelpEvent( rMousePos, nHelpMode );
                pSVData->maHelpData.mbRequestingHelp = true;
                pChild->RequestHelp( aHelpEvent );
                pSVData->maHelpData.mbRequestingHelp = false;
            }
            // #104172# do not kill keyboard activated tooltips
            else if ( pSVData->maHelpData.mpHelpWin && !pSVData->maHelpData.mbKeyboardHelp)
            {
                ImplDestroyHelpWindow( true );
            }
        }
    }
}

static void ImplSetMousePointer( vcl::Window* pChild )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maHelpData.mbExtHelpMode )
        pChild->ImplGetFrame()->SetPointer( PointerStyle::Help );
    else
        pChild->ImplGetFrame()->SetPointer( pChild->ImplGetMousePointer() );
}

static bool ImplCallCommand( vcl::Window* pChild, CommandEventId nEvt, void* pData = nullptr,
                             bool bMouse = false, Point* pPos = nullptr )
{
    Point aPos;
    if ( pPos )
        aPos = *pPos;
    else
    {
        if( bMouse )
            aPos = pChild->GetPointerPosPixel();
        else
        {
            // simulate mouseposition at center of window
            Size aSize( pChild->GetOutputSizePixel() );
            aPos = Point( aSize.getWidth()/2, aSize.getHeight()/2 );
        }
    }

    CommandEvent    aCEvt( aPos, nEvt, bMouse, pData );
    NotifyEvent     aNCmdEvt( MouseNotifyEvent::COMMAND, pChild, &aCEvt );
    ImplDelData     aDelData( pChild );
    bool bPreNotify = ImplCallPreNotify( aNCmdEvt );
    if ( aDelData.IsDead() )
        return false;
    if ( !bPreNotify )
    {
        pChild->ImplGetWindowImpl()->mbCommand = false;
        pChild->Command( aCEvt );

        if( aDelData.IsDead() )
            return false;
        pChild->ImplNotifyKeyMouseCommandEventListeners( aNCmdEvt );
        if ( aDelData.IsDead() )
            return false;
        if ( pChild->ImplGetWindowImpl()->mbCommand )
            return true;
    }

    return false;
}

/*  #i34277# delayed context menu activation;
*   necessary if there already was a popup menu running.
*/

struct ContextMenuEvent
{
    VclPtr<vcl::Window>  pWindow;
    ImplDelData     aDelData;
    Point           aChildPos;
};

static void ContextMenuEventLink( void* pCEvent, void* )
{
    ContextMenuEvent* pEv = static_cast<ContextMenuEvent*>(pCEvent);

    if( ! pEv->aDelData.IsDead() )
    {
        pEv->pWindow->ImplRemoveDel( &pEv->aDelData );
        ImplCallCommand( pEv->pWindow, CommandEventId::ContextMenu, nullptr, true, &pEv->aChildPos );
    }
    delete pEv;
}

bool ImplHandleMouseEvent( vcl::Window* pWindow, MouseNotifyEvent nSVEvent, bool bMouseLeave,
                           long nX, long nY, sal_uInt64 nMsgTime,
                           sal_uInt16 nCode, MouseEventModifiers nMode )
{
    ImplSVData* pSVData = ImplGetSVData();
    Point       aMousePos( nX, nY );
    vcl::Window*     pChild(nullptr);
    bool        bRet(false);
    sal_uInt16  nClicks(0);
    ImplFrameData* pWinFrameData = pWindow->ImplGetFrameData();
    sal_uInt16      nOldCode = pWinFrameData->mnMouseCode;

    // we need a mousemove event, before we get a mousebuttondown or a
    // mousebuttonup event
    if ( (nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN) || (nSVEvent == MouseNotifyEvent::MOUSEBUTTONUP) )
    {
        if ( (nSVEvent == MouseNotifyEvent::MOUSEBUTTONUP) && pSVData->maHelpData.mbExtHelpMode )
            Help::EndExtHelp();
        if ( pSVData->maHelpData.mpHelpWin )
        {
            if( pWindow->ImplGetWindow() == pSVData->maHelpData.mpHelpWin )
            {
                ImplDestroyHelpWindow( false );
                return true; // pWindow is dead now - avoid crash!
            }
            else
                ImplDestroyHelpWindow( true );
        }

        if ( (pWinFrameData->mnLastMouseX != nX) ||
             (pWinFrameData->mnLastMouseY != nY) )
        {
            ImplHandleMouseEvent( pWindow, MouseNotifyEvent::MOUSEMOVE, false, nX, nY, nMsgTime, nCode, nMode );
        }
    }

    // update frame data
    pWinFrameData->mnBeforeLastMouseX = pWinFrameData->mnLastMouseX;
    pWinFrameData->mnBeforeLastMouseY = pWinFrameData->mnLastMouseY;
    pWinFrameData->mnLastMouseX = nX;
    pWinFrameData->mnLastMouseY = nY;
    pWinFrameData->mnMouseCode  = nCode;
    MouseEventModifiers nTmpMask = MouseEventModifiers::SYNTHETIC | MouseEventModifiers::MODIFIERCHANGED;
    pWinFrameData->mnMouseMode  = nMode & ~nTmpMask;
    if ( bMouseLeave )
    {
        pWinFrameData->mbMouseIn = false;
        if ( pSVData->maHelpData.mpHelpWin && !pSVData->maHelpData.mbKeyboardHelp )
        {
            ImplDelData aDelData( pWindow );

            ImplDestroyHelpWindow( true );

            if ( aDelData.IsDead() )
                return true; // pWindow is dead now - avoid crash! (#122045#)
        }
    }
    else
        pWinFrameData->mbMouseIn = true;

    DBG_ASSERT( !pSVData->maWinData.mpTrackWin ||
                (pSVData->maWinData.mpTrackWin == pSVData->maWinData.mpCaptureWin),
                "ImplHandleMouseEvent: TrackWin != CaptureWin" );

    // AutoScrollMode
    if ( pSVData->maWinData.mpAutoScrollWin && (nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN) )
    {
        pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();
        return true;
    }

    // find mouse window
    if ( pSVData->maWinData.mpCaptureWin )
    {
        pChild = pSVData->maWinData.mpCaptureWin;

        DBG_ASSERT( pWindow == pChild->ImplGetFrameWindow(),
                    "ImplHandleMouseEvent: mouse event is not sent to capture window" );

        // java client cannot capture mouse correctly
        if ( pWindow != pChild->ImplGetFrameWindow() )
            return false;

        if ( bMouseLeave )
            return false;
    }
    else
    {
        if ( bMouseLeave )
            pChild = nullptr;
        else
            pChild = pWindow->ImplFindWindow( aMousePos );
    }

    // test this because mouse events are buffered in the remote version
    // and size may not be in sync
    if ( !pChild && !bMouseLeave )
        return false;

    // execute a few tests and catch the message or implement the status
    if ( pChild )
    {
        if( pChild->ImplIsAntiparallel() )
        {
            // - RTL - re-mirror frame pos at pChild
            const OutputDevice *pChildWinOutDev = pChild->GetOutDev();
            pChildWinOutDev->ReMirror( aMousePos );
        }
        // no mouse messages to system object windows ?
        // !!!KA: Is it OK to comment this out? !!!
//        if ( pChild->ImplGetWindowImpl()->mpSysObj )
//            return false;

        // no mouse messages to disabled windows
        // #106845# if the window was disabled during capturing we have to pass the mouse events to release capturing
        if ( pSVData->maWinData.mpCaptureWin.get() != pChild && (!pChild->IsEnabled() || !pChild->IsInputEnabled() || pChild->IsInModalMode() ) )
        {
            ImplHandleMouseFloatMode( pChild, aMousePos, nCode, nSVEvent, bMouseLeave );
            if ( nSVEvent == MouseNotifyEvent::MOUSEMOVE )
            {
                ImplHandleMouseHelpRequest( pChild, aMousePos );
                if( pWinFrameData->mpMouseMoveWin.get() != pChild )
                    nMode |= MouseEventModifiers::ENTERWINDOW;
            }

            // Call the hook also, if Window is disabled
            Point aChildPos = pChild->ImplFrameToOutput( aMousePos );
            MouseEvent aMEvt( aChildPos, pWinFrameData->mnClickCount, nMode, nCode, nCode );
            NotifyEvent aNEvt( nSVEvent, pChild, &aMEvt );
            Application::CallEventHooks( aNEvt );

            if( pChild->IsCallHandlersOnInputDisabled() )
            {
                pWinFrameData->mpMouseMoveWin = pChild;
                pChild->ImplNotifyKeyMouseCommandEventListeners( aNEvt );
            }

            if ( nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN )
                return true;
            else
            {
                // Set normal MousePointer for disabled windows
                if ( nSVEvent == MouseNotifyEvent::MOUSEMOVE )
                    ImplSetMousePointer( pChild );

                return false;
            }
        }

        // End ExtTextInput-Mode, if the user click in the same TopLevel Window
        if ( pSVData->maWinData.mpExtTextInputWin &&
             ((nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN) ||
              (nSVEvent == MouseNotifyEvent::MOUSEBUTTONUP)) )
            pSVData->maWinData.mpExtTextInputWin->EndExtTextInput( EndExtTextInputFlags::Complete );
    }

    // determine mouse event data
    if ( nSVEvent == MouseNotifyEvent::MOUSEMOVE )
    {
        // check if MouseMove belongs to same window and if the
        // status did not change
        if ( pChild )
        {
            Point aChildMousePos = pChild->ImplFrameToOutput( aMousePos );
            if ( !bMouseLeave &&
                 (pChild == pWinFrameData->mpMouseMoveWin) &&
                 (aChildMousePos.X() == pWinFrameData->mnLastMouseWinX) &&
                 (aChildMousePos.Y() == pWinFrameData->mnLastMouseWinY) &&
                 (nOldCode == pWinFrameData->mnMouseCode) )
            {
                // set mouse pointer anew, as it could have changed
                // due to the mode switch
                ImplSetMousePointer( pChild );
                return false;
            }

            pWinFrameData->mnLastMouseWinX = aChildMousePos.X();
            pWinFrameData->mnLastMouseWinY = aChildMousePos.Y();
        }

        // mouse click
        nClicks = pWinFrameData->mnClickCount;

        // call Start-Drag handler if required
        // Warning: should be called before Move, as otherwise during
        // fast mouse movements the applications move to the selection state
        vcl::Window* pMouseDownWin = pWinFrameData->mpMouseDownWin;
        if ( pMouseDownWin )
        {
            // check for matching StartDrag mode. We only compare
            // the status of the mouse buttons, such that e. g. Mod1 can
            // change immediately to the copy mode
            const MouseSettings& rMSettings = pMouseDownWin->GetSettings().GetMouseSettings();
            if ( (nCode & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)) ==
                 (rMSettings.GetStartDragCode() & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)) )
            {
                if ( !pMouseDownWin->ImplGetFrameData()->mbStartDragCalled )
                {
                    long nDragW  = rMSettings.GetStartDragWidth();
                    long nDragH  = rMSettings.GetStartDragHeight();
                    //long nMouseX = nX;
                    //long nMouseY = nY;
                    long nMouseX = aMousePos.X(); // #106074# use the possibly re-mirrored coordinates (RTL) ! nX,nY are unmodified !
                    long nMouseY = aMousePos.Y();
                    if ( !(((nMouseX-nDragW) <= pMouseDownWin->ImplGetFrameData()->mnFirstMouseX) &&
                           ((nMouseX+nDragW) >= pMouseDownWin->ImplGetFrameData()->mnFirstMouseX)) ||
                         !(((nMouseY-nDragH) <= pMouseDownWin->ImplGetFrameData()->mnFirstMouseY) &&
                           ((nMouseY+nDragH) >= pMouseDownWin->ImplGetFrameData()->mnFirstMouseY)) )
                    {
                        pMouseDownWin->ImplGetFrameData()->mbStartDragCalled  = true;

                        // Check if drag source provides it's own recognizer
                        if( pMouseDownWin->ImplGetFrameData()->mbInternalDragGestureRecognizer )
                        {
                            // query DropTarget from child window
                            css::uno::Reference< css::datatransfer::dnd::XDragGestureRecognizer > xDragGestureRecognizer =
                                css::uno::Reference< css::datatransfer::dnd::XDragGestureRecognizer > ( pMouseDownWin->ImplGetWindowImpl()->mxDNDListenerContainer,
                                    css::uno::UNO_QUERY );

                            if( xDragGestureRecognizer.is() )
                            {
                                // retrieve mouse position relative to mouse down window
                                Point relLoc = pMouseDownWin->ImplFrameToOutput( Point(
                                    pMouseDownWin->ImplGetFrameData()->mnFirstMouseX,
                                    pMouseDownWin->ImplGetFrameData()->mnFirstMouseY ) );

                                // create a uno mouse event out of the available data
                                css::awt::MouseEvent aMouseEvent( static_cast < css::uno::XInterface * > ( nullptr ),
#ifdef MACOSX
                                    nCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_MOD3),
#else
                                    nCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2),
#endif
                                    nCode & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE),
                                    nMouseX,
                                    nMouseY,
                                    nClicks,
                                    false );

                                SolarMutexReleaser aReleaser;

                                // FIXME: where do I get Action from ?
                                css::uno::Reference< css::datatransfer::dnd::XDragSource > xDragSource = pMouseDownWin->GetDragSource();

                                if( xDragSource.is() )
                                {
                                    static_cast < DNDListenerContainer * > ( xDragGestureRecognizer.get() )->fireDragGestureEvent( 0,
                                        relLoc.X(), relLoc.Y(), xDragSource, css::uno::makeAny( aMouseEvent ) );
                                }
                            }
                        }
                    }
                }
            }
            else
                pMouseDownWin->ImplGetFrameData()->mbStartDragCalled  = true;
        }

        // test for mouseleave and mouseenter
        vcl::Window* pMouseMoveWin = pWinFrameData->mpMouseMoveWin;
        if ( pChild != pMouseMoveWin )
        {
            if ( pMouseMoveWin )
            {
                Point       aLeaveMousePos = pMouseMoveWin->ImplFrameToOutput( aMousePos );
                MouseEvent  aMLeaveEvt( aLeaveMousePos, nClicks, nMode | MouseEventModifiers::LEAVEWINDOW, nCode, nCode );
                NotifyEvent aNLeaveEvt( MouseNotifyEvent::MOUSEMOVE, pMouseMoveWin, &aMLeaveEvt );
                ImplDelData aDelData;
                ImplDelData aDelData2;
                pWinFrameData->mbInMouseMove = true;
                pMouseMoveWin->ImplGetWinData()->mbMouseOver = false;
                pMouseMoveWin->ImplAddDel( &aDelData );

                // A MouseLeave can destroy this window
                if ( pChild )
                    pChild->ImplAddDel( &aDelData2 );
                if ( !ImplCallPreNotify( aNLeaveEvt ) )
                {
                    pMouseMoveWin->MouseMove( aMLeaveEvt );
                    if( !aDelData.IsDead() )
                        aNLeaveEvt.GetWindow()->ImplNotifyKeyMouseCommandEventListeners( aNLeaveEvt );
                }

                pWinFrameData->mpMouseMoveWin = nullptr;
                pWinFrameData->mbInMouseMove = false;

                if ( pChild )
                {
                    if ( aDelData2.IsDead() )
                        pChild = nullptr;
                    else
                        pChild->ImplRemoveDel( &aDelData2 );
                }
                if ( aDelData.IsDead() )
                    return true;
                pMouseMoveWin->ImplRemoveDel( &aDelData );
            }

            nMode |= MouseEventModifiers::ENTERWINDOW;
        }
        pWinFrameData->mpMouseMoveWin = pChild;
        if( pChild )
            pChild->ImplGetWinData()->mbMouseOver = true;

        // MouseLeave
        if ( !pChild )
            return false;
    }
    else
    {
        if (pChild)
        {
            // mouse click
            if ( nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN )
            {
                const MouseSettings& rMSettings = pChild->GetSettings().GetMouseSettings();
                sal_uInt64 nDblClkTime = rMSettings.GetDoubleClickTime();
                long    nDblClkW    = rMSettings.GetDoubleClickWidth();
                long    nDblClkH    = rMSettings.GetDoubleClickHeight();
                //long    nMouseX     = nX;
                //long    nMouseY     = nY;
                long nMouseX = aMousePos.X();   // #106074# use the possibly re-mirrored coordinates (RTL) ! nX,nY are unmodified !
                long nMouseY = aMousePos.Y();

                if ( (pChild == pChild->ImplGetFrameData()->mpMouseDownWin) &&
                     (nCode == pChild->ImplGetFrameData()->mnFirstMouseCode) &&
                     ((nMsgTime-pChild->ImplGetFrameData()->mnMouseDownTime) < nDblClkTime) &&
                     ((nMouseX-nDblClkW) <= pChild->ImplGetFrameData()->mnFirstMouseX) &&
                     ((nMouseX+nDblClkW) >= pChild->ImplGetFrameData()->mnFirstMouseX) &&
                     ((nMouseY-nDblClkH) <= pChild->ImplGetFrameData()->mnFirstMouseY) &&
                     ((nMouseY+nDblClkH) >= pChild->ImplGetFrameData()->mnFirstMouseY) )
                {
                    pChild->ImplGetFrameData()->mnClickCount++;
                    pChild->ImplGetFrameData()->mbStartDragCalled  = true;
                }
                else
                {
                    pChild->ImplGetFrameData()->mpMouseDownWin     = pChild;
                    pChild->ImplGetFrameData()->mnClickCount       = 1;
                    pChild->ImplGetFrameData()->mnFirstMouseX      = nMouseX;
                    pChild->ImplGetFrameData()->mnFirstMouseY      = nMouseY;
                    pChild->ImplGetFrameData()->mnFirstMouseCode   = nCode;
                    pChild->ImplGetFrameData()->mbStartDragCalled  = !((nCode & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)) ==
                                                                (rMSettings.GetStartDragCode() & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)));
                }
                pChild->ImplGetFrameData()->mnMouseDownTime = nMsgTime;
            }
            nClicks = pChild->ImplGetFrameData()->mnClickCount;
        }

        pSVData->maAppData.mnLastInputTime = tools::Time::GetSystemTicks();
    }

    DBG_ASSERT( pChild, "ImplHandleMouseEvent: pChild == NULL" );

    if (!pChild)
        return false;

    // create mouse event
    Point aChildPos = pChild->ImplFrameToOutput( aMousePos );
    MouseEvent aMEvt( aChildPos, nClicks, nMode, nCode, nCode );


    // tracking window gets the mouse events
    if ( pSVData->maWinData.mpTrackWin )
        pChild = pSVData->maWinData.mpTrackWin;

    // handle FloatingMode
    if ( !pSVData->maWinData.mpTrackWin && pSVData->maWinData.mpFirstFloat )
    {
        ImplDelData aDelData;
        pChild->ImplAddDel( &aDelData );
        if ( ImplHandleMouseFloatMode( pChild, aMousePos, nCode, nSVEvent, bMouseLeave ) )
        {
            if ( !aDelData.IsDead() )
            {
                pChild->ImplRemoveDel( &aDelData );
                pChild->ImplGetFrameData()->mbStartDragCalled = true;
            }
            return true;
        }
        else
            pChild->ImplRemoveDel( &aDelData );
    }

    // call handler
    bool bDrag = false;
    bool bCallHelpRequest = true;
    DBG_ASSERT( pChild, "ImplHandleMouseEvent: pChild is NULL" );

    if (!pChild)
        return false;

    ImplDelData aDelData;
    NotifyEvent aNEvt( nSVEvent, pChild, &aMEvt );
    pChild->ImplAddDel( &aDelData );
    if ( nSVEvent == MouseNotifyEvent::MOUSEMOVE )
        pChild->ImplGetFrameData()->mbInMouseMove = true;

    // bring window into foreground on mouseclick
    if ( nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN )
    {
        if( !pSVData->maWinData.mpFirstFloat && // totop for floating windows in popup would change the focus and would close them immediately
            !(pChild->ImplGetFrameWindow()->GetStyle() & WB_OWNERDRAWDECORATION) )    // ownerdrawdecorated windows must never grab focus
            pChild->ToTop();
        if ( aDelData.IsDead() )
            return true;
    }

    if ( ImplCallPreNotify( aNEvt ) || aDelData.IsDead() )
        bRet = true;
    else
    {
        bRet = false;
        if ( nSVEvent == MouseNotifyEvent::MOUSEMOVE )
        {
            if ( pSVData->maWinData.mpTrackWin )
            {
                TrackingEvent aTEvt( aMEvt );
                pChild->Tracking( aTEvt );
                if ( !aDelData.IsDead() )
                {
                    // When ScrollRepeat, we restart the timer
                    if ( pSVData->maWinData.mpTrackTimer &&
                         (pSVData->maWinData.mnTrackFlags & StartTrackingFlags::ScrollRepeat) )
                        pSVData->maWinData.mpTrackTimer->Start();
                }
                bCallHelpRequest = false;
                bRet = true;
            }
            else
            {
                // Auto-ToTop
                if ( !pSVData->maWinData.mpCaptureWin &&
                     (pChild->GetSettings().GetMouseSettings().GetOptions() & MouseSettingsOptions::AutoFocus) )
                    pChild->ToTop( ToTopFlags::NoGrabFocus );

                if( aDelData.IsDead() )
                    bCallHelpRequest = false;
                else
                {
                    // if the MouseMove handler changes the help window's visibility
                    // the HelpRequest handler should not be called anymore
                    vcl::Window* pOldHelpTextWin = pSVData->maHelpData.mpHelpWin;
                    pChild->ImplGetWindowImpl()->mbMouseMove = false;
                    pChild->MouseMove( aMEvt );
                    if ( pOldHelpTextWin != pSVData->maHelpData.mpHelpWin )
                        bCallHelpRequest = false;
                }
            }
        }
        else if ( nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN )
        {
            if ( pSVData->maWinData.mpTrackWin &&
                 !(pSVData->maWinData.mnTrackFlags & StartTrackingFlags::MouseButtonDown) )
                bRet = true;
            else
            {
                pChild->ImplGetWindowImpl()->mbMouseButtonDown = false;
                pChild->MouseButtonDown( aMEvt );
            }
        }
        else
        {
            if ( pSVData->maWinData.mpTrackWin )
            {
                pChild->EndTracking();
                bRet = true;
            }
            else
            {
                pChild->ImplGetWindowImpl()->mbMouseButtonUp = false;
                pChild->MouseButtonUp( aMEvt );
            }
        }

        assert(aNEvt.GetWindow() == pChild);

        if (!pChild->isDisposed() || !aDelData.IsDead())
            pChild->ImplNotifyKeyMouseCommandEventListeners( aNEvt );
    }

    if (pChild->isDisposed() || aDelData.IsDead())
        return true;

    if ( nSVEvent == MouseNotifyEvent::MOUSEMOVE )
        pChild->ImplGetWindowImpl()->mpFrameData->mbInMouseMove = false;

    if ( nSVEvent == MouseNotifyEvent::MOUSEMOVE )
    {
        if ( bCallHelpRequest && !pSVData->maHelpData.mbKeyboardHelp )
            ImplHandleMouseHelpRequest( pChild, pChild->OutputToScreenPixel( aMEvt.GetPosPixel() ) );
        bRet = true;
    }
    else if ( !bRet )
    {
        if ( nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN )
        {
            if ( !pChild->ImplGetWindowImpl()->mbMouseButtonDown )
                bRet = true;
        }
        else
        {
            if ( !pChild->ImplGetWindowImpl()->mbMouseButtonUp )
                bRet = true;
        }
    }

    pChild->ImplRemoveDel( &aDelData );

    if ( nSVEvent == MouseNotifyEvent::MOUSEMOVE )
    {
        // set new mouse pointer
        if ( !bMouseLeave )
            ImplSetMousePointer( pChild );
    }
    else if ( (nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN) || (nSVEvent == MouseNotifyEvent::MOUSEBUTTONUP) )
    {
        if ( !bDrag )
        {
            // Command-Events
            if ( /*!bRet &&*/ (nClicks == 1) && (nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN) &&
                 (nCode == MOUSE_MIDDLE) )
            {
                MouseMiddleButtonAction nMiddleAction = pChild->GetSettings().GetMouseSettings().GetMiddleButtonAction();
                if ( nMiddleAction == MouseMiddleButtonAction::AutoScroll )
                    bRet = !ImplCallCommand( pChild, CommandEventId::StartAutoScroll, nullptr, true, &aChildPos );
                else if ( nMiddleAction == MouseMiddleButtonAction::PasteSelection )
                    bRet = !ImplCallCommand( pChild, CommandEventId::PasteSelection, nullptr, true, &aChildPos );
            }
            else
            {
                // ContextMenu
                const MouseSettings& rMSettings = pChild->GetSettings().GetMouseSettings();
                if ( (nCode == rMSettings.GetContextMenuCode()) &&
                     (nClicks == rMSettings.GetContextMenuClicks()) )
                {
                    bool bContextMenu = (nSVEvent == MouseNotifyEvent::MOUSEBUTTONDOWN);
                    if ( bContextMenu )
                    {
                        if( pSVData->maAppData.mpActivePopupMenu )
                        {
                            /*  #i34277# there already is a context menu open
                            *   that was probably just closed with EndPopupMode.
                            *   We need to give the eventual corresponding
                            *   PopupMenu::Execute a chance to end properly.
                            *   Therefore delay context menu command and
                            *   issue only after popping one frame of the
                            *   Yield stack.
                            */
                            ContextMenuEvent* pEv = new ContextMenuEvent;
                            pEv->pWindow = pChild;
                            pEv->aChildPos = aChildPos;
                            pChild->ImplAddDel( &pEv->aDelData );
                            Application::PostUserEvent( Link<void*,void>( pEv, ContextMenuEventLink ) );
                        }
                        else
                            bRet = ! ImplCallCommand( pChild, CommandEventId::ContextMenu, nullptr, true, &aChildPos );
                    }
                }
            }
        }
    }

    return bRet;
}

static vcl::Window* ImplGetKeyInputWindow( vcl::Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();

    // determine last input time
    pSVData->maAppData.mnLastInputTime = tools::Time::GetSystemTicks();

    // #127104# workaround for destroyed windows
    if( pWindow->ImplGetWindowImpl() == nullptr )
        return nullptr;

    // find window - is every time the window which has currently the
    // focus or the last time the focus.
    // the first floating window always has the focus
    vcl::Window* pChild = pSVData->maWinData.mpFirstFloat;
    if( !pChild || ( pChild->ImplGetWindowImpl()->mbFloatWin && !static_cast<FloatingWindow *>(pChild)->GrabsFocus() ) )
        pChild = pWindow->ImplGetWindowImpl()->mpFrameData->mpFocusWin;
    else
    {
        // allow floaters to forward keyinput to some member
        pChild = pChild->GetPreferredKeyInputWindow();
    }

    // no child - than no input
    if ( !pChild )
        return nullptr;

    // We call also KeyInput if we haven't the focus, because on Unix
    // system this is often the case when a Lookup Choice Window has
    // the focus - because this windows send the KeyInput directly to
    // the window without resetting the focus
    SAL_WARN_IF( pChild != pSVData->maWinData.mpFocusWin, "vcl",
                       "ImplHandleKey: Keyboard-Input is sent to a frame without focus" );

    // no keyinput to disabled windows
    if ( !pChild->IsEnabled() || !pChild->IsInputEnabled() || pChild->IsInModalMode() )
        return nullptr;

    return pChild;
}

static bool ImplHandleKey( vcl::Window* pWindow, MouseNotifyEvent nSVEvent,
                           sal_uInt16 nKeyCode, sal_uInt16 nCharCode, sal_uInt16 nRepeat, bool bForward )
{
    ImplSVData* pSVData = ImplGetSVData();
    vcl::KeyCode aKeyCode( nKeyCode, nKeyCode );
    sal_uInt16 nEvCode = aKeyCode.GetCode();

    // allow application key listeners to remove the key event
    // but make sure we're not forwarding external KeyEvents, (ie where bForward is false)
    // because those are coming back from the listener itself and MUST be processed
    KeyEvent aKeyEvent( (sal_Unicode)nCharCode, aKeyCode, nRepeat );
    if( bForward )
    {
        sal_uInt16 nVCLEvent;
        switch( nSVEvent )
        {
            case MouseNotifyEvent::KEYINPUT:
                nVCLEvent = VCLEVENT_WINDOW_KEYINPUT;
                break;
            case MouseNotifyEvent::KEYUP:
                nVCLEvent = VCLEVENT_WINDOW_KEYUP;
                break;
            default:
                nVCLEvent = 0;
                break;
        }
        if( nVCLEvent && Application::HandleKey( nVCLEvent, pWindow, &aKeyEvent ) )
            return true;
    }

    // #i1820# use locale specific decimal separator
    if( nEvCode == KEY_DECIMAL )
    {
        if( Application::GetSettings().GetMiscSettings().GetEnableLocalizedDecimalSep() )
        {
            OUString aSep( pWindow->GetSettings().GetLocaleDataWrapper().getNumDecimalSep() );
            nCharCode = (sal_uInt16) aSep[0];
        }
    }

    bool bCtrlF6 = (aKeyCode.GetCode() == KEY_F6) && aKeyCode.IsMod1();

    // determine last input time
    pSVData->maAppData.mnLastInputTime = tools::Time::GetSystemTicks();

    // handle tracking window
    if ( nSVEvent == MouseNotifyEvent::KEYINPUT )
    {
        if ( pSVData->maHelpData.mbExtHelpMode )
        {
            Help::EndExtHelp();
            if ( nEvCode == KEY_ESCAPE )
                return true;
        }
        if ( pSVData->maHelpData.mpHelpWin )
            ImplDestroyHelpWindow( false );

        // AutoScrollMode
        if ( pSVData->maWinData.mpAutoScrollWin )
        {
            pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();
            if ( nEvCode == KEY_ESCAPE )
                return true;
        }

        if ( pSVData->maWinData.mpTrackWin )
        {
            sal_uInt16 nOrigCode = aKeyCode.GetCode();

            if ( (nOrigCode == KEY_ESCAPE) && !(pSVData->maWinData.mnTrackFlags & StartTrackingFlags::NoKeyCancel) )
            {
                pSVData->maWinData.mpTrackWin->EndTracking( TrackingEventFlags::Cancel | TrackingEventFlags::Key );
                if ( pSVData->maWinData.mpFirstFloat )
                {
                    FloatingWindow* pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
                    if ( !(pLastLevelFloat->GetPopupModeFlags() & FloatWinPopupFlags::NoKeyClose) )
                    {
                        sal_uInt16 nEscCode = aKeyCode.GetCode();

                        if ( nEscCode == KEY_ESCAPE )
                            pLastLevelFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
                    }
                }
                return true;
            }
            else if ( nOrigCode == KEY_RETURN )
            {
                pSVData->maWinData.mpTrackWin->EndTracking( TrackingEventFlags::Key );
                return true;
            }
            else if ( !(pSVData->maWinData.mnTrackFlags & StartTrackingFlags::KeyInput) )
                return true;
        }

        // handle FloatingMode
        if ( pSVData->maWinData.mpFirstFloat )
        {
            FloatingWindow* pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
            if ( !(pLastLevelFloat->GetPopupModeFlags() & FloatWinPopupFlags::NoKeyClose) )
            {
                sal_uInt16 nCode = aKeyCode.GetCode();

                if ( (nCode == KEY_ESCAPE) || bCtrlF6)
                {
                    pLastLevelFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
                    if( !bCtrlF6 )
                        return true;
                }
            }
        }

        // test for accel
        if ( pSVData->maAppData.mpAccelMgr )
        {
            if ( pSVData->maAppData.mpAccelMgr->IsAccelKey( aKeyCode, nRepeat ) )
                return true;
        }
    }

    // find window
    vcl::Window* pChild = ImplGetKeyInputWindow( pWindow );
    if ( !pChild )
        return false;

    // --- RTL --- mirror cursor keys
    const OutputDevice *pChildOutDev = pChild->GetOutDev();
    if( (aKeyCode.GetCode() == KEY_LEFT || aKeyCode.GetCode() == KEY_RIGHT) &&
      pChildOutDev->HasMirroredGraphics() && pChild->IsRTLEnabled() )
        aKeyCode = vcl::KeyCode( aKeyCode.GetCode() == KEY_LEFT ? KEY_RIGHT : KEY_LEFT, aKeyCode.GetModifier() );

    // call handler
    ImplDelData aDelData;
    pChild->ImplAddDel( &aDelData );

    KeyEvent    aKeyEvt( (sal_Unicode)nCharCode, aKeyCode, nRepeat );
    NotifyEvent aNotifyEvt( nSVEvent, pChild, &aKeyEvt );
    bool bKeyPreNotify = ImplCallPreNotify( aNotifyEvt );
    bool bRet = true;

    if ( !bKeyPreNotify && !aDelData.IsDead() )
    {
        if ( nSVEvent == MouseNotifyEvent::KEYINPUT )
        {
            pChild->ImplGetWindowImpl()->mbKeyInput = false;
            pChild->KeyInput( aKeyEvt );
        }
        else
        {
            pChild->ImplGetWindowImpl()->mbKeyUp = false;
            pChild->KeyUp( aKeyEvt );
        }
        if( !aDelData.IsDead() )
            aNotifyEvt.GetWindow()->ImplNotifyKeyMouseCommandEventListeners( aNotifyEvt );
    }

    if ( aDelData.IsDead() )
        return true;

    pChild->ImplRemoveDel( &aDelData );

    if ( nSVEvent == MouseNotifyEvent::KEYINPUT )
    {
        if ( !bKeyPreNotify && pChild->ImplGetWindowImpl()->mbKeyInput )
        {
            sal_uInt16 nCode = aKeyCode.GetCode();

            // #101999# is focus in or below toolbox
            bool bToolboxFocus=false;
            if( (nCode == KEY_F1) && aKeyCode.IsShift() )
            {
                vcl::Window *pWin = pWindow->ImplGetWindowImpl()->mpFrameData->mpFocusWin;
                while( pWin )
                {
                    if( pWin->ImplGetWindowImpl()->mbToolBox )
                    {
                        bToolboxFocus = true;
                        break;
                    }
                    else
                        pWin = pWin->GetParent();
                }
            }

            // ContextMenu
            if ( (nCode == KEY_CONTEXTMENU) || ((nCode == KEY_F10) && aKeyCode.IsShift() && !aKeyCode.IsMod1() && !aKeyCode.IsMod2() ) )
                bRet = !ImplCallCommand( pChild, CommandEventId::ContextMenu );
            else if ( ( (nCode == KEY_F2) && aKeyCode.IsShift() ) || ( (nCode == KEY_F1) && aKeyCode.IsMod1() ) ||
                // #101999# no active help when focus in toolbox, simulate BallonHelp instead
                ( (nCode == KEY_F1) && aKeyCode.IsShift() && bToolboxFocus ) )
            {
                // TipHelp via Keyboard (Shift-F2 or Ctrl-F1)
                // simulate mouseposition at center of window

                Size aSize = pChild->GetOutputSize();
                Point aPos = Point( aSize.getWidth()/2, aSize.getHeight()/2 );
                aPos = pChild->OutputToScreenPixel( aPos );

                HelpEvent aHelpEvent( aPos, HelpEventMode::BALLOON );
                aHelpEvent.SetKeyboardActivated( true );
                pSVData->maHelpData.mbSetKeyboardHelp = true;
                pChild->RequestHelp( aHelpEvent );
                pSVData->maHelpData.mbSetKeyboardHelp = false;
            }
            else if ( (nCode == KEY_F1) || (nCode == KEY_HELP) )
            {
                if ( !aKeyCode.GetModifier() )
                {
                    if ( pSVData->maHelpData.mbContextHelp )
                    {
                        Point       aMousePos = pChild->OutputToScreenPixel( pChild->GetPointerPosPixel() );
                        HelpEvent   aHelpEvent( aMousePos, HelpEventMode::CONTEXT );
                        pChild->RequestHelp( aHelpEvent );
                    }
                    else
                        bRet = false;
                }
                else if ( aKeyCode.IsShift() )
                {
                    if ( pSVData->maHelpData.mbExtHelp )
                        Help::StartExtHelp();
                    else
                        bRet = false;
                }
            }
            else
            {
                if ( ImplCallHotKey( aKeyCode ) )
                    bRet = true;
                else
                    bRet = false;
            }
        }
    }
    else
    {
        if ( !bKeyPreNotify && pChild->ImplGetWindowImpl()->mbKeyUp )
            bRet = false;
    }

    // #105591# send keyinput to parent if we are a floating window and the key was not processed yet
    if( !bRet && pWindow->ImplGetWindowImpl()->mbFloatWin && pWindow->GetParent() && (pWindow->ImplGetWindowImpl()->mpFrame != pWindow->GetParent()->ImplGetWindowImpl()->mpFrame) )
    {
        pChild = pWindow->GetParent();

        // call handler
        ImplDelData aChildDelData( pChild );
        KeyEvent    aKEvt( (sal_Unicode)nCharCode, aKeyCode, nRepeat );
        NotifyEvent aNEvt( nSVEvent, pChild, &aKEvt );
        bool bPreNotify = ImplCallPreNotify( aNEvt );
        if ( aChildDelData.IsDead() )
            return true;

        if ( !bPreNotify )
        {
            if ( nSVEvent == MouseNotifyEvent::KEYINPUT )
            {
                pChild->ImplGetWindowImpl()->mbKeyInput = false;
                pChild->KeyInput( aKEvt );
            }
            else
            {
                pChild->ImplGetWindowImpl()->mbKeyUp = false;
                pChild->KeyUp( aKEvt );
            }

            if( !aChildDelData.IsDead() )
                aNEvt.GetWindow()->ImplNotifyKeyMouseCommandEventListeners( aNEvt );
            if ( aChildDelData.IsDead() )
                return true;
        }

        if( bPreNotify || !pChild->ImplGetWindowImpl()->mbKeyInput )
            bRet = true;
    }

    return bRet;
}

static bool ImplHandleExtTextInput( vcl::Window* pWindow,
                                    const OUString& rText,
                                    const sal_uInt16* pTextAttr,
                                    sal_Int32 nCursorPos, sal_uInt16 nCursorFlags )
{
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window*     pChild = nullptr;

    int nTries = 200;
    while( nTries-- )
    {
        pChild = pSVData->maWinData.mpExtTextInputWin;
        if ( !pChild )
        {
            pChild = ImplGetKeyInputWindow( pWindow );
            if ( !pChild )
                return false;
        }
        if( !pChild->ImplGetWindowImpl()->mpFrameData->mnFocusId )
            break;
        Application::Yield();
    }

    // If it is the first ExtTextInput call, we inform the information
    // and allocate the data, which we must store in this mode
    ImplWinData* pWinData = pChild->ImplGetWinData();
    if ( !pChild->ImplGetWindowImpl()->mbExtTextInput )
    {
        pChild->ImplGetWindowImpl()->mbExtTextInput = true;
        pWinData->mpExtOldText = new OUString;
        if ( pWinData->mpExtOldAttrAry )
        {
            delete [] pWinData->mpExtOldAttrAry;
            pWinData->mpExtOldAttrAry = nullptr;
        }
        pSVData->maWinData.mpExtTextInputWin = pChild;
        ImplCallCommand( pChild, CommandEventId::StartExtTextInput );
    }

    // be aware of being recursively called in StartExtTextInput
    if ( !pChild->ImplGetWindowImpl()->mbExtTextInput )
        return false;

    // Test for changes
    bool bOnlyCursor = false;
    sal_Int32 nMinLen = std::min( pWinData->mpExtOldText->getLength(), rText.getLength() );
    sal_Int32 nDeltaStart = 0;
    while ( nDeltaStart < nMinLen )
    {
        if ( (*pWinData->mpExtOldText)[nDeltaStart] != rText[nDeltaStart] )
            break;
        nDeltaStart++;
    }
    if ( pWinData->mpExtOldAttrAry || pTextAttr )
    {
        if ( !pWinData->mpExtOldAttrAry || !pTextAttr )
            nDeltaStart = 0;
        else
        {
            sal_Int32 i = 0;
            while ( i < nDeltaStart )
            {
                if ( pWinData->mpExtOldAttrAry[i] != pTextAttr[i] )
                {
                    nDeltaStart = i;
                    break;
                }
                i++;
            }
        }
    }
    if ( (nDeltaStart >= nMinLen) &&
         (pWinData->mpExtOldText->getLength() == rText.getLength()) )
        bOnlyCursor = true;

    // Call Event and store the information
    CommandExtTextInputData aData( rText, pTextAttr,
                                   nCursorPos, nCursorFlags,
                                   bOnlyCursor );
    *pWinData->mpExtOldText = rText;
    if ( pWinData->mpExtOldAttrAry )
    {
        delete [] pWinData->mpExtOldAttrAry;
        pWinData->mpExtOldAttrAry = nullptr;
    }
    if ( pTextAttr )
    {
        pWinData->mpExtOldAttrAry = new sal_uInt16[rText.getLength()];
        memcpy( pWinData->mpExtOldAttrAry, pTextAttr, rText.getLength()*sizeof( sal_uInt16 ) );
    }
    return !ImplCallCommand( pChild, CommandEventId::ExtTextInput, &aData );
}

static bool ImplHandleEndExtTextInput( vcl::Window* /* pWindow */ )
{
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window*     pChild = pSVData->maWinData.mpExtTextInputWin;
    bool        nRet = false;

    if ( pChild )
    {
        pChild->ImplGetWindowImpl()->mbExtTextInput = false;
        pSVData->maWinData.mpExtTextInputWin = nullptr;
        ImplWinData* pWinData = pChild->ImplGetWinData();
        if ( pWinData->mpExtOldText )
        {
            delete pWinData->mpExtOldText;
            pWinData->mpExtOldText = nullptr;
        }
        if ( pWinData->mpExtOldAttrAry )
        {
            delete [] pWinData->mpExtOldAttrAry;
            pWinData->mpExtOldAttrAry = nullptr;
        }
        nRet = !ImplCallCommand( pChild, CommandEventId::EndExtTextInput );
    }

    return nRet;
}

static void ImplHandleExtTextInputPos( vcl::Window* pWindow,
                                       Rectangle& rRect, long& rInputWidth,
                                       bool * pVertical )
{
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window*     pChild = pSVData->maWinData.mpExtTextInputWin;

    if ( !pChild )
        pChild = ImplGetKeyInputWindow( pWindow );
    else
    {
        // Test, if the Window is related to the frame
        if ( !pWindow->ImplIsWindowOrChild( pChild ) )
            pChild = ImplGetKeyInputWindow( pWindow );
    }

    if ( pChild )
    {
        const OutputDevice *pChildOutDev = pChild->GetOutDev();
        ImplCallCommand( pChild, CommandEventId::CursorPos );
        const Rectangle* pRect = pChild->GetCursorRect();
        if ( pRect )
            rRect = pChildOutDev->ImplLogicToDevicePixel( *pRect );
        else
        {
            vcl::Cursor* pCursor = pChild->GetCursor();
            if ( pCursor )
            {
                Point aPos = pChildOutDev->ImplLogicToDevicePixel( pCursor->GetPos() );
                Size aSize = pChild->LogicToPixel( pCursor->GetSize() );
                if ( !aSize.Width() )
                    aSize.Width() = pChild->GetSettings().GetStyleSettings().GetCursorSize();
                rRect = Rectangle( aPos, aSize );
            }
            else
                rRect = Rectangle( Point( pChild->GetOutOffXPixel(), pChild->GetOutOffYPixel() ), Size() );
        }
        rInputWidth = pChild->ImplLogicWidthToDevicePixel( pChild->GetCursorExtTextInputWidth() );
        if ( !rInputWidth )
            rInputWidth = rRect.GetWidth();
    }
    if (pVertical != nullptr)
        *pVertical
            = pChild != nullptr && pChild->GetInputContext().GetFont().IsVertical();
}

static bool ImplHandleInputContextChange( vcl::Window* pWindow, LanguageType eNewLang )
{
    vcl::Window* pChild = ImplGetKeyInputWindow( pWindow );
    CommandInputContextData aData( eNewLang );
    return !ImplCallCommand( pChild, CommandEventId::InputContextChange, &aData );
}

static bool ImplCallWheelCommand( vcl::Window* pWindow, const Point& rPos,
                                  const CommandWheelData* pWheelData )
{
    Point               aCmdMousePos = pWindow->ImplFrameToOutput( rPos );
    CommandEvent        aCEvt( aCmdMousePos, CommandEventId::Wheel, true, pWheelData );
    NotifyEvent         aNCmdEvt( MouseNotifyEvent::COMMAND, pWindow, &aCEvt );
    ImplDelData         aDelData( pWindow );
    bool bPreNotify = ImplCallPreNotify( aNCmdEvt );
    if ( aDelData.IsDead() )
        return false;
    if ( !bPreNotify )
    {
        pWindow->ImplGetWindowImpl()->mbCommand = false;
        pWindow->Command( aCEvt );
        if ( aDelData.IsDead() )
            return false;
        if ( pWindow->ImplGetWindowImpl()->mbCommand )
            return true;
    }
    return false;
}

static bool acceptableWheelScrollTarget(const vcl::Window *pMouseWindow)
{
    return (pMouseWindow && !pMouseWindow->isDisposed() && pMouseWindow->IsInputEnabled() && !pMouseWindow->IsInModalMode());
}

//If the last event at the same absolute screen position was handled by a
//different window then reuse that window if the event occurs within 1/2 a
//second, i.e. so scrolling down something like the calc sidebar that contains
//widgets that respond to wheel events will continue to send the event to the
//scrolling widget in favour of the widget that happens to end up under the
//mouse.
static bool shouldReusePreviousMouseWindow(const SalWheelMouseEvent& rPrevEvt, const SalWheelMouseEvent& rEvt)
{
    return (rEvt.mnX == rPrevEvt.mnX && rEvt.mnY == rPrevEvt.mnY && rEvt.mnTime-rPrevEvt.mnTime < 500/*ms*/);
}

class HandleGestureEventBase
{
protected:
    ImplSVData* m_pSVData;
    VclPtr<vcl::Window> m_pWindow;
    Point m_aMousePos;

public:
    HandleGestureEventBase(vcl::Window *pWindow, const Point &rMousePos)
        : m_pSVData(ImplGetSVData())
        , m_pWindow(pWindow)
        , m_aMousePos(rMousePos)
    {
    }
    bool Setup();
    vcl::Window* FindTarget();
    vcl::Window* Dispatch(vcl::Window* pTarget);
    virtual bool CallCommand(vcl::Window *pWindow, const Point &rMousePos) = 0;
    virtual ~HandleGestureEventBase() {}
};

bool HandleGestureEventBase::Setup()
{
    ImplDelData aDogTag( m_pWindow );

    if (m_pSVData->maWinData.mpAutoScrollWin)
        m_pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();
    if (m_pSVData->maHelpData.mpHelpWin)
        ImplDestroyHelpWindow( true );
    if (aDogTag.IsDead())
        return false;
    return true;
}

vcl::Window* HandleGestureEventBase::FindTarget()
{
    // first check any floating window ( eg. drop down listboxes)
    vcl::Window *pMouseWindow = nullptr;

    if (m_pSVData->maWinData.mpFirstFloat && !m_pSVData->maWinData.mpCaptureWin &&
         !m_pSVData->maWinData.mpFirstFloat->ImplIsFloatPopupModeWindow( m_pWindow ) )
    {
        HitTest nHitTest = HITTEST_OUTSIDE;
        pMouseWindow = m_pSVData->maWinData.mpFirstFloat->ImplFloatHitTest( m_pWindow, m_aMousePos, nHitTest );
        if (!pMouseWindow)
            pMouseWindow = m_pSVData->maWinData.mpFirstFloat;
    }
    // then try the window directly beneath the mouse
    if( !pMouseWindow )
    {
        pMouseWindow = m_pWindow->ImplFindWindow( m_aMousePos );
    }
    else
    {
        // transform coordinates to float window frame coordinates
        pMouseWindow = pMouseWindow->ImplFindWindow(
                 pMouseWindow->OutputToScreenPixel(
                  pMouseWindow->AbsoluteScreenToOutputPixel(
                   m_pWindow->OutputToAbsoluteScreenPixel(
                    m_pWindow->ScreenToOutputPixel( m_aMousePos ) ) ) ) );
    }

    while (acceptableWheelScrollTarget(pMouseWindow))
    {
        if (pMouseWindow->IsEnabled())
            break;
        //try the parent if this one is disabled
        pMouseWindow = pMouseWindow->GetParent();
    }

    return pMouseWindow;
}

vcl::Window *HandleGestureEventBase::Dispatch(vcl::Window* pMouseWindow)
{
    vcl::Window *pDispatchedTo = nullptr;

    if (acceptableWheelScrollTarget(pMouseWindow) && pMouseWindow->IsEnabled())
    {
        // transform coordinates to float window frame coordinates
        Point aRelMousePos( pMouseWindow->OutputToScreenPixel(
                             pMouseWindow->AbsoluteScreenToOutputPixel(
                              m_pWindow->OutputToAbsoluteScreenPixel(
                               m_pWindow->ScreenToOutputPixel( m_aMousePos ) ) ) ) );
        bool bPropogate = CallCommand(pMouseWindow, aRelMousePos);
        if (!bPropogate)
            pDispatchedTo = pMouseWindow;
    }

    // if the command was not handled try the focus window
    if (!pDispatchedTo)
    {
        vcl::Window* pFocusWindow = m_pWindow->ImplGetWindowImpl()->mpFrameData->mpFocusWin;
        if ( pFocusWindow && (pFocusWindow != pMouseWindow) &&
             (pFocusWindow == m_pSVData->maWinData.mpFocusWin) )
        {
            // no wheel-messages to disabled windows
            if ( pFocusWindow->IsEnabled() && pFocusWindow->IsInputEnabled() && ! pFocusWindow->IsInModalMode() )
            {
                // transform coordinates to focus window frame coordinates
                Point aRelMousePos( pFocusWindow->OutputToScreenPixel(
                                     pFocusWindow->AbsoluteScreenToOutputPixel(
                                      m_pWindow->OutputToAbsoluteScreenPixel(
                                       m_pWindow->ScreenToOutputPixel( m_aMousePos ) ) ) ) );
                bool bPropogate = CallCommand(pFocusWindow, aRelMousePos);
                if (!bPropogate)
                    pDispatchedTo = pMouseWindow;
            }
        }
    }
    return pDispatchedTo;
}

class HandleWheelEvent : public HandleGestureEventBase
{
private:
    CommandWheelData m_aWheelData;
public:
    HandleWheelEvent(vcl::Window *pWindow, const SalWheelMouseEvent& rEvt, bool bScaleDirectly)
        : HandleGestureEventBase(pWindow, Point(rEvt.mnX, rEvt.mnY))
    {
        CommandWheelMode nMode;
        sal_uInt16 nCode = rEvt.mnCode;
        bool bHorz = rEvt.mbHorz;
        bool bPixel = rEvt.mbDeltaIsPixel;
        if (bScaleDirectly)
            nMode = CommandWheelMode::ZOOM_SCALE;
        else if ( nCode & KEY_MOD1 )
            nMode = CommandWheelMode::ZOOM;
        else if ( nCode & KEY_MOD2 )
            nMode = CommandWheelMode::DATAZOOM;
        else
        {
            nMode = CommandWheelMode::SCROLL;
            // #i85450# interpret shift-wheel as horizontal wheel action
            if( (nCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_MOD3)) == KEY_SHIFT )
                bHorz = true;
        }

        m_aWheelData = CommandWheelData(rEvt.mnDelta, rEvt.mnNotchDelta, rEvt.mnScrollLines, nMode, nCode, bHorz, bPixel);

    }
    virtual bool CallCommand(vcl::Window *pWindow, const Point &rMousePos) override
    {
        return ImplCallWheelCommand(pWindow, rMousePos, &m_aWheelData);
    }
    bool HandleEvent(const SalWheelMouseEvent& rEvt);
};

bool HandleWheelEvent::HandleEvent(const SalWheelMouseEvent& rEvt)
{
    static SalWheelMouseEvent aPreviousEvent;

    if (!Setup())
        return false;

    VclPtr<vcl::Window> xMouseWindow = FindTarget();

    ImplSVData* pSVData = ImplGetSVData();

    // avoid the problem that scrolling via wheel to this point brings a widget
    // under the mouse that also accepts wheel commands, so stick with the old
    // widget if the time gap is very small
    if (shouldReusePreviousMouseWindow(aPreviousEvent, rEvt) && acceptableWheelScrollTarget(pSVData->maWinData.mpLastWheelWindow))
    {
        xMouseWindow = pSVData->maWinData.mpLastWheelWindow;
    }

    aPreviousEvent = rEvt;

    pSVData->maWinData.mpLastWheelWindow = Dispatch(xMouseWindow);

    return pSVData->maWinData.mpLastWheelWindow.get();
}

class HandleGestureEvent : public HandleGestureEventBase
{
public:
    HandleGestureEvent(vcl::Window *pWindow, const Point &rMousePos)
        : HandleGestureEventBase(pWindow, rMousePos)
    {
    }
    bool HandleEvent();
};

bool HandleGestureEvent::HandleEvent()
{
    if (!Setup())
        return false;

    vcl::Window *pTarget = FindTarget();

    bool bHandled = Dispatch(pTarget) != nullptr;
    return bHandled;
}

static bool ImplHandleWheelEvent( vcl::Window* pWindow, const SalWheelMouseEvent& rEvt, bool scaleDirectly = false )
{
    HandleWheelEvent aHandler(pWindow, rEvt, scaleDirectly);
    return aHandler.HandleEvent(rEvt);
}

class HandleSwipeEvent : public HandleGestureEvent
{
private:
    CommandSwipeData m_aSwipeData;
public:
    HandleSwipeEvent(vcl::Window *pWindow, const SalSwipeEvent& rEvt)
        : HandleGestureEvent(pWindow, Point(rEvt.mnX, rEvt.mnY))
    {
        m_aSwipeData = CommandSwipeData(rEvt.mnVelocityX);
    }
    virtual bool CallCommand(vcl::Window *pWindow, const Point &/*rMousePos*/) override
    {
        return ImplCallCommand(pWindow, CommandEventId::Swipe, &m_aSwipeData);
    }
};

static bool ImplHandleSwipe(vcl::Window *pWindow, const SalSwipeEvent& rEvt)
{
    HandleSwipeEvent aHandler(pWindow, rEvt);
    return aHandler.HandleEvent();
}

class HandleLongPressEvent : public HandleGestureEvent
{
private:
    CommandLongPressData m_aLongPressData;
public:
    HandleLongPressEvent(vcl::Window *pWindow, const SalLongPressEvent& rEvt)
        : HandleGestureEvent(pWindow, Point(rEvt.mnX, rEvt.mnY))
    {
        m_aLongPressData = CommandLongPressData(rEvt.mnX, rEvt.mnY);
    }
    virtual bool CallCommand(vcl::Window *pWindow, const Point &/*rMousePos*/) override
    {
        return ImplCallCommand(pWindow, CommandEventId::LongPress, &m_aLongPressData);
    }
};

static bool ImplHandleLongPress(vcl::Window *pWindow, const SalLongPressEvent& rEvt)
{
    HandleLongPressEvent aHandler(pWindow, rEvt);
    return aHandler.HandleEvent();
}

#define IMPL_PAINT_CHECKRTL         ((sal_uInt16)0x0020)

static void ImplHandlePaint( vcl::Window* pWindow, const Rectangle& rBoundRect, bool bImmediateUpdate )
{
    // give up background save when system paints arrive
    vcl::Window* pSaveBackWin = pWindow->ImplGetWindowImpl()->mpFrameData->mpFirstBackWin;
    while ( pSaveBackWin )
    {
        vcl::Window* pNext = pSaveBackWin->ImplGetWindowImpl()->mpOverlapData->mpNextBackWin;
        Rectangle aRect( Point( pSaveBackWin->GetOutOffXPixel(), pSaveBackWin->GetOutOffYPixel() ),
                         Size( pSaveBackWin->GetOutputWidthPixel(), pSaveBackWin->GetOutputHeightPixel() ) );
        if ( aRect.IsOver( rBoundRect ) )
            pSaveBackWin->ImplDeleteOverlapBackground();
        pSaveBackWin = pNext;
    }

    // system paint events must be checked for re-mirroring
    pWindow->ImplGetWindowImpl()->mnPaintFlags |= IMPL_PAINT_CHECKRTL;

    // trigger paint for all windows that live in the new paint region
    vcl::Region aRegion( rBoundRect );
    pWindow->ImplInvalidateOverlapFrameRegion( aRegion );
    if( bImmediateUpdate )
    {
        // #i87663# trigger possible pending resize notifications
        // (GetSizePixel does that for us)
        pWindow->GetSizePixel();
        // force drawing immediately
        pWindow->Update();
    }
}

static void KillOwnPopups( vcl::Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window *pParent = pWindow->ImplGetWindowImpl()->mpFrameWindow;
    vcl::Window *pChild = pSVData->maWinData.mpFirstFloat;
    if ( pChild && pParent->ImplIsWindowOrChild( pChild, true ) )
    {
        if ( !(pSVData->maWinData.mpFirstFloat->GetPopupModeFlags() & FloatWinPopupFlags::NoAppFocusClose) )
            pSVData->maWinData.mpFirstFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
    }
}

void ImplHandleResize( vcl::Window* pWindow, long nNewWidth, long nNewHeight )
{
    if( pWindow->GetStyle() & (WB_MOVEABLE|WB_SIZEABLE) )
    {
        KillOwnPopups( pWindow );
        if( pWindow->ImplGetWindow() != ImplGetSVData()->maHelpData.mpHelpWin )
            ImplDestroyHelpWindow( true );
    }

    if (
         (nNewWidth > 0 && nNewHeight > 0) ||
         pWindow->ImplGetWindow()->ImplGetWindowImpl()->mbAllResize
       )
    {
        if ( (nNewWidth != pWindow->GetOutputWidthPixel()) || (nNewHeight != pWindow->GetOutputHeightPixel()) )
        {
            pWindow->mnOutWidth  = nNewWidth;
            pWindow->mnOutHeight = nNewHeight;
            pWindow->ImplGetWindowImpl()->mbWaitSystemResize = false;
            if ( pWindow->IsReallyVisible() )
                pWindow->ImplSetClipFlag();
            if ( pWindow->IsVisible() || pWindow->ImplGetWindow()->ImplGetWindowImpl()->mbAllResize ||
                ( pWindow->ImplGetWindowImpl()->mbFrame && pWindow->ImplGetWindowImpl()->mpClientWindow ) )   // propagate resize for system border windows
            {
                bool bStartTimer = true;
                // use resize buffering for user resizes
                // ownerdraw decorated windows and floating windows can be resized immediately (i.e. synchronously)
                if( pWindow->ImplGetWindowImpl()->mbFrame && (pWindow->GetStyle() & WB_SIZEABLE)
                    && !(pWindow->GetStyle() & WB_OWNERDRAWDECORATION)  // synchronous resize for ownerdraw decorated windows (toolbars)
                    && !pWindow->ImplGetWindowImpl()->mbFloatWin )             // synchronous resize for floating windows, #i43799#
                {
                    if( pWindow->ImplGetWindowImpl()->mpClientWindow )
                    {
                        // #i42750# presentation wants to be informed about resize
                        // as early as possible
                        WorkWindow* pWorkWindow = dynamic_cast<WorkWindow*>(pWindow->ImplGetWindowImpl()->mpClientWindow.get());
                        if( ! pWorkWindow || pWorkWindow->IsPresentationMode() )
                            bStartTimer = false;
                    }
                    else
                    {
                        WorkWindow* pWorkWindow = dynamic_cast<WorkWindow*>(pWindow);
                        if( ! pWorkWindow || pWorkWindow->IsPresentationMode() )
                            bStartTimer = false;
                    }
                }
                else
                    bStartTimer = false;

                if( bStartTimer )
                    pWindow->ImplGetWindowImpl()->mpFrameData->maResizeIdle.Start();
                else
                    pWindow->ImplCallResize(); // otherwise menus cannot be positioned
            }
            else
                pWindow->ImplGetWindowImpl()->mbCallResize = true;

            if (pWindow->SupportsDoubleBuffering() && pWindow->ImplGetWindowImpl()->mbFrame)
            {
                // Propagate resize for the frame's buffer.
                pWindow->ImplGetWindowImpl()->mpFrameData->mpBuffer->SetOutputSizePixel(pWindow->GetOutputSizePixel());
            }
        }
    }

    pWindow->ImplGetWindowImpl()->mpFrameData->mbNeedSysWindow = (nNewWidth < IMPL_MIN_NEEDSYSWIN) ||
                                            (nNewHeight < IMPL_MIN_NEEDSYSWIN);
    bool bMinimized = (nNewWidth <= 0) || (nNewHeight <= 0);
    if( bMinimized != pWindow->ImplGetWindowImpl()->mpFrameData->mbMinimized )
        pWindow->ImplGetWindowImpl()->mpFrameWindow->ImplNotifyIconifiedState( bMinimized );
    pWindow->ImplGetWindowImpl()->mpFrameData->mbMinimized = bMinimized;
}

static void ImplHandleMove( vcl::Window* pWindow )
{
    if( pWindow->ImplGetWindowImpl()->mbFrame && pWindow->ImplIsFloatingWindow() && pWindow->IsReallyVisible() )
    {
        static_cast<FloatingWindow*>(pWindow)->EndPopupMode( FloatWinPopupEndFlags::TearOff );
        pWindow->ImplCallMove();
    }

    if( pWindow->GetStyle() & (WB_MOVEABLE|WB_SIZEABLE) )
    {
        KillOwnPopups( pWindow );
        if( pWindow->ImplGetWindow() != ImplGetSVData()->maHelpData.mpHelpWin )
            ImplDestroyHelpWindow( true );
    }

    if ( pWindow->IsVisible() )
        pWindow->ImplCallMove();
    else
        pWindow->ImplGetWindowImpl()->mbCallMove = true; // make sure the framepos will be updated on the next Show()

    if ( pWindow->ImplGetWindowImpl()->mbFrame && pWindow->ImplGetWindowImpl()->mpClientWindow )
        pWindow->ImplGetWindowImpl()->mpClientWindow->ImplCallMove();   // notify client to update geometry

}

static void ImplHandleMoveResize( vcl::Window* pWindow, long nNewWidth, long nNewHeight )
{
    ImplHandleMove( pWindow );
    ImplHandleResize( pWindow, nNewWidth, nNewHeight );
}

static void ImplActivateFloatingWindows( vcl::Window* pWindow, bool bActive )
{
    // First check all overlapping windows
    vcl::Window* pTempWindow = pWindow->ImplGetWindowImpl()->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->GetActivateMode() == ActivateModeFlags::NONE )
        {
            if ( (pTempWindow->GetType() == WINDOW_BORDERWINDOW) &&
                 (pTempWindow->ImplGetWindow()->GetType() == WINDOW_FLOATINGWINDOW) )
                static_cast<ImplBorderWindow*>(pTempWindow)->SetDisplayActive( bActive );
        }

        ImplActivateFloatingWindows( pTempWindow, bActive );
        pTempWindow = pTempWindow->ImplGetWindowImpl()->mpNext;
    }
}

IMPL_LINK_NOARG_TYPED(vcl::Window, ImplAsyncFocusHdl, void*, void)
{
    ImplGetWindowImpl()->mpFrameData->mnFocusId = nullptr;

    // If the status has been preserved, because we got back the focus
    // in the meantime, we do nothing
    bool bHasFocus = ImplGetWindowImpl()->mpFrameData->mbHasFocus || ImplGetWindowImpl()->mpFrameData->mbSysObjFocus;

    // next execute the delayed functions
    if ( bHasFocus )
    {
        // redraw all floating windows inactive
        if ( ImplGetWindowImpl()->mpFrameData->mbStartFocusState != bHasFocus )
            ImplActivateFloatingWindows( this, bHasFocus );

        if ( ImplGetWindowImpl()->mpFrameData->mpFocusWin )
        {
            bool bHandled = false;
            if ( ImplGetWindowImpl()->mpFrameData->mpFocusWin->IsInputEnabled() &&
                 ! ImplGetWindowImpl()->mpFrameData->mpFocusWin->IsInModalMode() )
            {
                if ( ImplGetWindowImpl()->mpFrameData->mpFocusWin->IsEnabled() )
                {
                    ImplGetWindowImpl()->mpFrameData->mpFocusWin->GrabFocus();
                    bHandled = true;
                }
                else if( ImplGetWindowImpl()->mpFrameData->mpFocusWin->ImplHasDlgCtrl() )
                {
                // #109094# if the focus is restored to a disabled dialog control (was disabled meanwhile)
                // try to move it to the next control
                    ImplGetWindowImpl()->mpFrameData->mpFocusWin->ImplDlgCtrlNextWindow();
                    bHandled = true;
                }
            }
            if ( !bHandled )
            {
                ImplSVData* pSVData = ImplGetSVData();
                vcl::Window*     pTopLevelWindow = ImplGetWindowImpl()->mpFrameData->mpFocusWin->ImplGetFirstOverlapWindow();
                if ( ( ! pTopLevelWindow->IsInputEnabled() || pTopLevelWindow->IsInModalMode() )
                     && pSVData->maWinData.mpLastExecuteDlg )
                    pSVData->maWinData.mpLastExecuteDlg->ToTop( ToTopFlags::RestoreWhenMin | ToTopFlags::GrabFocusOnly);
                else
                    pTopLevelWindow->GrabFocus();
            }
        }
        else
            GrabFocus();
    }
    else
    {
        vcl::Window* pFocusWin = ImplGetWindowImpl()->mpFrameData->mpFocusWin;
        if ( pFocusWin )
        {
            ImplSVData* pSVData = ImplGetSVData();

            if ( pSVData->maWinData.mpFocusWin == pFocusWin )
            {
                // FocusWindow umsetzen
                vcl::Window* pOverlapWindow = pFocusWin->ImplGetFirstOverlapWindow();
                pOverlapWindow->ImplGetWindowImpl()->mpLastFocusWindow = pFocusWin;
                pSVData->maWinData.mpFocusWin = nullptr;

                if ( pFocusWin->ImplGetWindowImpl()->mpCursor )
                    pFocusWin->ImplGetWindowImpl()->mpCursor->ImplHide( true );

                // Deaktivate rufen
                vcl::Window* pOldFocusWindow = pFocusWin;
                if ( pOldFocusWindow )
                {
                    vcl::Window* pOldOverlapWindow = pOldFocusWindow->ImplGetFirstOverlapWindow();
                    vcl::Window* pOldRealWindow = pOldOverlapWindow->ImplGetWindow();

                    pOldOverlapWindow->ImplGetWindowImpl()->mbActive = false;
                    pOldOverlapWindow->Deactivate();
                    if ( pOldRealWindow != pOldOverlapWindow )
                    {
                        pOldRealWindow->ImplGetWindowImpl()->mbActive = false;
                        pOldRealWindow->Deactivate();
                    }
                }

                // TrackingMode is ended in ImplHandleLoseFocus
#ifdef _WIN32
                // To avoid problems with the Unix IME
                pFocusWin->EndExtTextInput( EndExtTextInputFlags::Complete );
#endif

                // XXX #102010# hack for accessibility: do not close the menu,
                // even after focus lost
                static const char* pEnv = getenv("SAL_FLOATWIN_NOAPPFOCUSCLOSE");
                if( !(pEnv && *pEnv) )
                {
                    NotifyEvent aNEvt( MouseNotifyEvent::LOSEFOCUS, pFocusWin );
                    if ( !ImplCallPreNotify( aNEvt ) )
                        pFocusWin->CompatLoseFocus();
                    pFocusWin->ImplCallDeactivateListeners( nullptr );
                }
                // XXX
            }
        }

        // Redraw all floating window inactive
        if ( ImplGetWindowImpl()->mpFrameData->mbStartFocusState != bHasFocus )
            ImplActivateFloatingWindows( this, bHasFocus );
    }
}

static void ImplHandleGetFocus( vcl::Window* pWindow )
{
    pWindow->ImplGetWindowImpl()->mpFrameData->mbHasFocus = true;

    // execute Focus-Events after a delay, such that SystemChildWindows
    // do not blink when they receive focus
    if ( !pWindow->ImplGetWindowImpl()->mpFrameData->mnFocusId )
    {
        pWindow->ImplGetWindowImpl()->mpFrameData->mbStartFocusState = !pWindow->ImplGetWindowImpl()->mpFrameData->mbHasFocus;
        pWindow->ImplGetWindowImpl()->mpFrameData->mnFocusId = Application::PostUserEvent( LINK( pWindow, vcl::Window, ImplAsyncFocusHdl ), nullptr, true);
        vcl::Window* pFocusWin = pWindow->ImplGetWindowImpl()->mpFrameData->mpFocusWin;
        if ( pFocusWin && pFocusWin->ImplGetWindowImpl()->mpCursor )
            pFocusWin->ImplGetWindowImpl()->mpCursor->ImplShow();
    }
}

static void ImplHandleLoseFocus( vcl::Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Abort the autoscroll if the frame loses focus
    if ( pSVData->maWinData.mpAutoScrollWin )
        pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();

    // Abort tracking if the frame loses focus
    if ( pSVData->maWinData.mpTrackWin )
    {
        if ( pSVData->maWinData.mpTrackWin->ImplGetWindowImpl()->mpFrameWindow == pWindow )
            pSVData->maWinData.mpTrackWin->EndTracking( TrackingEventFlags::Cancel );
    }

    pWindow->ImplGetWindowImpl()->mpFrameData->mbHasFocus = false;

    // execute Focus-Events after a delay, such that SystemChildWindows
    // do not flicker when they receive focus
    if ( !pWindow->ImplGetWindowImpl()->mpFrameData->mnFocusId )
    {
        pWindow->ImplGetWindowImpl()->mpFrameData->mbStartFocusState = !pWindow->ImplGetWindowImpl()->mpFrameData->mbHasFocus;
        pWindow->ImplGetWindowImpl()->mpFrameData->mnFocusId = Application::PostUserEvent( LINK( pWindow, vcl::Window, ImplAsyncFocusHdl ), nullptr, true );
    }

    vcl::Window* pFocusWin = pWindow->ImplGetWindowImpl()->mpFrameData->mpFocusWin;
    if ( pFocusWin && pFocusWin->ImplGetWindowImpl()->mpCursor )
        pFocusWin->ImplGetWindowImpl()->mpCursor->ImplHide( true );
}

struct DelayedCloseEvent
{
    VclPtr<vcl::Window> pWindow;
    ImplDelData     aDelData;
};

static void DelayedCloseEventLink( void* pCEvent, void* )
{
    DelayedCloseEvent* pEv = static_cast<DelayedCloseEvent*>(pCEvent);

    if( ! pEv->aDelData.IsDead() )
    {
        pEv->pWindow->ImplRemoveDel( &pEv->aDelData );
        // dispatch to correct window type
        if( pEv->pWindow->IsSystemWindow() )
            static_cast<SystemWindow*>(pEv->pWindow.get())->Close();
        else if( pEv->pWindow->IsDockingWindow() )
            static_cast<DockingWindow*>(pEv->pWindow.get())->Close();
    }
    delete pEv;
}

void ImplHandleClose( vcl::Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();

    bool bWasPopup = false;
    if( pWindow->ImplIsFloatingWindow() &&
        static_cast<FloatingWindow*>(pWindow)->ImplIsInPrivatePopupMode() )
    {
        bWasPopup = true;
    }

    // on Close stop all floating modes and end popups
    if ( pSVData->maWinData.mpFirstFloat )
    {
        FloatingWindow* pLastLevelFloat;
        pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
        pLastLevelFloat->EndPopupMode( FloatWinPopupEndFlags::Cancel | FloatWinPopupEndFlags::CloseAll );
    }
    if ( pSVData->maHelpData.mbExtHelpMode )
        Help::EndExtHelp();
    if ( pSVData->maHelpData.mpHelpWin )
        ImplDestroyHelpWindow( false );
    // AutoScrollMode
    if ( pSVData->maWinData.mpAutoScrollWin )
        pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();

    if ( pSVData->maWinData.mpTrackWin )
        pSVData->maWinData.mpTrackWin->EndTracking( TrackingEventFlags::Cancel | TrackingEventFlags::Key );

    if (bWasPopup)
        return;

    vcl::Window *pWin = pWindow->ImplGetWindow();
    SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(pWin);
    if (pSysWin)
    {
        // See if the custom close handler is set.
        const Link<SystemWindow&,void>& rLink = pSysWin->GetCloseHdl();
        if (rLink.IsSet())
        {
            rLink.Call(*pSysWin);
            return;
        }
    }

    // check whether close is allowed
    if ( pWin->IsEnabled() && pWin->IsInputEnabled() && !pWin->IsInModalMode() )
    {
        DelayedCloseEvent* pEv = new DelayedCloseEvent;
        pEv->pWindow = pWin;
        pWin->ImplAddDel( &pEv->aDelData );
        Application::PostUserEvent( Link<void*,void>( pEv, DelayedCloseEventLink ) );
    }
}

static void ImplHandleUserEvent( ImplSVEvent* pSVEvent )
{
    if ( pSVEvent )
    {
        if ( pSVEvent->mbCall && !pSVEvent->maDelData.IsDead() )
        {
            if ( pSVEvent->mpWindow )
            {
                pSVEvent->mpWindow->ImplRemoveDel( &(pSVEvent->maDelData) );
                pSVEvent->maLink.Call( pSVEvent->mpData );
            }
            else
            {
                pSVEvent->maLink.Call( pSVEvent->mpData );
            }
        }

        delete pSVEvent;
    }
}

static MouseEventModifiers ImplGetMouseMoveMode( SalMouseEvent* pEvent )
{
    MouseEventModifiers nMode = MouseEventModifiers::NONE;
    if ( !pEvent->mnCode )
        nMode |= MouseEventModifiers::SIMPLEMOVE;
    if ( (pEvent->mnCode & MOUSE_LEFT) && !(pEvent->mnCode & KEY_MOD1) )
        nMode |= MouseEventModifiers::DRAGMOVE;
    if ( (pEvent->mnCode & MOUSE_LEFT) && (pEvent->mnCode & KEY_MOD1) )
        nMode |= MouseEventModifiers::DRAGCOPY;
    return nMode;
}

static MouseEventModifiers ImplGetMouseButtonMode( SalMouseEvent* pEvent )
{
    MouseEventModifiers nMode = MouseEventModifiers::NONE;
    if ( pEvent->mnButton == MOUSE_LEFT )
        nMode |= MouseEventModifiers::SIMPLECLICK;
    if ( (pEvent->mnButton == MOUSE_LEFT) && !(pEvent->mnCode & (MOUSE_MIDDLE | MOUSE_RIGHT)) )
        nMode |= MouseEventModifiers::SELECT;
    if ( (pEvent->mnButton == MOUSE_LEFT) && (pEvent->mnCode & KEY_MOD1) &&
         !(pEvent->mnCode & (MOUSE_MIDDLE | MOUSE_RIGHT | KEY_SHIFT)) )
        nMode |= MouseEventModifiers::MULTISELECT;
    if ( (pEvent->mnButton == MOUSE_LEFT) && (pEvent->mnCode & KEY_SHIFT) &&
         !(pEvent->mnCode & (MOUSE_MIDDLE | MOUSE_RIGHT | KEY_MOD1)) )
        nMode |= MouseEventModifiers::RANGESELECT;
    return nMode;
}

inline bool ImplHandleSalMouseLeave( vcl::Window* pWindow, SalMouseEvent* pEvent )
{
    return ImplHandleMouseEvent( pWindow, MouseNotifyEvent::MOUSEMOVE, true,
                                 pEvent->mnX, pEvent->mnY,
                                 pEvent->mnTime, pEvent->mnCode,
                                 ImplGetMouseMoveMode( pEvent ) );
}

inline bool ImplHandleSalMouseMove( vcl::Window* pWindow, SalMouseEvent* pEvent )
{
    return ImplHandleMouseEvent( pWindow, MouseNotifyEvent::MOUSEMOVE, false,
                                 pEvent->mnX, pEvent->mnY,
                                 pEvent->mnTime, pEvent->mnCode,
                                 ImplGetMouseMoveMode( pEvent ) );
}

inline bool ImplHandleSalMouseButtonDown( vcl::Window* pWindow, SalMouseEvent* pEvent )
{
    return ImplHandleMouseEvent( pWindow, MouseNotifyEvent::MOUSEBUTTONDOWN, false,
                                 pEvent->mnX, pEvent->mnY,
                                 pEvent->mnTime,
#ifdef MACOSX
                 pEvent->mnButton | (pEvent->mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_MOD3)),
#else
                                 pEvent->mnButton | (pEvent->mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)),
#endif
                                 ImplGetMouseButtonMode( pEvent ) );
}

inline bool ImplHandleSalMouseButtonUp( vcl::Window* pWindow, SalMouseEvent* pEvent )
{
    return ImplHandleMouseEvent( pWindow, MouseNotifyEvent::MOUSEBUTTONUP, false,
                                 pEvent->mnX, pEvent->mnY,
                                 pEvent->mnTime,
#ifdef MACOSX
                 pEvent->mnButton | (pEvent->mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_MOD3)),
#else
                                 pEvent->mnButton | (pEvent->mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)),
#endif
                                 ImplGetMouseButtonMode( pEvent ) );
}

static bool ImplHandleMenuEvent( vcl::Window* pWindow, SalMenuEvent* pEvent, sal_uInt16 nEvent )
{
    // Find SystemWindow and its Menubar and let it dispatch the command
    bool bRet = false;
    vcl::Window *pWin = pWindow->ImplGetWindowImpl()->mpFirstChild;
    while ( pWin )
    {
        if ( pWin->ImplGetWindowImpl()->mbSysWin )
            break;
        pWin = pWin->ImplGetWindowImpl()->mpNext;
    }
    if( pWin )
    {
        MenuBar *pMenuBar = static_cast<SystemWindow*>(pWin)->GetMenuBar();
        if( pMenuBar )
        {
            switch( nEvent )
            {
                case SALEVENT_MENUACTIVATE:
                    bRet = pMenuBar->HandleMenuActivateEvent( static_cast<Menu*>(pEvent->mpMenu) );
                    break;
                case SALEVENT_MENUDEACTIVATE:
                    bRet = pMenuBar->HandleMenuDeActivateEvent( static_cast<Menu*>(pEvent->mpMenu) );
                    break;
                case SALEVENT_MENUHIGHLIGHT:
                    bRet = pMenuBar->HandleMenuHighlightEvent( static_cast<Menu*>(pEvent->mpMenu), pEvent->mnId );
                    break;
                case SALEVENT_MENUBUTTONCOMMAND:
                    bRet = pMenuBar->HandleMenuButtonEvent( static_cast<Menu*>(pEvent->mpMenu), pEvent->mnId );
                    break;
                case SALEVENT_MENUCOMMAND:
                    bRet = pMenuBar->HandleMenuCommandEvent( static_cast<Menu*>(pEvent->mpMenu), pEvent->mnId );
                    break;
                default:
                    break;
            }
        }
    }
    return bRet;
}

static void ImplHandleSalKeyMod( vcl::Window* pWindow, SalKeyModEvent* pEvent )
{
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window* pTrackWin = pSVData->maWinData.mpTrackWin;
    if ( pTrackWin )
        pWindow = pTrackWin;
#ifdef MACOSX
    sal_uInt16 nOldCode = pWindow->ImplGetWindowImpl()->mpFrameData->mnMouseCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_MOD3);
#else
    sal_uInt16 nOldCode = pWindow->ImplGetWindowImpl()->mpFrameData->mnMouseCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2);
#endif
    sal_uInt16 nNewCode = pEvent->mnCode;
    if ( nOldCode != nNewCode )
    {
#ifdef MACOSX
    nNewCode |= pWindow->ImplGetWindowImpl()->mpFrameData->mnMouseCode & ~(KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_MOD3);
#else
        nNewCode |= pWindow->ImplGetWindowImpl()->mpFrameData->mnMouseCode & ~(KEY_SHIFT | KEY_MOD1 | KEY_MOD2);
#endif
        pWindow->ImplGetWindowImpl()->mpFrameWindow->ImplCallMouseMove( nNewCode, true );
    }

    // #105224# send commandevent to allow special treatment of Ctrl-LeftShift/Ctrl-RightShift etc.

    // find window - first look to see if the system window is available
    vcl::Window* pChild = pWindow->ImplGetWindowImpl()->mpFirstChild;

    while ( pChild )
    {
        if ( pChild->ImplGetWindowImpl()->mbSysWin )
            break;
        pChild = pChild->ImplGetWindowImpl()->mpNext;
    }
    //...if not, try to find a key input window...
    if (!pChild) ImplGetKeyInputWindow( pWindow );
    //...otherwise fail safe...
    if (!pChild) pChild = pWindow;

    CommandModKeyData data( pEvent->mnModKeyCode );
    ImplCallCommand( pChild, CommandEventId::ModKeyChange, &data );
}

static void ImplHandleInputLanguageChange( vcl::Window* pWindow )
{
    // find window
    vcl::Window* pChild = ImplGetKeyInputWindow( pWindow );
    if ( !pChild )
        return;

    ImplCallCommand( pChild, CommandEventId::InputLanguageChange );
}

static void ImplHandleSalSettings( sal_uInt16 nEvent )
{
    Application* pApp = GetpApp();
    if ( !pApp )
        return;

    if ( nEvent == SALEVENT_SETTINGSCHANGED )
    {
        AllSettings aSettings = Application::GetSettings();
        Application::MergeSystemSettings( aSettings );
        pApp->OverrideSystemSettings( aSettings );
        Application::SetSettings( aSettings );
    }
    else
    {
        DataChangedEventType nType;
        switch ( nEvent )
        {
            case SALEVENT_PRINTERCHANGED:
                ImplDeletePrnQueueList();
                nType = DataChangedEventType::PRINTER;
                break;
            case SALEVENT_DISPLAYCHANGED:
                nType = DataChangedEventType::DISPLAY;
                break;
            case SALEVENT_FONTCHANGED:
                OutputDevice::ImplUpdateAllFontData( true );
                nType = DataChangedEventType::FONTS;
                break;
            default:
                nType = DataChangedEventType::NONE;
                break;
        }

        if ( nType != DataChangedEventType::NONE )
        {
            DataChangedEvent aDCEvt( nType );
            Application::NotifyAllWindows( aDCEvt );
        }
    }
}

static void ImplHandleSalExtTextInputPos( vcl::Window* pWindow, SalExtTextInputPosEvent* pEvt )
{
    Rectangle aCursorRect;
    ImplHandleExtTextInputPos( pWindow, aCursorRect, pEvt->mnExtWidth, &pEvt->mbVertical );
    if ( aCursorRect.IsEmpty() )
    {
        pEvt->mnX       = -1;
        pEvt->mnY       = -1;
        pEvt->mnWidth   = -1;
        pEvt->mnHeight  = -1;
    }
    else
    {
        pEvt->mnX       = aCursorRect.Left();
        pEvt->mnY       = aCursorRect.Top();
        pEvt->mnWidth   = aCursorRect.GetWidth();
        pEvt->mnHeight  = aCursorRect.GetHeight();
    }
}

static bool ImplHandleShowDialog( vcl::Window* pWindow, ShowDialogId nDialogId )
{
    if( ! pWindow )
        return false;

    if( pWindow->GetType() == WINDOW_BORDERWINDOW )
    {
        vcl::Window* pWrkWin = pWindow->GetWindow( GetWindowType::Client );
        if( pWrkWin )
            pWindow = pWrkWin;
    }
    CommandDialogData aCmdData( nDialogId );
    return ImplCallCommand( pWindow, CommandEventId::ShowDialog, &aCmdData );
}

static void ImplHandleSurroundingTextRequest( vcl::Window *pWindow,
                          OUString& rText,
                          Selection &rSelRange )
{
    vcl::Window* pChild = ImplGetKeyInputWindow( pWindow );

    if ( !pChild )
    {
    rText.clear();
    rSelRange.setMin( 0 );
    rSelRange.setMax( 0 );
    }
    else
    {
    rText = pChild->GetSurroundingText();
    Selection aSel = pChild->GetSurroundingTextSelection();
    rSelRange.setMin( aSel.Min() );
    rSelRange.setMax( aSel.Max() );
    }
}

static void ImplHandleSalSurroundingTextRequest( vcl::Window *pWindow,
                         SalSurroundingTextRequestEvent *pEvt )
{
    Selection aSelRange;
    ImplHandleSurroundingTextRequest( pWindow, pEvt->maText, aSelRange );

    aSelRange.Justify();

    if( aSelRange.Min() < 0 )
        pEvt->mnStart = 0;
    else if( aSelRange.Min() > pEvt->maText.getLength() )
        pEvt->mnStart = pEvt->maText.getLength();
    else
        pEvt->mnStart = aSelRange.Min();

    if( aSelRange.Max() < 0 )
        pEvt->mnStart = 0;
    else if( aSelRange.Max() > pEvt->maText.getLength() )
        pEvt->mnEnd = pEvt->maText.getLength();
    else
        pEvt->mnEnd = aSelRange.Max();
}

static void ImplHandleSurroundingTextSelectionChange( vcl::Window *pWindow,
                              sal_uLong nStart,
                              sal_uLong nEnd )
{
    vcl::Window* pChild = ImplGetKeyInputWindow( pWindow );
    if( pChild )
    {
        CommandSelectionChangeData data( nStart, nEnd );
        ImplCallCommand( pChild, CommandEventId::SelectionChange, &data );
    }
}

static void ImplHandleStartReconversion( vcl::Window *pWindow )
{
    vcl::Window* pChild = ImplGetKeyInputWindow( pWindow );
    if( pChild )
    ImplCallCommand( pChild, CommandEventId::PrepareReconversion );
}

static void ImplHandleSalQueryCharPosition( vcl::Window *pWindow,
                                            SalQueryCharPositionEvent *pEvt )
{
    pEvt->mbValid = false;
    pEvt->mbVertical = false;
    pEvt->mnCursorBoundX = 0;
    pEvt->mnCursorBoundY = 0;
    pEvt->mnCursorBoundWidth = 0;
    pEvt->mnCursorBoundHeight = 0;

    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window*     pChild = pSVData->maWinData.mpExtTextInputWin;

    if ( !pChild )
        pChild = ImplGetKeyInputWindow( pWindow );
    else
    {
        // Test, if the Window is related to the frame
        if ( !pWindow->ImplIsWindowOrChild( pChild ) )
            pChild = ImplGetKeyInputWindow( pWindow );
    }

    if( pChild )
    {
        ImplCallCommand( pChild, CommandEventId::QueryCharPosition );

        ImplWinData* pWinData = pChild->ImplGetWinData();
        if ( pWinData->mpCompositionCharRects && pEvt->mnCharPos < static_cast<sal_uLong>( pWinData->mnCompositionCharRects ) )
        {
            const OutputDevice *pChildOutDev = pChild->GetOutDev();
            const Rectangle& aRect = pWinData->mpCompositionCharRects[ pEvt->mnCharPos ];
            Rectangle aDeviceRect = pChildOutDev->ImplLogicToDevicePixel( aRect );
            Point aAbsScreenPos = pChild->OutputToAbsoluteScreenPixel( pChild->ScreenToOutputPixel(aDeviceRect.TopLeft()) );
            pEvt->mnCursorBoundX = aAbsScreenPos.X();
            pEvt->mnCursorBoundY = aAbsScreenPos.Y();
            pEvt->mnCursorBoundWidth = aDeviceRect.GetWidth();
            pEvt->mnCursorBoundHeight = aDeviceRect.GetHeight();
            pEvt->mbVertical = pWinData->mbVertical;
            pEvt->mbValid = true;
        }
    }
}

bool ImplWindowFrameProc( vcl::Window* _pWindow, SalFrame* /*pFrame*/,
                          sal_uInt16 nEvent, const void* pEvent )
{
    DBG_TESTSOLARMUTEX();

    // Ensure the window survives during this method.
    VclPtr<vcl::Window> pWindow( _pWindow );

    bool bRet = false;

    // #119709# for some unknown reason it is possible to receive events (in this case key events)
    // although the corresponding VCL window must have been destroyed already
    // at least ImplGetWindowImpl() was NULL in these cases, so check this here
    if( pWindow->ImplGetWindowImpl() == nullptr )
        return false;

    switch ( nEvent )
    {
        case SALEVENT_MOUSEMOVE:
            bRet = ImplHandleSalMouseMove( pWindow, const_cast<SalMouseEvent *>(static_cast<SalMouseEvent const *>(pEvent)) );
            break;
        case SALEVENT_EXTERNALMOUSEMOVE:
        {
            MouseEvent const * pMouseEvt = static_cast<MouseEvent const *>(pEvent);
            SalMouseEvent   aSalMouseEvent;

            aSalMouseEvent.mnTime = tools::Time::GetSystemTicks();
            aSalMouseEvent.mnX = pMouseEvt->GetPosPixel().X();
            aSalMouseEvent.mnY = pMouseEvt->GetPosPixel().Y();
            aSalMouseEvent.mnButton = 0;
            aSalMouseEvent.mnCode = pMouseEvt->GetButtons() | pMouseEvt->GetModifier();

            bRet = ImplHandleSalMouseMove( pWindow, &aSalMouseEvent );
        }
        break;
        case SALEVENT_MOUSELEAVE:
            bRet = ImplHandleSalMouseLeave( pWindow, const_cast<SalMouseEvent *>(static_cast<SalMouseEvent const *>(pEvent)) );
            break;
        case SALEVENT_MOUSEBUTTONDOWN:
            bRet = ImplHandleSalMouseButtonDown( pWindow, const_cast<SalMouseEvent *>(static_cast<SalMouseEvent const *>(pEvent)) );
            break;
        case SALEVENT_EXTERNALMOUSEBUTTONDOWN:
        {
            MouseEvent const * pMouseEvt = static_cast<MouseEvent const *>(pEvent);
            SalMouseEvent   aSalMouseEvent;

            aSalMouseEvent.mnTime = tools::Time::GetSystemTicks();
            aSalMouseEvent.mnX = pMouseEvt->GetPosPixel().X();
            aSalMouseEvent.mnY = pMouseEvt->GetPosPixel().Y();
            aSalMouseEvent.mnButton = pMouseEvt->GetButtons();
            aSalMouseEvent.mnCode = pMouseEvt->GetButtons() | pMouseEvt->GetModifier();

            bRet = ImplHandleSalMouseButtonDown( pWindow, &aSalMouseEvent );
        }
        break;
        case SALEVENT_MOUSEBUTTONUP:
            bRet = ImplHandleSalMouseButtonUp( pWindow, const_cast<SalMouseEvent *>(static_cast<SalMouseEvent const *>(pEvent)) );
            break;
        case SALEVENT_EXTERNALMOUSEBUTTONUP:
        {
            MouseEvent const * pMouseEvt = static_cast<MouseEvent const *>(pEvent);
            SalMouseEvent   aSalMouseEvent;

            aSalMouseEvent.mnTime = tools::Time::GetSystemTicks();
            aSalMouseEvent.mnX = pMouseEvt->GetPosPixel().X();
            aSalMouseEvent.mnY = pMouseEvt->GetPosPixel().Y();
            aSalMouseEvent.mnButton = pMouseEvt->GetButtons();
            aSalMouseEvent.mnCode = pMouseEvt->GetButtons() | pMouseEvt->GetModifier();

            bRet = ImplHandleSalMouseButtonUp( pWindow, &aSalMouseEvent );
        }
        break;
        case SALEVENT_MOUSEACTIVATE:
            bRet = false;
            break;
        case SALEVENT_KEYINPUT:
            {
            SalKeyEvent const * pKeyEvt = static_cast<SalKeyEvent const *>(pEvent);
            bRet = ImplHandleKey( pWindow, MouseNotifyEvent::KEYINPUT,
                pKeyEvt->mnCode, pKeyEvt->mnCharCode, pKeyEvt->mnRepeat, true );
            }
            break;
        case SALEVENT_EXTERNALKEYINPUT:
            {
            KeyEvent const * pKeyEvt = static_cast<KeyEvent const *>(pEvent);
            bRet = ImplHandleKey( pWindow, MouseNotifyEvent::KEYINPUT,
                pKeyEvt->GetKeyCode().GetFullCode(), pKeyEvt->GetCharCode(), pKeyEvt->GetRepeat(), false );
            }
            break;
        case SALEVENT_KEYUP:
            {
            SalKeyEvent const * pKeyEvt = static_cast<SalKeyEvent const *>(pEvent);
            bRet = ImplHandleKey( pWindow, MouseNotifyEvent::KEYUP,
                pKeyEvt->mnCode, pKeyEvt->mnCharCode, pKeyEvt->mnRepeat, true );
            }
            break;
        case SALEVENT_EXTERNALKEYUP:
            {
            KeyEvent const * pKeyEvt = static_cast<KeyEvent const *>(pEvent);
            bRet = ImplHandleKey( pWindow, MouseNotifyEvent::KEYUP,
                pKeyEvt->GetKeyCode().GetFullCode(), pKeyEvt->GetCharCode(), pKeyEvt->GetRepeat(), false );
            }
            break;
        case SALEVENT_KEYMODCHANGE:
            ImplHandleSalKeyMod( pWindow, const_cast<SalKeyModEvent *>(static_cast<SalKeyModEvent const *>(pEvent)) );
            break;

        case SALEVENT_INPUTLANGUAGECHANGE:
            ImplHandleInputLanguageChange( pWindow );
            break;

        case SALEVENT_MENUACTIVATE:
        case SALEVENT_MENUDEACTIVATE:
        case SALEVENT_MENUHIGHLIGHT:
        case SALEVENT_MENUCOMMAND:
        case SALEVENT_MENUBUTTONCOMMAND:
            bRet = ImplHandleMenuEvent( pWindow, const_cast<SalMenuEvent *>(static_cast<SalMenuEvent const *>(pEvent)), nEvent );
            break;

        case SALEVENT_WHEELMOUSE:
            bRet = ImplHandleWheelEvent( pWindow, *static_cast<const SalWheelMouseEvent*>(pEvent));
            break;

        case SALEVENT_PAINT:
            {
            SalPaintEvent const * pPaintEvt = static_cast<SalPaintEvent const *>(pEvent);

            if( AllSettings::GetLayoutRTL() )
            {
                // --- RTL --- (mirror paint rect)
                SalFrame* pSalFrame = pWindow->ImplGetWindowImpl()->mpFrame;
                const_cast<SalPaintEvent *>(pPaintEvt)->mnBoundX = pSalFrame->maGeometry.nWidth-pPaintEvt->mnBoundWidth-pPaintEvt->mnBoundX;
            }

            Rectangle aBoundRect( Point( pPaintEvt->mnBoundX, pPaintEvt->mnBoundY ),
                                  Size( pPaintEvt->mnBoundWidth, pPaintEvt->mnBoundHeight ) );
            ImplHandlePaint( pWindow, aBoundRect, pPaintEvt->mbImmediateUpdate );
            }
            break;

        case SALEVENT_MOVE:
            ImplHandleMove( pWindow );
            break;

        case SALEVENT_RESIZE:
            {
            long nNewWidth;
            long nNewHeight;
            pWindow->ImplGetWindowImpl()->mpFrame->GetClientSize( nNewWidth, nNewHeight );
            ImplHandleResize( pWindow, nNewWidth, nNewHeight );
            }
            break;

        case SALEVENT_MOVERESIZE:
            {
            SalFrameGeometry g = pWindow->ImplGetWindowImpl()->mpFrame->GetGeometry();
            ImplHandleMoveResize( pWindow, g.nWidth, g.nHeight );
            }
            break;

        case SALEVENT_CLOSEPOPUPS:
            {
            KillOwnPopups( pWindow );
            }
            break;

        case SALEVENT_GETFOCUS:
            ImplHandleGetFocus( pWindow );
            break;
        case SALEVENT_LOSEFOCUS:
            ImplHandleLoseFocus( pWindow );
            break;

        case SALEVENT_CLOSE:
            ImplHandleClose( pWindow );
            break;

        case SALEVENT_SHUTDOWN:
            {
                static bool bInQueryExit = false;
                if( !bInQueryExit )
                {
                    bInQueryExit = true;
                    if ( GetpApp()->QueryExit() )
                    {
                        // Message-Schleife beenden
                        Application::Quit();
                        return false;
                    }
                    else
                    {
                        bInQueryExit = false;
                        return true;
                    }
                }
                return false;
            }

        case SALEVENT_SETTINGSCHANGED:
        case SALEVENT_PRINTERCHANGED:
        case SALEVENT_DISPLAYCHANGED:
        case SALEVENT_FONTCHANGED:
            ImplHandleSalSettings( nEvent );
            break;

        case SALEVENT_USEREVENT:
            ImplHandleUserEvent( const_cast<ImplSVEvent *>(static_cast<ImplSVEvent const *>(pEvent)) );
            break;

        case SALEVENT_EXTTEXTINPUT:
            {
            SalExtTextInputEvent const * pEvt = static_cast<SalExtTextInputEvent const *>(pEvent);
            bRet = ImplHandleExtTextInput( pWindow,
                                           pEvt->maText, pEvt->mpTextAttr,
                                           pEvt->mnCursorPos, pEvt->mnCursorFlags );
            }
            break;
        case SALEVENT_ENDEXTTEXTINPUT:
            bRet = ImplHandleEndExtTextInput( pWindow );
            break;
        case SALEVENT_EXTTEXTINPUTPOS:
            ImplHandleSalExtTextInputPos( pWindow, const_cast<SalExtTextInputPosEvent *>(static_cast<SalExtTextInputPosEvent const *>(pEvent)) );
            break;
        case SALEVENT_INPUTCONTEXTCHANGE:
            bRet = ImplHandleInputContextChange( pWindow, static_cast<SalInputContextChangeEvent const *>(pEvent)->meLanguage );
            break;
        case SALEVENT_SHOWDIALOG:
            {
                ShowDialogId nDialogID = static_cast<ShowDialogId>(reinterpret_cast<sal_IntPtr>(pEvent));
                bRet = ImplHandleShowDialog( pWindow, nDialogID );
            }
            break;
        case SALEVENT_SURROUNDINGTEXTREQUEST:
            ImplHandleSalSurroundingTextRequest( pWindow, const_cast<SalSurroundingTextRequestEvent *>(static_cast<SalSurroundingTextRequestEvent const *>(pEvent)) );
            break;
        case SALEVENT_SURROUNDINGTEXTSELECTIONCHANGE:
        {
            SalSurroundingTextSelectionChangeEvent const * pEvt
             = static_cast<SalSurroundingTextSelectionChangeEvent const *>(pEvent);
            ImplHandleSurroundingTextSelectionChange( pWindow,
                              pEvt->mnStart,
                              pEvt->mnEnd );
            // Fallthrough really intended?
        }
        case SALEVENT_STARTRECONVERSION:
            ImplHandleStartReconversion( pWindow );
            break;
        case SALEVENT_EXTERNALZOOM:
            {
            ZoomEvent const * pZoomEvent = static_cast<ZoomEvent const *>(pEvent);
            SalWheelMouseEvent aSalWheelMouseEvent;
            aSalWheelMouseEvent.mnTime = tools::Time::GetSystemTicks();
            aSalWheelMouseEvent.mnX = pZoomEvent->GetCenter().getX();
            aSalWheelMouseEvent.mnY = pZoomEvent->GetCenter().getY();
            // Pass on the scale as a percentage * 100 of current zoom factor
            // so to assure zoom granularity
            aSalWheelMouseEvent.mnDelta = long(double(pZoomEvent->GetScale()) * double(MOBILE_ZOOM_SCALE_MULTIPLIER));
            // Other SalWheelMouseEvent fields ignored when the
            // scaleDirectly parameter to ImplHandleWheelEvent() is
            // true.
            bRet = ImplHandleWheelEvent( pWindow, aSalWheelMouseEvent, true );
            }
            break;
        case SALEVENT_EXTERNALSCROLL:
            {
            ScrollEvent const * pScrollEvent = static_cast<ScrollEvent const *>(pEvent);
            SalWheelMouseEvent aSalWheelMouseEvent;
            aSalWheelMouseEvent.mnTime = tools::Time::GetSystemTicks();
            aSalWheelMouseEvent.mbDeltaIsPixel = true;
            // event location holds delta values instead
            aSalWheelMouseEvent.mnX = long(pScrollEvent->GetXOffset());
            aSalWheelMouseEvent.mnY = long(pScrollEvent->GetYOffset());
            aSalWheelMouseEvent.mnScrollLines = 0;
            if (aSalWheelMouseEvent.mnX != 0 || aSalWheelMouseEvent.mnY != 0)
            {
                bRet = ImplHandleWheelEvent( pWindow, aSalWheelMouseEvent );
            }
        }
        break;
        case SALEVENT_QUERYCHARPOSITION:
            ImplHandleSalQueryCharPosition( pWindow, const_cast<SalQueryCharPositionEvent *>(static_cast<SalQueryCharPositionEvent const *>(pEvent)) );
            break;

        case SALEVENT_SWIPE:
            bRet = ImplHandleSwipe(pWindow, *static_cast<const SalSwipeEvent*>(pEvent));
            break;

        case SALEVENT_LONGPRESS:
            bRet = ImplHandleLongPress(pWindow, *static_cast<const SalLongPressEvent*>(pEvent));
            break;


#ifdef DBG_UTIL
        default:
            SAL_WARN( "vcl.layout", "ImplWindowFrameProc(): unknown event (" << nEvent << ")" );
            break;
#endif
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
