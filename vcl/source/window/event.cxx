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

#include <vcl/event.hxx>
#include <vcl/window.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/layout.hxx>
#include <vcl/opengl/OpenGLWrapper.hxx>

#include <window.h>
#include <svdata.hxx>
#include <salframe.hxx>

#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>

namespace vcl {

void Window::DataChanged( const DataChangedEvent& )
{
}

void Window::NotifyAllChildren( DataChangedEvent& rDCEvt )
{
    CompatDataChanged( rDCEvt );

    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->NotifyAllChildren( rDCEvt );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

bool Window::PreNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;
    if ( mpWindowImpl->mpParent && !ImplIsOverlapWindow() )
        bDone = mpWindowImpl->mpParent->CompatPreNotify( rNEvt );

    if ( !bDone )
    {
        if( rNEvt.GetType() == MouseNotifyEvent::GETFOCUS )
        {
            bool bCompoundFocusChanged = false;
            if ( mpWindowImpl->mbCompoundControl && !mpWindowImpl->mbCompoundControlHasFocus && HasChildPathFocus() )
            {
                mpWindowImpl->mbCompoundControlHasFocus = true;
                bCompoundFocusChanged = true;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                CallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
        }
        else if( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
        {
            bool bCompoundFocusChanged = false;
            if ( mpWindowImpl->mbCompoundControl && mpWindowImpl->mbCompoundControlHasFocus && !HasChildPathFocus() )
            {
                mpWindowImpl->mbCompoundControlHasFocus = false ;
                bCompoundFocusChanged = true;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                CallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS );
        }

        // #82968# mouse and key events will be notified after processing ( in ImplNotifyKeyMouseCommandEventListeners() )!
        //    see also ImplHandleMouseEvent(), ImplHandleKey()

    }

    return bDone;
}

bool Window::Notify( NotifyEvent& rNEvt )
{
    bool bRet = false;

    if (IsDisposed())
        return false;

    // check for docking window
    // but do nothing if window is docked and locked
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && !( !pWrapper->IsFloatingMode() && pWrapper->IsLocked() ) )
    {
        if ( rNEvt.GetType() == MouseNotifyEvent::MOUSEBUTTONDOWN )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            bool bHit = pWrapper->GetDragArea().IsInside( pMEvt->GetPosPixel() );
            if ( pMEvt->IsLeft() )
            {
                if ( pMEvt->IsMod1() && (pMEvt->GetClicks() == 2) )
                {
                    // ctrl double click toggles floating mode
                    pWrapper->SetFloatingMode( !pWrapper->IsFloatingMode() );
                    return true;
                }
                else if ( pMEvt->GetClicks() == 1 && bHit)
                {
                    // allow start docking during mouse move
                    pWrapper->ImplEnableStartDocking();
                    return true;
                }
            }
        }
        else if ( rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            bool bHit = pWrapper->GetDragArea().IsInside( pMEvt->GetPosPixel() );
            if ( pMEvt->IsLeft() )
            {
                // check if a single click initiated this sequence ( ImplStartDockingEnabled() )
                // check if window is docked and
                if( pWrapper->ImplStartDockingEnabled() && !pWrapper->IsFloatingMode() &&
                    !pWrapper->IsDocking() && bHit )
                {
                    Point   aPos = pMEvt->GetPosPixel();
                    vcl::Window* pWindow = rNEvt.GetWindow();
                    if ( pWindow != this )
                    {
                        aPos = pWindow->OutputToScreenPixel( aPos );
                        aPos = ScreenToOutputPixel( aPos );
                    }
                    pWrapper->ImplStartDocking( aPos );
                }
                return true;
            }
        }
        else if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            const vcl::KeyCode& rKey = rNEvt.GetKeyEvent()->GetKeyCode();
            if( rKey.GetCode() == KEY_F10 && rKey.GetModifier() &&
                rKey.IsShift() && rKey.IsMod1() )
            {
                pWrapper->SetFloatingMode( !pWrapper->IsFloatingMode() );
                /* At this point the floating toolbar frame does not have the
                 * input focus since these frames don't get the focus per default
                 * To enable keyboard handling of this toolbar set the input focus
                 * to the frame. This needs to be done with ToTop since GrabFocus
                 * would not notice any change since "this" already has the focus.
                 */
                if( pWrapper->IsFloatingMode() )
                    ToTop( ToTopFlags::GrabFocusOnly );
                return true;
            }
        }
    }

