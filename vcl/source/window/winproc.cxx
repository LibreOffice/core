/*************************************************************************
 *
 *  $RCSfile: winproc.cxx,v $
 *
 *  $Revision: 1.81 $
 *
 *  last change: $Author: kz $ $Date: 2003-11-18 14:57:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_WINPROC_CXX

#ifndef _SV_SVSYS_HXX
#include <svsys.h>
#endif

#ifndef REMOTE_APPSERVER
#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#else
#include <rmwindow.hxx>
#include <rmevents.hxx>
#endif

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _INTN_HXX
#include <tools/intn.hxx>
#endif

#define private public
#ifndef _SV_SVDATA_HXX
#include <svdata.hxx>
#endif
#ifndef _SV_DBGGUI_HXX
#include <dbggui.hxx>
#endif
#ifndef _SV_WINDATA_HXX
#include <windata.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SOUND_HXX
#include <sound.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_CURSOR_HXX
#include <cursor.hxx>
#endif
#ifndef _SV_ACCMGR_HXX
#include <accmgr.hxx>
#endif
#ifndef _SV_PRINT_H
#include <print.h>
#endif
#ifndef _SV_WINDOW_H
#include <window.h>
#endif
#ifndef _SV_WRKWIN_HXX
#include <wrkwin.hxx>
#endif
#ifndef _SV_FLOATWIN_HXX
#include <floatwin.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <dialog.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <help.hxx>
#endif
#ifndef _SV_HELPWIN_HXX
#include <helpwin.hxx>
#endif
#ifndef _SV_BRDWIN_HXX
#include <brdwin.hxx>
#endif
#ifndef _SV_DOCKWIN_HXX
#include <dockwin.hxx>
#endif
#ifndef _SV_SALGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <menu.hxx>
#endif
#undef private

#include <dndlcon.hxx>

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDRAGSOURCE_HPP_
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_MOUSEEVENT_HPP_
#include <com/sun/star/awt/MouseEvent.hpp>
#endif

#pragma hdrstop


// =======================================================================

#define IMPL_MIN_NEEDSYSWIN         49

// =======================================================================

long ImplCallPreNotify( NotifyEvent& rEvt )
{
    long nRet = Application::CallEventHooks( rEvt );
    if ( !nRet )
        nRet = rEvt.GetWindow()->PreNotify( rEvt );
    return nRet;
}

// =======================================================================

long ImplCallEvent( NotifyEvent& rEvt )
{
    long nRet = ImplCallPreNotify( rEvt );
    if ( !nRet )
    {
        Window* pWindow = rEvt.GetWindow();
        switch ( rEvt.GetType() )
        {
            case EVENT_MOUSEBUTTONDOWN:
                pWindow->MouseButtonDown( *rEvt.GetMouseEvent() );
                break;
            case EVENT_MOUSEBUTTONUP:
                pWindow->MouseButtonUp( *rEvt.GetMouseEvent() );
                break;
            case EVENT_MOUSEMOVE:
                pWindow->MouseMove( *rEvt.GetMouseEvent() );
                break;
            case EVENT_KEYINPUT:
                pWindow->KeyInput( *rEvt.GetKeyEvent() );
                break;
            case EVENT_KEYUP:
                pWindow->KeyUp( *rEvt.GetKeyEvent() );
                break;
            case EVENT_GETFOCUS:
                pWindow->GetFocus();
                break;
            case EVENT_LOSEFOCUS:
                pWindow->LoseFocus();
                break;
            case EVENT_COMMAND:
                pWindow->Command( *rEvt.GetCommandEvent() );
                break;
        }
    }

    return nRet;
}

// =======================================================================

static BOOL ImplHandleMouseFloatMode( Window* pChild, const Point& rMousePos,
                                      USHORT nCode, USHORT nSVEvent,
                                      BOOL bMouseLeave )
{
    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maWinData.mpFirstFloat && !pSVData->maWinData.mpCaptureWin &&
         !pSVData->maWinData.mpFirstFloat->ImplIsFloatPopupModeWindow( pChild ) )
    {
        /*
         *  #93895# since floats are system windows, coordinates have
         *  to be converted to float relative for the hittest
         */
        USHORT          nHitTest = IMPL_FLOATWIN_HITTEST_OUTSIDE;
        FloatingWindow* pFloat = pSVData->maWinData.mpFirstFloat->ImplFloatHitTest( pChild, rMousePos, nHitTest );
        FloatingWindow* pLastLevelFloat;
        ULONG           nPopupFlags;
        if ( nSVEvent == EVENT_MOUSEMOVE )
        {
            if ( bMouseLeave )
                return TRUE;

            if ( !pFloat || (nHitTest & IMPL_FLOATWIN_HITTEST_RECT) )
            {
                if ( pSVData->maHelpData.mpHelpWin && !pSVData->maHelpData.mbKeyboardHelp )
                    ImplDestroyHelpWindow();
                pChild->mpFrame->SetPointer( POINTER_ARROW );
                return TRUE;
            }
        }
        else
        {
            if ( nCode & MOUSE_LEFT )
            {
                if ( nSVEvent == EVENT_MOUSEBUTTONDOWN )
                {
                    if ( !pFloat )
                    {
                        pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
                        nPopupFlags = pLastLevelFloat->GetPopupModeFlags();
                        pLastLevelFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
// Erstmal ausgebaut als Hack fuer Bug 53378
//                        if ( nPopupFlags & FLOATWIN_POPUPMODE_PATHMOUSECANCELCLICK )
//                            return FALSE;
//                        else
                            return TRUE;
                    }
                    else if ( nHitTest & IMPL_FLOATWIN_HITTEST_RECT )
                    {
                        if ( !(pFloat->GetPopupModeFlags() & FLOATWIN_POPUPMODE_NOMOUSERECTCLOSE) )
                            pFloat->ImplSetMouseDown();
                        return TRUE;
                    }
                }
                else
                {
                    if ( pFloat )
                    {
                        if ( nHitTest & IMPL_FLOATWIN_HITTEST_RECT )
                        {
                            if ( pFloat->ImplIsMouseDown() )
                                pFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL );
                            return TRUE;
                        }
                    }
                    else
                    {
                        pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
                        nPopupFlags = pLastLevelFloat->GetPopupModeFlags();
                        if ( !(nPopupFlags & FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE) )
                        {
                            pLastLevelFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
                            return TRUE;
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
                    if ( nPopupFlags & FLOATWIN_POPUPMODE_ALLMOUSEBUTTONCLOSE )
                    {
                        if ( (nPopupFlags & FLOATWIN_POPUPMODE_NOMOUSEUPCLOSE) &&
                             (nSVEvent == EVENT_MOUSEBUTTONDOWN) )
                            return TRUE;
                        pLastLevelFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
                        if ( nPopupFlags & FLOATWIN_POPUPMODE_PATHMOUSECANCELCLICK )
                            return FALSE;
                        else
                            return TRUE;
                    }
                    else
                        return TRUE;
                }
            }
        }
    }

    return FALSE;
}

// -----------------------------------------------------------------------

static void ImplHandleMouseHelpRequest( Window* pChild, const Point& rMousePos )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( !pSVData->maHelpData.mpHelpWin ||
         !( pSVData->maHelpData.mpHelpWin->IsWindowOrChild( pChild ) ||
           pChild->IsWindowOrChild( pSVData->maHelpData.mpHelpWin ) ) )
    {
        USHORT nHelpMode = 0;
        if ( pSVData->maHelpData.mbQuickHelp )
            nHelpMode = HELPMODE_QUICK;
        if ( pSVData->maHelpData.mbBalloonHelp )
            nHelpMode |= HELPMODE_BALLOON;
        if ( nHelpMode )
        {
            if ( pChild->IsInputEnabled() )
            {
                HelpEvent aHelpEvent( rMousePos, nHelpMode );
                pSVData->maHelpData.mbRequestingHelp = TRUE;
                pChild->RequestHelp( aHelpEvent );
                pSVData->maHelpData.mbRequestingHelp = FALSE;
            }
            // #104172# do not kill keyboard activated tooltips
            else if ( pSVData->maHelpData.mpHelpWin && !pSVData->maHelpData.mbKeyboardHelp)
            {
                ImplDestroyHelpWindow( FALSE );
            }
        }
    }
}

// -----------------------------------------------------------------------

static void ImplSetMousePointer( Window* pChild )
{
    ImplSVData* pSVData = ImplGetSVData();
    if ( pSVData->maHelpData.mbExtHelpMode )
        pChild->mpFrame->SetPointer( POINTER_HELP );
    else
        pChild->mpFrame->SetPointer( pChild->ImplGetMousePointer() );
}

// -----------------------------------------------------------------------

static BOOL ImplCallCommand( Window* pChild, USHORT nEvt, void* pData = NULL,
                             BOOL bMouse = FALSE, Point* pPos = NULL )
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
            Size aSize = pChild->GetOutputSize();
            aPos = Point( aSize.getWidth()/2, aSize.getHeight()/2 );
        }
    }

    CommandEvent        aCEvt( aPos, nEvt, bMouse, pData );
    NotifyEvent         aNCmdEvt( EVENT_COMMAND, pChild, &aCEvt );
    ImplDelData         aDelData;
    BOOL                bPreNotify;
    pChild->ImplAddDel( &aDelData );
    if ( !ImplCallPreNotify( aNCmdEvt ) && !aDelData.IsDelete() )
    {
        bPreNotify = FALSE;

        pChild->mbCommand = FALSE;
        pChild->Command( aCEvt );
    }
    else
        bPreNotify = TRUE;
    if ( aDelData.IsDelete() )
        return FALSE;
    pChild->ImplRemoveDel( &aDelData );
    if ( !bPreNotify && pChild->mbCommand )
        return TRUE;

    return FALSE;
}

// -----------------------------------------------------------------------

