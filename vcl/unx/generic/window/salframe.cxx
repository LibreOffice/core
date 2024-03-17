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

#include <string.h>
#include <string_view>
#include <stdio.h>
#include <stdlib.h>

#include <tools/debug.hxx>

#include <vcl/event.hxx>
#include <vcl/toolkit/floatwin.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/settings.hxx>
#include <vcl/BitmapReadAccess.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/BitmapTools.hxx>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/extensions/shape.h>

#include <headless/BitmapHelper.hxx>
#include <headless/svpbmp.hxx>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/salframe.h>
#include <unx/wmadaptor.hxx>
#include <unx/i18n_ic.hxx>
#include <unx/i18n_keysym.hxx>
#include <opengl/zone.hxx>

#include <unx/gensys.h>
#include <window.h>

#include <sal/macros.h>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <com/sun/star/uno/Exception.hpp>

#include <salinst.hxx>
#include <svdata.hxx>
#include <bitmaps.hlst>

#include <cairo-xlib.h>

#include <optional>

#include <algorithm>

#ifndef Button6
# define Button6 6
#endif
#ifndef Button7
# define Button7 7
#endif

using namespace vcl_sal;

constexpr auto CLIENT_EVENTS = StructureNotifyMask
                                | SubstructureNotifyMask
                                | KeyPressMask
                                | KeyReleaseMask
                                | ButtonPressMask
                                | ButtonReleaseMask
                                | PointerMotionMask
                                | EnterWindowMask
                                | LeaveWindowMask
                                | FocusChangeMask
                                | ExposureMask
                                | VisibilityChangeMask
                                | PropertyChangeMask
                                | ColormapChangeMask;

static ::Window  hPresentationWindow = None, hPresFocusWindow = None;
static ::std::list< ::Window > aPresentationReparentList;
static int          nVisibleFloats      = 0;

static void doReparentPresentationDialogues( SalDisplay const * pDisplay )
{
    GetGenericUnixSalData()->ErrorTrapPush();
    for (auto const& elem : aPresentationReparentList)
    {
        int x, y;
        ::Window aRoot, aChild;
        unsigned int w, h, bw, d;
        XGetGeometry( pDisplay->GetDisplay(),
                      elem,
                      &aRoot,
                      &x, &y, &w, &h, &bw, &d );
        XTranslateCoordinates( pDisplay->GetDisplay(),
                               hPresentationWindow,
                               aRoot,
                               x, y,
                               &x, &y,
                               &aChild );
        XReparentWindow( pDisplay->GetDisplay(),
                         elem,
                         aRoot,
                         x, y );
    }
    aPresentationReparentList.clear();
    if( hPresFocusWindow )
        XSetInputFocus( pDisplay->GetDisplay(), hPresFocusWindow, PointerRoot, CurrentTime );
    XSync( pDisplay->GetDisplay(), False );
    GetGenericUnixSalData()->ErrorTrapPop();
}

bool X11SalFrame::IsOverrideRedirect() const
{
    return
        ((nStyle_ & SalFrameStyleFlags::INTRO) && !pDisplay_->getWMAdaptor()->supportsSplash())
        ||
        (!( nStyle_ & ~SalFrameStyleFlags::DEFAULT ) && !pDisplay_->getWMAdaptor()->supportsFullScreen())
        ;
}

bool X11SalFrame::IsFloatGrabWindow() const
{
    static const char* pDisableGrab = getenv( "SAL_DISABLE_FLOATGRAB" );

    return
        ( ( !pDisableGrab || !*pDisableGrab ) &&
          (
           (nStyle_ & SalFrameStyleFlags::FLOAT)    &&
           ! (nStyle_ & SalFrameStyleFlags::TOOLTIP)    &&
           ! (nStyle_ & SalFrameStyleFlags::OWNERDRAWDECORATION)
           )
          );
}

void X11SalFrame::setXEmbedInfo()
{
    if( !m_bXEmbed )
        return;

    tools::Long aInfo[2];
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

    GetGenericUnixSalData()->ErrorTrapPush();
    XSendEvent( pDisplay_->GetDisplay(),
                mhForeignParent,
                False, NoEventMask, &aEvent );
    XSync( pDisplay_->GetDisplay(), False );
    GetGenericUnixSalData()->ErrorTrapPop();
}

typedef std::vector< unsigned long > NetWmIconData;

namespace
{
    constexpr OUString SV_ICON_SIZE48[] =
    {
        MAINAPP_48_8,
        MAINAPP_48_8,
        ODT_48_8,
        OTT_48_8,
        ODS_48_8,
        OTS_48_8,
        ODG_48_8,
        MAINAPP_48_8,
        ODP_48_8,
        MAINAPP_48_8,
        ODM_48_8,
        MAINAPP_48_8,
        ODB_48_8,
        ODF_48_8
    };

    constexpr OUString SV_ICON_SIZE32[] =
    {
        MAINAPP_32_8,
        MAINAPP_32_8,
        ODT_32_8,
        OTT_32_8,
        ODS_32_8,
        OTS_32_8,
        ODG_32_8,
        MAINAPP_32_8,
        ODP_32_8,
        MAINAPP_32_8,
        ODM_32_8,
        MAINAPP_32_8,
        ODB_32_8,
        ODF_32_8
    };

    constexpr OUString SV_ICON_SIZE16[] =
    {
        MAINAPP_16_8,
        MAINAPP_16_8,
        ODT_16_8,
        OTT_16_8,
        ODS_16_8,
        OTS_16_8,
        ODG_16_8,
        MAINAPP_16_8,
        ODP_16_8,
        MAINAPP_16_8,
        ODM_16_8,
        MAINAPP_16_8,
        ODB_16_8,
        ODF_16_8
    };
}

static void CreateNetWmAppIcon( sal_uInt16 nIcon, NetWmIconData& netwm_icon )
{
    const int sizes[ 3 ] = { 48, 32, 16 };
    netwm_icon.resize( 48 * 48 + 32 * 32 + 16 * 16 + 3 * 2 );
    int pos = 0;
    for(int size : sizes)
    {
        OUString sIcon;
        if( size >= 48 )
            sIcon = SV_ICON_SIZE48[nIcon];
        else if( size >= 32 )
            sIcon = SV_ICON_SIZE32[nIcon];
        else
            sIcon = SV_ICON_SIZE16[nIcon];

        BitmapEx aIcon = vcl::bitmap::loadFromName(sIcon, ImageLoadFlags::IgnoreScalingFactor);

        if( aIcon.IsEmpty())
            continue;
        vcl::bitmap::convertBitmap32To24Plus8(aIcon, aIcon);
        Bitmap icon = aIcon.GetBitmap();
        AlphaMask mask = aIcon.GetAlphaMask();
        BitmapScopedReadAccess iconData(icon);
        BitmapScopedReadAccess maskData(mask);
        netwm_icon[ pos++ ] = size; // width
        netwm_icon[ pos++ ] = size; // height
        for( int y = 0; y < size; ++y )
            for( int x = 0; x < size; ++x )
            {
                BitmapColor col = iconData->GetColor( y, x );
                BitmapColor alpha = maskData->GetColor( y, x );
                netwm_icon[ pos++ ] = (((( 255 - alpha.GetBlue()) * 256U ) + col.GetRed()) * 256 + col.GetGreen()) * 256 + col.GetBlue();
            }
    }
    netwm_icon.resize( pos );
}

