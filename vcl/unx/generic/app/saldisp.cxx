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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#if defined(SOLARIS) || defined(AIX)
#include <sal/alloca.h>
#include <osl/module.h>
#endif

#include <tools/prex.h>
#include <X11/cursorfont.h>
#include "unx/x11_cursors/salcursors.h"
#include "unx/x11_cursors/invert50.h"
#ifdef SOLARIS
#define XK_KOREAN
#endif
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>

#ifdef USE_XINERAMA_XORG
#include <X11/extensions/Xinerama.h>
#elif defined USE_XINERAMA_XSUN
#if defined(SOLARIS) && defined(INTEL) // missing extension header in standard installation
#define MAXFRAMEBUFFERS       16
Bool XineramaGetState(Display*, int);
Status XineramaGetInfo(Display*, int, XRectangle*, unsigned char*, int*);
#else
#include <X11/extensions/xinerama.h>
#endif
#endif

#include <tools/postx.h>

#include <vcl/svapp.hxx>
#include <unx/salunx.h>
#include <sal/types.h>
#include "unx/i18n_im.hxx"
#include "unx/i18n_xkb.hxx"
#include <unx/saldisp.hxx>
#include <unx/saldata.hxx>
#include <salinst.hxx>
#include <unx/salgdi.h>
#include <unx/salframe.h>
#include <vcl/keycodes.hxx>
#include <unx/salbmp.h>
#include <osl/mutex.h>
#include <unx/salobj.h>
#include <unx/sm.hxx>
#include <unx/wmadaptor.hxx>

#include <osl/socket.h>
#include <poll.h>

using namespace vcl_sal;

using ::rtl::OUString;

// -=-= #defines -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define SALCOLOR_WHITE      MAKE_SALCOLOR( 0xFF, 0xFF, 0xFF )
#define SALCOLOR_BLACK      MAKE_SALCOLOR( 0x00, 0x00, 0x00 )

// -=-= Prototyps =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-= static variables -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static const char* const EventNames[] =
{
    NULL,
    NULL,
    "KeyPress",
    "KeyRelease",
    "ButtonPress",
    "ButtonRelease",
    "MotionNotify",
    "EnterNotify",
    "LeaveNotify",
    "FocusIn",
    "FocusOut",
    "KeymapNotify",
    "Expose",
    "GraphicsExpose",
    "NoExpose",
    "VisibilityNotify",
    "CreateNotify",
    "DestroyNotify",
    "UnmapNotify",
    "MapNotify",
    "MapRequest",
    "ReparentNotify",
    "ConfigureNotify",
    "ConfigureRequest",
    "GravityNotify",
    "ResizeRequest",
    "CirculateNotify",
    "CirculateRequest",
    "PropertyNotify",
    "SelectionClear",
    "SelectionRequest",
    "SelectionNotify",
    "ColormapNotify",
    "ClientMessage",
    "MappingNotify"
};

// -=-= global inline =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
inline const char *Null( const char *p ) { return p ? p : ""; }
inline const char *GetEnv( const char *p ) { return Null( getenv( p ) ); }
inline const char *KeyStr( KeySym n ) { return Null( XKeysymToString( n ) ); }

inline const char *GetAtomName( Display *d, Atom a )
{ return Null( XGetAtomName( d, a ) ); }

inline double Hypothenuse( long w, long h )
{ return sqrt( (double)((w*w)+(h*h)) ); }

inline int ColorDiff( int r, int g, int b )
{ return (r*r)+(g*g)+(b*b); }

inline int ColorDiff( SalColor c1, int r, int g, int b )
{ return ColorDiff( (int)SALCOLOR_RED  (c1)-r,
                    (int)SALCOLOR_GREEN(c1)-g,
                    (int)SALCOLOR_BLUE (c1)-b ); }

// -=-= global functions -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static int sal_Shift( Pixel nMask )
{
    int i = 24;
    if( nMask < 0x00010000 ) { nMask <<= 16; i -= 16; }
    if( nMask < 0x01000000 ) { nMask <<=  8; i -=  8; }
    if( nMask < 0x10000000 ) { nMask <<=  4; i -=  4; }
    if( nMask < 0x40000000 ) { nMask <<=  2; i -=  2; }
    if( nMask < 0x80000000 ) { nMask <<=  1; i -=  1; }
    return i;
}

static int sal_significantBits( Pixel nMask )
{
    int nRotate = sizeof(Pixel)*4;
    int nBits = 0;
    while( nRotate-- )
    {
        if( nMask & 1 )
            nBits++;
        nMask >>= 1;
    }
    return nBits;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static sal_Bool sal_GetVisualInfo( Display *pDisplay, XID nVID, XVisualInfo &rVI )
{
    int         nInfos;
    XVisualInfo aTemplate;
    XVisualInfo*pInfos;

    aTemplate.visualid = nVID;

    pInfos = XGetVisualInfo( pDisplay, VisualIDMask, &aTemplate, &nInfos );
    if( !pInfos )
        return sal_False;

    rVI = *pInfos;
    XFree( pInfos );

    DBG_ASSERT( rVI.visualid == nVID,
                "sal_GetVisualInfo: could not get correct visual by visualId" );
    return sal_True;
}

// ---------------------------------------------------------------------------
extern "C" srv_vendor_t
sal_GetServerVendor( Display *p_display )
{
    typedef struct {
        srv_vendor_t    e_vendor;   // vendor as enum
        const char      *p_name;    // vendor name as returned by VendorString()
        unsigned int    n_len;  // number of chars to compare
    } vendor_t;

    const vendor_t p_vendorlist[] = {
        { vendor_sun,         "Sun Microsystems, Inc.",          10 },
        // allways the last entry: vendor_none to indicate eol
        { vendor_none,        NULL,                               0 },
    };

    // handle regular server vendors
    char     *p_name   = ServerVendor( p_display );
    vendor_t *p_vendor;
    for (p_vendor = const_cast<vendor_t*>(p_vendorlist); p_vendor->e_vendor != vendor_none; p_vendor++)
    {
        if ( strncmp (p_name, p_vendor->p_name, p_vendor->n_len) == 0 )
            return p_vendor->e_vendor;
    }

    // vendor not found in list
    return vendor_unknown;
}

// -=-= SalDisplay -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
sal_Bool SalDisplay::BestVisual( Display     *pDisplay,
                             int          nScreen,
                             XVisualInfo &rVI )
{
    VisualID nDefVID = XVisualIDFromVisual( DefaultVisual( pDisplay, nScreen ) );
    VisualID    nVID = 0;
    char       *pVID = getenv( "SAL_VISUAL" );
    if( pVID )
        sscanf( pVID, "%li", &nVID );

    if( nVID && sal_GetVisualInfo( pDisplay, nVID, rVI ) )
        return rVI.visualid == nDefVID;

    XVisualInfo aVI;
    aVI.screen = nScreen;
    // get all visuals
    int nVisuals;
    XVisualInfo* pVInfos = XGetVisualInfo( pDisplay, VisualScreenMask,
                                           &aVI, &nVisuals );
    // pVInfos should contain at least one visual, otherwise
    // we're in trouble
    int* pWeight = (int*)alloca( sizeof(int)*nVisuals );
    int i;
    for( i = 0; i < nVisuals; i++ )
    {
        sal_Bool bUsable = sal_False;
        int nTrueColor = 1;

        if ( pVInfos[i].screen != nScreen )
        {
            bUsable = sal_False;
        }
        else
        if( pVInfos[i].c_class == TrueColor )
        {
            nTrueColor = 2048;
            if( pVInfos[i].depth == 24 )
                bUsable = sal_True;
        }
        else if( pVInfos[i].c_class == PseudoColor )
        {
            bUsable = sal_True;
        }
        pWeight[ i ] = bUsable ? nTrueColor*pVInfos[i].depth : -1024;
        pWeight[ i ] -= pVInfos[ i ].visualid;
    }

    int nBestVisual = 0;
    int nBestWeight = -1024;
    for( i = 0; i < nVisuals; i++ )
    {
        if( pWeight[ i ] > nBestWeight )
        {
            nBestWeight = pWeight[ i ];
            nBestVisual = i;
        }
    }

    rVI = pVInfos[ nBestVisual ];

    XFree( pVInfos );
    return rVI.visualid == nDefVID;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

SalDisplay::SalDisplay( Display *display ) :
        mpInputMethod( NULL ),
        pDisp_( display ),
        m_nXDefaultScreen( 0 ),
        m_pWMAdaptor( NULL ),
        m_bUseRandRWrapper( true ),
        m_nLastUserEventTime( CurrentTime )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "SalDisplay::SalDisplay()\n" );
#endif
    SalGenericData *pData = GetGenericData();

    pXLib_ = NULL;
    DBG_ASSERT( ! pData->GetDisplay(), "Second SalDisplay created !!!\n" );
    pData->SetDisplay( this );

    m_nXDefaultScreen = SalX11Screen( DefaultScreen( pDisp_ ) );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalDisplay::~SalDisplay()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "SalDisplay::~SalDisplay()\n" );
#endif
    if( pDisp_ )
    {
        doDestruct();
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "display %p closed\n", pDisp_ );
#endif
        pDisp_ = NULL;
    }
    // don't do this in doDestruct since RandR extension adds hooks into Display
    // that is XCloseDisplay still needs the RandR library if it was used
    DeInitRandR();
}

void SalDisplay::doDestruct()
{
    SalGenericData *pData = GetGenericData();

    delete m_pWMAdaptor;
    m_pWMAdaptor = NULL;
    X11SalBitmap::ImplDestroyCache();
    X11SalGraphics::releaseGlyphPeer();

    if( IsDisplay() )
    {
        delete mpInputMethod, mpInputMethod = (SalI18N_InputMethod*)ILLEGAL_POINTER;
        delete mpKbdExtension, mpKbdExtension = (SalI18N_KeyboardExtension*)ILLEGAL_POINTER;

        for( unsigned int i = 0; i < m_aScreens.size(); i++ )
        {
            ScreenData& rData = m_aScreens[i];
            if( rData.m_bInit )
            {
                if( rData.m_aMonoGC != rData.m_aCopyGC )
                    XFreeGC( pDisp_, rData.m_aMonoGC );
                XFreeGC( pDisp_, rData.m_aCopyGC );
                XFreeGC( pDisp_, rData.m_aAndInvertedGC );
                XFreeGC( pDisp_, rData.m_aAndGC );
                XFreeGC( pDisp_, rData.m_aOrGC );
                XFreeGC( pDisp_, rData.m_aStippleGC );
                XFreePixmap( pDisp_, rData.m_hInvert50 );
                XDestroyWindow( pDisp_, rData.m_aRefWindow );
                Colormap aColMap = rData.m_aColormap.GetXColormap();
                if( aColMap != None && aColMap != DefaultColormap( pDisp_, i ) )
                    XFreeColormap( pDisp_, aColMap );
            }
        }

        for( size_t i = 0; i < POINTER_COUNT; i++ )
        {
            if( aPointerCache_[i] )
                XFreeCursor( pDisp_, aPointerCache_[i] );
        }

        if( pXLib_ )
            pXLib_->Remove( ConnectionNumber( pDisp_ ) );
    }

    if( pData->GetDisplay() == static_cast<const SalGenericDisplay *>( this ) )
        pData->SetDisplay( NULL );
}

static int DisplayHasEvent( int fd, SalX11Display *pDisplay  )
{
  (void)fd;
  DBG_ASSERT( ConnectionNumber( pDisplay->GetDisplay() ) == fd,
              "wrong fd in DisplayHasEvent" );
  if( ! pDisplay->IsDisplay() )
      return 0;

  int result;

  GetSalData()->m_pInstance->GetYieldMutex()->acquire();
  result = pDisplay->IsEvent();
  GetSalData()->m_pInstance->GetYieldMutex()->release();
  return result;
}
static int DisplayQueue( int fd, SalX11Display *pDisplay )
{
  (void)fd;
  DBG_ASSERT( ConnectionNumber( pDisplay->GetDisplay() ) == fd,
              "wrong fd in DisplayHasEvent" );
  int result;

  GetSalData()->m_pInstance->GetYieldMutex()->acquire();
  result =  XEventsQueued( pDisplay->GetDisplay(),
                        QueuedAfterReading );
  GetSalData()->m_pInstance->GetYieldMutex()->release();

  return result;
}
static int DisplayYield( int fd, SalX11Display *pDisplay )
{
  (void)fd;
  DBG_ASSERT( ConnectionNumber( pDisplay->GetDisplay() ) == fd,
              "wrong fd in DisplayHasEvent" );

  GetSalData()->m_pInstance->GetYieldMutex()->acquire();
  pDisplay->Yield();
  GetSalData()->m_pInstance->GetYieldMutex()->release();
  return sal_True;
}