long ImplHandleMouseEvent( Window* pWindow, USHORT nSVEvent, BOOL bMouseLeave,
                           long nX, long nY, ULONG nMsgTime,
                           USHORT nCode, USHORT nMode )
{
    ImplSVData* pSVData = ImplGetSVData();
    Point       aMousePos( nX, nY );
    Window*     pChild;
    long        nRet;
    USHORT      nClicks;
    USHORT      nOldCode = pWindow->mpFrameData->mnMouseCode;

    // we need a mousemove event, befor we get a mousebuttondown or a
    // mousebuttonup event
    if ( (nSVEvent == EVENT_MOUSEBUTTONDOWN) ||
         (nSVEvent == EVENT_MOUSEBUTTONUP) )
    {
        if ( (nSVEvent == EVENT_MOUSEBUTTONUP) && pSVData->maHelpData.mbExtHelpMode )
            Help::EndExtHelp();
        if ( pSVData->maHelpData.mpHelpWin )
        {
            if( pWindow->ImplGetWindow() == pSVData->maHelpData.mpHelpWin )
            {
                ImplDestroyHelpWindow();
                return 1; // pWindow is dead now - avoid crash!
            }
            else
                ImplDestroyHelpWindow();
        }

        if ( (pWindow->mpFrameData->mnLastMouseX != nX) ||
             (pWindow->mpFrameData->mnLastMouseY != nY) )
        {
            ImplHandleMouseEvent( pWindow, EVENT_MOUSEMOVE, FALSE, nX, nY, nMsgTime, nCode, nMode );
        }
    }

    // update frame data
    pWindow->mpFrameData->mnLastMouseX = nX;
    pWindow->mpFrameData->mnLastMouseY = nY;
    pWindow->mpFrameData->mnMouseCode  = nCode;
    pWindow->mpFrameData->mnMouseMode  = nMode & ~(MOUSE_SYNTHETIC | MOUSE_MODIFIERCHANGED);
    if ( bMouseLeave )
    {
        pWindow->mpFrameData->mbMouseIn = FALSE;
        if ( pSVData->maHelpData.mpHelpWin && !pSVData->maHelpData.mbKeyboardHelp )
            ImplDestroyHelpWindow();
    }
    else
        pWindow->mpFrameData->mbMouseIn = TRUE;

    DBG_ASSERT( !pSVData->maWinData.mpTrackWin ||
                (pSVData->maWinData.mpTrackWin == pSVData->maWinData.mpCaptureWin),
                "ImplHandleMouseEvent: TrackWin != CaptureWin" );

    // AutoScrollMode
    if ( pSVData->maWinData.mpAutoScrollWin && (nSVEvent == EVENT_MOUSEBUTTONDOWN) )
    {
        pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();
        return 1;
    }

    // find mouse window
    if ( pSVData->maWinData.mpCaptureWin )
    {
        pChild = pSVData->maWinData.mpCaptureWin;

        DBG_ASSERT( pWindow == pChild->mpFrameWindow,
                    "ImplHandleMouseEvent: mouse event is not sent to capture window" );

        // java client cannot capture mouse correctly
        if ( pWindow != pChild->mpFrameWindow )
            return 0;

        if ( bMouseLeave )
            return 0;
    }
    else
    {
        if ( bMouseLeave )
            pChild = NULL;
        else
            pChild = pWindow->ImplFindWindow( aMousePos );
    }

    // test this because mouse events are buffered in the remote version
    // and size may not be in sync
    if ( !pChild && !bMouseLeave )
        return 0;

    // Ein paar Test ausfuehren und Message abfangen oder Status umsetzen
    if ( pChild )
    {
#ifndef REMOTE_APPSERVER
        if( pChild->ImplHasMirroredGraphics() && !pChild->IsRTLEnabled() )
        {
            // - RTL - re-mirror frame pos at pChild
            pChild->ImplReMirror( aMousePos );
        }
#endif
        // no mouse messages to system object windows
#ifndef REMOTE_APPSERVER
        if ( pChild->mpSysObj )
            return 0;
#endif

        // no mouse messages to disabled windows
        // #106845# if the window was disabed during capturing we have to pass the mouse events to release capturing
        if ( pSVData->maWinData.mpCaptureWin != pChild && (!pChild->IsEnabled() || !pChild->IsInputEnabled()) )
        {
            ImplHandleMouseFloatMode( pChild, aMousePos, nCode, nSVEvent, bMouseLeave );
            if ( nSVEvent == EVENT_MOUSEMOVE )
                ImplHandleMouseHelpRequest( pChild, aMousePos );

            // Call the hook also, if Window is disabled
            Point aChildPos = pChild->ImplFrameToOutput( aMousePos );
            MouseEvent aMEvt( aChildPos, pWindow->mpFrameData->mnClickCount, nMode, nCode, nCode );
            NotifyEvent aNEvt( nSVEvent, pChild, &aMEvt );
            Application::CallEventHooks( aNEvt );

            if ( nSVEvent == EVENT_MOUSEBUTTONDOWN )
            {
                Sound::Beep( SOUND_DISABLE, pChild );
                return 1;
            }
            else
            {
                // Set normal MousePointer for disabled windows
                if ( nSVEvent == EVENT_MOUSEMOVE )
                    ImplSetMousePointer( pChild );

                return 0;
            }
        }

        // End ExtTextInput-Mode, if the user click in the same TopLevel Window
        if ( pSVData->maWinData.mpExtTextInputWin &&
             ((nSVEvent == EVENT_MOUSEBUTTONDOWN) ||
              (nSVEvent == EVENT_MOUSEBUTTONUP)) )
            pSVData->maWinData.mpExtTextInputWin->EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );
    }

    // determine mouse event data
    if ( nSVEvent == EVENT_MOUSEMOVE )
    {
        // Testen, ob MouseMove an das gleiche Fenster geht und sich der
        // Status nicht geaendert hat
        if ( pChild )
        {
            Point aChildMousePos = pChild->ImplFrameToOutput( aMousePos );
            if ( !bMouseLeave &&
                 (pChild == pWindow->mpFrameData->mpMouseMoveWin) &&
                 (aChildMousePos.X() == pWindow->mpFrameData->mnLastMouseWinX) &&
                 (aChildMousePos.Y() == pWindow->mpFrameData->mnLastMouseWinY) &&
                 (nOldCode == pWindow->mpFrameData->mnMouseCode) )
            {
                // Mouse-Pointer neu setzen, da er sich geaendet haben
                // koennte, da ein Modus umgesetzt wurde
                ImplSetMousePointer( pChild );
                return 0;
            }

            pWindow->mpFrameData->mnLastMouseWinX = aChildMousePos.X();
            pWindow->mpFrameData->mnLastMouseWinY = aChildMousePos.Y();
        }

        // mouse click
        nClicks = pWindow->mpFrameData->mnClickCount;

        // Gegebenenfalls den Start-Drag-Handler rufen.
        // Achtung: Muss vor Move gerufen werden, da sonst bei schnellen
        // Mausbewegungen die Applikationen in den Selektionszustand gehen.
        Window* pMouseDownWin = pWindow->mpFrameData->mpMouseDownWin;
        if ( pMouseDownWin )
        {
            // Testen, ob StartDrag-Modus uebereinstimmt. Wir vergleichen nur
            // den Status der Maustasten, damit man mit Mod1 z.B. sofort
            // in den Kopiermodus gehen kann.
            const MouseSettings& rMSettings = pMouseDownWin->GetSettings().GetMouseSettings();
            if ( (nCode & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)) ==
                 (rMSettings.GetStartDragCode() & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)) )
            {
                if ( !pMouseDownWin->mpFrameData->mbStartDragCalled )
                {
                    long nDragW  = rMSettings.GetStartDragWidth();
                    long nDragH  = rMSettings.GetStartDragWidth();
                    //long nMouseX = nX;
                    //long nMouseY = nY;
                    long nMouseX = aMousePos.X(); // #106074# use the possibly re-mirrored coordinates (RTL) ! nX,nY are unmodified !
                    long nMouseY = aMousePos.Y();
                    if ( !(((nMouseX-nDragW) <= pMouseDownWin->mpFrameData->mnFirstMouseX) &&
                           ((nMouseX+nDragW) >= pMouseDownWin->mpFrameData->mnFirstMouseX)) ||
                         !(((nMouseY-nDragH) <= pMouseDownWin->mpFrameData->mnFirstMouseY) &&
                           ((nMouseY+nDragH) >= pMouseDownWin->mpFrameData->mnFirstMouseY)) )
                    {
                        pMouseDownWin->mpFrameData->mbStartDragCalled  = TRUE;

#if 0
                        /*
                         * old drag and drop api
                         */

                        Point aCmdMousePos( pMouseDownWin->mpFrameData->mnFirstMouseX,
                                            pMouseDownWin->mpFrameData->mnFirstMouseY );
                        aCmdMousePos = pMouseDownWin->ImplFrameToOutput( aCmdMousePos );
                        CommandEvent    aCEvt( aCmdMousePos, COMMAND_STARTDRAG, TRUE );
                        NotifyEvent     aNCmdEvt( EVENT_COMMAND, pMouseDownWin, &aCEvt );
                        ImplDelData     aDelData;
                        pMouseDownWin->ImplAddDel( &aDelData );
                        if ( !ImplCallPreNotify( aNCmdEvt ) )
                            pMouseDownWin->Command( aCEvt );
                        if ( aDelData.IsDelete() )
                            return 1;
                        pMouseDownWin->ImplRemoveDel( &aDelData );
#endif

                        /*
                         * new drag and drop api
                         */

                        // Check if drag source provides it's own recognizer
                        if( pMouseDownWin->mpFrameData->mbInternalDragGestureRecognizer )
                        {
                            // query DropTarget from child window
                            ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer > xDragGestureRecognizer =
                                ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer > ( pMouseDownWin->mxDNDListenerContainer,
                                    ::com::sun::star::uno::UNO_QUERY );

                            if( xDragGestureRecognizer.is() )
                            {
                                // retrieve mouse position relative to mouse down window
                                Point relLoc = pMouseDownWin->ImplFrameToOutput( Point(
                                    pMouseDownWin->mpFrameData->mnFirstMouseX,
                                    pMouseDownWin->mpFrameData->mnFirstMouseY ) );

                                // create a uno mouse event out of the available data
                                ::com::sun::star::awt::MouseEvent aMouseEvent(
                                    static_cast < ::com::sun::star::uno::XInterface * > ( 0 ),
                                    nCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2),
                                    nCode & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE),
                                    nMouseX,
                                    nMouseY,
                                    nClicks,
                                    sal_False );

                                ULONG nCount = Application::ReleaseSolarMutex();

                                // FIXME: where do I get Action from ?
                                ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource > xDragSource = pMouseDownWin->GetDragSource();

                                if( xDragSource.is() )
                                {
                                    static_cast < DNDListenerContainer * > ( xDragGestureRecognizer.get() )->fireDragGestureEvent( 0,
                                        relLoc.X(), relLoc.Y(), xDragSource, ::com::sun::star::uno::makeAny( aMouseEvent ) );
                                }

                                Application::AcquireSolarMutex( nCount );
                            }
                        }
                    }
                }
            }
            else
                pMouseDownWin->mpFrameData->mbStartDragCalled  = TRUE;
        }

        // test for mouseleave and mouseenter
        Window* pMouseMoveWin = pWindow->mpFrameData->mpMouseMoveWin;
        if ( pChild != pMouseMoveWin )
        {
            if ( pMouseMoveWin )
            {
                Point       aLeaveMousePos = pMouseMoveWin->ImplFrameToOutput( aMousePos );
                MouseEvent  aMLeaveEvt( aLeaveMousePos, nClicks, nMode | MOUSE_LEAVEWINDOW, nCode, nCode );
                NotifyEvent aNLeaveEvt( EVENT_MOUSEMOVE, pMouseMoveWin, &aMLeaveEvt );
                ImplDelData aDelData;
                ImplDelData aDelData2;
                pWindow->mpFrameData->mbInMouseMove = TRUE;
                pMouseMoveWin->ImplAddDel( &aDelData );
                // Durch MouseLeave kann auch dieses Fenster zerstoert
                // werden
                if ( pChild )
                    pChild->ImplAddDel( &aDelData2 );
                if ( !ImplCallPreNotify( aNLeaveEvt ) )
                {
                    pMouseMoveWin->MouseMove( aMLeaveEvt );
                    // #82968#
                    if( !aDelData.IsDelete() )
                        aNLeaveEvt.GetWindow()->ImplNotifyKeyMouseEventListeners( aNLeaveEvt );
                }

                pWindow->mpFrameData->mpMouseMoveWin = NULL;
                pWindow->mpFrameData->mbInMouseMove = FALSE;

                if ( pChild )
                {
                    if ( aDelData2.IsDelete() )
                        pChild = NULL;
                    else
                        pChild->ImplRemoveDel( &aDelData2 );
                }
                if ( aDelData.IsDelete() )
                    return 1;
                pMouseMoveWin->ImplRemoveDel( &aDelData );
            }

            nMode |= MOUSE_ENTERWINDOW;
        }
        pWindow->mpFrameData->mpMouseMoveWin = pChild;

        // MouseLeave
        if ( !pChild )
            return 0;
    }
    else
    {
        // mouse click
        if ( nSVEvent == EVENT_MOUSEBUTTONDOWN )
        {
            const MouseSettings& rMSettings = pChild->GetSettings().GetMouseSettings();
            ULONG   nDblClkTime = rMSettings.GetDoubleClickTime();
            long    nDblClkW    = rMSettings.GetDoubleClickWidth();
            long    nDblClkH    = rMSettings.GetDoubleClickHeight();
            //long    nMouseX     = nX;
            //long    nMouseY     = nY;
            long nMouseX = aMousePos.X();   // #106074# use the possibly re-mirrored coordinates (RTL) ! nX,nY are unmodified !
            long nMouseY = aMousePos.Y();

            if ( (pChild == pChild->mpFrameData->mpMouseDownWin) &&
                 (nCode == pChild->mpFrameData->mnFirstMouseCode) &&
                 ((nMsgTime-pChild->mpFrameData->mnMouseDownTime) < nDblClkTime) &&
                 ((nMouseX-nDblClkW) <= pChild->mpFrameData->mnFirstMouseX) &&
                 ((nMouseX+nDblClkW) >= pChild->mpFrameData->mnFirstMouseX) &&
                 ((nMouseY-nDblClkH) <= pChild->mpFrameData->mnFirstMouseY) &&
                 ((nMouseY+nDblClkH) >= pChild->mpFrameData->mnFirstMouseY) )
            {
                pChild->mpFrameData->mnClickCount++;
                pChild->mpFrameData->mbStartDragCalled  = TRUE;
            }
            else
            {
                pChild->mpFrameData->mpMouseDownWin     = pChild;
                pChild->mpFrameData->mnClickCount       = 1;
                pChild->mpFrameData->mnFirstMouseX      = nMouseX;
                pChild->mpFrameData->mnFirstMouseY      = nMouseY;
                pChild->mpFrameData->mnFirstMouseCode   = nCode;
                pChild->mpFrameData->mbStartDragCalled  = !((nCode & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)) ==
                                                            (rMSettings.GetStartDragCode() & (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)));
            }
            pChild->mpFrameData->mnMouseDownTime = nMsgTime;
        }
        nClicks = pChild->mpFrameData->mnClickCount;

        pSVData->maAppData.mnLastInputTime = Time::GetSystemTicks();
    }

    DBG_ASSERT( pChild, "ImplHandleMouseEvent: pChild == NULL" );

    // create mouse event
    Point aChildPos = pChild->ImplFrameToOutput( aMousePos );
    MouseEvent aMEvt( aChildPos, nClicks, nMode, nCode, nCode );

    // tracking window gets the mouse events
    BOOL bTracking = FALSE;
    if ( pSVData->maWinData.mpTrackWin )
        pChild = pSVData->maWinData.mpTrackWin;

    // handle FloatingMode
    if ( !pSVData->maWinData.mpTrackWin && pSVData->maWinData.mpFirstFloat )
    {
        ImplDelData aDelData;
        pChild->ImplAddDel( &aDelData );
        if ( ImplHandleMouseFloatMode( pChild, aMousePos, nCode, nSVEvent, bMouseLeave ) )
        {
            if ( !aDelData.IsDelete() )
            {
                pChild->ImplRemoveDel( &aDelData );
                pChild->mpFrameData->mbStartDragCalled = TRUE;
            }
            return 1;
        }
        else
            pChild->ImplRemoveDel( &aDelData );
    }

    // call handler
    BOOL bDrag = FALSE;
    BOOL bCallHelpRequest = TRUE;
    DBG_ASSERT( pChild, "ImplHandleMouseEvent: pChild is NULL" );

    ImplDelData aDelData;
    NotifyEvent aNEvt( nSVEvent, pChild, &aMEvt );
    pChild->ImplAddDel( &aDelData );
    if ( nSVEvent == EVENT_MOUSEMOVE )
        pChild->mpFrameData->mbInMouseMove = TRUE;

    // Fenster bei Klick nach vorne bringen
    if ( nSVEvent == EVENT_MOUSEBUTTONDOWN )
    {
        if( !pSVData->maWinData.mpFirstFloat ) // totop for floating windows would change the focus
            pChild->ToTop();
        if ( aDelData.IsDelete() )
            return 1;
    }

    if ( ImplCallPreNotify( aNEvt ) || aDelData.IsDelete() )
        nRet = 1;
    else
    {
        nRet = 0;
        if ( nSVEvent == EVENT_MOUSEMOVE )
        {
            if ( pSVData->maWinData.mpTrackWin )
            {
                TrackingEvent aTEvt( aMEvt );
                pChild->Tracking( aTEvt );
                if ( !aDelData.IsDelete() )
                {
                    // When ScrollRepeat, we restart the timer
                    if ( pSVData->maWinData.mpTrackTimer &&
                         (pSVData->maWinData.mnTrackFlags & STARTTRACK_SCROLLREPEAT) )
                        pSVData->maWinData.mpTrackTimer->Start();
                }
                bCallHelpRequest = FALSE;
                nRet = 1;
            }
            else
            {
                // Auto-ToTop
                if ( !pSVData->maWinData.mpCaptureWin &&
                     (pChild->GetSettings().GetMouseSettings().GetOptions() & MOUSE_OPTION_AUTOFOCUS) )
                    pChild->ToTop( TOTOP_NOGRABFOCUS );

                // Wenn Hilfe-Fenster im MouseMove angezeigt/gehidet wird,
                // wird danach nicht mehr der HelpRequest-Handler gerufen
                Window* pOldHelpTextWin = pSVData->maHelpData.mpHelpWin;
                pChild->mbMouseMove = FALSE;
                pChild->MouseMove( aMEvt );
                if ( pOldHelpTextWin != pSVData->maHelpData.mpHelpWin )
                    bCallHelpRequest = FALSE;
            }
        }
        else if ( nSVEvent == EVENT_MOUSEBUTTONDOWN )
        {
            if ( pSVData->maWinData.mpTrackWin &&
                 !(pSVData->maWinData.mnTrackFlags & STARTTRACK_MOUSEBUTTONDOWN) )
                nRet = 1;
            else
            {
                pChild->mbMouseButtonDown = FALSE;
                pChild->MouseButtonDown( aMEvt );
            }
        }
        else
        {
            if ( pSVData->maWinData.mpTrackWin )
            {
                pChild->EndTracking();
                nRet = 1;
            }
            else
            {
                pChild->mbMouseButtonUp = FALSE;
                pChild->MouseButtonUp( aMEvt );
            }
        }

        // #82968#
        if ( !aDelData.IsDelete() )
            aNEvt.GetWindow()->ImplNotifyKeyMouseEventListeners( aNEvt );
    }

    if ( aDelData.IsDelete() )
        return 1;


    if ( nSVEvent == EVENT_MOUSEMOVE )
        pChild->mpFrameData->mbInMouseMove = FALSE;

    if ( nSVEvent == EVENT_MOUSEMOVE )
    {
        if ( bCallHelpRequest && !pSVData->maHelpData.mbKeyboardHelp )
            ImplHandleMouseHelpRequest( pChild, pChild->OutputToScreenPixel( aMEvt.GetPosPixel() ) );
        nRet = 1;
    }
    else if ( !nRet )
    {
        if ( nSVEvent == EVENT_MOUSEBUTTONDOWN )
        {
            if ( !pChild->mbMouseButtonDown )
                nRet = 1;
        }
        else
        {
            if ( !pChild->mbMouseButtonUp )
                nRet = 1;
        }
    }

    pChild->ImplRemoveDel( &aDelData );

    if ( nSVEvent == EVENT_MOUSEMOVE )
    {
        // set new mouse pointer
        if ( !bMouseLeave )
            ImplSetMousePointer( pChild );
    }
    else if ( (nSVEvent == EVENT_MOUSEBUTTONDOWN) || (nSVEvent == EVENT_MOUSEBUTTONUP) )
    {
        if ( !bDrag )
        {
            // Command-Events
            if ( /*(nRet == 0) &&*/ (nClicks == 1) && (nSVEvent == EVENT_MOUSEBUTTONDOWN) &&
                 (nCode == MOUSE_MIDDLE) )
            {
                USHORT nMiddleAction = pChild->GetSettings().GetMouseSettings().GetMiddleButtonAction();
                if ( nMiddleAction == MOUSE_MIDDLE_AUTOSCROLL )
                    nRet = !ImplCallCommand( pChild, COMMAND_STARTAUTOSCROLL, NULL, TRUE, &aChildPos );
                else if ( nMiddleAction == MOUSE_MIDDLE_PASTESELECTION )
                    nRet = !ImplCallCommand( pChild, COMMAND_PASTESELECTION, NULL, TRUE, &aChildPos );
            }
            else
            {
                // ContextMenu
                const MouseSettings& rMSettings = pChild->GetSettings().GetMouseSettings();
                if ( (nCode == rMSettings.GetContextMenuCode()) &&
                     (nClicks == rMSettings.GetContextMenuClicks()) )
                {
                    BOOL bContextMenu;
                    if ( rMSettings.GetContextMenuDown() )
                        bContextMenu = (nSVEvent == EVENT_MOUSEBUTTONDOWN);
                    else
                        bContextMenu = (nSVEvent == EVENT_MOUSEBUTTONUP);
                    if ( bContextMenu )
                        nRet = !ImplCallCommand( pChild, COMMAND_CONTEXTMENU, NULL, TRUE, &aChildPos );
                }
            }
        }
    }

    return nRet;
}