    // manage the dialogs
    if ( (GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL )
    {
        // if the parent also has dialog control activated, the parent takes over control
        if ( (rNEvt.GetType() == MouseNotifyEvent::KEYINPUT) || (rNEvt.GetType() == MouseNotifyEvent::KEYUP) )
        {
            if ( ImplIsOverlapWindow() ||
                 ((getNonLayoutRealParent(this)->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
            {
                bRet = ImplDlgCtrl( *rNEvt.GetKeyEvent(), rNEvt.GetType() == MouseNotifyEvent::KEYINPUT );
            }
        }
        else if ( (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS) || (rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS) )
        {
            ImplDlgCtrlFocusChanged( rNEvt.GetWindow(), rNEvt.GetType() == MouseNotifyEvent::GETFOCUS );
            if ( (rNEvt.GetWindow() == this) && (rNEvt.GetType() == MouseNotifyEvent::GETFOCUS) &&
                 !(GetStyle() & WB_TABSTOP) && !(mpWindowImpl->mnDlgCtrlFlags & DialogControlFlags::WantFocus) )
            {
                sal_uInt16 n = 0;
                vcl::Window* pFirstChild = ImplGetDlgWindow( n, GetDlgWindowType::First );
                if ( pFirstChild )
                    pFirstChild->ImplControlFocus();
            }
        }
    }

    if ( !bRet )
    {
        if ( mpWindowImpl->mpParent && !ImplIsOverlapWindow() )
            bRet = mpWindowImpl->mpParent->CompatNotify( rNEvt );
    }

    return bRet;
}

void Window::CallEventListeners( sal_uLong nEvent, void* pData )
{
    VclWindowEvent aEvent( this, nEvent, pData );

    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    Application::ImplCallEventListeners( aEvent );

    if ( aDelData.IsDead() )
        return;

    if (!mpWindowImpl->maEventListeners.empty())
    {
        // Copy the list, because this can be destroyed when calling a Link...
        std::vector<Link<VclWindowEvent&,void>> aCopy( mpWindowImpl->maEventListeners );
        for ( Link<VclWindowEvent&,void>& rLink : aCopy )
        {
            if (aDelData.IsDead()) break;
            // check this hasn't been removed in some re-enterancy scenario fdo#47368
            if( std::find(mpWindowImpl->maEventListeners.begin(), mpWindowImpl->maEventListeners.end(), rLink) != mpWindowImpl->maEventListeners.end() )
                rLink.Call( aEvent );
        }
    }

    if ( aDelData.IsDead() )
        return;

    ImplRemoveDel( &aDelData );

    vcl::Window* pWindow = this;
    while ( pWindow )
    {
        pWindow->ImplAddDel( &aDelData );

        if ( aDelData.IsDead() )
            return;

        auto& rChildListeners = pWindow->mpWindowImpl->maChildEventListeners;
        if (!rChildListeners.empty())
        {
            // Copy the list, because this can be destroyed when calling a Link...
            std::vector<Link<VclWindowEvent&,void>> aCopy( rChildListeners );
            for ( Link<VclWindowEvent&,void>& rLink : aCopy )
            {
                if (aDelData.IsDead())
                    return;
                // check this hasn't been removed in some re-enterancy scenario fdo#47368
                if( std::find(rChildListeners.begin(), rChildListeners.end(), rLink) != rChildListeners.end() )
                    rLink.Call( aEvent );
            }
        }

        if ( aDelData.IsDead() )
            return;

        pWindow->ImplRemoveDel( &aDelData );

        pWindow = pWindow->GetParent();
    }
}

void Window::FireVclEvent( VclSimpleEvent& rEvent )
{
    Application::ImplCallEventListeners(rEvent);
}

void Window::AddEventListener( const Link<VclWindowEvent&,void>& rEventListener )
{
    mpWindowImpl->maEventListeners.push_back( rEventListener );
}

void Window::RemoveEventListener( const Link<VclWindowEvent&,void>& rEventListener )
{
    if (mpWindowImpl)
    {
        auto& rListeners = mpWindowImpl->maEventListeners;
        rListeners.erase( std::remove(rListeners.begin(), rListeners.end(), rEventListener ), rListeners.end() );
    }
}

void Window::AddChildEventListener( const Link<VclWindowEvent&,void>& rEventListener )
{
    mpWindowImpl->maChildEventListeners.push_back( rEventListener );
}

void Window::RemoveChildEventListener( const Link<VclWindowEvent&,void>& rEventListener )
{
    if (mpWindowImpl)
    {
        auto& rListeners = mpWindowImpl->maChildEventListeners;
        rListeners.erase( std::remove(rListeners.begin(), rListeners.end(), rEventListener ), rListeners.end() );
    }
}

ImplSVEvent * Window::PostUserEvent( const Link<void*,void>& rLink, void* pCaller, bool bReferenceLink )
{
    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mpData    = pCaller;
    pSVEvent->maLink    = rLink;
    pSVEvent->mpWindow  = this;
    pSVEvent->mbCall    = true;
    if (bReferenceLink)
    {
        // Double check that this is indeed a vcl::Window instance.
        assert(dynamic_cast<vcl::Window *>(
                        static_cast<vcl::Window *>(rLink.GetInstance())) ==
               static_cast<vcl::Window *>(rLink.GetInstance()));
        pSVEvent->mpInstanceRef = static_cast<vcl::Window *>(rLink.GetInstance());
    }

    ImplAddDel( &(pSVEvent->maDelData) );
    if ( !mpWindowImpl->mpFrame->PostEvent( pSVEvent ) )
    {
        ImplRemoveDel( &(pSVEvent->maDelData) );
        delete pSVEvent;
        pSVEvent = nullptr;
    }
    return pSVEvent;
}

void Window::RemoveUserEvent( ImplSVEvent * nUserEvent )
{
    DBG_ASSERT( nUserEvent->mpWindow.get() == this,
                "Window::RemoveUserEvent(): Event doesn't send to this window or is already removed" );
    DBG_ASSERT( nUserEvent->mbCall,
                "Window::RemoveUserEvent(): Event is already removed" );

    if ( nUserEvent->mpWindow )
    {
        nUserEvent->mpWindow->ImplRemoveDel( &(nUserEvent->maDelData) );
        nUserEvent->mpWindow = nullptr;
    }

    nUserEvent->mbCall = false;
}


MouseEvent ImplTranslateMouseEvent( const MouseEvent& rE, vcl::Window* pSource, vcl::Window* pDest )
{
    Point aPos = pSource->OutputToScreenPixel( rE.GetPosPixel() );
    aPos = pDest->ScreenToOutputPixel( aPos );
    return MouseEvent( aPos, rE.GetClicks(), rE.GetMode(), rE.GetButtons(), rE.GetModifier() );
}

CommandEvent ImplTranslateCommandEvent( const CommandEvent& rCEvt, vcl::Window* pSource, vcl::Window* pDest )
{
    if ( !rCEvt.IsMouseEvent() )
        return rCEvt;

    Point aPos = pSource->OutputToScreenPixel( rCEvt.GetMousePosPixel() );
    aPos = pDest->ScreenToOutputPixel( aPos );
    return CommandEvent( aPos, rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetEventData() );
}

void Window::ImplNotifyKeyMouseCommandEventListeners( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == MouseNotifyEvent::COMMAND )
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        if ( pCEvt->GetCommand() != CommandEventId::ContextMenu )
            // non context menu events are not to be notified up the chain
            // so we return immediately
            return;

        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                // not interested in: The event listeners are already called in ::Command,
                // and calling them here a second time doesn't make sense
                ;
            else
            {
                CommandEvent aCommandEvent = ImplTranslateCommandEvent( *pCEvt, rNEvt.GetWindow(), this );
                CallEventListeners( VCLEVENT_WINDOW_COMMAND, &aCommandEvent );
            }
        }
    }

    // #82968# notify event listeners for mouse and key events separately and
    // not in PreNotify ( as for focus listeners )
    // this allows for processing those events internally first and pass it to
    // the toolkit later

    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    if( rNEvt.GetType() == MouseNotifyEvent::MOUSEMOVE )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                CallEventListeners( VCLEVENT_WINDOW_MOUSEMOVE, const_cast<MouseEvent *>(rNEvt.GetMouseEvent()) );
            else
            {
                MouseEvent aMouseEvent = ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this );
                CallEventListeners( VCLEVENT_WINDOW_MOUSEMOVE, &aMouseEvent );
            }
        }
    }
    else if( rNEvt.GetType() == MouseNotifyEvent::MOUSEBUTTONUP )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                CallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, const_cast<MouseEvent *>(rNEvt.GetMouseEvent()) );
            else
            {
                MouseEvent aMouseEvent = ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this );
                CallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, &aMouseEvent );
            }
        }
    }
    else if( rNEvt.GetType() == MouseNotifyEvent::MOUSEBUTTONDOWN )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                CallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, const_cast<MouseEvent *>(rNEvt.GetMouseEvent()) );
            else
            {
                MouseEvent aMouseEvent = ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this );
                CallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, &aMouseEvent );
            }
        }
    }
    else if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
            CallEventListeners( VCLEVENT_WINDOW_KEYINPUT, const_cast<KeyEvent *>(rNEvt.GetKeyEvent()) );
    }
    else if( rNEvt.GetType() == MouseNotifyEvent::KEYUP )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
            CallEventListeners( VCLEVENT_WINDOW_KEYUP, const_cast<KeyEvent *>(rNEvt.GetKeyEvent()) );
    }

    if ( aDelData.IsDead() )
        return;
    ImplRemoveDel( &aDelData );

    // #106721# check if we're part of a compound control and notify
    vcl::Window *pParent = ImplGetParent();
    while( pParent )
    {
        if( pParent->IsCompoundControl() )
        {
            pParent->ImplNotifyKeyMouseCommandEventListeners( rNEvt );
            break;
        }
        pParent = pParent->ImplGetParent();
    }
}