SalX11Display::SalX11Display( Display *display )
        : SalDisplay( display )
{
    Init();

    pXLib_ = GetX11SalData()->GetLib();
    pXLib_->Insert( ConnectionNumber( pDisp_ ),
                    this,
                    (YieldFunc) DisplayHasEvent,
                    (YieldFunc) DisplayQueue,
                    (YieldFunc) DisplayYield );
}

SalX11Display::~SalX11Display()
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "SalX11Display::~SalX11Display()\n" );
#endif
    if( pDisp_ )
    {
        doDestruct();
        XCloseDisplay( pDisp_ );
        pDisp_ = NULL;
    }
}

void SalX11Display::PostUserEvent()
{
    if( pXLib_ )
        pXLib_->PostUserEvent();
}

SalDisplay::ScreenData *
SalDisplay::initScreen( SalX11Screen nXScreen ) const
{
    if( nXScreen.getXScreen() >= m_aScreens.size() )
        nXScreen = m_nXDefaultScreen;
    ScreenData* pSD = const_cast<ScreenData *>(&m_aScreens[nXScreen.getXScreen()]);
    if( pSD->m_bInit )
        return NULL;
    pSD->m_bInit = true;

    XVisualInfo aVI;
    Colormap    aColMap;

    if( SalDisplay::BestVisual( pDisp_, nXScreen.getXScreen(), aVI ) ) // DefaultVisual
        aColMap = DefaultColormap( pDisp_, nXScreen.getXScreen() );
    else
        aColMap = XCreateColormap( pDisp_,
                                   RootWindow( pDisp_, nXScreen.getXScreen() ),
                                   aVI.visual,
                                   AllocNone );

    Screen* pScreen = ScreenOfDisplay( pDisp_, nXScreen.getXScreen() );

    pSD->m_aSize = Size( WidthOfScreen( pScreen ), HeightOfScreen( pScreen ) );
    pSD->m_aRoot = RootWindow( pDisp_, nXScreen.getXScreen() );
    pSD->m_aVisual = SalVisual( &aVI );
    pSD->m_aColormap = SalColormap( this, aColMap, nXScreen );

    // we're interested in configure notification of root windows
    InitRandR( pSD->m_aRoot );

    // - - - - - - - - - - Reference Window/Default Drawable - -
    XSetWindowAttributes aXWAttributes;
    aXWAttributes.border_pixel      = 0;
    aXWAttributes.background_pixel  = 0;
    aXWAttributes.colormap          = aColMap;
    pSD->m_aRefWindow     = XCreateWindow( pDisp_,
                                          pSD->m_aRoot,
                                          0,0, 16,16, 0,
                                          pSD->m_aVisual.GetDepth(),
                                          InputOutput,
                                          pSD->m_aVisual.GetVisual(),
                                          CWBorderPixel|CWBackPixel|CWColormap,
                                          &aXWAttributes );

    // set client leader (session id gets set when session is started)
    if( pSD->m_aRefWindow )
    {
        // client leader must have WM_CLIENT_LEADER pointing to itself
        XChangeProperty( pDisp_,
                         pSD->m_aRefWindow,
                         XInternAtom( pDisp_, "WM_CLIENT_LEADER", False ),
                         XA_WINDOW,
                         32,
                         PropModeReplace,
                         (unsigned char*)&pSD->m_aRefWindow,
                         1
                         );

        rtl::OString aExec(rtl::OUStringToOString(SessionManagerClient::getExecName(), osl_getThreadTextEncoding()));
        const char* argv[2];
        argv[0] = "/bin/sh";
        argv[1] = aExec.getStr();
        XSetCommand( pDisp_, pSD->m_aRefWindow, const_cast<char**>(argv), 2 );
        XSelectInput( pDisp_, pSD->m_aRefWindow, PropertyChangeMask );

        // - - - - - - - - - - GCs - - - - - - - - - - - - - - - - -
        XGCValues values;
        values.graphics_exposures   = False;
        values.fill_style           = FillOpaqueStippled;
        values.background           = (1<<pSD->m_aVisual.GetDepth())-1;
        values.foreground           = 0;

        pSD->m_aCopyGC       = XCreateGC( pDisp_,
                                         pSD->m_aRefWindow,
                                         GCGraphicsExposures
                                         | GCForeground
                                         | GCBackground,
                                         &values );
        pSD->m_aAndInvertedGC= XCreateGC( pDisp_,
                                         pSD->m_aRefWindow,
                                         GCGraphicsExposures
                                         | GCForeground
                                         | GCBackground,
                                         &values );
        pSD->m_aAndGC        = XCreateGC( pDisp_,
                                         pSD->m_aRefWindow,
                                         GCGraphicsExposures
                                         | GCForeground
                                         | GCBackground,
                                         &values );
        pSD->m_aOrGC         = XCreateGC( pDisp_,
                                         pSD->m_aRefWindow,
                                         GCGraphicsExposures
                                         | GCForeground
                                         | GCBackground,
                                         &values    );
        pSD->m_aStippleGC    = XCreateGC( pDisp_,
                                         pSD->m_aRefWindow,
                                         GCGraphicsExposures
                                         | GCFillStyle
                                         | GCForeground
                                         | GCBackground,
                                         &values );

        XSetFunction( pDisp_, pSD->m_aAndInvertedGC,  GXandInverted );
        XSetFunction( pDisp_, pSD->m_aAndGC,          GXand );
        // PowerPC Solaris 2.5 (XSun 3500) Bug: GXor = GXnop
        XSetFunction( pDisp_, pSD->m_aOrGC,           GXxor );

        if( 1 == pSD->m_aVisual.GetDepth() )
        {
            XSetFunction( pDisp_, pSD->m_aCopyGC, GXcopyInverted );
            pSD->m_aMonoGC = pSD->m_aCopyGC;
        }
        else
        {
            Pixmap hPixmap = XCreatePixmap( pDisp_, pSD->m_aRefWindow, 1, 1, 1 );
            pSD->m_aMonoGC = XCreateGC( pDisp_,
                                       hPixmap,
                                       GCGraphicsExposures,
                                       &values );
            XFreePixmap( pDisp_, hPixmap );
        }
        pSD->m_hInvert50 = XCreateBitmapFromData( pDisp_,
                                                 pSD->m_aRefWindow,
                                                 reinterpret_cast<const char*>(invert50_bits),
                                                 invert50_width,
                                                 invert50_height );
    }
    return pSD;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalDisplay::Init()
{
    for( size_t i = 0; i < POINTER_COUNT; i++ )
        aPointerCache_[i] = None;

    mpFactory           = (AttributeProvider*)NULL;
    m_bXinerama         = false;

    int nDisplayScreens = ScreenCount( pDisp_ );
    m_aScreens = std::vector<ScreenData>(nDisplayScreens);

    mbExactResolution = false;
    /*  #i15507#
     *  Xft resolution should take precedence since
     *  it is what modern desktops use.
     */
    const char* pValStr = XGetDefault( pDisp_, "Xft", "dpi" );
    if( pValStr != NULL )
    {
        const rtl::OString aValStr( pValStr );
        const long nDPI = (long) aValStr.toDouble();
        // guard against insane resolution
        if( (nDPI >= 50) && (nDPI <= 500) )
        {
            aResolution_ = Pair( nDPI, nDPI );
            mbExactResolution = true;
        }
    }
    if( mbExactResolution == false )
    {
        aResolution_     =
            Pair( DPI( WidthOfScreen( DefaultScreenOfDisplay( pDisp_ ) ),
                       DisplayWidthMM ( pDisp_, m_nXDefaultScreen.getXScreen() ) ),
                  DPI( HeightOfScreen( DefaultScreenOfDisplay( pDisp_ ) ),
                       DisplayHeightMM( pDisp_, m_nXDefaultScreen.getXScreen() ) ) );
    }

    nMaxRequestSize_    = XExtendedMaxRequestSize( pDisp_ ) * 4;
    if( !nMaxRequestSize_ )
        nMaxRequestSize_ = XMaxRequestSize( pDisp_ ) * 4;

    SetServerVendor();
    X11SalBitmap::ImplCreateCache();

    // - - - - - - - - - - Synchronize - - - - - - - - - - - - -
    if( getenv( "SAL_SYNCHRONIZE" ) )
        XSynchronize( pDisp_, True );

    // - - - - - - - - - - Keyboardmapping - - - - - - - - - - -
    ModifierMapping();

    // - - - - - - - - - - Window Manager  - - - - - - - - - - -
    m_pWMAdaptor = ::vcl_sal::WMAdaptor::createWMAdaptor( this );

    InitXinerama();

#ifdef DBG_UTIL
    PrintInfo();
#endif
}

void SalX11Display::SetupInput( SalI18N_InputMethod *pInputMethod )
{
    SetInputMethod( pInputMethod );

    GetGenericData()->ErrorTrapPush();
    SalI18N_KeyboardExtension *pKbdExtension = new SalI18N_KeyboardExtension( pDisp_ );
    XSync( pDisp_, False );

    bool bError = GetGenericData()->ErrorTrapPop( false );
    GetGenericData()->ErrorTrapPush();
    pKbdExtension->UseExtension( ! bError );
    GetGenericData()->ErrorTrapPop();

    SetKbdExtension( pKbdExtension );
}

// Keyboard
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

namespace {

bool InitXkb(Display* dpy)
{
    int nOpcode, nEvent, nError;
    int nXkbMajor = XkbMajorVersion;
    int nXkbMinor = XkbMinorVersion;

    if (!XkbLibraryVersion(&nXkbMajor, &nXkbMinor))
        return false;

    return XkbQueryExtension(
        dpy, &nOpcode, &nEvent, &nError, &nXkbMajor, &nXkbMinor);
}

unsigned int GetKeySymMask(Display* dpy, KeySym nKeySym)
{
    int nMask = 0;
    XModifierKeymap* pXmkMap = XGetModifierMapping(dpy);
    KeyCode nKeyCode = XKeysymToKeycode(dpy, nKeySym);
    if (nKeyCode == NoSymbol)
        return 0;

    for (int i = 0; i < 8; ++i)
    {
        KeyCode nThisKeyCode = pXmkMap->modifiermap[pXmkMap->max_keypermod*i];
        if (nThisKeyCode == nKeyCode)
            nMask = 1 << i;
    }
    XFreeModifiermap(pXmkMap);
    return nMask;
}

}

void SalDisplay::SimulateKeyPress( sal_uInt16 nKeyCode )
{
    if (nKeyCode == KEY_CAPSLOCK)
    {
        Display* dpy = GetDisplay();
        if (!InitXkb(dpy))
            return;

        unsigned int nMask = GetKeySymMask(dpy, XK_Caps_Lock);
        XkbStateRec xkbState;
        XkbGetState(dpy, XkbUseCoreKbd, &xkbState);
        unsigned int nCapsLockState = xkbState.locked_mods & nMask;
        if (nCapsLockState)
            XkbLockModifiers (dpy, XkbUseCoreKbd, nMask, 0);
        else
            XkbLockModifiers (dpy, XkbUseCoreKbd, nMask, nMask);
    }
}

sal_uInt16 SalDisplay::GetIndicatorState() const
{
    unsigned int _state = 0;
    sal_uInt16 nState = 0;
    XkbGetIndicatorState(pDisp_, XkbUseCoreKbd, &_state);

    if ((_state & 0x00000001))
        nState |= INDICATOR_CAPSLOCK;
    if ((_state & 0x00000002))
        nState |= INDICATOR_NUMLOCK;
    if ((_state & 0x00000004))
        nState |= INDICATOR_SCROLLLOCK;

    return nState;
}

rtl::OUString SalDisplay::GetKeyNameFromKeySym( KeySym nKeySym ) const
{
    rtl::OUString aLang = Application::GetSettings().GetUILocale().Language;
    rtl::OUString aRet;

    // return an empty string for keysyms that are not bound to
    // any key code
    XLIB_KeyCode aKeyCode = XKeysymToKeycode( GetDisplay(), nKeySym );
    if( aKeyCode != 0 && aKeyCode != NoSymbol )
    {
        if( !nKeySym )
            aRet = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "???" ) );
        else
        {
            aRet = ::vcl_sal::getKeysymReplacementName( aLang, nKeySym );
            if( aRet.isEmpty() )
            {
                const char *pString = XKeysymToString( nKeySym );
                int n = strlen( pString );
                if( n > 2 && pString[n-2] == '_' )
                    aRet = rtl::OUString( pString, n-2, RTL_TEXTENCODING_ISO_8859_1 );
                else
                    aRet = rtl::OUString( pString, n, RTL_TEXTENCODING_ISO_8859_1 );
            }
        }
    }
    return aRet;
}