// -----------------------------------------------------------------------

static Window* ImplGetKeyInputWindow( Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();

    // determine last input time
    pSVData->maAppData.mnLastInputTime = Time::GetSystemTicks();

    // find window - is every time the window which has currently the
    // focus or the last time the focus.
    // the first floating window always has the focus
    Window* pChild = pSVData->maWinData.mpFirstFloat;
    if( !pChild || ( pChild->mbFloatWin && !((FloatingWindow *)pChild)->GrabsFocus() ) )
        pChild = pWindow->mpFrameData->mpFocusWin;
    else
    {
        // allow floaters to forward keyinput to some member
        pChild = pChild->GetPreferredKeyInputWindow();
    }

    // no child - than no input
    if ( !pChild )
        return 0;

    // We call also KeyInput if we haven't the focus, because on Unix
    // system this is often the case when a Lookup Choise Window has
    // the focus - because this windows send the KeyInput directly to
    // the window without resetting the focus
    DBG_ASSERTWARNING( pChild == pSVData->maWinData.mpFocusWin,
                       "ImplHandleKey: Keyboard-Input is sent to a frame without focus" );

    // no keyinput to disabled windows
    if ( !pChild->IsEnabled() || !pChild->IsInputEnabled() )
        return 0;

    return pChild;
}

// -----------------------------------------------------------------------

