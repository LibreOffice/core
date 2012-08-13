/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tools/debug.hxx"

#include "sal/alloca.h"

#include "vcl/floatwin.hxx"
#include "vcl/svapp.hxx"
#include "vcl/keycodes.hxx"
#include "vcl/printerinfomanager.hxx"
#include "vcl/settings.hxx"

#include <tools/prex.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include "FWS.hxx"
#include <X11/extensions/shape.h>
#if !defined(SOLARIS) && !defined(AIX)
#include <X11/extensions/dpms.h>
#endif
#include <tools/postx.h>

#include "unx/salunx.h"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "unx/salgdi.h"
#include "unx/salframe.h"
#include "unx/soicon.hxx"
#include "unx/sm.hxx"
#include "unx/wmadaptor.hxx"
#include "generic/genprn.h"
#include "unx/salbmp.h"
#include "unx/i18n_ic.hxx"
#include "unx/i18n_keysym.hxx"
#include "unx/i18n_status.hxx"
#include <unx/x11/xlimits.hxx>

#include "generic/gensys.h"
#include "sallayout.hxx"

#include <sal/macros.h>
#include <com/sun/star/uno/Exception.hpp>

#include <algorithm>

#ifndef Button6
# define Button6 6
#endif
#ifndef Button7
# define Button7 7
#endif

using namespace vcl_sal;
using namespace vcl;

// -=-= #defines -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define CLIENT_EVENTS           StructureNotifyMask \
                                | SubstructureNotifyMask \
                                | KeyPressMask \
                                | KeyReleaseMask \
                                | ButtonPressMask \
                                | ButtonReleaseMask \
                                | PointerMotionMask \
                                | EnterWindowMask \
                                | LeaveWindowMask \
                                | FocusChangeMask \
                                | ExposureMask \
                                | VisibilityChangeMask \
                                | PropertyChangeMask \
                                | ColormapChangeMask

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static XLIB_Window  hPresentationWindow = None, hPresFocusWindow = None;
static ::std::list< XLIB_Window > aPresentationReparentList;
static int          nVisibleFloats      = 0;

// -=-= C++ statics =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static void doReparentPresentationDialogues( SalDisplay* pDisplay )
{
    GetGenericData()->ErrorTrapPush();
    while( aPresentationReparentList.begin() != aPresentationReparentList.end() )
    {
        int x, y;
        XLIB_Window aRoot, aChild;
        unsigned int w, h, bw, d;
        XGetGeometry( pDisplay->GetDisplay(),
                      aPresentationReparentList.front(),
                      &aRoot,
                      &x, &y, &w, &h, &bw, &d );
        XTranslateCoordinates( pDisplay->GetDisplay(),
                               hPresentationWindow,
                               aRoot,
                               x, y,
                               &x, &y,
                               &aChild );
        XReparentWindow( pDisplay->GetDisplay(),
                         aPresentationReparentList.front(),
                         aRoot,
                         x, y );
        aPresentationReparentList.pop_front();
    }
    if( hPresFocusWindow )
        XSetInputFocus( pDisplay->GetDisplay(), hPresFocusWindow, PointerRoot, CurrentTime );
    XSync( pDisplay->GetDisplay(), False );
    GetGenericData()->ErrorTrapPop();
}

// -=-= SalFrame / X11SalFrame =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

bool X11SalFrame::IsOverrideRedirect() const
{
    return
        ((nStyle_ & SAL_FRAME_STYLE_INTRO) && !pDisplay_->getWMAdaptor()->supportsSplash())
        ||
        (!( nStyle_ & ~SAL_FRAME_STYLE_DEFAULT ) && !pDisplay_->getWMAdaptor()->supportsFullScreen())
        ;
}

bool X11SalFrame::IsFloatGrabWindow() const
{
    static const char* pDisableGrab = getenv( "SAL_DISABLE_FLOATGRAB" );

    return
        ( ( !pDisableGrab || !*pDisableGrab ) &&
          (
           (nStyle_ & SAL_FRAME_STYLE_FLOAT)    &&
           ! (nStyle_ & SAL_FRAME_STYLE_TOOLTIP)    &&
           ! (nStyle_ & SAL_FRAME_STYLE_OWNERDRAWDECORATION)
           )
          );
}

void X11SalFrame::setXEmbedInfo()
{
    if( m_bXEmbed )
    {
        long aInfo[2];
        aInfo[0] = 1; // XEMBED protocol version
        aInfo[1] = (bMapped_ ? 1 : 0); // XEMBED_MAPPED
        XChangeProperty( pDisplay_->GetDisplay(),
                         mhWindow,
                         pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::XEMBED_INFO ),
                         pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::XEMBED_INFO ),
                         32,
                         PropModeReplace,
                         reinterpret_cast<unsigned char*>(aInfo),
                         SAL_N_ELEMENTS(aInfo) );
    }
}

void X11SalFrame::askForXEmbedFocus( sal_Int32 i_nTimeCode )
{
    XEvent aEvent;

    memset( &aEvent, 0, sizeof(aEvent) );
    aEvent.xclient.window = mhForeignParent;
    aEvent.xclient.type = ClientMessage;
    aEvent.xclient.message_type = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::XEMBED );
    aEvent.xclient.format = 32;
    aEvent.xclient.data.l[0] = i_nTimeCode ? i_nTimeCode : CurrentTime;
    aEvent.xclient.data.l[1] = 3; // XEMBED_REQUEST_FOCUS
    aEvent.xclient.data.l[2] = 0;
    aEvent.xclient.data.l[3] = 0;
    aEvent.xclient.data.l[4] = 0;

    GetGenericData()->ErrorTrapPush();
    XSendEvent( pDisplay_->GetDisplay(),
                mhForeignParent,
                False, NoEventMask, &aEvent );
    XSync( pDisplay_->GetDisplay(), False );
    GetGenericData()->ErrorTrapPop();
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::Init( sal_uLong nSalFrameStyle, SalX11Screen nXScreen, SystemParentData* pParentData, bool bUseGeometry )
{
    if( nXScreen.getXScreen() >= GetDisplay()->GetXScreenCount() )
        nXScreen = GetDisplay()->GetDefaultXScreen();
    if( mpParent )
        nXScreen = mpParent->m_nXScreen;

    m_nXScreen  = nXScreen;
    nStyle_     = nSalFrameStyle;
    XWMHints Hints;
    Hints.flags = InputHint;
    Hints.input = (nSalFrameStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) ? False : True;

    int x = 0, y = 0;
    unsigned int w = 500, h = 500;
    XSetWindowAttributes Attributes;

    int nAttrMask =   CWBorderPixel
                    | CWBackPixmap
                    | CWColormap
                    | CWOverrideRedirect
                    | CWEventMask
                    ;
    Attributes.border_pixel             = 0;
    Attributes.background_pixmap        = None;
    Attributes.colormap                 = GetDisplay()->GetColormap( m_nXScreen ).GetXColormap();
    Attributes.override_redirect        = False;
    Attributes.event_mask               = CLIENT_EVENTS;

    const SalVisual& rVis = GetDisplay()->GetVisual( m_nXScreen );
    XLIB_Window aFrameParent = pParentData ? pParentData->aWindow : GetDisplay()->GetRootWindow( m_nXScreen );
    XLIB_Window aClientLeader = None;

    if( bUseGeometry )
    {
        x = maGeometry.nX;
        y = maGeometry.nY;
        w = maGeometry.nWidth;
        h = maGeometry.nHeight;
    }

    if( (nSalFrameStyle & SAL_FRAME_STYLE_FLOAT) &&
        ! (nSalFrameStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION)
        )
    {
        if( nShowState_ == SHOWSTATE_UNKNOWN )
        {
            w = 10;
            h = 10;
        }
        Attributes.override_redirect = True;
    }
    else if( (nSalFrameStyle & SAL_FRAME_STYLE_SYSTEMCHILD ) )
    {
        DBG_ASSERT( mpParent, "SAL_FRAME_STYLE_SYSTEMCHILD window without parent" );
        if( mpParent )
        {
            aFrameParent = mpParent->mhWindow;
            // FIXME: since with SAL_FRAME_STYLE_SYSTEMCHILD
            // multiple X11SalFrame objects can have the same shell window
            // dispatching events in saldisp.cxx is unclear (the first frame)
            // wins. HTH this correctly is unclear yet
            // for the time being, treat set the shell window to own window
            // like for a normal frame
            // mhShellWindow = mpParent->GetShellWindow();
        }
    }
    else if( pParentData )
    {
        // plugin parent may be killed unexpectedly by plugging
        // process; start permanantly ignoring X errors ...
        GetGenericData()->ErrorTrapPush();

        nStyle_ |= SAL_FRAME_STYLE_PLUG;
        Attributes.override_redirect = True;
        if( pParentData->nSize >= sizeof(SystemParentData) )
            m_bXEmbed = pParentData->bXEmbedSupport;

        int x_ret, y_ret;
        unsigned int bw, d;
        XLIB_Window aRoot, aParent;

        XGetGeometry( GetXDisplay(), pParentData->aWindow,
                      &aRoot, &x_ret, &y_ret, &w, &h, &bw, &d );
        mhForeignParent = pParentData->aWindow;

        mhShellWindow = aParent = mhForeignParent;
        XLIB_Window* pChildren;
        unsigned int nChildren;
        bool bBreak = false;
        do
        {
            XQueryTree( GetDisplay()->GetDisplay(), mhShellWindow,
                        &aRoot, &aParent, &pChildren, &nChildren );
            XFree( pChildren );
            if( aParent != aRoot )
                mhShellWindow = aParent;
            int nCount = 0;
            Atom* pProps = XListProperties( GetDisplay()->GetDisplay(),
                                            mhShellWindow,
                                            &nCount );
            for( int i = 0; i < nCount && ! bBreak; ++i )
                bBreak = (pProps[i] == XA_WM_HINTS);
            if( pProps )
                XFree( pProps );
        } while( aParent != aRoot && ! bBreak );

        // check if this is really one of our own frames
        // do not change the input mask in that case
        const std::list< SalFrame* >& rFrames = GetDisplay()->getFrames();
        std::list< SalFrame* >::const_iterator it = rFrames.begin();
        while( it != rFrames.end() && mhForeignParent != static_cast<const X11SalFrame*>(*it)->GetWindow() )
            ++it;

        if( it == rFrames.end() )
        {
            XSelectInput( GetDisplay()->GetDisplay(), mhForeignParent, StructureNotifyMask | FocusChangeMask );
            XSelectInput( GetDisplay()->GetDisplay(), mhShellWindow, StructureNotifyMask | FocusChangeMask );
        }
    }
    else
    {
        if( ! bUseGeometry )
        {
            Size aScreenSize( GetDisplay()->getDataForScreen( m_nXScreen ).m_aSize );
            w = aScreenSize.Width();
            h = aScreenSize.Height();
            if( nSalFrameStyle & SAL_FRAME_STYLE_SIZEABLE &&
                nSalFrameStyle & SAL_FRAME_STYLE_MOVEABLE )
            {
                // fill in holy default values brought to us by product management
                if( aScreenSize.Width() >= 800 )
                    w = 785;
                if( aScreenSize.Width() >= 1024 )
                    w = 920;
                if( aScreenSize.Width() >= 1280 )
                    w = 1050;

                if( aScreenSize.Height() >= 600 )
                    h = 550;
                if( aScreenSize.Height() >= 768 )
                    h = 630;
                if( aScreenSize.Height() >= 1024 )
                    h = 875;
            }
            if( ! mpParent )
            {
                // find the last document window (if any)
                const X11SalFrame* pFrame = NULL;
                const std::list< SalFrame* >& rFrames = GetDisplay()->getFrames();
                std::list< SalFrame* >::const_iterator it = rFrames.begin();
                while( it != rFrames.end() )
                {
                    pFrame = static_cast< const X11SalFrame* >(*it);
                    if( ! ( pFrame->mpParent
                            || pFrame->mbFullScreen
                            || ! ( pFrame->nStyle_ & SAL_FRAME_STYLE_SIZEABLE )
                            || ! pFrame->GetUnmirroredGeometry().nWidth
                            || ! pFrame->GetUnmirroredGeometry().nHeight
                            )
                        )
                        break;
                    ++it;
                }

                if( it != rFrames.end() )
                {
                    // set a document position and size
                    // the first frame gets positioned by the window manager
                    const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
                    x = rGeom.nX;
                    y = rGeom.nY;
                    if( x+(int)w+40 <= (int)aScreenSize.Width() &&
                        y+(int)h+40 <= (int)aScreenSize.Height()
                        )
                    {
                        y += 40;
                        x += 40;
                    }
                    else
                    {
                        x = 10; // leave some space for decoration
                        y = 20;
                    }
                }
                else if( GetDisplay()->IsXinerama() )
                {
                    // place frame on same screen as mouse pointer
                    XLIB_Window aRoot, aChild;
                    int root_x = 0, root_y = 0, lx, ly;
                    unsigned int mask;
                    XQueryPointer( GetXDisplay(),
                                   GetDisplay()->GetRootWindow( m_nXScreen ),
                                   &aRoot, &aChild,
                                   &root_x, &root_y, &lx, &ly, &mask );
                    const std::vector< Rectangle >& rScreens = GetDisplay()->GetXineramaScreens();
                    for( unsigned int i = 0; i < rScreens.size(); i++ )
                        if( rScreens[i].IsInside( Point( root_x, root_y ) ) )
                        {
                            x = rScreens[i].Left();
                            y = rScreens[i].Top();
                            break;
                        }
                }
            }
        }
        Attributes.win_gravity = pDisplay_->getWMAdaptor()->getInitWinGravity();
        nAttrMask |= CWWinGravity;
        if( mpParent )
        {
            Attributes.save_under = True;
            nAttrMask |= CWSaveUnder;
        }
        if( IsOverrideRedirect() )
            Attributes.override_redirect = True;
        // default icon
        if( (nStyle_ & SAL_FRAME_STYLE_INTRO) == 0 )
        {
            bool bOk=false;
            try
            {
                bOk=SelectAppIconPixmap( pDisplay_, m_nXScreen,
                                         mnIconID != 1 ? mnIconID :
                                         (mpParent ? mpParent->mnIconID : 1), 32,
                                         Hints.icon_pixmap, Hints.icon_mask );
            }
            catch( com::sun::star::uno::Exception& )
            {
                // can happen - no ucb during early startup
            }
            if( bOk )
            {
                Hints.flags     |= IconPixmapHint;
                if( Hints.icon_mask )
                    Hints.flags |= IconMaskHint;
            }
        }

        // find the top level frame of the transience hierarchy
        X11SalFrame* pFrame = this;
        while( pFrame->mpParent )
            pFrame = pFrame->mpParent;
        if( (pFrame->nStyle_ & SAL_FRAME_STYLE_PLUG ) )
        {
            // if the top level window is a plugin window,
            // then we should place us in the same window group as
            // the parent application (or none if there is no window group
            // hint in the parent).
            if( pFrame->GetShellWindow() )
            {
                XWMHints* pWMHints = XGetWMHints( pDisplay_->GetDisplay(),
                    pFrame->GetShellWindow() );
                if( pWMHints )
                {
                    if( (pWMHints->flags & WindowGroupHint) )
                    {
                        Hints.flags |= WindowGroupHint;
                        Hints.window_group = pWMHints->window_group;
                    }
                    XFree( pWMHints );
                }
            }
        }
        else
        {
            Hints.flags         |= WindowGroupHint;
            Hints.window_group  = pFrame->GetShellWindow();
            // note: for a normal document window this will produce None
            // as the window is not yet created and the shell window is
            // initialized to None. This must be corrected after window creation.
            aClientLeader = GetDisplay()->GetDrawable( m_nXScreen );
        }
    }

    nShowState_                 = SHOWSTATE_UNKNOWN;
    bViewable_                  = sal_True;
    bMapped_                    = sal_False;
    nVisibility_                = VisibilityFullyObscured;
    mhWindow = XCreateWindow( GetXDisplay(),
                              aFrameParent,
                              x, y,
                              w, h,
                              0,
                              rVis.GetDepth(),
                              InputOutput,
                              rVis.GetVisual(),
                              nAttrMask,
                              &Attributes );
    // FIXME: see above: fake shell window for now to own window
    if( pParentData == NULL )
    {
        mhShellWindow = mhWindow;
    }

    // correct window group if necessary
    if( (Hints.flags & WindowGroupHint) == WindowGroupHint )
    {
        if( Hints.window_group == None )
            Hints.window_group = GetShellWindow();
    }

    maGeometry.nX       = x;
    maGeometry.nY       = y;
    maGeometry.nWidth   = w;
    maGeometry.nHeight  = h;
    updateScreenNumber();

    XSync( GetXDisplay(), False );
    setXEmbedInfo();

    XLIB_Time nUserTime = (nStyle_ & (SAL_FRAME_STYLE_OWNERDRAWDECORATION | SAL_FRAME_STYLE_TOOLWINDOW) ) == 0 ?
        pDisplay_->GetLastUserEventTime() : 0;
    pDisplay_->getWMAdaptor()->setUserTime( this, nUserTime );

    if( ! pParentData && ! IsChildWindow() && ! Attributes.override_redirect )
    {
        XSetWMHints( GetXDisplay(), mhWindow, &Hints );
        // WM Protocols && internals
        Atom a[3];
        int  n = 0;
        a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_DELETE_WINDOW );
        if( pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::NET_WM_PING ) )
            a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::NET_WM_PING );
        if( (nSalFrameStyle & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
            a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_TAKE_FOCUS );
        XSetWMProtocols( GetXDisplay(), GetShellWindow(), a, n );

        // force wm class hint
        mnExtStyle = ~0;
        if (mpParent)
            m_sWMClass = mpParent->m_sWMClass;
        SetExtendedFrameStyle( 0 );

        XSizeHints* pHints = XAllocSizeHints();
        pHints->flags       = PWinGravity | PPosition;
        pHints->win_gravity = GetDisplay()->getWMAdaptor()->getPositionWinGravity();
        pHints->x           = 0;
        pHints->y           = 0;
        if( mbFullScreen )
        {
            pHints->flags |= PMaxSize | PMinSize;
            pHints->max_width = w+100;
            pHints->max_height = h+100;
            pHints->min_width  = w;
            pHints->min_height = h;
        }
        XSetWMNormalHints( GetXDisplay(),
                           GetShellWindow(),
                           pHints );
        XFree (pHints);

        // set PID and WM_CLIENT_MACHINE
        pDisplay_->getWMAdaptor()->setClientMachine( this );
        pDisplay_->getWMAdaptor()->setPID( this );

        // set client leader
        if( aClientLeader )
        {
            XChangeProperty( GetXDisplay(),
                             mhWindow,
                             pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_CLIENT_LEADER),
                             XA_WINDOW,
                             32,
                             PropModeReplace,
                             (unsigned char*)&aClientLeader,
                             1
                             );
        }