inline KeySym sal_XModifier2Keysym( Display         *pDisplay,
                                    XModifierKeymap *pXModMap,
                                    int              n )
{
    return XkbKeycodeToKeysym( pDisplay,
                             pXModMap->modifiermap[n*pXModMap->max_keypermod],
                             0,0 );
}

void SalDisplay::ModifierMapping()
{
    XModifierKeymap *pXModMap = XGetModifierMapping( pDisp_ );

    bNumLockFromXS_ = True;
    nShiftKeySym_   = sal_XModifier2Keysym( pDisp_, pXModMap, ShiftMapIndex );
    nCtrlKeySym_    = sal_XModifier2Keysym( pDisp_, pXModMap, ControlMapIndex );
    nMod1KeySym_    = sal_XModifier2Keysym( pDisp_, pXModMap, Mod1MapIndex );
    // Auf Sun-Servern und SCO-Severn beruecksichtigt XLookupString
    // nicht den NumLock Modifier.
    if( GetServerVendor() == vendor_sun )
    {
        XLIB_KeyCode aNumLock = XKeysymToKeycode( pDisp_, XK_Num_Lock );

        if( aNumLock ) for( int i = ShiftMapIndex; i <= Mod5MapIndex; i++ )
        {
            if( pXModMap->modifiermap[i*pXModMap->max_keypermod] == aNumLock )
            {
                bNumLockFromXS_ = False;
                nNumLockIndex_  = i;
                nNumLockMask_   = 1<<i;
                break;
            }
        }
    }

    XFreeModifiermap( pXModMap );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
rtl::OUString SalDisplay::GetKeyName( sal_uInt16 nKeyCode ) const
{
    String aStrMap;
    String aCustomKeyName;

    if( nKeyCode & KEY_MOD1 )
        aStrMap += GetKeyNameFromKeySym( nCtrlKeySym_ );

    if( nKeyCode & KEY_MOD2 )
    {
        if( aStrMap.Len() )
            aStrMap += '+';
        aStrMap += GetKeyNameFromKeySym( nMod1KeySym_ );
    }

    if( nKeyCode & KEY_SHIFT )
    {
        if( aStrMap.Len() )
            aStrMap += '+';
        aStrMap += GetKeyNameFromKeySym( nShiftKeySym_ );
    }
    nKeyCode &= 0x0FFF;

    KeySym nKeySym = 0;

    if( KEY_0 <= nKeyCode && nKeyCode <= KEY_9 )
        nKeySym = XK_0 + (nKeyCode - KEY_0);
    else if( KEY_A <= nKeyCode && nKeyCode <= KEY_Z )
        nKeySym = XK_A + (nKeyCode - KEY_A);
    else if( KEY_F1 <= nKeyCode && nKeyCode <= KEY_F26 ) // Existiert die Taste
        nKeySym = XK_F1 + (nKeyCode - KEY_F1);
    else switch( nKeyCode )
    {
        case KEY_DOWN:
            nKeySym = XK_Down;
            break;
        case KEY_UP:
            nKeySym = XK_Up;
            break;
        case KEY_LEFT:
            nKeySym = XK_Left;
            break;
        case KEY_RIGHT:
            nKeySym = XK_Right;
            break;
        case KEY_HOME:
            nKeySym = XK_Home;
            break;
        case KEY_END:
            nKeySym = XK_End;
            break;
        case KEY_PAGEUP:
            nKeySym = XK_Prior;
            break;
        case KEY_PAGEDOWN:
            nKeySym = XK_Next;
            break;
        case KEY_RETURN:
            nKeySym = XK_Return;
            break;
        case KEY_ESCAPE:
            nKeySym = XK_Escape;
            break;
        case KEY_TAB:
            nKeySym = XK_Tab;
            break;
        case KEY_BACKSPACE:
            nKeySym = XK_BackSpace;
            break;
        case KEY_SPACE:
            nKeySym = XK_space;
            break;
        case KEY_INSERT:
            nKeySym = XK_Insert;
            break;
        case KEY_DELETE:
            nKeySym = XK_Delete;
            break;

        #if !defined (SunXK_Undo)
            #define SunXK_Stop      0x0000FF69  // XK_Cancel
            #define SunXK_Props     0x1005FF70
            #define SunXK_Front     0x1005FF71
            #define SunXK_Copy      0x1005FF72
            #define SunXK_Open      0x1005FF73
            #define SunXK_Paste     0x1005FF74
            #define SunXK_Cut       0x1005FF75
        #endif

        case KEY_REPEAT:
            nKeySym = XK_Redo;
            break;
        case KEY_PROPERTIES:
            nKeySym = SunXK_Props;
            break;
        case KEY_UNDO:
            nKeySym = XK_Undo;
            break;
        case KEY_FRONT:
            nKeySym = SunXK_Front;
            break;
        case KEY_COPY:
            nKeySym = SunXK_Copy;
            break;
        case KEY_OPEN:
            nKeySym = SunXK_Open;
            break;
        case KEY_PASTE:
            nKeySym = SunXK_Paste;
            break;
        case KEY_FIND:
            nKeySym = XK_Find;
            break;
        case KEY_CUT:
            nKeySym = GetServerVendor() == vendor_sun ? SunXK_Cut   : XK_L10;
            break;
        case KEY_ADD:
            aCustomKeyName = '+';
            break;
        case KEY_SUBTRACT:
            aCustomKeyName = '-';
            break;
        case KEY_MULTIPLY:
            nKeySym = XK_asterisk;
            break;
        case KEY_DIVIDE:
            nKeySym = XK_slash;
            break;
        case KEY_POINT:
            aCustomKeyName = '.';
            break;
        case KEY_COMMA:
            nKeySym = XK_comma;
            break;
        case KEY_LESS:
            nKeySym = XK_less;
            break;
        case KEY_GREATER:
            nKeySym = XK_greater;
            break;
        case KEY_EQUAL:
            nKeySym = XK_equal;
            break;
        case KEY_HELP:
            nKeySym = XK_Help;
            break;
        case KEY_HANGUL_HANJA:
            nKeySym = XK_Hangul_Hanja;
            break;
        case KEY_TILDE:
            nKeySym = XK_asciitilde;
            break;
        case KEY_QUOTELEFT:
            nKeySym = XK_grave;
            break;
        case KEY_BRACKETLEFT:
            aCustomKeyName = '[';
            break;
        case KEY_BRACKETRIGHT:
            aCustomKeyName = ']';
            break;
        case KEY_SEMICOLON:
            aCustomKeyName = ';';
            break;

        default:
            nKeySym = 0;
            break;
    }

    if( nKeySym )
    {
        String aKeyName = GetKeyNameFromKeySym( nKeySym );
        if( aKeyName.Len() )
        {
            if( aStrMap.Len() )
                aStrMap += '+';
            aStrMap += aKeyName;
        }
        else
            aStrMap.Erase();
    }
    else if (aCustomKeyName.Len())
    {
        // For semicolumn, bracket left and bracket right, it's better to use
        // their keys than their names. (fdo#32891)
        if (aStrMap.Len())
            aStrMap += '+';
        aStrMap += aCustomKeyName;
    }
    else
        aStrMap.Erase();

    return aStrMap;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifndef IsISOKey
#define IsISOKey( n ) (0x0000FE00==((n)&0xFFFFFF00))
#endif

sal_uInt16 SalDisplay::GetKeyCode( KeySym keysym, char*pcPrintable ) const
{
    sal_uInt16 nKey = 0;

    if( XK_a <= keysym && XK_z >= keysym )
        nKey = (sal_uInt16)(KEY_A + (keysym - XK_a));
    else if( XK_A <= keysym && XK_Z >= keysym )
        nKey = (sal_uInt16)(KEY_A + (keysym - XK_A));
    else if( XK_0 <= keysym && XK_9 >= keysym )
        nKey = (sal_uInt16)(KEY_0 + (keysym - XK_0));
    else if( IsModifierKey( keysym ) )
        ;
    else if( IsKeypadKey( keysym ) )
    {
        if( (keysym >= XK_KP_0) && (keysym <= XK_KP_9) )
        {
            nKey = (sal_uInt16)(KEY_0 + (keysym - XK_KP_0));
            *pcPrintable = '0' + nKey - KEY_0;
        }
        else if( IsPFKey( keysym ) )
            nKey = (sal_uInt16)(KEY_F1 + (keysym - XK_KP_F1));
        else switch( keysym )
        {
            case XK_KP_Space:
                nKey = KEY_SPACE;
                *pcPrintable = ' ';
                break;
            case XK_KP_Tab:
                nKey = KEY_TAB;
                break;
            case XK_KP_Enter:
                nKey = KEY_RETURN;
                break;
            case XK_KP_Begin:
            case XK_KP_Home:
                nKey = KEY_HOME;
                break;
            case XK_KP_Left:
                nKey = KEY_LEFT;
                break;
            case XK_KP_Up:
                nKey = KEY_UP;
                break;
            case XK_KP_Right:
                nKey = KEY_RIGHT;
                break;
            case XK_KP_Down:
                nKey = KEY_DOWN;
                break;
            case XK_KP_Prior: // XK_KP_Page_Up
                nKey = KEY_PAGEUP;
                break;
            case XK_KP_Next: // XK_KP_Page_Down
                nKey = KEY_PAGEDOWN;
                break;
            case XK_KP_End:
                nKey = KEY_END;
                break;
            case XK_KP_Insert:
                nKey = KEY_INSERT;
                break;
            case XK_KP_Delete:
                nKey = KEY_DELETE;
                break;
            case XK_KP_Equal:
                nKey = KEY_EQUAL;
                *pcPrintable = '=';
                break;
            case XK_KP_Multiply:
                nKey = KEY_MULTIPLY;
                *pcPrintable = '*';
                break;
            case XK_KP_Add:
                nKey = KEY_ADD;
                *pcPrintable = '+';
                break;
            case XK_KP_Separator:
                nKey = KEY_DECIMAL;
                *pcPrintable = ',';
                break;
            case XK_KP_Subtract:
                nKey = KEY_SUBTRACT;
                *pcPrintable = '-';
                break;
            case XK_KP_Decimal:
                nKey = KEY_DECIMAL;
                *pcPrintable = '.';
                break;
            case XK_KP_Divide:
                nKey = KEY_DIVIDE;
                *pcPrintable = '/';
                break;
        }
    }
    else if( IsFunctionKey( keysym ) )
    {
        if( bNumLockFromXS_ )
        {
            if( keysym >= XK_F1 && keysym <= XK_F26 )
                nKey = (sal_uInt16)(KEY_F1 + keysym - XK_F1);
        }
        else switch( keysym )
        {
            // - - - - - Sun X-Server Tastatur ohne Cursorblock ??? - - -
            case XK_R7: // XK_F27:
                nKey = KEY_HOME;
                break;
            case XK_R8: // XK_F28:
                nKey = KEY_UP;
                break;
            case XK_R9: // XK_F29:
                nKey = KEY_PAGEUP;
                break;
            case XK_R10: // XK_F30:
                nKey = KEY_LEFT;
                break;
            case XK_R11: // XK_F31:
                nKey = 0; // KEY_F31
                break;
            case XK_R12: // XK_F32:
                nKey = KEY_RIGHT;
                break;
            case XK_R13: // XK_F33:
                nKey = KEY_END;
                break;
            case XK_R14: // XK_F34:
                nKey = KEY_DOWN;
                break;
            case XK_R15: // XK_F35:
                nKey = KEY_PAGEDOWN;
                break;
            // - - - - - Sun X-Server Tastatur ??? - - - - - - - - - - - -
            case XK_L1: // XK_F11:
                nKey = KEY_F11; // on a sun keyboard this actually is usally SunXK_Stop,
                // but VCL doesn't have a key defintion for that
                break;
            case XK_L2: // XK_F12:
                if ( GetServerVendor() == vendor_sun )
                    nKey = KEY_REPEAT;
                else
                    nKey = KEY_F12;
                break;
            case XK_L3: // XK_F13:
                nKey = KEY_PROPERTIES; // KEY_F13
                break;
            case XK_L4: // XK_F14:
                nKey = KEY_UNDO; // KEY_F14
                break;
            case XK_L5: // XK_F15:
                nKey = KEY_F15; // KEY_FRONT
                break;
            case XK_L6: // XK_F16:
                nKey = KEY_COPY; // KEY_F16
                break;
            case XK_L7: // XK_F17:
                nKey = KEY_F17; // KEY_OPEN
                break;
            case XK_L8: // XK_F18:
                nKey = KEY_PASTE; // KEY_F18
                break;
            case XK_L9: // XK_F19:
                nKey = KEY_F19; // KEY_FIND
                break;
            case XK_L10: // XK_F20:
                nKey = KEY_CUT; // KEY_F20
                break;
            default:
                if( keysym >= XK_F1 && keysym <= XK_F26 )
                    nKey = (sal_uInt16)(KEY_F1 + keysym - XK_F1);
                break;
        }
    }
    else if( IsCursorKey( keysym ) )
    {
        switch( keysym )
        {
            case XK_Begin:
            case XK_Home:
                nKey = KEY_HOME;
                break;
            case XK_Left:
                nKey = KEY_LEFT;
                break;
            case XK_Up:
                nKey = KEY_UP;
                break;
            case XK_Right:
                nKey = KEY_RIGHT;
                break;
            case XK_Down:
                nKey = KEY_DOWN;
                break;
            case XK_Prior: // XK_Page_Up
                nKey = KEY_PAGEUP;
                break;
            case XK_Next: // XK_Page_Down
                nKey = KEY_PAGEDOWN;
                break;
            case XK_End:
                nKey = KEY_END;
                break;
        }
    }
    else if( IsMiscFunctionKey( keysym ) )
    {
        switch( keysym )
        {
            case XK_Insert:
                nKey = KEY_INSERT;
                break;
            case XK_Redo:
                nKey = KEY_REPEAT;
                break;
            case XK_Undo:
                nKey = KEY_UNDO;
                break;
            case XK_Find:
                nKey = KEY_FIND;
                break;
            case XK_Help:
                nKey = KEY_HELP;
                break;
            case XK_Menu:
                nKey = KEY_CONTEXTMENU;
                break;
        }
    }
    else if( IsISOKey( keysym ) )  // XK_ISO_
    {
        switch( keysym )
        {
            case 0xFE20: // XK_ISO_Left_Tab:
                nKey = KEY_TAB;
                break;
        }
    }
    else switch( keysym )
    {
        case XK_Return:
            nKey = KEY_RETURN;
            break;
        case XK_BackSpace:
            nKey = KEY_BACKSPACE;
            break;
        case XK_Delete:
            nKey = KEY_DELETE;
            break;
        case XK_space:
            nKey = KEY_SPACE;
            break;
        case XK_Tab:
            nKey = KEY_TAB;
            break;
        case XK_Escape:
            nKey = KEY_ESCAPE;
            break;
        case XK_plus:
            nKey = KEY_ADD;
            break;
        case XK_minus:
            nKey = KEY_SUBTRACT;
            break;
        case XK_asterisk:
            nKey = KEY_MULTIPLY;
            break;
        case XK_slash:
            nKey = KEY_DIVIDE;
            break;
        case XK_period:
            nKey = KEY_POINT;
            *pcPrintable = '.';
            break;
        case XK_comma:
            nKey = KEY_COMMA;
            break;
        case XK_less:
            nKey = KEY_LESS;
            break;
        case XK_greater:
            nKey = KEY_GREATER;
            break;
        case XK_equal:
            nKey = KEY_EQUAL;
            break;
        case XK_Hangul_Hanja:
            nKey = KEY_HANGUL_HANJA;
            break;
        case XK_asciitilde:
            nKey = KEY_TILDE;
            *pcPrintable = '~';
            break;
        case XK_grave:
            nKey = KEY_QUOTELEFT;
            *pcPrintable = '`';
            break;
        case XK_bracketleft:
            nKey = KEY_BRACKETLEFT;
            *pcPrintable = '[';
            break;
         case XK_bracketright:
             nKey = KEY_BRACKETRIGHT;
             *pcPrintable = ']';
             break;
        case XK_semicolon:
            nKey = KEY_SEMICOLON;
            *pcPrintable = ';';
            break;
        // - - - - - - - - - - - - -  Apollo - - - - - - - - - - - - - 0x1000
        case 0x1000FF02: // apXK_Copy
            nKey = KEY_COPY;
            break;
        case 0x1000FF03: // apXK_Cut
            nKey = KEY_CUT;
            break;
        case 0x1000FF04: // apXK_Paste
            nKey = KEY_PASTE;
            break;
        case 0x1000FF14: // apXK_Repeat
            nKey = KEY_REPEAT;
            break;
        // Exit, Save
        // - - - - - - - - - - - - - - D E C - - - - - - - - - - - - - 0x1000
        case 0x1000FF00:
            nKey = KEY_DELETE;
            break;
        // - - - - - - - - - - - - - -  H P  - - - - - - - - - - - - - 0x1000
        case 0x1000FF73: // hpXK_DeleteChar
            nKey = KEY_DELETE;
            break;
        case 0x1000FF74: // hpXK_BackTab
        case 0x1000FF75: // hpXK_KP_BackTab
            nKey = KEY_TAB;
            break;
        // - - - - - - - - - - - - - - I B M - - - - - - - - - - - - -
        // - - - - - - - - - - - - - - O S F - - - - - - - - - - - - - 0x1004
        case 0x1004FF02: // osfXK_Copy
            nKey = KEY_COPY;
            break;
        case 0x1004FF03: // osfXK_Cut
            nKey = KEY_CUT;
            break;
        case 0x1004FF04: // osfXK_Paste
            nKey = KEY_PASTE;
            break;
        case 0x1004FF07: // osfXK_BackTab
            nKey = KEY_TAB;
            break;
        case 0x1004FF08: // osfXK_BackSpace
            nKey = KEY_BACKSPACE;
            break;
        case 0x1004FF1B: // osfXK_Escape
            nKey = KEY_ESCAPE;
            break;
        // Up, Down, Left, Right, PageUp, PageDown
        // - - - - - - - - - - - - - - S C O - - - - - - - - - - - - -
        // - - - - - - - - - - - - - - S G I - - - - - - - - - - - - - 0x1007
        // - - - - - - - - - - - - - - S N I - - - - - - - - - - - - -
        // - - - - - - - - - - - - - - S U N - - - - - - - - - - - - - 0x1005
        case 0x1005FF10: // SunXK_F36
            nKey = KEY_F11;
            break;
        case 0x1005FF11: // SunXK_F37
            nKey = KEY_F12;
            break;
        case 0x1005FF70: // SunXK_Props
            nKey = KEY_PROPERTIES;
            break;
        case 0x1005FF71: // SunXK_Front
            nKey = KEY_FRONT;
            break;
        case 0x1005FF72: // SunXK_Copy
            nKey = KEY_COPY;
            break;
        case 0x1005FF73: // SunXK_Open
            nKey = KEY_OPEN;
            break;
        case 0x1005FF74: // SunXK_Paste
            nKey = KEY_PASTE;
            break;
        case 0x1005FF75: // SunXK_Cut
            nKey = KEY_CUT;
            break;
    }
    return nKey;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
KeySym SalDisplay::GetKeySym( XKeyEvent        *pEvent,
                                    unsigned char    *pPrintable,
                                    int              *pLen,
                                    KeySym           *pUnmodifiedKeySym,
                                    Status           *pStatusReturn,
                                    XIC              aInputContext ) const
{
    KeySym nKeySym = 0;
    memset( pPrintable, 0, *pLen );
    *pStatusReturn = 0;

    // first get the printable of the possibly modified KeySym
    if (   (aInputContext == 0)
        || (pEvent->type == KeyRelease)
        || (mpInputMethod != NULL && mpInputMethod->PosixLocale()) )
    {
        // XmbLookupString must not be called for KeyRelease events
        // Cannot enter space in c locale problem #89616# #88978# btraq #4478197
        *pLen = XLookupString( pEvent, (char*)pPrintable, 1, &nKeySym, NULL );
    }
    else
    {
        *pLen = XmbLookupString( aInputContext,
                        pEvent, (char*)pPrintable, *pLen - 1, &nKeySym, pStatusReturn );

        // Lookup the string again, now with appropriate size
        if ( *pStatusReturn == XBufferOverflow )
        {
            pPrintable[ 0 ] = (char)0;
            return 0;
        }

        switch ( *pStatusReturn )
        {
            case XBufferOverflow:
                /* unhandled error */
                break;
            case XLookupNone:
                /* unhandled error */
                break;
            case XLookupKeySym:
                /* this is a strange one: on exceed sometimes
                 * no printable is returned for the first char entered,
                 * just to retry lookup solves the problem. The problem
                 * is not yet fully understood, so restrict 2nd lookup
                 * to 7bit ascii chars */
                if ( (XK_space <= nKeySym) && (XK_asciitilde >= nKeySym) )
                {
                    *pLen = 1;
                    pPrintable[ 0 ] = (char)nKeySym;
                }
                break;
            case XLookupBoth:
            case XLookupChars:

                /* nothing to, char allready in pPrintable */
                break;
        }
    }

    if( !bNumLockFromXS_
        && (IsCursorKey(nKeySym)
            || IsFunctionKey(nKeySym)
            || IsKeypadKey(nKeySym)
            || XK_Delete == nKeySym ) )
    {
        // Bei einigen X-Servern muss man bei den Keypadtasten
        // schon sehr genau hinschauen. ZB. Solaris XServer:
        // 2, 4, 6, 8 werden als Cursorkeys klassifiziert (Up, Down, Left, Right
        // 1, 3, 5, 9 werden als Functionkeys klassifiziert (F27,F29,F33,F35)
        // 0 als Keypadkey und der Dezimalpunkt gar nicht (KP_Insert)
        KeySym nNewKeySym = XLookupKeysym( pEvent, nNumLockIndex_ );
        if( nNewKeySym != NoSymbol )
            nKeySym = nNewKeySym;
    }

    // Now get the unmodified KeySym for KeyCode retrieval
    // try to strip off modifiers, e.g. Ctrl-$ becomes Ctrl-Shift-4
    *pUnmodifiedKeySym  = XkbKeycodeToKeysym( GetDisplay(), pEvent->keycode, 0, 0);

    return nKeySym;
}

// Pointer
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define MAKE_BITMAP( name ) \
    XCreateBitmapFromData( pDisp_, \
                           DefaultRootWindow( pDisp_ ), \
                           reinterpret_cast<const char*>(name##_bits), \
                           name##_width, \
                           name##_height )

#define MAKE_CURSOR( name ) \
    aCursBitmap = MAKE_BITMAP( name##curs ); \
    aMaskBitmap = MAKE_BITMAP( name##mask ); \
    nXHot = name##curs_x_hot; \
    nYHot = name##curs_y_hot

XLIB_Cursor SalDisplay::GetPointer( int ePointerStyle )
{
    if( ePointerStyle >= POINTER_COUNT )
        return 0;

    XLIB_Cursor &aCur = aPointerCache_[ePointerStyle];

    if( aCur != None )
        return aCur;

    Pixmap          aCursBitmap = None, aMaskBitmap = None;
    unsigned int    nXHot = 0, nYHot = 0;

    switch( ePointerStyle )
    {
        case POINTER_NULL:
            MAKE_CURSOR( null );
            break;
        case POINTER_ARROW:
            aCur = XCreateFontCursor( pDisp_, XC_left_ptr );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_WAIT:
            aCur = XCreateFontCursor( pDisp_, XC_watch );
            break;
        case POINTER_TEXT:          // Mouse Pointer ist ein "I" Beam
            aCur = XCreateFontCursor( pDisp_, XC_xterm );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_HELP:
            aCur = XCreateFontCursor( pDisp_, XC_question_arrow );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_CROSS:         // Mouse Pointer ist ein Kreuz
            aCur = XCreateFontCursor( pDisp_, XC_crosshair );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_NSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_sb_v_double_arrow );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_SSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_sb_v_double_arrow );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_WSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_sb_h_double_arrow );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_ESIZE:
            aCur = XCreateFontCursor( pDisp_, XC_sb_h_double_arrow );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_WINDOW_NSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_top_side );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_WINDOW_SSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_bottom_side );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_WINDOW_WSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_left_side );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_WINDOW_ESIZE:
            aCur = XCreateFontCursor( pDisp_, XC_right_side );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_NWSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_top_left_corner );
            break;
        case POINTER_NESIZE:
            aCur = XCreateFontCursor( pDisp_, XC_top_right_corner );
            break;
        case POINTER_SWSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_bottom_left_corner );
            break;
        case POINTER_SESIZE:
            aCur = XCreateFontCursor( pDisp_, XC_bottom_right_corner );
            break;
        case POINTER_WINDOW_NWSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_top_left_corner );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_WINDOW_NESIZE:
            aCur = XCreateFontCursor( pDisp_, XC_top_right_corner );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_WINDOW_SWSIZE:
            aCur = XCreateFontCursor( pDisp_, XC_bottom_left_corner );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_WINDOW_SESIZE:
            aCur = XCreateFontCursor( pDisp_, XC_bottom_right_corner );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_HSPLIT:
            aCur = XCreateFontCursor( pDisp_, XC_sb_h_double_arrow );
            break;
        case POINTER_VSPLIT:
            aCur = XCreateFontCursor( pDisp_, XC_sb_v_double_arrow );
            break;
        case POINTER_HSIZEBAR:
            aCur = XCreateFontCursor( pDisp_, XC_sb_h_double_arrow ); // ???
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_VSIZEBAR:
            aCur = XCreateFontCursor( pDisp_, XC_sb_v_double_arrow ); // ???
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_REFHAND:
            aCur = XCreateFontCursor( pDisp_, XC_hand1 );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_HAND:
            aCur = XCreateFontCursor( pDisp_, XC_hand2 );
            break;
        case POINTER_MAGNIFY:
            MAKE_CURSOR( magnify_ );
            break;
        case POINTER_FILL:
            MAKE_CURSOR( fill_ );
            break;
        case POINTER_MOVE:
            aCur = XCreateFontCursor( pDisp_, XC_fleur );
            break;
        case POINTER_MOVEDATA:
            MAKE_CURSOR( movedata_ );
            break;
        case POINTER_COPYDATA:
            MAKE_CURSOR( copydata_ );
            break;
        case POINTER_MOVEFILE:
            MAKE_CURSOR( movefile_ );
            break;
        case POINTER_COPYFILE:
            MAKE_CURSOR( copyfile_ );
            break;
        case POINTER_MOVEFILES:
            MAKE_CURSOR( movefiles_ );
            break;
        case POINTER_COPYFILES:
            MAKE_CURSOR( copyfiles_ );
            break;
        case POINTER_NOTALLOWED:
            MAKE_CURSOR( nodrop_ );
            break;
        case POINTER_ROTATE:
            MAKE_CURSOR( rotate_ );
            break;
        case POINTER_HSHEAR:
            MAKE_CURSOR( hshear_ );
            break;
        case POINTER_VSHEAR:
            MAKE_CURSOR( vshear_ );
            break;
        case POINTER_DRAW_LINE:
            MAKE_CURSOR( drawline_ );
            break;
        case POINTER_DRAW_RECT:
            MAKE_CURSOR( drawrect_ );
            break;
        case POINTER_DRAW_POLYGON:
            MAKE_CURSOR( drawpolygon_ );
            break;
        case POINTER_DRAW_BEZIER:
            MAKE_CURSOR( drawbezier_ );
            break;
        case POINTER_DRAW_ARC:
            MAKE_CURSOR( drawarc_ );
            break;
        case POINTER_DRAW_PIE:
            MAKE_CURSOR( drawpie_ );
            break;
        case POINTER_DRAW_CIRCLECUT:
            MAKE_CURSOR( drawcirclecut_ );
            break;
        case POINTER_DRAW_ELLIPSE:
            MAKE_CURSOR( drawellipse_ );
            break;
        case POINTER_DRAW_CONNECT:
            MAKE_CURSOR( drawconnect_ );
            break;
        case POINTER_DRAW_TEXT:
            MAKE_CURSOR( drawtext_ );
            break;
        case POINTER_MIRROR:
            MAKE_CURSOR( mirror_ );
            break;
        case POINTER_CROOK:
            MAKE_CURSOR( crook_ );
            break;
        case POINTER_CROP:
            MAKE_CURSOR( crop_ );
            break;
        case POINTER_MOVEPOINT:
            MAKE_CURSOR( movepoint_ );
            break;
        case POINTER_MOVEBEZIERWEIGHT:
            MAKE_CURSOR( movebezierweight_ );
            break;
        case POINTER_DRAW_FREEHAND:
            MAKE_CURSOR( drawfreehand_ );
            break;
        case POINTER_DRAW_CAPTION:
            MAKE_CURSOR( drawcaption_ );
            break;
        case POINTER_PEN:       // Mouse Pointer ist ein Stift
            aCur = XCreateFontCursor( pDisp_, XC_pencil );
            DBG_ASSERT( aCur != None, "GetPointer: Could not define cursor" );
            break;
        case POINTER_LINKDATA:
            MAKE_CURSOR( linkdata_ );
            break;
        case POINTER_MOVEDATALINK:
            MAKE_CURSOR( movedlnk_ );
            break;
        case POINTER_COPYDATALINK:
            MAKE_CURSOR( copydlnk_ );
            break;
        case POINTER_LINKFILE:
            MAKE_CURSOR( linkfile_ );
            break;
        case POINTER_MOVEFILELINK:
            MAKE_CURSOR( moveflnk_ );
            break;
        case POINTER_COPYFILELINK:
            MAKE_CURSOR( copyflnk_ );
            break;
        case POINTER_CHART:
            MAKE_CURSOR( chart_ );
            break;
        case POINTER_DETECTIVE:
            MAKE_CURSOR( detective_ );
            break;
        case POINTER_PIVOT_COL:
            MAKE_CURSOR( pivotcol_ );
            break;
        case POINTER_PIVOT_ROW:
            MAKE_CURSOR( pivotrow_ );
            break;
        case POINTER_PIVOT_FIELD:
            MAKE_CURSOR( pivotfld_ );
            break;
        case POINTER_PIVOT_DELETE:
            MAKE_CURSOR( pivotdel_ );
            break;
        case POINTER_CHAIN:
            MAKE_CURSOR( chain_ );
            break;
        case POINTER_CHAIN_NOTALLOWED:
            MAKE_CURSOR( chainnot_ );
            break;
        case POINTER_TIMEEVENT_MOVE:
            MAKE_CURSOR( timemove_ );
            break;
        case POINTER_TIMEEVENT_SIZE:
            MAKE_CURSOR( timesize_ );
            break;
        case POINTER_AUTOSCROLL_N:
            MAKE_CURSOR(asn_ );
            break;
        case POINTER_AUTOSCROLL_S:
            MAKE_CURSOR( ass_ );
            break;
        case POINTER_AUTOSCROLL_W:
            MAKE_CURSOR( asw_ );
            break;
        case POINTER_AUTOSCROLL_E:
            MAKE_CURSOR( ase_ );
            break;
        case POINTER_AUTOSCROLL_NW:
            MAKE_CURSOR( asnw_ );
            break;
        case POINTER_AUTOSCROLL_NE:
            MAKE_CURSOR( asne_ );
            break;
        case POINTER_AUTOSCROLL_SW:
            MAKE_CURSOR( assw_ );
            break;
        case POINTER_AUTOSCROLL_SE:
            MAKE_CURSOR( asse_ );
            break;
        case POINTER_AUTOSCROLL_NS:
            MAKE_CURSOR( asns_ );
            break;
        case POINTER_AUTOSCROLL_WE:
            MAKE_CURSOR( aswe_ );
            break;
        case POINTER_AUTOSCROLL_NSWE:
            MAKE_CURSOR( asnswe_ );
            break;
        case POINTER_AIRBRUSH:
            MAKE_CURSOR( airbrush_ );
            break;
        case POINTER_TEXT_VERTICAL:
            MAKE_CURSOR( vertcurs_ );
            break;

        // #i32329# Enhanced table selection
        case POINTER_TAB_SELECT_S:
            MAKE_CURSOR( tblsels_ );
            break;
        case POINTER_TAB_SELECT_E:
            MAKE_CURSOR( tblsele_ );
            break;
        case POINTER_TAB_SELECT_SE:
            MAKE_CURSOR( tblselse_ );
            break;
        case POINTER_TAB_SELECT_W:
            MAKE_CURSOR( tblselw_ );
            break;
        case POINTER_TAB_SELECT_SW:
            MAKE_CURSOR( tblselsw_ );
            break;

        // #i20119# Paintbrush tool
        case POINTER_PAINTBRUSH :
            MAKE_CURSOR( paintbrush_ );
            break;

        default:
            OSL_FAIL("pointer not implemented");
            aCur = XCreateFontCursor( pDisp_, XC_arrow );
            break;
    }

    if( None == aCur )
    {
        XColor      aBlack, aWhite, aDummy;
        Colormap    hColormap = GetColormap(m_nXDefaultScreen).GetXColormap();

        XAllocNamedColor( pDisp_, hColormap, "black", &aBlack, &aDummy );
        XAllocNamedColor( pDisp_, hColormap, "white", &aWhite, &aDummy );

        aCur = XCreatePixmapCursor( pDisp_,
                                    aCursBitmap, aMaskBitmap,
                                    &aBlack, &aWhite,
                                    nXHot, nYHot );

        XFreePixmap( pDisp_, aCursBitmap );
        XFreePixmap( pDisp_, aMaskBitmap );
    }

    return aCur;
}