static long ImplHandleKey( Window* pWindow, USHORT nSVEvent,
                           USHORT nKeyCode, USHORT nCharCode, USHORT nRepeat, BOOL bForward )
{
    ImplSVData* pSVData = ImplGetSVData();
    KeyCode     aKeyCode( nKeyCode, nKeyCode );
    USHORT      nCode = aKeyCode.GetCode();

    // allow application key listeners to remove the key event
    // but make sure we're not forwarding external KeyEvents, (ie where bForward is FALSE)
    // becasue those are coming back from the listener itself and MUST be processed
    KeyEvent aKeyEvent( (xub_Unicode)nCharCode, aKeyCode, nRepeat );
    if( bForward )
    {
        USHORT nVCLEvent;
        switch( nSVEvent )
        {
            case EVENT_KEYINPUT:
                nVCLEvent = VCLEVENT_WINDOW_KEYINPUT;
                break;
            case EVENT_KEYUP:
                nVCLEvent = VCLEVENT_WINDOW_KEYUP;
                break;
            default:
                nVCLEvent = 0;
                break;
        }
        if( nVCLEvent && pSVData->mpApp->HandleKey( nVCLEvent, pWindow, &aKeyEvent ) )
            return 1;
    }

    BOOL bCtrlF6 = (aKeyCode.GetCode() == KEY_F6) && aKeyCode.IsMod1();

    // determine last input time
    pSVData->maAppData.mnLastInputTime = Time::GetSystemTicks();

    // handle tracking window
    if ( nSVEvent == EVENT_KEYINPUT )
    {
#ifdef DBG_UTIL
#ifdef REMOTE_APPSERVER
        if ( aKeyCode.IsShift() && aKeyCode.IsMod2() && (aKeyCode.GetCode() == KEY_D) )
#else
        // #105224# use Ctrl-Alt-Shift-D, Ctrl-Shift-D must be useable by app
        if ( aKeyCode.IsShift() && aKeyCode.IsMod1() && aKeyCode.IsMod2() && (aKeyCode.GetCode() == KEY_D) )
#endif
        {
            DBGGUI_START();
            return 1;
        }
#endif

        if ( pSVData->maHelpData.mbExtHelpMode )
        {
            Help::EndExtHelp();
            if ( nCode == KEY_ESCAPE )
                return 1;
        }
        if ( pSVData->maHelpData.mpHelpWin )
            ImplDestroyHelpWindow();

        // AutoScrollMode
        if ( pSVData->maWinData.mpAutoScrollWin )
        {
            pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();
            if ( nCode == KEY_ESCAPE )
                return 1;
        }

        if ( pSVData->maWinData.mpTrackWin )
        {
            USHORT nCode = aKeyCode.GetCode();

            if ( (nCode == KEY_ESCAPE) && !(pSVData->maWinData.mnTrackFlags & STARTTRACK_NOKEYCANCEL) )
            {
                pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL | ENDTRACK_KEY );
                if ( pSVData->maWinData.mpFirstFloat )
                {
                    FloatingWindow* pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
                    if ( !(pLastLevelFloat->GetPopupModeFlags() & FLOATWIN_POPUPMODE_NOKEYCLOSE) )
                    {
                        USHORT nCode = aKeyCode.GetCode();

                        if ( nCode == KEY_ESCAPE )
                            pLastLevelFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
                    }
                }
                return 1;
            }
            else if ( nCode == KEY_RETURN )
            {
                pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_KEY );
                return 1;
            }
            else if ( !(pSVData->maWinData.mnTrackFlags & STARTTRACK_KEYINPUT) )
                return 1;
        }

        // handle FloatingMode
        if ( pSVData->maWinData.mpFirstFloat )
        {
            FloatingWindow* pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
            if ( !(pLastLevelFloat->GetPopupModeFlags() & FLOATWIN_POPUPMODE_NOKEYCLOSE) )
            {
                USHORT nCode = aKeyCode.GetCode();

                if ( (nCode == KEY_ESCAPE) || bCtrlF6)
                {
                    pLastLevelFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
                    if( !bCtrlF6 )
                        return 1;
                }
            }
        }

        // test for accel
        if ( pSVData->maAppData.mpAccelMgr )
        {
            if ( pSVData->maAppData.mpAccelMgr->IsAccelKey( aKeyCode, nRepeat ) )
                return 1;
        }
    }

    // find window
    Window* pChild = ImplGetKeyInputWindow( pWindow );
    if ( !pChild )
        return 0;

    // --- RTL --- mirror cursor keys
    if( (aKeyCode.GetCode() == KEY_LEFT || aKeyCode.GetCode() == KEY_RIGHT) &&
      pChild->ImplHasMirroredGraphics() && pChild->IsRTLEnabled() )
        aKeyCode = KeyCode( aKeyCode.GetCode() == KEY_LEFT ? KEY_RIGHT : KEY_LEFT, aKeyCode.GetModifier() );

    // call handler
    ImplDelData aDelData;
    KeyEvent    aKEvt( (xub_Unicode)nCharCode, aKeyCode, nRepeat );
    NotifyEvent aNEvt( nSVEvent, pChild, &aKEvt );
    BOOL        bPreNotify;
    long        nRet = 1;

    pChild->ImplAddDel( &aDelData );
    if ( !ImplCallPreNotify( aNEvt ) && !aDelData.IsDelete() )
    {
         bPreNotify = FALSE;

        if ( nSVEvent == EVENT_KEYINPUT )
        {
            pChild->mbKeyInput = FALSE;
            pChild->KeyInput( aKEvt );
        }
        else
        {
            pChild->mbKeyUp = FALSE;
            pChild->KeyUp( aKEvt );
        }
        // #82968#
        if( !aDelData.IsDelete() )
            aNEvt.GetWindow()->ImplNotifyKeyMouseEventListeners( aNEvt );
    }
    else
        bPreNotify = TRUE;

    if ( aDelData.IsDelete() )
        return 1;

    pChild->ImplRemoveDel( &aDelData );

    if ( nSVEvent == EVENT_KEYINPUT )
    {
        if ( !bPreNotify && pChild->mbKeyInput )
        {
            USHORT nCode = aKeyCode.GetCode();

            // #101999# is focus in or below toolbox
            BOOL bToolboxFocus=FALSE;
            if( (nCode == KEY_F1) && aKeyCode.IsShift() )
            {
                Window *pWin = pWindow->mpFrameData->mpFocusWin;
                while( pWin )
                {
                    if( pWin->mbToolBox )
                    {
                        bToolboxFocus = TRUE;
                        break;
                    }
                    else
                        pWin = pWin->GetParent();
                }
            }

            // ContextMenu
            if ( (nCode == KEY_CONTEXTMENU) || ((nCode == KEY_F10) && aKeyCode.IsShift() && !aKeyCode.IsMod1() && !aKeyCode.IsMod2() ) )
                nRet = !ImplCallCommand( pChild, COMMAND_CONTEXTMENU, NULL, FALSE );
            else if ( ( (nCode == KEY_F2) && aKeyCode.IsShift() ) || ( (nCode == KEY_F1) && aKeyCode.IsMod1() ) ||
                // #101999# no active help when focus in toolbox, simulate BallonHelp instead
                ( (nCode == KEY_F1) && aKeyCode.IsShift() && bToolboxFocus ) )
            {
                // TipHelp via Keyboard (Shift-F2 or Ctrl-F1)
                // simulate mouseposition at center of window

                Size aSize = pChild->GetOutputSize();
                Point aPos = Point( aSize.getWidth()/2, aSize.getHeight()/2 );
                aPos = pChild->OutputToScreenPixel( aPos );

                HelpEvent aHelpEvent( aPos, HELPMODE_BALLOON );
                aHelpEvent.SetKeyboardActivated( TRUE );
                pSVData->maHelpData.mbKeyboardHelp = TRUE;
                pChild->RequestHelp( aHelpEvent );
            }
            else if ( (nCode == KEY_F1) || (nCode == KEY_HELP) )
            {
                if ( !aKeyCode.GetModifier() )
                {
                    if ( pSVData->maHelpData.mbContextHelp )
                    {
                        Point       aMousePos = pChild->OutputToScreenPixel( pChild->GetPointerPosPixel() );
                        HelpEvent   aHelpEvent( aMousePos, HELPMODE_CONTEXT );
                        pChild->RequestHelp( aHelpEvent );
                    }
                    else
                        nRet = 0;
                }
                else if ( aKeyCode.IsShift() )
                {
                    if ( pSVData->maHelpData.mbExtHelp )
                        Help::StartExtHelp();
                    else
                        nRet = 0;
                }
            }
            else
            {
                if ( ImplCallHotKey( aKeyCode ) )
                    nRet = 1;
                else
                    nRet = 0;
            }
        }
    }
    else
    {
        if ( !bPreNotify && pChild->mbKeyUp )
            nRet = 0;
    }

    // #105591# send keyinput to parent if we are a floating window and the key was not pocessed yet
    if( !nRet && pWindow->mbFloatWin && pWindow->GetParent() && (pWindow->mpFrame != pWindow->GetParent()->mpFrame) )
    {
        pChild = pWindow->GetParent();

        // call handler
        ImplDelData aDelData;
        KeyEvent    aKEvt( (xub_Unicode)nCharCode, aKeyCode, nRepeat );
        NotifyEvent aNEvt( nSVEvent, pChild, &aKEvt );
        BOOL        bPreNotify;

        pChild->ImplAddDel( &aDelData );
        if ( !ImplCallPreNotify( aNEvt ) && !aDelData.IsDelete() )
        {
            bPreNotify = FALSE;

            if ( nSVEvent == EVENT_KEYINPUT )
            {
                pChild->mbKeyInput = FALSE;
                pChild->KeyInput( aKEvt );
            }
            else
            {
                pChild->mbKeyUp = FALSE;
                pChild->KeyUp( aKEvt );
            }
            // #82968#
            if( !aDelData.IsDelete() )
                aNEvt.GetWindow()->ImplNotifyKeyMouseEventListeners( aNEvt );
        }
        else
            bPreNotify = TRUE;

        if ( aDelData.IsDelete() )
            return 1;

        pChild->ImplRemoveDel( &aDelData );

        if( bPreNotify || !pChild->mbKeyInput )
            nRet = 1;
    }

    return nRet;
}

// -----------------------------------------------------------------------

static long ImplHandleExtTextInput( Window* pWindow, ULONG nTime,
                                    const XubString& rText,
                                    const USHORT* pTextAttr,
                                    ULONG nCursorPos, USHORT nCursorFlags )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pChild = pSVData->maWinData.mpExtTextInputWin;

    if ( !pChild )
    {
        pChild = ImplGetKeyInputWindow( pWindow );
        if ( !pChild )
            return 0;
    }

    // If it is the first ExtTextInput call, we inform the information
    // and allocate the data, which we must store in this mode
    ImplWinData* pWinData = pChild->ImplGetWinData();
    if ( !pChild->mbExtTextInput )
    {
        pChild->mbExtTextInput = TRUE;
        if ( !pWinData->mpExtOldText )
            pWinData->mpExtOldText = new UniString;
        else
            pWinData->mpExtOldText->Erase();
        if ( pWinData->mpExtOldAttrAry )
        {
            delete [] pWinData->mpExtOldAttrAry;
            pWinData->mpExtOldAttrAry = NULL;
        }
        pSVData->maWinData.mpExtTextInputWin = pChild;
        ImplCallCommand( pChild, COMMAND_STARTEXTTEXTINPUT );
    }

    // be aware of being recursively called in StartExtTextInput
    if ( !pChild->mbExtTextInput )
    {
        return 0;
    }

    // Test for changes
    BOOL        bOnlyCursor = FALSE;
    xub_StrLen  nMinLen = Min( pWinData->mpExtOldText->Len(), rText.Len() );
    xub_StrLen  nDeltaStart = 0;
    while ( nDeltaStart < nMinLen )
    {
        if ( pWinData->mpExtOldText->GetChar( nDeltaStart ) != rText.GetChar( nDeltaStart ) )
            break;
        nDeltaStart++;
    }
    if ( pWinData->mpExtOldAttrAry || pTextAttr )
    {
        if ( !pWinData->mpExtOldAttrAry || !pTextAttr )
            nDeltaStart = 0;
        else
        {
            xub_StrLen i = 0;
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
         (pWinData->mpExtOldText->Len() == rText.Len()) )
        bOnlyCursor = TRUE;

    // Call Event and store the information
    CommandExtTextInputData aData( rText, pTextAttr,
                                   (xub_StrLen)nCursorPos, nCursorFlags,
                                   nDeltaStart, pWinData->mpExtOldText->Len(),
                                   bOnlyCursor );
    *pWinData->mpExtOldText = rText;
    if ( pWinData->mpExtOldAttrAry )
    {
        delete [] pWinData->mpExtOldAttrAry;
        pWinData->mpExtOldAttrAry = NULL;
    }
    if ( pTextAttr )
    {
        pWinData->mpExtOldAttrAry = new USHORT[rText.Len()];
        memcpy( pWinData->mpExtOldAttrAry, pTextAttr, rText.Len()*sizeof( USHORT ) );
    }
    return !ImplCallCommand( pChild, COMMAND_EXTTEXTINPUT, &aData );
}