void Window::ImplCallInitShow()
{
    mpWindowImpl->mbReallyShown   = true;
    mpWindowImpl->mbInInitShow    = true;
    CompatStateChanged( StateChangedType::InitShow );
    mpWindowImpl->mbInInitShow    = false;

    vcl::Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplCallInitShow();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplCallInitShow();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}


void Window::ImplCallResize()
{
    mpWindowImpl->mbCallResize = false;

    // OpenGL has a charming feature of black clearing the whole window
    // some legacy code eg. the app-menu has the beautiful feature of
    // avoiding re-paints when width doesn't change => invalidate all.
    if( OpenGLWrapper::isVCLOpenGLEnabled() )
        Invalidate();

    // Normally we avoid blanking on re-size unless people might notice:
    else if( GetBackground().IsGradient() )
        Invalidate();

    Resize();

    // #88419# Most classes don't call the base class in Resize() and Move(),
    // => Call ImpleResize/Move instead of Resize/Move directly...
    CallEventListeners( VCLEVENT_WINDOW_RESIZE );
}

void Window::ImplCallMove()
{
    mpWindowImpl->mbCallMove = false;

    if( mpWindowImpl->mbFrame )
    {
        // update frame position
        SalFrame *pParentFrame = nullptr;
        vcl::Window *pParent = ImplGetParent();
        while( pParent )
        {
            if( pParent->mpWindowImpl->mpFrame != mpWindowImpl->mpFrame )
            {
                pParentFrame = pParent->mpWindowImpl->mpFrame;
                break;
            }
            pParent = pParent->GetParent();
        }

        SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
        mpWindowImpl->maPos = Point( g.nX, g.nY );
        if( pParentFrame )
        {
            g = pParentFrame->GetGeometry();
            mpWindowImpl->maPos -= Point( g.nX, g.nY );
        }
        // the client window and all its subclients have the same position as the borderframe
        // this is important for floating toolbars where the borderwindow is a floating window
        // which has another borderwindow (ie the system floating window)
        vcl::Window *pClientWin = mpWindowImpl->mpClientWindow;
        while( pClientWin )
        {
            pClientWin->mpWindowImpl->maPos = mpWindowImpl->maPos;
            pClientWin = pClientWin->mpWindowImpl->mpClientWindow;
        }
    }

    Move();

    CallEventListeners( VCLEVENT_WINDOW_MOVE );
}