int SalDisplay::CaptureMouse( SalFrame *pCapture )
{
    static const char* pEnv = getenv( "SAL_NO_MOUSEGRABS" );

    if( !pCapture )
    {
        m_pCapture = NULL;
        if( !pEnv || !*pEnv )
            XUngrabPointer( GetDisplay(), CurrentTime );
        XFlush( GetDisplay() );
        return 0;
    }

    m_pCapture = NULL;

    // FIXME: get rid of X11SalFrame
    const SystemEnvData* pEnvData = pCapture->GetSystemData();
    if( !pEnv || !*pEnv )
    {
        int ret = XGrabPointer( GetDisplay(),
                                (XLIB_Window)pEnvData->aWindow,
                                False,
                                PointerMotionMask| ButtonPressMask|ButtonReleaseMask,
                                GrabModeAsync,
                                GrabModeAsync,
                                None,
                                static_cast<X11SalFrame*>(pCapture)->GetCursor(),
                                CurrentTime );

        if( ret != GrabSuccess )
        {
            DBG_ASSERT( 1, "SalDisplay::CaptureMouse could not grab pointer\n");
            return -1;
        }
    }

    m_pCapture = pCapture;
    return 1;
}

// Events
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

sal_Bool SalX11Display::IsEvent()
{
    if( HasUserEvents() || XEventsQueued( pDisp_, QueuedAlready ) )
        return sal_True;

    XFlush( pDisp_ );
    return sal_False;
}