// -----------------------------------------------------------------------

static long ImplHandleEndExtTextInput( Window* /* pWindow */ )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pChild = pSVData->maWinData.mpExtTextInputWin;
    long        nRet = 0;

    if ( pChild )
    {
        pChild->mbExtTextInput = FALSE;
        pSVData->maWinData.mpExtTextInputWin = NULL;
        ImplWinData* pWinData = pChild->ImplGetWinData();
        if ( pWinData->mpExtOldText )
        {
            delete pWinData->mpExtOldText;
            pWinData->mpExtOldText = NULL;
        }
        if ( pWinData->mpExtOldAttrAry )
        {
            delete [] pWinData->mpExtOldAttrAry;
            pWinData->mpExtOldAttrAry = NULL;
        }
        nRet = !ImplCallCommand( pChild, COMMAND_ENDEXTTEXTINPUT );
    }

    return nRet;
}

// -----------------------------------------------------------------------

static void ImplHandleExtTextInputPos( Window* pWindow,
                                       Rectangle& rRect, long& rInputWidth,
                                       bool * pVertical )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pChild = pSVData->maWinData.mpExtTextInputWin;

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
        ImplCallCommand( pChild, COMMAND_CURSORPOS );
        const Rectangle* pRect = pChild->GetCursorRect();
        if ( pRect )
            rRect = pChild->ImplLogicToDevicePixel( *pRect );
        else
        {
            Cursor* pCursor = pChild->GetCursor();
            if ( pCursor )
            {
                Point aPos = pChild->ImplLogicToDevicePixel( pCursor->GetPos() );
                Size aSize = pChild->LogicToPixel( pCursor->GetSize() );
                if ( !aSize.Width() )
                    aSize.Width() = pChild->GetSettings().GetStyleSettings().GetCursorSize();
                rRect = Rectangle( aPos, aSize );
            }
            else
                rRect = Rectangle( Point( pChild->mnOutOffX, pChild->mnOutOffY ), Size() );
        }
        rInputWidth = pChild->ImplLogicWidthToDevicePixel( pChild->GetCursorExtTextInputWidth() );
        if ( !rInputWidth )
            rInputWidth = rRect.GetWidth();
    }
    if (pVertical != 0)
        *pVertical
            = pChild != 0 && pChild->GetInputContext().GetFont().IsVertical();
}

// -----------------------------------------------------------------------

static long ImplHandleInputContextChange( Window* pWindow, LanguageType eNewLang )
{
    Window* pChild = ImplGetKeyInputWindow( pWindow );
    CommandInputContextData aData( eNewLang );
    return !ImplCallCommand( pChild, COMMAND_INPUTCONTEXTCHANGE, &aData );
}

// -----------------------------------------------------------------------

static BOOL ImplCallWheelCommand( Window* pWindow, const Point& rPos,
                                  const CommandWheelData* pWheelData )
{
    Point               aCmdMousePos = pWindow->ImplFrameToOutput( rPos );
    CommandEvent        aCEvt( aCmdMousePos, COMMAND_WHEEL, TRUE, pWheelData );
    NotifyEvent         aNCmdEvt( EVENT_COMMAND, pWindow, &aCEvt );
    ImplDelData         aDelData;
    BOOL                bPreNotify;
    pWindow->ImplAddDel( &aDelData );
    if ( !ImplCallPreNotify( aNCmdEvt ) && !aDelData.IsDelete() )
    {
        bPreNotify = FALSE;

        pWindow->mbCommand = FALSE;
        pWindow->Command( aCEvt );
    }
    else
        bPreNotify = TRUE;
    if ( aDelData.IsDelete() )
        return FALSE;
    pWindow->ImplRemoveDel( &aDelData );
    if ( !bPreNotify && pWindow->mbCommand )
        return TRUE;
    return FALSE;
}

// -----------------------------------------------------------------------

long ImplHandleWheelEvent( Window* pWindow,
                           long nX, long nY, ULONG nMsgTime,
                           long nDelta, long nNotchDelta,
                           ULONG nScrollLines, USHORT nCode, BOOL bHorz )
{
    ImplSVData* pSVData = ImplGetSVData();
    USHORT      nMode;

    if ( pSVData->maHelpData.mpHelpWin )
        ImplDestroyHelpWindow();

    if ( nCode & KEY_MOD1 )
        nMode = COMMAND_WHEEL_ZOOM;
    else if ( nCode & KEY_SHIFT )
        nMode = COMMAND_WHEEL_DATAZOOM;
    else
        nMode = COMMAND_WHEEL_SCROLL;

    Point               aMousePos( nX, nY );
    CommandWheelData    aWheelData( nDelta, nNotchDelta, nScrollLines, nMode, nCode, bHorz );
    BOOL                bRet = TRUE;

    // first check any floating window ( eg. drop down listboxes)
    Window *pMouseWindow = NULL;
    if ( pSVData->maWinData.mpFirstFloat && !pSVData->maWinData.mpCaptureWin &&
         !pSVData->maWinData.mpFirstFloat->ImplIsFloatPopupModeWindow( pWindow ) )
    {
        USHORT nHitTest = IMPL_FLOATWIN_HITTEST_OUTSIDE;
        pMouseWindow = pSVData->maWinData.mpFirstFloat->ImplFloatHitTest( pWindow, aMousePos, nHitTest );
    }
    // then try the window directly beneath the mouse
    if( !pMouseWindow )
        pMouseWindow = pWindow->ImplFindWindow( aMousePos );
    else
        // transform coordinates to float window frame coordinates
        pMouseWindow = pMouseWindow->ImplFindWindow(
                 pMouseWindow->OutputToScreenPixel(
                  pMouseWindow->AbsoluteScreenToOutputPixel(
                   pWindow->OutputToAbsoluteScreenPixel(
                    pWindow->ScreenToOutputPixel( aMousePos ) ) ) ) );

    if ( pMouseWindow &&
         pMouseWindow->IsEnabled() && pMouseWindow->IsInputEnabled() )
    {
        // transform coordinates to float window frame coordinates
        Point aRelMousePos( pMouseWindow->OutputToScreenPixel(
                             pMouseWindow->AbsoluteScreenToOutputPixel(
                              pWindow->OutputToAbsoluteScreenPixel(
                               pWindow->ScreenToOutputPixel( aMousePos ) ) ) ) );
        bRet = ImplCallWheelCommand( pMouseWindow, aRelMousePos, &aWheelData );
    }

    // if the commad was not handled try the focus window
    if ( bRet )
    {
        Window* pFocusWindow = pWindow->mpFrameData->mpFocusWin;
        if ( pFocusWindow && (pFocusWindow != pMouseWindow) &&
             (pFocusWindow == pSVData->maWinData.mpFocusWin) )
        {
            // no wheel-messages to disabled windows
            if ( pFocusWindow->IsEnabled() && pFocusWindow->IsInputEnabled() )
            {
                // transform coordinates to focus window frame coordinates
                Point aRelMousePos( pFocusWindow->OutputToScreenPixel(
                                     pFocusWindow->AbsoluteScreenToOutputPixel(
                                      pWindow->OutputToAbsoluteScreenPixel(
                                       pWindow->ScreenToOutputPixel( aMousePos ) ) ) ) );
                bRet = ImplCallWheelCommand( pFocusWindow, aRelMousePos, &aWheelData );
            }
        }
    }

    return !bRet;
}

// -----------------------------------------------------------------------
#define IMPL_PAINT_CHECKRTL         ((USHORT)0x0020)

static void ImplHandlePaint( Window* pWindow, const Rectangle& rBoundRect )
{
    // give up background save when sytem paints arrive
    Window* pSaveBackWin = pWindow->mpFrameData->mpFirstBackWin;
    while ( pSaveBackWin )
    {
        Window* pNext = pSaveBackWin->mpOverlapData->mpNextBackWin;
        Rectangle aRect( Point( pSaveBackWin->mnOutOffX, pSaveBackWin->mnOutOffY ),
                         Size( pSaveBackWin->mnOutWidth, pSaveBackWin->mnOutHeight ) );
        if ( aRect.IsOver( rBoundRect ) )
            pSaveBackWin->ImplDeleteOverlapBackground();
        pSaveBackWin = pNext;
    }

    // system paint events must be checked for re-mirroring
    pWindow->mnPaintFlags |= IMPL_PAINT_CHECKRTL;

    // trigger paint for all windows that live in the new paint region
    Region aRegion( rBoundRect );
    pWindow->ImplInvalidateOverlapFrameRegion( aRegion );
}

// -----------------------------------------------------------------------

static void KillOwnPopups( Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window *pParent = pWindow->mpFrameWindow;
    if( pParent->GetParent() )
        pParent = pParent->GetParent();
    Window *pChild = pSVData->maWinData.mpFirstFloat;
    if ( pChild && pParent->ImplIsWindowOrChild( pChild, TRUE ) )
    {
        if ( !(pSVData->maWinData.mpFirstFloat->GetPopupModeFlags() & FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE) )
            pSVData->maWinData.mpFirstFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
    }
}

// -----------------------------------------------------------------------

void ImplHandleResize( Window* pWindow, long nNewWidth, long nNewHeight )
{
    if( pWindow->GetStyle() & (WB_MOVEABLE|WB_SIZEABLE) )
    {
        KillOwnPopups( pWindow );
        if( pWindow->ImplGetWindow() != ImplGetSVData()->maHelpData.mpHelpWin )
            ImplDestroyHelpWindow();
    }

    if ( (nNewWidth > 0) && (nNewHeight > 0) ||
         pWindow->ImplGetWindow()->mbAllResize )
    {
        if ( (nNewWidth != pWindow->mnOutWidth) || (nNewHeight != pWindow->mnOutHeight) )
        {
            pWindow->mnOutWidth  = nNewWidth;
            pWindow->mnOutHeight = nNewHeight;
            pWindow->mbWaitSystemResize = FALSE;
            if ( pWindow->IsReallyVisible() )
                pWindow->ImplSetClipFlag();
            if ( pWindow->IsVisible() || pWindow->ImplGetWindow()->mbAllResize ||
                ( pWindow->mbFrame && pWindow->mpClientWindow ) )   // propagate resize for system border windows
                pWindow->ImplCallResize();                                  // otherwise menues cannot be positioned
            else
                pWindow->mbCallResize = TRUE;
        }
    }

    pWindow->mpFrameData->mbNeedSysWindow = (nNewWidth < IMPL_MIN_NEEDSYSWIN) ||
                                            (nNewHeight < IMPL_MIN_NEEDSYSWIN);
    BOOL bMinimized = (nNewWidth <= 0) || (nNewHeight <= 0);
    if( bMinimized != pWindow->mpFrameData->mbMinimized )
        pWindow->mpFrameWindow->ImplNotifyIconifiedState( bMinimized );
    pWindow->mpFrameData->mbMinimized = bMinimized;
}

// -----------------------------------------------------------------------