#define DECOFLAGS (SAL_FRAME_STYLE_MOVEABLE | SAL_FRAME_STYLE_SIZEABLE | SAL_FRAME_STYLE_CLOSEABLE)
        int nDecoFlags = WMAdaptor::decoration_All;
        if( (nStyle_ & SAL_FRAME_STYLE_PARTIAL_FULLSCREEN) ||
            (nStyle_ & SAL_FRAME_STYLE_OWNERDRAWDECORATION)
            )
            nDecoFlags = 0;
        else if( (nStyle_ & DECOFLAGS ) != DECOFLAGS || (nStyle_ & SAL_FRAME_STYLE_TOOLWINDOW) )
        {
            if( nStyle_ & DECOFLAGS )
                // if any decoration, then show a border
                nDecoFlags = WMAdaptor::decoration_Border;
            else
                nDecoFlags = 0;

            if( ! mpParent && (nStyle_ & DECOFLAGS) )
                // don't add a min button if window should be decorationless
                nDecoFlags |= WMAdaptor::decoration_MinimizeBtn;
            if( nStyle_ & SAL_FRAME_STYLE_CLOSEABLE )
                nDecoFlags |= WMAdaptor::decoration_CloseBtn;
            if( nStyle_ & SAL_FRAME_STYLE_SIZEABLE )
            {
                nDecoFlags |= WMAdaptor::decoration_Resize;
                if( ! (nStyle_ & SAL_FRAME_STYLE_TOOLWINDOW) )
                    nDecoFlags |= WMAdaptor::decoration_MaximizeBtn;
            }
            if( nStyle_ & SAL_FRAME_STYLE_MOVEABLE )
                nDecoFlags |= WMAdaptor::decoration_Title;
        }

        WMAdaptor::WMWindowType eType = WMAdaptor::windowType_Normal;
        if( nStyle_ & SAL_FRAME_STYLE_INTRO )
            eType = WMAdaptor::windowType_Splash;
        if( (nStyle_ & SAL_FRAME_STYLE_DIALOG) && hPresentationWindow == None )
            eType = WMAdaptor::windowType_ModelessDialogue;
        if( nStyle_ & SAL_FRAME_STYLE_TOOLWINDOW )
            eType = WMAdaptor::windowType_Utility;
        if( nStyle_ & SAL_FRAME_STYLE_OWNERDRAWDECORATION )
            eType = WMAdaptor::windowType_Toolbar;
        if(    (nStyle_ & SAL_FRAME_STYLE_PARTIAL_FULLSCREEN)
            && GetDisplay()->getWMAdaptor()->isLegacyPartialFullscreen() )
            eType = WMAdaptor::windowType_Dock;

        GetDisplay()->getWMAdaptor()->
            setFrameTypeAndDecoration( this,
                                       eType,
                                       nDecoFlags,
                                       hPresentationWindow ? NULL : mpParent );

        if( (nStyle_ & (SAL_FRAME_STYLE_DEFAULT |
                        SAL_FRAME_STYLE_OWNERDRAWDECORATION|
                        SAL_FRAME_STYLE_FLOAT |
                        SAL_FRAME_STYLE_INTRO |
                        SAL_FRAME_STYLE_PARTIAL_FULLSCREEN) )
             == SAL_FRAME_STYLE_DEFAULT )
            pDisplay_->getWMAdaptor()->maximizeFrame( this, true, true );
    }

    m_nWorkArea = GetDisplay()->getWMAdaptor()->getCurrentWorkArea();

    // Pointer
    SetPointer( POINTER_ARROW );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
X11SalFrame::X11SalFrame( SalFrame *pParent, sal_uLong nSalFrameStyle,
                          SystemParentData* pSystemParent ) :
    m_nXScreen( 0 )
{
    SalGenericData *pData = GetGenericData();

    // initialize frame geometry
    memset( &maGeometry, 0, sizeof(maGeometry) );

    mpParent                    = static_cast< X11SalFrame* >( pParent );

    mbTransientForRoot          = false;

    pDisplay_                   = pData->GetSalDisplay();
    // insert frame in framelist
    pDisplay_->registerFrame( this );

    mhWindow                    = None;
    mhShellWindow               = None;
    mhStackingWindow            = None;
    mhForeignParent             = None;
    mhBackgroundPixmap          = None;
    m_bSetFocusOnMap            = false;

    pGraphics_                  = NULL;
    pFreeGraphics_              = NULL;

    hCursor_                    = None;
    nCaptured_                  = 0;

     nReleaseTime_              = 0;
    nKeyCode_                   = 0;
    nKeyState_                  = 0;
    nCompose_                   = -1;
    mbSendExtKeyModChange       = false;
    mnExtKeyMod                 = 0;

    nShowState_                 = SHOWSTATE_UNKNOWN;
    nWidth_                     = 0;
    nHeight_                    = 0;
    nStyle_                     = 0;
    mnExtStyle                  = 0;
    bAlwaysOnTop_               = sal_False;

    // set bViewable_ to sal_True: hack GetClientSize to report something
    // different to 0/0 before first map
    bViewable_                  = sal_True;
    bMapped_                    = sal_False;
    bDefaultPosition_           = sal_True;
    nVisibility_                = VisibilityFullyObscured;
    m_nWorkArea                 = 0;
    mbInShow                    = sal_False;
    m_bXEmbed                   = false;

    nScreenSaversTimeout_       = 0;

    mpInputContext              = NULL;
    mbInputFocus                = False;

    maAlwaysOnTopRaiseTimer.SetTimeoutHdl( LINK( this, X11SalFrame, HandleAlwaysOnTopRaise ) );
    maAlwaysOnTopRaiseTimer.SetTimeout( 100 );

    meWindowType                = WMAdaptor::windowType_Normal;
    mnDecorationFlags           = WMAdaptor::decoration_All;
    mbMaximizedVert             = false;
    mbMaximizedHorz             = false;
    mbShaded                    = false;
    mbFullScreen                = false;

    mnIconID                    = 1; // ICON_LO_DEFAULT

    m_pClipRectangles           = NULL;
    m_nCurClipRect              = 0;
    m_nMaxClipRect              = 0;

    if( mpParent )
        mpParent->maChildren.push_back( this );

    Init( nSalFrameStyle, GetDisplay()->GetDefaultXScreen(), pSystemParent );
}