void SalX11Display::Yield()
{
    if( DispatchInternalEvent() )
        return;

    XEvent aEvent;
    DBG_ASSERT( static_cast<SalYieldMutex*>(GetSalData()->m_pInstance->GetYieldMutex())->GetThreadId() ==
                osl::Thread::getCurrentIdentifier(),
                "will crash soon since solar mutex not locked in SalDisplay::Yield" );

    XNextEvent( pDisp_, &aEvent );

    Dispatch( &aEvent );

#ifdef DBG_UTIL
    if( GetX11SalData()->HasXErrorOccurred() )
    {
        XFlush( pDisp_ );
        DbgPrintDisplayEvent("SalDisplay::Yield (WasXError)", &aEvent);
    }
#endif
    GetX11SalData()->ResetXErrorOccurred();
}

long SalX11Display::Dispatch( XEvent *pEvent )
{
    if( pEvent->type == XLIB_KeyPress || pEvent->type == KeyRelease )
    {
        XLIB_Window aWindow = pEvent->xkey.window;

        std::list< SalFrame* >::const_iterator it;
        for( it = m_aFrames.begin(); it != m_aFrames.end(); ++it )
        {
            const X11SalFrame* pFrame = static_cast< const X11SalFrame* >(*it);
            if( pFrame->GetWindow() == aWindow || pFrame->GetShellWindow() == aWindow )
            {
                aWindow = pFrame->GetWindow();
                break;
            }
        }
        if( it != m_aFrames.end() )
        {
            if ( mpInputMethod->FilterEvent( pEvent , aWindow ) )
                return 0;
        }
    }
    else
        if ( mpInputMethod->FilterEvent( pEvent, None ) )
            return 0;

    SalInstance* pInstance = GetSalData()->m_pInstance;
    pInstance->CallEventCallback( pEvent, sizeof( XEvent ) );

    switch( pEvent->type )
    {
        case MotionNotify:
            while( XCheckWindowEvent( pEvent->xany.display,
                                      pEvent->xany.window,
                                      ButtonMotionMask,
                                      pEvent ) )
                ;
            m_nLastUserEventTime = pEvent->xmotion.time;
            break;
        case PropertyNotify:
            if( pEvent->xproperty.atom == getWMAdaptor()->getAtom( WMAdaptor::VCL_SYSTEM_SETTINGS ) )
            {
                for( unsigned int i = 0; i < m_aScreens.size(); i++ )
                {
                    if( pEvent->xproperty.window == m_aScreens[i].m_aRefWindow )
                    {
                        std::list< SalFrame* >::const_iterator it;
                        for( it = m_aFrames.begin(); it != m_aFrames.end(); ++it )
                            (*it)->CallCallback( SALEVENT_SETTINGSCHANGED, NULL );
                        return 0;
                    }
                }
            }
            break;
        case MappingNotify:
            if( MappingModifier == pEvent->xmapping.request )
            {
                XRefreshKeyboardMapping( &pEvent->xmapping );
                ModifierMapping();
            }
            break;
        case ButtonPress:
        case ButtonRelease:
            m_nLastUserEventTime = pEvent->xbutton.time;
            break;
        case XLIB_KeyPress:
        case KeyRelease:
            m_nLastUserEventTime = pEvent->xkey.time;
            break;
        default:

            if (   GetKbdExtension()->UseExtension()
                && GetKbdExtension()->GetEventBase() == pEvent->type )
            {
                GetKbdExtension()->Dispatch( pEvent );
                return 1;
            }
            break;
    }

    std::list< SalFrame* >::iterator it;
    for( it = m_aFrames.begin(); it != m_aFrames.end(); ++it )
    {
        X11SalFrame* pFrame = static_cast< X11SalFrame* >(*it);
        XLIB_Window aDispatchWindow = pEvent->xany.window;
        if( pFrame->GetWindow() == aDispatchWindow
            || pFrame->GetShellWindow() == aDispatchWindow
            || pFrame->GetForeignParent() == aDispatchWindow
            )
        {
            return pFrame->Dispatch( pEvent );
        }
        if( pEvent->type == ConfigureNotify && pEvent->xconfigure.window == pFrame->GetStackingWindow() )
        {
            return pFrame->Dispatch( pEvent );
        }
    }

    // dispatch to salobjects
    X11SalObject::Dispatch( pEvent );

    // is this perhaps a root window that changed size ?
    processRandREvent( pEvent );

    return 0;
}