void X11SalFrame::Init( SalFrameStyleFlags nSalFrameStyle, SalX11Screen nXScreen, SystemParentData const * pParentData, bool bUseGeometry )
{
    if( nXScreen.getXScreen() >= GetDisplay()->GetXScreenCount() )
        nXScreen = GetDisplay()->GetDefaultXScreen();
    if( mpParent )
        nXScreen = mpParent->m_nXScreen;

    m_nXScreen  = nXScreen;
    nStyle_     = nSalFrameStyle;
    XWMHints Hints;
    Hints.flags = InputHint;
    Hints.input = (nSalFrameStyle & SalFrameStyleFlags::OWNERDRAWDECORATION) ? False : True;
    NetWmIconData netwm_icon;

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
    ::Window aFrameParent = pParentData ? pParentData->aWindow : GetDisplay()->GetRootWindow( m_nXScreen );
    ::Window aClientLeader = None;

    if( bUseGeometry )
    {
        x = maGeometry.x();
        y = maGeometry.y();
        w = maGeometry.width();
        h = maGeometry.height();
    }

    if( (nSalFrameStyle & SalFrameStyleFlags::FLOAT) &&
        ! (nSalFrameStyle & SalFrameStyleFlags::OWNERDRAWDECORATION)
        )
    {
        if( nShowState_ == X11ShowState::Unknown )
        {
            w = 10;
            h = 10;
        }
        Attributes.override_redirect = True;
    }
    else if( nSalFrameStyle & SalFrameStyleFlags::SYSTEMCHILD )
    {
        SAL_WARN_IF( !mpParent, "vcl", "SalFrameStyleFlags::SYSTEMCHILD window without parent" );
        if( mpParent )
        {
            aFrameParent = mpParent->mhWindow;
            // FIXME: since with SalFrameStyleFlags::SYSTEMCHILD
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
        // process; start permanently ignoring X errors...
        GetGenericUnixSalData()->ErrorTrapPush();

        nStyle_ |= SalFrameStyleFlags::PLUG;
        Attributes.override_redirect = True;
        if( pParentData->nSize >= sizeof(SystemParentData) )
            m_bXEmbed = pParentData->bXEmbedSupport;

        int x_ret, y_ret;
        unsigned int bw, d;
        ::Window aRoot, aParent;

        XGetGeometry( GetXDisplay(), pParentData->aWindow,
                      &aRoot, &x_ret, &y_ret, &w, &h, &bw, &d );
        mhForeignParent = pParentData->aWindow;

        mhShellWindow = aParent = mhForeignParent;
        ::Window* pChildren;
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
        bool bIsReallyOurFrame = false;
        for (auto pSalFrame : GetDisplay()->getFrames() )
            if ( static_cast<const X11SalFrame*>( pSalFrame )->GetWindow() == mhForeignParent )
            {
                bIsReallyOurFrame = true;
                break;
            }
        if (!bIsReallyOurFrame)
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
            if( nSalFrameStyle & SalFrameStyleFlags::SIZEABLE &&
                nSalFrameStyle & SalFrameStyleFlags::MOVEABLE )
            {
                Size aBestFitSize(bestmaxFrameSizeForScreenSize(aScreenSize));
                w = aBestFitSize.Width();
                h = aBestFitSize.Height();
            }
            if( ! mpParent )
            {
                // find the last document window (if any)
                const X11SalFrame* pFrame = nullptr;
                bool bIsDocumentWindow = false;
                for (auto pSalFrame : GetDisplay()->getFrames() )
                {
                    pFrame = static_cast< const X11SalFrame* >( pSalFrame );
                    if( !pFrame->mpParent
                        && !pFrame->mbFullScreen
                        && ( pFrame->nStyle_ & SalFrameStyleFlags::SIZEABLE )
                        && pFrame->GetUnmirroredGeometry().width()
                        && pFrame->GetUnmirroredGeometry().height() )
                    {
                        bIsDocumentWindow = true;
                        break;
                    }
                }

                if( bIsDocumentWindow )
                {
                    // set a document position and size
                    // the first frame gets positioned by the window manager
                    const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
                    x = rGeom.x();
                    y = rGeom.y();
                    if( x+static_cast<int>(w)+40 <= static_cast<int>(aScreenSize.Width()) &&
                        y+static_cast<int>(h)+40 <= static_cast<int>(aScreenSize.Height())
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
                    ::Window aRoot, aChild;
                    int root_x = 0, root_y = 0, lx, ly;
                    unsigned int mask;
                    XQueryPointer( GetXDisplay(),
                                   GetDisplay()->GetRootWindow( m_nXScreen ),
                                   &aRoot, &aChild,
                                   &root_x, &root_y, &lx, &ly, &mask );
                    const std::vector< AbsoluteScreenPixelRectangle >& rScreens = GetDisplay()->GetXineramaScreens();
                    for(const auto & rScreen : rScreens)
                        if( rScreen.Contains( AbsoluteScreenPixelPoint( root_x, root_y ) ) )
                        {
                            x = rScreen.Left();
                            y = rScreen.Top();
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
        if( !(nStyle_ & SalFrameStyleFlags::INTRO) && !(nStyle_ & SalFrameStyleFlags::NOICON))
        {
            try
            {
                CreateNetWmAppIcon( mnIconID != SV_ICON_ID_OFFICE ? mnIconID :
                                    (mpParent ? mpParent->mnIconID : SV_ICON_ID_OFFICE), netwm_icon );
            }
            catch( css::uno::Exception& )
            {
                // can happen - no ucb during early startup
            }
        }

        // find the top level frame of the transience hierarchy
        X11SalFrame* pFrame = this;
        while( pFrame->mpParent )
            pFrame = pFrame->mpParent;
        if( pFrame->nStyle_ & SalFrameStyleFlags::PLUG )
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
                    if( pWMHints->flags & WindowGroupHint )
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

    nShowState_                 = X11ShowState::Unknown;
    bViewable_                  = true;
    bMapped_                    = false;
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
    mpSurface = cairo_xlib_surface_create(GetXDisplay(), mhWindow,
                                          rVis.GetVisual(),
                                          w, h);

    // FIXME: see above: fake shell window for now to own window
    if( pParentData == nullptr )
    {
        mhShellWindow = mhWindow;
    }

    // correct window group if necessary
    if( (Hints.flags & WindowGroupHint) == WindowGroupHint )
    {
        if( Hints.window_group == None )
            Hints.window_group = GetShellWindow();
    }

    maGeometry.setPosSize({ x, y }, { static_cast<tools::Long>(w), static_cast<tools::Long>(h) });
    updateScreenNumber();

    XSync( GetXDisplay(), False );
    setXEmbedInfo();

    Time nUserTime = (nStyle_ & (SalFrameStyleFlags::OWNERDRAWDECORATION | SalFrameStyleFlags::TOOLWINDOW) ) == SalFrameStyleFlags::NONE ?
        pDisplay_->GetLastUserEventTime() : 0;
    pDisplay_->getWMAdaptor()->setUserTime( this, nUserTime );

    if( ! pParentData && ! IsChildWindow() && ! Attributes.override_redirect )
    {
        XSetWMHints( GetXDisplay(), mhWindow, &Hints );
        // WM Protocols && internals
        Atom a[3];
        int  n = 0;
        a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::WM_DELETE_WINDOW );

// LibreOffice advertises NET_WM_PING atom, so mutter rightfully warns of an unresponsive application during debugging.
// Hack that out unconditionally for debug builds, as per https://bugzilla.redhat.com/show_bug.cgi?id=981149
// upstream refuses to make this configurable in any way.
// NOTE: You need to use the 'gen' backend for this to work (SAL_USE_VCLPLUGIN=gen)
#if OSL_DEBUG_LEVEL < 1
        if( pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::NET_WM_PING ) )
            a[n++] = pDisplay_->getWMAdaptor()->getAtom( WMAdaptor::NET_WM_PING );
#endif

        if( nSalFrameStyle & SalFrameStyleFlags::OWNERDRAWDECORATION )
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
                             reinterpret_cast<unsigned char*>(&aClientLeader),
                             1
                             );
        }
#define DECOFLAGS (SalFrameStyleFlags::MOVEABLE | SalFrameStyleFlags::SIZEABLE | SalFrameStyleFlags::CLOSEABLE)
        int nDecoFlags = WMAdaptor::decoration_All;
        if (m_bIsPartialFullScreen || (nStyle_ & SalFrameStyleFlags::OWNERDRAWDECORATION))
            nDecoFlags = 0;
        else if( (nStyle_ & DECOFLAGS ) != DECOFLAGS || (nStyle_ & SalFrameStyleFlags::TOOLWINDOW) )
        {
            if( nStyle_ & DECOFLAGS )
                // if any decoration, then show a border
                nDecoFlags = WMAdaptor::decoration_Border;
            else
                nDecoFlags = 0;

            if( ! mpParent && (nStyle_ & DECOFLAGS) )
                // don't add a min button if window should be decorationless
                nDecoFlags |= WMAdaptor::decoration_MinimizeBtn;
            if( nStyle_ & SalFrameStyleFlags::CLOSEABLE )
                nDecoFlags |= WMAdaptor::decoration_CloseBtn;
            if( nStyle_ & SalFrameStyleFlags::SIZEABLE )
            {
                nDecoFlags |= WMAdaptor::decoration_Resize;
                if( ! (nStyle_ & SalFrameStyleFlags::TOOLWINDOW) )
                    nDecoFlags |= WMAdaptor::decoration_MaximizeBtn;
            }
            if( nStyle_ & SalFrameStyleFlags::MOVEABLE )
                nDecoFlags |= WMAdaptor::decoration_Title;
        }

        WMWindowType eType = WMWindowType::Normal;
        if( nStyle_ & SalFrameStyleFlags::INTRO )
            eType = WMWindowType::Splash;
        if( (nStyle_ & SalFrameStyleFlags::DIALOG) && hPresentationWindow == None )
            eType = WMWindowType::ModelessDialogue;
        if( nStyle_ & SalFrameStyleFlags::TOOLWINDOW )
            eType = WMWindowType::Utility;
        if( nStyle_ & SalFrameStyleFlags::OWNERDRAWDECORATION )
            eType = WMWindowType::Toolbar;
        if (m_bIsPartialFullScreen && GetDisplay()->getWMAdaptor()->isLegacyPartialFullscreen())
            eType = WMWindowType::Dock;

        GetDisplay()->getWMAdaptor()->
            setFrameTypeAndDecoration( this,
                                       eType,
                                       nDecoFlags,
                                       hPresentationWindow ? nullptr : mpParent );

        if (!m_bIsPartialFullScreen && (nStyle_ & (SalFrameStyleFlags::DEFAULT |
                        SalFrameStyleFlags::OWNERDRAWDECORATION|
                        SalFrameStyleFlags::FLOAT |
                        SalFrameStyleFlags::INTRO))
             == SalFrameStyleFlags::DEFAULT )
            pDisplay_->getWMAdaptor()->maximizeFrame( this );

        if( !netwm_icon.empty() && GetDisplay()->getWMAdaptor()->getAtom( WMAdaptor::NET_WM_ICON ))
            XChangeProperty( GetXDisplay(), mhWindow,
                GetDisplay()->getWMAdaptor()->getAtom( WMAdaptor::NET_WM_ICON ),
                XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<unsigned char*>(netwm_icon.data()), netwm_icon.size());
    }

    m_nWorkArea = GetDisplay()->getWMAdaptor()->getCurrentWorkArea();

    // Pointer
    SetPointer( PointerStyle::Arrow );
}

X11SalFrame::X11SalFrame( SalFrame *pParent, SalFrameStyleFlags nSalFrameStyle,
                          SystemParentData const * pSystemParent ) :
    m_nXScreen( 0 ),
    maAlwaysOnTopRaiseTimer( "vcl::X11SalFrame maAlwaysOnTopRaiseTimer" )
{
    GenericUnixSalData *pData = GetGenericUnixSalData();

    mpParent                    = static_cast< X11SalFrame* >( pParent );

    mbTransientForRoot          = false;

    pDisplay_                   = vcl_sal::getSalDisplay(pData);
    // insert frame in framelist
    pDisplay_->registerFrame( this );

    mhWindow                    = None;
    mpSurface                   = nullptr;
    mhShellWindow               = None;
    mhStackingWindow            = None;
    mhForeignParent             = None;
    m_bSetFocusOnMap            = false;

    pGraphics_                  = nullptr;
    pFreeGraphics_              = nullptr;

    hCursor_                    = None;
    nCaptured_                  = 0;

    mbSendExtKeyModChange       = false;
    mnExtKeyMod                 = ModKeyFlags::NONE;

    nShowState_                 = X11ShowState::Unknown;
    nWidth_                     = 0;
    nHeight_                    = 0;
    nStyle_                     = SalFrameStyleFlags::NONE;
    mnExtStyle                  = 0;
    bAlwaysOnTop_               = false;

    // set bViewable_ to true: hack GetClientSize to report something
    // different to 0/0 before first map
    bViewable_                  = true;
    bMapped_                    = false;
    bDefaultPosition_           = true;
    nVisibility_                = VisibilityFullyObscured;
    m_nWorkArea                 = 0;
    m_bXEmbed                   = false;


    mpInputContext              = nullptr;
    mbInputFocus                = False;

    maAlwaysOnTopRaiseTimer.SetInvokeHandler( LINK( this, X11SalFrame, HandleAlwaysOnTopRaise ) );
    maAlwaysOnTopRaiseTimer.SetTimeout( 100 );

    meWindowType                = WMWindowType::Normal;
    mbMaximizedVert             = false;
    mbMaximizedHorz             = false;
    mbFullScreen                = false;
    m_bIsPartialFullScreen = false;

    mnIconID                    = SV_ICON_ID_OFFICE;

    if( mpParent )
        mpParent->maChildren.push_back( this );

    Init( nSalFrameStyle, GetDisplay()->GetDefaultXScreen(), pSystemParent );
}

X11SalFrame::~X11SalFrame()
{
    notifyDelete();

    m_vClipRectangles.clear();

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

    ShowFullScreen( false, 0 );

    if( bMapped_ )
        Show( false );

    if( mpInputContext )
    {
        mpInputContext->UnsetICFocus();
        mpInputContext->Unmap();
        mpInputContext.reset();
    }

    if( GetWindow() == hPresentationWindow )
    {
        hPresentationWindow = None;
        doReparentPresentationDialogues( GetDisplay() );
    }

    if( pGraphics_ )
    {
        pGraphics_->DeInit();
        pGraphics_.reset();
    }

    if( pFreeGraphics_ )
    {
        pFreeGraphics_->DeInit();
        pFreeGraphics_.reset();
    }

    // reset all OpenGL contexts using this window
    rtl::Reference<OpenGLContext> pContext = ImplGetSVData()->maGDIData.mpLastContext;
    while( pContext.is() )
    {
        if (static_cast<const GLX11Window&>(pContext->getOpenGLWindow()).win == mhWindow)
            pContext->reset();
        pContext = pContext->mpPrevContext;
    }

    if (mpSurface)
        cairo_surface_destroy(mpSurface);

    XDestroyWindow( GetXDisplay(), mhWindow );
}

void X11SalFrame::SetExtendedFrameStyle( SalExtStyle nStyle )
{
    if( nStyle != mnExtStyle && ! IsChildWindow() )
    {
        mnExtStyle = nStyle;
        updateWMClass();
    }
}

const SystemEnvData* X11SalFrame::GetSystemData() const
{
    X11SalFrame *pFrame = const_cast<X11SalFrame*>(this);
    pFrame->maSystemChildData.pDisplay      = GetXDisplay();
    pFrame->maSystemChildData.SetWindowHandle(pFrame->GetWindow());
    pFrame->maSystemChildData.pSalFrame     = pFrame;
    pFrame->maSystemChildData.pWidget       = nullptr;
    pFrame->maSystemChildData.pVisual       = GetDisplay()->GetVisual( m_nXScreen ).GetVisual();
    pFrame->maSystemChildData.nScreen       = m_nXScreen.getXScreen();
    pFrame->maSystemChildData.aShellWindow  = pFrame->GetShellWindow();
    pFrame->maSystemChildData.toolkit       = SystemEnvData::Toolkit::Gen;
    pFrame->maSystemChildData.platform      = SystemEnvData::Platform::Xcb;
    return &maSystemChildData;
}

SalGraphics *X11SalFrame::AcquireGraphics()
{
    if( pGraphics_ )
        return nullptr;

    if( pFreeGraphics_ )
    {
        pGraphics_      = std::move(pFreeGraphics_);
    }
    else
    {
        pGraphics_.reset(new X11SalGraphics());
        pGraphics_->Init(*this, GetWindow(), m_nXScreen);
    }

    return pGraphics_.get();
}

void X11SalFrame::ReleaseGraphics( SalGraphics *pGraphics )
{
    SAL_WARN_IF( pGraphics != pGraphics_.get(), "vcl", "SalFrame::ReleaseGraphics pGraphics!=pGraphics_" );

    if( pGraphics != pGraphics_.get() )
        return;

    pFreeGraphics_  = std::move(pGraphics_);
}

void X11SalFrame::updateGraphics( bool bClear )
{
    Drawable aDrawable = bClear ? None : GetWindow();
    if( pGraphics_ )
        pGraphics_->SetDrawable( aDrawable, mpSurface, m_nXScreen );
    if( pFreeGraphics_ )
        pFreeGraphics_->SetDrawable( aDrawable, mpSurface, m_nXScreen );
}

void X11SalFrame::SetIcon( sal_uInt16 nIcon )
{
    if (  IsChildWindow() )
        return;

    // 0 == default icon -> #1
    if ( nIcon == 0 )
        nIcon = 1;

    mnIconID = nIcon;

    NetWmIconData netwm_icon;
    CreateNetWmAppIcon( nIcon, netwm_icon );

    if( !netwm_icon.empty() && GetDisplay()->getWMAdaptor()->getAtom( WMAdaptor::NET_WM_ICON ))
        XChangeProperty( GetXDisplay(), mhWindow,
            GetDisplay()->getWMAdaptor()->getAtom( WMAdaptor::NET_WM_ICON ),
            XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<unsigned char*>(netwm_icon.data()), netwm_icon.size());
}

void X11SalFrame::SetMaxClientSize( tools::Long nWidth, tools::Long nHeight )
{
    if(  IsChildWindow() )
        return;

    if( !GetShellWindow() ||
        (nStyle_ & (SalFrameStyleFlags::FLOAT|SalFrameStyleFlags::OWNERDRAWDECORATION) ) == SalFrameStyleFlags::FLOAT )
        return;

    XSizeHints* pHints = XAllocSizeHints();
    tools::Long nSupplied = 0;
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

void X11SalFrame::SetMinClientSize( tools::Long nWidth, tools::Long nHeight )
{
    if(  IsChildWindow() )
        return;

    if( !GetShellWindow() ||
        (nStyle_ & (SalFrameStyleFlags::FLOAT|SalFrameStyleFlags::OWNERDRAWDECORATION) ) == SalFrameStyleFlags::FLOAT )
        return;

    XSizeHints* pHints = XAllocSizeHints();
    tools::Long nSupplied = 0;
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

// Show + Pos (x,y,z) + Size (width,height)

void X11SalFrame::Show( bool bVisible, bool bNoActivate )
{
    if( ( bVisible && bMapped_ )
        || ( !bVisible && !bMapped_ ) )
        return;

    // HACK: this is a workaround for (at least) kwin
    // even though transient frames should be kept above their parent
    // this does not necessarily hold true for DOCK type windows
    // so artificially set ABOVE and remove it again on hide
    if( mpParent && mpParent->m_bIsPartialFullScreen && pDisplay_->getWMAdaptor()->isLegacyPartialFullscreen())
        pDisplay_->getWMAdaptor()->enableAlwaysOnTop( this, bVisible );

    bMapped_   = bVisible;
    bViewable_ = bVisible;
    setXEmbedInfo();
    if( bVisible )
    {
        if( ! (nStyle_ & SalFrameStyleFlags::INTRO) )
        {
            // hide all INTRO frames
            for (auto pSalFrame : GetDisplay()->getFrames() )
            {
                const X11SalFrame* pFrame = static_cast< const X11SalFrame* >( pSalFrame );
                // look for intro bit map; if present, hide it
                if( pFrame->nStyle_ & SalFrameStyleFlags::INTRO )
                {
                    if( pFrame->bMapped_ )
                        const_cast<X11SalFrame*>(pFrame)->Show( false );
                }
            }
        }

        // update NET_WM_STATE which may have been deleted due to earlier Show(false)
        if( nShowState_ == X11ShowState::Hidden )
            GetDisplay()->getWMAdaptor()->frameIsMapping( this );

        /*
         *  Actually this is rather exotic and currently happens only in conjunction
         *  with the basic dialogue editor,
         *  which shows a frame and instantly hides it again. After that the
         *  editor window is shown and the WM takes this as an opportunity
         *  to show our hidden transient frame also. So Show( false ) must
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

        Time nUserTime = 0;
        if( ! bNoActivate && !(nStyle_ & SalFrameStyleFlags::OWNERDRAWDECORATION) )
            nUserTime = pDisplay_->GetX11ServerTime();
        GetDisplay()->getWMAdaptor()->setUserTime( this, nUserTime );
        if( ! bNoActivate && (nStyle_ & SalFrameStyleFlags::TOOLWINDOW) )
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
            if( nStyle_ & SalFrameStyleFlags::FLOAT )
                XMapRaised( GetXDisplay(), GetWindow() );
            else
                XMapWindow( GetXDisplay(), GetWindow() );
        }
        XSelectInput( GetXDisplay(), GetWindow(), CLIENT_EVENTS );

        if( maGeometry.width() > 0
            && maGeometry.height() > 0
            && (   nWidth_  != static_cast<int>(maGeometry.width())
                || nHeight_ != static_cast<int>(maGeometry.height()) ) )
        {
            nWidth_  = maGeometry.width();
            nHeight_ = maGeometry.height();
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
        CallCallback( SalEvent::Resize, nullptr );

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
            for (auto const& child : maChildren)
            {
                if( child->mbTransientForRoot )
                    GetDisplay()->getWMAdaptor()->changeReferenceFrame( child, this );
            }
        }
        /*
         *  leave X11ShowState::Unknown as this indicates first mapping
         *  and is only reset int HandleSizeEvent
         */
        if( nShowState_ != X11ShowState::Unknown )
            nShowState_ = X11ShowState::Normal;

        /*
         *  plugged windows don't necessarily get the
         *  focus on show because the parent may already be mapped
         *  and have the focus. So try to set the focus
         *  to the child on Show(true)
         */
        if( (nStyle_ & SalFrameStyleFlags::PLUG) && ! m_bXEmbed )
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
            getInputContext()->Unmap();

        if( ! IsChildWindow() )
        {
            /*  FIXME: Is deleting the property really necessary ? It hurts
             *  owner drawn windows at least.
             */
            if( mpParent && ! (nStyle_ & SalFrameStyleFlags::OWNERDRAWDECORATION) )
                XDeleteProperty( GetXDisplay(), GetShellWindow(), GetDisplay()->getWMAdaptor()->getAtom( WMAdaptor::WM_TRANSIENT_FOR ) );
            XWithdrawWindow( GetXDisplay(), GetShellWindow(), m_nXScreen.getXScreen() );
        }
        else if( ! m_bXEmbed )
            XUnmapWindow( GetXDisplay(), GetWindow() );

        nShowState_ = X11ShowState::Hidden;
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

void X11SalFrame::ToTop( SalFrameToTop nFlags )
{
    if( ( nFlags & SalFrameToTop::RestoreWhenMin )
        && ! ( nStyle_ & SalFrameStyleFlags::FLOAT )
        && nShowState_ != X11ShowState::Hidden
        && nShowState_ != X11ShowState::Unknown
        )
    {
        GetDisplay()->getWMAdaptor()->frameIsMapping( this );
        if( GetWindow() != GetShellWindow() && ! IsSysChildWindow() )
            XMapWindow( GetXDisplay(), GetShellWindow() );
        XMapWindow( GetXDisplay(), GetWindow() );
    }

    ::Window aToTopWindow = IsSysChildWindow() ? GetWindow() : GetShellWindow();
    if( ! (nFlags & SalFrameToTop::GrabFocusOnly) )
    {
        XRaiseWindow( GetXDisplay(), aToTopWindow );
    }

    if( ( ( nFlags & SalFrameToTop::GrabFocus ) || ( nFlags & SalFrameToTop::GrabFocusOnly ) )
        && bMapped_ )
    {
        if( m_bXEmbed )
            askForXEmbedFocus( 0 );
        else
            XSetInputFocus( GetXDisplay(), aToTopWindow, RevertToParent, CurrentTime );
    }
    else if( ( nFlags & SalFrameToTop::RestoreWhenMin ) || ( nFlags & SalFrameToTop::ForegroundTask ) )
    {
            Time nTimestamp = pDisplay_->GetX11ServerTime();
            GetDisplay()->getWMAdaptor()->activateWindow( this, nTimestamp );
    }
}

void X11SalFrame::GetWorkArea( AbsoluteScreenPixelRectangle& rWorkArea )
{
    rWorkArea = pDisplay_->getWMAdaptor()->getWorkArea( 0 );
}

void X11SalFrame::GetClientSize( tools::Long &rWidth, tools::Long &rHeight )
{
    if( ! bViewable_  )
    {
        rWidth = rHeight = 0;
        return;
    }

    rWidth  = maGeometry.width();
    rHeight = maGeometry.height();

    if( !rWidth || !rHeight )
    {
        XWindowAttributes aAttrib;

        XGetWindowAttributes( GetXDisplay(), GetWindow(), &aAttrib );

        rWidth = aAttrib.width;
        rHeight = aAttrib.height;
        maGeometry.setSize({ aAttrib.width, aAttrib.height });
    }
}

void X11SalFrame::Center( )
{
    int             nX, nY;
    AbsoluteScreenPixelSize aRealScreenSize(GetDisplay()->getDataForScreen(m_nXScreen).m_aSize);
    AbsoluteScreenPixelRectangle aScreen({ 0, 0 }, aRealScreenSize);

    if( GetDisplay()->IsXinerama() )
    {
        // get xinerama screen we are on
        // if there is a parent, use its center for screen determination
        // else use the pointer
        ::Window aRoot, aChild;
        int root_x, root_y, x, y;
        unsigned int mask;
        if( mpParent )
        {
            root_x = mpParent->maGeometry.x() + mpParent->maGeometry.width() / 2;
            root_y = mpParent->maGeometry.y() + mpParent->maGeometry.height() / 2;
        }
        else
            XQueryPointer( GetXDisplay(),
                           GetShellWindow(),
                           &aRoot, &aChild,
                           &root_x, &root_y,
                           &x, &y,
                           &mask );
        const std::vector< AbsoluteScreenPixelRectangle >& rScreens = GetDisplay()->GetXineramaScreens();
        for(const auto & rScreen : rScreens)
            if( rScreen.Contains( AbsoluteScreenPixelPoint( root_x, root_y ) ) )
            {
                aScreen.SetPos(rScreen.GetPos());
                aRealScreenSize = rScreen.GetSize();
                break;
            }
    }

    if( mpParent )
    {
        X11SalFrame* pFrame = mpParent;
        while( pFrame->mpParent )
            pFrame = pFrame->mpParent;
        if( pFrame->maGeometry.width() < 1  || pFrame->maGeometry.height() < 1 )
        {
            AbsoluteScreenPixelRectangle aRect;
            pFrame->GetPosSize( aRect );
            pFrame->maGeometry.setPosSize(tools::Rectangle(aRect));
        }

        if( pFrame->nStyle_ & SalFrameStyleFlags::PLUG )
        {
            ::Window aRoot;
            unsigned int nScreenWidth, nScreenHeight, bw, depth;
            int nScreenX, nScreenY;
            XGetGeometry( GetXDisplay(),
                          pFrame->GetShellWindow(),
                          &aRoot,
                          &nScreenX, &nScreenY,
                          &nScreenWidth, &nScreenHeight,
                          &bw, &depth );
            aScreen = {{ nScreenX, nScreenY }, Size(nScreenWidth, nScreenHeight)};
        }
        else
            aScreen = AbsoluteScreenPixelRectangle(pFrame->maGeometry.posSize());
    }

    if( mpParent && mpParent->nShowState_ == X11ShowState::Normal )
    {
        if( maGeometry.width() >= mpParent->maGeometry.width() &&
            maGeometry.height() >= mpParent->maGeometry.height() )
        {
            nX = aScreen.getX() + 40;
            nY = aScreen.getY() + 40;
        }
        else
        {
            // center the window relative to the top level frame
            nX = (aScreen.GetWidth()  - static_cast<int>(maGeometry.width()) ) / 2 + aScreen.getX();
            nY = (aScreen.GetHeight() - static_cast<int>(maGeometry.height())) / 2 + aScreen.getY();
        }
    }
    else
    {
        // center the window relative to screen
        nX = (aRealScreenSize.getWidth()  - static_cast<int>(maGeometry.width()) ) / 2 + aScreen.getX();
        nY = (aRealScreenSize.getHeight() - static_cast<int>(maGeometry.height())) / 2 + aScreen.getY();
    }
    nX = nX < 0 ? 0 : nX;
    nY = nY < 0 ? 0 : nY;

    bDefaultPosition_ = False;
    if( mpParent )
    {
        nX -= mpParent->maGeometry.x();
        nY -= mpParent->maGeometry.y();
    }

    SetPosSize({ { nX, nY }, maGeometry.size() });
}

void X11SalFrame::updateScreenNumber()
{
    if( GetDisplay()->IsXinerama() && GetDisplay()->GetXineramaScreens().size() > 1 )
    {
        AbsoluteScreenPixelPoint aPoint( maGeometry.x(), maGeometry.y() );
        const std::vector<AbsoluteScreenPixelRectangle>& rScreenRects( GetDisplay()->GetXineramaScreens() );
        size_t nScreens = rScreenRects.size();
        for( size_t i = 0; i < nScreens; i++ )
        {
            if( rScreenRects[i].Contains( aPoint ) )
            {
                maGeometry.setScreen(static_cast<unsigned int>(i));
                break;
            }
        }
    }
    else
        maGeometry.setScreen(m_nXScreen.getXScreen());
}

void X11SalFrame::SetPosSize( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, sal_uInt16 nFlags )
{
    if( nStyle_ & SalFrameStyleFlags::PLUG )
        return;

    // relative positioning in X11SalFrame::SetPosSize
    AbsoluteScreenPixelRectangle aPosSize( AbsoluteScreenPixelPoint( maGeometry.x(), maGeometry.y() ), AbsoluteScreenPixelSize( maGeometry.width(), maGeometry.height() ) );
    aPosSize.Normalize();

    if( ! ( nFlags & SAL_FRAME_POSSIZE_X ) )
    {
        nX = aPosSize.Left();
        if( mpParent )
            nX -= mpParent->maGeometry.x();
    }
    if( ! ( nFlags & SAL_FRAME_POSSIZE_Y ) )
    {
        nY = aPosSize.Top();
        if( mpParent )
            nY -= mpParent->maGeometry.y();
    }
    if( ! ( nFlags & SAL_FRAME_POSSIZE_WIDTH ) )
        nWidth = aPosSize.GetWidth();
    if( ! ( nFlags & SAL_FRAME_POSSIZE_HEIGHT ) )
        nHeight = aPosSize.GetHeight();

    aPosSize = AbsoluteScreenPixelRectangle( AbsoluteScreenPixelPoint( nX, nY ), AbsoluteScreenPixelSize( nWidth, nHeight ) );

    if( ! ( nFlags & ( SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y ) ) )
    {
        if( bDefaultPosition_ )
        {
            maGeometry.setSize(Size(aPosSize.GetSize()));
            Center();
        }
        else
            SetSize( Size( nWidth, nHeight ) );
    }
    else
        SetPosSize( aPosSize );

    bDefaultPosition_ = False;
}

void X11SalFrame::SetAlwaysOnTop( bool bOnTop )
{
    if( ! IsOverrideRedirect() )
    {
        bAlwaysOnTop_ = bOnTop;
        pDisplay_->getWMAdaptor()->enableAlwaysOnTop( this, bOnTop );
    }
}

constexpr auto FRAMESTATE_MASK_MAXIMIZED_GEOMETRY =
     vcl::WindowDataMask::MaximizedX     | vcl::WindowDataMask::MaximizedY |
     vcl::WindowDataMask::MaximizedWidth | vcl::WindowDataMask::MaximizedHeight;

void X11SalFrame::SetWindowState( const vcl::WindowData *pState )
{
    if (pState == nullptr)
        return;

    // Request for position or size change
    if (pState->mask() & vcl::WindowDataMask::PosSize)
    {
        /* #i44325#
         * if maximized, set restore size and guess maximized size from last time
         * in state change below maximize window
         */
        if( ! IsChildWindow() &&
            (pState->mask() & vcl::WindowDataMask::PosSizeState) == vcl::WindowDataMask::PosSizeState &&
            (pState->state() & vcl::WindowState::Maximized) &&
            (pState->mask() & FRAMESTATE_MASK_MAXIMIZED_GEOMETRY) == FRAMESTATE_MASK_MAXIMIZED_GEOMETRY
            )
        {
            XSizeHints* pHints = XAllocSizeHints();
            tools::Long nSupplied = 0;
            XGetWMNormalHints( GetXDisplay(),
                               GetShellWindow(),
                               pHints,
                               &nSupplied );
            pHints->flags |= PPosition | PWinGravity;
            pHints->x = pState->x();
            pHints->y = pState->y();
            pHints->win_gravity = pDisplay_->getWMAdaptor()->getPositionWinGravity();
            XSetWMNormalHints(GetXDisplay(), GetShellWindow(), pHints);
            XFree( pHints );

            XMoveResizeWindow(GetXDisplay(), GetShellWindow(), pState->x(), pState->y(),
                              pState->width(), pState->height());
            // guess maximized geometry from last time
            maGeometry.setPos({ pState->GetMaximizedX(), pState->GetMaximizedY() });
            maGeometry.setSize({ static_cast<tools::Long>(pState->GetMaximizedWidth()), static_cast<tools::Long>(pState->GetMaximizedHeight()) });
            cairo_xlib_surface_set_size(mpSurface,  pState->GetMaximizedWidth(), pState->GetMaximizedHeight());
            updateScreenNumber();
        }
        else
        {
            bool bDoAdjust = false;
            AbsoluteScreenPixelRectangle aPosSize;
            // initialize with current geometry
            if ((pState->mask() & vcl::WindowDataMask::PosSize) != vcl::WindowDataMask::PosSize)
                GetPosSize(aPosSize);

            sal_uInt16 nPosFlags = 0;

            // change requested properties
            if (pState->mask() & vcl::WindowDataMask::X)
            {
                aPosSize.SetPosX(pState->x() - (mpParent ? mpParent->maGeometry.x() : 0));
                nPosFlags |= SAL_FRAME_POSSIZE_X;
            }
            if (pState->mask() & vcl::WindowDataMask::Y)
            {
                aPosSize.SetPosY(pState->y() - (mpParent ? mpParent->maGeometry.y() : 0));
                nPosFlags |= SAL_FRAME_POSSIZE_Y;
            }
            if (pState->mask() & vcl::WindowDataMask::Width)
            {
                tools::Long nWidth = pState->width() > 0 ? pState->width()  - 1 : 0;
                aPosSize.setWidth (nWidth);
                bDoAdjust = true;
            }
            if (pState->mask() & vcl::WindowDataMask::Height)
            {
                int nHeight = pState->height() > 0 ? pState->height() - 1 : 0;
                aPosSize.setHeight (nHeight);
                bDoAdjust = true;
            }

            const AbsoluteScreenPixelSize& aScreenSize = pDisplay_->getDataForScreen( m_nXScreen ).m_aSize;

            if( bDoAdjust && aPosSize.GetWidth() <= aScreenSize.Width()
                && aPosSize.GetHeight() <= aScreenSize.Height() )
            {
                SalFrameGeometry aGeom = maGeometry;

                if( ! (nStyle_ & ( SalFrameStyleFlags::FLOAT | SalFrameStyleFlags::PLUG ) ) &&
                    mpParent && aGeom.leftDecoration() == 0 && aGeom.topDecoration() == 0)
                {
                    aGeom = mpParent->maGeometry;
                    if (aGeom.leftDecoration() == 0 && aGeom.topDecoration() == 0)
                        aGeom.setDecorations(5, 20, 5, 5);
                }

                auto nRight = aPosSize.Right() + (mpParent ? mpParent->maGeometry.x() : 0);
                auto nBottom = aPosSize.Bottom() + (mpParent ? mpParent->maGeometry.y() : 0);
                auto nLeft = aPosSize.Left() + (mpParent ? mpParent->maGeometry.x() : 0);
                auto nTop = aPosSize.Top() + (mpParent ? mpParent->maGeometry.y() : 0);

                // adjust position so that frame fits onto screen
                if( nRight+static_cast<tools::Long>(aGeom.rightDecoration()) > aScreenSize.Width()-1 )
                    aPosSize.Move( aScreenSize.Width() - nRight - static_cast<tools::Long>(aGeom.rightDecoration()), 0 );
                if( nBottom+static_cast<tools::Long>(aGeom.bottomDecoration()) > aScreenSize.Height()-1 )
                    aPosSize.Move( 0, aScreenSize.Height() - nBottom - static_cast<tools::Long>(aGeom.bottomDecoration()) );
                if( nLeft < static_cast<tools::Long>(aGeom.leftDecoration()) )
                    aPosSize.Move( static_cast<tools::Long>(aGeom.leftDecoration()) - nLeft, 0 );
                if( nTop < static_cast<tools::Long>(aGeom.topDecoration()) )
                    aPosSize.Move( 0, static_cast<tools::Long>(aGeom.topDecoration()) - nTop );
            }

            SetPosSize(aPosSize.getX(), aPosSize.getY(),
                       aPosSize.GetWidth(), aPosSize.GetHeight(),
                       SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT |
                       nPosFlags);
        }
    }

    // request for status change
    if (!(pState->mask() & vcl::WindowDataMask::State))
        return;

    if (pState->state() & vcl::WindowState::Maximized)
    {
        nShowState_ = X11ShowState::Normal;
        if( ! (pState->state() & (vcl::WindowState::MaximizedHorz|vcl::WindowState::MaximizedVert) ) )
            Maximize();
        else
        {
            bool bHorz(pState->state() & vcl::WindowState::MaximizedHorz);
            bool bVert(pState->state() & vcl::WindowState::MaximizedVert);
            GetDisplay()->getWMAdaptor()->maximizeFrame( this, bHorz, bVert );
        }
        maRestorePosSize = AbsoluteScreenPixelRectangle(pState->posSize());
    }
    else if( mbMaximizedHorz || mbMaximizedVert )
        GetDisplay()->getWMAdaptor()->maximizeFrame( this, false, false );

    if (pState->state() & vcl::WindowState::Minimized)
    {
        if (nShowState_ == X11ShowState::Unknown)
            nShowState_ = X11ShowState::Normal;
        Minimize();
    }
    if (pState->state() & vcl::WindowState::Normal)
    {
        if (nShowState_ != X11ShowState::Normal)
            Restore();
    }
}

bool X11SalFrame::GetWindowState( vcl::WindowData* pState )
{
    if( X11ShowState::Minimized == nShowState_ )
        pState->setState(vcl::WindowState::Minimized);
    else
        pState->setState(vcl::WindowState::Normal);

    AbsoluteScreenPixelRectangle aPosSize;
    if( maRestorePosSize.IsEmpty() )
        GetPosSize( aPosSize );
    else
        aPosSize = maRestorePosSize;

    if( mbMaximizedHorz )
        pState->rState() |= vcl::WindowState::MaximizedHorz;
    if( mbMaximizedVert )
        pState->rState() |= vcl::WindowState::MaximizedVert;

    pState->setPosSize(tools::Rectangle(aPosSize));
    pState->setMask(vcl::WindowDataMask::PosSizeState);

    if (! maRestorePosSize.IsEmpty() )
    {
        GetPosSize( aPosSize );
        pState->rState() |= vcl::WindowState::Maximized;
        pState->SetMaximizedX(aPosSize.Left());
        pState->SetMaximizedY(aPosSize.Top());
        pState->SetMaximizedWidth(aPosSize.GetWidth());
        pState->SetMaximizedHeight(aPosSize.GetHeight());
        pState->rMask() |= FRAMESTATE_MASK_MAXIMIZED_GEOMETRY;
    }

    return true;
}

void X11SalFrame::SetMenu( SalMenu* )
{
}

void X11SalFrame::GetPosSize( AbsoluteScreenPixelRectangle &rPosSize )
{
    if( maGeometry.width() < 1 || maGeometry.height() < 1 )
    {
        const AbsoluteScreenPixelSize& aScreenSize = pDisplay_->getDataForScreen( m_nXScreen ).m_aSize;
        tools::Long w = aScreenSize.Width()  - maGeometry.leftDecoration() - maGeometry.rightDecoration();
        tools::Long h = aScreenSize.Height() - maGeometry.topDecoration() - maGeometry.bottomDecoration();

        rPosSize = AbsoluteScreenPixelRectangle( AbsoluteScreenPixelPoint( maGeometry.x(), maGeometry.y() ), AbsoluteScreenPixelSize( w, h ) );
    }
    else
        rPosSize = AbsoluteScreenPixelRectangle( AbsoluteScreenPixelPoint( maGeometry.x(), maGeometry.y() ),
                              AbsoluteScreenPixelSize( maGeometry.width(), maGeometry.height() ) );
}

void X11SalFrame::SetSize( const Size &rSize )
{
    if( rSize.IsEmpty() )
        return;

    if( ! ( nStyle_ & SalFrameStyleFlags::SIZEABLE )
        && ! IsChildWindow()
        && ( nStyle_ & (SalFrameStyleFlags::FLOAT|SalFrameStyleFlags::OWNERDRAWDECORATION) ) != SalFrameStyleFlags::FLOAT )
    {
        XSizeHints* pHints = XAllocSizeHints();
        tools::Long nSupplied = 0;
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
        if( nStyle_ & SalFrameStyleFlags::PLUG )
            XMoveResizeWindow( GetXDisplay(), GetWindow(), 0, 0, rSize.Width(), rSize.Height() );
        else
            XResizeWindow( GetXDisplay(), GetWindow(), rSize.Width(), rSize.Height() );
    }

    cairo_xlib_surface_set_size(mpSurface, rSize.Width(), rSize.Height());
    maGeometry.setSize(rSize);

    // allow the external status window to reposition
    if (mbInputFocus && mpInputContext != nullptr)
        mpInputContext->SetICFocus ( this );
}

void X11SalFrame::SetPosSize( const AbsoluteScreenPixelRectangle &rPosSize )
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
        if( AllSettings::GetLayoutRTL() )
            values.x = mpParent->maGeometry.width()-values.width-1-values.x;

        ::Window aChild;
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
    if( values.x != maGeometry.x() || values.y != maGeometry.y() )
        bMoved = true;
    if( values.width != static_cast<int>(maGeometry.width()) || values.height != static_cast<int>(maGeometry.height()) )
        bSized = true;

    // do not set WMNormalHints for...
    if(
        // child windows
        ! IsChildWindow()
        // popups (menu, help window, etc.)
        &&  (nStyle_ & (SalFrameStyleFlags::FLOAT|SalFrameStyleFlags::OWNERDRAWDECORATION) ) != SalFrameStyleFlags::FLOAT
        // shown, sizeable windows
        && ( nShowState_ == X11ShowState::Unknown ||
             nShowState_ == X11ShowState::Hidden ||
             ! ( nStyle_ & SalFrameStyleFlags::SIZEABLE )
             )
        )
    {
        XSizeHints* pHints = XAllocSizeHints();
        tools::Long nSupplied = 0;
        XGetWMNormalHints( GetXDisplay(),
                           GetShellWindow(),
                           pHints,
                           &nSupplied
                           );
        if( ! ( nStyle_ & SalFrameStyleFlags::SIZEABLE ) )
        {
            pHints->min_width   = rPosSize.GetWidth();
            pHints->min_height  = rPosSize.GetHeight();
            pHints->max_width   = rPosSize.GetWidth();
            pHints->max_height  = rPosSize.GetHeight();
            pHints->flags |= PMinSize | PMaxSize;
        }
        if( nShowState_ == X11ShowState::Unknown || nShowState_ == X11ShowState::Hidden )
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
        if( nStyle_ & SalFrameStyleFlags::PLUG )
            XMoveResizeWindow( GetXDisplay(), GetWindow(), 0, 0, values.width, values.height );
        else
            XMoveResizeWindow( GetXDisplay(), GetWindow(), values.x, values.y, values.width, values.height );
    }

    cairo_xlib_surface_set_size(mpSurface, values.width, values.height);
    maGeometry.setPosSize({ values.x, values.y }, { values.width, values.height });
    if( IsSysChildWindow() && mpParent )
        // translate back to root coordinates
        maGeometry.move(mpParent->maGeometry.x(), mpParent->maGeometry.y());

    updateScreenNumber();
    if( bSized && ! bMoved )
        CallCallback( SalEvent::Resize, nullptr );
    else if( bMoved && ! bSized )
        CallCallback( SalEvent::Move, nullptr );
    else
        CallCallback( SalEvent::MoveResize, nullptr );

    // allow the external status window to reposition
    if (mbInputFocus && mpInputContext != nullptr)
        mpInputContext->SetICFocus ( this );
}

void X11SalFrame::Minimize()
{
    if( IsSysChildWindow() )
        return;

    if( X11ShowState::Unknown == nShowState_ || X11ShowState::Hidden == nShowState_ )
    {
        SAL_WARN( "vcl", "X11SalFrame::Minimize on withdrawn window" );
        return;
    }

    if( XIconifyWindow( GetXDisplay(),
                        GetShellWindow(),
                        pDisplay_->GetDefaultXScreen().getXScreen() ) )
        nShowState_ = X11ShowState::Minimized;
}

void X11SalFrame::Maximize()
{
    if( IsSysChildWindow() )
        return;

    if( X11ShowState::Minimized == nShowState_ )
    {
        GetDisplay()->getWMAdaptor()->frameIsMapping( this );
        XMapWindow( GetXDisplay(), GetShellWindow() );
        nShowState_ = X11ShowState::Normal;
    }

    pDisplay_->getWMAdaptor()->maximizeFrame( this );
}

void X11SalFrame::Restore()
{
    if( IsSysChildWindow() )
        return;

    if( X11ShowState::Unknown == nShowState_ || X11ShowState::Hidden == nShowState_ )
    {
        SAL_INFO( "vcl", "X11SalFrame::Restore on withdrawn window" );
        return;
    }

    if( X11ShowState::Minimized == nShowState_ )
    {
        GetDisplay()->getWMAdaptor()->frameIsMapping( this );
        XMapWindow( GetXDisplay(), GetShellWindow() );
        nShowState_ = X11ShowState::Normal;
    }

    pDisplay_->getWMAdaptor()->maximizeFrame( this, false, false );
}

void X11SalFrame::SetScreenNumber( unsigned int nNewScreen )
{
    if( nNewScreen == maGeometry.screen() )
        return;

    if( GetDisplay()->IsXinerama() && GetDisplay()->GetXineramaScreens().size() > 1 )
    {
        if( nNewScreen >= GetDisplay()->GetXineramaScreens().size() )
            return;

        tools::Rectangle aOldScreenRect( GetDisplay()->GetXineramaScreens()[maGeometry.screen()] );
        tools::Rectangle aNewScreenRect( GetDisplay()->GetXineramaScreens()[nNewScreen] );
        bool bVisible = bMapped_;
        if( bVisible )
            Show( false );
        maGeometry.setX(aNewScreenRect.Left() + (maGeometry.x() - aOldScreenRect.Left()));
        maGeometry.setY(aNewScreenRect.Top() + (maGeometry.y() - aOldScreenRect.Top()));
        createNewWindow( None, m_nXScreen );
        if( bVisible )
            Show( true );
        maGeometry.setScreen(nNewScreen);
    }
    else if( nNewScreen < GetDisplay()->GetXScreenCount() )
    {
        bool bVisible = bMapped_;
        if( bVisible )
            Show( false );
        createNewWindow( None, SalX11Screen( nNewScreen ) );
        if( bVisible )
            Show( true );
        maGeometry.setScreen(nNewScreen);
    }
}

void X11SalFrame::SetApplicationID( const OUString &rWMClass )
{
    if( rWMClass != m_sWMClass && ! IsChildWindow() )
    {
        m_sWMClass = rWMClass;
        updateWMClass();
        for (auto const& child : maChildren)
            child->SetApplicationID(rWMClass);
    }
}

void X11SalFrame::updateWMClass()
{
    XClassHint* pClass = XAllocClassHint();
    OString aResName = SalGenericSystem::getFrameResName();
    pClass->res_name  = const_cast<char*>(aResName.getStr());

    OString aResClass = OUStringToOString(m_sWMClass, RTL_TEXTENCODING_ASCII_US);
    const char *pResClass = !aResClass.isEmpty() ? aResClass.getStr() :
                            SalGenericSystem::getFrameClassName();

    pClass->res_class = const_cast<char*>(pResClass);
    XSetClassHint( GetXDisplay(), GetShellWindow(), pClass );
    XFree( pClass );
}

void X11SalFrame::ShowFullScreen( bool bFullScreen, sal_Int32 nScreen )
{
    if( GetDisplay()->IsXinerama() && GetDisplay()->GetXineramaScreens().size() > 1 )
    {
        if( mbFullScreen == bFullScreen )
            return;
        if( bFullScreen )
        {
            maRestorePosSize = AbsoluteScreenPixelRectangle(maGeometry.posSize());
            AbsoluteScreenPixelRectangle aRect;
            if( nScreen < 0 || o3tl::make_unsigned(nScreen) >= GetDisplay()->GetXineramaScreens().size() )
                aRect = AbsoluteScreenPixelRectangle( AbsoluteScreenPixelPoint(0,0), GetDisplay()->GetScreenSize( m_nXScreen ) );
            else
                aRect = GetDisplay()->GetXineramaScreens()[nScreen];
            m_bIsPartialFullScreen = true;
            bool bVisible = bMapped_;
            if( bVisible )
                Show( false );
            maGeometry.setPosSize(tools::Rectangle(aRect));
            mbMaximizedHorz = mbMaximizedVert = false;
            mbFullScreen = true;
            createNewWindow( None, m_nXScreen );
            if( GetDisplay()->getWMAdaptor()->isLegacyPartialFullscreen() )
                GetDisplay()->getWMAdaptor()->enableAlwaysOnTop( this, true );
            else
                GetDisplay()->getWMAdaptor()->showFullScreen( this, true );
            if( bVisible )
                Show(true);

        }
        else
        {
            mbFullScreen = false;
            m_bIsPartialFullScreen = false;
            bool bVisible = bMapped_;
            AbsoluteScreenPixelRectangle aRect = maRestorePosSize;
            maRestorePosSize = AbsoluteScreenPixelRectangle();
            if( bVisible )
                Show( false );
            createNewWindow( None, m_nXScreen );
            if( !aRect.IsEmpty() )
                SetPosSize( aRect.Left(), aRect.Top(), aRect.GetWidth(), aRect.GetHeight(),
                            SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y |
                            SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
            if( bVisible )
                Show( true );
        }
    }
    else
    {
        if( nScreen < 0 || o3tl::make_unsigned(nScreen) >= GetDisplay()->GetXScreenCount() )
            nScreen = m_nXScreen.getXScreen();
        if( nScreen != static_cast<int>(m_nXScreen.getXScreen()) )
        {
            bool bVisible = bMapped_;
            if( mbFullScreen )
                pDisplay_->getWMAdaptor()->showFullScreen( this, false );
            if( bVisible )
                Show( false );
            createNewWindow( None, SalX11Screen( nScreen ) );
            if( mbFullScreen )
                pDisplay_->getWMAdaptor()->showFullScreen( this, true );
            if( bVisible )
                Show( true );
        }
        if( mbFullScreen == bFullScreen )
            return;

        pDisplay_->getWMAdaptor()->showFullScreen( this, bFullScreen );
    }
}

void X11SalFrame::StartPresentation( bool bStart )
{
    maSessionManagerInhibitor.inhibit( bStart,
                                    u"presentation",
                                    APPLICATION_INHIBIT_IDLE,
                                    mhWindow,
                                    GetXDisplay() );

    if( ! bStart && hPresentationWindow != None )
        doReparentPresentationDialogues( GetDisplay() );
    hPresentationWindow = (bStart && IsOverrideRedirect() ) ? GetWindow() : None;

    if( bStart && hPresentationWindow )
    {
        /*  #i10559# workaround for WindowMaker: try to restore
         *  current focus after presentation window is gone
         */
        int revert_to = 0;
        XGetInputFocus( GetXDisplay(), &hPresFocusWindow, &revert_to );
    }
}

// Pointer

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

void X11SalFrame::SetPointerPos(tools::Long nX, tools::Long nY)
{
    /* when the application tries to center the mouse in the dialog the
     * window isn't mapped already. So use coordinates relative to the root window.
     */
    unsigned int nWindowLeft = maGeometry.x() + nX;
    unsigned int nWindowTop  = maGeometry.y() + nY;

    XWarpPointer( GetXDisplay(), None, pDisplay_->GetRootWindow( pDisplay_->GetDefaultXScreen() ),
                  0, 0, 0, 0, nWindowLeft, nWindowTop);
}

// delay handling of extended text input
#if !defined(__synchronous_extinput__)
void
X11SalFrame::HandleExtTextEvent (XClientMessageEvent const *pEvent)
{
    #if SAL_TYPES_SIZEOFLONG > 4
    void* pExtTextEvent = reinterpret_cast<void*>(  (pEvent->data.l[0] & 0xffffffff)
                                                  | (pEvent->data.l[1] << 32) );
    #else
    void* pExtTextEvent = reinterpret_cast<void*>(pEvent->data.l[0]);
    #endif
    SalEvent nExtTextEventType = SalEvent(pEvent->data.l[2]);

    CallCallback(nExtTextEventType, pExtTextEvent);

    switch (nExtTextEventType)
    {
        case SalEvent::EndExtTextInput:
            break;

        case SalEvent::ExtTextInput:
            break;

        default:
            SAL_WARN("vcl.window",
                    "X11SalFrame::HandleExtTextEvent: invalid extended input.");
    }
}
#endif /* defined(__synchronous_extinput__) */

// PostEvent

bool X11SalFrame::PostEvent(std::unique_ptr<ImplSVEvent> pData)
{
    GetDisplay()->SendInternalEvent( this, pData.release() );
    return true;
}

// Title

void X11SalFrame::SetTitle( const OUString& rTitle )
{
    if( ! ( IsChildWindow() || (nStyle_ & SalFrameStyleFlags::FLOAT ) ) )
    {
        m_aTitle = rTitle;
        GetDisplay()->getWMAdaptor()->setWMName( this, rTitle );
    }
}

void X11SalFrame::Flush()
{
    if( pGraphics_ )
        pGraphics_->Flush();
    XFlush( GetDisplay()->GetDisplay() );
}

// Keyboard

void X11SalFrame::SetInputContext( SalInputContext* pContext )
{
    if (pContext == nullptr)
        return;

    // 1. We should create an input context for this frame
    //    only when InputContextFlags::Text is set.

    if (!(pContext->mnOptions & InputContextFlags::Text))
    {
        if( mpInputContext )
            mpInputContext->Unmap();
        return;
    }

    // 2. We should use on-the-spot inputstyle
    //    only when InputContextFlags::ExtTExt is set.

    if (mpInputContext == nullptr)
    {
        mpInputContext.reset( new SalI18N_InputContext( this ) );
        if (mpInputContext->UseContext())
        {
            mpInputContext->ExtendEventMask( GetShellWindow() );
            if (mbInputFocus)
                mpInputContext->SetICFocus( this );
        }
    }
    else
        mpInputContext->Map( this );
}

void X11SalFrame::EndExtTextInput( EndExtTextInputFlags )
{
    if (mpInputContext != nullptr)
          mpInputContext->EndExtTextInput();
}

OUString X11SalFrame::GetKeyName( sal_uInt16 nKeyCode )
{
    return GetDisplay()->GetKeyName( nKeyCode );
}

bool X11SalFrame::MapUnicodeToKeyCode( sal_Unicode , LanguageType , vcl::KeyCode& )
{
    // not supported yet
    return false;
}

LanguageType X11SalFrame::GetInputLanguage()
{
    // could be improved by checking unicode ranges of the last input
    return LANGUAGE_DONTKNOW;
}

// Settings

void X11SalFrame::UpdateSettings( AllSettings& rSettings )
{
    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    aStyleSettings.SetCursorBlinkTime( 500 );
    aStyleSettings.SetMenuBarTextColor( aStyleSettings.GetPersonaMenuBarTextColor().value_or( COL_BLACK ) );
    rSettings.SetStyleSettings( aStyleSettings );
}

void X11SalFrame::CaptureMouse( bool bCapture )
{
    nCaptured_ = pDisplay_->CaptureMouse( bCapture ? this : nullptr );
}

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

void X11SalFrame::createNewWindow( ::Window aNewParent, SalX11Screen nXScreen )
{
    bool bWasVisible = bMapped_;
    if( bWasVisible )
        Show( false );

    if( nXScreen.getXScreen() >= GetDisplay()->GetXScreenCount() )
        nXScreen = m_nXScreen;

    SystemParentData aParentData;
    aParentData.nSize = sizeof(SystemParentData);
    aParentData.aWindow = aNewParent;
    aParentData.bXEmbedSupport = (aNewParent != None && m_bXEmbed); // caution: this is guesswork
    if( aNewParent == None )
    {
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
        mpInputContext->UnsetICFocus();
        mpInputContext->Unmap();
    }
    if( GetWindow() == hPresentationWindow )
    {
        hPresentationWindow = None;
        doReparentPresentationDialogues( GetDisplay() );
    }
    if (mpSurface)
    {
        cairo_surface_destroy(mpSurface);
        mpSurface = nullptr;
    }
    XDestroyWindow( GetXDisplay(), mhWindow );
    mhWindow = None;

    // now init with new parent again
    if ( aParentData.aWindow != None )
        Init( nStyle_ | SalFrameStyleFlags::PLUG, nXScreen, &aParentData );
    else
        Init( nStyle_ & ~SalFrameStyleFlags::PLUG, nXScreen, nullptr, true );

    // update graphics if necessary
    updateGraphics(false);

    if( ! m_aTitle.isEmpty() )
        SetTitle( m_aTitle );

    if( mpParent )
    {
        if( mpParent->m_nXScreen != m_nXScreen )
            SetParent( nullptr );
        else
            pDisplay_->getWMAdaptor()->changeReferenceFrame( this, mpParent );
    }

    if( bWasVisible )
        Show( true );

    std::list< X11SalFrame* > aChildren = maChildren;
    for (auto const& child : aChildren)
        child->createNewWindow( None, m_nXScreen );

    // FIXME: SalObjects
}

void X11SalFrame::SetPluginParent( SystemParentData* pNewParent )
{
    if( pNewParent->nSize >= sizeof(SystemParentData) )
        m_bXEmbed = pNewParent->aWindow != None && pNewParent->bXEmbedSupport;

    createNewWindow(pNewParent->aWindow);
}

// Sound
void X11SalFrame::Beep()
{
    GetDisplay()->Beep();
}

// Event Handling

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
    // except macOS
    if( state & Mod3Mask )
        nCode |= KEY_MOD3;

    return nCode;
}

SalFrame::SalPointerState X11SalFrame::GetPointerState()
{
    SalPointerState aState;
    ::Window aRoot, aChild;
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

KeyIndicatorState X11SalFrame::GetIndicatorState()
{
    return vcl_sal::getSalDisplay(GetGenericUnixSalData())->GetIndicatorState();
}

void X11SalFrame::SimulateKeyPress( sal_uInt16 nKeyCode )
{
    vcl_sal::getSalDisplay(GetGenericUnixSalData())->SimulateKeyPress(nKeyCode);
}

namespace
{
struct CompressWheelEventsData
{
    XEvent* firstEvent;
    bool ignore;
    int count; // number of compressed events
};

Bool compressWheelEvents( Display*, XEvent* event, XPointer p )
{
    CompressWheelEventsData* data = reinterpret_cast< CompressWheelEventsData* >( p );
    if( data->ignore )
        return False; // we're already after the events to compress
    if( event->type == ButtonPress || event->type == ButtonRelease )
    {
        const unsigned int mask = Button1Mask << ( event->xbutton.button - Button1 );
        if( event->xbutton.button == data->firstEvent->xbutton.button
            && event->xbutton.window == data->firstEvent->xbutton.window
            && event->xbutton.x == data->firstEvent->xbutton.x
            && event->xbutton.y == data->firstEvent->xbutton.y
            && ( event->xbutton.state | mask ) == ( data->firstEvent->xbutton.state | mask ))
        {
            // Count if it's another press (i.e. wheel start event).
            if( event->type == ButtonPress )
                ++data->count;
            return True; // And remove the event from the queue.
        }
    }
    // Non-matching event, skip certain events that cannot possibly affect input processing,
    // but otherwise ignore all further events.
    switch( event->type )
    {
        case Expose:
        case NoExpose:
            break;
        default:
            data->ignore = true;
            break;
    }
    return False;
}

} // namespace

bool X11SalFrame::HandleMouseEvent( XEvent *pEvent )
{
    SalMouseEvent aMouseEvt;
    SalEvent            nEvent = SalEvent::NONE;
    bool                bClosePopups = false;

    if( nVisibleFloats && pEvent->type == EnterNotify )
        return false;

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
            return false;

        aMouseEvt.mnX       = pEvent->xcrossing.x;
        aMouseEvt.mnY       = pEvent->xcrossing.y;
        aMouseEvt.mnTime    = pEvent->xcrossing.time;
        aMouseEvt.mnCode    = sal_GetCode( pEvent->xcrossing.state );
        aMouseEvt.mnButton  = 0;

        nEvent              = LeaveNotify == pEvent->type
                              ? SalEvent::MouseLeave
                              : SalEvent::MouseMove;
    }
    else if( pEvent->type == MotionNotify )
    {
        aMouseEvt.mnX       = pEvent->xmotion.x;
        aMouseEvt.mnY       = pEvent->xmotion.y;
        aMouseEvt.mnTime    = pEvent->xmotion.time;
        aMouseEvt.mnCode    = sal_GetCode( pEvent->xmotion.state );

        aMouseEvt.mnButton  = 0;

        nEvent              = SalEvent::MouseMove;
        if( nVisibleFloats > 0 && mpParent )
        {
            Cursor aCursor = mpParent->GetCursor();
            if( pEvent->xmotion.x >= 0 && pEvent->xmotion.x < static_cast<int>(maGeometry.width()) &&
                pEvent->xmotion.y >= 0 && pEvent->xmotion.y < static_cast<int>(maGeometry.height()) )
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
            if( ! (nStyle_ & SalFrameStyleFlags::OWNERDRAWDECORATION) )
                XUngrabPointer( GetXDisplay(), CurrentTime );
        }
        else if( pEvent->type == ButtonPress )
        {
            // see if the user clicks outside all of the floats
            // if yes release the grab
            bool bInside = false;
            for (auto pSalFrame : GetDisplay()->getFrames() )
            {
                const X11SalFrame* pFrame = static_cast< const X11SalFrame* >( pSalFrame );
                if( pFrame->IsFloatGrabWindow()                                     &&
                    pFrame->bMapped_                                                &&
                    pEvent->xbutton.x_root >= pFrame->maGeometry.x()                             &&
                    pEvent->xbutton.x_root < pFrame->maGeometry.x() + static_cast<int>(pFrame->maGeometry.width()) &&
                    pEvent->xbutton.y_root >= pFrame->maGeometry.y()                             &&
                    pEvent->xbutton.y_root < pFrame->maGeometry.y() + static_cast<int>(pFrame->maGeometry.height()) )
                {
                    bInside = true;
                    break;
                }
            }
            if( ! bInside )
            {
                // need not take care of the XUngrabPointer in Show( false )
                // because XUngrabPointer does not produce errors if pointer
                // is not grabbed
                XUngrabPointer( GetXDisplay(), CurrentTime );
                bClosePopups = true;

                /*  #i15246# only close popups if pointer is outside all our frames
                 *  cannot use our own geometry data here because stacking
                 *  is unknown (the above case implicitly assumes
                 *  that floats are on top which should be true)
                 */
                ::Window aRoot, aChild;
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
                    for (auto pSalFrame : GetDisplay()->getFrames() )
                    {
                        const X11SalFrame* pFrame = static_cast< const X11SalFrame* >( pSalFrame );
                        if( ! pFrame->IsFloatGrabWindow()
                            && ( pFrame->GetWindow() == aChild ||
                                 pFrame->GetShellWindow() == aChild ||
                                 pFrame->GetStackingWindow() == aChild )
                            )
                        {
                            // #i63638# check that pointer is inside window, not
                            // only inside stacking window
                            if( root_x >= pFrame->maGeometry.x() && root_x < sal::static_int_cast< int >(pFrame->maGeometry.x()+pFrame->maGeometry.width()) &&
                                root_y >= pFrame->maGeometry.y() && root_y < sal::static_int_cast< int >(pFrame->maGeometry.x()+pFrame->maGeometry.height()) )
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
                ? SalEvent::MouseButtonDown
                : SalEvent::MouseButtonUp;
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
                return false;

            static sal_uLong        nLines = 0;
            if( ! nLines )
            {
                char* pEnv = getenv( "SAL_WHEELLINES" );
                nLines = pEnv ? atoi( pEnv ) : 3;
                if( nLines > 10 )
                    nLines = SAL_WHEELMOUSE_EVENT_PAGESCROLL;
            }

            // Compress consecutive wheel events (way too fine scrolling may cause lags if one scrolling steps takes long).
            CompressWheelEventsData data;
            data.firstEvent = pEvent;
            data.count = 1;
            XEvent dummy;
            do
            {
                data.ignore = false;
            } while( XCheckIfEvent( pEvent->xany.display, &dummy, compressWheelEvents, reinterpret_cast< XPointer >( &data )));

            SalWheelMouseEvent  aWheelEvt;
            aWheelEvt.mnTime        = pEvent->xbutton.time;
            aWheelEvt.mnX           = pEvent->xbutton.x;
            aWheelEvt.mnY           = pEvent->xbutton.y;
            aWheelEvt.mnDelta       = ( bIncrement ? 120 : -120 ) * data.count;
            aWheelEvt.mnNotchDelta  = bIncrement ? 1 : -1;
            aWheelEvt.mnScrollLines = nLines * data.count;
            aWheelEvt.mnCode        = sal_GetCode( pEvent->xbutton.state );
            aWheelEvt.mbHorz        = bHoriz;

            nEvent = SalEvent::WheelMouse;

            if( AllSettings::GetLayoutRTL() )
                aWheelEvt.mnX = nWidth_-1-aWheelEvt.mnX;
            return CallCallback( nEvent, &aWheelEvt );
        }
    }

    bool nRet = false;
    if( nEvent == SalEvent::MouseLeave
        || ( aMouseEvt.mnX <  nWidth_  && aMouseEvt.mnX >  -1 &&
             aMouseEvt.mnY <  nHeight_ && aMouseEvt.mnY >  -1 )
        || pDisplay_->MouseCaptured( this )
        )
    {
        if( AllSettings::GetLayoutRTL() )
            aMouseEvt.mnX = nWidth_-1-aMouseEvt.mnX;
        nRet = CallCallback( nEvent, &aMouseEvt );
    }

    if( bClosePopups )
    {
        /*  #108213# close popups after dispatching the event outside the popup;
         *  applications do weird things.
         */
        ImplSVData* pSVData = ImplGetSVData();
        if (pSVData->mpWinData->mpFirstFloat)
        {
            if (!(pSVData->mpWinData->mpFirstFloat->GetPopupModeFlags()
                  & FloatWinPopupFlags::NoAppFocusClose))
                pSVData->mpWinData->mpFirstFloat->EndPopupMode(FloatWinPopupEndFlags::Cancel
                                                               | FloatWinPopupEndFlags::CloseAll);
        }
    }

    return nRet;
}

namespace {

// F10 means either KEY_F10 or KEY_MENU, which has to be decided
// in the independent part.
struct KeyAlternate
{
    sal_uInt16          nKeyCode;
    sal_Unicode     nCharCode;
    KeyAlternate() : nKeyCode( 0 ), nCharCode( 0 ) {}
    KeyAlternate( sal_uInt16 nKey, sal_Unicode nChar = 0 ) : nKeyCode( nKey ), nCharCode( nChar ) {}
};

}

static KeyAlternate
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
    OUString& rSeq( GetGenericUnixSalData()->GetUnicodeCommand() );
    vcl::DeletionListener aDeleteWatch( this );

    if( !rSeq.isEmpty() )
        endUnicodeSequence();

    rSeq = "u";

    if( ! aDeleteWatch.isDeleted() )
    {
        ExtTextInputAttr nTextAttr = ExtTextInputAttr::Underline;
        SalExtTextInputEvent aEv;
        aEv.maText          = rSeq;
        aEv.mpTextAttr      = &nTextAttr;
        aEv.mnCursorPos     = 0;
        aEv.mnCursorFlags   = 0;

        CallCallback(SalEvent::ExtTextInput, static_cast<void*>(&aEv));
    }
}

bool X11SalFrame::appendUnicodeSequence( sal_Unicode c )
{
    bool bRet = false;
    OUString& rSeq( GetGenericUnixSalData()->GetUnicodeCommand() );
    if( !rSeq.isEmpty() )
    {
        // range check
        if( (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F') )
        {
            rSeq += OUStringChar(c);
            std::vector<ExtTextInputAttr> attribs( rSeq.getLength(), ExtTextInputAttr::Underline );

            SalExtTextInputEvent aEv;
            aEv.maText          = rSeq;
            aEv.mpTextAttr      = attribs.data();
            aEv.mnCursorPos     = 0;
            aEv.mnCursorFlags   = 0;

            CallCallback(SalEvent::ExtTextInput, static_cast<void*>(&aEv));
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
    OUString& rSeq( GetGenericUnixSalData()->GetUnicodeCommand() );

    vcl::DeletionListener aDeleteWatch( this );
    if( rSeq.getLength() > 1 && rSeq.getLength() < 6 )
    {
        // cut the "u"
        std::u16string_view aNumbers( rSeq.subView( 1 ) );
        sal_uInt32 nValue = o3tl::toUInt32(aNumbers, 16);
        if( nValue >= 32 )
        {
            ExtTextInputAttr nTextAttr = ExtTextInputAttr::Underline;
            SalExtTextInputEvent aEv;
            aEv.maText          = OUString( sal_Unicode(nValue) );
            aEv.mpTextAttr      = &nTextAttr;
            aEv.mnCursorPos     = 0;
            aEv.mnCursorFlags   = 0;
            CallCallback(SalEvent::ExtTextInput, static_cast<void*>(&aEv));
        }
    }
    bool bWasInput = !rSeq.isEmpty();
    rSeq.clear();
    if( bWasInput && ! aDeleteWatch.isDeleted() )
        CallCallback(SalEvent::EndExtTextInput, nullptr);
    return bWasInput;
}

bool X11SalFrame::HandleKeyEvent( XKeyEvent *pEvent )
{
    if( pEvent->type == KeyRelease )
    {
        // Ignore autorepeat keyrelease events. If there is a series of keypress+keyrelease+keypress events
        // generated by holding down a key, and if these are from autorepeat (keyrelease and the following keypress
        // have the same timestamp), drop the autorepeat keyrelease event. Not exactly sure why this is done
        // (possibly hiding differences between platforms, or just making it more sensible, because technically
        // the key has not been released at all).
        bool ignore = false;
        // Discard queued excessive autorepeat events.
        // If the user presses and holds down a key, the autorepeating keypress events
        // may overload LO (e.g. if the key is PageDown and the LO cannot keep up scrolling).
        // Reduce the load by simply discarding such excessive events (so for a KeyRelease event,
        // check if it's followed by matching KeyPress+KeyRelease pair(s) and discard those).
        // This shouldn't have any negative effects - unlike with normal (non-autorepeat
        // events), the user is unlikely to rely on the exact number of resulting actions
        // (since autorepeat generates keypress events rather quickly and it's hard to estimate
        // how many exactly) and the idea should be just keeping the key pressed until something
        // happens (in which case more events that just lag LO shouldn't make a difference).
        Display* dpy = pEvent->display;
        XKeyEvent previousRelease = *pEvent;
        while( XPending( dpy ))
        {
            XEvent nextEvent1;
            bool discard1 = false;
            XNextEvent( dpy, &nextEvent1 );
            if( nextEvent1.type == KeyPress && nextEvent1.xkey.time == previousRelease.time
                && !nextEvent1.xkey.send_event && nextEvent1.xkey.window == previousRelease.window
                && nextEvent1.xkey.state == previousRelease.state && nextEvent1.xkey.keycode == previousRelease.keycode )
            {   // This looks like another autorepeat keypress.
                ignore = true;
                if( XPending( dpy ))
                {
                    XEvent nextEvent2;
                    XNextEvent( dpy, &nextEvent2 );
                    if( nextEvent2.type == KeyRelease && nextEvent2.xkey.time <= ( previousRelease.time + 100 )
                        && !nextEvent2.xkey.send_event && nextEvent2.xkey.window == previousRelease.window
                        && nextEvent2.xkey.state == previousRelease.state && nextEvent2.xkey.keycode == previousRelease.keycode )
                    {   // And the matching keyrelease -> drop them both.
                        discard1 = true;
                        previousRelease = nextEvent2.xkey;
                        ignore = false; // There either will be another autorepeating keypress that'll lead to discarding
                                        // the pEvent keyrelease, it this discarding makes that keyrelease the last one.
                    }
                    else
                    {
                        XPutBackEvent( dpy, &nextEvent2 );
                        break;
                    }
                }
            }
            if( !discard1 )
            {   // Unrelated event, put back and stop compressing.
                XPutBackEvent( dpy, &nextEvent1 );
                break;
            }
        }
        if( ignore ) // This autorepeating keyrelease is followed by another keypress.
            return false;
    }

    KeySym          nKeySym;
    KeySym          nUnmodifiedKeySym;
    int             nLen = 2048;
    char            *pPrintable = static_cast<char*>(alloca( nLen ));

    // singlebyte code composed by input method, the new default
    if (mpInputContext != nullptr && mpInputContext->UseContext())
    {
        // returns a keysym as well as the pPrintable (in system encoding)
        // printable may be empty.
        Status nStatus;
        nKeySym = pDisplay_->GetKeySym( pEvent, pPrintable, &nLen,
                                        &nUnmodifiedKeySym,
                                        &nStatus, mpInputContext->GetContext() );
        if ( nStatus == XBufferOverflow )
        {
            // In case of overflow, XmbLookupString (called by GetKeySym)
            // returns required size
            // TODO : check if +1 is needed for 0 terminator
            nLen += 1;
            pPrintable = static_cast<char*>(alloca( nLen ));
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
    sal_uInt16  nKeyCode;
    sal_uInt16  nModCode = 0;
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
        ||  nKeySym == XK_Super_L   || nKeySym == XK_Super_R )
    {
        SalKeyModEvent aModEvt;
        aModEvt.mbDown = false; // auto-accelerator feature not supported here.
        aModEvt.mnModKeyCode = ModKeyFlags::NONE;
        if( pEvent->type == KeyPress && mnExtKeyMod == ModKeyFlags::NONE )
            mbSendExtKeyModChange = true;
        else if( pEvent->type == KeyRelease && mbSendExtKeyModChange )
        {
            aModEvt.mnModKeyCode = mnExtKeyMod;
            mnExtKeyMod = ModKeyFlags::NONE;
        }

        // pressing just the ctrl key leads to a keysym of XK_Control but
        // the event state does not contain ControlMask. In the release
        // event it's the other way round: it does contain the Control mask.
        // The modifier mode therefore has to be adapted manually.
        ModKeyFlags nExtModMask = ModKeyFlags::NONE;
        sal_uInt16 nModMask = 0;
        switch( nKeySym )
        {
            case XK_Control_L:
                nExtModMask = ModKeyFlags::LeftMod1;
                nModMask = KEY_MOD1;
                break;
            case XK_Control_R:
                nExtModMask = ModKeyFlags::RightMod1;
                nModMask = KEY_MOD1;
                break;
            case XK_Alt_L:
                nExtModMask = ModKeyFlags::LeftMod2;
                nModMask = KEY_MOD2;
                break;
            case XK_Alt_R:
                nExtModMask = ModKeyFlags::RightMod2;
                nModMask = KEY_MOD2;
                break;
            case XK_Shift_L:
                nExtModMask = ModKeyFlags::LeftShift;
                nModMask = KEY_SHIFT;
                break;
            case XK_Shift_R:
                nExtModMask = ModKeyFlags::RightShift;
                nModMask = KEY_SHIFT;
                break;
            // Map Meta/Super keys to MOD3 modifier on all Unix systems
            // except macOS
            case XK_Meta_L:
            case XK_Super_L:
                nExtModMask = ModKeyFlags::LeftMod3;
                nModMask = KEY_MOD3;
                break;
            case XK_Meta_R:
            case XK_Super_R:
                nExtModMask = ModKeyFlags::RightMod3;
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

        return CallCallback( SalEvent::KeyModChange, &aModEvt );
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
        return false;

    vcl::DeletionListener aDeleteWatch( this );

    if( nModCode == (KEY_SHIFT | KEY_MOD1) && pEvent->type == KeyPress )
    {
        sal_uInt16 nSeqKeyCode = pDisplay_->GetKeyCode( nUnmodifiedKeySym, &aDummy );
        if( nSeqKeyCode == KEY_U )
        {
            beginUnicodeSequence();
            return true;
        }
        else if( nSeqKeyCode >= KEY_0 && nSeqKeyCode <= KEY_9 )
        {
            if( appendUnicodeSequence( u'0' + sal_Unicode(nSeqKeyCode - KEY_0) ) )
                return true;
        }
        else if( nSeqKeyCode >= KEY_A && nSeqKeyCode <= KEY_F )
        {
            if( appendUnicodeSequence( u'a' + sal_Unicode(nSeqKeyCode - KEY_A) ) )
                return true;
        }
        else
            endUnicodeSequence();
    }

    if( aDeleteWatch.isDeleted() )
        return false;

    rtl_TextEncoding nEncoding = osl_getThreadTextEncoding();

    sal_Unicode *pBuffer;
    sal_Unicode *pString;
    sal_Size     nBufferSize = nLen * 2;
    sal_Size     nSize;
    pBuffer = static_cast<sal_Unicode*>(malloc( nBufferSize + 2 ));
    pBuffer[ 0 ] = 0;

    if (nKeyString != 0)
    {
        pString = &nKeyString;
        nSize = 1;
    }
    else if (nLen > 0 && nEncoding != RTL_TEXTENCODING_UNICODE)
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
                                pPrintable, nLen,
                                pBuffer, nBufferSize,
                                RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE |
                                RTL_TEXTTOUNICODE_FLAGS_INVALID_IGNORE,
                                &nConversionInfo, &nConvertedChars );

        // destroy converter
        rtl_destroyTextToUnicodeContext( aConverter, aContext );
        rtl_destroyTextToUnicodeConverter( aConverter );

        pString = pBuffer;
    }
    else if (nLen > 0 /* nEncoding == RTL_TEXTENCODING_UNICODE */)
    {
        pString = reinterpret_cast<sal_Unicode*>(pPrintable);
        nSize = nLen;
    }
    else
    {
        pString = pBuffer;
        nSize   = 0;
    }

    if (   mpInputContext != nullptr
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
        aKeyEvt.mnCharCode = pString[ 0 ];

        if( KeyRelease == pEvent->type )
        {
            CallCallback( SalEvent::KeyUp, &aKeyEvt );
        }
        else
        {
            if ( ! CallCallback(SalEvent::KeyInput, &aKeyEvt) )
            {
                // independent layer doesn't want to handle key-event, so check
                // whether the keycode may have an alternate meaning
                KeyAlternate aAlternate = GetAlternateKeyCode( nKeyCode );
                if ( aAlternate.nKeyCode != 0 )
                {
                    aKeyEvt.mnCode = aAlternate.nKeyCode | nModCode;
                    if( aAlternate.nCharCode )
                        aKeyEvt.mnCharCode = aAlternate.nCharCode;
                    CallCallback(SalEvent::KeyInput, &aKeyEvt);
                }
            }
        }
    }

      // update the spot location for PreeditPosition IME style

    if (! aDeleteWatch.isDeleted())
    {
        if (mpInputContext != nullptr && mpInputContext->UseContext())
            mpInputContext->UpdateSpotLocation();
    }

    free (pBuffer);
    return true;
}

bool X11SalFrame::HandleFocusEvent( XFocusChangeEvent const *pEvent )
{
    // ReflectionX in Windows mode changes focus while mouse is grabbed
    if( nVisibleFloats > 0 && GetDisplay()->getWMAdaptor()->getWindowManagerName() == "ReflectionX Windows" )
        return true;

    /*  ignore focusout resulting from keyboard grabs
     *  we do not grab it and are not interested when
     *  someone else does CDE e.g. does a XGrabKey on arrow keys
     *  handle focus events with mode NotifyWhileGrabbed
     *  because with CDE alt-tab focus changing we do not get
     *  normal focus events
     *  cast focus event to the input context, otherwise the
     *  status window does not follow the application frame
     */

    if ( mpInputContext != nullptr  )
    {
        if( FocusIn == pEvent->type )
            mpInputContext->SetICFocus( this );
    }

    if ( pEvent->mode == NotifyNormal || pEvent->mode == NotifyWhileGrabbed ||
         ( ( nStyle_ & SalFrameStyleFlags::PLUG ) && pEvent->window == GetShellWindow() )
         )
    {
        if( hPresentationWindow != None && hPresentationWindow != GetShellWindow() )
            return false;

        if( FocusIn == pEvent->type )
        {
            GetSalInstance()->updatePrinterUpdate();
            mbInputFocus = True;
            ImplSVData* pSVData = ImplGetSVData();

            bool nRet = CallCallback( SalEvent::GetFocus,  nullptr );
            if ((mpParent != nullptr && nStyle_ == SalFrameStyleFlags::NONE)
                && pSVData->mpWinData->mpFirstFloat)
            {
                FloatWinPopupFlags nMode = pSVData->mpWinData->mpFirstFloat->GetPopupModeFlags();
                pSVData->mpWinData->mpFirstFloat->SetPopupModeFlags(
                    nMode & ~FloatWinPopupFlags::NoAppFocusClose);
            }
            return nRet;
        }
        else
        {
            mbInputFocus = False;
            mbSendExtKeyModChange = false;
            mnExtKeyMod = ModKeyFlags::NONE;
            return CallCallback( SalEvent::LoseFocus, nullptr );
        }
    }

    return false;
}

bool X11SalFrame::HandleExposeEvent( XEvent const *pEvent )
{
    XRectangle  aRect = { 0, 0, 0, 0 };
    sal_uInt16  nCount = 0;

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
        aPresentationReparentList.empty() )
        // we are in fullscreen mode -> override redirect
         // focus is possibly lost, so reget it
         XSetInputFocus( GetXDisplay(), GetShellWindow(), RevertToNone, CurrentTime );

    // width and height are extents, so they are of by one for rectangle
    maPaintRegion.Union( tools::Rectangle( Point(aRect.x, aRect.y), Size(aRect.width+1, aRect.height+1) ) );

    if( nCount )
        // wait for last expose rectangle, do not wait for resize timer
        // if a completed graphics expose sequence is available
        return true;

    SalPaintEvent aPEvt( maPaintRegion.Left(), maPaintRegion.Top(), maPaintRegion.GetWidth(), maPaintRegion.GetHeight() );

    CallCallback( SalEvent::Paint, &aPEvt );
    maPaintRegion = tools::Rectangle();

    return true;
}

void X11SalFrame::RestackChildren( ::Window* pTopLevelWindows, int nTopLevelWindows )
{
    if( maChildren.empty() )
        return;

    int nWindow = nTopLevelWindows;
    while( nWindow-- )
        if( pTopLevelWindows[nWindow] == GetStackingWindow() )
            break;
    if( nWindow < 0 )
        return;

    for (auto const& child : maChildren)
    {
        if( child->bMapped_ )
        {
            int nChild = nWindow;
            while( nChild-- )
            {
                if( pTopLevelWindows[nChild] == child->GetStackingWindow() )
                {
                    // if a child is behind its parent, place it above the
                    // parent (for insane WMs like Dtwm and olwm)
                    XWindowChanges aCfg;
                    aCfg.sibling    = GetStackingWindow();
                    aCfg.stack_mode = Above;
                    XConfigureWindow( GetXDisplay(), child->GetStackingWindow(), CWSibling|CWStackMode, &aCfg );
                    break;
                }
            }
        }
    }
    for (auto const& child : maChildren)
    {
        child->RestackChildren( pTopLevelWindows, nTopLevelWindows );
    }
}

void X11SalFrame::RestackChildren()
{
    if( maChildren.empty() )
        return;

    ::Window aRoot, aParent, *pChildren = nullptr;
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

bool X11SalFrame::HandleSizeEvent( XConfigureEvent *pEvent )
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
        return true;
    }

    if( ( nStyle_ & SalFrameStyleFlags::PLUG ) && pEvent->window == GetShellWindow() )
    {
        // just update the children's positions
        RestackChildren();
        return true;
    }

    if( pEvent->window == GetForeignParent() )
    {
        XResizeWindow( GetXDisplay(),
                       GetWindow(),
                       pEvent->width,
                       pEvent->height );
        cairo_xlib_surface_set_size(mpSurface, pEvent->width, pEvent->height);
    }

    ::Window hDummy;
    XTranslateCoordinates( GetXDisplay(),
                           GetWindow(),
                           pDisplay_->GetRootWindow( pDisplay_->GetDefaultXScreen() ),
                           0, 0,
                           &pEvent->x, &pEvent->y,
                           &hDummy );

    if( pEvent->window == GetStackingWindow() )
    {
        if( maGeometry.x() != pEvent->x || maGeometry.y() != pEvent->y )
        {
            maGeometry.setPos({ pEvent->x, pEvent->y });
            CallCallback( SalEvent::Move, nullptr );
        }
        return true;
    }

    // check size hints in first time SalFrame::Show
    if( X11ShowState::Unknown == nShowState_ && bMapped_ )
        nShowState_ = X11ShowState::Normal;

    // Avoid a race condition where resizing this window to one size and shortly after that
    // to another size generates first size event with the old size and only after that
    // with the new size, temporarily making us think the old size is valid (bnc#674806).
    // So if there is another size event for this window pending, ignore this one.
    mPendingSizeEvent = false;
    XEvent dummy;
    XCheckIfEvent( GetXDisplay(), &dummy, size_event_predicate, reinterpret_cast< XPointer >( this ));
    if( mPendingSizeEvent )
        return true;

    nWidth_     = pEvent->width;
    nHeight_    = pEvent->height;

    bool bMoved = ( pEvent->x != maGeometry.x() || pEvent->y != maGeometry.y() );
    bool bSized = ( pEvent->width != static_cast<int>(maGeometry.width()) || pEvent->height != static_cast<int>(maGeometry.height()) );

    cairo_xlib_surface_set_size(mpSurface, pEvent->width, pEvent->height);
    maGeometry.setPosSize({ pEvent->x, pEvent->y }, { pEvent->width, pEvent->height });
    updateScreenNumber();

    // update children's position
    RestackChildren();

    if( bSized && ! bMoved )
        CallCallback( SalEvent::Resize, nullptr );
    else if( bMoved && ! bSized )
        CallCallback( SalEvent::Move, nullptr );
    else if( bMoved && bSized )
        CallCallback( SalEvent::MoveResize, nullptr );

    return true;
}

IMPL_LINK_NOARG(X11SalFrame, HandleAlwaysOnTopRaise, Timer *, void)
{
    if( bMapped_ )
        ToTop( SalFrameToTop::NONE );
}

bool X11SalFrame::HandleReparentEvent( XReparentEvent *pEvent )
{
    Display        *pDisplay   = pEvent->display;
    ::Window        hWM_Parent;
    ::Window        hRoot, *Children, hDummy;
    unsigned int    nChildren;

    static const char* pDisableStackingCheck = getenv( "SAL_DISABLE_STACKING_CHECK" );

    GetGenericUnixSalData()->ErrorTrapPush();

    /*
     *  don't rely on the new parent from the event.
     *  the event may be "out of date", that is the window manager
     *  window may not exist anymore. This can happen if someone
     *  shows a frame and hides it again quickly (not that it would
     *  be very sensible)
     */
    hWM_Parent = GetShellWindow();
    do
    {
        Children = nullptr;
        XQueryTree( pDisplay,
                    hWM_Parent,
                    &hRoot,
                    &hDummy,
                    &Children,
                    &nChildren );

        bool bError = GetGenericUnixSalData()->ErrorTrapPop( false );
        GetGenericUnixSalData()->ErrorTrapPush();

        if( bError )
        {
            hWM_Parent = GetShellWindow();
            break;
        }
         /* this sometimes happens if a Show(true) is
         *  immediately followed by Show(false) (which is braindead anyway)
         */
        if( hDummy == hWM_Parent )
            hDummy = hRoot;
        if( hDummy != hRoot )
            hWM_Parent = hDummy;
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
        XSelectInput( pDisplay, GetStackingWindow(), StructureNotifyMask );
    }

    if(     hWM_Parent == pDisplay_->GetRootWindow( pDisplay_->GetDefaultXScreen() )
            ||  hWM_Parent == GetForeignParent()
            ||  pEvent->parent == pDisplay_->GetRootWindow( pDisplay_->GetDefaultXScreen() )
            || ( nStyle_ & SalFrameStyleFlags::FLOAT ) )
    {
        // Reparenting before Destroy
        aPresentationReparentList.remove( GetStackingWindow() );
        mhStackingWindow = None;
        GetGenericUnixSalData()->ErrorTrapPop();
        return false;
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
        ::Window aChild;
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
    maGeometry.setLeftDecoration(nLeft > 0 ? nLeft-1 : 0);
    maGeometry.setTopDecoration(nTop  > 0 ? nTop-1  : 0);

    /*
     *  decorations are not symmetric,
     *  so need real geometries here
     *  (this will fail with virtual roots ?)
     */

    // reset error occurred
    GetGenericUnixSalData()->ErrorTrapPop();
    GetGenericUnixSalData()->ErrorTrapPush();

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
    bool bError = GetGenericUnixSalData()->ErrorTrapPop( false );
    GetGenericUnixSalData()->ErrorTrapPush();

    if( ! bError )
    {
        maGeometry.setRightDecoration(wp - w - maGeometry.leftDecoration());
        maGeometry.setBottomDecoration(hp - h - maGeometry.topDecoration());
        bResized = w != o3tl::make_unsigned(maGeometry.width()) ||
                   h != o3tl::make_unsigned(maGeometry.height());
        /*
         *  note: this works because hWM_Parent is direct child of root,
         *  not necessarily parent of GetShellWindow()
         */
        maGeometry.setPosSize({ xp + nLeft, yp + nTop }, { static_cast<tools::Long>(w), static_cast<tools::Long>(h) });
    }

    // limit width and height if we are too large: #47757
    // olwm and fvwm need this, it doesn't harm the rest

    // #i81311# do this only for sizable frames
    if( nStyle_ & SalFrameStyleFlags::SIZEABLE )
    {
        AbsoluteScreenPixelSize aScreenSize = GetDisplay()->GetScreenSize( m_nXScreen );
        int nScreenWidth  = aScreenSize.Width();
        int nScreenHeight = aScreenSize.Height();
        int nFrameWidth   = maGeometry.width() + maGeometry.leftDecoration() + maGeometry.rightDecoration();
        int nFrameHeight  = maGeometry.height() + maGeometry.topDecoration()  + maGeometry.bottomDecoration();

        if ((nFrameWidth > nScreenWidth) || (nFrameHeight > nScreenHeight))
        {
            Size aSize(maGeometry.width(), maGeometry.height());

            if (nFrameWidth  > nScreenWidth)
                aSize.setWidth( nScreenWidth  - maGeometry.rightDecoration() - maGeometry.leftDecoration() );
            if (nFrameHeight > nScreenHeight)
                aSize.setHeight( nScreenHeight - maGeometry.bottomDecoration() - maGeometry.topDecoration() );

            SetSize( aSize );
            bResized = false;
        }
    }
    if( bResized )
        CallCallback( SalEvent::Resize, nullptr );

    GetGenericUnixSalData()->ErrorTrapPop();

    return true;
}

bool X11SalFrame::HandleStateEvent( XPropertyEvent const *pEvent )
{
    Atom          actual_type;
    int           actual_format;
    unsigned long nitems, bytes_after;
    unsigned char *prop = nullptr;

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
        return false;

    DBG_ASSERT( actual_type == pEvent->atom
                && 32 == actual_format
                &&  2 == nitems
                &&  0 == bytes_after, "HandleStateEvent" );

    if( *reinterpret_cast<unsigned long*>(prop) == NormalState )
        nShowState_ = X11ShowState::Normal;
    else if( *reinterpret_cast<unsigned long*>(prop) == IconicState )
        nShowState_ = X11ShowState::Minimized;

    XFree( prop );
    return true;
}

bool X11SalFrame::HandleClientMessage( XClientMessageEvent *pEvent )
{
    const WMAdaptor& rWMAdaptor( *pDisplay_->getWMAdaptor() );

#if !defined(__synchronous_extinput__)
    if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::SAL_EXTTEXTEVENT ) )
    {
        HandleExtTextEvent (pEvent);
        return true;
    }
#endif
    else if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::SAL_QUITEVENT ) )
    {
        SAL_WARN( "vcl", "X11SalFrame::Dispatch Quit" );
        Close(); // ???
        return true;
    }
    else if( pEvent->message_type == rWMAdaptor.getAtom( WMAdaptor::WM_PROTOCOLS ) )
    {
        if( static_cast<Atom>(pEvent->data.l[0]) == rWMAdaptor.getAtom( WMAdaptor::NET_WM_PING ) )
            rWMAdaptor.answerPing( this, pEvent );
        else if( ! ( nStyle_ & SalFrameStyleFlags::PLUG )
              && ! (( nStyle_ & SalFrameStyleFlags::FLOAT ) && (nStyle_ & SalFrameStyleFlags::OWNERDRAWDECORATION))
             )
        {
            if( static_cast<Atom>(pEvent->data.l[0]) == rWMAdaptor.getAtom( WMAdaptor::WM_DELETE_WINDOW ) )
            {
                Close();
                return true;
            }
            else if( static_cast<Atom>(pEvent->data.l[0]) == rWMAdaptor.getAtom( WMAdaptor::WM_TAKE_FOCUS ) )
            {
                // do nothing, we set the input focus in ToTop() if necessary
#if OSL_DEBUG_LEVEL > 1
                SAL_INFO("vcl.window", "got WM_TAKE_FOCUS on "
                        << ((nStyle_ &
                                SalFrameStyleFlags::OWNERDRAWDECORATION) ?
                            "ownerdraw" :
                            "NON OWNERDRAW" )
                        << " window.");
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
    return false;
}

bool X11SalFrame::Dispatch( XEvent *pEvent )
{
    bool nRet = false;

    if( -1 == nCaptured_ )
    {
        CaptureMouse( true );
#ifdef DBG_UTIL
        if( -1 != nCaptured_ )
            pDisplay_->DbgPrintDisplayEvent("Captured", pEvent);
#endif
    }

    if( pEvent->xany.window == GetShellWindow() || pEvent->xany.window == GetWindow() )
    {
        switch( pEvent->type )
        {
            case KeyPress:
                nRet        = HandleKeyEvent( &pEvent->xkey );
                break;

            case KeyRelease:
                nRet = HandleKeyEvent( &pEvent->xkey );
            break;

            case ButtonPress:
                // if we lose the focus in presentation mode
                // there are good chances that we never get it back
                // since the WM ignores us
                 if( IsOverrideRedirect() )
                 {
                     XSetInputFocus( GetXDisplay(), GetShellWindow(),
                             RevertToNone, CurrentTime );
                 }
                [[fallthrough]];
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
                    if( nShowState_ == X11ShowState::Hidden )
                    {
                        /*
                         *  workaround for (at least) KWin 2.2.2
                         *  which will map windows that were once transient
                         *  even if they are withdrawn when the respective
                         *  document is mapped.
                         */
                        if( ! (nStyle_ & SalFrameStyleFlags::PLUG) )
                            XUnmapWindow( GetXDisplay(), GetShellWindow() );
                        break;
                    }
                    bMapped_   = true;
                    bViewable_ = true;
                    nRet = true;
                    if ( mpInputContext != nullptr )
                        mpInputContext->Map( this );
                    CallCallback( SalEvent::Resize, nullptr );

                    bool bSetFocus = m_bSetFocusOnMap;

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
                        for (auto const& child : maChildren)
                        {
                            if( child->mbTransientForRoot )
                                pDisplay_->getWMAdaptor()->changeReferenceFrame( child, this );
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
                    m_bSetFocusOnMap = false;
                }
                break;

            case UnmapNotify:
                if( pEvent->xunmap.window == GetShellWindow() )
                {
                    bMapped_   = false;
                    bViewable_ = false;
                    nRet = true;
                    if ( mpInputContext != nullptr )
                        mpInputContext->Unmap();
                    CallCallback( SalEvent::Resize, nullptr );
                }
                break;

            case ConfigureNotify:
                if( pEvent->xconfigure.window == GetShellWindow()
                    || pEvent->xconfigure.window == GetWindow() )
                    nRet = HandleSizeEvent( &pEvent->xconfigure );
                break;

            case VisibilityNotify:
                nVisibility_ = pEvent->xvisibility.state;
                nRet = true;
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
                break;

            case ColormapNotify:
                nRet = false;
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
                if( ( nStyle_ & SalFrameStyleFlags::PLUG )
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
    m_vClipRectangles.clear();

    const int   dest_kind   = ShapeBounding;
    const int   op          = ShapeSet;
    const int   ordering    = YSorted;

    XWindowAttributes win_attrib;
    XRectangle        win_size;

    ::Window aShapeWindow = mhShellWindow;

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

void X11SalFrame::BeginSetClipRegion( sal_uInt32 /*nRects*/ )
{
    m_vClipRectangles.clear();
}

void X11SalFrame::UnionClipRegion( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight )
{
    m_vClipRectangles.emplace_back( XRectangle { static_cast<short>(nX), static_cast<short>(nY),
                                                 static_cast<unsigned short>(nWidth), static_cast<unsigned short>(nHeight) } );
}

void X11SalFrame::EndSetClipRegion()
{
    const int   dest_kind   = ShapeBounding;
    const int   ordering    = YSorted;
    const int   op = ShapeSet;

    ::Window aShapeWindow = mhShellWindow;
    XShapeCombineRectangles ( GetDisplay()->GetDisplay(),
                              aShapeWindow,
                              dest_kind,
                              0, 0, // x_off, y_off
                              m_vClipRectangles.data(),
                              m_vClipRectangles.size(),
                              op, ordering );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
