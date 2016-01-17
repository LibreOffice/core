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
#include <rtl/strbuf.hxx>
#include <tools/rc.h>

#include <sal/types.h>
#include <vcl/salgtype.hxx>
#include <vcl/event.hxx>
#include <vcl/help.hxx>
#include <vcl/cursor.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <vcl/syschild.hxx>
#include <vcl/dockwin.hxx>
#include <vcl/wall.hxx>
#include <vcl/fixed.hxx>
#include <vcl/gradient.hxx>
#include <vcl/button.hxx>
#include <vcl/taskpanelist.hxx>
#include <vcl/dialog.hxx>
#include <vcl/unowrap.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/lazydelete.hxx>
#include <vcl/virdev.hxx>
#include <vcl/settings.hxx>
#include <vcl/sysdata.hxx>

#include <salframe.hxx>
#include <salobj.hxx>
#include <salinst.hxx>
#include <salgdi.hxx>
#include <svdata.hxx>
#include <dbggui.hxx>
#include <window.h>
#include <toolbox.h>
#include <outdev.h>
#include <brdwin.hxx>
#include <helpwin.hxx>
#include <dndlcon.hxx>

#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XDisplayConnection.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/SystemClipboard.hpp>
#include <com/sun/star/rendering/CanvasFactory.hpp>
#include <com/sun/star/rendering/XSpriteCanvas.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/configmgr.hxx>

#include <cassert>
#include <set>
#include <typeinfo>

#ifdef WNT // see #140456#
#include <win/salframe.h>
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;

using ::com::sun::star::awt::XTopWindow;

namespace vcl {

Window::Window( WindowType nType )
{
    ImplInitWindowData( nType );
}

Window::Window( vcl::Window* pParent, WinBits nStyle )
{

    ImplInitWindowData( WINDOW_WINDOW );
    ImplInit( pParent, nStyle, nullptr );
}

Window::Window( vcl::Window* pParent, const ResId& rResId )
    : mpWindowImpl(nullptr)
{
    rResId.SetRT( RSC_WINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInitWindowData( WINDOW_WINDOW );
    ImplInit( pParent, nStyle, nullptr );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

#if OSL_DEBUG_LEVEL > 0
namespace
{
     OString lcl_createWindowInfo(const vcl::Window* pWindow)
     {
         // skip border windows, they do not carry information that
         // would help with diagnosing the problem
         const vcl::Window* pTempWin( pWindow );
         while ( pTempWin && pTempWin->GetType() == WINDOW_BORDERWINDOW ) {
             pTempWin = pTempWin->GetWindow( GetWindowType::FirstChild );
         }
         // check if pTempWin is not null, otherwise use the
         // original address
         if ( pTempWin ) {
             pWindow = pTempWin;
         }

         OStringBuffer aErrorString;
         aErrorString.append(' ');
         aErrorString.append(typeid( *pWindow ).name());
         aErrorString.append("(");
         aErrorString.append(
                 OUStringToOString(
                     pWindow->GetText(),
                     RTL_TEXTENCODING_UTF8
                     )
                 );
         aErrorString.append(")");
         return aErrorString.makeStringAndClear();
     }
}
#endif

bool Window::IsDisposed() const
{
    return !mpWindowImpl;
}

void Window::dispose()
{
    assert( mpWindowImpl );
    assert( !mpWindowImpl->mbInDispose ); // should only be called from disposeOnce()
    assert( (!mpWindowImpl->mpParent ||
            !mpWindowImpl->mpParent->IsDisposed()) &&
            "vcl::Window child should have its parent disposed first" );

    // remove Key and Mouse events issued by Application::PostKey/MouseEvent
    Application::RemoveMouseAndKeyEvents( this );

    // Dispose of the canvas implementation (which, currently, has an
    // own wrapper window as a child to this one.
    Reference< css::rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );
    if( xCanvas.is() )
    {
        Reference < XComponent > xCanvasComponent( xCanvas, UNO_QUERY );
        if( xCanvasComponent.is() )
            xCanvasComponent->dispose();
    }

    mpWindowImpl->mbInDispose = true;

    CallEventListeners( VCLEVENT_OBJECT_DYING );

    // do not send child events for frames that were registered as native frames
    if( !ImplIsAccessibleNativeFrame() && mpWindowImpl->mbReallyVisible )
        if ( ImplIsAccessibleCandidate() && GetAccessibleParentWindow() )
            GetAccessibleParentWindow()->CallEventListeners( VCLEVENT_WINDOW_CHILDDESTROYED, this );

    // remove associated data structures from dockingmanager
    ImplGetDockingManager()->RemoveWindow( this );

    // remove ownerdraw decorated windows from list in the top-most frame window
    if( (GetStyle() & WB_OWNERDRAWDECORATION) && mpWindowImpl->mbFrame )
    {
        ::std::vector< VclPtr<vcl::Window> >& rList = ImplGetOwnerDrawList();
        auto p = ::std::find( rList.begin(), rList.end(), VclPtr<vcl::Window>(this) );
        if( p != rList.end() )
            rList.erase( p );
    }

    // shutdown drag and drop
    Reference < XComponent > xDnDComponent( mpWindowImpl->mxDNDListenerContainer, UNO_QUERY );

    if( xDnDComponent.is() )
        xDnDComponent->dispose();

    if( mpWindowImpl->mbFrame && mpWindowImpl->mpFrameData )
    {
        try
        {
            // deregister drop target listener
            if( mpWindowImpl->mpFrameData->mxDropTargetListener.is() )
            {
                Reference< XDragGestureRecognizer > xDragGestureRecognizer =
                    Reference< XDragGestureRecognizer > (mpWindowImpl->mpFrameData->mxDragSource, UNO_QUERY);
                if( xDragGestureRecognizer.is() )
                {
                    xDragGestureRecognizer->removeDragGestureListener(
                        Reference< XDragGestureListener > (mpWindowImpl->mpFrameData->mxDropTargetListener, UNO_QUERY));
                }

                mpWindowImpl->mpFrameData->mxDropTarget->removeDropTargetListener( mpWindowImpl->mpFrameData->mxDropTargetListener );
                mpWindowImpl->mpFrameData->mxDropTargetListener.clear();
            }

            // shutdown drag and drop for this frame window
            Reference< XComponent > xComponent( mpWindowImpl->mpFrameData->mxDropTarget, UNO_QUERY );

            // DNDEventDispatcher does not hold a reference of the DropTarget,
            // so it's ok if it does not support XComponent
            if( xComponent.is() )
                xComponent->dispose();
        }
        catch (const Exception&)
        {
            // can be safely ignored here.
        }
    }

    UnoWrapperBase* pWrapper = Application::GetUnoWrapper( false );
    if ( pWrapper )
        pWrapper->WindowDestroyed( this );

    // MT: Must be called after WindowDestroyed!
    // Otherwise, if the accessible is a VCLXWindow, it will try to destroy this window again!
    // But accessibility implementations from applications need this dispose.
    if ( mpWindowImpl->mxAccessible.is() )
    {
        Reference< XComponent> xC( mpWindowImpl->mxAccessible, UNO_QUERY );
        if ( xC.is() )
            xC->dispose();
    }

    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maHelpData.mpHelpWin && (pSVData->maHelpData.mpHelpWin->GetParent() == this) )
        ImplDestroyHelpWindow( true );

    DBG_ASSERT( pSVData->maWinData.mpTrackWin.get() != this,
                "Window::~Window(): Window is in TrackingMode" );
    DBG_ASSERT( pSVData->maWinData.mpCaptureWin.get() != this,
                "Window::~Window(): Window has the mouse captured" );

    // due to old compatibility
    if ( pSVData->maWinData.mpTrackWin == this )
        EndTracking();
    if ( pSVData->maWinData.mpCaptureWin == this )
        ReleaseMouse();
    if ( pSVData->maWinData.mpDefDialogParent == this )
        pSVData->maWinData.mpDefDialogParent = nullptr;

#if OSL_DEBUG_LEVEL > 0
    if ( true ) // always perform these tests in debug builds
    {
        OStringBuffer aErrorStr;
        bool        bError = false;
        vcl::Window*     pTempWin;

        if ( mpWindowImpl->mpFirstChild )
        {
            OStringBuffer aTempStr("Window (");
            aTempStr.append(lcl_createWindowInfo(this));
            aTempStr.append(") with live children destroyed: ");
            pTempWin = mpWindowImpl->mpFirstChild;
            while ( pTempWin )
            {
                aTempStr.append(lcl_createWindowInfo(pTempWin));
                pTempWin = pTempWin->mpWindowImpl->mpNext;
            }
            OSL_FAIL( aTempStr.getStr() );
            Application::Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in debug builds, this must be fixed!
        }

        if (mpWindowImpl->mpFrameData != nullptr)
        {
            pTempWin = mpWindowImpl->mpFrameData->mpFirstOverlap;
            while ( pTempWin )
            {
                if ( ImplIsRealParentPath( pTempWin ) )
                {
                    bError = true;
                    aErrorStr.append(lcl_createWindowInfo(pTempWin));
                }
                pTempWin = pTempWin->mpWindowImpl->mpNextOverlap;
            }
            if ( bError )
            {
                OStringBuffer aTempStr;
                aTempStr.append("Window (");
                aTempStr.append(lcl_createWindowInfo(this));
                aTempStr.append(") with live SystemWindows destroyed: ");
                aTempStr.append(aErrorStr.toString());
                OSL_FAIL(aTempStr.getStr());
                // abort in debug builds, must be fixed!
                Application::Abort(OStringToOUString(
                                     aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));
            }
        }

        bError = false;
        pTempWin = pSVData->maWinData.mpFirstFrame;
        while ( pTempWin )
        {
            if ( ImplIsRealParentPath( pTempWin ) )
            {
                bError = true;
                aErrorStr.append(lcl_createWindowInfo(pTempWin));
            }
            pTempWin = pTempWin->mpWindowImpl->mpFrameData->mpNextFrame;
        }
        if ( bError )
        {
            OStringBuffer aTempStr( "Window (" );
            aTempStr.append(lcl_createWindowInfo(this));
            aTempStr.append(") with live SystemWindows destroyed: ");
            aTempStr.append(aErrorStr.toString());
            OSL_FAIL( aTempStr.getStr() );
            Application::Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in debug builds, this must be fixed!
        }

        if ( mpWindowImpl->mpFirstOverlap )
        {
            OStringBuffer aTempStr("Window (");
            aTempStr.append(lcl_createWindowInfo(this));
            aTempStr.append(") with live SystemWindows destroyed: ");
            pTempWin = mpWindowImpl->mpFirstOverlap;
            while ( pTempWin )
            {
                aTempStr.append(lcl_createWindowInfo(pTempWin));
                pTempWin = pTempWin->mpWindowImpl->mpNext;
            }
            OSL_FAIL( aTempStr.getStr() );
            Application::Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in debug builds, this must be fixed!
        }

        vcl::Window* pMyParent = GetParent();
        SystemWindow* pMySysWin = nullptr;

        while ( pMyParent )
        {
            if ( pMyParent->IsSystemWindow() )
            {
                pMySysWin = dynamic_cast<SystemWindow *>(pMyParent);
            }
            pMyParent = pMyParent->GetParent();
        }
        if ( pMySysWin && pMySysWin->ImplIsInTaskPaneList( this ) )
        {
            OStringBuffer aTempStr("Window (");
            aTempStr.append(lcl_createWindowInfo(this));
            aTempStr.append(") still in TaskPanelList!");
            OSL_FAIL( aTempStr.getStr() );
            Application::Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in debug builds, this must be fixed!
        }
    }
#endif

    if( mpWindowImpl->mbIsInTaskPaneList )
    {
        vcl::Window* pMyParent = GetParent();
        SystemWindow* pMySysWin = nullptr;

        while ( pMyParent )
        {
            if ( pMyParent->IsSystemWindow() )
            {
                pMySysWin = dynamic_cast<SystemWindow *>(pMyParent);
            }
            pMyParent = pMyParent->GetParent();
        }
        if ( pMySysWin && pMySysWin->ImplIsInTaskPaneList( this ) )
        {
            pMySysWin->GetTaskPaneList()->RemoveWindow( this );
        }
        else
        {
            OStringBuffer aTempStr("Window (");
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") not found in TaskPanelList!");
            OSL_FAIL( aTempStr.getStr() );
        }
    }

    // remove from size-group if necessary
    remove_from_all_size_groups();

    // clear mnemonic labels
    std::vector<VclPtr<FixedText> > aMnemonicLabels(list_mnemonic_labels());
    for (auto aI = aMnemonicLabels.begin(); aI != aMnemonicLabels.end(); ++aI)
    {
        remove_mnemonic_label(*aI);
    }

    // hide window in order to trigger the Paint-Handling
    Hide();

    // announce the window is to be destroyed
    {
        NotifyEvent aNEvt( MouseNotifyEvent::DESTROY, this );
        CompatNotify( aNEvt );
    }

    // EndExtTextInputMode
    if ( pSVData->maWinData.mpExtTextInputWin == this )
    {
        EndExtTextInput( EndExtTextInputFlags::Complete );
        if ( pSVData->maWinData.mpExtTextInputWin == this )
            pSVData->maWinData.mpExtTextInputWin = nullptr;
    }

    // check if the focus window is our child
    bool bHasFocussedChild = false;
    if( pSVData->maWinData.mpFocusWin && ImplIsRealParentPath( pSVData->maWinData.mpFocusWin ) )
    {
        // #122232#, this must not happen and is an application bug ! but we try some cleanup to hopefully avoid crashes, see below
        bHasFocussedChild = true;
#if OSL_DEBUG_LEVEL > 0
        OStringBuffer aTempStr("Window (");
        aTempStr.append(OUStringToOString(GetText(),
            RTL_TEXTENCODING_UTF8)).
                append(") with focussed child window destroyed ! THIS WILL LEAD TO CRASHES AND MUST BE FIXED !");
        OSL_FAIL( aTempStr.getStr() );
        Application::Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ));   // abort in debug build version, this must be fixed!
#endif
    }

    // if we get focus pass focus to another window
    vcl::Window* pOverlapWindow = ImplGetFirstOverlapWindow();
    if ( pSVData->maWinData.mpFocusWin == this
        || bHasFocussedChild )  // #122232#, see above, try some cleanup
    {
        if ( mpWindowImpl->mbFrame )
        {
            pSVData->maWinData.mpFocusWin = nullptr;
            pOverlapWindow->mpWindowImpl->mpLastFocusWindow = nullptr;
        }
        else
        {
            vcl::Window* pParent = GetParent();
            vcl::Window* pBorderWindow = mpWindowImpl->mpBorderWindow;
        // when windows overlap, give focus to the parent
        // of the next FrameWindow
            if ( pBorderWindow )
            {
                if ( pBorderWindow->ImplIsOverlapWindow() )
                    pParent = pBorderWindow->mpWindowImpl->mpOverlapWindow;
            }
            else if ( ImplIsOverlapWindow() )
                pParent = mpWindowImpl->mpOverlapWindow;

            if ( pParent && pParent->IsEnabled() && pParent->IsInputEnabled() && ! pParent->IsInModalMode() )
                pParent->GrabFocus();
            else
                mpWindowImpl->mpFrameWindow->GrabFocus();

            // If the focus was set back to 'this' set it to nothing
            if ( pSVData->maWinData.mpFocusWin == this )
            {
                pSVData->maWinData.mpFocusWin = nullptr;
                pOverlapWindow->mpWindowImpl->mpLastFocusWindow = nullptr;
            }
        }
    }

    if ( pOverlapWindow != nullptr &&
         pOverlapWindow->mpWindowImpl->mpLastFocusWindow == this )
        pOverlapWindow->mpWindowImpl->mpLastFocusWindow = nullptr;

    // reset hint for DefModalDialogParent
    if( pSVData->maWinData.mpActiveApplicationFrame == this )
        pSVData->maWinData.mpActiveApplicationFrame = nullptr;

    // reset hint of what was the last wheeled window
    if( pSVData->maWinData.mpLastWheelWindow == this )
        pSVData->maWinData.mpLastWheelWindow = nullptr;

    // reset marked windows
    if ( mpWindowImpl->mpFrameData != nullptr )
    {
        if ( mpWindowImpl->mpFrameData->mpFocusWin == this )
            mpWindowImpl->mpFrameData->mpFocusWin = nullptr;
        if ( mpWindowImpl->mpFrameData->mpMouseMoveWin == this )
            mpWindowImpl->mpFrameData->mpMouseMoveWin = nullptr;
        if ( mpWindowImpl->mpFrameData->mpMouseDownWin == this )
            mpWindowImpl->mpFrameData->mpMouseDownWin = nullptr;
    }

    // reset Deactivate-Window
    if ( pSVData->maWinData.mpLastDeacWin == this )
        pSVData->maWinData.mpLastDeacWin = nullptr;

    if ( mpWindowImpl->mpFrameData )
    {
        if ( mpWindowImpl->mpFrameData->mnFocusId )
            Application::RemoveUserEvent( mpWindowImpl->mpFrameData->mnFocusId );
        mpWindowImpl->mpFrameData->mnFocusId = nullptr;
        if ( mpWindowImpl->mpFrameData->mnMouseMoveId )
            Application::RemoveUserEvent( mpWindowImpl->mpFrameData->mnMouseMoveId );
        mpWindowImpl->mpFrameData->mnMouseMoveId = nullptr;
    }

    // release SalGraphics
    OutputDevice *pOutDev = GetOutDev();
    pOutDev->ReleaseGraphics();

    // notify ImplDelData subscribers of this window about the window deletion
    ImplDelData* pDelData = mpWindowImpl->mpFirstDel;
    while ( pDelData )
    {
        pDelData->mbDel = true;
        pDelData->mpWindow.clear();  // #112873# pDel is not associated with a Window anymore
        pDelData = pDelData->mpNext;
    }

    // remove window from the lists
    ImplRemoveWindow( true );

    // de-register as "top window child" at our parent, if necessary
    if ( mpWindowImpl->mbFrame )
    {
        bool bIsTopWindow = mpWindowImpl->mpWinData && ( mpWindowImpl->mpWinData->mnIsTopWindow == 1 );
        if ( mpWindowImpl->mpRealParent && bIsTopWindow )
        {
            ImplWinData* pParentWinData = mpWindowImpl->mpRealParent->ImplGetWinData();

            auto myPos = ::std::find( pParentWinData->maTopWindowChildren.begin(),
                pParentWinData->maTopWindowChildren.end(), VclPtr<vcl::Window>(this) );
            DBG_ASSERT( myPos != pParentWinData->maTopWindowChildren.end(), "Window::~Window: inconsistency in top window chain!" );
            if ( myPos != pParentWinData->maTopWindowChildren.end() )
                pParentWinData->maTopWindowChildren.erase( myPos );
        }
    }

    // cleanup Extra Window Data, TODO: add and use ImplWinData destructor
    if ( mpWindowImpl->mpWinData )
    {
        delete mpWindowImpl->mpWinData->mpExtOldText;
        delete mpWindowImpl->mpWinData->mpExtOldAttrAry;
        delete mpWindowImpl->mpWinData->mpCursorRect;
        delete[] mpWindowImpl->mpWinData->mpCompositionCharRects;
        delete mpWindowImpl->mpWinData->mpFocusRect;
        delete mpWindowImpl->mpWinData->mpTrackRect;
        delete mpWindowImpl->mpWinData;
    }

    // cleanup overlap related window data
    delete mpWindowImpl->mpOverlapData;

    // remove BorderWindow or Frame window data
    mpWindowImpl->mpBorderWindow.disposeAndClear();
    if ( mpWindowImpl->mbFrame )
    {
        if ( pSVData->maWinData.mpFirstFrame == this )
            pSVData->maWinData.mpFirstFrame = mpWindowImpl->mpFrameData->mpNextFrame;
        else
        {
            vcl::Window* pSysWin = pSVData->maWinData.mpFirstFrame;
            while ( pSysWin->mpWindowImpl->mpFrameData->mpNextFrame.get() != this )
                pSysWin = pSysWin->mpWindowImpl->mpFrameData->mpNextFrame;

            assert (mpWindowImpl->mpFrameData->mpNextFrame.get() != pSysWin);
            pSysWin->mpWindowImpl->mpFrameData->mpNextFrame = mpWindowImpl->mpFrameData->mpNextFrame;
        }
        mpWindowImpl->mpFrame->SetCallback( nullptr, nullptr );
        pSVData->mpDefInst->DestroyFrame( mpWindowImpl->mpFrame );
        assert (mpWindowImpl->mpFrameData->mnFocusId == nullptr);
        assert (mpWindowImpl->mpFrameData->mnMouseMoveId == nullptr);
        delete mpWindowImpl->mpFrameData;
    }

    // should be the last statements
    delete mpWindowImpl; mpWindowImpl = nullptr;

    OutputDevice::dispose();
}