#ifdef DBG_UTIL
void SalDisplay::DbgPrintDisplayEvent(const char *pComment, XEvent *pEvent) const
{
    if( pEvent->type <= MappingNotify )
    {
        fprintf( stderr, "[%s] %s s=%d w=%ld\n",
                 pComment,
                 EventNames[pEvent->type],
                 pEvent->xany.send_event,
                 pEvent->xany.window );

        switch( pEvent->type )
        {
            case XLIB_KeyPress:
            case KeyRelease:
                fprintf( stderr, "\t\ts=%d c=%d\n",
                         pEvent->xkey.state,
                         pEvent->xkey.keycode );
                break;

            case ButtonPress:
            case ButtonRelease:
                fprintf( stderr, "\t\ts=%d b=%d x=%d y=%d rx=%d ry=%d\n",
                         pEvent->xbutton.state,
                         pEvent->xbutton.button,
                         pEvent->xbutton.x,
                         pEvent->xbutton.y,
                         pEvent->xbutton.x_root,
                         pEvent->xbutton.y_root );
                break;

            case MotionNotify:
                fprintf( stderr, "\t\ts=%d x=%d y=%d\n",
                         pEvent->xmotion.state,
                         pEvent->xmotion.x,
                         pEvent->xmotion.y );
                break;

            case EnterNotify:
            case LeaveNotify:
                fprintf( stderr, "\t\tm=%d f=%d x=%d y=%d\n",
                         pEvent->xcrossing.mode,
                         pEvent->xcrossing.focus,
                         pEvent->xcrossing.x,
                         pEvent->xcrossing.y );
                break;

            case FocusIn:
            case FocusOut:
                fprintf( stderr, "\t\tm=%d d=%d\n",
                         pEvent->xfocus.mode,
                         pEvent->xfocus.detail );
                break;

            case Expose:
            case GraphicsExpose:
                fprintf( stderr, "\t\tc=%d %d*%d %d+%d\n",
                         pEvent->xexpose.count,
                         pEvent->xexpose.width,
                         pEvent->xexpose.height,
                         pEvent->xexpose.x,
                         pEvent->xexpose.y );
                break;

            case VisibilityNotify:
                fprintf( stderr, "\t\ts=%d\n",
                         pEvent->xvisibility.state );
                break;

            case CreateNotify:
            case DestroyNotify:
                break;

            case MapNotify:
            case UnmapNotify:
                break;

            case ReparentNotify:
                fprintf( stderr, "\t\tp=%d x=%d y=%d\n",
                         sal::static_int_cast< int >(pEvent->xreparent.parent),
                         pEvent->xreparent.x,
                         pEvent->xreparent.y );
                break;

            case ConfigureNotify:
                fprintf( stderr, "\t\tb=%d %d*%d %d+%d\n",
                         pEvent->xconfigure.border_width,
                         pEvent->xconfigure.width,
                         pEvent->xconfigure.height,
                         pEvent->xconfigure.x,
                         pEvent->xconfigure.y );
                break;

            case PropertyNotify:
                fprintf( stderr, "\t\ta=%s (0x%X)\n",
                         GetAtomName( pDisp_, pEvent->xproperty.atom ),
                         sal::static_int_cast< unsigned int >(
                             pEvent->xproperty.atom) );
                break;

            case ColormapNotify:
                fprintf( stderr, "\t\tc=%ld n=%d s=%d\n",
                         pEvent->xcolormap.colormap,
                         pEvent->xcolormap.c_new,
                         pEvent->xcolormap.state );
                break;

            case ClientMessage:
                fprintf( stderr, "\t\ta=%s (0x%X) f=%i [0x%lX,0x%lX,0x%lX,0x%lX,0x%lX])\n",
                         GetAtomName( pDisp_, pEvent->xclient.message_type ),
                         sal::static_int_cast< unsigned int >(
                             pEvent->xclient.message_type),
                         pEvent->xclient.format,
                         pEvent->xclient.data.l[0],
                         pEvent->xclient.data.l[1],
                         pEvent->xclient.data.l[2],
                         pEvent->xclient.data.l[3],
                         pEvent->xclient.data.l[4] );
                break;

            case MappingNotify:
                fprintf( stderr, "\t\tr=%sd\n",
                         MappingModifier == pEvent->xmapping.request
                         ? "MappingModifier"
                         : MappingKeyboard == pEvent->xmapping.request
                           ? "MappingKeyboard"
                           : "MappingPointer" );

                break;
        }
    }
    else
        fprintf( stderr, "[%s] %d s=%d w=%ld\n",
                 pComment,
                 pEvent->type,
                 pEvent->xany.send_event,
                 pEvent->xany.window );
}
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void SalDisplay::PrintInfo() const
{
    if( IsDisplay() )
    {
        fprintf( stderr, "\n" );
        fprintf( stderr, "Environment\n" );
        fprintf( stderr, "\t$DISPLAY          \t\"%s\"\n",
                 GetEnv( "DISPLAY" ) );
        fprintf( stderr, "\t$SAL_VISUAL       \t\"%s\"\n",
                 GetEnv( "SAL_VISUAL" ) );
        fprintf( stderr, "\t$SAL_IGNOREXERRORS\t\"%s\"\n",
                 GetEnv( "SAL_IGNOREXERRORS" ) );
        fprintf( stderr, "\t$SAL_PROPERTIES   \t\"%s\"\n",
                 GetEnv( "SAL_PROPERTIES" ) );
        fprintf( stderr, "\t$SAL_SYNCHRONIZE  \t\"%s\"\n",
                 GetEnv( "SAL_SYNCHRONIZE" ) );

        char sHostname[ 120 ];
        gethostname (sHostname, 120 );
        fprintf( stderr, "Client\n" );
        fprintf( stderr, "\tHost              \t\"%s\"\n",
                 sHostname );

        fprintf( stderr, "Display\n" );
        fprintf( stderr, "\tHost              \t\"%s\"\n",
                 DisplayString(pDisp_) );
        fprintf( stderr, "\tVendor (Release)  \t\"%s (%d)\"\n",
                 ServerVendor(pDisp_), VendorRelease(pDisp_) );
        fprintf( stderr, "\tProtocol          \t%d.%d\n",
                 ProtocolVersion(pDisp_), ProtocolRevision(pDisp_) );
        fprintf( stderr, "\tScreen (count,def)\t%d (%d,%d)\n",
                 m_nXDefaultScreen.getXScreen(),
                 ScreenCount(pDisp_), DefaultScreen(pDisp_) );
        fprintf( stderr, "\tshift ctrl alt    \t%s (0x%X) %s (0x%X) %s (0x%X)\n",
                 KeyStr( nShiftKeySym_ ), sal::static_int_cast< unsigned int >(nShiftKeySym_),
                 KeyStr( nCtrlKeySym_ ),  sal::static_int_cast< unsigned int >(nCtrlKeySym_),
                 KeyStr( nMod1KeySym_ ),  sal::static_int_cast< unsigned int >(nMod1KeySym_) );
        if( XExtendedMaxRequestSize(pDisp_) * 4 )
            fprintf( stderr, "\tXMaxRequestSize   \t%ld %ld [bytes]\n",
                     XMaxRequestSize(pDisp_) * 4, XExtendedMaxRequestSize(pDisp_) * 4 );
        if( GetProperties() != PROPERTY_DEFAULT )
            fprintf( stderr, "\tProperties        \t0x%lX\n", GetProperties() );
        fprintf( stderr, "\tWMName            \t%s\n", rtl::OUStringToOString( getWMAdaptor()->getWindowManagerName(), osl_getThreadTextEncoding() ).getStr() );
    }
    fprintf( stderr, "Screen\n" );
    fprintf( stderr, "\tResolution/Size   \t%ld*%ld %ld*%ld %.1lf\"\n",
             aResolution_.A(), aResolution_.B(),
             m_aScreens[m_nXDefaultScreen.getXScreen()].m_aSize.Width(),
             m_aScreens[m_nXDefaultScreen.getXScreen()].m_aSize.Height(),
             Hypothenuse( DisplayWidthMM ( pDisp_, m_nXDefaultScreen.getXScreen() ),
                          DisplayHeightMM( pDisp_, m_nXDefaultScreen.getXScreen() ) ) / 25.4 );
    fprintf( stderr, "\tBlack&White       \t%lu %lu\n",
             GetColormap(m_nXDefaultScreen).GetBlackPixel(),
             GetColormap(m_nXDefaultScreen).GetWhitePixel() );
    fprintf( stderr, "\tRGB               \t0x%lx 0x%lx 0x%lx\n",
             GetVisual(m_nXDefaultScreen).red_mask,
             GetVisual(m_nXDefaultScreen).green_mask,
             GetVisual(m_nXDefaultScreen).blue_mask );
}