void ImplHandleMove( Window* pWindow, long nNewX, long nNewY )
{
    if( pWindow->mbFrame && pWindow->ImplIsFloatingWindow() && pWindow->IsReallyVisible() )
    {
        static_cast<FloatingWindow*>(pWindow)->EndPopupMode( FLOATWIN_POPUPMODEEND_TEAROFF );
        pWindow->ImplCallMove();
    }

    if( pWindow->GetStyle() & (WB_MOVEABLE|WB_SIZEABLE) )
    {
        KillOwnPopups( pWindow );
        if( pWindow->ImplGetWindow() != ImplGetSVData()->maHelpData.mpHelpWin )
            ImplDestroyHelpWindow();
    }

    if ( pWindow->IsVisible() )
        pWindow->ImplCallMove();

    if ( pWindow->mbFrame && pWindow->mpClientWindow )
        pWindow->mpClientWindow->ImplCallMove();    // notify client to update geometry

}

// -----------------------------------------------------------------------

void ImplHandleMoveResize( Window* pWindow, long nNewX, long nNewY, long nNewWidth, long nNewHeight )
{
    ImplHandleMove( pWindow, nNewX, nNewY );
    ImplHandleResize( pWindow, nNewWidth, nNewHeight );
}

// -----------------------------------------------------------------------

static void ImplActivateFloatingWindows( Window* pWindow, BOOL bActive )
{
    // Zuerst alle ueberlappenden Fenster ueberpruefen
    Window* pTempWindow = pWindow->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( !pTempWindow->GetActivateMode() )
        {
            if ( (pTempWindow->GetType() == WINDOW_BORDERWINDOW) &&
                 (pTempWindow->ImplGetWindow()->GetType() == WINDOW_FLOATINGWINDOW) )
                ((ImplBorderWindow*)pTempWindow)->SetDisplayActive( bActive );
        }

        ImplActivateFloatingWindows( pTempWindow, bActive );
        pTempWindow = pTempWindow->mpNext;
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( Window, ImplAsyncFocusHdl, void*, EMPTYARG )
{
    mpFrameData->mnFocusId = 0;

    // Wenn Status erhalten geblieben ist, weil wir den Focus in der
    // zwischenzeit schon wiederbekommen haben, brauchen wir auch
    // nichts machen
    BOOL bHasFocus = mpFrameData->mbHasFocus || mpFrameData->mbSysObjFocus;

    // Dann die zeitverzoegerten Funktionen ausfuehren
    if ( bHasFocus )
    {
        // Alle FloatingFenster deaktiv zeichnen
        if ( mpFrameData->mbStartFocusState != bHasFocus )
            ImplActivateFloatingWindows( this, bHasFocus );

        if ( mpFrameData->mpFocusWin )
        {
            if ( mpFrameData->mpFocusWin->IsEnabled() && mpFrameData->mpFocusWin->IsInputEnabled() )
                mpFrameData->mpFocusWin->GrabFocus();
            else if( mpFrameData->mpFocusWin->ImplHasDlgCtrl() )
            {
                // #109094# if the focus is restored to a disabled dialog control (was disabled meanwhile)
                // try to move it to the next control
                mpFrameData->mpFocusWin->ImplDlgCtrlNextWindow();
            }
            else
            {
                ImplSVData* pSVData = ImplGetSVData();
                Window*     pTopLevelWindow = mpFrameData->mpFocusWin->ImplGetFirstOverlapWindow();
                if ( !pTopLevelWindow->IsInputEnabled() && pSVData->maWinData.mpLastExecuteDlg )
                    pSVData->maWinData.mpLastExecuteDlg->ToTop( TOTOP_RESTOREWHENMIN | TOTOP_GRABFOCUSONLY);
                else
                    pTopLevelWindow->GrabFocus();
            }
        }
        else
            GrabFocus();
    }
    else
    {
        Window* pFocusWin = mpFrameData->mpFocusWin;
        if ( pFocusWin )
        {
            ImplSVData* pSVData = ImplGetSVData();

            if ( pSVData->maWinData.mpFocusWin == pFocusWin )
            {
                // FocusWindow umsetzen
                Window* pOverlapWindow = pFocusWin->ImplGetFirstOverlapWindow();
                pOverlapWindow->mpLastFocusWindow = pFocusWin;
                pSVData->maWinData.mpFocusWin = NULL;

                if ( pFocusWin->mpCursor )
                    pFocusWin->mpCursor->ImplHide();

                // Deaktivate rufen
                Window* pOldFocusWindow = pFocusWin;
                if ( pOldFocusWindow )
                {
                    Window* pOldOverlapWindow = pOldFocusWindow->ImplGetFirstOverlapWindow();
                    Window* pOldRealWindow = pOldOverlapWindow->ImplGetWindow();

                    pOldOverlapWindow->mbActive = FALSE;
                    pOldOverlapWindow->Deactivate();
                    if ( pOldRealWindow != pOldOverlapWindow )
                    {
                        pOldRealWindow->mbActive = FALSE;
                        pOldRealWindow->Deactivate();
                    }
                }

                // TrackingMode is ended in ImplHandleLoseFocus
// To avoid problems with the Unix IME
//                pFocusWin->EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );

                // XXX #102010# hack for accessibility: do not close the menu,
                // even after focus lost
                static const char* pEnv = getenv("SAL_FLOATWIN_NOAPPFOCUSCLOSE");
                if( !(pEnv && *pEnv) )
                {
                    NotifyEvent aNEvt( EVENT_LOSEFOCUS, pFocusWin );
                    if ( !ImplCallPreNotify( aNEvt ) )
                        pFocusWin->LoseFocus();
                    pFocusWin->ImplCallDeactivateListeners( NULL );
                    GetpApp()->FocusChanged();
                }
                // XXX
            }
        }

        // Alle FloatingFenster deaktiv zeichnen
        if ( mpFrameData->mbStartFocusState != bHasFocus )
            ImplActivateFloatingWindows( this, bHasFocus );
    }

    return 0;
}

// -----------------------------------------------------------------------

static void ImplHandleGetFocus( Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();

    pWindow->mpFrameData->mbHasFocus = TRUE;

    // Focus-Events zeitverzoegert ausfuehren, damit bei SystemChildFenstern
    // nicht alles flackert, wenn diese den Focus bekommen
    if ( !pWindow->mpFrameData->mnFocusId )
    {
        pWindow->mpFrameData->mbStartFocusState = !pWindow->mpFrameData->mbHasFocus;
        Application::PostUserEvent( pWindow->mpFrameData->mnFocusId, LINK( pWindow, Window, ImplAsyncFocusHdl ) );
        Window* pFocusWin = pWindow->mpFrameData->mpFocusWin;
        if ( pFocusWin && pFocusWin->mpCursor )
            pFocusWin->mpCursor->ImplShow();
    }
}

// -----------------------------------------------------------------------

static void ImplHandleLoseFocus( Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();

    // Wenn Frame den Focus verliert, brechen wir auch ein AutoScroll ab
    if ( pSVData->maWinData.mpAutoScrollWin )
        pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();

    // Wenn Frame den Focus verliert, brechen wir auch ein Tracking ab
    if ( pSVData->maWinData.mpTrackWin )
    {
        if ( pSVData->maWinData.mpTrackWin->mpFrameWindow == pWindow )
            pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    }

    // handle FloatingMode
    // hier beenden wir immer den PopupModus, auch dann, wenn NOFOCUSCLOSE
    // gesetzt ist, damit wir nicht beim Wechsel noch Fenster stehen lassen
    if ( pSVData->maWinData.mpFirstFloat )
    {
        if ( !(pSVData->maWinData.mpFirstFloat->GetPopupModeFlags() & FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE) )
            pSVData->maWinData.mpFirstFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
    }

    pWindow->mpFrameData->mbHasFocus = FALSE;

    // Focus-Events zeitverzoegert ausfuehren, damit bei SystemChildFenstern
    // nicht alles flackert, wenn diese den Focus bekommen
    if ( !pWindow->mpFrameData->mnFocusId )
    {
        pWindow->mpFrameData->mbStartFocusState = !pWindow->mpFrameData->mbHasFocus;
        Application::PostUserEvent( pWindow->mpFrameData->mnFocusId, LINK( pWindow, Window, ImplAsyncFocusHdl ) );
    }

    Window* pFocusWin = pWindow->mpFrameData->mpFocusWin;
    if ( pFocusWin && pFocusWin->mpCursor )
        pFocusWin->mpCursor->ImplHide();
}

// -----------------------------------------------------------------------

void ImplHandleClose( Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();

    bool bWasPopup = false;
    if( pWindow->ImplIsFloatingWindow() &&
        static_cast<FloatingWindow*>(pWindow)->mbInPopupMode )
    {
        bWasPopup = true;
    }

    // on Close stop all floating modes and end popups
    if ( pSVData->maWinData.mpFirstFloat )
    {
        FloatingWindow* pLastLevelFloat;
        pLastLevelFloat = pSVData->maWinData.mpFirstFloat->ImplFindLastLevelFloat();
        pLastLevelFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
    }
    if ( pSVData->maHelpData.mbExtHelpMode )
        Help::EndExtHelp();
    if ( pSVData->maHelpData.mpHelpWin )
        ImplDestroyHelpWindow();
    // AutoScrollMode
    if ( pSVData->maWinData.mpAutoScrollWin )
        pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();

    if ( pSVData->maWinData.mpTrackWin )
        pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL | ENDTRACK_KEY );

    if( ! bWasPopup )
    {
        Window *pWin = pWindow->ImplGetWindow();
        // check whether close is allowed
        if ( !pWin->IsEnabled() || !pWin->IsInputEnabled() )
            Sound::Beep( SOUND_DISABLE, pWin );
        // dispatch to correct window type
        else if( pWin->IsSystemWindow() )
            ((SystemWindow*)pWin)->Close();
        else if( pWin->ImplIsDockingWindow() )
            ((DockingWindow*)pWin)->Close();
    }
}

// -----------------------------------------------------------------------

static void ImplHandleUserEvent( ImplSVEvent* pSVEvent )
{
    if ( pSVEvent )
    {
        if ( pSVEvent->mbCall && !pSVEvent->maDelData.IsDelete() )
        {
            if ( pSVEvent->mpWindow )
            {
                pSVEvent->mpWindow->ImplRemoveDel( &(pSVEvent->maDelData) );
                if ( pSVEvent->mpLink )
                    pSVEvent->mpLink->Call( pSVEvent->mpData );
                else
                    pSVEvent->mpWindow->UserEvent( pSVEvent->mnEvent, pSVEvent->mpData );
            }
            else
            {
                if ( pSVEvent->mpLink )
                    pSVEvent->mpLink->Call( pSVEvent->mpData );
                else
                    GetpApp()->UserEvent( pSVEvent->mnEvent, pSVEvent->mpData );
            }
        }

        delete pSVEvent->mpLink;
        delete pSVEvent;
    }
}

// =======================================================================

#ifndef REMOTE_APPSERVER

static USHORT ImplGetMouseMoveMode( SalMouseEvent* pEvent )
{
    USHORT nMode = 0;
    if ( !pEvent->mnCode )
        nMode |= MOUSE_SIMPLEMOVE;
    if ( (pEvent->mnCode & MOUSE_LEFT) && !(pEvent->mnCode & KEY_MOD1) )
        nMode |= MOUSE_DRAGMOVE;
    if ( (pEvent->mnCode & MOUSE_LEFT) && (pEvent->mnCode & KEY_MOD1) )
        nMode |= MOUSE_DRAGCOPY;
    return nMode;
}

// -----------------------------------------------------------------------