Window::~Window()
{
    // FIXME: we should kill all LazyDeletor usage.
    vcl::LazyDeletor::Undelete( this );
    disposeOnce();
}

// We will eventually being removing the inheritance of OutputDevice
// from Window. It will be replaced with a transient relationship such
// that the OutputDevice is only live for the scope of the Paint method.
// In the meantime this can help move us towards a Window use an
// OutputDevice, not being one.

::OutputDevice const* Window::GetOutDev() const
{
    return this;
}

::OutputDevice* Window::GetOutDev()
{
    return this;
}

} /* namespace vcl */

WindowImpl::WindowImpl( WindowType nType )
{
    maZoom                              = Fraction( 1, 1 );
    maWinRegion                         = vcl::Region(true);
    maWinClipRegion                     = vcl::Region(true);
    mpWinData                           = nullptr;                      // Extra Window Data, that we don't need for all windows
    mpOverlapData                       = nullptr;                      // Overlap Data
    mpFrameData                         = nullptr;                      // Frame Data
    mpFrame                             = nullptr;                      // Pointer to frame window
    mpSysObj                            = nullptr;
    mpFrameWindow                       = nullptr;                      // window to top level parent (same as frame window)
    mpOverlapWindow                     = nullptr;                      // first overlap parent
    mpBorderWindow                      = nullptr;                      // Border-Window
    mpClientWindow                      = nullptr;                      // Client-Window of a FrameWindow
    mpParent                            = nullptr;                      // parent (incl. BorderWindow)
    mpRealParent                        = nullptr;                      // real parent (excl. BorderWindow)
    mpFirstChild                        = nullptr;                      // first child window
    mpLastChild                         = nullptr;                      // last child window
    mpFirstOverlap                      = nullptr;                      // first overlap window (only set in overlap windows)
    mpLastOverlap                       = nullptr;                      // last overlap window (only set in overlap windows)
    mpPrev                              = nullptr;                      // prev window
    mpNext                              = nullptr;                      // next window
    mpNextOverlap                       = nullptr;                      // next overlap window of frame
    mpLastFocusWindow                   = nullptr;                      // window for focus restore
    mpDlgCtrlDownWindow                 = nullptr;                      // window for dialog control
    mpFirstDel                          = nullptr;                      // Dtor notification list
    mpUserData                          = nullptr;                      // user data
    mpCursor                            = nullptr;                      // cursor
    mpControlFont                       = nullptr;                      // font properties
    mpVCLXWindow                        = nullptr;
    mpAccessibleInfos                   = nullptr;
    maControlForeground                 = Color( COL_TRANSPARENT );  // no foreground set
    maControlBackground                 = Color( COL_TRANSPARENT );  // no background set
    mnLeftBorder                        = 0;                         // left border
    mnTopBorder                         = 0;                         // top border
    mnRightBorder                       = 0;                         // right border
    mnBottomBorder                      = 0;                         // bottom border
    mnWidthRequest                      = -1;                        // width request
    mnHeightRequest                     = -1;                        // height request
    mnOptimalWidthCache                 = -1;                        // optimal width cache
    mnOptimalHeightCache                = -1;                        // optimal height cache
    mnX                                 = 0;                         // X-Position to Parent
    mnY                                 = 0;                         // Y-Position to Parent
    mnAbsScreenX                        = 0;                         // absolute X-position on screen, used for RTL window positioning
    mpChildClipRegion                   = nullptr;                      // Child-Clip-Region when ClipChildren
    mpPaintRegion                       = nullptr;                      // Paint-ClipRegion
    mnStyle                             = 0;                         // style (init in ImplInitWindow)
    mnPrevStyle                         = 0;                         // prevstyle (set in SetStyle)
    mnExtendedStyle                     = 0;                         // extended style (init in ImplInitWindow)
    mnPrevExtendedStyle                 = 0;                         // prevstyle (set in SetExtendedStyle)
    mnType                              = nType;                     // type
    mnGetFocusFlags                     = GetFocusFlags::NONE;       // Flags fuer GetFocus()-Aufruf
    mnWaitCount                         = 0;                         // Wait-Count (>1 == Warte-MousePointer)
    mnPaintFlags                        = 0;                         // Flags for ImplCallPaint
    mnParentClipMode                    = ParentClipMode::NONE;      // Flags for Parent-ClipChildren-Mode
    mnActivateMode                      = ActivateModeFlags::NONE;   // Will be converted in System/Overlap-Windows
    mnDlgCtrlFlags                      = DialogControlFlags::NONE;  // DialogControl-Flags
    mnLockCount                         = 0;                         // LockCount
    meAlwaysInputMode                   = AlwaysInputNone;           // neither AlwaysEnableInput nor AlwaysDisableInput called
    meHalign                            = VCL_ALIGN_FILL;
    meValign                            = VCL_ALIGN_FILL;
    mePackType                          = VCL_PACK_START;
    mnPadding                           = 0;
    mnGridHeight                        = 1;
    mnGridLeftAttach                    = -1;
    mnGridTopAttach                     = -1;
    mnGridWidth                         = 1;
    mnBorderWidth                       = 0;
    mnMarginLeft                        = 0;
    mnMarginRight                       = 0;
    mnMarginTop                         = 0;
    mnMarginBottom                      = 0;
    mbFrame                             = false;                     // true: Window is a frame window
    mbBorderWin                         = false;                     // true: Window is a border window
    mbOverlapWin                        = false;                     // true: Window is a overlap window
    mbSysWin                            = false;                     // true: SystemWindow is the base class
    mbDialog                            = false;                     // true: Dialog is the base class
    mbDockWin                           = false;                     // true: DockingWindow is the base class
    mbFloatWin                          = false;                     // true: FloatingWindow is the base class
    mbPushButton                        = false;                     // true: PushButton is the base class
    mbToolBox                           = false;                     // true: ToolBox is the base class
    mbMenuFloatingWindow                = false;                     // true: MenuFloatingWindow is the base class
    mbToolbarFloatingWindow             = false;                     // true: ImplPopupFloatWin is the base class, used for subtoolbars
    mbSplitter                          = false;                     // true: Splitter is the base class
    mbVisible                           = false;                     // true: Show( true ) called
    mbOverlapVisible                    = false;                     // true: Hide called for visible window from ImplHideAllOverlapWindow()
    mbDisabled                          = false;                     // true: Enable( false ) called
    mbInputDisabled                     = false;                     // true: EnableInput( false ) called
    mbDropDisabled                      = false;                     // true: Drop is enabled
    mbNoUpdate                          = false;                     // true: SetUpdateMode( false ) called
    mbNoParentUpdate                    = false;                     // true: SetParentUpdateMode( false ) called
    mbActive                            = false;                     // true: Window Active
    mbParentActive                      = false;                     // true: OverlapActive from Parent
    mbReallyVisible                     = false;                     // true: this and all parents to an overlapped window are visible
    mbReallyShown                       = false;                     // true: this and all parents to an overlapped window are shown
    mbInInitShow                        = false;                     // true: we are in InitShow
    mbChildNotify                       = false;                     // true: ChildNotify
    mbChildPtrOverwrite                 = false;                     // true: PointerStyle overwrites Child-Pointer
    mbNoPtrVisible                      = false;                     // true: ShowPointer( false ) called
    mbMouseMove                         = false;                     // true: BaseMouseMove called
    mbPaintFrame                        = false;                     // true: Paint is visible, but not painted
    mbInPaint                           = false;                     // true: Inside PaintHdl
    mbMouseButtonDown                   = false;                     // true: BaseMouseButtonDown called
    mbMouseButtonUp                     = false;                     // true: BaseMouseButtonUp called
    mbKeyInput                          = false;                     // true: BaseKeyInput called
    mbKeyUp                             = false;                     // true: BaseKeyUp called
    mbCommand                           = false;                     // true: BaseCommand called
    mbDefPos                            = true;                      // true: Position is not Set
    mbDefSize                           = true;                      // true: Size is not Set
    mbCallMove                          = true;                      // true: Move must be called by Show
    mbCallResize                        = true;                      // true: Resize must be called by Show
    mbWaitSystemResize                  = true;                      // true: Wait for System-Resize
    mbInitWinClipRegion                 = true;                      // true: Calc Window Clip Region
    mbInitChildRegion                   = false;                     // true: InitChildClipRegion
    mbWinRegion                         = false;                     // true: Window Region
    mbClipChildren                      = false;                     // true: Child-window should be clipped
    mbClipSiblings                      = false;                     // true: Adjacent Child-window should be clipped
    mbChildTransparent                  = false;                     // true: Child-windows are allowed to switch to transparent (incl. Parent-CLIPCHILDREN)
    mbPaintTransparent                  = false;                     // true: Paints should be executed on the Parent
    mbMouseTransparent                  = false;                     // true: Window is transparent for Mouse
    mbDlgCtrlStart                      = false;                     // true: From here on own Dialog-Control
    mbFocusVisible                      = false;                     // true: Focus Visible
    mbUseNativeFocus                    = false;
    mbNativeFocusVisible                = false;                     // true: native Focus Visible
    mbInShowFocus                       = false;                     // prevent recursion
    mbInHideFocus                       = false;                     // prevent recursion
    mbTrackVisible                      = false;                     // true: Tracking Visible
    mbControlForeground                 = false;                     // true: Foreground-Property set
    mbControlBackground                 = false;                     // true: Background-Property set
    mbAlwaysOnTop                       = false;                     // true: always visible for all others windows
    mbCompoundControl                   = false;                     // true: Composite Control => Listener...
    mbCompoundControlHasFocus           = false;                     // true: Composite Control has focus somewhere
    mbPaintDisabled                     = false;                     // true: Paint should not be executed
    mbAllResize                         = false;                     // true: Also sent ResizeEvents with 0,0
    mbInDispose                         = false;                     // true: We're still in Window::dispose()
    mbExtTextInput                      = false;                     // true: ExtTextInput-Mode is active
    mbInFocusHdl                        = false;                     // true: Within GetFocus-Handler
    mbCreatedWithToolkit                = false;
    mbSuppressAccessibilityEvents       = false;                     // true: do not send any accessibility events
    mbDrawSelectionBackground           = false;                     // true: draws transparent window background to indicate (toolbox) selection
    mbIsInTaskPaneList                  = false;                     // true: window was added to the taskpanelist in the topmost system window
    mnNativeBackground                  = 0;                         // initialize later, depends on type
    mbCallHandlersDuringInputDisabled   = false;                     // true: call event handlers even if input is disabled
    mbHelpTextDynamic                   = false;                     // true: append help id in HELP_DEBUG case
    mbFakeFocusSet                      = false;                     // true: pretend as if the window has focus.
    mbHexpand                           = false;
    mbVexpand                           = false;
    mbExpand                            = false;
    mbFill                              = true;
    mbSecondary                         = false;
    mbNonHomogeneous                    = false;
    mbDoubleBufferingRequested = getenv("VCL_DOUBLEBUFFERING_FORCE_ENABLE"); // when we are not sure, assume it cannot do double-buffering via RenderContext
}

WindowImpl::~WindowImpl()
{
    delete mpChildClipRegion;
    delete mpAccessibleInfos;
    delete mpControlFont;
}

namespace vcl {

bool Window::AcquireGraphics() const
{
    DBG_TESTSOLARMUTEX();

    if ( mpGraphics )
        return true;

    mbInitLineColor     = true;
    mbInitFillColor     = true;
    mbInitFont          = true;
    mbInitTextColor     = true;
    mbInitClipRegion    = true;

    ImplSVData* pSVData = ImplGetSVData();

    mpGraphics = mpWindowImpl->mpFrame->AcquireGraphics();
    // try harder if no wingraphics was available directly
    if ( !mpGraphics )
    {
        // find another output device in the same frame
        OutputDevice* pReleaseOutDev = pSVData->maGDIData.mpLastWinGraphics;
        while ( pReleaseOutDev )
        {
            if ( static_cast<vcl::Window*>(pReleaseOutDev)->mpWindowImpl->mpFrame == mpWindowImpl->mpFrame )
                break;
            pReleaseOutDev = pReleaseOutDev->mpPrevGraphics;
        }

        if ( pReleaseOutDev )
        {
            // steal the wingraphics from the other outdev
            mpGraphics = pReleaseOutDev->mpGraphics;
            pReleaseOutDev->ReleaseGraphics( false );
        }
        else
        {
            // if needed retry after releasing least recently used wingraphics
            while ( !mpGraphics )
            {
                if ( !pSVData->maGDIData.mpLastWinGraphics )
                    break;
                pSVData->maGDIData.mpLastWinGraphics->ReleaseGraphics();
                mpGraphics = mpWindowImpl->mpFrame->AcquireGraphics();
            }
        }
    }

    // update global LRU list of wingraphics
    if ( mpGraphics )
    {
        mpNextGraphics = pSVData->maGDIData.mpFirstWinGraphics;
        pSVData->maGDIData.mpFirstWinGraphics = const_cast<vcl::Window*>(this);
        if ( mpNextGraphics )
            mpNextGraphics->mpPrevGraphics = const_cast<vcl::Window*>(this);
        if ( !pSVData->maGDIData.mpLastWinGraphics )
            pSVData->maGDIData.mpLastWinGraphics = const_cast<vcl::Window*>(this);
    }

    if ( mpGraphics )
    {
        mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp), ROP_INVERT == meRasterOp );
        mpGraphics->setAntiAliasB2DDraw(bool(mnAntialiasing & AntialiasingFlags::EnableB2dDraw));
    }

    return mpGraphics != nullptr;
}

void Window::ReleaseGraphics( bool bRelease )
{
    DBG_TESTSOLARMUTEX();

    if ( !mpGraphics )
        return;

    // release the fonts of the physically released graphics device
    if( bRelease )
        ImplReleaseFonts();

    ImplSVData* pSVData = ImplGetSVData();

    vcl::Window* pWindow = this;

    if ( bRelease )
        pWindow->mpWindowImpl->mpFrame->ReleaseGraphics( mpGraphics );
    // remove from global LRU list of window graphics
    if ( mpPrevGraphics )
        mpPrevGraphics->mpNextGraphics = mpNextGraphics;
    else
        pSVData->maGDIData.mpFirstWinGraphics = mpNextGraphics;
    if ( mpNextGraphics )
        mpNextGraphics->mpPrevGraphics = mpPrevGraphics;
    else
        pSVData->maGDIData.mpLastWinGraphics = mpPrevGraphics;

    mpGraphics      = nullptr;
    mpPrevGraphics  = nullptr;
    mpNextGraphics  = nullptr;
}

static sal_Int32 CountDPIScaleFactor(sal_Int32 nDPI)
{
    sal_Int32 nResult = 1;

#ifndef MACOSX
    // Setting of HiDPI is unfortunately all only a heuristic; and to add
    // insult to an injury, the system is constantly lying to us about
    // the DPI and whatnot
    // eg. fdo#77059 - set the value from which we do consider the
    // screen hi-dpi to greater than 168
    if (nDPI > 168)
        nResult = std::max(sal_Int32(1), (nDPI + 48) / 96);
#else
    (void)nDPI;
#endif

    return nResult;
}