void Window::ImplCallFocusChangeActivate( vcl::Window* pNewOverlapWindow,
                                          vcl::Window* pOldOverlapWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window*     pNewRealWindow;
    vcl::Window*     pOldRealWindow;
    bool bCallActivate = true;
    bool bCallDeactivate = true;

    pOldRealWindow = pOldOverlapWindow->ImplGetWindow();
    pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
    if ( (pOldRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
         pOldRealWindow->GetActivateMode() != ActivateModeFlags::NONE )
    {
        if ( (pNewRealWindow->GetType() == WINDOW_FLOATINGWINDOW) &&
             pNewRealWindow->GetActivateMode() == ActivateModeFlags::NONE)
        {
            pSVData->maWinData.mpLastDeacWin = pOldOverlapWindow;
            bCallDeactivate = false;
        }
    }
    else if ( (pNewRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
              pNewRealWindow->GetActivateMode() != ActivateModeFlags::NONE )
    {
        if ( pSVData->maWinData.mpLastDeacWin )
        {
            if ( pSVData->maWinData.mpLastDeacWin.get() == pNewOverlapWindow )
                bCallActivate = false;
            else
            {
                vcl::Window* pLastRealWindow = pSVData->maWinData.mpLastDeacWin->ImplGetWindow();
                pSVData->maWinData.mpLastDeacWin->mpWindowImpl->mbActive = false;
                pSVData->maWinData.mpLastDeacWin->Deactivate();
                if ( pLastRealWindow != pSVData->maWinData.mpLastDeacWin.get() )
                {
                    pLastRealWindow->mpWindowImpl->mbActive = true;
                    pLastRealWindow->Activate();
                }
            }
            pSVData->maWinData.mpLastDeacWin = nullptr;
        }
    }

    if ( bCallDeactivate )
    {
        if( pOldOverlapWindow->mpWindowImpl->mbActive )
        {
            pOldOverlapWindow->mpWindowImpl->mbActive = false;
            pOldOverlapWindow->Deactivate();
        }
        if ( pOldRealWindow != pOldOverlapWindow )
        {
            if( pOldRealWindow->mpWindowImpl->mbActive )
            {
                pOldRealWindow->mpWindowImpl->mbActive = false;
                pOldRealWindow->Deactivate();
            }
        }
    }
    if ( bCallActivate && ! pNewOverlapWindow->mpWindowImpl->mbActive )
    {
        if( ! pNewOverlapWindow->mpWindowImpl->mbActive )
        {
            pNewOverlapWindow->mpWindowImpl->mbActive = true;
            pNewOverlapWindow->Activate();
        }
        if ( pNewRealWindow != pNewOverlapWindow )
        {
            if( ! pNewRealWindow->mpWindowImpl->mbActive )
            {
                pNewRealWindow->mpWindowImpl->mbActive = true;
                pNewRealWindow->Activate();
            }
        }
    }
}

} /* namespace vcl */

NotifyEvent::NotifyEvent()
{
    mpWindow    = nullptr;
    mpData      = nullptr;
    mnEventType = MouseNotifyEvent::NONE;
    mnRetValue  = 0;
}

NotifyEvent::NotifyEvent( MouseNotifyEvent nEventType, vcl::Window* pWindow,
                          const void* pEvent, long nRet )
{
    mpWindow    = pWindow;
    mpData      = const_cast<void*>(pEvent);
    mnEventType  = nEventType;
    mnRetValue  = nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