static USHORT ImplGetMouseButtonMode( SalMouseEvent* pEvent )
{
    USHORT nMode = 0;
    if ( pEvent->mnButton == MOUSE_LEFT )
        nMode |= MOUSE_SIMPLECLICK;
    if ( (pEvent->mnButton == MOUSE_LEFT) && !(pEvent->mnCode & (MOUSE_MIDDLE | MOUSE_RIGHT)) )
        nMode |= MOUSE_SELECT;
    if ( (pEvent->mnButton == MOUSE_LEFT) && (pEvent->mnCode & KEY_MOD1) &&
         !(pEvent->mnCode & (MOUSE_MIDDLE | MOUSE_RIGHT | KEY_SHIFT)) )
        nMode |= MOUSE_MULTISELECT;
    if ( (pEvent->mnButton == MOUSE_LEFT) && (pEvent->mnCode & KEY_SHIFT) &&
         !(pEvent->mnCode & (MOUSE_MIDDLE | MOUSE_RIGHT | KEY_MOD1)) )
        nMode |= MOUSE_RANGESELECT;
    return nMode;
}

// -----------------------------------------------------------------------

inline long ImplHandleSalMouseLeave( Window* pWindow, SalMouseEvent* pEvent )
{
    return ImplHandleMouseEvent( pWindow, EVENT_MOUSEMOVE, TRUE,
                                 pEvent->mnX, pEvent->mnY,
                                 pEvent->mnTime, pEvent->mnCode,
                                 ImplGetMouseMoveMode( pEvent ) );
}

// -----------------------------------------------------------------------

inline long ImplHandleSalMouseMove( Window* pWindow, SalMouseEvent* pEvent )
{
    return ImplHandleMouseEvent( pWindow, EVENT_MOUSEMOVE, FALSE,
                                 pEvent->mnX, pEvent->mnY,
                                 pEvent->mnTime, pEvent->mnCode,
                                 ImplGetMouseMoveMode( pEvent ) );
}

// -----------------------------------------------------------------------

inline long ImplHandleSalMouseButtonDown( Window* pWindow, SalMouseEvent* pEvent )
{
    return ImplHandleMouseEvent( pWindow, EVENT_MOUSEBUTTONDOWN, FALSE,
                                 pEvent->mnX, pEvent->mnY,
                                 pEvent->mnTime,
                                 pEvent->mnButton | (pEvent->mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)),
                                 ImplGetMouseButtonMode( pEvent ) );
}

// -----------------------------------------------------------------------

inline long ImplHandleSalMouseButtonUp( Window* pWindow, SalMouseEvent* pEvent )
{
    return ImplHandleMouseEvent( pWindow, EVENT_MOUSEBUTTONUP, FALSE,
                                 pEvent->mnX, pEvent->mnY,
                                 pEvent->mnTime,
                                 pEvent->mnButton | (pEvent->mnCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2)),
                                 ImplGetMouseButtonMode( pEvent ) );
}

// -----------------------------------------------------------------------

long ImplHandleSalMouseActivate( Window* pWindow, SalMouseActivateEvent* pEvent )
{
    return FALSE;
}

// -----------------------------------------------------------------------

static long ImplHandleMenuEvent( Window* pWindow, SalMenuEvent* pEvent, USHORT nEvent )
{
    // Find SystemWindow and its Menubar and let it dispatch the command
    long nRet = 0;
    Window *pWin = pWindow->mpFirstChild;
    while ( pWin )
    {
        if ( pWin->mbSysWin )
            break;
        pWin = pWin->mpNext;
    }
    if( pWin )
    {
        MenuBar *pMenuBar = ((SystemWindow*) pWin)->GetMenuBar();
        if( pMenuBar )
        {
            switch( nEvent )
            {
                case SALEVENT_MENUACTIVATE:
                    nRet = pMenuBar->HandleMenuActivateEvent( (Menu*) pEvent->mpMenu ) ? 1 : 0;
                    break;
                case SALEVENT_MENUDEACTIVATE:
                    nRet = pMenuBar->HandleMenuDeActivateEvent( (Menu*) pEvent->mpMenu ) ? 1 : 0;
                    break;
                case SALEVENT_MENUHIGHLIGHT:
                    nRet = pMenuBar->HandleMenuHighlightEvent( (Menu*) pEvent->mpMenu, pEvent->mnId ) ? 1 : 0;
                    break;
                case SALEVENT_MENUCOMMAND:
                    nRet = pMenuBar->HandleMenuCommandEvent( (Menu*) pEvent->mpMenu, pEvent->mnId ) ? 1 : 0;
                    break;
                default:
                    break;
            }
        }
    }
    return nRet;
}

// -----------------------------------------------------------------------

static void ImplHandleSalKeyMod( Window* pWindow, SalKeyModEvent* pEvent )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window* pTrackWin = pSVData->maWinData.mpTrackWin;
    if ( pTrackWin )
        pWindow = pTrackWin;
    USHORT nOldCode = pWindow->mpFrameData->mnMouseCode & (KEY_SHIFT | KEY_MOD1 | KEY_MOD2);
    USHORT nNewCode = pEvent->mnCode;
    if ( nOldCode != nNewCode )
    {
        nNewCode |= pWindow->mpFrameData->mnMouseCode & ~(KEY_SHIFT | KEY_MOD1 | KEY_MOD2);
        pWindow->mpFrameWindow->ImplCallMouseMove( nNewCode, TRUE );
    }

    // #105224# send commandevent to allow special treatment of Ctrl-LeftShift/Ctrl-RightShift etc.

    // find window
    Window* pChild = ImplGetKeyInputWindow( pWindow );
    if ( !pChild )
        return;

    // send modkey events only if useful data is available
    if( pEvent->mnModKeyCode != 0 )
    {
        CommandModKeyData data( pEvent->mnModKeyCode );
        ImplCallCommand( pChild, COMMAND_MODKEYCHANGE, &data );
    }
}

// -----------------------------------------------------------------------

static void ImplHandleSalSettings( Window* pWindow, USHORT nEvent )
{
    // Application Notification werden nur fuer das erste Window ausgeloest
    ImplSVData* pSVData = ImplGetSVData();
    if ( pWindow != pSVData->maWinData.mpFirstFrame )
        return;

    Application* pApp = GetpApp();
    if ( !pApp )
        return;

    if ( nEvent == SALEVENT_SETTINGSCHANGED )
    {
        AllSettings aSettings = pApp->GetSettings();
        pApp->MergeSystemSettings( aSettings );
        pApp->SystemSettingsChanging( aSettings, pWindow );
        pApp->SetSettings( aSettings );
    }
    else
    {
        USHORT nType;
        switch ( nEvent )
        {
            case SALEVENT_VOLUMECHANGED:
                nType = 0;
                break;
            case SALEVENT_PRINTERCHANGED:
                ImplDeletePrnQueueList();
                nType = DATACHANGED_PRINTER;
                break;
            case SALEVENT_DISPLAYCHANGED:
                nType = DATACHANGED_DISPLAY;
                break;
            case SALEVENT_FONTCHANGED:
                OutputDevice::ImplUpdateAllFontData( TRUE );
                nType = DATACHANGED_FONTS;
                break;
            case SALEVENT_DATETIMECHANGED:
                nType = DATACHANGED_DATETIME;
                break;
            case SALEVENT_KEYBOARDCHANGED:
                nType = 0;
                break;
            default:
                nType = 0;
                break;
        }

        if ( nType )
        {
            DataChangedEvent aDCEvt( nType );
            pApp->DataChanged( aDCEvt );
            pApp->NotifyAllWindows( aDCEvt );
        }
    }
}

// -----------------------------------------------------------------------

static void ImplHandleSalExtTextInputPos( Window* pWindow, SalExtTextInputPosEvent* pEvt )
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

// -----------------------------------------------------------------------

long ImplWindowFrameProc( void* pInst, SalFrame* pFrame,
                          USHORT nEvent, const void* pEvent )
{
    DBG_TESTSOLARMUTEX();

    long nRet = 0;

    switch ( nEvent )
    {
        case SALEVENT_MOUSEMOVE:
            nRet = ImplHandleSalMouseMove( (Window*)pInst, (SalMouseEvent*)pEvent );
            break;
        case SALEVENT_MOUSELEAVE:
            nRet = ImplHandleSalMouseLeave( (Window*)pInst, (SalMouseEvent*)pEvent );
            break;
        case SALEVENT_MOUSEBUTTONDOWN:
            nRet = ImplHandleSalMouseButtonDown( (Window*)pInst, (SalMouseEvent*)pEvent );
            break;
        case SALEVENT_MOUSEBUTTONUP:
            nRet = ImplHandleSalMouseButtonUp( (Window*)pInst, (SalMouseEvent*)pEvent );
            break;
        case SALEVENT_MOUSEACTIVATE:
            nRet = ImplHandleSalMouseActivate( (Window*)pInst, (SalMouseActivateEvent*)pEvent );
            break;

        case SALEVENT_KEYINPUT:
        case SALEVENT_EXTERNALKEYINPUT:
            {
            SalKeyEvent* pKeyEvt = (SalKeyEvent*)pEvent;
            nRet = ImplHandleKey( (Window*)pInst, EVENT_KEYINPUT,
                pKeyEvt->mnCode, pKeyEvt->mnCharCode, pKeyEvt->mnRepeat, SALEVENT_KEYINPUT ? TRUE : FALSE );
            }
            break;
        case SALEVENT_KEYUP:
        case SALEVENT_EXTERNALKEYUP:
            {
            SalKeyEvent* pKeyEvt = (SalKeyEvent*)pEvent;
            nRet = ImplHandleKey( (Window*)pInst, EVENT_KEYUP,
                pKeyEvt->mnCode, pKeyEvt->mnCharCode, pKeyEvt->mnRepeat, SALEVENT_KEYUP ? TRUE : FALSE );
            }
            break;
        case SALEVENT_KEYMODCHANGE:
            ImplHandleSalKeyMod( (Window*)pInst, (SalKeyModEvent*)pEvent );
            break;

        case SALEVENT_MENUACTIVATE:
        case SALEVENT_MENUDEACTIVATE:
        case SALEVENT_MENUHIGHLIGHT:
        case SALEVENT_MENUCOMMAND:
            nRet = ImplHandleMenuEvent( (Window*)pInst, (SalMenuEvent*)pEvent, nEvent );
            break;

        case SALEVENT_WHEELMOUSE:
            {
            ImplSVData* pSVData = ImplGetSVData();

            if ( pSVData->maWinData.mpAutoScrollWin )
                pSVData->maWinData.mpAutoScrollWin->EndAutoScroll();

            SalWheelMouseEvent* pWheelEvt = (SalWheelMouseEvent*)pEvent;
            nRet = ImplHandleWheelEvent( (Window*)pInst,
                                         pWheelEvt->mnX, pWheelEvt->mnY,
                                         pWheelEvt->mnTime,
                                         pWheelEvt->mnDelta,
                                         pWheelEvt->mnNotchDelta,
                                         pWheelEvt->mnScrollLines,
                                         pWheelEvt->mnCode, pWheelEvt->mbHorz );
            }
            break;

        case SALEVENT_PAINT:
            {
            SalPaintEvent* pPaintEvt = (SalPaintEvent*)pEvent;
            Rectangle aBoundRect( Point( pPaintEvt->mnBoundX, pPaintEvt->mnBoundY ),
                                  Size( pPaintEvt->mnBoundWidth, pPaintEvt->mnBoundHeight ) );
            ImplHandlePaint( (Window*)pInst, aBoundRect );
            }
            break;

        case SALEVENT_MOVE:
            {
            SalFrameGeometry g = ((Window*)pInst)->mpFrame->GetGeometry();
            ImplHandleMove( (Window*)pInst, g.nX, g.nY );
            }
            break;

        case SALEVENT_RESIZE:
            {
            long nNewWidth;
            long nNewHeight;
            ((Window*)pInst)->mpFrame->GetClientSize( nNewWidth, nNewHeight );
            ImplHandleResize( (Window*)pInst, nNewWidth, nNewHeight );
            }
            break;

        case SALEVENT_MOVERESIZE:
            {
            SalFrameGeometry g = ((Window*)pInst)->mpFrame->GetGeometry();
            ImplHandleMoveResize( (Window*)pInst, g.nX, g.nY, g.nWidth, g.nHeight );
            }
            break;

        case SALEVENT_CLOSEPOPUPS:
            {
            KillOwnPopups( (Window*)pInst );
            }
            break;

        case SALEVENT_GETFOCUS:
            ImplHandleGetFocus( (Window*)pInst );
            break;
        case SALEVENT_LOSEFOCUS:
            ImplHandleLoseFocus( (Window*)pInst );
            break;

        case SALEVENT_CLOSE:
            ImplHandleClose( (Window*)pInst );
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
                        return FALSE;
                    }
                    else
                    {
                        bInQueryExit = false;
                        return TRUE;
                    }
                }
                else
                    return FALSE;
            }
            break;

        case SALEVENT_SETTINGSCHANGED:
        case SALEVENT_VOLUMECHANGED:
        case SALEVENT_PRINTERCHANGED:
        case SALEVENT_DISPLAYCHANGED:
        case SALEVENT_FONTCHANGED:
        case SALEVENT_DATETIMECHANGED:
        case SALEVENT_KEYBOARDCHANGED:
            ImplHandleSalSettings( (Window*)pInst, nEvent );
            break;

        case SALEVENT_USEREVENT:
            ImplHandleUserEvent( (ImplSVEvent*)pEvent );
            break;

        case SALEVENT_EXTTEXTINPUT:
            {
            SalExtTextInputEvent* pEvt = (SalExtTextInputEvent*)pEvent;
            nRet = ImplHandleExtTextInput( (Window*)pInst, pEvt->mnTime,
                                           pEvt->maText, pEvt->mpTextAttr,
                                           pEvt->mnCursorPos, pEvt->mnCursorFlags );
            }
            break;
        case SALEVENT_ENDEXTTEXTINPUT:
            nRet = ImplHandleEndExtTextInput( (Window*)pInst );
            break;
        case SALEVENT_EXTTEXTINPUTPOS:
            ImplHandleSalExtTextInputPos( (Window*)pInst, (SalExtTextInputPosEvent*)pEvent );
            break;
        case SALEVENT_INPUTCONTEXTCHANGE:
            nRet = ImplHandleInputContextChange( (Window*)pInst, ((SalInputContextChangeEvent*)pEvent)->meLanguage );
            break;