void Window::ImplInit( vcl::Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
{
    DBG_ASSERT( mpWindowImpl->mbFrame || pParent || GetType() == WINDOW_FIXEDIMAGE,
        "Window::Window(): pParent == NULL" );

    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window*     pRealParent = pParent;

    // inherit 3D look
    if ( !mpWindowImpl->mbOverlapWin && pParent && (pParent->GetStyle() & WB_3DLOOK) )
        nStyle |= WB_3DLOOK;

    // create border window if necessary
    if ( !mpWindowImpl->mbFrame && !mpWindowImpl->mbBorderWin && !mpWindowImpl->mpBorderWindow
         && (nStyle & (WB_BORDER | WB_SYSTEMCHILDWINDOW) ) )
    {
        sal_uInt16 nBorderTypeStyle = 0;
        if( (nStyle & WB_SYSTEMCHILDWINDOW) )
        {
            // handle WB_SYSTEMCHILDWINDOW
            // these should be analogous to a top level frame; meaning they
            // should have a border window with style BORDERWINDOW_STYLE_FRAME
            // which controls their size
            nBorderTypeStyle |= BORDERWINDOW_STYLE_FRAME;
            nStyle |= WB_BORDER;
        }
        VclPtrInstance<ImplBorderWindow> pBorderWin( pParent, nStyle & (WB_BORDER | WB_DIALOGCONTROL | WB_NODIALOGCONTROL), nBorderTypeStyle );
        static_cast<vcl::Window*>(pBorderWin)->mpWindowImpl->mpClientWindow = this;
        pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
        mpWindowImpl->mpBorderWindow  = pBorderWin;
        pParent = mpWindowImpl->mpBorderWindow;
    }
    else if( !mpWindowImpl->mbFrame && ! pParent )
    {
        mpWindowImpl->mbOverlapWin  = true;
        mpWindowImpl->mbFrame = true;
    }

    // insert window in list
    ImplInsertWindow( pParent );
    mpWindowImpl->mnStyle = nStyle;

    // Overlap-Window-Data
    if ( mpWindowImpl->mbOverlapWin )
    {
        mpWindowImpl->mpOverlapData                   = new ImplOverlapData;
        mpWindowImpl->mpOverlapData->mpSaveBackDev    = nullptr;
        mpWindowImpl->mpOverlapData->mpSaveBackRgn    = nullptr;
        mpWindowImpl->mpOverlapData->mpNextBackWin    = nullptr;
        mpWindowImpl->mpOverlapData->mnSaveBackSize   = 0;
        mpWindowImpl->mpOverlapData->mbSaveBack       = false;
        mpWindowImpl->mpOverlapData->mnTopLevel       = 1;
    }

    if( pParent && ! mpWindowImpl->mbFrame )
        mbEnableRTL = AllSettings::GetLayoutRTL();

    // test for frame creation
    if ( mpWindowImpl->mbFrame )
    {
        // create frame
        SalFrameStyleFlags nFrameStyle = SalFrameStyleFlags::NONE;

        if ( nStyle & WB_MOVEABLE )
            nFrameStyle |= SalFrameStyleFlags::MOVEABLE;
        if ( nStyle & WB_SIZEABLE )
            nFrameStyle |= SalFrameStyleFlags::SIZEABLE;
        if ( nStyle & WB_CLOSEABLE )
            nFrameStyle |= SalFrameStyleFlags::CLOSEABLE;
        if ( nStyle & WB_APP )
            nFrameStyle |= SalFrameStyleFlags::DEFAULT;
        // check for undecorated floating window
        if( // 1. floating windows that are not moveable/sizeable (only closeable allowed)
            ( !(nFrameStyle & ~SalFrameStyleFlags::CLOSEABLE) &&
            ( mpWindowImpl->mbFloatWin || ((GetType() == WINDOW_BORDERWINDOW) && static_cast<ImplBorderWindow*>(this)->mbFloatWindow) || (nStyle & WB_SYSTEMFLOATWIN) ) ) ||
            // 2. borderwindows of floaters with ownerdraw decoration
            ( ((GetType() == WINDOW_BORDERWINDOW) && static_cast<ImplBorderWindow*>(this)->mbFloatWindow && (nStyle & WB_OWNERDRAWDECORATION) ) ) )
        {
            nFrameStyle = SalFrameStyleFlags::FLOAT;
            if( nStyle & WB_OWNERDRAWDECORATION )
                nFrameStyle |= (SalFrameStyleFlags::OWNERDRAWDECORATION | SalFrameStyleFlags::NOSHADOW);
        }
        else if( mpWindowImpl->mbFloatWin )
            nFrameStyle |= SalFrameStyleFlags::TOOLWINDOW;

        if( nStyle & WB_INTROWIN )
            nFrameStyle |= SalFrameStyleFlags::INTRO;
        if( nStyle & WB_TOOLTIPWIN )
            nFrameStyle |= SalFrameStyleFlags::TOOLTIP;

        if( nStyle & WB_NOSHADOW )
            nFrameStyle |= SalFrameStyleFlags::NOSHADOW;

        if( nStyle & WB_SYSTEMCHILDWINDOW )
            nFrameStyle |= SalFrameStyleFlags::SYSTEMCHILD;

        switch (mpWindowImpl->mnType)
        {
            case WINDOW_DIALOG:
            case WINDOW_TABDIALOG:
            case WINDOW_MODALDIALOG:
            case WINDOW_MODELESSDIALOG:
            case WINDOW_MESSBOX:
            case WINDOW_INFOBOX:
            case WINDOW_WARNINGBOX:
            case WINDOW_ERRORBOX:
            case WINDOW_QUERYBOX:
                nFrameStyle |= SalFrameStyleFlags::DIALOG;
            default:
                break;
        }

        SalFrame* pParentFrame = nullptr;
        if ( pParent )
            pParentFrame = pParent->mpWindowImpl->mpFrame;
        SalFrame* pFrame;
        if ( pSystemParentData )
            pFrame = pSVData->mpDefInst->CreateChildFrame( pSystemParentData, nFrameStyle | SalFrameStyleFlags::PLUG );
        else
            pFrame = pSVData->mpDefInst->CreateFrame( pParentFrame, nFrameStyle );
        if ( !pFrame )
        {
            // do not abort but throw an exception, may be the current thread terminates anyway (plugin-scenario)
            throw RuntimeException(
                "Could not create system window!",
                Reference< XInterface >() );
        }

        pFrame->SetCallback( this, ImplWindowFrameProc );

        // set window frame data
        mpWindowImpl->mpFrameData     = new ImplFrameData;
        mpWindowImpl->mpFrame         = pFrame;
        mpWindowImpl->mpFrameWindow   = this;
        mpWindowImpl->mpOverlapWindow = this;

        // set frame data
        assert (pSVData->maWinData.mpFirstFrame.get() != this);
        mpWindowImpl->mpFrameData->mpNextFrame        = pSVData->maWinData.mpFirstFrame;
        pSVData->maWinData.mpFirstFrame = this;
        mpWindowImpl->mpFrameData->mpFirstOverlap     = nullptr;
        mpWindowImpl->mpFrameData->mpFocusWin         = nullptr;
        mpWindowImpl->mpFrameData->mpMouseMoveWin     = nullptr;
        mpWindowImpl->mpFrameData->mpMouseDownWin     = nullptr;
        mpWindowImpl->mpFrameData->mpFirstBackWin     = nullptr;
        mpWindowImpl->mpFrameData->mpFontCollection   = pSVData->maGDIData.mpScreenFontList;
        mpWindowImpl->mpFrameData->mpFontCache        = pSVData->maGDIData.mpScreenFontCache;
        mpWindowImpl->mpFrameData->mnAllSaveBackSize  = 0;
        mpWindowImpl->mpFrameData->mnFocusId          = nullptr;
        mpWindowImpl->mpFrameData->mnMouseMoveId      = nullptr;
        mpWindowImpl->mpFrameData->mnLastMouseX       = -1;
        mpWindowImpl->mpFrameData->mnLastMouseY       = -1;
        mpWindowImpl->mpFrameData->mnBeforeLastMouseX = -1;
        mpWindowImpl->mpFrameData->mnBeforeLastMouseY = -1;
        mpWindowImpl->mpFrameData->mnFirstMouseX      = -1;
        mpWindowImpl->mpFrameData->mnFirstMouseY      = -1;
        mpWindowImpl->mpFrameData->mnLastMouseWinX    = -1;
        mpWindowImpl->mpFrameData->mnLastMouseWinY    = -1;
        mpWindowImpl->mpFrameData->mnModalMode        = 0;
        mpWindowImpl->mpFrameData->mnMouseDownTime    = 0;
        mpWindowImpl->mpFrameData->mnClickCount       = 0;
        mpWindowImpl->mpFrameData->mnFirstMouseCode   = 0;
        mpWindowImpl->mpFrameData->mnMouseCode        = 0;
        mpWindowImpl->mpFrameData->mnMouseMode        = MouseEventModifiers::NONE;
        mpWindowImpl->mpFrameData->meMapUnit          = MAP_PIXEL;
        mpWindowImpl->mpFrameData->mbHasFocus         = false;
        mpWindowImpl->mpFrameData->mbInMouseMove      = false;
        mpWindowImpl->mpFrameData->mbMouseIn          = false;
        mpWindowImpl->mpFrameData->mbStartDragCalled  = false;
        mpWindowImpl->mpFrameData->mbNeedSysWindow    = false;
        mpWindowImpl->mpFrameData->mbMinimized        = false;
        mpWindowImpl->mpFrameData->mbStartFocusState  = false;
        mpWindowImpl->mpFrameData->mbInSysObjFocusHdl = false;
        mpWindowImpl->mpFrameData->mbInSysObjToTopHdl = false;
        mpWindowImpl->mpFrameData->mbSysObjFocus      = false;
        if (!ImplDoTiledRendering())
        {
            mpWindowImpl->mpFrameData->maPaintIdle.SetPriority( SchedulerPriority::REPAINT );
            mpWindowImpl->mpFrameData->maPaintIdle.SetIdleHdl( LINK( this, Window, ImplHandlePaintHdl ) );
            mpWindowImpl->mpFrameData->maPaintIdle.SetDebugName( "vcl::Window maPaintIdle" );
        }
        mpWindowImpl->mpFrameData->maResizeIdle.SetPriority( SchedulerPriority::RESIZE );
        mpWindowImpl->mpFrameData->maResizeIdle.SetIdleHdl( LINK( this, Window, ImplHandleResizeTimerHdl ) );
        mpWindowImpl->mpFrameData->maResizeIdle.SetDebugName( "vcl::Window maResizeIdle" );
        mpWindowImpl->mpFrameData->mbInternalDragGestureRecognizer = false;
        if (!(nStyle & WB_DEFAULTWIN) && mpWindowImpl->mbDoubleBufferingRequested)
            RequestDoubleBuffering(true);
        mpWindowImpl->mpFrameData->mbInBufferedPaint = false;

        if ( pRealParent && IsTopWindow() )
        {
            ImplWinData* pParentWinData = pRealParent->ImplGetWinData();
            pParentWinData->maTopWindowChildren.push_back( this );
        }
    }

    // init data
    mpWindowImpl->mpRealParent = pRealParent;

    // #99318: make sure fontcache and list is available before call to SetSettings
    mpFontCollection      = mpWindowImpl->mpFrameData->mpFontCollection;
    mpFontCache     = mpWindowImpl->mpFrameData->mpFontCache;

    if ( mpWindowImpl->mbFrame )
    {
        if ( pParent )
        {
            mpWindowImpl->mpFrameData->mnDPIX     = pParent->mpWindowImpl->mpFrameData->mnDPIX;
            mpWindowImpl->mpFrameData->mnDPIY     = pParent->mpWindowImpl->mpFrameData->mnDPIY;
        }
        else
        {
            OutputDevice *pOutDev = GetOutDev();
            if ( pOutDev->AcquireGraphics() )
            {
                mpGraphics->GetResolution( mpWindowImpl->mpFrameData->mnDPIX, mpWindowImpl->mpFrameData->mnDPIY );
            }
        }

        // add ownerdraw decorated frame windows to list in the top-most frame window
        // so they can be hidden on lose focus
        if( nStyle & WB_OWNERDRAWDECORATION )
            ImplGetOwnerDrawList().push_back( this );

        // delay settings initialization until first "real" frame
        // this relies on the IntroWindow not needing any system settings
        if ( !pSVData->maAppData.mbSettingsInit &&
             ! (nStyle & (WB_INTROWIN|WB_DEFAULTWIN))
             )
        {
            // side effect: ImplUpdateGlobalSettings does an ImplGetFrame()->UpdateSettings
            ImplUpdateGlobalSettings( *pSVData->maAppData.mpSettings );
            OutputDevice::SetSettings( *pSVData->maAppData.mpSettings );
            pSVData->maAppData.mbSettingsInit = true;
        }

        // If we create a Window with default size, query this
        // size directly, because we want resize all Controls to
        // the correct size before we display the window
        if ( nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_APP) )
            mpWindowImpl->mpFrame->GetClientSize( mnOutWidth, mnOutHeight );
    }
    else
    {
        if ( pParent )
        {
            if ( !ImplIsOverlapWindow() )
            {
                mpWindowImpl->mbDisabled          = pParent->mpWindowImpl->mbDisabled;
                mpWindowImpl->mbInputDisabled     = pParent->mpWindowImpl->mbInputDisabled;
                mpWindowImpl->meAlwaysInputMode   = pParent->mpWindowImpl->meAlwaysInputMode;
            }

            if (!utl::ConfigManager::IsAvoidConfig())
                OutputDevice::SetSettings( pParent->GetSettings() );
        }

    }

    // setup the scale factor for Hi-DPI displays
    mnDPIScaleFactor = CountDPIScaleFactor(mpWindowImpl->mpFrameData->mnDPIY);

    if (!utl::ConfigManager::IsAvoidConfig())
    {
        const StyleSettings& rStyleSettings = mxSettings->GetStyleSettings();
        sal_uInt16 nScreenZoom = rStyleSettings.GetScreenZoom();
        mnDPIX          = (mpWindowImpl->mpFrameData->mnDPIX*nScreenZoom)/100;
        mnDPIY          = (mpWindowImpl->mpFrameData->mnDPIY*nScreenZoom)/100;
        maFont          = rStyleSettings.GetAppFont();

        if ( nStyle & WB_3DLOOK )
        {
            SetTextColor( rStyleSettings.GetButtonTextColor() );
            SetBackground( Wallpaper( rStyleSettings.GetFaceColor() ) );
        }
        else
        {
            SetTextColor( rStyleSettings.GetWindowTextColor() );
            SetBackground( Wallpaper( rStyleSettings.GetWindowColor() ) );
        }
    }
    else
    {
        mnDPIX          = 96;
        mnDPIY          = 96;
        maFont = GetDefaultFont( DefaultFontType::FIXED, LANGUAGE_ENGLISH_US, GetDefaultFontFlags::NONE );
    }

    ImplPointToLogic(*this, maFont);

    (void)ImplUpdatePos();

    // calculate app font res (except for the Intro Window or the default window)
    if ( mpWindowImpl->mbFrame && !pSVData->maGDIData.mnAppFontX && ! (nStyle & (WB_INTROWIN|WB_DEFAULTWIN)) )
        ImplInitAppFontData( this );

    if ( GetAccessibleParentWindow()  && GetParent() != Application::GetDefDialogParent() )
        GetAccessibleParentWindow()->CallEventListeners( VCLEVENT_WINDOW_CHILDCREATED, this );
}

void Window::ImplInitAppFontData( vcl::Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    long nTextHeight = pWindow->GetTextHeight();
    long nTextWidth = pWindow->approximate_char_width() * 8;
    long nSymHeight = nTextHeight*4;
    // Make the basis wider if the font is too narrow
    // such that the dialog looks symmetrical and does not become too narrow.
    // Add some extra space when the dialog has the same width,
    // as a little more space is better.
    if ( nSymHeight > nTextWidth )
        nTextWidth = nSymHeight;
    else if ( nSymHeight+5 > nTextWidth )
        nTextWidth = nSymHeight+5;
    pSVData->maGDIData.mnAppFontX = nTextWidth * 10 / 8;
    pSVData->maGDIData.mnAppFontY = nTextHeight * 10;

#ifdef MACOSX
    // FIXME: this is currently only on OS X, check with other
    // platforms
    if( pSVData->maNWFData.mbNoFocusRects )
    {
        // try to find out whether there is a large correction
        // of control sizes, if yes, make app font scalings larger
        // so dialog positioning is not completely off
        ImplControlValue aControlValue;
        Rectangle aCtrlRegion( Point(), Size( nTextWidth < 10 ? 10 : nTextWidth, nTextHeight < 10 ? 10 : nTextHeight ) );
        Rectangle aBoundingRgn( aCtrlRegion );
        Rectangle aContentRgn( aCtrlRegion );
        if( pWindow->GetNativeControlRegion( CTRL_EDITBOX, PART_ENTIRE_CONTROL, aCtrlRegion,
                                             ControlState::ENABLED, aControlValue, OUString(),
                                             aBoundingRgn, aContentRgn ) )
        {
            // comment: the magical +6 is for the extra border in bordered
            // (which is the standard) edit fields
            if( aContentRgn.GetHeight() - nTextHeight > (nTextHeight+4)/4 )
                pSVData->maGDIData.mnAppFontY = (aContentRgn.GetHeight()-4) * 10;
        }
    }
#endif

    pSVData->maGDIData.mnRealAppFontX = pSVData->maGDIData.mnAppFontX;
    if ( pSVData->maAppData.mnDialogScaleX )
        pSVData->maGDIData.mnAppFontX += (pSVData->maGDIData.mnAppFontX*pSVData->maAppData.mnDialogScaleX)/100;
}

void Window::ImplInitWindowData( WindowType nType )
{
    mpWindowImpl = new WindowImpl( nType );

    meOutDevType        = OUTDEV_WINDOW;

    mbEnableRTL         = AllSettings::GetLayoutRTL();         // true: this outdev will be mirrored if RTL window layout (UI mirroring) is globally active
}

void Window::getFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize, OutputDevice& rDev )
{
    bool bOldMap = mbMap;
    mbMap = false;
    rDev.DrawOutDev( rDevPt, rDevSize, rPt, rDevSize, *this );
    mbMap = bOldMap;
}