void SalDisplay::addXineramaScreenUnique( int i, long i_nX, long i_nY, long i_nWidth, long i_nHeight )
{
    // see if any frame buffers are at the same coordinates
    // this can happen with weird configuration e.g. on
    // XFree86 and Clone displays
    const size_t nScreens = m_aXineramaScreens.size();
    for( size_t n = 0; n < nScreens; n++ )
    {
        if( m_aXineramaScreens[n].Left() == i_nX &&
            m_aXineramaScreens[n].Top() == i_nY )
        {
            if( m_aXineramaScreens[n].GetWidth() < i_nWidth ||
                m_aXineramaScreens[n].GetHeight() < i_nHeight )
            {
                m_aXineramaScreenIndexMap[i] = n;
                m_aXineramaScreens[n].SetSize( Size( i_nWidth, i_nHeight ) );
            }
            return;
        }
    }
    m_aXineramaScreenIndexMap[i] = m_aXineramaScreens.size();
    m_aXineramaScreens.push_back( Rectangle( Point( i_nX, i_nY ), Size( i_nWidth, i_nHeight ) ) );
}

void SalDisplay::InitXinerama()
{
    if( m_aScreens.size() > 1 )
    {
        m_bXinerama = false;
        return; // multiple screens mean no xinerama
    }
#if defined(USE_XINERAMA_XSUN)
    int nFramebuffers = 1;
    if( XineramaGetState( pDisp_, m_nDefaultScreen ) )
    {
        XRectangle pFramebuffers[MAXFRAMEBUFFERS];
        unsigned char hints[MAXFRAMEBUFFERS];
        int result = XineramaGetInfo( pDisp_,
                                      m_nDefaultScreen,
                                      pFramebuffers,
                                      hints,
                                      &nFramebuffers );
        if( result > 0 && nFramebuffers > 1 )
        {
            m_bXinerama = true;
            m_aXineramaScreens = std::vector<Rectangle>();
            m_aXineramaScreenIndexMap = std::vector<int>(nFramebuffers);
            for( int i = 0; i < nFramebuffers; i++ )
                addXineramaScreenUnique( i, pFramebuffers[i].x,
                                         pFramebuffers[i].y,
                                         pFramebuffers[i].width,
                                         pFramebuffers[i].height );
        }
    }
#elif defined(USE_XINERAMA_XORG)
    if( XineramaIsActive( pDisp_ ) )
    {
        int nFramebuffers = 1;
        XineramaScreenInfo* pScreens = XineramaQueryScreens( pDisp_, &nFramebuffers );
        if( pScreens )
        {
            if( nFramebuffers > 1 )
            {
                m_aXineramaScreens = std::vector<Rectangle>();
                m_aXineramaScreenIndexMap = std::vector<int>(nFramebuffers);
                for( int i = 0; i < nFramebuffers; i++ )
                {
                    addXineramaScreenUnique( i, pScreens[i].x_org,
                                             pScreens[i].y_org,
                                             pScreens[i].width,
                                             pScreens[i].height );
                }
                m_bXinerama = m_aXineramaScreens.size() > 1;
            }
            XFree( pScreens );
        }
    }
#endif
#if OSL_DEBUG_LEVEL > 1
    if( m_bXinerama )
    {
        for( std::vector< Rectangle >::const_iterator it = m_aXineramaScreens.begin(); it != m_aXineramaScreens.end(); ++it )
            fprintf( stderr, "Xinerama screen: %ldx%ld+%ld+%ld\n", it->GetWidth(), it->GetHeight(), it->Left(), it->Top() );
    }
#endif
}


extern "C"
{
    static Bool timestamp_predicate( Display*, XEvent* i_pEvent, XPointer i_pArg )
    {
        SalDisplay* pSalDisplay = reinterpret_cast<SalDisplay*>(i_pArg);
        if( i_pEvent->type == PropertyNotify &&
            i_pEvent->xproperty.window == pSalDisplay->GetDrawable( pSalDisplay->GetDefaultXScreen() ) &&
            i_pEvent->xproperty.atom == pSalDisplay->getWMAdaptor()->getAtom( WMAdaptor::SAL_GETTIMEEVENT )
            )
        return True;

        return False;
    }
}

XLIB_Time SalDisplay::GetLastUserEventTime( bool i_bAlwaysReget ) const
{
    if( m_nLastUserEventTime == CurrentTime || i_bAlwaysReget )
    {
        // get current server time
        unsigned char c = 0;
        XEvent aEvent;
        Atom nAtom = getWMAdaptor()->getAtom( WMAdaptor::SAL_GETTIMEEVENT );
        XChangeProperty( GetDisplay(), GetDrawable( GetDefaultXScreen() ),
                         nAtom, nAtom, 8, PropModeReplace, &c, 1 );
        XFlush( GetDisplay() );

        if( ! XIfEventWithTimeout( &aEvent, (XPointer)this, timestamp_predicate ) )
        {
            // this should not happen at all; still sometimes it happens
            aEvent.xproperty.time = CurrentTime;
        }

        m_nLastUserEventTime = aEvent.xproperty.time;
    }
    return m_nLastUserEventTime;
}

bool SalDisplay::XIfEventWithTimeout( XEvent* o_pEvent, XPointer i_pPredicateData,
                                      X_if_predicate i_pPredicate, long i_nTimeout ) const
{
    /* #i99360# ugly workaround an X11 library bug
       this replaces the following call:
       XIfEvent( GetDisplay(), o_pEvent, i_pPredicate, i_pPredicateData );
    */
    bool bRet = true;

    if( ! XCheckIfEvent( GetDisplay(), o_pEvent, i_pPredicate, i_pPredicateData ) )
    {
        // wait for some event to arrive
        struct pollfd aFD;
        aFD.fd = ConnectionNumber(GetDisplay());
        aFD.events = POLLIN;
        aFD.revents = 0;
        poll( &aFD, 1, i_nTimeout );
        if( ! XCheckIfEvent( GetDisplay(), o_pEvent, i_pPredicate, i_pPredicateData ) )
        {
            poll( &aFD, 1, i_nTimeout ); // try once more for a packet of events from the Xserver
            if( ! XCheckIfEvent( GetDisplay(), o_pEvent, i_pPredicate, i_pPredicateData ) )
            {
                bRet = false;
            }
        }
    }
    return bRet;
}

// -=-= SalVisual -=-=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalVisual::SalVisual()
{
    memset( this, 0, sizeof( SalVisual ) );
}