X11SalFrame::~X11SalFrame()
{
    notifyDelete();

    if( m_pClipRectangles )
    {
        delete [] m_pClipRectangles;
        m_pClipRectangles = NULL;
        m_nCurClipRect = m_nMaxClipRect = 0;
    }

    if( mhBackgroundPixmap )
    {
        XSetWindowBackgroundPixmap( GetXDisplay(), GetWindow(), None );
        XFreePixmap( GetXDisplay(), mhBackgroundPixmap );
    }

    if( mhStackingWindow )
        aPresentationReparentList.remove( mhStackingWindow );

    // remove from parent's list
    if( mpParent )
        mpParent->maChildren.remove( this );

    // deregister on SalDisplay
    pDisplay_->deregisterFrame( this );

    // unselect all events, some may be still in the queue anyway
    if( ! IsSysChildWindow() )
        XSelectInput( GetXDisplay(), GetShellWindow(), 0 );
    XSelectInput( GetXDisplay(), GetWindow(), 0 );

    ShowFullScreen( sal_False, 0 );

    if( bMapped_ )
        Show( sal_False );

    if( mpInputContext )
    {
        mpInputContext->UnsetICFocus( this );
        mpInputContext->Unmap( this );
        delete mpInputContext;
    }

    if( GetWindow() == hPresentationWindow )
    {
        hPresentationWindow = None;
        doReparentPresentationDialogues( GetDisplay() );
    }

    if( pGraphics_ )
    {
        pGraphics_->DeInit();
        delete pGraphics_;
    }

    if( pFreeGraphics_ )
    {
        pFreeGraphics_->DeInit();
        delete pFreeGraphics_;
    }


    XDestroyWindow( GetXDisplay(), mhWindow );

    /*
     *  check if there is only the status frame left
     *  if so, free it
     */
    if( ! GetDisplay()->getFrames().empty() && I18NStatus::exists() )
    {
        SalFrame* pStatusFrame = I18NStatus::get().getStatusFrame();
        std::list< SalFrame* >::const_iterator sit = GetDisplay()->getFrames().begin();
        if( pStatusFrame
            && *sit == pStatusFrame
            && ++sit == GetDisplay()->getFrames().end() )
            vcl::I18NStatus::free();
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::SetExtendedFrameStyle( SalExtStyle nStyle )
{
    if( nStyle != mnExtStyle && ! IsChildWindow() )
    {
        mnExtStyle = nStyle;
        updateWMClass();
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::SetBackgroundBitmap( SalBitmap* pBitmap )
{
    if( mhBackgroundPixmap )
    {
        XSetWindowBackgroundPixmap( GetXDisplay(), GetWindow(), None );
        XFreePixmap( GetXDisplay(), mhBackgroundPixmap );
        mhBackgroundPixmap = None;
    }
    if( pBitmap )
    {
        X11SalBitmap* pBM = static_cast<X11SalBitmap*>(pBitmap);
        Size aSize = pBM->GetSize();
        if( aSize.Width() && aSize.Height() )
        {
            mhBackgroundPixmap =
                limitXCreatePixmap( GetXDisplay(),
                               GetWindow(),
                               aSize.Width(),
                               aSize.Height(),
                               GetDisplay()->GetVisual( m_nXScreen ).GetDepth() );
            if( mhBackgroundPixmap )
            {
                SalTwoRect aTwoRect;
                aTwoRect.mnSrcX = aTwoRect.mnSrcY = aTwoRect.mnDestX = aTwoRect.mnDestY = 0;
                aTwoRect.mnSrcWidth = aTwoRect.mnDestWidth = aSize.Width();
                aTwoRect.mnSrcHeight = aTwoRect.mnDestHeight = aSize.Height();
                pBM->ImplDraw( mhBackgroundPixmap,
                               m_nXScreen,
                               GetDisplay()->GetVisual( m_nXScreen ).GetDepth(),
                               aTwoRect, GetDisplay()->GetCopyGC( m_nXScreen ) );
                XSetWindowBackgroundPixmap( GetXDisplay(), GetWindow(), mhBackgroundPixmap );
            }
        }
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

const SystemChildData* X11SalFrame::GetSystemData() const
{
    X11SalFrame *pFrame = const_cast<X11SalFrame*>(this);
    pFrame->maSystemChildData.nSize         = sizeof( SystemChildData );
    pFrame->maSystemChildData.pDisplay      = GetXDisplay();
    pFrame->maSystemChildData.aWindow       = pFrame->GetWindow();
    pFrame->maSystemChildData.pSalFrame     = pFrame;
    pFrame->maSystemChildData.pWidget       = NULL;
    pFrame->maSystemChildData.pVisual       = GetDisplay()->GetVisual( m_nXScreen ).GetVisual();
    pFrame->maSystemChildData.nScreen       = m_nXScreen.getXScreen();
    pFrame->maSystemChildData.nDepth        = GetDisplay()->GetVisual( m_nXScreen ).GetDepth();
    pFrame->maSystemChildData.aColormap     = GetDisplay()->GetColormap( m_nXScreen ).GetXColormap();
    pFrame->maSystemChildData.pAppContext   = NULL;
    pFrame->maSystemChildData.aShellWindow  = pFrame->GetShellWindow();
    pFrame->maSystemChildData.pShellWidget  = NULL;
    return &maSystemChildData;
}

SalGraphics *X11SalFrame::GetGraphics()
{
    if( pGraphics_ )
        return NULL;

    if( pFreeGraphics_ )
    {
        pGraphics_      = pFreeGraphics_;
        pFreeGraphics_  = NULL;
    }
    else
    {
        pGraphics_ = new X11SalGraphics();
        pGraphics_->Init( this, GetWindow(), m_nXScreen );
    }

    return pGraphics_;
}

void X11SalFrame::ReleaseGraphics( SalGraphics *pGraphics )
{
    DBG_ASSERT( pGraphics == pGraphics_, "SalFrame::ReleaseGraphics pGraphics!=pGraphics_" );

    if( pGraphics != pGraphics_ )
        return;

    pFreeGraphics_  = pGraphics_;
    pGraphics_      = NULL;
}

void X11SalFrame::updateGraphics( bool bClear )
{
    Drawable aDrawable = bClear ? None : GetWindow();
    if( pGraphics_ )
        pGraphics_->SetDrawable( aDrawable, m_nXScreen );
    if( pFreeGraphics_ )
        pFreeGraphics_->SetDrawable( aDrawable, m_nXScreen );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::Enable( sal_Bool /*bEnable*/ )
{
    // NYI: enable/disable frame
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::SetIcon( sal_uInt16 nIcon )
{
    if ( ! IsChildWindow() )
    {
        // 0 == default icon -> #1
        if ( nIcon == 0 )
            nIcon = 1;

        mnIconID = nIcon;

        XIconSize *pIconSize = NULL;
        int nSizes = 0;
        int iconSize = 32;
        if ( XGetIconSizes( GetXDisplay(), GetDisplay()->GetRootWindow( m_nXScreen ), &pIconSize, &nSizes ) )
        {
#if OSL_DEBUG_LEVEL > 1
            fprintf(stderr, "X11SalFrame::SetIcon(): found %d IconSizes:\n", nSizes);
#endif

            const int ourLargestIconSize = 48;

            int i;
            for( i=0; i<nSizes; i++)
            {
               // select largest supported icon

               // Note: olwm/olvwm reports a huge max icon size of
               // 160x160 pixels; always choosing the max as the
               // preferred icon size is apparently wrong under olvwm
               // - so we keep the safe default |iconSize| when we see
               // unreasonable large max icon sizes (> twice of our
               // largest available icon) reported by XGetIconSizes.
                if( pIconSize[i].max_width > iconSize
                    && pIconSize[i].max_width <= 2*ourLargestIconSize )
                {
                    iconSize = pIconSize[i].max_width;
                }
                iconSize = pIconSize[i].max_width;

#if OSL_DEBUG_LEVEL > 1
                fprintf(stderr, "min: %d, %d\nmax: %d, %d\ninc: %d, %d\n\n",
                        pIconSize[i].min_width, pIconSize[i].min_height,
                        pIconSize[i].max_width, pIconSize[i].max_height,
                        pIconSize[i].width_inc, pIconSize[i].height_inc);
#endif
            }

            XFree( pIconSize );
        }
        else
        {
            const String& rWM( pDisplay_->getWMAdaptor()->getWindowManagerName() );
            if( rWM.EqualsAscii( "KWin" ) )         // assume KDE is running
                iconSize = 48;
            static bool bGnomeIconSize = false;
            static bool bGnomeChecked = false;
            if( ! bGnomeChecked )
            {
                bGnomeChecked=true;
                int nCount = 0;
                Atom* pProps = XListProperties( GetXDisplay(),
                                                GetDisplay()->GetRootWindow( m_nXScreen ),
                                                &nCount );
                for( int i = 0; i < nCount && !bGnomeIconSize; i++ )
                 {
                    char* pName = XGetAtomName( GetXDisplay(), pProps[i] );
                    if( !strcmp( pName, "GNOME_PANEL_DESKTOP_AREA" ) )
                        bGnomeIconSize = true;
                    if( pName )
                        XFree( pName );
                 }
                if( pProps )
                    XFree( pProps );
            }
            if( bGnomeIconSize )
                iconSize = 48;
        }

        XWMHints Hints;
        Hints.flags = 0;
        XWMHints *pHints = XGetWMHints( GetXDisplay(), GetShellWindow() );
        if( pHints )
        {
            memcpy(&Hints, pHints, sizeof( XWMHints ));
            XFree( pHints );
        }
        pHints = &Hints;

        sal_Bool bOk = SelectAppIconPixmap( GetDisplay(), m_nXScreen,
                                        nIcon, iconSize,
                                        pHints->icon_pixmap, pHints->icon_mask );
        if ( !bOk )
        {
            // load default icon (0)
            bOk = SelectAppIconPixmap( GetDisplay(), m_nXScreen,
                                       0, iconSize,
                                       pHints->icon_pixmap, pHints->icon_mask );
        }
        if( bOk )
        {
            pHints->flags    |= IconPixmapHint;
            if( pHints->icon_mask )
                pHints->flags |= IconMaskHint;

            XSetWMHints( GetXDisplay(), GetShellWindow(), pHints );
        }
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
    if( ! IsChildWindow() )
    {
        if( GetShellWindow() && (nStyle_ & (SAL_FRAME_STYLE_FLOAT|SAL_FRAME_STYLE_OWNERDRAWDECORATION) ) != SAL_FRAME_STYLE_FLOAT )
        {
            XSizeHints* pHints = XAllocSizeHints();
            long nSupplied = 0;
            XGetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints,
                               &nSupplied
                               );
            pHints->max_width   = nWidth;
            pHints->max_height  = nHeight;
            pHints->flags |= PMaxSize;
            XSetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints );
            XFree( pHints );
        }
    }
}

void X11SalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    if( ! IsChildWindow() )
    {
        if( GetShellWindow() && (nStyle_ & (SAL_FRAME_STYLE_FLOAT|SAL_FRAME_STYLE_OWNERDRAWDECORATION) ) != SAL_FRAME_STYLE_FLOAT )
        {
            XSizeHints* pHints = XAllocSizeHints();
            long nSupplied = 0;
            XGetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints,
                               &nSupplied
                               );
            pHints->min_width   = nWidth;
            pHints->min_height  = nHeight;
            pHints->flags |= PMinSize;
            XSetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints );
            XFree( pHints );
        }
    }
}

// Show + Pos (x,y,z) + Size (width,height)
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::Show( sal_Bool bVisible, sal_Bool bNoActivate )
{
    if( ( bVisible && bMapped_ )
        || ( !bVisible && !bMapped_ ) )
        return;

    // HACK: this is a workaround for (at least) kwin
    // even though transient frames should be kept above their parent
    // this does not necessarily hold true for DOCK type windows
    // so artificially set ABOVE and remove it again on hide
    if( mpParent && (mpParent->nStyle_ & SAL_FRAME_STYLE_PARTIAL_FULLSCREEN ) && pDisplay_->getWMAdaptor()->isLegacyPartialFullscreen())
        pDisplay_->getWMAdaptor()->enableAlwaysOnTop( this, bVisible );

    bMapped_   = bVisible;
    bViewable_ = bVisible;
    setXEmbedInfo();
    if( bVisible )
    {
        mbInShow = sal_True;
        if( ! (nStyle_ & SAL_FRAME_STYLE_INTRO) )
        {
            // hide all INTRO frames
            const std::list< SalFrame* >& rFrames = GetDisplay()->getFrames();
            for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
            {
                const X11SalFrame* pFrame = static_cast< const X11SalFrame* >(*it);
                // look for intro bit map; if present, hide it
                if( pFrame->nStyle_ & SAL_FRAME_STYLE_INTRO )
                {
                    if( pFrame->bMapped_ )
                        const_cast<X11SalFrame*>(pFrame)->Show( sal_False );
                }
            }
        }

        // update NET_WM_STATE which may have been deleted due to earlier Show(sal_False)
        if( nShowState_ == SHOWSTATE_HIDDEN )
            GetDisplay()->getWMAdaptor()->frameIsMapping( this );

        /*
         *  Actually this is rather exotic and currently happens only in conjunction
         *  with the basic dialogue editor,
         *  which shows a frame and instantly hides it again. After that the
         *  editor window is shown and the WM takes this as an opportunity
         *  to show our hidden transient frame also. So Show( sal_False ) must
         *  withdraw the frame AND delete the WM_TRANSIENT_FOR property.
         *  In case the frame is shown again, the transient hint must be restored here.
         */
        if(    ! IsChildWindow()
            && ! IsOverrideRedirect()
            && ! IsFloatGrabWindow()
            && mpParent
            )
        {
            GetDisplay()->getWMAdaptor()->changeReferenceFrame( this, mpParent );
        }

        // #i45160# switch to desktop where a dialog with parent will appear
        if( mpParent && mpParent->m_nWorkArea != m_nWorkArea )
            GetDisplay()->getWMAdaptor()->switchToWorkArea( mpParent->m_nWorkArea );

        if( IsFloatGrabWindow() &&
            mpParent &&
            nVisibleFloats == 0 &&
            ! GetDisplay()->GetCaptureFrame() )
        {
            /* #i39420#
             * outsmart KWin's "focus strictly under mouse" mode
             * which insists on taking the focus from the document
             * to the new float. Grab focus to parent frame BEFORE
             * showing the float (cannot grab it to the float
             * before show).
             */
            XGrabPointer( GetXDisplay(),
                          mpParent->GetWindow(),
                          True,
                          PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                          GrabModeAsync,
                          GrabModeAsync,
                          None,
                          mpParent ? mpParent->GetCursor() : None,
                          CurrentTime
                          );
        }

        XLIB_Time nUserTime = 0;
        if( ! bNoActivate && (nStyle_ & (SAL_FRAME_STYLE_OWNERDRAWDECORATION)) == 0 )
            nUserTime = pDisplay_->GetLastUserEventTime( true );
        GetDisplay()->getWMAdaptor()->setUserTime( this, nUserTime );
        if( ! bNoActivate && (nStyle_ & SAL_FRAME_STYLE_TOOLWINDOW) )
            m_bSetFocusOnMap = true;

        // actually map the window
        if( m_bXEmbed )
            askForXEmbedFocus( 0 );
        else
        {
            if( GetWindow() != GetShellWindow() && ! IsSysChildWindow() )
            {
                if( IsChildWindow() )
                    XMapWindow( GetXDisplay(), GetShellWindow() );
                XSelectInput( GetXDisplay(), GetShellWindow(), CLIENT_EVENTS );
            }
            if( nStyle_ & SAL_FRAME_STYLE_FLOAT )
                XMapRaised( GetXDisplay(), GetWindow() );
            else
                XMapWindow( GetXDisplay(), GetWindow() );
        }
        XSelectInput( GetXDisplay(), GetWindow(), CLIENT_EVENTS );

        if( maGeometry.nWidth > 0
            && maGeometry.nHeight > 0
            && (   nWidth_  != (int)maGeometry.nWidth
                || nHeight_ != (int)maGeometry.nHeight ) )
        {
            nWidth_  = maGeometry.nWidth;
            nHeight_ = maGeometry.nHeight;
        }

        XSync( GetXDisplay(), False );

        if( IsFloatGrabWindow() )
        {
            /*
             *  Sawfish and twm can be switched to enter-exit focus behaviour. In this case
             *  we must grab the pointer else the dumb WM will put the focus to the
             *  override-redirect float window. The application window will be deactivated
             *  which causes that the floats are destroyed, so the user can never click on
             *  a menu because it vanishes as soon as he enters it.
             */
            nVisibleFloats++;
            if( nVisibleFloats == 1 && ! GetDisplay()->GetCaptureFrame() )
            {
                /* #i39420# now move grab to the new float window */
                XGrabPointer( GetXDisplay(),
                              GetWindow(),
                              True,
                              PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                              GrabModeAsync,
                              GrabModeAsync,
                              None,
                              mpParent ? mpParent->GetCursor() : None,
                              CurrentTime
                              );
            }
        }
        CallCallback( SALEVENT_RESIZE, NULL );

        /*
         *  sometimes a message box/dialogue is brought up when a frame is not mapped
         *  the corresponding TRANSIENT_FOR hint is then set to the root window
         *  so that the dialogue shows in all cases. Correct it here if the
         *  frame is shown afterwards.
         */
        if( ! IsChildWindow()
            && ! IsOverrideRedirect()
            && ! IsFloatGrabWindow()
            )
        {
            for( std::list< X11SalFrame* >::const_iterator it = maChildren.begin();
                 it != maChildren.end(); ++it )
            {
                if( (*it)->mbTransientForRoot )
                    GetDisplay()->getWMAdaptor()->changeReferenceFrame( *it, this );
            }
        }
        /*
         *  leave SHOWSTATE_UNKNOWN as this indicates first mapping
         *  and is only reset int HandleSizeEvent
         */
        if( nShowState_ != SHOWSTATE_UNKNOWN )
            nShowState_ = SHOWSTATE_NORMAL;

        /*
         *  plugged windows don't necessarily get the
         *  focus on show because the parent may already be mapped
         *  and have the focus. So try to set the focus
         *  to the child on Show(sal_True)
         */
        if( (nStyle_ & SAL_FRAME_STYLE_PLUG) && ! m_bXEmbed )
            XSetInputFocus( GetXDisplay(),
                            GetWindow(),
                            RevertToParent,
                            CurrentTime );

        if( mpParent )
        {
            // push this frame so it will be in front of its siblings
            // only necessary for insane transient behaviour of Dtwm/olwm
            mpParent->maChildren.remove( this );
            mpParent->maChildren.push_front(this);
        }
    }
    else
    {
        if( getInputContext() )
            getInputContext()->Unmap( this );

        if( ! IsChildWindow() )
        {
            /*  FIXME: Is deleting the property really necessary ? It hurts
             *  owner drawn windows at least.
             */
            if( mpParent && ! (nStyle_ & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
                XDeleteProperty( GetXDisplay(), GetShellWindow(), GetDisplay()->getWMAdaptor()->getAtom( WMAdaptor::WM_TRANSIENT_FOR ) );
            XWithdrawWindow( GetXDisplay(), GetShellWindow(), m_nXScreen.getXScreen() );
        }
        else if( ! m_bXEmbed )
            XUnmapWindow( GetXDisplay(), GetWindow() );

        nShowState_ = SHOWSTATE_HIDDEN;
        if( IsFloatGrabWindow() && nVisibleFloats )
        {
            nVisibleFloats--;
            if( nVisibleFloats == 0  && ! GetDisplay()->GetCaptureFrame() )
                XUngrabPointer( GetXDisplay(),
                                CurrentTime );
        }
        // flush here; there may be a very seldom race between
        // the display connection used for clipboard and our connection
        Flush();
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::ToTop( sal_uInt16 nFlags )
{
    if( ( nFlags & SAL_FRAME_TOTOP_RESTOREWHENMIN )
        && ! ( nStyle_ & SAL_FRAME_STYLE_FLOAT )
        && nShowState_ != SHOWSTATE_HIDDEN
        && nShowState_ != SHOWSTATE_UNKNOWN
        )
    {
        GetDisplay()->getWMAdaptor()->frameIsMapping( this );
        if( GetWindow() != GetShellWindow() && ! IsSysChildWindow() )
            XMapWindow( GetXDisplay(), GetShellWindow() );
        XMapWindow( GetXDisplay(), GetWindow() );
    }

    XLIB_Window aToTopWindow = IsSysChildWindow() ? GetWindow() : GetShellWindow();
    if( ! (nFlags & SAL_FRAME_TOTOP_GRABFOCUS_ONLY) )
    {
        XRaiseWindow( GetXDisplay(), aToTopWindow );
        if( ! GetDisplay()->getWMAdaptor()->isTransientBehaviourAsExpected() )
            for( std::list< X11SalFrame* >::const_iterator it = maChildren.begin();
                 it != maChildren.end(); ++it )
                (*it)->ToTop( nFlags & ~SAL_FRAME_TOTOP_GRABFOCUS );
    }

    if( ( ( nFlags & SAL_FRAME_TOTOP_GRABFOCUS ) || ( nFlags & SAL_FRAME_TOTOP_GRABFOCUS_ONLY ) )
        && bMapped_ )
    {
        if( m_bXEmbed )
            askForXEmbedFocus( 0 );
        else
            XSetInputFocus( GetXDisplay(), aToTopWindow, RevertToParent, CurrentTime );
    }
}
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::GetWorkArea( Rectangle& rWorkArea )
{
    rWorkArea = pDisplay_->getWMAdaptor()->getWorkArea( 0 );
}
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::GetClientSize( long &rWidth, long &rHeight )
{
    if( ! bViewable_  )
    {
        rWidth = rHeight = 0;
        return;
    }

    rWidth  = maGeometry.nWidth;
    rHeight = maGeometry.nHeight;

    if( !rWidth || !rHeight )
    {
        XWindowAttributes aAttrib;

        XGetWindowAttributes( GetXDisplay(), GetWindow(), &aAttrib );

        maGeometry.nWidth = rWidth = aAttrib.width;
        maGeometry.nHeight = rHeight = aAttrib.height;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::Center( )
{
    int             nX, nY, nScreenWidth, nScreenHeight;
    int             nRealScreenWidth, nRealScreenHeight;
    int             nScreenX = 0, nScreenY = 0;

    const Size& aScreenSize = GetDisplay()->getDataForScreen( m_nXScreen ).m_aSize;
    nScreenWidth        = aScreenSize.Width();
    nScreenHeight       = aScreenSize.Height();
    nRealScreenWidth    = nScreenWidth;
    nRealScreenHeight   = nScreenHeight;

    if( GetDisplay()->IsXinerama() )
    {
        // get xinerama screen we are on
        // if there is a parent, use its center for screen determination
        // else use the pointer
        XLIB_Window aRoot, aChild;
        int root_x, root_y, x, y;
        unsigned int mask;
        if( mpParent )
        {
            root_x = mpParent->maGeometry.nX + mpParent->maGeometry.nWidth/2;
            root_y = mpParent->maGeometry.nY + mpParent->maGeometry.nHeight/2;
        }
        else
            XQueryPointer( GetXDisplay(),
                           GetShellWindow(),
                           &aRoot, &aChild,
                           &root_x, &root_y,
                           &x, &y,
                           &mask );
        const std::vector< Rectangle >& rScreens = GetDisplay()->GetXineramaScreens();
        for( unsigned int i = 0; i < rScreens.size(); i++ )
            if( rScreens[i].IsInside( Point( root_x, root_y ) ) )
            {
                nScreenX            = rScreens[i].Left();
                nScreenY            = rScreens[i].Top();
                nRealScreenWidth    = rScreens[i].GetWidth();
                nRealScreenHeight   = rScreens[i].GetHeight();
                break;
            }
    }

    if( mpParent )
    {
        X11SalFrame* pFrame = mpParent;
        while( pFrame->mpParent )
            pFrame = pFrame->mpParent;
        if( pFrame->maGeometry.nWidth < 1  || pFrame->maGeometry.nHeight < 1 )
        {
            Rectangle aRect;
            pFrame->GetPosSize( aRect );
            pFrame->maGeometry.nX       = aRect.Left();
            pFrame->maGeometry.nY       = aRect.Top();
            pFrame->maGeometry.nWidth   = aRect.GetWidth();
            pFrame->maGeometry.nHeight  = aRect.GetHeight();
        }

        if( pFrame->nStyle_ & SAL_FRAME_STYLE_PLUG )
        {
            XLIB_Window aRoot;
            unsigned int bw, depth;
            XGetGeometry( GetXDisplay(),
                          pFrame->GetShellWindow(),
                          &aRoot,
                          &nScreenX, &nScreenY,
                          (unsigned int*)&nScreenWidth,
                          (unsigned int*)&nScreenHeight,
                          &bw, &depth );
        }
        else
        {
            nScreenX        = pFrame->maGeometry.nX;
            nScreenY        = pFrame->maGeometry.nY;
            nScreenWidth    = pFrame->maGeometry.nWidth;
            nScreenHeight   = pFrame->maGeometry.nHeight;
        }
    }

    if( mpParent && mpParent->nShowState_ == SHOWSTATE_NORMAL )
    {
        if( maGeometry.nWidth >= mpParent->maGeometry.nWidth &&
            maGeometry.nHeight >= mpParent->maGeometry.nHeight )
        {
            nX = nScreenX + 40;
            nY = nScreenY + 40;
        }
        else
        {
            // center the window relative to the top level frame
            nX = (nScreenWidth  - (int)maGeometry.nWidth ) / 2 + nScreenX;
            nY = (nScreenHeight - (int)maGeometry.nHeight) / 2 + nScreenY;
        }
    }
    else
    {
        // center the window relative to screen
        nX = (nRealScreenWidth  - (int)maGeometry.nWidth ) / 2 + nScreenX;
        nY = (nRealScreenHeight - (int)maGeometry.nHeight) / 2 + nScreenY;
    }
    nX = nX < 0 ? 0 : nX;
    nY = nY < 0 ? 0 : nY;

    bDefaultPosition_ = False;
    if( mpParent )
    {
        nX -= mpParent->maGeometry.nX;
        nY -= mpParent->maGeometry.nY;
    }

    Point aPoint(nX, nY);
    SetPosSize( Rectangle( aPoint, Size( maGeometry.nWidth, maGeometry.nHeight ) ) );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::updateScreenNumber()
{
    if( GetDisplay()->IsXinerama() && GetDisplay()->GetXineramaScreens().size() > 1 )
    {
        Point aPoint( maGeometry.nX, maGeometry.nY );
        const std::vector<Rectangle>& rScreenRects( GetDisplay()->GetXineramaScreens() );
        size_t nScreens = rScreenRects.size();
        for( size_t i = 0; i < nScreens; i++ )
        {
            if( rScreenRects[i].IsInside( aPoint ) )
            {
                maGeometry.nDisplayScreenNumber = static_cast<unsigned int>(i);
                break;
            }
        }
    }
    else
        maGeometry.nDisplayScreenNumber = m_nXScreen.getXScreen();
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags )
{
    if( nStyle_ & SAL_FRAME_STYLE_PLUG )
        return;

    // relative positioning in X11SalFrame::SetPosSize
    Rectangle aPosSize( Point( maGeometry.nX, maGeometry.nY ), Size( maGeometry.nWidth, maGeometry.nHeight ) );
    aPosSize.Justify();

    if( ! ( nFlags & SAL_FRAME_POSSIZE_X ) )
    {
        nX = aPosSize.Left();
        if( mpParent )
            nX -= mpParent->maGeometry.nX;
    }
    if( ! ( nFlags & SAL_FRAME_POSSIZE_Y ) )
    {
        nY = aPosSize.Top();
        if( mpParent )
            nY -= mpParent->maGeometry.nY;
    }
    if( ! ( nFlags & SAL_FRAME_POSSIZE_WIDTH ) )
        nWidth = aPosSize.GetWidth();
    if( ! ( nFlags & SAL_FRAME_POSSIZE_HEIGHT ) )
        nHeight = aPosSize.GetHeight();

    aPosSize = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );

    if( ! ( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) ) )
    {
        if( bDefaultPosition_ )
        {
            maGeometry.nWidth = aPosSize.GetWidth();
            maGeometry.nHeight = aPosSize.GetHeight();
            Center();
        }
        else
            SetSize( Size( nWidth, nHeight ) );
    }
    else
        SetPosSize( aPosSize );

    bDefaultPosition_ = False;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::SetAlwaysOnTop( sal_Bool bOnTop )
{
    if( ! IsOverrideRedirect() )
    {
        bAlwaysOnTop_ = bOnTop;
        pDisplay_->getWMAdaptor()->enableAlwaysOnTop( this, bOnTop );
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#define _FRAMESTATE_MASK_GEOMETRY \
     (SAL_FRAMESTATE_MASK_X     | SAL_FRAMESTATE_MASK_Y |   \
      SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT)
#define _FRAMESTATE_MASK_MAXIMIZED_GEOMETRY \
     (SAL_FRAMESTATE_MASK_MAXIMIZED_X     | SAL_FRAMESTATE_MASK_MAXIMIZED_Y |   \
      SAL_FRAMESTATE_MASK_MAXIMIZED_WIDTH | SAL_FRAMESTATE_MASK_MAXIMIZED_HEIGHT)

void X11SalFrame::SetWindowState( const SalFrameState *pState )
{
    if (pState == NULL)
        return;

    // Request for position or size change
    if (pState->mnMask & _FRAMESTATE_MASK_GEOMETRY)
    {
        Rectangle aPosSize;
        bool bDoAdjust = false;

        /* #i44325#
         * if maximized, set restore size and guess maximized size from last time
         * in state change below maximize window
         */
        if( ! IsChildWindow() &&
            (pState->mnMask & SAL_FRAMESTATE_MASK_STATE) &&
            (pState->mnState & SAL_FRAMESTATE_MAXIMIZED) &&
            (pState->mnMask & _FRAMESTATE_MASK_GEOMETRY) == _FRAMESTATE_MASK_GEOMETRY &&
            (pState->mnMask & _FRAMESTATE_MASK_MAXIMIZED_GEOMETRY) == _FRAMESTATE_MASK_MAXIMIZED_GEOMETRY
            )
        {
            XSizeHints* pHints = XAllocSizeHints();
            long nSupplied = 0;
            XGetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints,
                               &nSupplied );
            pHints->flags |= PPosition | PWinGravity;
            pHints->x           = pState->mnX;
            pHints->y           = pState->mnY;
            pHints->win_gravity = pDisplay_->getWMAdaptor()->getPositionWinGravity();
            XSetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints );
            XFree( pHints );

            XMoveResizeWindow( GetXDisplay(), GetShellWindow(),
                               pState->mnX, pState->mnY,
                               pState->mnWidth, pState->mnHeight );
            // guess maximized geometry from last time
            maGeometry.nX      = pState->mnMaximizedX;
            maGeometry.nY      = pState->mnMaximizedY;
            maGeometry.nWidth  = pState->mnMaximizedWidth;
            maGeometry.nHeight = pState->mnMaximizedHeight;
            updateScreenNumber();
        }
        else
        {
            // initialize with current geometry
            if ((pState->mnMask & _FRAMESTATE_MASK_GEOMETRY) != _FRAMESTATE_MASK_GEOMETRY)
                GetPosSize (aPosSize);

            // change requested properties
            if (pState->mnMask & SAL_FRAMESTATE_MASK_X)
            {
                aPosSize.setX (pState->mnX);
            }
            if (pState->mnMask & SAL_FRAMESTATE_MASK_Y)
            {
                aPosSize.setY (pState->mnY);
            }
            if (pState->mnMask & SAL_FRAMESTATE_MASK_WIDTH)
            {
                long nWidth = pState->mnWidth > 0 ? pState->mnWidth  - 1 : 0;
                aPosSize.setWidth (nWidth);
                bDoAdjust = true;
            }
            if (pState->mnMask & SAL_FRAMESTATE_MASK_HEIGHT)
            {
                int nHeight = pState->mnHeight > 0 ? pState->mnHeight - 1 : 0;
                aPosSize.setHeight (nHeight);
                bDoAdjust = true;
            }

            const Size& aScreenSize = pDisplay_->getDataForScreen( m_nXScreen ).m_aSize;

            if( bDoAdjust && aPosSize.GetWidth() <= aScreenSize.Width()
                && aPosSize.GetHeight() <= aScreenSize.Height() )
            {
                SalFrameGeometry aGeom = maGeometry;

                if( ! (nStyle_ & ( SAL_FRAME_STYLE_FLOAT | SAL_FRAME_STYLE_PLUG ) ) &&
                    mpParent &&
                aGeom.nLeftDecoration == 0 &&
                aGeom.nTopDecoration == 0 )
                {
                    aGeom = mpParent->maGeometry;
                    if( aGeom.nLeftDecoration == 0 &&
                        aGeom.nTopDecoration == 0 )
                    {
                        aGeom.nLeftDecoration = 5;
                        aGeom.nTopDecoration = 20;
                        aGeom.nRightDecoration = 5;
                        aGeom.nBottomDecoration = 5;
                    }
                }

                // adjust position so that frame fits onto screen
                if( aPosSize.Right()+(long)aGeom.nRightDecoration > aScreenSize.Width()-1 )
                    aPosSize.Move( (long)aScreenSize.Width() - (long)aPosSize.Right() - (long)aGeom.nRightDecoration, 0 );
                if( aPosSize.Bottom()+(long)aGeom.nBottomDecoration > aScreenSize.Height()-1 )
                    aPosSize.Move( 0, (long)aScreenSize.Height() - (long)aPosSize.Bottom() - (long)aGeom.nBottomDecoration );
                if( aPosSize.Left() < (long)aGeom.nLeftDecoration )
                    aPosSize.Move( (long)aGeom.nLeftDecoration - (long)aPosSize.Left(), 0 );
                if( aPosSize.Top() < (long)aGeom.nTopDecoration )
                    aPosSize.Move( 0, (long)aGeom.nTopDecoration - (long)aPosSize.Top() );
            }

            SetPosSize( 0, 0, aPosSize.GetWidth(), aPosSize.GetHeight(), SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
        }
    }

    // request for status change
    if (pState->mnMask & SAL_FRAMESTATE_MASK_STATE)
    {
        if (pState->mnState & SAL_FRAMESTATE_MAXIMIZED)
        {
            nShowState_ = SHOWSTATE_NORMAL;
            if( ! (pState->mnState & (SAL_FRAMESTATE_MAXIMIZED_HORZ|SAL_FRAMESTATE_MAXIMIZED_VERT) ) )
                Maximize();
            else
            {
                bool bHorz = (pState->mnState & SAL_FRAMESTATE_MAXIMIZED_HORZ) ? true : false;
                bool bVert = (pState->mnState & SAL_FRAMESTATE_MAXIMIZED_VERT) ? true : false;
                GetDisplay()->getWMAdaptor()->maximizeFrame( this, bHorz, bVert );
            }
            maRestorePosSize.Left() = pState->mnX;
            maRestorePosSize.Top() = pState->mnY;
            maRestorePosSize.Right() = maRestorePosSize.Left() + pState->mnWidth;
            maRestorePosSize.Right() = maRestorePosSize.Left() + pState->mnHeight;
        }
        else if( mbMaximizedHorz || mbMaximizedVert )
            GetDisplay()->getWMAdaptor()->maximizeFrame( this, false, false );

        if (pState->mnState & SAL_FRAMESTATE_MINIMIZED)
        {
            if (nShowState_ == SHOWSTATE_UNKNOWN)
                nShowState_ = SHOWSTATE_NORMAL;
            Minimize();
        }
        if (pState->mnState & SAL_FRAMESTATE_NORMAL)
        {
            if (nShowState_ != SHOWSTATE_NORMAL)
                Restore();
        }
        if (pState->mnState & SAL_FRAMESTATE_ROLLUP)
            GetDisplay()->getWMAdaptor()->shade( this, true );
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
sal_Bool X11SalFrame::GetWindowState( SalFrameState* pState )
{
    if( SHOWSTATE_MINIMIZED == nShowState_ )
        pState->mnState = SAL_FRAMESTATE_MINIMIZED;
    else
        pState->mnState = SAL_FRAMESTATE_NORMAL;

    Rectangle aPosSize;
    if( maRestorePosSize.IsEmpty() )
        GetPosSize( aPosSize );
    else
        aPosSize = maRestorePosSize;

    if( mbMaximizedHorz )
        pState->mnState |= SAL_FRAMESTATE_MAXIMIZED_HORZ;
    if( mbMaximizedVert )
        pState->mnState |= SAL_FRAMESTATE_MAXIMIZED_VERT;
    if( mbShaded )
        pState->mnState |= SAL_FRAMESTATE_ROLLUP;

    pState->mnX      = aPosSize.Left();
    pState->mnY      = aPosSize.Top();
    pState->mnWidth  = aPosSize.GetWidth();
    pState->mnHeight = aPosSize.GetHeight();

    pState->mnMask   = _FRAMESTATE_MASK_GEOMETRY | SAL_FRAMESTATE_MASK_STATE;


    if (! maRestorePosSize.IsEmpty() )
    {
        GetPosSize( aPosSize );
        pState->mnState |= SAL_FRAMESTATE_MAXIMIZED;
        pState->mnMaximizedX      = aPosSize.Left();
        pState->mnMaximizedY      = aPosSize.Top();
        pState->mnMaximizedWidth  = aPosSize.GetWidth();
        pState->mnMaximizedHeight = aPosSize.GetHeight();
        pState->mnMask |= _FRAMESTATE_MASK_MAXIMIZED_GEOMETRY;
    }

    return sal_True;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// native menu implementation - currently empty
void X11SalFrame::DrawMenuBar()
{
}

void X11SalFrame::SetMenu( SalMenu* )
{
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::GetPosSize( Rectangle &rPosSize )
{
    if( maGeometry.nWidth < 1 || maGeometry.nHeight < 1 )
    {
        const Size& aScreenSize = pDisplay_->getDataForScreen( m_nXScreen ).m_aSize;
        long w = aScreenSize.Width()  - maGeometry.nLeftDecoration - maGeometry.nRightDecoration;
        long h = aScreenSize.Height() - maGeometry.nTopDecoration - maGeometry.nBottomDecoration;

        rPosSize = Rectangle( Point( maGeometry.nX, maGeometry.nY ), Size( w, h ) );
    }
    else
        rPosSize = Rectangle( Point( maGeometry.nX, maGeometry.nY ),
                              Size( maGeometry.nWidth, maGeometry.nHeight ) );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::SetSize( const Size &rSize )
{
    if( rSize.Width() > 0 && rSize.Height() > 0 )
    {
         if( ! ( nStyle_ & SAL_FRAME_STYLE_SIZEABLE )
            && ! IsChildWindow()
            && ( nStyle_ & (SAL_FRAME_STYLE_FLOAT|SAL_FRAME_STYLE_OWNERDRAWDECORATION) ) != SAL_FRAME_STYLE_FLOAT )
         {
            XSizeHints* pHints = XAllocSizeHints();
            long nSupplied = 0;
            XGetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints,
                               &nSupplied
                               );
            pHints->min_width   = rSize.Width();
            pHints->min_height  = rSize.Height();
            pHints->max_width   = rSize.Width();
            pHints->max_height  = rSize.Height();
            pHints->flags |= PMinSize | PMaxSize;
            XSetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints );
            XFree( pHints );
         }
        XResizeWindow( GetXDisplay(), IsSysChildWindow() ? GetWindow() : GetShellWindow(), rSize.Width(), rSize.Height() );
        if( GetWindow() != GetShellWindow() )
        {
            if( (nStyle_ & SAL_FRAME_STYLE_PLUG ) )
                XMoveResizeWindow( GetXDisplay(), GetWindow(), 0, 0, rSize.Width(), rSize.Height() );
            else
                XResizeWindow( GetXDisplay(), GetWindow(), rSize.Width(), rSize.Height() );
        }

        maGeometry.nWidth  = rSize.Width();
        maGeometry.nHeight = rSize.Height();

        // allow the external status window to reposition
        if (mbInputFocus && mpInputContext != NULL)
            mpInputContext->SetICFocus ( this );
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::SetPosSize( const Rectangle &rPosSize )
{
    XWindowChanges values;
    values.x        = rPosSize.Left();
    values.y        = rPosSize.Top();
    values.width    = rPosSize.GetWidth();
    values.height   = rPosSize.GetHeight();

    if( !values.width || !values.height )
        return;

     if( mpParent && ! IsSysChildWindow() )
     {
        // --- RTL --- (mirror window pos)
        if( Application::GetSettings().GetLayoutRTL() )
            values.x = mpParent->maGeometry.nWidth-values.width-1-values.x;

         XLIB_Window aChild;
         // coordinates are relative to parent, so translate to root coordinates
         XTranslateCoordinates( GetDisplay()->GetDisplay(),
                                mpParent->GetWindow(),
                                GetDisplay()->GetRootWindow( m_nXScreen ),
                                values.x, values.y,
                                &values.x, &values.y,
                                & aChild );
     }

    bool bMoved = false;
    bool bSized = false;
    if( values.x != maGeometry.nX || values.y != maGeometry.nY )
        bMoved = true;
    if( values.width != (int)maGeometry.nWidth || values.height != (int)maGeometry.nHeight )
        bSized = true;

    if( ! ( nStyle_ & ( SAL_FRAME_STYLE_PLUG | SAL_FRAME_STYLE_FLOAT ) )
        && !(pDisplay_->GetProperties() & PROPERTY_SUPPORT_WM_ClientPos) )
    {
        values.x    -= maGeometry.nLeftDecoration;
        values.y    -= maGeometry.nTopDecoration;
    }

    // do net set WMNormalHints for ..
    if(
        // child windows
        ! IsChildWindow()
        // popups (menu, help window, etc.)
        &&  (nStyle_ & (SAL_FRAME_STYLE_FLOAT|SAL_FRAME_STYLE_OWNERDRAWDECORATION) ) != SAL_FRAME_STYLE_FLOAT
        // shown, sizeable windows
        && ( nShowState_ == SHOWSTATE_UNKNOWN ||
             nShowState_ == SHOWSTATE_HIDDEN ||
             ! ( nStyle_ & SAL_FRAME_STYLE_SIZEABLE )
             )
        )
    {
        XSizeHints* pHints = XAllocSizeHints();
        long nSupplied = 0;
        XGetWMNormalHints( GetXDisplay(),
                           GetShellWindow(),
                           pHints,
                           &nSupplied
                           );
        if( ! ( nStyle_ & SAL_FRAME_STYLE_SIZEABLE ) )
        {
            pHints->min_width   = rPosSize.GetWidth();
            pHints->min_height  = rPosSize.GetHeight();
            pHints->max_width   = rPosSize.GetWidth();
            pHints->max_height  = rPosSize.GetHeight();
            pHints->flags |= PMinSize | PMaxSize;
        }
        if( nShowState_ == SHOWSTATE_UNKNOWN || nShowState_ == SHOWSTATE_HIDDEN )
        {
            pHints->flags |= PPosition | PWinGravity;
            pHints->x           = values.x;
            pHints->y           = values.y;
            pHints->win_gravity = pDisplay_->getWMAdaptor()->getPositionWinGravity();
        }
        if( mbFullScreen )
        {
            pHints->max_width   = 10000;
            pHints->max_height  = 10000;
            pHints->flags |= PMaxSize;
        }
        XSetWMNormalHints( GetXDisplay(),
                           GetShellWindow(),
                           pHints );
        XFree( pHints );
    }

    XMoveResizeWindow( GetXDisplay(), IsSysChildWindow() ? GetWindow() : GetShellWindow(), values.x, values.y, values.width, values.height );
    if( GetShellWindow() != GetWindow() )
    {
        if( (nStyle_ & SAL_FRAME_STYLE_PLUG ) )
            XMoveResizeWindow( GetXDisplay(), GetWindow(), 0, 0, values.width, values.height );
        else
            XMoveResizeWindow( GetXDisplay(), GetWindow(), values.x, values.y, values.width, values.height );
    }

    maGeometry.nX       = values.x;
    maGeometry.nY       = values.y;
    maGeometry.nWidth   = values.width;
    maGeometry.nHeight  = values.height;
    if( IsSysChildWindow() && mpParent )
    {
        // translate back to root coordinates
        maGeometry.nX += mpParent->maGeometry.nX;
        maGeometry.nY += mpParent->maGeometry.nY;
    }

    updateScreenNumber();
    if( bSized && ! bMoved )
        CallCallback( SALEVENT_RESIZE, NULL );
    else if( bMoved && ! bSized )
        CallCallback( SALEVENT_MOVE, NULL );
    else
        CallCallback( SALEVENT_MOVERESIZE, NULL );

    // allow the external status window to reposition
    if (mbInputFocus && mpInputContext != NULL)
        mpInputContext->SetICFocus ( this );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::Minimize()
{
    if( IsSysChildWindow() )
        return;

    if( SHOWSTATE_UNKNOWN == nShowState_ || SHOWSTATE_HIDDEN == nShowState_ )
    {
        stderr0( "X11SalFrame::Minimize on withdrawn window\n" );
        return;
    }

    if( XIconifyWindow( GetXDisplay(),
                        GetShellWindow(),
                        pDisplay_->GetDefaultXScreen().getXScreen() ) )
        nShowState_ = SHOWSTATE_MINIMIZED;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::Maximize()
{
    if( IsSysChildWindow() )
        return;

    if( SHOWSTATE_MINIMIZED == nShowState_ )
    {
        GetDisplay()->getWMAdaptor()->frameIsMapping( this );
        XMapWindow( GetXDisplay(), GetShellWindow() );
        nShowState_ = SHOWSTATE_NORMAL;
    }

    pDisplay_->getWMAdaptor()->maximizeFrame( this, true, true );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::Restore()
{
    if( IsSysChildWindow() )
        return;

    if( SHOWSTATE_UNKNOWN == nShowState_ || SHOWSTATE_HIDDEN == nShowState_ )
    {
        stderr0( "X11SalFrame::Restore on withdrawn window\n" );
        return;
    }

    if( SHOWSTATE_MINIMIZED == nShowState_ )
    {
        GetDisplay()->getWMAdaptor()->frameIsMapping( this );
        XMapWindow( GetXDisplay(), GetShellWindow() );
        nShowState_ = SHOWSTATE_NORMAL;
    }

    pDisplay_->getWMAdaptor()->maximizeFrame( this, false, false );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::SetScreenNumber( unsigned int nNewScreen )
{
    if( nNewScreen == maGeometry.nDisplayScreenNumber )
        return;

    if( GetDisplay()->IsXinerama() && GetDisplay()->GetXineramaScreens().size() > 1 )
    {
        if( nNewScreen >= GetDisplay()->GetXineramaScreens().size() )
            return;

        Rectangle aOldScreenRect( GetDisplay()->GetXineramaScreens()[maGeometry.nDisplayScreenNumber] );
        Rectangle aNewScreenRect( GetDisplay()->GetXineramaScreens()[nNewScreen] );
        bool bVisible = bMapped_;
        if( bVisible )
            Show( sal_False );
        maGeometry.nX = aNewScreenRect.Left() + (maGeometry.nX - aOldScreenRect.Left());
        maGeometry.nY = aNewScreenRect.Top() + (maGeometry.nY - aOldScreenRect.Top());
        createNewWindow( None, m_nXScreen );
        if( bVisible )
            Show( sal_True );
        maGeometry.nDisplayScreenNumber = nNewScreen;
    }
    else if( nNewScreen < GetDisplay()->GetXScreenCount() )
    {
        bool bVisible = bMapped_;
        if( bVisible )
            Show( sal_False );
        createNewWindow( None, SalX11Screen( nNewScreen ) );
        if( bVisible )
            Show( sal_True );
        maGeometry.nDisplayScreenNumber = nNewScreen;
    }
}

void X11SalFrame::SetApplicationID( const rtl::OUString &rWMClass )
{
    if( rWMClass != m_sWMClass && ! IsChildWindow() )
    {
        m_sWMClass = rWMClass;
        updateWMClass();
        std::list< X11SalFrame* >::const_iterator it;
        for( it = maChildren.begin(); it != maChildren.end(); ++it )
            (*it)->SetApplicationID(rWMClass);
    }
}

void X11SalFrame::updateWMClass()
{
    XClassHint* pClass = XAllocClassHint();
    rtl::OString aResName = SalGenericSystem::getFrameResName( mnExtStyle );
    pClass->res_name  = const_cast<char*>(aResName.getStr());

    rtl::OString aResClass = rtl::OUStringToOString(m_sWMClass, RTL_TEXTENCODING_ASCII_US);
    const char *pResClass = !aResClass.isEmpty() ? aResClass.getStr() :
                            SalGenericSystem::getFrameClassName();

    pClass->res_class = const_cast<char*>(pResClass);
    XSetClassHint( GetXDisplay(), GetShellWindow(), pClass );
    XFree( pClass );
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::ShowFullScreen( sal_Bool bFullScreen, sal_Int32 nScreen )
{
    if( GetDisplay()->IsXinerama() && GetDisplay()->GetXineramaScreens().size() > 1 )
    {
        if( mbFullScreen == (bool)bFullScreen )
            return;
        if( bFullScreen )
        {
            maRestorePosSize = Rectangle( Point( maGeometry.nX, maGeometry.nY ),
                                          Size( maGeometry.nWidth, maGeometry.nHeight ) );
            Rectangle aRect;
            if( nScreen < 0 || nScreen >= static_cast<int>(GetDisplay()->GetXineramaScreens().size()) )
                aRect = Rectangle( Point(0,0), GetDisplay()->GetScreenSize( m_nXScreen ) );
            else
                aRect = GetDisplay()->GetXineramaScreens()[nScreen];
            nStyle_ |= SAL_FRAME_STYLE_PARTIAL_FULLSCREEN;
            bool bVisible = bMapped_;
            if( bVisible )
                Show( sal_False );
            maGeometry.nX = aRect.Left();
            maGeometry.nY = aRect.Top();
            maGeometry.nWidth = aRect.GetWidth();
            maGeometry.nHeight = aRect.GetHeight();
            mbMaximizedHorz = mbMaximizedVert = false;
            mbFullScreen = true;
            createNewWindow( None, m_nXScreen );
            if( GetDisplay()->getWMAdaptor()->isLegacyPartialFullscreen() )
                GetDisplay()->getWMAdaptor()->enableAlwaysOnTop( this, true );
            else
                GetDisplay()->getWMAdaptor()->showFullScreen( this, true );
            if( bVisible )
                Show(sal_True);

        }
        else
        {
            mbFullScreen = false;
            nStyle_ &= ~SAL_FRAME_STYLE_PARTIAL_FULLSCREEN;
            bool bVisible = bMapped_;
            Rectangle aRect = maRestorePosSize;
            maRestorePosSize = Rectangle();
            if( bVisible )
                Show( sal_False );
            createNewWindow( None, m_nXScreen );
            if( !aRect.IsEmpty() )
                SetPosSize( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(),
                            SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y |
                            SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
            if( bVisible )
                Show( sal_True );
        }
    }
    else
    {
        if( nScreen < 0 || nScreen >= (int)GetDisplay()->GetXScreenCount() )
            nScreen = m_nXScreen.getXScreen();
        if( nScreen != (int)m_nXScreen.getXScreen() )
        {
            bool bVisible = bMapped_;
            if( mbFullScreen )
                pDisplay_->getWMAdaptor()->showFullScreen( this, false );
            if( bVisible )
                Show( sal_False );
            createNewWindow( None, SalX11Screen( nScreen ) );
            if( mbFullScreen )
                pDisplay_->getWMAdaptor()->showFullScreen( this, true );
            if( bVisible )
                Show( sal_True );
        }
        if( mbFullScreen == (bool)bFullScreen )
            return;

        pDisplay_->getWMAdaptor()->showFullScreen( this, bFullScreen );
        if( IsOverrideRedirect()
            && WMSupportsFWS( GetXDisplay(), GetDisplay()->GetRootWindow( m_nXScreen ) ) )
        {
            AddFwsProtocols( GetXDisplay(), GetShellWindow() );
            RegisterFwsWindow( GetXDisplay(), GetShellWindow() );
        }
    }
}

/* ---------------------------------------------------------------------
   the xautolock pseudo screen saver needs special treatment since it
   doesn't cooperate with XxxxScreenSaver settings
   ------------------------------------------------------------------- */

static Bool
IsRunningXAutoLock( Display *p_display, XLIB_Window a_window )
{
    const char *p_atomname = "XAUTOLOCK_SEMAPHORE_PID";
    Atom        a_pidatom;

    // xautolock interns this atom
    a_pidatom    = XInternAtom( p_display, p_atomname, True );
    if ( a_pidatom == None )
        return False;

    Atom          a_type;
    int           n_format;
    unsigned long n_items;
    unsigned long n_bytes_after;
    pid_t        *p_pid;
    pid_t         n_pid;
    // get pid of running xautolock
    XGetWindowProperty (p_display, a_window, a_pidatom, 0L, 2L, False,
            AnyPropertyType, &a_type, &n_format, &n_items, &n_bytes_after,
            (unsigned char**) &p_pid );
    n_pid = *p_pid;
    XFree( p_pid );

      if ( a_type == XA_INTEGER )
      {
        // check if xautolock pid points to a running process
        if ( kill(n_pid, 0) == -1 )
            return False;
        else
            return True;
    }

    return False;
}

/* definitions from xautolock.c (pl15) */
#define XAUTOLOCK_DISABLE 1
#define XAUTOLOCK_ENABLE  2

static Bool
MessageToXAutoLock( Display *p_display, int n_message )
{
    const char *p_atomname = "XAUTOLOCK_MESSAGE" ;
    Atom        a_messageatom;
    XLIB_Window a_rootwindow;

    a_rootwindow = RootWindowOfScreen( ScreenOfDisplay(p_display, 0) );
    if ( ! IsRunningXAutoLock(p_display, a_rootwindow) )
    {
        // remove any pending messages
        a_messageatom = XInternAtom( p_display, p_atomname, True );
        if ( a_messageatom != None )
            XDeleteProperty( p_display, a_rootwindow, a_messageatom );
        return False;
    }

    a_messageatom = XInternAtom( p_display, p_atomname, False );
    XChangeProperty (p_display, a_rootwindow, a_messageatom, XA_INTEGER,
            8, PropModeReplace, (unsigned char*)&n_message, sizeof(n_message) );

    return True;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::StartPresentation( sal_Bool bStart )
{
    I18NStatus::get().show( !bStart, I18NStatus::presentation );
    if ( bStart )
        MessageToXAutoLock( GetXDisplay(), XAUTOLOCK_DISABLE );
    else
        MessageToXAutoLock( GetXDisplay(), XAUTOLOCK_ENABLE );

    if( ! bStart && hPresentationWindow != None )
        doReparentPresentationDialogues( GetDisplay() );
    hPresentationWindow = (bStart && IsOverrideRedirect() ) ? GetWindow() : None;


    // needs static here to save DPMS settings
    int dummy;
    static bool DPMSExtensionAvailable =
#if !defined(SOLARIS) && !defined(AIX)
        (DPMSQueryExtension(GetXDisplay(), &dummy, &dummy) != 0);
    static sal_Bool DPMSEnabled = false;
#else
        false;
    bool DPMSEnabled = false;
    (void)dummy;
#define CARD16 unsigned short
#endif
    static CARD16 dpms_standby_timeout=0;
    static CARD16 dpms_suspend_timeout=0;
    static CARD16 dpms_off_timeout=0;


    if( bStart || nScreenSaversTimeout_ || DPMSEnabled)
    {
        if( hPresentationWindow )
        {
            /*  #i10559# workaround for WindowMaker: try to restore
             *  current focus after presentation window is gone
             */
            int revert_to = 0;
            XGetInputFocus( GetXDisplay(), &hPresFocusWindow, &revert_to );
        }
        int timeout, interval, prefer_blanking, allow_exposures;
        XGetScreenSaver( GetXDisplay(),
                         &timeout,
                         &interval,
                         &prefer_blanking,
                         &allow_exposures );


        // get the DPMS state right before the start
        if (DPMSExtensionAvailable)
        {
#if !defined(SOLARIS) && !defined(AIX)
            CARD16 state; // card16 is defined in Xdm.h
            DPMSInfo(   GetXDisplay(),
                        &state,
                        &DPMSEnabled);
#endif
        }
        if( bStart ) // start show
        {
            if ( timeout )
            {
                nScreenSaversTimeout_ = timeout;
                XResetScreenSaver( GetXDisplay() );
                XSetScreenSaver( GetXDisplay(),
                                 0,
                                 interval,
                                 prefer_blanking,
                                 allow_exposures );
            }
#if !defined(SOLARIS) && !defined(AIX)
            if( DPMSEnabled )
            {
                if ( DPMSExtensionAvailable )
                {
                    DPMSGetTimeouts(    GetXDisplay(),
                                        &dpms_standby_timeout,
                                        &dpms_suspend_timeout,
                                        &dpms_off_timeout);
                    DPMSSetTimeouts(GetXDisplay(), 0,0,0);
                }
            }
#endif
        }
        else
        {
            if( nScreenSaversTimeout_ )
            {
                XSetScreenSaver( GetXDisplay(),
                             nScreenSaversTimeout_,
                             interval,
                             prefer_blanking,
                             allow_exposures );
                nScreenSaversTimeout_ = 0;
            }
#if !defined(SOLARIS) && !defined(AIX)
            if ( DPMSEnabled )
            {
                if ( DPMSExtensionAvailable )
                {
                // restore timeouts
                    DPMSSetTimeouts(GetXDisplay(), dpms_standby_timeout,
                        dpms_suspend_timeout, dpms_off_timeout);
                }
            }
#endif
        }
    }
}

// Pointer
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::SetPointer( PointerStyle ePointerStyle )
{
    hCursor_ = pDisplay_->GetPointer( ePointerStyle );
    XDefineCursor( GetXDisplay(), GetWindow(), hCursor_ );

    if( IsCaptured() || nVisibleFloats > 0 )
        XChangeActivePointerGrab( GetXDisplay(),
                        PointerMotionMask|ButtonPressMask|ButtonReleaseMask,
                        hCursor_,
                        CurrentTime );
}

void X11SalFrame::SetPointerPos(long nX, long nY)
{
    /* when the application tries to center the mouse in the dialog the
     * window isn't mapped already. So use coordinates relative to the root window.
     */
    unsigned int nWindowLeft = maGeometry.nX + nX;
    unsigned int nWindowTop  = maGeometry.nY + nY;

    XWarpPointer( GetXDisplay(), None, pDisplay_->GetRootWindow( pDisplay_->GetDefaultXScreen() ),
                  0, 0, 0, 0, nWindowLeft, nWindowTop);
}

// delay handling of extended text input
#if !defined(__synchronous_extinput__)
void
X11SalFrame::HandleExtTextEvent (XClientMessageEvent *pEvent)
{
    #if SAL_TYPES_SIZEOFLONG > 4
    void* pExtTextEvent = (void*)(  (pEvent->data.l[0] & 0xffffffff)
                                  | (pEvent->data.l[1] << 32) );
    #else
    void* pExtTextEvent = (void*)(pEvent->data.l[0]);
    #endif
    sal_uInt16 nExtTextEventType = sal_uInt16(pEvent->data.l[2]);

    CallCallback(nExtTextEventType, pExtTextEvent);

    switch (nExtTextEventType)
    {
        case SALEVENT_ENDEXTTEXTINPUT:
            break;

        case SALEVENT_EXTTEXTINPUT:
            break;

        default:

            fprintf(stderr, "X11SalFrame::HandleExtTextEvent: invalid extended input\n");
    }
}
#endif /* defined(__synchronous_extinput__) */

// PostEvent
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
sal_Bool X11SalFrame::PostEvent( void *pData )
{
    GetDisplay()->SendInternalEvent( this, pData );
    return sal_True;
}

// Title
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalFrame::SetTitle( const rtl::OUString& rTitle )
{
    if( ! ( IsChildWindow() || (nStyle_ & SAL_FRAME_STYLE_FLOAT ) ) )
    {
        m_aTitle = rTitle;
        GetDisplay()->getWMAdaptor()->setWMName( this, rTitle );
    }
}

// -----------------------------------------------------------------------

void X11SalFrame::Flush()
{
    XFlush( GetDisplay()->GetDisplay() );
}

// -----------------------------------------------------------------------

void X11SalFrame::Sync()
{
    XSync( GetDisplay()->GetDisplay(), False );
}

// Keyboard
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// -----------------------------------------------------------------------

void X11SalFrame::SetInputContext( SalInputContext* pContext )
{
      if (pContext == NULL)
        return;

      // 1. We should create an input context for this frame
      //    only when SAL_INPUTCONTEXT_TEXT is set.

      if (!(pContext->mnOptions & SAL_INPUTCONTEXT_TEXT))
    {
        if( mpInputContext )
            mpInputContext->Unmap( this );
        return;
    }

    // 2. We should use on-the-spot inputstyle
      //    only when SAL_INPUTCONTEXT_EXTTEXTINPUT is set.

      if (mpInputContext == NULL)
    {
        I18NStatus& rStatus( I18NStatus::get() );
        rStatus.setParent( this );
        mpInputContext = new SalI18N_InputContext( this );
        if (mpInputContext->UseContext())
        {
              mpInputContext->ExtendEventMask( GetShellWindow() );
              if (pContext->mnOptions & SAL_INPUTCONTEXT_CHANGELANGUAGE)
                mpInputContext->SetLanguage(pContext->meLanguage);
            if (mbInputFocus)
                mpInputContext->SetICFocus( this );
        }
      }
    else
        mpInputContext->Map( this );
      return;
}

// -----------------------------------------------------------------------

void X11SalFrame::EndExtTextInput( sal_uInt16 nFlags )
{
    if (mpInputContext != NULL)
          mpInputContext->EndExtTextInput( nFlags );
}

// -----------------------------------------------------------------------

rtl::OUString X11SalFrame::GetKeyName( sal_uInt16 nKeyCode )
{
    return GetDisplay()->GetKeyName( nKeyCode );
}

sal_Bool X11SalFrame::MapUnicodeToKeyCode( sal_Unicode , LanguageType , KeyCode& )
{
    // not supported yet
    return sal_False;
}

LanguageType X11SalFrame::GetInputLanguage()
{
    // could be improved by checking unicode ranges of the last input
    return LANGUAGE_DONTKNOW;
}

// Settings
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

inline Color getColorFromLong( long nColor )
{
    return Color( (nColor & 0xff), (nColor & 0xff00)>>8, (nColor & 0xff0000)>>16);
}

void X11SalFrame::UpdateSettings( AllSettings& rSettings )
{
    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    aStyleSettings.SetCursorBlinkTime( 500 );
    rSettings.SetStyleSettings( aStyleSettings );
}

void X11SalFrame::CaptureMouse( sal_Bool bCapture )
{
    nCaptured_ = pDisplay_->CaptureMouse( bCapture ? this : NULL );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void X11SalFrame::SetParent( SalFrame* pNewParent )
{
    if( mpParent != pNewParent )
    {
        if( mpParent )
            mpParent->maChildren.remove( this );

        mpParent = static_cast<X11SalFrame*>(pNewParent);
        mpParent->maChildren.push_back( this );
        if( mpParent->m_nXScreen != m_nXScreen )
            createNewWindow( None, mpParent->m_nXScreen );
        GetDisplay()->getWMAdaptor()->changeReferenceFrame( this, mpParent );
    }
}

SalFrame* X11SalFrame::GetParent() const
{
    return mpParent;
}

void X11SalFrame::createNewWindow( XLIB_Window aNewParent, SalX11Screen nXScreen )
{
    bool bWasVisible = bMapped_;
    if( bWasVisible )
        Show( sal_False );

    if( nXScreen.getXScreen() >= GetDisplay()->GetXScreenCount() )
        nXScreen = m_nXScreen;

    SystemParentData aParentData;
    aParentData.aWindow = aNewParent;
    aParentData.bXEmbedSupport = (aNewParent != None && m_bXEmbed); // caution: this is guesswork
    if( aNewParent == None )
    {
        aNewParent = GetDisplay()->GetRootWindow(nXScreen);
        aParentData.aWindow = None;
        m_bXEmbed = false;
    }
    else
    {
        // is new parent a root window ?
        Display* pDisp = GetDisplay()->GetDisplay();
        int nScreens = GetDisplay()->GetXScreenCount();
        for( int i = 0; i < nScreens; i++ )
        {
            if( aNewParent == RootWindow( pDisp, i ) )
            {
                nXScreen = SalX11Screen( i );
                aParentData.aWindow = None;
                m_bXEmbed = false;
                break;
            }
        }
    }

    // first deinit frame
    updateGraphics(true);
    if( mpInputContext )
    {
        mpInputContext->UnsetICFocus( this );
        mpInputContext->Unmap( this );
    }
    if( GetWindow() == hPresentationWindow )
    {
        hPresentationWindow = None;
        doReparentPresentationDialogues( GetDisplay() );
    }
    XDestroyWindow( GetXDisplay(), mhWindow );
    mhWindow = None;

    // now init with new parent again
    if ( aParentData.aWindow != None )
        Init( nStyle_ | SAL_FRAME_STYLE_PLUG, nXScreen, &aParentData );
    else
        Init( nStyle_ & ~SAL_FRAME_STYLE_PLUG, nXScreen, NULL, true );

    // update graphics if necessary
    updateGraphics(false);

    if( ! m_aTitle.isEmpty() )
        SetTitle( m_aTitle );

    if( mpParent )
    {
        if( mpParent->m_nXScreen != m_nXScreen )
            SetParent( NULL );
        else
            pDisplay_->getWMAdaptor()->changeReferenceFrame( this, mpParent );
    }

    if( bWasVisible )
        Show( sal_True );

    std::list< X11SalFrame* > aChildren = maChildren;
    for( std::list< X11SalFrame* >::iterator it = aChildren.begin(); it != aChildren.end(); ++it )
        (*it)->createNewWindow( None, m_nXScreen );

    // FIXME: SalObjects
}

bool X11SalFrame::SetPluginParent( SystemParentData* pNewParent )
{
    if( pNewParent->nSize >= sizeof(SystemParentData) )
        m_bXEmbed = pNewParent->aWindow != None && pNewParent->bXEmbedSupport;
    createNewWindow( pNewParent ? pNewParent->aWindow : None );

    return true;
}

// Event Handling
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static sal_uInt16 sal_GetCode( int state )
{
    sal_uInt16 nCode = 0;

    if( state & Button1Mask )
        nCode |= MOUSE_LEFT;
    if( state & Button2Mask )
        nCode |= MOUSE_MIDDLE;
    if( state & Button3Mask )
        nCode |= MOUSE_RIGHT;

    if( state & ShiftMask )
        nCode |= KEY_SHIFT;
    if( state & ControlMask )
        nCode |= KEY_MOD1;
    if( state & Mod1Mask )
        nCode |= KEY_MOD2;

        // Map Meta/Super modifier to MOD3 on all Unix systems
        // except Mac OS X
        if( (state & Mod3Mask) )
            nCode |= KEY_MOD3;

    return nCode;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

SalFrame::SalPointerState X11SalFrame::GetPointerState()
{
    SalPointerState aState;
    XLIB_Window aRoot, aChild;
    int rx, ry, wx, wy;
    unsigned int nMask = 0;
    XQueryPointer( GetXDisplay(),
                   GetShellWindow(),
                   &aRoot,
                   &aChild,
                   &rx, &ry,
                   &wx, &wy,
                   &nMask
                   );

    aState.maPos = Point(wx, wy);
    aState.mnState = sal_GetCode( nMask );
    return aState;
}

SalFrame::SalIndicatorState X11SalFrame::GetIndicatorState()
{
    SalIndicatorState aState;
    aState.mnState = GetGenericData()->GetSalDisplay()->GetIndicatorState();
    return aState;
}

void X11SalFrame::SimulateKeyPress( sal_uInt16 nKeyCode )
{
    GetGenericData()->GetSalDisplay()->SimulateKeyPress(nKeyCode);
}

long X11SalFrame::HandleMouseEvent( XEvent *pEvent )
{
    SalMouseEvent       aMouseEvt;
    sal_uInt16              nEvent = 0;
    bool                bClosePopups = false;

    if( nVisibleFloats && pEvent->type == EnterNotify )
        return 0;

    // Solaris X86: clicking the right button on a two-button mouse
    // generates a button2 event not a button3 event
    if (pDisplay_->GetProperties() & PROPERTY_SUPPORT_3ButtonMouse )
    {
        switch (pEvent->type)
        {
            case EnterNotify:
            case LeaveNotify:
                if ( pEvent->xcrossing.state & Button2Mask )
                {
                    pEvent->xcrossing.state &= ~Button2Mask;
                    pEvent->xcrossing.state |=  Button3Mask;
                }
                break;

            case MotionNotify:
                if ( pEvent->xmotion.state & Button2Mask )
                {
                    pEvent->xmotion.state &= ~Button2Mask;
                    pEvent->xmotion.state |=  Button3Mask;
                }
                break;

            default:
                if ( Button2 == pEvent->xbutton.button )
                {
                    pEvent->xbutton.state &= ~Button2Mask;
                    pEvent->xbutton.state |=  Button3Mask;
                    pEvent->xbutton.button =  Button3;
                }
                break;
        }
    }


    if( LeaveNotify == pEvent->type || EnterNotify == pEvent->type )
    {
        /*
         *  some WMs (and/or) applications  have a passive grab on
         *  mouse buttons (XGrabButton). This leads to enter/leave notifies
         *  with mouse buttons pressed in the state mask before the actual
         *  ButtonPress event gets dispatched. But EnterNotify
         *  is reported in vcl as MouseMove event. Some office code
         *  decides that a pressed button in a MouseMove belongs to
         *  a drag operation which leads to doing things differently.
         *
         *  ignore Enter/LeaveNotify resulting from grabs so that
         *  help windows do not disappear just after appearing
         *
         *  hopefully this workaround will not break anything.
         */
        if( pEvent->xcrossing.mode == NotifyGrab || pEvent->xcrossing.mode == NotifyUngrab  )
            return 0;

        aMouseEvt.mnX       = pEvent->xcrossing.x;
        aMouseEvt.mnY       = pEvent->xcrossing.y;
        aMouseEvt.mnTime    = pEvent->xcrossing.time;
        aMouseEvt.mnCode    = sal_GetCode( pEvent->xcrossing.state );
        aMouseEvt.mnButton  = 0;

        nEvent              = LeaveNotify == pEvent->type
                              ? SALEVENT_MOUSELEAVE
                              : SALEVENT_MOUSEMOVE;
    }
    else if( pEvent->type == MotionNotify )
    {
        aMouseEvt.mnX       = pEvent->xmotion.x;
        aMouseEvt.mnY       = pEvent->xmotion.y;
        aMouseEvt.mnTime    = pEvent->xmotion.time;
        aMouseEvt.mnCode    = sal_GetCode( pEvent->xmotion.state );

        aMouseEvt.mnButton  = 0;

        nEvent              = SALEVENT_MOUSEMOVE;
        if( nVisibleFloats > 0 && mpParent )
        {
            XLIB_Cursor aCursor = mpParent->GetCursor();
            if( pEvent->xmotion.x >= 0 && pEvent->xmotion.x < (int)maGeometry.nWidth &&
                pEvent->xmotion.y >= 0 && pEvent->xmotion.y < (int)maGeometry.nHeight )
                aCursor = None;

            XChangeActivePointerGrab( GetXDisplay(),
                                      PointerMotionMask|ButtonPressMask|ButtonReleaseMask,
                                      aCursor,
                                      CurrentTime );
        }
    }
    else
    {
        // let mouse events reach the correct window
        if( nVisibleFloats < 1 )
        {
            if( ! (nStyle_ & SAL_FRAME_STYLE_OWNERDRAWDECORATION) )
                XUngrabPointer( GetXDisplay(), CurrentTime );
        }
        else if( pEvent->type == ButtonPress )
        {
            // see if the user clicks outside all of the floats
            // if yes release the grab
            bool bInside = false;
            const std::list< SalFrame* >& rFrames = GetDisplay()->getFrames();
            for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
            {
                const X11SalFrame* pFrame = static_cast< const X11SalFrame* >(*it);
                if( pFrame->IsFloatGrabWindow()                                     &&
                    pFrame->bMapped_                                                &&
                    pEvent->xbutton.x_root >= pFrame->maGeometry.nX                             &&
                    pEvent->xbutton.x_root < pFrame->maGeometry.nX + (int)pFrame->maGeometry.nWidth &&
                    pEvent->xbutton.y_root >= pFrame->maGeometry.nY                             &&
                    pEvent->xbutton.y_root < pFrame->maGeometry.nY + (int)pFrame->maGeometry.nHeight )
                {
                    bInside = true;
                    break;
                }
            }
            if( ! bInside )
            {
                // need not take care of the XUngrabPointer in Show( sal_False )
                // because XUngrabPointer does not produce errors if pointer
                // is not grabbed
                XUngrabPointer( GetXDisplay(), CurrentTime );
                bClosePopups = true;

                /*  #i15246# only close popups if pointer is outside all our frames
                 *  cannot use our own geometry data here because stacking
                 *  is unknown (the above case implicitly assumes
                 *  that floats are on top which should be true)
                 */
                XLIB_Window aRoot, aChild;
                int root_x, root_y, win_x, win_y;
                unsigned int mask_return;
                if( XQueryPointer( GetXDisplay(),
                                   GetDisplay()->GetRootWindow( m_nXScreen ),
                                   &aRoot, &aChild,
                                   &root_x, &root_y,
                                   &win_x, &win_y,
                                   &mask_return )
                    && aChild // pointer may not be in any child
                    )
                {
                    for( std::list< SalFrame* >::const_iterator it = rFrames.begin(); it != rFrames.end(); ++it )
                    {
                        const X11SalFrame* pFrame = static_cast< const X11SalFrame* >(*it);
                        if( ! pFrame->IsFloatGrabWindow()
                            && ( pFrame->GetWindow() == aChild ||
                                 pFrame->GetShellWindow() == aChild ||
                                 pFrame->GetStackingWindow() == aChild )
                            )
                        {
                            // #i63638# check that pointer is inside window, not
                            // only inside stacking window
                            if( root_x >= pFrame->maGeometry.nX && root_x < sal::static_int_cast< int >(pFrame->maGeometry.nX+pFrame->maGeometry.nWidth) &&
                                root_y >= pFrame->maGeometry.nY && root_y < sal::static_int_cast< int >(pFrame->maGeometry.nX+pFrame->maGeometry.nHeight) )
                            {
                                bClosePopups = false;
                            }
                            break;
                        }
                    }
                }
            }
        }

        if( m_bXEmbed && pEvent->xbutton.button == Button1 )
            askForXEmbedFocus( pEvent->xbutton.time );

        if( pEvent->xbutton.button == Button1 ||
            pEvent->xbutton.button == Button2 ||
            pEvent->xbutton.button == Button3 )
        {
            aMouseEvt.mnX       = pEvent->xbutton.x;
            aMouseEvt.mnY       = pEvent->xbutton.y;
            aMouseEvt.mnTime    = pEvent->xbutton.time;
            aMouseEvt.mnCode    = sal_GetCode( pEvent->xbutton.state );

            if( Button1 == pEvent->xbutton.button )
                aMouseEvt.mnButton  = MOUSE_LEFT;
            else if( Button2 == pEvent->xbutton.button )
                aMouseEvt.mnButton  = MOUSE_MIDDLE;
            else if( Button3 == pEvent->xbutton.button )
                aMouseEvt.mnButton  = MOUSE_RIGHT;

            nEvent              = ButtonPress == pEvent->type
                ? SALEVENT_MOUSEBUTTONDOWN
                : SALEVENT_MOUSEBUTTONUP;
        }
        else if( pEvent->xbutton.button == Button4 ||
                 pEvent->xbutton.button == Button5 ||
                 pEvent->xbutton.button == Button6 ||
                 pEvent->xbutton.button == Button7 )
        {
            const bool bIncrement(
                pEvent->xbutton.button == Button4 ||
                pEvent->xbutton.button == Button6 );
            const bool bHoriz(
                pEvent->xbutton.button == Button6 ||
                pEvent->xbutton.button == Button7 );

            if( pEvent->type == ButtonRelease )
                return 0;

            static sal_uLong        nLines = 0;
            if( ! nLines )
            {
                char* pEnv = getenv( "SAL_WHEELLINES" );
                nLines = pEnv ? atoi( pEnv ) : 3;
                if( nLines > 10 )
                    nLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
            }

            SalWheelMouseEvent  aWheelEvt;
            aWheelEvt.mnTime        = pEvent->xbutton.time;
            aWheelEvt.mnX           = pEvent->xbutton.x;
            aWheelEvt.mnY           = pEvent->xbutton.y;
            aWheelEvt.mnDelta       = bIncrement ? 120 : -120;
            aWheelEvt.mnNotchDelta  = bIncrement ? 1 : -1;
            aWheelEvt.mnScrollLines = nLines;
            aWheelEvt.mnCode        = sal_GetCode( pEvent->xbutton.state );
            aWheelEvt.mbHorz        = bHoriz;

            nEvent = SALEVENT_WHEELMOUSE;

            // --- RTL --- (mirror mouse pos)
            if( Application::GetSettings().GetLayoutRTL() )
                aWheelEvt.mnX = nWidth_-1-aWheelEvt.mnX;
            return CallCallback( nEvent, &aWheelEvt );
        }
    }

    int nRet = 0;
    if( nEvent == SALEVENT_MOUSELEAVE
        || ( aMouseEvt.mnX <  nWidth_  && aMouseEvt.mnX >  -1 &&
             aMouseEvt.mnY <  nHeight_ && aMouseEvt.mnY >  -1 )
        || pDisplay_->MouseCaptured( this )
        )
    {
        // --- RTL --- (mirror mouse pos)
        if( Application::GetSettings().GetLayoutRTL() )
            aMouseEvt.mnX = nWidth_-1-aMouseEvt.mnX;
        nRet = CallCallback( nEvent, &aMouseEvt );
    }

    if( bClosePopups )
    {
        /*  #108213# close popups after dispatching the event outside the popup;
         *  applications do weird things.
         */
        ImplSVData* pSVData = ImplGetSVData();
        if ( pSVData->maWinData.mpFirstFloat )
        {
            static const char* pEnv = getenv( "SAL_FLOATWIN_NOAPPFOCUSCLOSE" );
            if ( !(pSVData->maWinData.mpFirstFloat->GetPopupModeFlags() & FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE) && !(pEnv && *pEnv) )
                pSVData->maWinData.mpFirstFloat->EndPopupMode( FLOATWIN_POPUPMODEEND_CANCEL | FLOATWIN_POPUPMODEEND_CLOSEALL );
        }
    }

    return nRet;
}

// F10 means either KEY_F10 or KEY_MENU, which has to be decided
// in the independent part.
struct KeyAlternate
{
    sal_uInt16          nKeyCode;
    sal_Unicode     nCharCode;
    KeyAlternate() : nKeyCode( 0 ), nCharCode( 0 ) {}
    KeyAlternate( sal_uInt16 nKey, sal_Unicode nChar = 0 ) : nKeyCode( nKey ), nCharCode( nChar ) {}
};

inline KeyAlternate
GetAlternateKeyCode( const sal_uInt16 nKeyCode )
{
    KeyAlternate aAlternate;

    switch( nKeyCode )
    {
        case KEY_F10: aAlternate = KeyAlternate( KEY_MENU );break;
        case KEY_F24: aAlternate = KeyAlternate( KEY_SUBTRACT, '-' );break;
    }

    return aAlternate;
}

void X11SalFrame::beginUnicodeSequence()
{
    rtl::OUString& rSeq( GetGenericData()->GetUnicodeCommand() );
    DeletionListener aDeleteWatch( this );

    if( !rSeq.isEmpty() )
        endUnicodeSequence();

    rSeq = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "u" ) );

    if( ! aDeleteWatch.isDeleted() )
    {
        sal_uInt16 nTextAttr = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
        SalExtTextInputEvent aEv;
        aEv.mnTime          = 0;
        aEv.maText          = rSeq;
        aEv.mpTextAttr      = &nTextAttr;
        aEv.mnCursorPos     = 0;
        aEv.mnDeltaStart    = 0;
        aEv.mnCursorFlags   = 0;
        aEv.mbOnlyCursor    = sal_False;

        CallCallback(SALEVENT_EXTTEXTINPUT, (void*)&aEv);
    }
}

bool X11SalFrame::appendUnicodeSequence( sal_Unicode c )
{
    bool bRet = false;
    rtl::OUString& rSeq( GetGenericData()->GetUnicodeCommand() );
    if( !rSeq.isEmpty() )
    {
        // range check
        if( (c >= sal_Unicode('0') && c <= sal_Unicode('9')) ||
            (c >= sal_Unicode('a') && c <= sal_Unicode('f')) ||
            (c >= sal_Unicode('A') && c <= sal_Unicode('F')) )
        {
            rtl::OUStringBuffer aBuf( rSeq.getLength() + 1 );
            aBuf.append( rSeq );
            aBuf.append( c );
            rSeq = aBuf.makeStringAndClear();
            std::vector<sal_uInt16> attribs( rSeq.getLength(), SAL_EXTTEXTINPUT_ATTR_UNDERLINE );

            SalExtTextInputEvent aEv;
            aEv.mnTime          = 0;
            aEv.maText          = rSeq;
            aEv.mpTextAttr      = &attribs[0];
            aEv.mnCursorPos     = 0;
            aEv.mnDeltaStart    = 0;
            aEv.mnCursorFlags   = 0;
            aEv.mbOnlyCursor    = sal_False;

            CallCallback(SALEVENT_EXTTEXTINPUT, (void*)&aEv);
            bRet = true;
        }
        else
            bRet = endUnicodeSequence();
    }
    else
        endUnicodeSequence();
    return bRet;
}

bool X11SalFrame::endUnicodeSequence()
{
    rtl::OUString& rSeq( GetGenericData()->GetUnicodeCommand() );

    DeletionListener aDeleteWatch( this );
    if( rSeq.getLength() > 1 && rSeq.getLength() < 6 )
    {
        // cut the "u"
        rtl::OUString aNumbers( rSeq.copy( 1 ) );
        sal_Int32 nValue = aNumbers.toInt32( 16 );
        if( nValue >= 32 )
        {
            sal_uInt16 nTextAttr = SAL_EXTTEXTINPUT_ATTR_UNDERLINE;
            SalExtTextInputEvent aEv;
            aEv.mnTime          = 0;
            aEv.maText          = rtl::OUString( sal_Unicode(nValue) );
            aEv.mpTextAttr      = &nTextAttr;
            aEv.mnCursorPos     = 0;
            aEv.mnDeltaStart    = 0;
            aEv.mnCursorFlags   = 0;
            aEv.mbOnlyCursor    = sal_False;
            CallCallback(SALEVENT_EXTTEXTINPUT, (void*)&aEv);
        }
    }
    bool bWasInput = !rSeq.isEmpty();
    rSeq = rtl::OUString();
    if( bWasInput && ! aDeleteWatch.isDeleted() )
        CallCallback(SALEVENT_ENDEXTTEXTINPUT, NULL);
    return bWasInput;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long X11SalFrame::HandleKeyEvent( XKeyEvent *pEvent )
{
    KeySym          nKeySym;
    KeySym          nUnmodifiedKeySym;
    int             nLen = 2048;
    unsigned char   *pPrintable = (unsigned char*)alloca( nLen );

    // singlebyte code composed by input method, the new default
    if (mpInputContext != NULL && mpInputContext->UseContext())
    {
        // returns a keysym as well as the pPrintable (in system encoding)
        // printable may be empty.
        Status nStatus;
        nKeySym = pDisplay_->GetKeySym( pEvent, pPrintable, &nLen,
                                        &nUnmodifiedKeySym,
                                        &nStatus, mpInputContext->GetContext() );
        if ( nStatus == XBufferOverflow )
        {
            nLen *= 2;
            pPrintable = (unsigned char*)alloca( nLen );
            nKeySym = pDisplay_->GetKeySym( pEvent, pPrintable, &nLen,
                                            &nUnmodifiedKeySym,
                                            &nStatus, mpInputContext->GetContext() );
        }
    }
    else
    {
        // fallback, this should never ever be called
        Status nStatus = 0;
           nKeySym = pDisplay_->GetKeySym( pEvent, pPrintable, &nLen, &nUnmodifiedKeySym, &nStatus );
     }

    SalKeyEvent aKeyEvt;
    sal_uInt16      nKeyCode;
    sal_uInt16 nModCode = 0;
    char        aDummy;

    if( pEvent->state & ShiftMask )
        nModCode |= KEY_SHIFT;
    if( pEvent->state & ControlMask )
        nModCode |= KEY_MOD1;
    if( pEvent->state & Mod1Mask )
        nModCode |= KEY_MOD2;

    if( nModCode != (KEY_SHIFT|KEY_MOD1) )
        endUnicodeSequence();

    if(     nKeySym == XK_Shift_L   || nKeySym == XK_Shift_R
        ||  nKeySym == XK_Control_L || nKeySym == XK_Control_R
        ||  nKeySym == XK_Alt_L     || nKeySym == XK_Alt_R
        ||  nKeySym == XK_Meta_L    || nKeySym == XK_Meta_R
                ||      nKeySym == XK_Super_L   || nKeySym == XK_Super_R )
    {
        SalKeyModEvent aModEvt;
        aModEvt.mnModKeyCode = 0;
        if( pEvent->type == XLIB_KeyPress && mnExtKeyMod == 0 )
            mbSendExtKeyModChange = true;
        else if( pEvent->type == KeyRelease && mbSendExtKeyModChange )
        {
            aModEvt.mnModKeyCode = mnExtKeyMod;
            mnExtKeyMod = 0;
        }

        // pressing just the ctrl key leads to a keysym of XK_Control but
        // the event state does not contain ControlMask. In the release
        // event its the other way round: it does contain the Control mask.
        // The modifier mode therefore has to be adapted manually.
        sal_uInt16 nExtModMask = 0;
        sal_uInt16 nModMask = 0;
        switch( nKeySym )
        {
            case XK_Control_L:
                nExtModMask = MODKEY_LMOD1;
                nModMask = KEY_MOD1;
                break;
            case XK_Control_R:
                nExtModMask = MODKEY_RMOD1;
                nModMask = KEY_MOD1;
                break;
            case XK_Alt_L:
                nExtModMask = MODKEY_LMOD2;
                nModMask = KEY_MOD2;
                break;
            case XK_Alt_R:
                nExtModMask = MODKEY_RMOD2;
                nModMask = KEY_MOD2;
                break;
            case XK_Shift_L:
                nExtModMask = MODKEY_LSHIFT;
                nModMask = KEY_SHIFT;
                break;
            case XK_Shift_R:
                nExtModMask = MODKEY_RSHIFT;
                nModMask = KEY_SHIFT;
                break;
            // Map Meta/Super keys to MOD3 modifier on all Unix systems
            // except Mac OS X
            case XK_Meta_L:
            case XK_Super_L:
                nExtModMask = MODKEY_LMOD3;
                nModMask = KEY_MOD3;
                break;
            case XK_Meta_R:
            case XK_Super_R:
                nExtModMask = MODKEY_RMOD3;
                nModMask = KEY_MOD3;
                break;
        }
        if( pEvent->type == KeyRelease )
        {
            nModCode &= ~nModMask;
            mnExtKeyMod &= ~nExtModMask;
        }
        else
        {
            nModCode |= nModMask;
            mnExtKeyMod |= nExtModMask;
        }

        aModEvt.mnCode = nModCode;
        aModEvt.mnTime = pEvent->time;

        int nRet = CallCallback( SALEVENT_KEYMODCHANGE, &aModEvt );

        return nRet;
    }

    mbSendExtKeyModChange = false;

    // try to figure out the vcl code for the keysym
    // #i52338# use the unmodified KeySym if there is none for the real KeySym
    // because the independent part has only keycodes for unshifted keys
    nKeyCode = pDisplay_->GetKeyCode( nKeySym, &aDummy );
    if( nKeyCode == 0 )
        nKeyCode = pDisplay_->GetKeyCode( nUnmodifiedKeySym, &aDummy );

    // try to figure out a printable if XmbLookupString returns only a keysym
    // and NOT a printable. Do not store it in pPrintable[0] since it is expected to
    // be in system encoding, not unicode.
    // #i8988##, if KeySym and printable look equally promising then prefer KeySym
    // the printable is bound to the encoding so the KeySym might contain more
    // information (in et_EE locale: "Compose + Z + <" delivers "," in printable and
    // (the desired) Zcaron in KeySym
    sal_Unicode nKeyString = 0x0;
    if (   (nLen == 0)
        || ((nLen == 1) && (nKeySym > 0)) )
        nKeyString = KeysymToUnicode (nKeySym);
    // if we have nothing we give up
    if( !nKeyCode && !nLen && !nKeyString)
        return 0;

    DeletionListener aDeleteWatch( this );

    if( nModCode == (KEY_SHIFT | KEY_MOD1) && pEvent->type == XLIB_KeyPress )
    {
        sal_uInt16 nSeqKeyCode = pDisplay_->GetKeyCode( nUnmodifiedKeySym, &aDummy );
        if( nSeqKeyCode == KEY_U )
        {
            beginUnicodeSequence();
            return 1;
        }
        else if( nSeqKeyCode >= KEY_0 && nSeqKeyCode <= KEY_9 )
        {
            if( appendUnicodeSequence( sal_Unicode( '0' ) + sal_Unicode(nSeqKeyCode - KEY_0) ) )
                return 1;
        }
        else if( nSeqKeyCode >= KEY_A && nSeqKeyCode <= KEY_F )
        {
            if( appendUnicodeSequence( sal_Unicode( 'a' ) + sal_Unicode(nSeqKeyCode - KEY_A) ) )
                return 1;
        }
        else
            endUnicodeSequence();
    }

    if( aDeleteWatch.isDeleted() )
        return 0;

    rtl_TextEncoding nEncoding = osl_getThreadTextEncoding();

    sal_Unicode *pBuffer;
    sal_Unicode *pString;
    sal_Size     nBufferSize = nLen * 2;
    sal_Size     nSize;
    pBuffer = (sal_Unicode*) malloc( nBufferSize + 2 );
    pBuffer[ 0 ] = 0;

    if (nKeyString != 0)
    {
        pString = &nKeyString;
        nSize = 1;
    }
    else
    if (nLen > 0 && nEncoding != RTL_TEXTENCODING_UNICODE)
    {
        // create text converter
        rtl_TextToUnicodeConverter aConverter =
                rtl_createTextToUnicodeConverter( nEncoding );
        rtl_TextToUnicodeContext aContext =
                 rtl_createTextToUnicodeContext( aConverter );

        sal_uInt32  nConversionInfo;
        sal_Size    nConvertedChars;

        // convert to single byte text stream
        nSize = rtl_convertTextToUnicode(
                                aConverter, aContext,
                                (char*)pPrintable, nLen,
                                pBuffer, nBufferSize,
                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE |
                                RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE,
                                &nConversionInfo, &nConvertedChars );

        // destroy converter
        rtl_destroyTextToUnicodeContext( aConverter, aContext );
        rtl_destroyTextToUnicodeConverter( aConverter );

        pString = pBuffer;
    }
    else
    if (nLen > 0 /* nEncoding == RTL_TEXTENCODING_UNICODE */)
    {
        pString = (sal_Unicode*)pPrintable;
          nSize = nLen;
    }
    else
    {
        pString = pBuffer;
        nSize   = 0;
    }

    if (   mpInputContext != NULL
        && mpInputContext->UseContext()
        && KeyRelease != pEvent->type
        && (   (nSize >  1)
            || (nSize >  0 && mpInputContext->IsPreeditMode())) )
    {
        mpInputContext->CommitKeyEvent(pString, nSize);
    }
    else
    // normal single character keyinput
    {
        aKeyEvt.mnCode     = nKeyCode | nModCode;
        aKeyEvt.mnRepeat   = 0;
        aKeyEvt.mnTime     = pEvent->time;
        aKeyEvt.mnCharCode = pString[ 0 ];

        if( KeyRelease == pEvent->type )
        {
            CallCallback( SALEVENT_KEYUP, &aKeyEvt );
        }
        else
        {
            if ( ! CallCallback(SALEVENT_KEYINPUT, &aKeyEvt) )
            {
                // independent layer doesnt want to handle key-event, so check
                // whether the keycode may have an alternate meaning
                KeyAlternate aAlternate = GetAlternateKeyCode( nKeyCode );
                if ( aAlternate.nKeyCode != 0 )
                {
                    aKeyEvt.mnCode = aAlternate.nKeyCode | nModCode;
                    if( aAlternate.nCharCode )
                        aKeyEvt.mnCharCode = aAlternate.nCharCode;
                    CallCallback(SALEVENT_KEYINPUT, &aKeyEvt);
                }
            }
        }
    }

      //
      // update the spot location for PreeditPosition IME style
      //
    if (! aDeleteWatch.isDeleted())
    {
        if (mpInputContext != NULL && mpInputContext->UseContext())
            mpInputContext->UpdateSpotLocation();
    }

    free (pBuffer);
    return True;
}


// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long X11SalFrame::HandleFocusEvent( XFocusChangeEvent *pEvent )
{
    // ReflectionX in Windows mode changes focus while mouse is grabbed
    if( nVisibleFloats > 0 && GetDisplay()->getWMAdaptor()->getWindowManagerName().EqualsAscii( "ReflectionX Windows" ) )
        return 1;

    /*  ignore focusout resulting from keyboard grabs
     *  we do not grab it and are not interested when
     *  someone else does CDE e.g. does a XGrabKey on arrow keys
     *  handle focus events with mode NotifyWhileGrabbed
     *  because with CDE alt-tab focus changing we do not get
     *  normal focus events
     *  cast focus event to the input context, otherwise the
     *  status window does not follow the application frame
     */

    if ( mpInputContext != NULL  )
    {
        if( FocusIn == pEvent->type )
            mpInputContext->SetICFocus( this );
        else
        {
            /*
             *  do not unset the IC focuse here because would kill
             *  a lookup choice windows that might have the focus now
             *      mpInputContext->UnsetICFocus( this );
             */
            I18NStatus::get().show( false, I18NStatus::focus );
        }
    }


    if ( pEvent->mode == NotifyNormal || pEvent->mode == NotifyWhileGrabbed ||
         ( ( nStyle_ & SAL_FRAME_STYLE_PLUG ) && pEvent->window == GetShellWindow() )
         )
    {
        if( hPresentationWindow != None && hPresentationWindow != GetShellWindow() )
            return 0;

        if( FocusIn == pEvent->type )
        {
            GetSalData()->m_pInstance->updatePrinterUpdate();
            mbInputFocus = True;
            ImplSVData* pSVData = ImplGetSVData();



            long nRet = CallCallback( SALEVENT_GETFOCUS,  0 );
            if ((mpParent != NULL && nStyle_ == 0)
                && pSVData->maWinData.mpFirstFloat )
            {
                sal_uLong nMode = pSVData->maWinData.mpFirstFloat->GetPopupModeFlags();
                pSVData->maWinData.mpFirstFloat->SetPopupModeFlags(
                                        nMode & ~(FLOATWIN_POPUPMODE_NOAPPFOCUSCLOSE));
            }
            return nRet;
        }
        else
        {
            mbInputFocus = False;
            mbSendExtKeyModChange = false;
            mnExtKeyMod = 0;
            return CallCallback( SALEVENT_LOSEFOCUS, 0 );
        }
    }

    return 0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

long X11SalFrame::HandleExposeEvent( XEvent *pEvent )
{
    XRectangle  aRect = { 0, 0, 0, 0 };
    sal_uInt16      nCount = 0;

    if( pEvent->type == Expose )
    {
        aRect.x         = pEvent->xexpose.x;
        aRect.y         = pEvent->xexpose.y;
        aRect.width     = pEvent->xexpose.width;
        aRect.height    = pEvent->xexpose.height;
        nCount          = pEvent->xexpose.count;
    }
    else if( pEvent->type == GraphicsExpose )
    {
        aRect.x         = pEvent->xgraphicsexpose.x;
        aRect.y         = pEvent->xgraphicsexpose.y;
        aRect.width     = pEvent->xgraphicsexpose.width;
        aRect.height    = pEvent->xgraphicsexpose.height;
        nCount          = pEvent->xgraphicsexpose.count;
    }

    if( IsOverrideRedirect() && mbFullScreen &&
        aPresentationReparentList.begin() == aPresentationReparentList.end() )
        // we are in fullscreen mode -> override redirect
         // focus is possibly lost, so reget it
         XSetInputFocus( GetXDisplay(), GetShellWindow(), RevertToNone, CurrentTime );

    // width and height are extents, so they are of by one for rectangle
    maPaintRegion.Union( Rectangle( Point(aRect.x, aRect.y), Size(aRect.width+1, aRect.height+1) ) );

    if( nCount )
        // wait for last expose rectangle, do not wait for resize timer
        // if a completed graphics expose sequence is available
        return 1;

    SalPaintEvent aPEvt( maPaintRegion.Left(), maPaintRegion.Top(), maPaintRegion.GetWidth(), maPaintRegion.GetHeight() );

     CallCallback( SALEVENT_PAINT, &aPEvt );
    maPaintRegion = Rectangle();

    return 1;
}

void X11SalFrame::RestackChildren( XLIB_Window* pTopLevelWindows, int nTopLevelWindows )
{
    if( maChildren.begin() != maChildren.end() )
    {
        int nWindow = nTopLevelWindows;
        while( nWindow-- )
            if( pTopLevelWindows[nWindow] == GetStackingWindow() )
                break;
        if( nWindow < 0 )
            return;

        std::list< X11SalFrame* >::const_iterator it;
        for( it = maChildren.begin(); it != maChildren.end(); ++it )
        {
            X11SalFrame* pData = *it;
            if( pData->bMapped_ )
            {
                int nChild = nWindow;
                while( nChild-- )
                {
                    if( pTopLevelWindows[nChild] == pData->GetStackingWindow() )
                    {
                        // if a child is behind its parent, place it above the
                        // parent (for insane WMs like Dtwm and olwm)
                        XWindowChanges aCfg;
                        aCfg.sibling    = GetStackingWindow();
                        aCfg.stack_mode = Above;
                        XConfigureWindow( GetXDisplay(), pData->GetStackingWindow(), CWSibling|CWStackMode, &aCfg );
                        break;
                    }
                }
            }
        }
        for( it = maChildren.begin(); it != maChildren.end(); ++it )
        {
            X11SalFrame* pData = *it;
            pData->RestackChildren( pTopLevelWindows, nTopLevelWindows );
        }
    }
}

void X11SalFrame::RestackChildren()
{
    if( ! GetDisplay()->getWMAdaptor()->isTransientBehaviourAsExpected()
        && maChildren.begin() != maChildren.end() )
    {
        XLIB_Window aRoot, aParent, *pChildren = NULL;
        unsigned int nChildren;
        if( XQueryTree( GetXDisplay(),
                        GetDisplay()->GetRootWindow( m_nXScreen ),
                        &aRoot,
                        &aParent,
                        &pChildren,
                        &nChildren ) )
        {
            RestackChildren( pChildren, nChildren );
            XFree( pChildren );
        }
    }
}

static Bool size_event_predicate( Display*, XEvent* event, XPointer arg )
{
    if( event->type != ConfigureNotify )
        return False;
    X11SalFrame* frame = reinterpret_cast< X11SalFrame* >( arg );
    XConfigureEvent* pEvent = &event->xconfigure;
    if( pEvent->window != frame->GetShellWindow()
        && pEvent->window != frame->GetWindow()
        && pEvent->window != frame->GetForeignParent()
        && pEvent->window != frame->GetStackingWindow())
    { // ignored at top of HandleSizeEvent()
        return False;
    }
    if( pEvent->window == frame->GetStackingWindow())
        return False; // filtered later in HandleSizeEvent()
    // at this point we know that there is another similar event in the queue
    frame->setPendingSizeEvent();
    return False; // but do not process the new event out of order
}

void X11SalFrame::setPendingSizeEvent()
{
    mPendingSizeEvent = true;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long X11SalFrame::HandleSizeEvent( XConfigureEvent *pEvent )
{
    // NOTE: if you add more tests in this function, make sure to update size_event_predicate()
    // so that it finds exactly the same events

    if (   pEvent->window != GetShellWindow()
           && pEvent->window != GetWindow()
           && pEvent->window != GetForeignParent()
           && pEvent->window != GetStackingWindow()
           )
    {
        // could be as well a sys-child window (aka SalObject)
        return 1;
    }


    if( ( nStyle_ & SAL_FRAME_STYLE_PLUG ) && pEvent->window == GetShellWindow() )
    {
        // just update the children's positions
        RestackChildren();
        return 1;
    }

    if( pEvent->window == GetForeignParent() )
        XResizeWindow( GetXDisplay(),
                       GetWindow(),
                       pEvent->width,
                       pEvent->height );

    XLIB_Window hDummy;
    XTranslateCoordinates( GetXDisplay(),
                           GetWindow(),
                           pDisplay_->GetRootWindow( pDisplay_->GetDefaultXScreen() ),
                           0, 0,
                           &pEvent->x, &pEvent->y,
                           &hDummy );

    if( pEvent->window == GetStackingWindow() )
    {
        if( maGeometry.nX != pEvent->x || maGeometry.nY != pEvent->y )
        {
            maGeometry.nX = pEvent->x;
            maGeometry.nY = pEvent->y;
            CallCallback( SALEVENT_MOVE, NULL );
        }
        return 1;
    }

    // check size hints in first time SalFrame::Show
    if( SHOWSTATE_UNKNOWN == nShowState_ && bMapped_ )
        nShowState_ = SHOWSTATE_NORMAL;

    // Avoid a race condition where resizing this window to one size and shortly after that
    // to another size generates first size event with the old size and only after that
    // with the new size, temporarily making us think the old size is valid (bnc#674806).
    // So if there is another size event for this window pending, ignore this one.
    mPendingSizeEvent = false;
    XEvent dummy;
    XCheckIfEvent( GetXDisplay(), &dummy, size_event_predicate, reinterpret_cast< XPointer >( this ));
    if( mPendingSizeEvent )
        return 1;

    nWidth_     = pEvent->width;
    nHeight_    = pEvent->height;

    bool bMoved = ( pEvent->x != maGeometry.nX || pEvent->y != maGeometry.nY );
    bool bSized = ( pEvent->width != (int)maGeometry.nWidth || pEvent->height != (int)maGeometry.nHeight );

    maGeometry.nX       = pEvent->x;
    maGeometry.nY       = pEvent->y;
    maGeometry.nWidth   = pEvent->width;
    maGeometry.nHeight  = pEvent->height;
    updateScreenNumber();

    // update children's position
    RestackChildren();

    if( bSized && ! bMoved )
        CallCallback( SALEVENT_RESIZE, NULL );
    else if( bMoved && ! bSized )
        CallCallback( SALEVENT_MOVE, NULL );
    else if( bMoved && bSized )
        CallCallback( SALEVENT_MOVERESIZE, NULL );

    return 1;
}

IMPL_LINK_NOARG(X11SalFrame, HandleAlwaysOnTopRaise)
{
    if( bMapped_ )
        ToTop( 0 );
    return 0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long X11SalFrame::HandleReparentEvent( XReparentEvent *pEvent )
{
    Display        *pDisplay   = pEvent->display;
    XLIB_Window     hWM_Parent;
    XLIB_Window     hRoot, *Children, hDummy;
    unsigned int    nChildren;
    sal_Bool            bNone = pDisplay_->GetProperties()
                            & PROPERTY_SUPPORT_WM_Parent_Pixmap_None;
    sal_Bool            bAccessParentWindow = ! (pDisplay_->GetProperties()
                            & PROPERTY_FEATURE_TrustedSolaris);

    static const char* pDisableStackingCheck = getenv( "SAL_DISABLE_STACKING_CHECK" );

    GetGenericData()->ErrorTrapPush();

    /*
     *  don't rely on the new parent from the event.
     *  the event may be "out of date", that is the window manager
     *  window may not exist anymore. This can happen if someone
     *  shows a frame and hides it again quickly (not that that would
     *  be very sensible)
     */
    hWM_Parent = GetShellWindow();
    do
    {
        Children = NULL;
        XQueryTree( pDisplay,
                    hWM_Parent,
                    &hRoot,
                    &hDummy,
                    &Children,
                    &nChildren );

        bool bError = GetGenericData()->ErrorTrapPop( false );
        GetGenericData()->ErrorTrapPush();

        if( bError )
        {
            hWM_Parent = GetShellWindow();
            break;
        }
         /* this sometimes happens if a Show(sal_True) is
         *  immediately followed by Show(sal_False) (which is braindead anyway)
         */
        if( hDummy == hWM_Parent )
            hDummy = hRoot;
        if( hDummy != hRoot )
        {
            hWM_Parent = hDummy;
            if( bAccessParentWindow && bNone )
                XSetWindowBackgroundPixmap( pDisplay, hWM_Parent, None );
        }
        if( Children )
            XFree( Children );
    } while( hDummy != hRoot );

    if( GetStackingWindow() == None
        && hWM_Parent != hPresentationWindow
        && hWM_Parent != GetShellWindow()
        && ( ! pDisableStackingCheck || ! *pDisableStackingCheck )
        )
    {
        mhStackingWindow = hWM_Parent;
        if (bAccessParentWindow)
            XSelectInput( pDisplay, GetStackingWindow(), StructureNotifyMask );
    }

    if(     hWM_Parent == pDisplay_->GetRootWindow( pDisplay_->GetDefaultXScreen() )
            ||  hWM_Parent == GetForeignParent()
            ||  pEvent->parent == pDisplay_->GetRootWindow( pDisplay_->GetDefaultXScreen() )
            || ( nStyle_ & SAL_FRAME_STYLE_FLOAT ) )
    {
        // Reparenting before Destroy
        aPresentationReparentList.remove( GetStackingWindow() );
        mhStackingWindow = None;
        GetGenericData()->ErrorTrapPop();
        return 0;
    }

    /*
     *  evil hack to show decorated windows on top
     *  of override redirect presentation windows:
     *  reparent the window manager window to the presentation window
     *  does not work with non-reparenting WMs
     *  in future this should not be necessary anymore with
     *  _NET_WM_STATE_FULLSCREEN available
     */
    if( hPresentationWindow != None
        && hPresentationWindow != GetWindow()
        && GetStackingWindow() != None
        && GetStackingWindow() != GetDisplay()->GetRootWindow( m_nXScreen )
        )
    {
        int x = 0, y = 0;
        XLIB_Window aChild;
        XTranslateCoordinates( GetXDisplay(),
                               GetStackingWindow(),
                               GetDisplay()->GetRootWindow( m_nXScreen ),
                               0, 0,
                               &x, &y,
                               &aChild
                               );
        XReparentWindow( GetXDisplay(),
                         GetStackingWindow(),
                         hPresentationWindow,
                         x, y
                         );
        aPresentationReparentList.push_back( GetStackingWindow() );
    }

    int nLeft = 0, nTop = 0;
    XTranslateCoordinates( GetXDisplay(),
                           GetShellWindow(),
                           hWM_Parent,
                           0, 0,
                           &nLeft,
                           &nTop,
                           &hDummy );
    maGeometry.nLeftDecoration  = nLeft > 0 ? nLeft-1 : 0;
    maGeometry.nTopDecoration   = nTop  > 0 ? nTop-1  : 0;

    /*
     *  decorations are not symmetric,
     *  so need real geometries here
     *  (this will fail with virtual roots ?)
     */

    // reset error occurred
    GetGenericData()->ErrorTrapPop();
    GetGenericData()->ErrorTrapPush();

    int xp, yp, x, y;
    unsigned int wp, w, hp, h, bw, d;
    XGetGeometry( GetXDisplay(),
                  GetShellWindow(),
                  &hRoot,
                  &x, &y, &w, &h, &bw, &d );
    XGetGeometry( GetXDisplay(),
                  hWM_Parent,
                  &hRoot,
                  &xp, &yp, &wp, &hp, &bw, &d );
    bool bResized = false;
    bool bError = GetGenericData()->ErrorTrapPop( false );
    GetGenericData()->ErrorTrapPush();

    if( ! bError )
    {
        maGeometry.nRightDecoration     = wp - w - maGeometry.nLeftDecoration;
        maGeometry.nBottomDecoration    = hp - h - maGeometry.nTopDecoration;
        /*
         *  note: this works because hWM_Parent is direct child of root,
         *  not necessarily parent of GetShellWindow()
         */
        maGeometry.nX       = xp + nLeft;
        maGeometry.nY       = yp + nTop;
        bResized = w != maGeometry.nWidth || h != maGeometry.nHeight;
        maGeometry.nWidth   = w;
        maGeometry.nHeight = h;
    }

    // limit width and height if we are too large: #47757
    // olwm and fvwm need this, it doesnt harm the rest

    // #i81311# do this only for sizable frames
    if( (nStyle_ & SAL_FRAME_STYLE_SIZEABLE) != 0 )
    {
        Size aScreenSize = GetDisplay()->GetScreenSize( m_nXScreen );
        int nScreenWidth  = aScreenSize.Width();
        int nScreenHeight = aScreenSize.Height();
        int nFrameWidth   = maGeometry.nWidth + maGeometry.nLeftDecoration + maGeometry.nRightDecoration;
        int nFrameHeight  = maGeometry.nHeight + maGeometry.nTopDecoration  + maGeometry.nBottomDecoration;

        if ((nFrameWidth > nScreenWidth) || (nFrameHeight > nScreenHeight))
        {
            Size aSize(maGeometry.nWidth, maGeometry.nHeight);

            if (nFrameWidth  > nScreenWidth)
                aSize.Width()  = nScreenWidth  - maGeometry.nRightDecoration - maGeometry.nLeftDecoration;
            if (nFrameHeight > nScreenHeight)
                aSize.Height() = nScreenHeight - maGeometry.nBottomDecoration - maGeometry.nTopDecoration;

            SetSize( aSize );
            bResized = false;
        }
    }
    if( bResized )
        CallCallback( SALEVENT_RESIZE, NULL );

    GetGenericData()->ErrorTrapPop();

    return 1;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long X11SalFrame::HandleColormapEvent( XColormapEvent* )
{
    return 0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long X11SalFrame::HandleStateEvent( XPropertyEvent *pEvent )
{
    Atom          actual_type;
    int           actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = NULL;

    if( 0 != XGetWindowProperty( GetXDisplay(),
                                 GetShellWindow(),
                                 pEvent->atom,          // property
                                 0,                     // long_offset (32bit)
                                 2,                     // long_length (32bit)
                                 False,                 // delete
                                 pEvent->atom,          // req_type
                                 &actual_type,
                                 &actual_format,
                                 &nitems,
                                 &bytes_after,
                                 &prop )
        || ! prop
        )
        return 0;

    DBG_ASSERT( actual_type = pEvent->atom
                && 32 == actual_format
                &&  2 == nitems
                &&  0 == bytes_after, "HandleStateEvent" );

    if( *(unsigned long*)prop == NormalState )
        nShowState_ = SHOWSTATE_NORMAL;
    else if( *(unsigned long*)prop == IconicState )
        nShowState_ = SHOWSTATE_MINIMIZED;

    XFree( prop );
    return 1;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long X11SalFrame::HandleClientMessage( XClientMessageEvent *pEvent )
{
    const WMAdaptor& rWMAdaptor( *pDisplay_->getWMAdaptor() );

#if !defined(__synchronous_extinput__)
    if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::SAL_EXTTEXTEVENT ) )
    {
        HandleExtTextEvent (pEvent);
        return 1;
    }
#endif
    else if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::SAL_QUITEVENT ) )
    {
        stderr0( "X11SalFrame::Dispatch Quit\n" );
        Close(); // ???
        return 1;
    }
    else if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::WM_PROTOCOLS ) )
    {
        if( (Atom)pEvent->data.l[0] == rWMAdaptor.getAtom( WMAdaptor::NET_WM_PING ) )
            rWMAdaptor.answerPing( this, pEvent );
        else if( ! ( nStyle_ & SAL_FRAME_STYLE_PLUG )
              && ! (( nStyle_ & SAL_FRAME_STYLE_FLOAT ) && (nStyle_ & SAL_FRAME_STYLE_OWNERDRAWDECORATION))
             )
        {
            if( (Atom)pEvent->data.l[0] == rWMAdaptor.getAtom( WMAdaptor::WM_DELETE_WINDOW ) )
            {
                Close();
                return 1;
            }
            else if( (Atom)pEvent->data.l[0] == rWMAdaptor.getAtom( WMAdaptor::WM_TAKE_FOCUS ) )
            {
                // do nothing, we set the input focus in ToTop() if necessary
    #if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "got WM_TAKE_FOCUS on %s window\n",
                         (nStyle_&SAL_FRAME_STYLE_OWNERDRAWDECORATION) ?
                         "ownerdraw" : "NON OWNERDRAW" );
    #endif
            }
        }
    }
    else if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::XEMBED ) &&
             pEvent->window == GetWindow() )
    {
        if( pEvent->data.l[1] == 1 || // XEMBED_WINDOW_ACTIVATE
            pEvent->data.l[1] == 2 )  // XEMBED_WINDOW_DEACTIVATE
        {
            XFocusChangeEvent aEvent;
            aEvent.type         = (pEvent->data.l[1] == 1 ? FocusIn : FocusOut);
            aEvent.serial       = pEvent->serial;
            aEvent.send_event   = True;
            aEvent.display      = pEvent->display;
            aEvent.window       = pEvent->window;
            aEvent.mode         = NotifyNormal;
            aEvent.detail       = NotifyDetailNone;
            HandleFocusEvent( &aEvent );
        }
    }
    return 0;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

extern "C"
{
Bool call_checkKeyReleaseForRepeat( Display* pDisplay, XEvent* pCheck, XPointer pX11SalFrame )
{
    return X11SalFrame::checkKeyReleaseForRepeat( pDisplay, pCheck, pX11SalFrame );
}
}

Bool X11SalFrame::checkKeyReleaseForRepeat( Display*, XEvent* pCheck, XPointer pX11SalFrame )
{
    X11SalFrame* pThis = (X11SalFrame*)pX11SalFrame;
    return
        pCheck->type            == XLIB_KeyPress &&
        pCheck->xkey.state      == pThis->nKeyState_ &&
        pCheck->xkey.keycode    == pThis->nKeyCode_ &&
        pCheck->xkey.time       == pThis->nReleaseTime_  ? True : False;
}

long X11SalFrame::Dispatch( XEvent *pEvent )
{
    long nRet = 0;

    if( -1 == nCaptured_ )
    {
        CaptureMouse( sal_True );
#ifdef DBG_UTIL
        if( -1 != nCaptured_ )
            pDisplay_->DbgPrintDisplayEvent("Captured", pEvent);
#endif
    }

    if( pEvent->xany.window == GetShellWindow() || pEvent->xany.window == GetWindow() )
    {
        switch( pEvent->type )
        {
            case XLIB_KeyPress:
                nKeyCode_   = pEvent->xkey.keycode;
                nKeyState_  = pEvent->xkey.state;
                nRet        = HandleKeyEvent( &pEvent->xkey );
                break;

            case KeyRelease:
                if( -1 == nCompose_ )
                {
                    nReleaseTime_ = pEvent->xkey.time;
                    XEvent aEvent;
                    if( XCheckIfEvent( pEvent->xkey.display, &aEvent, call_checkKeyReleaseForRepeat, (XPointer)this ) )
                        XPutBackEvent( pEvent->xkey.display, &aEvent );
                    else
                        nRet        = HandleKeyEvent( &pEvent->xkey );
                }
            break;

            case ButtonPress:
                // if we loose the focus in presentation mode
                // there are good chances that we never get it back
                // since the WM ignores us
                 if( IsOverrideRedirect() )
                 {
                     XSetInputFocus( GetXDisplay(), GetShellWindow(),
                             RevertToNone, CurrentTime );
                 }

            case ButtonRelease:
            case MotionNotify:
            case EnterNotify:
            case LeaveNotify:
                nRet = HandleMouseEvent( pEvent );
                break;

            case FocusIn:
            case FocusOut:
                nRet = HandleFocusEvent( &pEvent->xfocus );
                break;

            case Expose:
            case GraphicsExpose:
                nRet = HandleExposeEvent( pEvent );
                break;

            case MapNotify:
                if( pEvent->xmap.window == GetShellWindow() )
                {
                    if( nShowState_ == SHOWSTATE_HIDDEN )
                    {
                        /*
                         *  workaround for (at least) KWin 2.2.2
                         *  which will map windows that were once transient
                         *  even if they are withdrawn when the respective
                         *  document is mapped.
                         */
                        if( ! (nStyle_ & SAL_FRAME_STYLE_PLUG) )
                            XUnmapWindow( GetXDisplay(), GetShellWindow() );
                        break;
                    }
                    bMapped_   = sal_True;
                    bViewable_ = sal_True;
                    nRet = sal_True;
                    if ( mpInputContext != NULL )
                        mpInputContext->Map( this );
                    CallCallback( SALEVENT_RESIZE, NULL );

                    bool bSetFocus = m_bSetFocusOnMap;
                    /*  another workaround for sawfish: if a transient window for the same parent is shown
                     *  sawfish does not set the focus to it. Applies only for click to focus mode.
                     */
                    if( ! (nStyle_ & SAL_FRAME_STYLE_FLOAT ) && mbInShow && GetDisplay()->getWMAdaptor()->getWindowManagerName().EqualsAscii( "Sawfish" ) )
                    {
                        // don't set the focus into the IME status window
                        // since this will lead to a parent loose-focus, close status,
                        // reget focus, open status, .... flicker loop
                        if ( (I18NStatus::get().getStatusFrame() != this) )
                            bSetFocus = true;
                    }

                    /*
                     *  sometimes a message box/dialogue is brought up when a frame is not mapped
                     *  the corresponding TRANSIENT_FOR hint is then set to the root window
                     *  so that the dialogue shows in all cases. Correct it here if the
                     *  frame is shown afterwards.
                     */
                    if( ! IsChildWindow()
                        && ! IsOverrideRedirect()
                        && ! IsFloatGrabWindow()
                        )
                    {
                        for( std::list< X11SalFrame* >::const_iterator it = maChildren.begin();
                             it != maChildren.end(); ++it )
                        {
                            if( (*it)->mbTransientForRoot )
                                pDisplay_->getWMAdaptor()->changeReferenceFrame( *it, this );
                        }
                    }

                    if( hPresentationWindow != None && GetShellWindow() == hPresentationWindow )
                        XSetInputFocus( GetXDisplay(), GetShellWindow(), RevertToParent, CurrentTime );

                    if( bSetFocus )
                    {
                        XSetInputFocus( GetXDisplay(),
                                        GetShellWindow(),
                                        RevertToParent,
                                        CurrentTime );
                    }


                    RestackChildren();
                    mbInShow = sal_False;
                    m_bSetFocusOnMap = false;
                }
                break;

            case UnmapNotify:
                if( pEvent->xunmap.window == GetShellWindow() )
                {
                    bMapped_   = sal_False;
                    bViewable_ = sal_False;
                    nRet = sal_True;
                    if ( mpInputContext != NULL )
                        mpInputContext->Unmap( this );
                    CallCallback( SALEVENT_RESIZE, NULL );
                }
                break;

            case ConfigureNotify:
                if( pEvent->xconfigure.window == GetShellWindow()
                    || pEvent->xconfigure.window == GetWindow() )
                    nRet = HandleSizeEvent( &pEvent->xconfigure );
                break;

            case VisibilityNotify:
                nVisibility_ = pEvent->xvisibility.state;
                nRet = sal_True;
                if( bAlwaysOnTop_
                    && bMapped_
                    && ! GetDisplay()->getWMAdaptor()->isAlwaysOnTopOK()
                    && nVisibility_ != VisibilityUnobscured )
                    maAlwaysOnTopRaiseTimer.Start();
            break;

            case ReparentNotify:
                nRet = HandleReparentEvent( &pEvent->xreparent );
                break;

            case MappingNotify:
                if( MappingPointer != pEvent->xmapping.request )
                    nRet = CallCallback( SALEVENT_KEYBOARDCHANGED, 0 );
                break;

            case ColormapNotify:
                nRet = HandleColormapEvent( &pEvent->xcolormap );
                break;

            case PropertyNotify:
            {
                if( pEvent->xproperty.atom == pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_STATE ) )
                    nRet = HandleStateEvent( &pEvent->xproperty );
                else
                    nRet = pDisplay_->getWMAdaptor()->handlePropertyNotify( this, &pEvent->xproperty );
                break;
            }

            case ClientMessage:
                nRet = HandleClientMessage( &pEvent->xclient );
                break;
        }
    }
    else
    {
        switch( pEvent->type )
        {
             case FocusIn:
             case FocusOut:
                if( ( nStyle_ & SAL_FRAME_STYLE_PLUG )
                    && ( pEvent->xfocus.window == GetShellWindow()
                         || pEvent->xfocus.window == GetForeignParent() )
                    )
                {
                    nRet = HandleFocusEvent( &pEvent->xfocus );
                }
                 break;

            case ConfigureNotify:
                if( pEvent->xconfigure.window == GetForeignParent() ||
                    pEvent->xconfigure.window == GetShellWindow() )
                    nRet = HandleSizeEvent( &pEvent->xconfigure );

                if( pEvent->xconfigure.window == GetStackingWindow() )
                    nRet = HandleSizeEvent( &pEvent->xconfigure );

                RestackChildren();
                break;
        }
    }

    return nRet;
}

void X11SalFrame::ResetClipRegion()
{
    delete [] m_pClipRectangles;
    m_pClipRectangles = NULL;
    m_nCurClipRect = m_nMaxClipRect = 0;

    const int   dest_kind   = ShapeBounding;
    const int   op          = ShapeSet;
    const int   ordering    = YSorted;

    XWindowAttributes win_attrib;
    XRectangle        win_size;

    XLIB_Window aShapeWindow = mhShellWindow;

    XGetWindowAttributes ( GetDisplay()->GetDisplay(),
                           aShapeWindow,
                           &win_attrib );

    win_size.x      = 0;
    win_size.y      = 0;
    win_size.width  = win_attrib.width;
    win_size.height = win_attrib.height;

    XShapeCombineRectangles ( GetDisplay()->GetDisplay(),
                              aShapeWindow,
                              dest_kind,
                              0, 0,             // x_off, y_off
                              &win_size,        // list of rectangles
                              1,                // number of rectangles
                              op, ordering );
}

void X11SalFrame::BeginSetClipRegion( sal_uLong nRects )
{
    if( m_pClipRectangles )
        delete [] m_pClipRectangles;
    if( nRects )
        m_pClipRectangles = new XRectangle[nRects];
    else
        m_pClipRectangles = NULL;
    m_nMaxClipRect = static_cast<int>(nRects);
    m_nCurClipRect = 0;
}

void X11SalFrame::UnionClipRegion( long nX, long nY, long nWidth, long nHeight )
{
    if( m_pClipRectangles && m_nCurClipRect < m_nMaxClipRect )
    {
        m_pClipRectangles[m_nCurClipRect].x      = nX;
        m_pClipRectangles[m_nCurClipRect].y      = nY;
        m_pClipRectangles[m_nCurClipRect].width  = nWidth;
        m_pClipRectangles[m_nCurClipRect].height = nHeight;
        m_nCurClipRect++;
    }
}

void X11SalFrame::EndSetClipRegion()
{
    const int   dest_kind   = ShapeBounding;
    const int   ordering    = YSorted;
    const int   op = ShapeSet;

    XLIB_Window aShapeWindow = mhShellWindow;
    XShapeCombineRectangles ( GetDisplay()->GetDisplay(),
                              aShapeWindow,
                              dest_kind,
                              0, 0, // x_off, y_off
                              m_pClipRectangles,
                              m_nCurClipRect,
                              op, ordering );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