void Window::drawFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                           const OutputDevice& rOutDev, const vcl::Region& rRegion )
{

    GDIMetaFile*    pOldMetaFile = mpMetaFile;
    bool            bOldMap = mbMap;
    RasterOp        eOldROP = GetRasterOp();
    mpMetaFile = nullptr;
    mbMap = false;
    SetRasterOp( ROP_OVERPAINT );

    if ( !IsDeviceOutputNecessary() )
        return;

    if ( !mpGraphics )
    {
        if ( !AcquireGraphics() )
            return;
    }

    if ( rRegion.IsNull() )
        mpGraphics->ResetClipRegion();
    else
        SelectClipRegion( rRegion );

    SalTwoRect aPosAry(rDevPt.X(), rDevPt.Y(), rDevSize.Width(), rDevSize.Height(),
                       rPt.X(), rPt.Y(), rDevSize.Width(), rDevSize.Height());
    drawOutDevDirect( &rOutDev, aPosAry );

    // Ensure that ClipRegion is recalculated and set
    mbInitClipRegion = true;

    SetRasterOp( eOldROP );
    mbMap = bOldMap;
    mpMetaFile = pOldMetaFile;
}


ImplWinData* Window::ImplGetWinData() const
{
    if ( !mpWindowImpl->mpWinData )
    {
        static const char* pNoNWF = getenv( "SAL_NO_NWF" );

        const_cast<vcl::Window*>(this)->mpWindowImpl->mpWinData = new ImplWinData;
        mpWindowImpl->mpWinData->mpExtOldText     = nullptr;
        mpWindowImpl->mpWinData->mpExtOldAttrAry  = nullptr;
        mpWindowImpl->mpWinData->mpCursorRect     = nullptr;
        mpWindowImpl->mpWinData->mnCursorExtWidth = 0;
        mpWindowImpl->mpWinData->mpCompositionCharRects = nullptr;
        mpWindowImpl->mpWinData->mnCompositionCharRects = 0;
        mpWindowImpl->mpWinData->mpFocusRect      = nullptr;
        mpWindowImpl->mpWinData->mpTrackRect      = nullptr;
        mpWindowImpl->mpWinData->mnTrackFlags     = 0;
        mpWindowImpl->mpWinData->mnIsTopWindow  = (sal_uInt16) ~0;  // not initialized yet, 0/1 will indicate TopWindow (see IsTopWindow())
        mpWindowImpl->mpWinData->mbMouseOver      = false;
        mpWindowImpl->mpWinData->mbEnableNativeWidget = !(pNoNWF && *pNoNWF); // true: try to draw this control with native theme API
    }

    return mpWindowImpl->mpWinData;
}


void Window::CopyDeviceArea( SalTwoRect& aPosAry, bool bWindowInvalidate )
{
    if (aPosAry.mnSrcWidth == 0 || aPosAry.mnSrcHeight == 0 || aPosAry.mnDestWidth == 0 || aPosAry.mnDestHeight == 0)
        return;

    if (bWindowInvalidate)
    {
        const Rectangle aSrcRect(Point(aPosAry.mnSrcX, aPosAry.mnSrcY),
                Size(aPosAry.mnSrcWidth, aPosAry.mnSrcHeight));

        ImplMoveAllInvalidateRegions(aSrcRect,
                aPosAry.mnDestX-aPosAry.mnSrcX,
                aPosAry.mnDestY-aPosAry.mnSrcY,
                false);

        mpGraphics->CopyArea(aPosAry.mnDestX, aPosAry.mnDestY,
                aPosAry.mnSrcX, aPosAry.mnSrcY,
                aPosAry.mnSrcWidth, aPosAry.mnSrcHeight,
                SAL_COPYAREA_WINDOWINVALIDATE, this);

        return;
    }

    OutputDevice::CopyDeviceArea(aPosAry, bWindowInvalidate);
}

SalGraphics* Window::ImplGetFrameGraphics() const
{
    if ( mpWindowImpl->mpFrameWindow->mpGraphics )
    {
        mpWindowImpl->mpFrameWindow->mbInitClipRegion = true;
    }
    else
    {
        OutputDevice* pFrameWinOutDev = mpWindowImpl->mpFrameWindow;
        if ( ! pFrameWinOutDev->AcquireGraphics() )
        {
            return nullptr;
        }
    }
    mpWindowImpl->mpFrameWindow->mpGraphics->ResetClipRegion();
    return mpWindowImpl->mpFrameWindow->mpGraphics;
}

void Window::ImplSetReallyVisible()
{
    // #i43594# it is possible that INITSHOW was never send, because the visibility state changed between
    // ImplCallInitShow() and ImplSetReallyVisible() when called from Show()
    // mbReallyShown is a useful indicator
    if( !mpWindowImpl->mbReallyShown )
        ImplCallInitShow();

    bool bBecameReallyVisible = !mpWindowImpl->mbReallyVisible;

    mbDevOutput     = true;
    mpWindowImpl->mbReallyVisible = true;
    mpWindowImpl->mbReallyShown   = true;

    // the SHOW/HIDE events serve as indicators to send child creation/destroy events to the access bridge.
    // For this, the data member of the event must not be NULL.
    // Previously, we did this in Window::Show, but there some events got lost in certain situations. Now
    // we're doing it when the visibility really changes
    if( bBecameReallyVisible && ImplIsAccessibleCandidate() )
        CallEventListeners( VCLEVENT_WINDOW_SHOW, this );
        // TODO. It's kind of a hack that we're re-using the VCLEVENT_WINDOW_SHOW. Normally, we should
        // introduce another event which explicitly triggers the Accessibility implementations.

    vcl::Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplSetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplSetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplAddDel( ImplDelData* pDel ) // TODO: make "const" when incompatibility ok
{
    if ( IsDisposed() )
    {
        pDel->mbDel = true;
        return;
    }

    DBG_ASSERT( !pDel->mpWindow, "Window::ImplAddDel(): cannot add ImplDelData twice !" );
    if( !pDel->mpWindow )
    {
        pDel->mpWindow = this;  // #112873# store ref to this window, so pDel can remove itself
        pDel->mpNext = mpWindowImpl->mpFirstDel;
        mpWindowImpl->mpFirstDel = pDel;
    }
}

void Window::ImplRemoveDel( ImplDelData* pDel ) // TODO: make "const" when incompatibility ok
{
    pDel->mpWindow = nullptr;      // #112873# pDel is not associated with a Window anymore

    if ( IsDisposed() )
        return;

    if ( mpWindowImpl->mpFirstDel == pDel )
        mpWindowImpl->mpFirstDel = pDel->mpNext;
    else
    {
        ImplDelData* pData = mpWindowImpl->mpFirstDel;
        while ( pData->mpNext != pDel )
            pData = pData->mpNext;
        pData->mpNext = pDel->mpNext;
    }
}

void Window::ImplInitResolutionSettings()
{
    // recalculate AppFont-resolution and DPI-resolution
    if (mpWindowImpl->mbFrame)
    {
        const StyleSettings& rStyleSettings = mxSettings->GetStyleSettings();
        sal_uInt16 nScreenZoom = rStyleSettings.GetScreenZoom();
        mnDPIX = (mpWindowImpl->mpFrameData->mnDPIX*nScreenZoom)/100;
        mnDPIY = (mpWindowImpl->mpFrameData->mnDPIY*nScreenZoom)/100;

        // setup the scale factor for Hi-DPI displays
        mnDPIScaleFactor = CountDPIScaleFactor(mpWindowImpl->mpFrameData->mnDPIY);
        SetPointFont(*this, rStyleSettings.GetAppFont());
    }
    else if ( mpWindowImpl->mpParent )
    {
        mnDPIX  = mpWindowImpl->mpParent->mnDPIX;
        mnDPIY  = mpWindowImpl->mpParent->mnDPIY;
        mnDPIScaleFactor = mpWindowImpl->mpParent->mnDPIScaleFactor;
    }

    // update the recalculated values for logical units
    // and also tools belonging to the values
    if (IsMapModeEnabled())
    {
        MapMode aMapMode = GetMapMode();
        SetMapMode();
        SetMapMode( aMapMode );
    }
}

void Window::ImplPointToLogic(vcl::RenderContext& rRenderContext, vcl::Font& rFont) const
{
    Size aSize = rFont.GetSize();
    sal_uInt16 nScreenFontZoom;
    if (!utl::ConfigManager::IsAvoidConfig())
        nScreenFontZoom = rRenderContext.GetSettings().GetStyleSettings().GetScreenFontZoom();
    else
        nScreenFontZoom = 100;

    if (aSize.Width())
    {
        aSize.Width() *= mpWindowImpl->mpFrameData->mnDPIX;
        aSize.Width() += 72 / 2;
        aSize.Width() /= 72;
        aSize.Width() *= nScreenFontZoom;
        aSize.Width() /= 100;
    }
    aSize.Height() *= mpWindowImpl->mpFrameData->mnDPIY;
    aSize.Height() += 72/2;
    aSize.Height() /= 72;
    aSize.Height() *= nScreenFontZoom;
    aSize.Height() /= 100;

    if (rRenderContext.IsMapModeEnabled())
        aSize = rRenderContext.PixelToLogic(aSize);

    rFont.SetSize(aSize);
}

void Window::ImplLogicToPoint(vcl::RenderContext& rRenderContext, vcl::Font& rFont) const
{
    Size aSize = rFont.GetSize();
    sal_uInt16 nScreenFontZoom;
    if (!utl::ConfigManager::IsAvoidConfig())
        nScreenFontZoom = rRenderContext.GetSettings().GetStyleSettings().GetScreenFontZoom();
    else
        nScreenFontZoom = 100;

    if (rRenderContext.IsMapModeEnabled())
        aSize = rRenderContext.LogicToPixel(aSize);

    if (aSize.Width())
    {
        aSize.Width() *= 100;
        aSize.Width() /= nScreenFontZoom;
        aSize.Width() *= 72;
        aSize.Width() += mpWindowImpl->mpFrameData->mnDPIX / 2;
        aSize.Width() /= mpWindowImpl->mpFrameData->mnDPIX;
    }
    aSize.Height() *= 100;
    aSize.Height() /= nScreenFontZoom;
    aSize.Height() *= 72;
    aSize.Height() += mpWindowImpl->mpFrameData->mnDPIY / 2;
    aSize.Height() /= mpWindowImpl->mpFrameData->mnDPIY;

    rFont.SetSize(aSize);
}

bool Window::ImplUpdatePos()
{
    bool bSysChild = false;

    if ( ImplIsOverlapWindow() )
    {
        mnOutOffX  = mpWindowImpl->mnX;
        mnOutOffY  = mpWindowImpl->mnY;
    }
    else
    {
        vcl::Window* pParent = ImplGetParent();

        mnOutOffX  = mpWindowImpl->mnX + pParent->mnOutOffX;
        mnOutOffY  = mpWindowImpl->mnY + pParent->mnOutOffY;
    }

    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        if ( pChild->ImplUpdatePos() )
            bSysChild = true;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    if ( mpWindowImpl->mpSysObj )
        bSysChild = true;

    return bSysChild;
}

void Window::ImplUpdateSysObjPos()
{
    if ( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );

    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateSysObjPos();
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

void Window::ImplPosSizeWindow( long nX, long nY,
                                long nWidth, long nHeight, PosSizeFlags nFlags )
{
    bool    bNewPos         = false;
    bool    bNewSize        = false;
    bool    bCopyBits       = false;
    long    nOldOutOffX     = mnOutOffX;
    long    nOldOutOffY     = mnOutOffY;
    long    nOldOutWidth    = mnOutWidth;
    long    nOldOutHeight   = mnOutHeight;
    vcl::Region* pOverlapRegion  = nullptr;
    vcl::Region* pOldRegion      = nullptr;

    if ( IsReallyVisible() )
    {
        if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
            ImplInvalidateAllOverlapBackgrounds();

        Rectangle aOldWinRect( Point( nOldOutOffX, nOldOutOffY ),
                               Size( nOldOutWidth, nOldOutHeight ) );
        pOldRegion = new vcl::Region( aOldWinRect );
        if ( mpWindowImpl->mbWinRegion )
            pOldRegion->Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );

        if ( mnOutWidth && mnOutHeight && !mpWindowImpl->mbPaintTransparent &&
             !mpWindowImpl->mbInitWinClipRegion && !mpWindowImpl->maWinClipRegion.IsEmpty() &&
             !HasPaintEvent() )
            bCopyBits = true;
    }

    bool bnXRecycled = false; // avoid duplicate mirroring in RTL case
    if ( nFlags & PosSizeFlags::Width )
    {
        if(!( nFlags & PosSizeFlags::X ))
        {
            nX = mpWindowImpl->mnX;
            nFlags |= PosSizeFlags::X;
            bnXRecycled = true; // we're using a mnX which was already mirrored in RTL case
        }

        if ( nWidth < 0 )
            nWidth = 0;
        if ( nWidth != mnOutWidth )
        {
            mnOutWidth = nWidth;
            bNewSize = true;
            bCopyBits = false;
        }
    }
    if ( nFlags & PosSizeFlags::Height )
    {
        if ( nHeight < 0 )
            nHeight = 0;
        if ( nHeight != mnOutHeight )
        {
            mnOutHeight = nHeight;
            bNewSize = true;
            bCopyBits = false;
        }
    }

    if ( nFlags & PosSizeFlags::X )
    {
        long nOrgX = nX;
        // --- RTL ---  (compare the screen coordinates)
        Point aPtDev( Point( nX+mnOutOffX, 0 ) );
        OutputDevice *pOutDev = GetOutDev();
        if( pOutDev->HasMirroredGraphics() )
        {
            mpGraphics->mirror( aPtDev.X(), this );

            // #106948# always mirror our pos if our parent is not mirroring, even
            // if we are also not mirroring
            // --- RTL --- check if parent is in different coordinates
            if( !bnXRecycled && mpWindowImpl->mpParent && !mpWindowImpl->mpParent->mpWindowImpl->mbFrame && mpWindowImpl->mpParent->ImplIsAntiparallel() )
            {
                // --- RTL --- (re-mirror at parent window)
                nX = mpWindowImpl->mpParent->mnOutWidth - mnOutWidth - nX;
            }
            /* #i99166# An LTR window in RTL UI that gets sized only would be
               expected to not moved its upper left point
            */
            if( bnXRecycled )
            {
                if( ImplIsAntiparallel() )
                {
                    aPtDev.X() = mpWindowImpl->mnAbsScreenX;
                    nOrgX = mpWindowImpl->maPos.X();
                }
            }
        }
        else if( !bnXRecycled && mpWindowImpl->mpParent && !mpWindowImpl->mpParent->mpWindowImpl->mbFrame && mpWindowImpl->mpParent->ImplIsAntiparallel() )
        {
            // mirrored window in LTR UI
            {
                // --- RTL --- (re-mirror at parent window)
                nX = mpWindowImpl->mpParent->mnOutWidth - mnOutWidth - nX;
            }
        }

        // check maPos as well, as it could have been changed for client windows (ImplCallMove())
        if ( mpWindowImpl->mnAbsScreenX != aPtDev.X() || nX != mpWindowImpl->mnX || nOrgX != mpWindowImpl->maPos.X() )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new vcl::Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, false, true, true );
            }
            mpWindowImpl->mnX = nX;
            mpWindowImpl->maPos.X() = nOrgX;
            mpWindowImpl->mnAbsScreenX = aPtDev.X();    // --- RTL --- (store real screen pos)
            bNewPos = true;
        }
    }
    if ( nFlags & PosSizeFlags::Y )
    {
        // check maPos as well, as it could have been changed for client windows (ImplCallMove())
        if ( nY != mpWindowImpl->mnY || nY != mpWindowImpl->maPos.Y() )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new vcl::Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, false, true, true );
            }
            mpWindowImpl->mnY = nY;
            mpWindowImpl->maPos.Y() = nY;
            bNewPos = true;
        }
    }

    if ( bNewPos || bNewSize )
    {
        bool bUpdateSysObjPos = false;
        if ( bNewPos )
            bUpdateSysObjPos = ImplUpdatePos();

        // the borderwindow always specifies the position for its client window
        if ( mpWindowImpl->mpBorderWindow )
            mpWindowImpl->maPos = mpWindowImpl->mpBorderWindow->mpWindowImpl->maPos;

        if ( mpWindowImpl->mpClientWindow )
        {
            mpWindowImpl->mpClientWindow->ImplPosSizeWindow( mpWindowImpl->mpClientWindow->mpWindowImpl->mnLeftBorder,
                                               mpWindowImpl->mpClientWindow->mpWindowImpl->mnTopBorder,
                                               mnOutWidth-mpWindowImpl->mpClientWindow->mpWindowImpl->mnLeftBorder-mpWindowImpl->mpClientWindow->mpWindowImpl->mnRightBorder,
                                               mnOutHeight-mpWindowImpl->mpClientWindow->mpWindowImpl->mnTopBorder-mpWindowImpl->mpClientWindow->mpWindowImpl->mnBottomBorder,
                                               PosSizeFlags::X | PosSizeFlags::Y |
                                               PosSizeFlags::Width | PosSizeFlags::Height );
            // If we have a client window, then this is the position
            // of the Application's floating windows
            mpWindowImpl->mpClientWindow->mpWindowImpl->maPos = mpWindowImpl->maPos;
            if ( bNewPos )
            {
                if ( mpWindowImpl->mpClientWindow->IsVisible() )
                {
                    mpWindowImpl->mpClientWindow->ImplCallMove();
                }
                else
                {
                    mpWindowImpl->mpClientWindow->mpWindowImpl->mbCallMove = true;
                }
            }
        }

        // Move()/Resize() will be called only for Show(), such that
        // at least one is called before Show()
        if ( IsVisible() )
        {
            if ( bNewPos )
            {
                ImplCallMove();
            }
            if ( bNewSize )
            {
                ImplCallResize();
            }
        }
        else
        {
            if ( bNewPos )
                mpWindowImpl->mbCallMove = true;
            if ( bNewSize )
                mpWindowImpl->mbCallResize = true;
        }

        bool bUpdateSysObjClip = false;
        if ( IsReallyVisible() )
        {
            if ( bNewPos || bNewSize )
            {
                // reset background storage
                if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                // set Clip-Flag
                bUpdateSysObjClip = !ImplSetClipFlag( true );
            }

            // invalidate window content ?
            if ( bNewPos || (mnOutWidth > nOldOutWidth) || (mnOutHeight > nOldOutHeight) )
            {
                if ( bNewPos )
                {
                    bool bInvalidate = false;
                    bool bParentPaint = true;
                    if ( !ImplIsOverlapWindow() )
                        bParentPaint = mpWindowImpl->mpParent->IsPaintEnabled();
                    if ( bCopyBits && bParentPaint && !HasPaintEvent() )
                    {
                        Point aPoint( mnOutOffX, mnOutOffY );
                        vcl::Region aRegion( Rectangle( aPoint,
                                                   Size( mnOutWidth, mnOutHeight ) ) );
                        if ( mpWindowImpl->mbWinRegion )
                            aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
                        ImplClipBoundaries( aRegion, false, true );
                        if ( !pOverlapRegion->IsEmpty() )
                        {
                            pOverlapRegion->Move( mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY );
                            aRegion.Exclude( *pOverlapRegion );
                        }
                        if ( !aRegion.IsEmpty() )
                        {
                            // adapt Paint areas
                            ImplMoveAllInvalidateRegions( Rectangle( Point( nOldOutOffX, nOldOutOffY ),
                                                                     Size( nOldOutWidth, nOldOutHeight ) ),
                                                          mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY,
                                                          true );
                            SalGraphics* pGraphics = ImplGetFrameGraphics();
                            if ( pGraphics )
                            {

                                OutputDevice *pOutDev = GetOutDev();
                                const bool bSelectClipRegion = pOutDev->SelectClipRegion( aRegion, pGraphics );
                                if ( bSelectClipRegion )
                                {
                                    pGraphics->CopyArea( mnOutOffX, mnOutOffY,
                                                         nOldOutOffX, nOldOutOffY,
                                                         nOldOutWidth, nOldOutHeight,
                                                         SAL_COPYAREA_WINDOWINVALIDATE, this );
                                }
                                else
                                    bInvalidate = true;
                            }
                            else
                                bInvalidate = true;
                            if ( !bInvalidate )
                            {
                                if ( !pOverlapRegion->IsEmpty() )
                                    ImplInvalidateFrameRegion( pOverlapRegion, InvalidateFlags::Children );
                            }
                        }
                        else
                            bInvalidate = true;
                    }
                    else
                        bInvalidate = true;
                    if ( bInvalidate )
                        ImplInvalidateFrameRegion( nullptr, InvalidateFlags::Children );
                }
                else
                {
                    Point aPoint( mnOutOffX, mnOutOffY );
                    vcl::Region aRegion( Rectangle( aPoint,
                                               Size( mnOutWidth, mnOutHeight ) ) );
                    aRegion.Exclude( *pOldRegion );
                    if ( mpWindowImpl->mbWinRegion )
                        aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
                    ImplClipBoundaries( aRegion, false, true );
                    if ( !aRegion.IsEmpty() )
                        ImplInvalidateFrameRegion( &aRegion, InvalidateFlags::Children );
                }
            }

            // invalidate Parent or Overlaps
            if ( bNewPos ||
                 (mnOutWidth < nOldOutWidth) || (mnOutHeight < nOldOutHeight) )
            {
                vcl::Region aRegion( *pOldRegion );
                if ( !mpWindowImpl->mbPaintTransparent )
                    ImplExcludeWindowRegion( aRegion );
                ImplClipBoundaries( aRegion, false, true );
                if ( !aRegion.IsEmpty() && !mpWindowImpl->mpBorderWindow )
                    ImplInvalidateParentFrameRegion( aRegion );
            }
        }

        // adapt system objects
        if ( bUpdateSysObjClip )
            ImplUpdateSysObjClip();
        if ( bUpdateSysObjPos )
            ImplUpdateSysObjPos();
        if ( bNewSize && mpWindowImpl->mpSysObj )
            mpWindowImpl->mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );
    }

    delete pOverlapRegion;
    delete pOldRegion;
}