SalVisual::SalVisual( const XVisualInfo* pXVI )
{
    *(XVisualInfo*)this = *pXVI;
    if( GetClass() == TrueColor )
    {
        nRedShift_      = sal_Shift( red_mask );
        nGreenShift_    = sal_Shift( green_mask );
        nBlueShift_     = sal_Shift( blue_mask );

        nRedBits_       = sal_significantBits( red_mask );
        nGreenBits_     = sal_significantBits( green_mask );
        nBlueBits_      = sal_significantBits( blue_mask );

        if( GetDepth() == 24 )
            if( red_mask == 0xFF0000 )
                if( green_mask == 0xFF00 )
                    if( blue_mask  == 0xFF )
                        eRGBMode_ = RGB;
                    else
                        eRGBMode_ = otherSalRGB;
                else if( blue_mask  == 0xFF00 )
                    if( green_mask == 0xFF )
                        eRGBMode_ = RBG;
                    else
                        eRGBMode_ = otherSalRGB;
                else
                    eRGBMode_ = otherSalRGB;
            else if( green_mask == 0xFF0000 )
                if( red_mask == 0xFF00 )
                    if( blue_mask  == 0xFF )
                        eRGBMode_ = GRB;
                    else
                        eRGBMode_ = otherSalRGB;
                else if( blue_mask == 0xFF00 )
                    if( red_mask  == 0xFF )
                        eRGBMode_ = GBR;
                    else
                        eRGBMode_ = otherSalRGB;
                else
                    eRGBMode_ = otherSalRGB;
            else if( blue_mask == 0xFF0000 )
                if( red_mask == 0xFF00 )
                    if( green_mask  == 0xFF )
                        eRGBMode_ = BRG;
                    else
                        eRGBMode_ = otherSalRGB;
                else if( green_mask == 0xFF00 )
                    if( red_mask == 0xFF )
                        eRGBMode_ = BGR;
                    else
                        eRGBMode_ = otherSalRGB;
                else
                    eRGBMode_ = otherSalRGB;
            else
                eRGBMode_ = otherSalRGB;
        else
            eRGBMode_ = otherSalRGB;
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalVisual::~SalVisual()
{
    if( -1 == screen && VisualID(-1) == visualid ) delete visual;
}

// Konvertiert die Reihenfolge der Bytes eines Pixel in Bytes eines SalColors
// fuer die 6 XXXA ist das nicht reversibel
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// SalColor is RGB (ABGR) a=0xFF000000, r=0xFF0000, g=0xFF00, b=0xFF

#define SALCOLOR        RGB
#define SALCOLORREVERSE BGR

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalColor SalVisual::GetTCColor( Pixel nPixel ) const
{
    if( SALCOLOR == eRGBMode_ )
        return (SalColor)nPixel;

    if( SALCOLORREVERSE == eRGBMode_ )
        return MAKE_SALCOLOR( (nPixel & 0x0000FF),
                              (nPixel & 0x00FF00) >>  8,
                              (nPixel & 0xFF0000) >> 16);

    Pixel r = nPixel & red_mask;
    Pixel g = nPixel & green_mask;
    Pixel b = nPixel & blue_mask;

    if( otherSalRGB != eRGBMode_ ) // 8+8+8=24
        return MAKE_SALCOLOR( r >> nRedShift_,
                              g >> nGreenShift_,
                              b >> nBlueShift_ );

    if( nRedShift_ > 0 )   r >>= nRedShift_;   else r <<= -nRedShift_;
    if( nGreenShift_ > 0 ) g >>= nGreenShift_; else g <<= -nGreenShift_;
    if( nBlueShift_ > 0 )  b >>= nBlueShift_;  else b <<= -nBlueShift_;

    if( nRedBits_ != 8 )
        r |= (r & 0xff) >> (8-nRedBits_);
    if( nGreenBits_ != 8 )
        g |= (g & 0xff) >> (8-nGreenBits_);
    if( nBlueBits_ != 8 )
        b |= (b & 0xff) >> (8-nBlueBits_);

    return MAKE_SALCOLOR( r, g, b );
}

Pixel SalVisual::GetTCPixel( SalColor nSalColor ) const
{
    if( SALCOLOR == eRGBMode_ )
        return (Pixel)nSalColor;

    Pixel r = (Pixel)SALCOLOR_RED( nSalColor );
    Pixel g = (Pixel)SALCOLOR_GREEN( nSalColor );
    Pixel b = (Pixel)SALCOLOR_BLUE( nSalColor );

    if( SALCOLORREVERSE == eRGBMode_ )
        return (b << 16) | (g << 8) | (r);

    if( otherSalRGB != eRGBMode_ ) // 8+8+8=24
        return (r << nRedShift_) | (g << nGreenShift_) | (b << nBlueShift_);

    if( nRedShift_ > 0 )   r <<= nRedShift_;   else r >>= -nRedShift_;
    if( nGreenShift_ > 0 ) g <<= nGreenShift_; else g >>= -nGreenShift_;
    if( nBlueShift_ > 0 )  b <<= nBlueShift_;  else b >>= -nBlueShift_;

    return (r&red_mask) | (g&green_mask) | (b&blue_mask);
}

// -=-= SalColormap -=--=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalColormap::SalColormap( const SalDisplay *pDisplay, Colormap hColormap,
                          SalX11Screen nXScreen )
    : m_pDisplay( pDisplay ),
      m_hColormap( hColormap ),
      m_nXScreen( nXScreen )
{
    m_aVisual = m_pDisplay->GetVisual( m_nXScreen );

    XColor aColor;

    GetXPixel( aColor, 0x00, 0x00, 0x00 );
    m_nBlackPixel = aColor.pixel;

    GetXPixel( aColor, 0xFF, 0xFF, 0xFF );
    m_nWhitePixel = aColor.pixel;

    m_nUsed = 1 << m_aVisual.GetDepth();

    if( m_aVisual.GetClass() == PseudoColor )
    {
        int r, g, b;

        // black, white, gray, ~gray = 4
        GetXPixels( aColor, 0xC0, 0xC0, 0xC0 );

        // light colors: 3 * 2 = 6

        GetXPixels( aColor, 0x00, 0x00, 0xFF );
        GetXPixels( aColor, 0x00, 0xFF, 0x00 );
        GetXPixels( aColor, 0x00, 0xFF, 0xFF );

        // standard colors: 7 * 2 = 14
        GetXPixels( aColor, 0x00, 0x00, 0x80 );
        GetXPixels( aColor, 0x00, 0x80, 0x00 );
        GetXPixels( aColor, 0x00, 0x80, 0x80 );
        GetXPixels( aColor, 0x80, 0x00, 0x00 );
        GetXPixels( aColor, 0x80, 0x00, 0x80 );
        GetXPixels( aColor, 0x80, 0x80, 0x00 );
        GetXPixels( aColor, 0x80, 0x80, 0x80 );
        GetXPixels( aColor, 0x00, 0xB8, 0xFF ); // Blue 7

        // cube: 6*6*6 - 8 = 208
        for( r = 0; r < 0x100; r += 0x33 ) // 0x33, 0x66, 0x99, 0xCC, 0xFF
            for( g = 0; g < 0x100; g += 0x33 )
                for( b = 0; b < 0x100; b += 0x33 )
                    GetXPixels( aColor, r, g, b );

        // gray: 16 - 6 = 10
        for( g = 0x11; g < 0xFF; g += 0x11 )
            GetXPixels( aColor, g, g, g );

        // green: 16 - 6 = 10
        for( g = 0x11; g < 0xFF; g += 0x11 )
            GetXPixels( aColor, 0, g, 0 );

        // red: 16 - 6 = 10
        for( r = 0x11; r < 0xFF; r += 0x11 )
            GetXPixels( aColor, r, 0, 0 );

        // blue: 16 - 6 = 10
        for( b = 0x11; b < 0xFF; b += 0x11 )
            GetXPixels( aColor, 0, 0, b );
    }
}

// MonoChrome
SalColormap::SalColormap()
    : m_pDisplay( GetGenericData()->GetSalDisplay() ),
      m_hColormap( None ),
      m_nWhitePixel( 1 ),
      m_nBlackPixel( 0 ),
      m_nUsed( 2 ),
      m_nXScreen( m_pDisplay != NULL ? m_pDisplay->GetDefaultXScreen() : SalX11Screen( 0 ) )
{
    m_aPalette = std::vector<SalColor>(m_nUsed);

    m_aPalette[m_nBlackPixel] = SALCOLOR_BLACK;
    m_aPalette[m_nWhitePixel] = SALCOLOR_WHITE;
}

// TrueColor
SalColormap::SalColormap( sal_uInt16 nDepth )
    : m_pDisplay( GetGenericData()->GetSalDisplay() ),
      m_hColormap( None ),
      m_nWhitePixel( (1 << nDepth) - 1 ),
      m_nBlackPixel( 0x00000000 ),
      m_nUsed( 1 << nDepth ),
      m_nXScreen( GetGenericData()->GetSalDisplay()->GetDefaultXScreen() )
{
    const SalVisual *pVisual = &m_pDisplay->GetVisual( m_nXScreen );

    if( pVisual->GetClass() == TrueColor && pVisual->GetDepth() == nDepth )
        m_aVisual = *pVisual;
    else
    {
        XVisualInfo aVI;

        if( !XMatchVisualInfo( m_pDisplay->GetDisplay(),
                               m_pDisplay->GetDefaultXScreen().getXScreen(),
                               nDepth,
                               TrueColor,
                               &aVI ) )
        {
            aVI.visual          = new Visual();
            aVI.visualid        = (VisualID)0; // beware of temporary destructor below
            aVI.screen          = 0;
            aVI.depth           = nDepth;
            aVI.c_class         = TrueColor;
            if( 24 == nDepth ) // 888
            {
                aVI.red_mask        = 0xFF0000;
                aVI.green_mask      = 0x00FF00;
                aVI.blue_mask       = 0x0000FF;
            }
            else if( 16 == nDepth ) // 565
            {
                aVI.red_mask        = 0x00F800;
                aVI.green_mask      = 0x0007E0;
                aVI.blue_mask       = 0x00001F;
            }
            else if( 15 == nDepth ) // 555
            {
                aVI.red_mask        = 0x007C00;
                aVI.green_mask      = 0x0003E0;
                aVI.blue_mask       = 0x00001F;
            }
            else if( 12 == nDepth ) // 444
            {
                aVI.red_mask        = 0x000F00;
                aVI.green_mask      = 0x0000F0;
                aVI.blue_mask       = 0x00000F;
            }
            else if( 8 == nDepth ) // 332
            {
                aVI.red_mask        = 0x0000E0;
                aVI.green_mask      = 0x00001C;
                aVI.blue_mask       = 0x000003;
            }
            else
            {
                aVI.red_mask        = 0x000000;
                aVI.green_mask      = 0x000000;
                aVI.blue_mask       = 0x000000;
            }
            aVI.colormap_size   = 0;
            aVI.bits_per_rgb    = 8;

            aVI.visual->ext_data        = NULL;
            aVI.visual->visualid        = aVI.visualid;
            aVI.visual->c_class         = aVI.c_class;
            aVI.visual->red_mask        = aVI.red_mask;
            aVI.visual->green_mask      = aVI.green_mask;
            aVI.visual->blue_mask       = aVI.blue_mask;
            aVI.visual->bits_per_rgb    = aVI.bits_per_rgb;
            aVI.visual->map_entries     = aVI.colormap_size;

            m_aVisual = SalVisual( &aVI );
            // give ownership of constructed Visual() to m_aVisual
            // see SalVisual destructor
            m_aVisual.visualid        = (VisualID)-1;
            m_aVisual.screen          = -1;
        }
        else
            m_aVisual = SalVisual( &aVI );
    }
}

SalColormap::~SalColormap()
{
#ifdef DBG_UTIL
    m_hColormap      = (Colormap)ILLEGAL_POINTER;
    m_pDisplay       = (SalDisplay*)ILLEGAL_POINTER;
#endif
}

void SalColormap::GetPalette()
{
    Pixel i;
    m_aPalette = std::vector<SalColor>(m_nUsed);

    XColor *aColor = new XColor[m_nUsed];

    for( i = 0; i < m_nUsed; i++ )
    {
        aColor[i].red = aColor[i].green = aColor[i].blue = 0;
        aColor[i].pixel = i;
    }

    XQueryColors( m_pDisplay->GetDisplay(), m_hColormap, aColor, m_nUsed );

    for( i = 0; i < m_nUsed; i++ )
    {
        m_aPalette[i] = MAKE_SALCOLOR( aColor[i].red   >> 8,
                                       aColor[i].green >> 8,
                                       aColor[i].blue  >> 8 );
    }

    delete [] aColor;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
static sal_uInt16 sal_Lookup( const std::vector<SalColor>& rPalette,
                                int r, int g, int b,
                                Pixel nUsed )
{
    sal_uInt16 nPixel = 0;
    int    nBest  = ColorDiff( rPalette[0], r, g, b );

    for( sal_uInt16 i = 1; i < nUsed; i++ )
    {
        int n = ColorDiff( rPalette[i], r, g, b );

        if( n < nBest )
        {
            if( !n )
                return i;

            nPixel = i;
            nBest  = n;
        }
    }
    return nPixel;
}

void SalColormap::GetLookupTable()
{
    m_aLookupTable = std::vector<sal_uInt16>(16*16*16);

    int i = 0;
    for( int r = 0; r < 256; r += 17 )
        for( int g = 0; g < 256; g += 17 )
            for( int b = 0; b < 256; b += 17 )
                m_aLookupTable[i++] = sal_Lookup( m_aPalette, r, g, b, m_nUsed );
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalColor SalColormap::GetColor( Pixel nPixel ) const
{
    if( m_nBlackPixel == nPixel ) return SALCOLOR_BLACK;
    if( m_nWhitePixel == nPixel ) return SALCOLOR_WHITE;

    if( m_aVisual.GetVisual() )
    {
        if( m_aVisual.GetClass() == TrueColor )
            return m_aVisual.GetTCColor( nPixel );

        if( m_aPalette.empty()
            && m_hColormap
            && m_aVisual.GetDepth() <= 12
            && m_aVisual.GetClass() == PseudoColor )
            ((SalColormap*)this)->GetPalette();
    }

    if( !m_aPalette.empty() && nPixel < m_nUsed )
        return m_aPalette[nPixel];

    if( m_hColormap )
    {
        DBG_ASSERT( 1, "SalColormap::GetColor() !hColormap_\n" );
        return nPixel;
    }

    // DirectColor, StaticColor, StaticGray, GrayScale
    XColor aColor;

    aColor.pixel = nPixel;

    XQueryColor( m_pDisplay->GetDisplay(), m_hColormap, &aColor );

    return MAKE_SALCOLOR( aColor.red>>8, aColor.green>>8, aColor.blue>>8 );
}

inline sal_Bool SalColormap::GetXPixel( XColor &rColor,
                                          int     r,
                                          int     g,
                                          int     b ) const
{
    rColor.red      = r * 257;
    rColor.green    = g * 257;
    rColor.blue     = b * 257;
    return XAllocColor( GetXDisplay(), m_hColormap, &rColor );
}

sal_Bool SalColormap::GetXPixels( XColor &rColor,
                                    int     r,
                                    int     g,
                                    int     b ) const
{
    if( !GetXPixel( rColor, r, g, b ) )
        return sal_False;
    if( rColor.pixel & 1 )
        return sal_True;
    return GetXPixel( rColor, r^0xFF, g^0xFF, b^0xFF );
}

Pixel SalColormap::GetPixel( SalColor nSalColor ) const
{
    if( SALCOLOR_NONE == nSalColor )  return 0;
    if( SALCOLOR_BLACK == nSalColor ) return m_nBlackPixel;
    if( SALCOLOR_WHITE == nSalColor ) return m_nWhitePixel;

    if( m_aVisual.GetClass() == TrueColor )
        return m_aVisual.GetTCPixel( nSalColor );

    if( m_aLookupTable.empty() )
    {
        if( m_aPalette.empty()
            && m_hColormap
            && m_aVisual.GetDepth() <= 12
            && m_aVisual.GetClass() == PseudoColor ) // what else ???
            ((SalColormap*)this)->GetPalette();

        if( !m_aPalette.empty() )
            for( Pixel i = 0; i < m_nUsed; i++ )
                if( m_aPalette[i] == nSalColor )
                    return i;

        if( m_hColormap )
        {
            // DirectColor, StaticColor, StaticGray, GrayScale (PseudoColor)
            XColor aColor;

            if( GetXPixel( aColor,
                           SALCOLOR_RED  ( nSalColor ),
                           SALCOLOR_GREEN( nSalColor ),
                           SALCOLOR_BLUE ( nSalColor ) ) )
            {
                if( !m_aPalette.empty() && !m_aPalette[aColor.pixel] )
                {
                    const_cast<SalColormap*>(this)->m_aPalette[aColor.pixel] = nSalColor;

                    if( !(aColor.pixel & 1) && !m_aPalette[aColor.pixel+1] )
                    {
                        XColor aInversColor;

                        SalColor nInversColor = nSalColor ^ 0xFFFFFF;

                        GetXPixel( aInversColor,
                                   SALCOLOR_RED  ( nInversColor ),
                                   SALCOLOR_GREEN( nInversColor ),
                                   SALCOLOR_BLUE ( nInversColor ) );

                        if( !m_aPalette[aInversColor.pixel] )
                            const_cast<SalColormap*>(this)->m_aPalette[aInversColor.pixel] = nInversColor;
#ifdef DBG_UTIL
                        else
                            fprintf( stderr, "SalColormap::GetPixel() 0x%06lx=%lu 0x%06lx=%lu\n",
                                     static_cast< unsigned long >(nSalColor), aColor.pixel,
                                     static_cast< unsigned long >(nInversColor), aInversColor.pixel);
#endif
                    }
                }

                return aColor.pixel;
            }

#ifdef DBG_UTIL
            fprintf( stderr, "SalColormap::GetPixel() !XAllocColor %lx\n",
                     static_cast< unsigned long >(nSalColor) );
#endif
        }

        if( m_aPalette.empty() )
        {
#ifdef DBG_UTIL
            fprintf( stderr, "SalColormap::GetPixel() Palette empty %lx\n",
                     static_cast< unsigned long >(nSalColor));
#endif
            return nSalColor;
        }

        ((SalColormap*)this)->GetLookupTable();
    }

    // Colormatching ueber Palette
    sal_uInt16 r = SALCOLOR_RED  ( nSalColor );
    sal_uInt16 g = SALCOLOR_GREEN( nSalColor );
    sal_uInt16 b = SALCOLOR_BLUE ( nSalColor );
    return m_aLookupTable[ (((r+8)/17) << 8)
                         + (((g+8)/17) << 4)
                         +  ((b+8)/17) ];
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