#ifdef DBG_UTIL
        default:
            DBG_ERROR1( "ImplWindowFrameProc(): unknown event (%lu)", (ULONG)nEvent );
            break;
#endif
    }

    return nRet;
}

#else   // => REMOTE_APPSERVER

void ImplUpdateCursorRect( Window *pWindow )
{
    RmFrameWindow *pFrame;
    if( pWindow && ( ( pFrame = pWindow->ImplGetFrame() ) != NULL ) )
    {
        if( pFrame->IsInEvtHandler() )
            return;     // we'll update later
        Rectangle rRect;
        long rWidth;
        pFrame->IsInEvtHandler( true ); // avoid recursion
        ImplHandleExtTextInputPos( pWindow, rRect, rWidth, 0 );
        pFrame->IsInEvtHandler( false );
        pFrame->SetCursorRect( &rRect, rWidth );
    }
}

void ImplHandleGeometryChange( Window *pWindow, Rectangle *pRect )
{
    if( pWindow && pWindow->ImplGetFrame() && pRect )
    {
        pWindow->ImplGetFrame()->maGeometry.nX = pRect->nLeft;
        pWindow->ImplGetFrame()->maGeometry.nY = pRect->nTop;
        pWindow->ImplGetFrame()->maGeometry.nWidth = pRect->nRight - pRect->nLeft + 1;
        pWindow->ImplGetFrame()->maGeometry.nHeight = pRect->nBottom - pRect->nTop + 1;
        ImplHandleMoveResize( pWindow, pRect->nLeft, pRect->nTop,
            pWindow->ImplGetFrame()->maGeometry.nWidth,
            pWindow->ImplGetFrame()->maGeometry.nHeight );
    }
}

void ImplHandleDecorationChange( Window *pWindow, Rectangle *pRect )
{
    if( pWindow && pWindow->ImplGetFrame() && pRect )
    {
        pWindow->ImplGetFrame()->maGeometry.nLeftDecoration = pRect->nLeft;
        pWindow->ImplGetFrame()->maGeometry.nTopDecoration = pRect->nTop;
        pWindow->ImplGetFrame()->maGeometry.nRightDecoration = pRect->nRight;
        pWindow->ImplGetFrame()->maGeometry.nBottomDecoration = pRect->nBottom;
    }
}

void ImplRemoteWindowFrameProc( ExtRmEvent* pEvent )
{
    DBG_TESTSOLARMUTEX();

    ImplDelData aDelData;
    Window *pWindow = pEvent->GetWindow();
    if( pWindow && pWindow->ImplGetFrame() )
    {
        pWindow->ImplAddDel( &aDelData );
        // disable updates like remote CursorRect
        pWindow->ImplGetFrame()->IsInEvtHandler( true );
    }

    ULONG nId = pEvent->GetId();
    switch ( nId )
    {
        case RMEVENT_KEYINPUT:
        {
            RmKeyEventData* pData = (RmKeyEventData*)pEvent->GetData();
            ImplHandleKey( pEvent->GetWindow(), EVENT_KEYINPUT,
                           pData->nKeyCode, pData->nChar, pData->nCount, TRUE );
        }
        break;
        case RMEVENT_KEYUP:
        {
            RmKeyEventData* pData = (RmKeyEventData*)pEvent->GetData();
            ImplHandleKey( pEvent->GetWindow(), EVENT_KEYUP,
                           pData->nKeyCode, pData->nChar, 0, TRUE );
        }
        break;
        case RMEVENT_MOUSEBUTTONDOWN:
        case RMEVENT_MOUSEBUTTONUP:
        case RMEVENT_MOUSEMOVE:
        {
            USHORT nSVEvent;
            if ( nId == RMEVENT_MOUSEBUTTONDOWN )
                nSVEvent = EVENT_MOUSEBUTTONDOWN;
            else if ( nId == RMEVENT_MOUSEBUTTONUP )
                nSVEvent = EVENT_MOUSEBUTTONUP;
            else
                nSVEvent = EVENT_MOUSEMOVE;
            RmMouseEventData* pData = (RmMouseEventData*)pEvent->GetData();
            BOOL bMouseLeave = ( pData->nMode & MOUSE_LEAVEWINDOW ) ? TRUE : FALSE;
            pData->nMode &= ~(MOUSE_ENTERWINDOW|MOUSE_LEAVEWINDOW);

            // Bei MOUSE_MOVE eine Bestaetigung zurueckschicken, damit der
            // RClient solange verzoegert...
            // Vorm ImplHandleMouseEvent, falls dort z.B. ein modaler Dialog
            // aufgemacht wird.
            if ( nId == RMEVENT_MOUSEMOVE )
            {
                DBG_ASSERT( pEvent->GetWindow()->ImplGetFrame(), "RemoteWindowProc: Frame?" );
                if ( pEvent->GetWindow()->ImplGetFrame() )
                    pEvent->GetWindow()->ImplGetFrame()->MouseMoveProcessed();
            }

            ImplHandleMouseEvent( pEvent->GetWindow(), nSVEvent, bMouseLeave,
                           pData->nX, pData->nY, pData->nSysTime,
                           pData->nCode, pData->nMode );

        }
        break;
        case RMEVENT_PAINT:
        {
            Rectangle* pRect = (Rectangle*)pEvent->GetData();
            ImplHandlePaint( pEvent->GetWindow(), *pRect );
        }
        break;
        case RMEVENT_RESIZE:
        {
            Size* pSize = (Size*)pEvent->GetData();
            ImplHandleResize( pEvent->GetWindow(), pSize->Width(), pSize->Height() );
        }
        break;
        case RMEVENT_USEREVENT:
        {
            ImplHandleUserEvent( (ImplSVEvent*)pEvent->GetData() );
        }
        break;
        case RMEVENT_CLOSE:
        {
            ImplHandleClose( pEvent->GetWindow() );
        }
        break;
        case RMEVENT_GETFOCUS:
        {
            ImplHandleGetFocus( pEvent->GetWindow() );
        };
        break;
        case RMEVENT_LOSEFOCUS:
        {
            ImplSVData* pSVData = ImplGetSVData();
            ULONG nOldFlags;
            if ( pSVData->maWinData.mpFirstFloat )
            {
                nOldFlags = pSVData->maWinData.mpFirstFloat->GetPopupModeFlags();
                pSVData->maWinData.mpFirstFloat->SetPopupModeFlags( nOldFlags | FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE );
            }
            ImplHandleLoseFocus( pEvent->GetWindow() );
            if ( pSVData->maWinData.mpFirstFloat )
                pSVData->maWinData.mpFirstFloat->SetPopupModeFlags( nOldFlags );
        };
        break;
        case RMEVENT_MOUSEWHEEL:
        {
            RmMouseWheelEventData* pData = (RmMouseWheelEventData*)pEvent->GetData();
            ImplHandleWheelEvent( pEvent->GetWindow(),
                                  pData->nX,
                                  pData->nY,
                                  pData->nSysTime,
                                  pData->nDelta,
                                  pData->nNotchDelta,
                                  pData->nScrollLines,
                                  pData->nCode,
                                  pData->bHorz );
        };
        break;
        case RMEVENT_STARTEXTTEXTINPUT:
        {
            // ???
        }
        break;
        case RMEVENT_EXTTEXTINPUT:
        {
            RmExtTextInputData* pData = (RmExtTextInputData*)pEvent->GetData();
            ImplHandleExtTextInput( pEvent->GetWindow(), pData->nSysTime,
                                           pData->aText, pData->pTextAttr,
                                           pData->nCursorPos, pData->nCursorFlags );
        }
        break;
        case RMEVENT_ENDEXTTEXTINPUT:
        {
            ImplHandleEndExtTextInput( pEvent->GetWindow() );
        }
        break;
        case RMEVENT_INPUTCONTEXTCHANGE:
        {
            LanguageType *pL = (LanguageType*)pEvent->GetData();
            ImplHandleInputContextChange( pEvent->GetWindow(), *pL );
        }
        break;
        case RMEVENT_GEOMETRYCHANGE:
        {
            Rectangle* pRect = (Rectangle*)pEvent->GetData();
            ImplHandleGeometryChange( pEvent->GetWindow(), pRect );
        }
        break;
        case RMEVENT_DECORATIONCHANGE:
        {
            Rectangle* pRect = (Rectangle*)pEvent->GetData();
            ImplHandleDecorationChange( pEvent->GetWindow(), pRect );
        }
        break;
    }

    if( pWindow && !aDelData.IsDelete() )   // window not deleted ?
    {
        pWindow->ImplRemoveDel( &aDelData );
        // enable updates like remote CursorRect
        pWindow->ImplGetFrame()->IsInEvtHandler( false );
        ImplUpdateCursorRect( pWindow );
    }
}

#endif