void Window::ImplNewInputContext()
{
    ImplSVData* pSVData = ImplGetSVData();
    vcl::Window*     pFocusWin = pSVData->maWinData.mpFocusWin;
    if ( !pFocusWin )
        return;

    // Is InputContext changed?
    const InputContext& rInputContext = pFocusWin->GetInputContext();
    if ( rInputContext == pFocusWin->mpWindowImpl->mpFrameData->maOldInputContext )
        return;

    pFocusWin->mpWindowImpl->mpFrameData->maOldInputContext = rInputContext;

    SalInputContext         aNewContext;
    const vcl::Font&        rFont = rInputContext.GetFont();
    const OUString&         rFontName = rFont.GetFamilyName();
    ImplFontEntry*          pFontEntry = nullptr;
    aNewContext.mpFont = nullptr;
    if (!rFontName.isEmpty())
    {
        OutputDevice *pFocusWinOutDev = pFocusWin->GetOutDev();
        Size aSize = pFocusWinOutDev->ImplLogicToDevicePixel( rFont.GetSize() );
        if ( !aSize.Height() )
        {
            // only set default sizes if the font height in logical
            // coordinates equals 0
            if ( rFont.GetSize().Height() )
                aSize.Height() = 1;
            else
                aSize.Height() = (12*pFocusWin->mnDPIY)/72;
        }
        pFontEntry = pFocusWin->mpFontCache->GetFontEntry( pFocusWin->mpFontCollection,
                         rFont, aSize, static_cast<float>(aSize.Height()) );
        if ( pFontEntry )
            aNewContext.mpFont = &pFontEntry->maFontSelData;
    }
    aNewContext.meLanguage  = rFont.GetLanguage();
    aNewContext.mnOptions   = rInputContext.GetOptions();
    pFocusWin->ImplGetFrame()->SetInputContext( &aNewContext );

    if ( pFontEntry )
        pFocusWin->mpFontCache->Release( pFontEntry );
}

void Window::doLazyDelete()
{
    SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(this);
    DockingWindow* pDockWin = dynamic_cast<DockingWindow*>(this);
    if( pSysWin || ( pDockWin && pDockWin->IsFloatingMode() ) )
    {
        Show( false );
        SetParent( ImplGetDefaultWindow() );
    }
    vcl::LazyDeletor::Delete( this );
}

KeyIndicatorState Window::GetIndicatorState() const
{
    return mpWindowImpl->mpFrame->GetIndicatorState();
}

void Window::SimulateKeyPress( sal_uInt16 nKeyCode ) const
{
    mpWindowImpl->mpFrame->SimulateKeyPress(nKeyCode);
}

void Window::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode cod = rKEvt.GetKeyCode ();
    bool accel = ImplGetSVData()->maNWFData.mbEnableAccel;
    bool autoacc = ImplGetSVData()->maNWFData.mbAutoAccel;

    // do not respond to accelerators unless Alt is held */
    if (cod.GetCode () >= 0x200 && cod.GetCode () <= 0x219)
    {
        if (!accel) return;
        if (autoacc && cod.GetModifier () != 0x4000) return;
    }

    NotifyEvent aNEvt( MouseNotifyEvent::KEYINPUT, this, &rKEvt );
    if ( !CompatNotify( aNEvt ) )
        mpWindowImpl->mbKeyInput = true;
}

void Window::KeyUp( const KeyEvent& rKEvt )
{
    NotifyEvent aNEvt( MouseNotifyEvent::KEYUP, this, &rKEvt );
    if ( !CompatNotify( aNEvt ) )
        mpWindowImpl->mbKeyUp = true;
}

void Window::Draw( OutputDevice*, const Point&, const Size&, DrawFlags )
{
}

void Window::Move() {}

void Window::Resize() {}

void Window::Activate() {}

void Window::Deactivate() {}

void Window::GetFocus()
{
    if ( HasFocus() && mpWindowImpl->mpLastFocusWindow && !(mpWindowImpl->mnDlgCtrlFlags & DialogControlFlags::WantFocus) )
    {
        ImplDelData aDogtag( this );
        mpWindowImpl->mpLastFocusWindow->GrabFocus();
        if( aDogtag.IsDead() )
            return;
    }

    NotifyEvent aNEvt( MouseNotifyEvent::GETFOCUS, this );
    CompatNotify( aNEvt );
}

void Window::LoseFocus()
{
    NotifyEvent aNEvt( MouseNotifyEvent::LOSEFOCUS, this );
    CompatNotify( aNEvt );
}

void Window::RequestHelp( const HelpEvent& rHEvt )
{
    // if Balloon-Help is requested, show the balloon
    // with help text set
    if ( rHEvt.GetMode() & HelpEventMode::BALLOON )
    {
        OUString rStr = GetHelpText();
        if ( rStr.isEmpty() )
            rStr = GetQuickHelpText();
        if ( rStr.isEmpty() && ImplGetParent() && !ImplIsOverlapWindow() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            Point aPos = GetPosPixel();
            if ( ImplGetParent() && !ImplIsOverlapWindow() )
                aPos = ImplGetParent()->OutputToScreenPixel( aPos );
            Rectangle   aRect( aPos, GetSizePixel() );

            Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), aRect, rStr );
        }
    }
    else if ( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        const OUString& rStr = GetQuickHelpText();
        if ( rStr.isEmpty() && ImplGetParent() && !ImplIsOverlapWindow() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            Point aPos = GetPosPixel();
            if ( ImplGetParent() && !ImplIsOverlapWindow() )
                aPos = ImplGetParent()->OutputToScreenPixel( aPos );
            Rectangle   aRect( aPos, GetSizePixel() );
            OUString      aHelpText;
            if ( !rStr.isEmpty() )
                aHelpText = GetHelpText();
            Help::ShowQuickHelp( this, aRect, rStr, aHelpText, QuickHelpFlags::CtrlText );
        }
    }
    else
    {
        OUString aStrHelpId( OStringToOUString( GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
        if ( aStrHelpId.isEmpty() && ImplGetParent() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                if( !aStrHelpId.isEmpty() )
                    pHelp->Start( aStrHelpId, this );
                else
                    pHelp->Start( OOO_HELP_INDEX, this );
            }
        }
    }
}

void Window::Command( const CommandEvent& rCEvt )
{
    CallEventListeners( VCLEVENT_WINDOW_COMMAND, const_cast<CommandEvent *>(&rCEvt) );

    NotifyEvent aNEvt( MouseNotifyEvent::COMMAND, this, &rCEvt );
    if ( !CompatNotify( aNEvt ) )
        mpWindowImpl->mbCommand = true;
}

void Window::Tracking( const TrackingEvent& rTEvt )
{

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
        pWrapper->Tracking( rTEvt );
}

void Window::StateChanged(StateChangedType eType)
{
    switch (eType)
    {
        //stuff that doesn't invalidate the layout
        case StateChangedType::ControlForeground:
        case StateChangedType::ControlBackground:
        case StateChangedType::Transparent:
        case StateChangedType::UpdateMode:
        case StateChangedType::ReadOnly:
        case StateChangedType::Enable:
        case StateChangedType::State:
        case StateChangedType::Data:
        case StateChangedType::InitShow:
        case StateChangedType::ControlFocus:
            break;
        //stuff that does invalidate the layout
        default:
            queue_resize(eType);
            break;
    }
}

bool Window::IsLocked( bool bChildren ) const
{
    if ( mpWindowImpl->mnLockCount != 0 )
        return true;

    if ( bChildren || mpWindowImpl->mbChildNotify )
    {
        vcl::Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            if ( pChild->IsLocked( true ) )
                return true;
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    return false;
}

void Window::SetStyle( WinBits nStyle )
{
    if ( mpWindowImpl && mpWindowImpl->mnStyle != nStyle )
    {
        mpWindowImpl->mnPrevStyle = mpWindowImpl->mnStyle;
        mpWindowImpl->mnStyle = nStyle;
        CompatStateChanged( StateChangedType::Style );
    }
}

void Window::SetExtendedStyle( WinBits nExtendedStyle )
{

    if ( mpWindowImpl->mnExtendedStyle != nExtendedStyle )
    {
        vcl::Window* pWindow = ImplGetBorderWindow();
        if( ! pWindow )
            pWindow = this;
        if( pWindow->mpWindowImpl->mbFrame )
        {
            SalExtStyle nExt = 0;
            if( (nExtendedStyle & WB_EXT_DOCUMENT) )
                nExt |= SAL_FRAME_EXT_STYLE_DOCUMENT;
            if( (nExtendedStyle & WB_EXT_DOCMODIFIED) )
                nExt |= SAL_FRAME_EXT_STYLE_DOCMODIFIED;

            pWindow->ImplGetFrame()->SetExtendedFrameStyle( nExt );
        }
        mpWindowImpl->mnPrevExtendedStyle = mpWindowImpl->mnExtendedStyle;
        mpWindowImpl->mnExtendedStyle = nExtendedStyle;
        CompatStateChanged( StateChangedType::ExtendedStyle );
    }
}

void Window::SetBorderStyle( WindowBorderStyle nBorderStyle )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        if( nBorderStyle == WindowBorderStyle::REMOVEBORDER &&
            ! mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame &&
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mpParent
            )
        {
            // this is a little awkward: some controls (e.g. svtools ProgressBar)
            // cannot avoid getting constructed with WB_BORDER but want to disable
            // borders in case of NWF drawing. So they need a method to remove their border window
            VclPtr<vcl::Window> pBorderWin = mpWindowImpl->mpBorderWindow;
            // remove us as border window's client
            pBorderWin->mpWindowImpl->mpClientWindow = nullptr;
            mpWindowImpl->mpBorderWindow = nullptr;
            mpWindowImpl->mpRealParent = pBorderWin->mpWindowImpl->mpParent;
            // reparent us above the border window
            SetParent( pBorderWin->mpWindowImpl->mpParent );
            // set us to the position and size of our previous border
            Point aBorderPos( pBorderWin->GetPosPixel() );
            Size aBorderSize( pBorderWin->GetSizePixel() );
            setPosSizePixel( aBorderPos.X(), aBorderPos.Y(), aBorderSize.Width(), aBorderSize.Height() );
            // release border window
            pBorderWin.disposeAndClear();

            // set new style bits
            SetStyle( GetStyle() & (~WB_BORDER) );
        }
        else
        {
            if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
                static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->SetBorderStyle( nBorderStyle );
            else
                mpWindowImpl->mpBorderWindow->SetBorderStyle( nBorderStyle );
        }
    }
}

WindowBorderStyle Window::GetBorderStyle() const
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->GetBorderStyle();
        else
            return mpWindowImpl->mpBorderWindow->GetBorderStyle();
    }

    return WindowBorderStyle::NONE;
}

long Window::CalcTitleWidth() const
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->CalcTitleWidth();
        else
            return mpWindowImpl->mpBorderWindow->CalcTitleWidth();
    }
    else if ( mpWindowImpl->mbFrame && (mpWindowImpl->mnStyle & WB_MOVEABLE) )
    {
        // we guess the width for frame windows as we do not know the
        // border of external dialogs
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        vcl::Font aFont = GetFont();
        const_cast<vcl::Window*>(this)->SetPointFont(*const_cast<Window*>(this), rStyleSettings.GetTitleFont());
        long nTitleWidth = GetTextWidth( GetText() );
        const_cast<vcl::Window*>(this)->SetFont( aFont );
        nTitleWidth += rStyleSettings.GetTitleHeight() * 3;
        nTitleWidth += rStyleSettings.GetBorderSize() * 2;
        nTitleWidth += 10;
        return nTitleWidth;
    }

    return 0;
}

void Window::SetInputContext( const InputContext& rInputContext )
{

    mpWindowImpl->maInputContext = rInputContext;
    if ( !mpWindowImpl->mbInFocusHdl && HasFocus() )
        ImplNewInputContext();
}

void Window::EndExtTextInput( EndExtTextInputFlags nFlags )
{

    if ( mpWindowImpl->mbExtTextInput )
        ImplGetFrame()->EndExtTextInput( nFlags );
}

void Window::SetCursorRect( const Rectangle* pRect, long nExtTextInputWidth )
{

    ImplWinData* pWinData = ImplGetWinData();
    if ( pWinData->mpCursorRect )
    {
        if ( pRect )
            *pWinData->mpCursorRect = *pRect;
        else
        {
            delete pWinData->mpCursorRect;
            pWinData->mpCursorRect = nullptr;
        }
    }
    else
    {
        if ( pRect )
            pWinData->mpCursorRect = new Rectangle( *pRect );
    }

    pWinData->mnCursorExtWidth = nExtTextInputWidth;

}

const Rectangle* Window::GetCursorRect() const
{

    ImplWinData* pWinData = ImplGetWinData();
    return pWinData->mpCursorRect;
}

long Window::GetCursorExtTextInputWidth() const
{

    ImplWinData* pWinData = ImplGetWinData();
    return pWinData->mnCursorExtWidth;
}

void Window::SetCompositionCharRect( const Rectangle* pRect, long nCompositionLength, bool bVertical ) {

    ImplWinData* pWinData = ImplGetWinData();
    delete[] pWinData->mpCompositionCharRects;
    pWinData->mbVertical = bVertical;
    pWinData->mpCompositionCharRects = nullptr;
    pWinData->mnCompositionCharRects = nCompositionLength;
    if ( pRect && (nCompositionLength > 0) )
    {
        pWinData->mpCompositionCharRects = new Rectangle[nCompositionLength];
        for (long i = 0; i < nCompositionLength; ++i)
            pWinData->mpCompositionCharRects[i] = pRect[i];
    }
}

void Window::CollectChildren(::std::vector<vcl::Window *>& rAllChildren )
{
    rAllChildren.push_back( this );

    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->CollectChildren( rAllChildren );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

void Window::SetPointFont(vcl::RenderContext& rRenderContext, const vcl::Font& rFont)
{
    vcl::Font aFont = rFont;
    ImplPointToLogic(rRenderContext, aFont);
    rRenderContext.SetFont(aFont);
}

vcl::Font Window::GetPointFont(vcl::RenderContext& rRenderContext) const
{
    vcl::Font aFont = rRenderContext.GetFont();
    ImplLogicToPoint(rRenderContext, aFont);
    return aFont;
}

void Window::Show(bool bVisible, ShowFlags nFlags)
{
    if ( IsDisposed() || mpWindowImpl->mbVisible == bVisible )
        return;

    ImplDelData aDogTag( this );

    bool bRealVisibilityChanged = false;
    mpWindowImpl->mbVisible = bVisible;

    if ( !bVisible )
    {
        ImplHideAllOverlaps();
        if( aDogTag.IsDead() )
            return;

        if ( mpWindowImpl->mpBorderWindow )
        {
            bool bOldUpdate = mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate;
            if ( mpWindowImpl->mbNoParentUpdate )
                mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate = true;
            mpWindowImpl->mpBorderWindow->Show( false, nFlags );
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate = bOldUpdate;
        }
        else if ( mpWindowImpl->mbFrame )
        {
            mpWindowImpl->mbSuppressAccessibilityEvents = true;
            mpWindowImpl->mpFrame->Show( false );
        }

        CompatStateChanged( StateChangedType::Visible );

        if ( mpWindowImpl->mbReallyVisible )
        {
            vcl::Region  aInvRegion;
            bool    bSaveBack = false;

            if ( ImplIsOverlapWindow() && !mpWindowImpl->mbFrame )
            {
                if ( ImplRestoreOverlapBackground( aInvRegion ) )
                    bSaveBack = true;
            }

            if ( !bSaveBack )
            {
                if ( mpWindowImpl->mbInitWinClipRegion )
                    ImplInitWinClipRegion();
                aInvRegion = mpWindowImpl->maWinClipRegion;
            }

            if( aDogTag.IsDead() )
                return;

            bRealVisibilityChanged = mpWindowImpl->mbReallyVisible;
            ImplResetReallyVisible();
            ImplSetClipFlag();

            if ( ImplIsOverlapWindow() && !mpWindowImpl->mbFrame )
            {
                // convert focus
                if ( !(nFlags & ShowFlags::NoFocusChange) && HasChildPathFocus() )
                {
                    if ( mpWindowImpl->mpOverlapWindow->IsEnabled() &&
                         mpWindowImpl->mpOverlapWindow->IsInputEnabled() &&
                         ! mpWindowImpl->mpOverlapWindow->IsInModalMode()
                         )
                        mpWindowImpl->mpOverlapWindow->GrabFocus();
                }
            }

            if ( !mpWindowImpl->mbFrame )
            {
                if( mpWindowImpl->mpWinData && mpWindowImpl->mpWinData->mbEnableNativeWidget )
                {
                    /*
                    * #i48371# native theming: some themes draw outside the control
                    * area we tell them to (bad thing, but we cannot do much about it ).
                    * On hiding these controls they get invalidated with their window rectangle
                    * which leads to the parts outside the control area being left and not
                    * invalidated. Workaround: invalidate an area on the parent, too
                    */
                    const int workaround_border = 5;
                    Rectangle aBounds( aInvRegion.GetBoundRect() );
                    aBounds.Left()      -= workaround_border;
                    aBounds.Top()       -= workaround_border;
                    aBounds.Right()     += workaround_border;
                    aBounds.Bottom()    += workaround_border;
                    aInvRegion = aBounds;
                }
                if ( !mpWindowImpl->mbNoParentUpdate && !(nFlags & ShowFlags::NoParentUpdate) )
                {
                    if ( !aInvRegion.IsEmpty() )
                        ImplInvalidateParentFrameRegion( aInvRegion );
                }
                ImplGenerateMouseMove();
            }
        }
    }
    else
    {
        // inherit native widget flag for form controls
        // required here, because frames never show up in the child hierarchy - which should be fixed....
        // eg, the drop down of a combobox which is a system floating window
        if( mpWindowImpl->mbFrame && GetParent() && GetParent()->IsCompoundControl() &&
            GetParent()->IsNativeWidgetEnabled() != IsNativeWidgetEnabled() )
        {
            EnableNativeWidget( GetParent()->IsNativeWidgetEnabled() );
        }

        if ( mpWindowImpl->mbCallMove )
        {
            ImplCallMove();
        }
        if ( mpWindowImpl->mbCallResize )
        {
            ImplCallResize();
        }

        CompatStateChanged( StateChangedType::Visible );

        vcl::Window* pTestParent;
        if ( ImplIsOverlapWindow() )
            pTestParent = mpWindowImpl->mpOverlapWindow;
        else
            pTestParent = ImplGetParent();
        if ( mpWindowImpl->mbFrame || pTestParent->mpWindowImpl->mbReallyVisible )
        {
            // if a window becomes visible, send all child windows a StateChange,
            // such that these can initialise themselves
            ImplCallInitShow();

            // If it is a SystemWindow it automatically pops up on top of
            // all other windows if needed.
            if ( ImplIsOverlapWindow() && !(nFlags & ShowFlags::NoActivate) )
            {
                ImplStartToTop(( nFlags & ShowFlags::ForegroundTask ) ? ToTopFlags::ForegroundTask : ToTopFlags::NONE );
                ImplFocusToTop( ToTopFlags::NONE, false );
            }

            // save background
            if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mbSaveBack )
                ImplSaveOverlapBackground();
            // adjust mpWindowImpl->mbReallyVisible
            bRealVisibilityChanged = !mpWindowImpl->mbReallyVisible;
            ImplSetReallyVisible();

            // assure clip rectangles will be recalculated
            ImplSetClipFlag();

            if ( !mpWindowImpl->mbFrame )
            {
                InvalidateFlags nInvalidateFlags = InvalidateFlags::Children;
                if( ! IsPaintTransparent() )
                    nInvalidateFlags |= InvalidateFlags::NoTransparent;
                ImplInvalidate( nullptr, nInvalidateFlags );
                ImplGenerateMouseMove();
            }
        }

        if ( mpWindowImpl->mpBorderWindow )
            mpWindowImpl->mpBorderWindow->Show( true, nFlags );
        else if ( mpWindowImpl->mbFrame )
        {
            // #106431#, hide SplashScreen
            ImplSVData* pSVData = ImplGetSVData();
            if ( !pSVData->mpIntroWindow )
            {
                // The right way would be just to call this (not even in the 'if')
                GetpApp()->InitFinished();
            }
            else if ( !ImplIsWindowOrChild( pSVData->mpIntroWindow ) )
            {
                // ... but the VCL splash is broken, and it needs this
                // (for ./soffice .uno:NewDoc)
                pSVData->mpIntroWindow->Hide();
            }

            //DBG_ASSERT( !mpWindowImpl->mbSuppressAccessibilityEvents, "Window::Show() - Frame reactivated");
            mpWindowImpl->mbSuppressAccessibilityEvents = false;

            mpWindowImpl->mbPaintFrame = true;
            if (!Application::GetSettings().GetMiscSettings().GetPseudoHeadless())
            {
                bool bNoActivate(nFlags & (ShowFlags::NoActivate|ShowFlags::NoFocusChange));
                mpWindowImpl->mpFrame->Show( true, bNoActivate );
            }
            if( aDogTag.IsDead() )
                return;

            // Query the correct size of the window, if we are waiting for
            // a system resize
            if ( mpWindowImpl->mbWaitSystemResize )
            {
                long nOutWidth;
                long nOutHeight;
                mpWindowImpl->mpFrame->GetClientSize( nOutWidth, nOutHeight );
                ImplHandleResize( this, nOutWidth, nOutHeight );
            }

            if (mpWindowImpl->mpFrameData->mpBuffer && mpWindowImpl->mpFrameData->mpBuffer->GetOutputSizePixel() != GetOutputSizePixel())
                // Make sure that the buffer size matches the window size, even if no resize was needed.
                mpWindowImpl->mpFrameData->mpBuffer->SetOutputSizePixel(GetOutputSizePixel());
        }

        if( aDogTag.IsDead() )
            return;

        ImplShowAllOverlaps();
    }

    if( aDogTag.IsDead() )
        return;
    // invalidate all saved backgrounds
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    // the SHOW/HIDE events also serve as indicators to send child creation/destroy events to the access bridge
    // However, the access bridge only uses this event if the data member is not NULL (it's kind of a hack that
    // we re-use the SHOW/HIDE events this way, with this particular semantics).
    // Since #104887#, the notifications for the access bridge are done in Impl(Set|Reset)ReallyVisible. Here, we
    // now only notify with a NULL data pointer, for all other clients except the access bridge.
    if ( !bRealVisibilityChanged )
        CallEventListeners( mpWindowImpl->mbVisible ? VCLEVENT_WINDOW_SHOW : VCLEVENT_WINDOW_HIDE );
    if( aDogTag.IsDead() )
        return;

}

Size Window::GetSizePixel() const
{
    if (!mpWindowImpl)
    {
        SAL_WARN("vcl.layout", "WTF no windowimpl");
        return Size(0,0);
    }

    // #i43257# trigger pending resize handler to assure correct window sizes
    if( mpWindowImpl->mpFrameData->maResizeIdle.IsActive() )
    {
        ImplDelData aDogtag( const_cast<Window*>(this) );
        mpWindowImpl->mpFrameData->maResizeIdle.Stop();
        mpWindowImpl->mpFrameData->maResizeIdle.GetIdleHdl().Call( nullptr );
        if( aDogtag.IsDead() )
            return Size(0,0);
    }

    return Size( mnOutWidth+mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder,
                 mnOutHeight+mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder );
}

void Window::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                               sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    rLeftBorder     = mpWindowImpl->mnLeftBorder;
    rTopBorder      = mpWindowImpl->mnTopBorder;
    rRightBorder    = mpWindowImpl->mnRightBorder;
    rBottomBorder   = mpWindowImpl->mnBottomBorder;
}

void Window::Enable( bool bEnable, bool bChild )
{
    if ( IsDisposed() )
        return;

    if ( !bEnable )
    {
        // the tracking mode will be stopped or the capture will be stolen
        // when a window is disabled,
        if ( IsTracking() )
            EndTracking( TrackingEventFlags::Cancel );
        if ( IsMouseCaptured() )
            ReleaseMouse();
        // try to pass focus to the next control
        // if the window has focus and is contained in the dialog control
        // mpWindowImpl->mbDisabled should only be set after a call of ImplDlgCtrlNextWindow().
        // Otherwise ImplDlgCtrlNextWindow() should be used
        if ( HasFocus() )
            ImplDlgCtrlNextWindow();
    }

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->Enable( bEnable, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->mpMenuBarWindow )
            static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->mpMenuBarWindow->Enable( bEnable );
    }

    // #i56102# restore app focus win in case the
    // window was disabled when the frame focus changed
    ImplSVData* pSVData = ImplGetSVData();
    if( bEnable &&
        pSVData->maWinData.mpFocusWin == nullptr &&
        mpWindowImpl->mpFrameData->mbHasFocus &&
        mpWindowImpl->mpFrameData->mpFocusWin == this )
        pSVData->maWinData.mpFocusWin = this;

    if ( mpWindowImpl->mbDisabled != !bEnable )
    {
        mpWindowImpl->mbDisabled = !bEnable;
        if ( mpWindowImpl->mpSysObj )
            mpWindowImpl->mpSysObj->Enable( bEnable && !mpWindowImpl->mbInputDisabled );
        CompatStateChanged( StateChangedType::Enable );

        CallEventListeners( bEnable ? VCLEVENT_WINDOW_ENABLED : VCLEVENT_WINDOW_DISABLED );
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        vcl::Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->Enable( bEnable, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    if ( IsReallyVisible() )
        ImplGenerateMouseMove();
}

void Window::SetCallHandlersOnInputDisabled( bool bCall )
{
    mpWindowImpl->mbCallHandlersDuringInputDisabled = bCall;

    vcl::Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->SetCallHandlersOnInputDisabled( bCall );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

bool Window::IsCallHandlersOnInputDisabled() const
{
    return mpWindowImpl->mbCallHandlersDuringInputDisabled;
}

void Window::EnableInput( bool bEnable, bool bChild )
{

    bool bNotify = (bEnable != mpWindowImpl->mbInputDisabled);
    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->EnableInput( bEnable, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->mpMenuBarWindow )
            static_cast<ImplBorderWindow*>(mpWindowImpl->mpBorderWindow.get())->mpMenuBarWindow->EnableInput( bEnable );
    }

    if ( (! bEnable && mpWindowImpl->meAlwaysInputMode != AlwaysInputEnabled) ||
         (  bEnable && mpWindowImpl->meAlwaysInputMode != AlwaysInputDisabled) )
    {
        // automatically stop the tracking mode or steal capture
        // if the window is disabled
        if ( !bEnable )
        {
            if ( IsTracking() )
                EndTracking( TrackingEventFlags::Cancel );
            if ( IsMouseCaptured() )
                ReleaseMouse();
        }

        if ( mpWindowImpl->mbInputDisabled != !bEnable )
        {
            mpWindowImpl->mbInputDisabled = !bEnable;
            if ( mpWindowImpl->mpSysObj )
                mpWindowImpl->mpSysObj->Enable( !mpWindowImpl->mbDisabled && bEnable );
        }
    }

    // #i56102# restore app focus win in case the
    // window was disabled when the frame focus changed
    ImplSVData* pSVData = ImplGetSVData();
    if( bEnable &&
        pSVData->maWinData.mpFocusWin == nullptr &&
        mpWindowImpl->mpFrameData->mbHasFocus &&
        mpWindowImpl->mpFrameData->mpFocusWin == this )
        pSVData->maWinData.mpFocusWin = this;

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        vcl::Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->EnableInput( bEnable, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    if ( IsReallyVisible() )
        ImplGenerateMouseMove();

    // #104827# notify parent
    if ( bNotify )
    {
        NotifyEvent aNEvt( bEnable ? MouseNotifyEvent::INPUTENABLE : MouseNotifyEvent::INPUTDISABLE, this );
        CompatNotify( aNEvt );
    }
}

void Window::EnableInput( bool bEnable, bool bChild, bool bSysWin,
                          const vcl::Window* pExcludeWindow )
{

    EnableInput( bEnable, bChild );
    if ( bSysWin )
    {
        // pExcuteWindow is the first Overlap-Frame --> if this
        // shouldn't be the case, than this must be changed in dialog.cxx
        if( pExcludeWindow )
            pExcludeWindow = pExcludeWindow->ImplGetFirstOverlapWindow();
        vcl::Window* pSysWin = mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            // Is Window in the path from this window
            if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pSysWin, true ) )
            {
                // Is Window not in the exclude window path or not the
                // exclude window, than change the status
                if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( pSysWin, true ) )
                    pSysWin->EnableInput( bEnable, bChild );
            }
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        // enable/disable floating system windows as well
        vcl::Window* pFrameWin = ImplGetSVData()->maWinData.mpFirstFrame;
        while ( pFrameWin )
        {
            if( pFrameWin->ImplIsFloatingWindow() )
            {
                // Is Window in the path from this window
                if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pFrameWin, true ) )
                {
                    // Is Window not in the exclude window path or not the
                    // exclude window, than change the status
                    if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( pFrameWin, true ) )
                        pFrameWin->EnableInput( bEnable, bChild );
                }
            }
            pFrameWin = pFrameWin->mpWindowImpl->mpFrameData->mpNextFrame;
        }

        // the same for ownerdraw floating windows
        if( mpWindowImpl->mbFrame )
        {
            ::std::vector< VclPtr<vcl::Window> >& rList = mpWindowImpl->mpFrameData->maOwnerDrawList;
            auto p = rList.begin();
            while( p != rList.end() )
            {
                // Is Window in the path from this window
                if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( (*p), true ) )
                {
                    // Is Window not in the exclude window path or not the
                    // exclude window, than change the status
                    if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( (*p), true ) )
                        (*p)->EnableInput( bEnable, bChild );
                }
                ++p;
            }
        }
    }
}

void Window::AlwaysEnableInput( bool bAlways, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->AlwaysEnableInput( bAlways, false );

    if( bAlways && mpWindowImpl->meAlwaysInputMode != AlwaysInputEnabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputEnabled;

        if ( bAlways )
            EnableInput( true, false );
    }
    else if( ! bAlways && mpWindowImpl->meAlwaysInputMode == AlwaysInputEnabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputNone;
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        vcl::Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->AlwaysEnableInput( bAlways, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::AlwaysDisableInput( bool bAlways, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->AlwaysDisableInput( bAlways, false );

    if( bAlways && mpWindowImpl->meAlwaysInputMode != AlwaysInputDisabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputDisabled;

        if ( bAlways )
            EnableInput( false, false );
    }
    else if( ! bAlways && mpWindowImpl->meAlwaysInputMode == AlwaysInputDisabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputNone;
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        vcl::Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->AlwaysDisableInput( bAlways, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::SetActivateMode( ActivateModeFlags nMode )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetActivateMode( nMode );

    if ( mpWindowImpl->mnActivateMode != nMode )
    {
        mpWindowImpl->mnActivateMode = nMode;

        // possibly trigger Decativate/Activate
        if ( mpWindowImpl->mnActivateMode != ActivateModeFlags::NONE )
        {
            if ( (mpWindowImpl->mbActive || (GetType() == WINDOW_BORDERWINDOW)) &&
                 !HasChildPathFocus( true ) )
            {
                mpWindowImpl->mbActive = false;
                Deactivate();
            }
        }
        else
        {
            if ( !mpWindowImpl->mbActive || (GetType() == WINDOW_BORDERWINDOW) )
            {
                mpWindowImpl->mbActive = true;
                Activate();
            }
        }
    }
}

void Window::setPosSizePixel( long nX, long nY,
                              long nWidth, long nHeight, PosSizeFlags nFlags )
{
    bool bHasValidSize = !mpWindowImpl->mbDefSize;

    if ( nFlags & PosSizeFlags::Pos )
        mpWindowImpl->mbDefPos = false;
    if ( nFlags & PosSizeFlags::Size )
        mpWindowImpl->mbDefSize = false;

    // The top BorderWindow is the window which is to be positioned
    vcl::Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;

    if ( pWindow->mpWindowImpl->mbFrame )
    {
        // Note: if we're positioning a frame, the coordinates are interpreted
        // as being the top-left corner of the window's client area and NOT
        // as the position of the border ! (due to limitations of several UNIX window managers)
        long nOldWidth  = pWindow->mnOutWidth;

        if ( !(nFlags & PosSizeFlags::Width) )
            nWidth = pWindow->mnOutWidth;
        if ( !(nFlags & PosSizeFlags::Height) )
            nHeight = pWindow->mnOutHeight;

        sal_uInt16 nSysFlags=0;
        vcl::Window *pParent = GetParent();

        if( nFlags & PosSizeFlags::Width )
            nSysFlags |= SAL_FRAME_POSSIZE_WIDTH;
        if( nFlags & PosSizeFlags::Height )
            nSysFlags |= SAL_FRAME_POSSIZE_HEIGHT;
        if( nFlags & PosSizeFlags::ByDrag )
            nSysFlags |= SAL_FRAME_POSSIZE_BYDRAG;
        if( nFlags & PosSizeFlags::X )
        {
            nSysFlags |= SAL_FRAME_POSSIZE_X;
            if( pParent && (pWindow->GetStyle() & WB_SYSTEMCHILDWINDOW) )
            {
                nX += pParent->mnOutOffX;
            }
            if( pParent && pParent->ImplIsAntiparallel() )
            {
                // --- RTL --- (re-mirror at parent window)
                Rectangle aRect( Point ( nX, nY ), Size( nWidth, nHeight ) );
                const OutputDevice *pParentOutDev = pParent->GetOutDev();
                pParentOutDev->ReMirror( aRect );
                nX = aRect.Left();
            }
        }
        if( !(nFlags & PosSizeFlags::X) && bHasValidSize && pWindow->mpWindowImpl->mpFrame->maGeometry.nWidth )
        {
            // --- RTL ---  make sure the old right aligned position is not changed
            //              system windows will always grow to the right
            if ( pParent )
            {
                OutputDevice *pParentOutDev = pParent->GetOutDev();
                if( pParentOutDev->HasMirroredGraphics() )
                {
                    long myWidth = nOldWidth;
                    if( !myWidth )
                        myWidth = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nWidth;
                    if( !myWidth )
                        myWidth = nWidth;
                    nFlags |= PosSizeFlags::X;
                    nSysFlags |= SAL_FRAME_POSSIZE_X;
                    nX = pParent->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX - mpWindowImpl->mpFrame->GetUnmirroredGeometry().nLeftDecoration +
                        pParent->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nWidth - myWidth - 1 - mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX;
                    if(!(nFlags & PosSizeFlags::Y))
                    {
                        nFlags |= PosSizeFlags::Y;
                        nSysFlags |= SAL_FRAME_POSSIZE_Y;
                        nY = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nY - pWindow->GetParent()->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nY -
                            mpWindowImpl->mpFrame->GetUnmirroredGeometry().nTopDecoration;
                    }
                }
            }
        }
        if( nFlags & PosSizeFlags::Y )
        {
            nSysFlags |= SAL_FRAME_POSSIZE_Y;
            if( pParent && (pWindow->GetStyle() & WB_SYSTEMCHILDWINDOW) )
            {
                nY += pParent->mnOutOffY;
            }
        }

        if( nSysFlags & (SAL_FRAME_POSSIZE_WIDTH|SAL_FRAME_POSSIZE_HEIGHT) )
        {
            // check for min/max client size and adjust size accordingly
            // otherwise it may happen that the resize event is ignored, i.e. the old size remains
            // unchanged but ImplHandleResize() is called with the wrong size
            SystemWindow *pSystemWindow = dynamic_cast< SystemWindow* >( pWindow );
            if( pSystemWindow )
            {
                Size aMinSize = pSystemWindow->GetMinOutputSizePixel();
                Size aMaxSize = pSystemWindow->GetMaxOutputSizePixel();
                if( nWidth < aMinSize.Width() )
                    nWidth = aMinSize.Width();
                if( nHeight < aMinSize.Height() )
                    nHeight = aMinSize.Height();

                if( nWidth > aMaxSize.Width() )
                    nWidth = aMaxSize.Width();
                if( nHeight > aMaxSize.Height() )
                    nHeight = aMaxSize.Height();
            }
        }

        pWindow->mpWindowImpl->mpFrame->SetPosSize( nX, nY, nWidth, nHeight, nSysFlags );

        // Resize should be called directly. If we haven't
        // set the correct size, we get a second resize from
        // the system with the correct size. This can be happened
        // if the size is to small or to large.
        ImplHandleResize( pWindow, nWidth, nHeight );
    }
    else
    {
        pWindow->ImplPosSizeWindow( nX, nY, nWidth, nHeight, nFlags );
        if ( IsReallyVisible() )
            ImplGenerateMouseMove();
    }
}

Point Window::GetPosPixel() const
{
    return mpWindowImpl->maPos;
}

Rectangle Window::GetDesktopRectPixel() const
{
    Rectangle rRect;
    mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrame->GetWorkArea( rRect );
    return rRect;
}

Point Window::OutputToScreenPixel( const Point& rPos ) const
{
    // relative to top level parent
    return Point( rPos.X()+mnOutOffX, rPos.Y()+mnOutOffY );
}

Point Window::ScreenToOutputPixel( const Point& rPos ) const
{
    // relative to top level parent
    return Point( rPos.X()-mnOutOffX, rPos.Y()-mnOutOffY );
}

long Window::ImplGetUnmirroredOutOffX()
{
    // revert mnOutOffX changes that were potentially made in ImplPosSizeWindow
    long offx = mnOutOffX;
    OutputDevice *pOutDev = GetOutDev();
    if( pOutDev->HasMirroredGraphics() )
    {
        if( mpWindowImpl->mpParent && !mpWindowImpl->mpParent->mpWindowImpl->mbFrame && mpWindowImpl->mpParent->ImplIsAntiparallel() )
        {
            if ( !ImplIsOverlapWindow() )
                offx -= mpWindowImpl->mpParent->mnOutOffX;

            offx = mpWindowImpl->mpParent->mnOutWidth - mnOutWidth - offx;

            if ( !ImplIsOverlapWindow() )
                offx += mpWindowImpl->mpParent->mnOutOffX;

        }
    }
    return offx;
}

// normalized screen pixel are independent of mirroring
Point Window::OutputToNormalizedScreenPixel( const Point& rPos ) const
{
    // relative to top level parent
    long offx = const_cast<vcl::Window*>(this)->ImplGetUnmirroredOutOffX();
    return Point( rPos.X()+offx, rPos.Y()+mnOutOffY );
}

Point Window::NormalizedScreenToOutputPixel( const Point& rPos ) const
{
    // relative to top level parent
    long offx = const_cast<vcl::Window*>(this)->ImplGetUnmirroredOutOffX();
    return Point( rPos.X()-offx, rPos.Y()-mnOutOffY );
}

Point Window::OutputToAbsoluteScreenPixel( const Point& rPos ) const
{
    // relative to the screen
    Point p = OutputToScreenPixel( rPos );
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    p.X() += g.nX;
    p.Y() += g.nY;
    return p;
}

Point Window::AbsoluteScreenToOutputPixel( const Point& rPos ) const
{
    // relative to the screen
    Point p = ScreenToOutputPixel( rPos );
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    p.X() -= g.nX;
    p.Y() -= g.nY;
    return p;
}

Rectangle Window::ImplOutputToUnmirroredAbsoluteScreenPixel( const Rectangle &rRect ) const
{
    // this method creates unmirrored screen coordinates to be compared with the desktop
    // and is used for positioning of RTL popup windows correctly on the screen
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetUnmirroredGeometry();

    Point p1 = OutputToScreenPixel( rRect.TopRight() );
    p1.X() = g.nX+g.nWidth-p1.X();
    p1.Y() += g.nY;

    Point p2 = OutputToScreenPixel( rRect.BottomLeft() );
    p2.X() = g.nX+g.nWidth-p2.X();
    p2.Y() += g.nY;

    return Rectangle( p1, p2 );
}

Rectangle Window::GetWindowExtentsRelative( vcl::Window *pRelativeWindow ) const
{
    // with decoration
    return ImplGetWindowExtentsRelative( pRelativeWindow, false );
}

Rectangle Window::GetClientWindowExtentsRelative( vcl::Window *pRelativeWindow ) const
{
    // without decoration
    return ImplGetWindowExtentsRelative( pRelativeWindow, true );
}

Rectangle Window::ImplGetWindowExtentsRelative( vcl::Window *pRelativeWindow, bool bClientOnly ) const
{
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    // make sure we use the extent of our border window,
    // otherwise we miss a few pixels
    const vcl::Window *pWin = (!bClientOnly && mpWindowImpl->mpBorderWindow) ? mpWindowImpl->mpBorderWindow : this;

    Point aPos( pWin->OutputToScreenPixel( Point(0,0) ) );
    aPos.X() += g.nX;
    aPos.Y() += g.nY;
    Size aSize ( pWin->GetSizePixel() );
    // #104088# do not add decoration to the workwindow to be compatible to java accessibility api
    if( !bClientOnly && (mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame && GetType() != WINDOW_WORKWINDOW)) )
    {
        aPos.X() -= g.nLeftDecoration;
        aPos.Y() -= g.nTopDecoration;
        aSize.Width() += g.nLeftDecoration + g.nRightDecoration;
        aSize.Height() += g.nTopDecoration + g.nBottomDecoration;
    }
    if( pRelativeWindow )
    {
        // #106399# express coordinates relative to borderwindow
        vcl::Window *pRelWin = (!bClientOnly && pRelativeWindow->mpWindowImpl->mpBorderWindow) ? pRelativeWindow->mpWindowImpl->mpBorderWindow.get() : pRelativeWindow;
        aPos = pRelWin->AbsoluteScreenToOutputPixel( aPos );
    }
    return Rectangle( aPos, aSize );
}

void Window::Scroll( long nHorzScroll, long nVertScroll, ScrollFlags nFlags )
{

    ImplScroll( Rectangle( Point( mnOutOffX, mnOutOffY ),
                           Size( mnOutWidth, mnOutHeight ) ),
                nHorzScroll, nVertScroll, nFlags & ~ScrollFlags::Clip );
}

void Window::Scroll( long nHorzScroll, long nVertScroll,
                     const Rectangle& rRect, ScrollFlags nFlags )
{
    OutputDevice *pOutDev = GetOutDev();
    Rectangle aRect = pOutDev->ImplLogicToDevicePixel( rRect );
    aRect.Intersection( Rectangle( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) ) );
    if ( !aRect.IsEmpty() )
        ImplScroll( aRect, nHorzScroll, nVertScroll, nFlags );
}

void Window::Flush()
{

    const Rectangle aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
    mpWindowImpl->mpFrame->Flush( aWinRect );
}

void Window::SetUpdateMode( bool bUpdate )
{
    mpWindowImpl->mbNoUpdate = !bUpdate;
    CompatStateChanged( StateChangedType::UpdateMode );
}

void Window::GrabFocus()
{
    ImplGrabFocus( GetFocusFlags::NONE );
}

bool Window::HasFocus() const
{
    return (this == ImplGetSVData()->maWinData.mpFocusWin);
}

void Window::GrabFocusToDocument()
{
    ImplGrabFocusToDocument(GetFocusFlags::NONE);
}

void Window::SetFakeFocus( bool bFocus )
{
    ImplGetWindowImpl()->mbFakeFocusSet = bFocus;
}

bool Window::HasChildPathFocus( bool bSystemWindow ) const
{

    vcl::Window* pFocusWin = ImplGetSVData()->maWinData.mpFocusWin;
    if ( pFocusWin )
        return ImplIsWindowOrChild( pFocusWin, bSystemWindow );
    return false;
}

void Window::SetCursor( vcl::Cursor* pCursor )
{

    if ( mpWindowImpl->mpCursor != pCursor )
    {
        if ( mpWindowImpl->mpCursor )
            mpWindowImpl->mpCursor->ImplHide( true );
        mpWindowImpl->mpCursor = pCursor;
        if ( pCursor )
            pCursor->ImplShow();
    }
}

void Window::SetText( const OUString& rStr )
{
    if (!mpWindowImpl || rStr == mpWindowImpl->maText)
        return;

    OUString oldTitle( mpWindowImpl->maText );
    mpWindowImpl->maText = rStr;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetText( rStr );
    else if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetTitle( rStr );

    CallEventListeners( VCLEVENT_WINDOW_FRAMETITLECHANGED, &oldTitle );

    // #107247# needed for accessibility
    // The VCLEVENT_WINDOW_FRAMETITLECHANGED is (mis)used to notify accessible name changes.
    // Therefore a window, which is labeled by this window, must also notify an accessible
    // name change.
    if ( IsReallyVisible() )
    {
        vcl::Window* pWindow = GetAccessibleRelationLabelFor();
        if ( pWindow && pWindow != this )
            pWindow->CallEventListeners( VCLEVENT_WINDOW_FRAMETITLECHANGED, &oldTitle );
    }

    CompatStateChanged( StateChangedType::Text );
}

OUString Window::GetText() const
{

    return mpWindowImpl->maText;
}

OUString Window::GetDisplayText() const
{

    return GetText();
}

const Wallpaper& Window::GetDisplayBackground() const
{
    // FIXME: fix issue 52349, need to fix this really in
    // all NWF enabled controls
    const ToolBox* pTB = dynamic_cast<const ToolBox*>(this);
    if( pTB )
    {
        if( IsNativeWidgetEnabled() )
            return pTB->ImplGetToolBoxPrivateData()->maDisplayBackground;
    }

    if( !IsBackground() )
    {
        if( mpWindowImpl->mpParent )
            return mpWindowImpl->mpParent->GetDisplayBackground();
    }

    const Wallpaper& rBack = GetBackground();
    if( ! rBack.IsBitmap() &&
        ! rBack.IsGradient() &&
        rBack.GetColor().GetColor() == COL_TRANSPARENT &&
        mpWindowImpl->mpParent )
            return mpWindowImpl->mpParent->GetDisplayBackground();
    return rBack;
}

const OUString& Window::GetHelpText() const
{
    OUString aStrHelpId( OStringToOUString( GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
    bool bStrHelpId = !aStrHelpId.isEmpty();

    if ( !mpWindowImpl->maHelpText.getLength() && bStrHelpId )
    {
        if ( !IsDialog() && (mpWindowImpl->mnType != WINDOW_TABPAGE) && (mpWindowImpl->mnType != WINDOW_FLOATINGWINDOW) )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                mpWindowImpl->maHelpText = pHelp->GetHelpText(aStrHelpId, this);
                mpWindowImpl->mbHelpTextDynamic = false;
            }
        }
    }
    else if( mpWindowImpl->mbHelpTextDynamic && bStrHelpId )
    {
        static const char* pEnv = getenv( "HELP_DEBUG" );
        if( pEnv && *pEnv )
        {
            OUStringBuffer aTxt( 64+mpWindowImpl->maHelpText.getLength() );
            aTxt.append( mpWindowImpl->maHelpText );
            aTxt.append( "\n------------------\n" );
            aTxt.append( OUString( aStrHelpId ) );
            mpWindowImpl->maHelpText = aTxt.makeStringAndClear();
        }
        mpWindowImpl->mbHelpTextDynamic = false;
    }

    return mpWindowImpl->maHelpText;
}

void Window::SetWindowPeer( Reference< css::awt::XWindowPeer > xPeer, VCLXWindow* pVCLXWindow  )
{
    // be safe against re-entrance: first clear the old ref, then assign the new one
    mpWindowImpl->mxWindowPeer.clear();
    mpWindowImpl->mxWindowPeer = xPeer;

    mpWindowImpl->mpVCLXWindow = pVCLXWindow;
}

Reference< css::awt::XWindowPeer > Window::GetComponentInterface( bool bCreate )
{
    if ( !mpWindowImpl->mxWindowPeer.is() && bCreate )
    {
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
        if ( pWrapper )
            mpWindowImpl->mxWindowPeer = pWrapper->GetWindowInterface( this, true );
    }
    return mpWindowImpl->mxWindowPeer;
}

void Window::SetComponentInterface( Reference< css::awt::XWindowPeer > xIFace )
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    DBG_ASSERT( pWrapper, "SetComponentInterface: No Wrapper!" );
    if ( pWrapper )
        pWrapper->SetWindowInterface( this, xIFace );
}

void Window::ImplCallDeactivateListeners( vcl::Window *pNew )
{
    // no deactivation if the newly activated window is my child
    if ( !pNew || !ImplIsChild( pNew ) )
    {
        ImplDelData aDogtag( this );
        CallEventListeners( VCLEVENT_WINDOW_DEACTIVATE );
        if( aDogtag.IsDead() )
            return;

        // #100759#, avoid walking the wrong frame's hierarchy
        //           eg, undocked docking windows (ImplDockFloatWin)
        if ( ImplGetParent() && mpWindowImpl->mpFrameWindow == ImplGetParent()->mpWindowImpl->mpFrameWindow )
            ImplGetParent()->ImplCallDeactivateListeners( pNew );
    }
}

void Window::ImplCallActivateListeners( vcl::Window *pOld )
{
    // no activation if the old active window is my child
    if ( !pOld || !ImplIsChild( pOld ) )
    {
        ImplDelData aDogtag( this );
        CallEventListeners( VCLEVENT_WINDOW_ACTIVATE, pOld );
        if( aDogtag.IsDead() )
            return;

        if ( ImplGetParent() )
            ImplGetParent()->ImplCallActivateListeners( pOld );
        else if( (mpWindowImpl->mnStyle & WB_INTROWIN) == 0 )
        {
            // top level frame reached: store hint for DefModalDialogParent
            ImplGetSVData()->maWinData.mpActiveApplicationFrame = mpWindowImpl->mpFrameWindow;
        }
    }
}

void Window::SetClipboard(Reference<XClipboard> xClipboard)
{
    if (mpWindowImpl->mpFrameData)
        mpWindowImpl->mpFrameData->mxClipboard = xClipboard;
}

Reference< XClipboard > Window::GetClipboard()
{

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxClipboard.is() )
        {
            try
            {
                mpWindowImpl->mpFrameData->mxClipboard
                    = css::datatransfer::clipboard::SystemClipboard::create(
                        comphelper::getProcessComponentContext());
            }
            catch (DeploymentException & e)
            {
                SAL_WARN(
                    "vcl.window",
                    "ignoring DeploymentException \"" << e.Message << "\"");
            }
        }

        return mpWindowImpl->mpFrameData->mxClipboard;
    }

    return static_cast < XClipboard * > (nullptr);
}

Reference< XClipboard > Window::GetPrimarySelection()
{

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxSelection.is() )
        {
            try
            {
                Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );

#if HAVE_FEATURE_X11
                // A hack, making the primary selection available as an instance
                // of the SystemClipboard service on X11:
                Sequence< Any > args(1);
                args[0] <<= OUString("PRIMARY");
                mpWindowImpl->mpFrameData->mxSelection.set(
                    (xContext->getServiceManager()->
                     createInstanceWithArgumentsAndContext(
                         "com.sun.star.datatransfer.clipboard.SystemClipboard",
                         args, xContext)),
                    UNO_QUERY_THROW);
#else
                static Reference< XClipboard > s_xSelection(
                    xContext->getServiceManager()->createInstanceWithContext( "com.sun.star.datatransfer.clipboard.GenericClipboard", xContext ), UNO_QUERY );

                mpWindowImpl->mpFrameData->mxSelection = s_xSelection;
#endif
            }
            catch (RuntimeException & e)
            {
                SAL_WARN(
                    "vcl.window",
                    "ignoring RuntimeException \"" << e.Message << "\"");
            }
        }

        return mpWindowImpl->mpFrameData->mxSelection;
    }

    return static_cast < XClipboard * > (nullptr);
}

void Window::RecordLayoutData( vcl::ControlLayoutData* pLayout, const Rectangle& rRect )
{
    assert(mpOutDevData);
    mpOutDevData->mpRecordLayout = pLayout;
    mpOutDevData->maRecordRect = rRect;
    Paint(*this, rRect);
    mpOutDevData->mpRecordLayout = nullptr;
}

void Window::DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, bool bChecked, bool bDrawBorder, bool bDrawExtBorderOnly )
{
    DrawSelectionBackground( rRect, highlight, bChecked, bDrawBorder, bDrawExtBorderOnly, 0, nullptr, nullptr );
}

void Window::DrawSelectionBackground( const Rectangle& rRect,
                                      sal_uInt16 highlight,
                                      bool bChecked,
                                      bool bDrawBorder,
                                      bool bDrawExtBorderOnly,
                                      long nCornerRadius,
                                      Color* pSelectionTextColor,
                                      Color* pPaintColor
                                      )
{
    if( rRect.IsEmpty() )
        return;

    bool bRoundEdges = nCornerRadius > 0;

    const StyleSettings& rStyles = GetSettings().GetStyleSettings();

    // colors used for item highlighting
    Color aSelectionBorderCol( pPaintColor ? *pPaintColor : rStyles.GetHighlightColor() );
    Color aSelectionFillCol( aSelectionBorderCol );

    bool bDark = rStyles.GetFaceColor().IsDark();
    bool bBright = ( rStyles.GetFaceColor() == Color( COL_WHITE ) );

    int c1 = aSelectionBorderCol.GetLuminance();
    int c2 = GetDisplayBackground().GetColor().GetLuminance();

    if( !bDark && !bBright && abs( c2-c1 ) < (pPaintColor ? 40 : 75) )
    {
        // constrast too low
        sal_uInt16 h,s,b;
        aSelectionFillCol.RGBtoHSB( h, s, b );
        if( b > 50 )    b -= 40;
        else            b += 40;
        aSelectionFillCol.SetColor( Color::HSBtoRGB( h, s, b ) );
        aSelectionBorderCol = aSelectionFillCol;
    }

    if( bRoundEdges )
    {
        if( aSelectionBorderCol.IsDark() )
            aSelectionBorderCol.IncreaseLuminance( 128 );
        else
            aSelectionBorderCol.DecreaseLuminance( 128 );
    }

    Rectangle aRect( rRect );
    if( bDrawExtBorderOnly )
    {
        --aRect.Left();
        --aRect.Top();
        ++aRect.Right();
        ++aRect.Bottom();
    }
    Color oldFillCol = GetFillColor();
    Color oldLineCol = GetLineColor();

    if( bDrawBorder )
        SetLineColor( bDark ? Color(COL_WHITE) : ( bBright ? Color(COL_BLACK) : aSelectionBorderCol ) );
    else
        SetLineColor();

    sal_uInt16 nPercent = 0;
    if( !highlight )
    {
        if( bDark )
            aSelectionFillCol = COL_BLACK;
        else
            nPercent = 80;  // just checked (light)
    }
    else
    {
        if( bChecked && highlight == 2 )
        {
            if( bDark )
                aSelectionFillCol = COL_LIGHTGRAY;
            else if ( bBright )
            {
                aSelectionFillCol = COL_BLACK;
                SetLineColor( COL_BLACK );
                nPercent = 0;
            }
            else
                nPercent = bRoundEdges ? 40 : 20;          // selected, pressed or checked ( very dark )
        }
        else if( bChecked || highlight == 1 )
        {
            if( bDark )
                aSelectionFillCol = COL_GRAY;
            else if ( bBright )
            {
                aSelectionFillCol = COL_BLACK;
                SetLineColor( COL_BLACK );
                nPercent = 0;
            }
            else
                nPercent = bRoundEdges ? 60 : 35;          // selected, pressed or checked ( very dark )
        }
        else
        {
            if( bDark )
                aSelectionFillCol = COL_LIGHTGRAY;
            else if ( bBright )
            {
                aSelectionFillCol = COL_BLACK;
                SetLineColor( COL_BLACK );
                if( highlight == 3 )
                    nPercent = 80;
                else
                    nPercent = 0;
            }
            else
                nPercent = 70;          // selected ( dark )
        }
    }

    if( bDark && bDrawExtBorderOnly )
    {
        SetFillColor();
        if( pSelectionTextColor )
            *pSelectionTextColor = rStyles.GetHighlightTextColor();
    }
    else
    {
        SetFillColor( aSelectionFillCol );
        if( pSelectionTextColor )
        {
            Color aTextColor = IsControlBackground() ? GetControlForeground() : rStyles.GetButtonTextColor();
            Color aHLTextColor = rStyles.GetHighlightTextColor();
            int nTextDiff = abs(aSelectionFillCol.GetLuminance() - aTextColor.GetLuminance());
            int nHLDiff = abs(aSelectionFillCol.GetLuminance() - aHLTextColor.GetLuminance());
            *pSelectionTextColor = (nHLDiff >= nTextDiff) ? aHLTextColor : aTextColor;
        }
    }

    if( bDark )
    {
        DrawRect( aRect );
    }
    else
    {
        if( bRoundEdges )
        {
            tools::Polygon aPoly( aRect, nCornerRadius, nCornerRadius );
            tools::PolyPolygon aPolyPoly( aPoly );
            DrawTransparent( aPolyPoly, nPercent );
        }
        else
        {
            tools::Polygon aPoly( aRect );
            tools::PolyPolygon aPolyPoly( aPoly );
            DrawTransparent( aPolyPoly, nPercent );
        }
    }

    SetFillColor( oldFillCol );
    SetLineColor( oldLineCol );
}

// controls should return the window that gets the
// focus by default, so keyevents can be sent to that window directly
vcl::Window* Window::GetPreferredKeyInputWindow()
{
    return this;
}

bool Window::IsScrollable() const
{
    // check for scrollbars
    vcl::Window *pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        if( pChild->GetType() == WINDOW_SCROLLBAR )
            return true;
        else
            pChild = pChild->mpWindowImpl->mpNext;
    }
    return false;
}

void Window::ImplMirrorFramePos( Point &pt ) const
{
    pt.X() = mpWindowImpl->mpFrame->maGeometry.nWidth-1-pt.X();
}

// frame based modal counter (dialogs are not modal to the whole application anymore)
bool Window::IsInModalMode() const
{
    return (mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrameData->mnModalMode != 0);
}

void Window::ImplIncModalCount()
{
    vcl::Window* pFrameWindow = mpWindowImpl->mpFrameWindow;
    vcl::Window* pParent = pFrameWindow;
    while( pFrameWindow )
    {
        pFrameWindow->mpWindowImpl->mpFrameData->mnModalMode++;
        while( pParent && pParent->mpWindowImpl->mpFrameWindow == pFrameWindow )
        {
            pParent = pParent->GetParent();
        }
        pFrameWindow = pParent ? pParent->mpWindowImpl->mpFrameWindow.get() : nullptr;
    }
}
void Window::ImplDecModalCount()
{
    vcl::Window* pFrameWindow = mpWindowImpl->mpFrameWindow;
    vcl::Window* pParent = pFrameWindow;
    while( pFrameWindow )
    {
        pFrameWindow->mpWindowImpl->mpFrameData->mnModalMode--;
        while( pParent && pParent->mpWindowImpl->mpFrameWindow == pFrameWindow )
        {
            pParent = pParent->GetParent();
        }
        pFrameWindow = pParent ? pParent->mpWindowImpl->mpFrameWindow.get() : nullptr;
    }
}

void Window::ImplIsInTaskPaneList( bool mbIsInTaskList )
{
    mpWindowImpl->mbIsInTaskPaneList = mbIsInTaskList;
}

void Window::ImplNotifyIconifiedState( bool bIconified )
{
    mpWindowImpl->mpFrameWindow->CallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
    // #109206# notify client window as well to have toolkit topwindow listeners notified
    if( mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow && mpWindowImpl->mpFrameWindow != mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow )
        mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow->CallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
}

bool Window::HasActiveChildFrame()
{
    bool bRet = false;
    vcl::Window *pFrameWin = ImplGetSVData()->maWinData.mpFirstFrame;
    while( pFrameWin )
    {
        if( pFrameWin != mpWindowImpl->mpFrameWindow )
        {
            bool bDecorated = false;
            vcl::Window *pChildFrame = pFrameWin->ImplGetWindow();
            // #i15285# unfortunately WB_MOVEABLE is the same as WB_TABSTOP which can
            // be removed for ToolBoxes to influence the keyboard accessibility
            // thus WB_MOVEABLE is no indicator for decoration anymore
            // but FloatingWindows carry this information in their TitleType...
            // TODO: avoid duplicate WinBits !!!
            if( pChildFrame && pChildFrame->ImplIsFloatingWindow() )
                bDecorated = static_cast<FloatingWindow*>(pChildFrame)->GetTitleType() != FloatWinTitleType::NONE;
            if( bDecorated || (pFrameWin->mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) ) )
                if( pChildFrame && pChildFrame->IsVisible() && pChildFrame->IsActive() )
                {
                    if( ImplIsChild( pChildFrame, true ) )
                    {
                        bRet = true;
                        break;
                    }
                }
        }
        pFrameWin = pFrameWin->mpWindowImpl->mpFrameData->mpNextFrame;
    }
    return bRet;
}

LanguageType Window::GetInputLanguage() const
{
    return mpWindowImpl->mpFrame->GetInputLanguage();
}

void Window::EnableNativeWidget( bool bEnable )
{
    static const char* pNoNWF = getenv( "SAL_NO_NWF" );
    if( pNoNWF && *pNoNWF )
        bEnable = false;

    if( bEnable != ImplGetWinData()->mbEnableNativeWidget )
    {
        ImplGetWinData()->mbEnableNativeWidget = bEnable;

        // send datachanged event to allow for internal changes required for NWF
        // like clipmode, transparency, etc.
        DataChangedEvent aDCEvt( DataChangedEventType::SETTINGS, mxSettings.get(), AllSettingsFlags::STYLE );
        CompatDataChanged( aDCEvt );

        // sometimes the borderwindow is queried, so keep it in sync
        if( mpWindowImpl->mpBorderWindow )
            mpWindowImpl->mpBorderWindow->ImplGetWinData()->mbEnableNativeWidget = bEnable;
    }

    // push down, useful for compound controls
    vcl::Window *pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        pChild->EnableNativeWidget( bEnable );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

bool Window::IsNativeWidgetEnabled() const
{
    return ImplGetWinData()->mbEnableNativeWidget;
}

Reference< css::rendering::XCanvas > Window::ImplGetCanvas( const Size& rFullscreenSize,
                                                       bool        bFullscreen,
                                                       bool        bSpriteCanvas ) const
{
    // try to retrieve hard reference from weak member
    Reference< css::rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );

    // canvas still valid? Then we're done.
    if( xCanvas.is() )
        return xCanvas;

    Sequence< Any > aArg(6);

    // Feed any with operating system's window handle

    // common: first any is VCL pointer to window (for VCL canvas)
    aArg[ 0 ] = makeAny( reinterpret_cast<sal_Int64>(this) );

    // TODO(Q1): Make GetSystemData method virtual

    // check whether we're a SysChild: have to fetch system data
    // directly from SystemChildWindow, because the GetSystemData
    // method is unfortunately not virtual
    const SystemChildWindow* pSysChild = dynamic_cast< const SystemChildWindow* >( this );
    if( pSysChild )
    {
        aArg[ 1 ] = pSysChild->GetSystemDataAny();
        aArg[ 5 ] = pSysChild->GetSystemGfxDataAny();
    }
    else
    {
        aArg[ 1 ] = GetSystemDataAny();
        aArg[ 5 ] = GetSystemGfxDataAny();
    }

    if( bFullscreen )
        aArg[ 2 ] = makeAny( css::awt::Rectangle( 0, 0,
                                    rFullscreenSize.Width(),
                                    rFullscreenSize.Height() ) );
    else
        aArg[ 2 ] = makeAny( css::awt::Rectangle( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight ) );

    aArg[ 3 ] = makeAny( mpWindowImpl->mbAlwaysOnTop );
    aArg[ 4 ] = makeAny( Reference< css::awt::XWindow >(
                             const_cast<vcl::Window*>(this)->GetComponentInterface(),
                             UNO_QUERY ));

    Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();

    // Create canvas instance with window handle

    static vcl::DeleteUnoReferenceOnDeinit<XMultiComponentFactory> xStaticCanvasFactory(
        css::rendering::CanvasFactory::create( xContext ) );
    Reference<XMultiComponentFactory> xCanvasFactory(xStaticCanvasFactory.get());

    if(xCanvasFactory.is())
    {
#ifdef WNT
        // see #140456# - if we're running on a multiscreen setup,
        // request special, multi-screen safe sprite canvas
        // implementation (not DX5 canvas, as it cannot cope with
        // surfaces spanning multiple displays). Note: canvas
        // (without sprite) stays the same)
        const sal_uInt32 nDisplay = static_cast< WinSalFrame* >( mpWindowImpl->mpFrame )->mnDisplay;
        if( (nDisplay >= Application::GetScreenCount()) )
        {
            xCanvas.set( xCanvasFactory->createInstanceWithArgumentsAndContext(
                                 bSpriteCanvas ?
                                 OUString( "com.sun.star.rendering.SpriteCanvas.MultiScreen" ) :
                                 OUString( "com.sun.star.rendering.Canvas.MultiScreen" ),
                                 aArg,
                                 xContext ),
                             UNO_QUERY );

        }
        else
        {
#endif
            xCanvas.set( xCanvasFactory->createInstanceWithArgumentsAndContext(
                             bSpriteCanvas ?
                             OUString( "com.sun.star.rendering.SpriteCanvas" ) :
                             OUString( "com.sun.star.rendering.Canvas" ),
                             aArg,
                             xContext ),
                         UNO_QUERY );

#ifdef WNT
        }
#endif
        mpWindowImpl->mxCanvas = xCanvas;
    }

    // no factory??? Empty reference, then.
    return xCanvas;
}

Reference< css::rendering::XCanvas > Window::GetCanvas() const
{
    return ImplGetCanvas( Size(), false, false );
}

Reference< css::rendering::XSpriteCanvas > Window::GetSpriteCanvas() const
{
    Reference< css::rendering::XSpriteCanvas > xSpriteCanvas(
        ImplGetCanvas( Size(), false, true ), UNO_QUERY );
    return xSpriteCanvas;
}

OUString Window::GetSurroundingText() const
{
  return OUString();
}

Selection Window::GetSurroundingTextSelection() const
{
  return Selection( 0, 0 );
}

bool Window::UsePolyPolygonForComplexGradient()
{
    if ( meRasterOp != ROP_OVERPAINT )
        return true;

    return false;
}

void Window::ApplySettings(vcl::RenderContext& /*rRenderContext*/)
{
}

const SystemEnvData* Window::GetSystemData() const
{

    return mpWindowImpl->mpFrame ? mpWindowImpl->mpFrame->GetSystemData() : nullptr;
}

Any Window::GetSystemDataAny() const
{
    Any aRet;
    const SystemEnvData* pSysData = GetSystemData();
    if( pSysData )
    {
        Sequence< sal_Int8 > aSeq( reinterpret_cast<sal_Int8 const *>(pSysData), pSysData->nSize );
        aRet <<= aSeq;
    }
    return aRet;
}

bool Window::SupportsDoubleBuffering() const
{
    return mpWindowImpl->mpFrameData->mpBuffer;
}

void Window::RequestDoubleBuffering(bool bRequest)
{
    if (bRequest)
    {
        mpWindowImpl->mpFrameData->mpBuffer = VclPtrInstance<VirtualDevice>();
        // Make sure that the buffer size matches the frame size.
        mpWindowImpl->mpFrameData->mpBuffer->SetOutputSizePixel(mpWindowImpl->mpFrameWindow->GetOutputSizePixel());
    }
    else
        mpWindowImpl->mpFrameData->mpBuffer.reset();
}

/*
 * The rational here is that we moved destructors to
 * dispose and this altered a lot of code paths, that
 * are better left unchanged for now.
 */
#define COMPAT_BODY(method,args) \
    if (!mpWindowImpl || mpWindowImpl->mbInDispose) \
        Window::method args; \
    else \
        method args;

void Window::CompatGetFocus()
{
    COMPAT_BODY(GetFocus,())
}

void Window::CompatLoseFocus()
{
    COMPAT_BODY(LoseFocus,())
}

void Window::CompatStateChanged( StateChangedType nStateChange )
{
    COMPAT_BODY(StateChanged,(nStateChange))
}

void Window::CompatDataChanged( const DataChangedEvent& rDCEvt )
{
    COMPAT_BODY(DataChanged,(rDCEvt))
}

bool Window::CompatPreNotify( NotifyEvent& rNEvt )
{
    if (!mpWindowImpl || mpWindowImpl->mbInDispose)
        return Window::PreNotify( rNEvt );
    else
        return PreNotify( rNEvt );
}

bool Window::CompatNotify( NotifyEvent& rNEvt )
{
    if (!mpWindowImpl || mpWindowImpl->mbInDispose)
        return Window::Notify( rNEvt );
    else
        return Notify( rNEvt );
}

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
