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
#include <stdio.h>
#include <stdlib.h>

#include <sal/alloca.h>
#include <rtl/locale.h>

#include <osl/thread.h>
#include <osl/process.h>
#include <sal/macros.h>
#include <vcl/configsettings.hxx>

#include <unx/wmadaptor.hxx>
#include <unx/saldisp.hxx>
#include <unx/salinst.h>
#include <unx/salframe.h>

#include <salgdi.hxx>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>

namespace vcl_sal {

class NetWMAdaptor : public WMAdaptor
{
    void setNetWMState( X11SalFrame* pFrame ) const;
    void initAtoms();
    virtual bool isValid() const override;
public:
    explicit NetWMAdaptor( SalDisplay* );

    virtual void setWMName( X11SalFrame* pFrame, const OUString& rWMName ) const override;
    virtual void maximizeFrame( X11SalFrame* pFrame, bool bHorizontal = true, bool bVertical = true ) const override;
    virtual void shade( X11SalFrame* pFrame, bool bToShaded ) const override;
    virtual void setFrameTypeAndDecoration( X11SalFrame* pFrame, WMWindowType eType, int nDecorationFlags, X11SalFrame* pTransientFrame ) const override;
    virtual void enableAlwaysOnTop( X11SalFrame* pFrame, bool bEnable ) const override;
    virtual int handlePropertyNotify( X11SalFrame* pFrame, XPropertyEvent* pEvent ) const override;
    virtual void showFullScreen( X11SalFrame* pFrame, bool bFullScreen ) const override;
    virtual void frameIsMapping( X11SalFrame* pFrame ) const override;
    virtual void setUserTime( X11SalFrame* i_pFrame, long i_nUserTime ) const override;
};

class GnomeWMAdaptor : public WMAdaptor
{
    bool m_bValid;

    void setGnomeWMState( X11SalFrame* pFrame ) const;
    void initAtoms();
    virtual bool isValid() const override;
public:
    explicit GnomeWMAdaptor( SalDisplay * );

    virtual void maximizeFrame( X11SalFrame* pFrame, bool bHorizontal = true, bool bVertical = true ) const override;
    virtual void shade( X11SalFrame* pFrame, bool bToShaded ) const override;
    virtual void enableAlwaysOnTop( X11SalFrame* pFrame, bool bEnable ) const override;
    virtual int handlePropertyNotify( X11SalFrame* pFrame, XPropertyEvent* pEvent ) const override;
};

}

using namespace vcl_sal;

struct WMAdaptorProtocol
{
    const char* pProtocol;
    int             nProtocol;
};

/*
 *  table must be sorted ascending in strings
 *  since it is use with bsearch
 */
static const WMAdaptorProtocol aProtocolTab[] =
{
    { "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE", WMAdaptor::KDE_NET_WM_WINDOW_TYPE_OVERRIDE },
    { "_NET_ACTIVE_WINDOW", WMAdaptor::NET_ACTIVE_WINDOW },
    { "_NET_CURRENT_DESKTOP", WMAdaptor::NET_CURRENT_DESKTOP },
    { "_NET_NUMBER_OF_DESKTOPS", WMAdaptor::NET_NUMBER_OF_DESKTOPS },
    { "_NET_WM_DESKTOP", WMAdaptor::NET_WM_DESKTOP },
    { "_NET_WM_ICON", WMAdaptor::NET_WM_ICON },
    { "_NET_WM_ICON_NAME", WMAdaptor::NET_WM_ICON_NAME },
    { "_NET_WM_PING", WMAdaptor::NET_WM_PING },
    { "_NET_WM_STATE", WMAdaptor::NET_WM_STATE },
    { "_NET_WM_STATE_ABOVE", WMAdaptor::NET_WM_STATE_STAYS_ON_TOP },
    { "_NET_WM_STATE_FULLSCREEN", WMAdaptor::NET_WM_STATE_FULLSCREEN },
    { "_NET_WM_STATE_MAXIMIZED_HORIZ", WMAdaptor::NET_WM_STATE_MAXIMIZED_HORZ }, // common bug in e.g. older kwin and sawfish implementations
    { "_NET_WM_STATE_MAXIMIZED_HORZ", WMAdaptor::NET_WM_STATE_MAXIMIZED_HORZ },
    { "_NET_WM_STATE_MAXIMIZED_VERT", WMAdaptor::NET_WM_STATE_MAXIMIZED_VERT },
    { "_NET_WM_STATE_MODAL", WMAdaptor::NET_WM_STATE_MODAL },
    { "_NET_WM_STATE_SHADED", WMAdaptor::NET_WM_STATE_SHADED },
    { "_NET_WM_STATE_SKIP_PAGER", WMAdaptor::NET_WM_STATE_SKIP_PAGER },
    { "_NET_WM_STATE_SKIP_TASKBAR", WMAdaptor::NET_WM_STATE_SKIP_TASKBAR },
    { "_NET_WM_STATE_STAYS_ON_TOP", WMAdaptor::NET_WM_STATE_STAYS_ON_TOP },
    { "_NET_WM_STATE_STICKY", WMAdaptor::NET_WM_STATE_STICKY },
    { "_NET_WM_STRUT", WMAdaptor::NET_WM_STRUT },
    { "_NET_WM_STRUT_PARTIAL", WMAdaptor::NET_WM_STRUT_PARTIAL },
    { "_NET_WM_WINDOW_TYPE", WMAdaptor::NET_WM_WINDOW_TYPE },
    { "_NET_WM_WINDOW_TYPE_DESKTOP", WMAdaptor::NET_WM_WINDOW_TYPE_DESKTOP },
    { "_NET_WM_WINDOW_TYPE_DIALOG", WMAdaptor::NET_WM_WINDOW_TYPE_DIALOG },
    { "_NET_WM_WINDOW_TYPE_DOCK", WMAdaptor::NET_WM_WINDOW_TYPE_DOCK },
    { "_NET_WM_WINDOW_TYPE_MENU", WMAdaptor::NET_WM_WINDOW_TYPE_MENU },
    { "_NET_WM_WINDOW_TYPE_NORMAL", WMAdaptor::NET_WM_WINDOW_TYPE_NORMAL },
    { "_NET_WM_WINDOW_TYPE_SPLASH", WMAdaptor::NET_WM_WINDOW_TYPE_SPLASH },
    { "_NET_WM_WINDOW_TYPE_SPLASHSCREEN", WMAdaptor::NET_WM_WINDOW_TYPE_SPLASH }, // bug in Metacity 2.4.1
    { "_NET_WM_WINDOW_TYPE_TOOLBAR", WMAdaptor::NET_WM_WINDOW_TYPE_TOOLBAR },
    { "_NET_WM_WINDOW_TYPE_UTILITY", WMAdaptor::NET_WM_WINDOW_TYPE_UTILITY },
    { "_NET_WORKAREA", WMAdaptor::NET_WORKAREA },
    { "_WIN_APP_STATE", WMAdaptor::WIN_APP_STATE },
    { "_WIN_CLIENT_LIST", WMAdaptor::WIN_CLIENT_LIST },
    { "_WIN_EXPANDED_SIZE", WMAdaptor::WIN_EXPANDED_SIZE },
    { "_WIN_HINTS", WMAdaptor::WIN_HINTS },
    { "_WIN_ICONS", WMAdaptor::WIN_ICONS },
    { "_WIN_LAYER", WMAdaptor::WIN_LAYER },
    { "_WIN_STATE", WMAdaptor::WIN_STATE },
    { "_WIN_WORKSPACE", WMAdaptor::WIN_WORKSPACE },
    { "_WIN_WORKSPACE_COUNT", WMAdaptor::WIN_WORKSPACE_COUNT }
};

/*
 *  table containing atoms to get anyway
 */

static const WMAdaptorProtocol aAtomTab[] =
{
    { "WM_STATE", WMAdaptor::WM_STATE },
    { "_MOTIF_WM_HINTS", WMAdaptor::MOTIF_WM_HINTS },
    { "WM_PROTOCOLS", WMAdaptor::WM_PROTOCOLS },
    { "WM_DELETE_WINDOW", WMAdaptor::WM_DELETE_WINDOW },
    { "WM_TAKE_FOCUS", WMAdaptor::WM_TAKE_FOCUS },
    { "WM_COMMAND", WMAdaptor::WM_COMMAND },
    { "WM_CLIENT_LEADER", WMAdaptor::WM_CLIENT_LEADER },
    { "WM_LOCALE_NAME", WMAdaptor::WM_LOCALE_NAME },
    { "WM_TRANSIENT_FOR", WMAdaptor::WM_TRANSIENT_FOR },
    { "SAL_QUITEVENT", WMAdaptor::SAL_QUITEVENT },
    { "SAL_USEREVENT", WMAdaptor::SAL_USEREVENT },
    { "SAL_EXTTEXTEVENT", WMAdaptor::SAL_EXTTEXTEVENT },
    { "SAL_GETTIMEEVENT", WMAdaptor::SAL_GETTIMEEVENT },
    { "VCL_SYSTEM_SETTINGS", WMAdaptor::VCL_SYSTEM_SETTINGS },
    { "_XSETTINGS_SETTINGS", WMAdaptor::XSETTINGS },
    { "_XEMBED", WMAdaptor::XEMBED },
    { "_XEMBED_INFO", WMAdaptor::XEMBED_INFO },
    { "_NET_WM_USER_TIME", WMAdaptor::NET_WM_USER_TIME },
    { "_NET_WM_PID", WMAdaptor::NET_WM_PID }
};

extern "C" {
static int compareProtocol( const void* pLeft, const void* pRight )
{
    return strcmp( static_cast<const WMAdaptorProtocol*>(pLeft)->pProtocol, static_cast<const WMAdaptorProtocol*>(pRight)->pProtocol );
}
}

std::unique_ptr<WMAdaptor> WMAdaptor::createWMAdaptor( SalDisplay* pSalDisplay )
{
    std::unique_ptr<WMAdaptor> pAdaptor;

    // try a NetWM
    pAdaptor.reset(new NetWMAdaptor( pSalDisplay ));
    if( ! pAdaptor->isValid() )
    {
        pAdaptor.reset();
    }
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "WM supports extended WM hints\n" );
#endif

    // try a GnomeWM
    if( ! pAdaptor )
    {
        pAdaptor.reset(new GnomeWMAdaptor( pSalDisplay ));
        if( ! pAdaptor->isValid() )
        {
            pAdaptor.reset();
        }
#if OSL_DEBUG_LEVEL > 1
        else
            fprintf( stderr, "WM supports GNOME WM hints\n" );
#endif
    }

    if( ! pAdaptor )
        pAdaptor.reset(new WMAdaptor( pSalDisplay ));

#if OSL_DEBUG_LEVEL > 1
    fprintf(stderr, "Window Manager's name is \"%s\"\n",
        OUStringToOString(pAdaptor->getWindowManagerName(),
        RTL_TEXTENCODING_UTF8).getStr());
#endif
    return pAdaptor;
}

/*
 *  WMAdaptor constructor
 */

WMAdaptor::WMAdaptor( SalDisplay* pDisplay ) :
        m_pSalDisplay( pDisplay ),
        m_bEnableAlwaysOnTopWorks( false ),
        m_bLegacyPartialFullscreen( false ),
        m_nWinGravity( StaticGravity ),
        m_nInitWinGravity( StaticGravity ),
        m_bWMshouldSwitchWorkspace( true ),
        m_bWMshouldSwitchWorkspaceInit( false )
{
    Atom                aRealType   = None;
    int                 nFormat     = 8;
    unsigned long       nItems      = 0;
    unsigned long       nBytesLeft  = 0;
    unsigned char*  pProperty   = nullptr;

    // default desktops
    m_nDesktops = 1;
    m_aWMWorkAreas = ::std::vector< tools::Rectangle >
        ( 1, tools::Rectangle( Point(), m_pSalDisplay->GetScreenSize( m_pSalDisplay->GetDefaultXScreen() ) ) );
    m_bEqualWorkAreas = true;

    memset( m_aWMAtoms, 0, sizeof( m_aWMAtoms ) );
    m_pDisplay = m_pSalDisplay->GetDisplay();

    initAtoms();
    getNetWmName(); // try to discover e.g. Sawfish

    if( m_aWMName.isEmpty() )
    {
        // check for ReflectionX wm (as it needs a workaround in Windows mode
        Atom aRwmRunning = XInternAtom( m_pDisplay, "RWM_RUNNING", True );
        if( aRwmRunning != None &&
            XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                aRwmRunning,
                                0, 32,
                                False,
                                aRwmRunning,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty ) == 0 )
        {
            if( aRealType == aRwmRunning )
                m_aWMName = "ReflectionX";
            XFree( pProperty );
        }
        else if( (aRwmRunning = XInternAtom( m_pDisplay, "_WRQ_WM_RUNNING", True )) != None &&
            XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                aRwmRunning,
                                0, 32,
                                False,
                                XA_STRING,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty ) == 0 )
        {
            if( aRealType == XA_STRING )
                m_aWMName = "ReflectionX Windows";
            XFree( pProperty );
        }
    }
    if( m_aWMName.isEmpty() )
    {
        Atom aTTAPlatform = XInternAtom( m_pDisplay, "TTA_CLIENT_PLATFORM", True );
        if( aTTAPlatform != None &&
            XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                aTTAPlatform,
                                0, 32,
                                False,
                                XA_STRING,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty ) == 0 )
        {
            if( aRealType == XA_STRING )
            {
                m_aWMName = "Tarantella";
                // #i62319# pretend that AlwaysOnTop works since
                // the alwaysontop workaround in salframe.cxx results
                // in a raise/lower loop on a Windows tarantella client
                // FIXME: this property contains an identification string that
                // in theory should be good enough to recognize running on a
                // Windows client; however this string does not seem to be
                // documented as well as the property itself.
                m_bEnableAlwaysOnTopWorks = true;
            }
            XFree( pProperty );
        }
    }
}

/*
 *  WMAdaptor destructor
 */

WMAdaptor::~WMAdaptor()
{
}

/*
 *  NetWMAdaptor constructor
 */

NetWMAdaptor::NetWMAdaptor( SalDisplay* pSalDisplay ) :
        WMAdaptor( pSalDisplay )
{
    // currently all _NET WMs do transient like expected

    Atom                aRealType   = None;
    int                 nFormat     = 8;
    unsigned long       nItems      = 0;
    unsigned long       nBytesLeft  = 0;
    unsigned char*  pProperty   = nullptr;

    initAtoms();

    // check for NetWM
    bool bNetWM = getNetWmName();
    if( bNetWM
        && XGetWindowProperty( m_pDisplay,
                               m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                               m_aWMAtoms[ NET_SUPPORTED ],
                               0, 0,
                               False,
                               XA_ATOM,
                               &aRealType,
                               &nFormat,
                               &nItems,
                               &nBytesLeft,
                               &pProperty ) == 0
        && aRealType == XA_ATOM
        && nFormat == 32
        )
    {
        if( pProperty )
        {
            XFree( pProperty );
            pProperty = nullptr;
        }
        // collect supported protocols
        if( XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                m_aWMAtoms[ NET_SUPPORTED ],
                                0, nBytesLeft/4,
                                False,
                                XA_ATOM,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty ) == 0
            && nItems
            )
        {
            Atom* pAtoms = reinterpret_cast<Atom*>(pProperty);
            char** pAtomNames = static_cast<char**>(alloca( sizeof(char*)*nItems ));
            if( XGetAtomNames( m_pDisplay, pAtoms, nItems, pAtomNames ) )
            {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "supported protocols:\n" );
#endif
                for( unsigned long i = 0; i < nItems; i++ )
                {
                    // #i80971# protect against invalid atoms
                    if( pAtomNames[i] == nullptr )
                        continue;

                    WMAdaptorProtocol aSearch;
                    aSearch.pProtocol = pAtomNames[i];
                    WMAdaptorProtocol* pMatch = static_cast<WMAdaptorProtocol*>(
                        bsearch( &aSearch,
                                 aProtocolTab,
                                 SAL_N_ELEMENTS( aProtocolTab ),
                                 sizeof( struct WMAdaptorProtocol ),
                                 compareProtocol ));
                    if( pMatch )
                    {
                        m_aWMAtoms[ pMatch->nProtocol ] = pAtoms[ i ];
                        if( pMatch->nProtocol == NET_WM_STATE_STAYS_ON_TOP )
                            m_bEnableAlwaysOnTopWorks = true;
                    }
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "  %s%s\n", pAtomNames[i], ((pMatch)&&(pMatch->nProtocol != -1)) ? "" : " (unsupported)" );
#endif
                    XFree( pAtomNames[i] );
                }
            }
            XFree( pProperty );
            pProperty = nullptr;
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = nullptr;
        }

        // get number of desktops
        if( m_aWMAtoms[ NET_NUMBER_OF_DESKTOPS ]
            && XGetWindowProperty( m_pDisplay,
                                   m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                   m_aWMAtoms[ NET_NUMBER_OF_DESKTOPS ],
                                   0, 1,
                                   False,
                                   XA_CARDINAL,
                                   &aRealType,
                                   &nFormat,
                                   &nItems,
                                   &nBytesLeft,
                                   &pProperty ) == 0
            && pProperty
            )
        {
            m_nDesktops = *reinterpret_cast<long*>(pProperty);
            XFree( pProperty );
            pProperty = nullptr;
            // get work areas
            if( m_aWMAtoms[ NET_WORKAREA ]
                && XGetWindowProperty( m_pDisplay,
                                       m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                       m_aWMAtoms[ NET_WORKAREA ],
                                       0, 4*m_nDesktops,
                                       False,
                                       XA_CARDINAL,
                                       &aRealType,
                                       &nFormat,
                                       &nItems,
                                       &nBytesLeft,
                                       &pProperty
                                       ) == 0
                && nItems == 4*static_cast<unsigned>(m_nDesktops)
                )
            {
                m_aWMWorkAreas = ::std::vector< tools::Rectangle > ( m_nDesktops );
                long* pValues = reinterpret_cast<long*>(pProperty);
                for( int i = 0; i < m_nDesktops; i++ )
                {
                    Point aPoint( pValues[4*i],
                                  pValues[4*i+1] );
                    Size aSize( pValues[4*i+2],
                                pValues[4*i+3] );
                    tools::Rectangle aWorkArea( aPoint, aSize );
                    m_aWMWorkAreas[i] = aWorkArea;
                    if( aWorkArea != m_aWMWorkAreas[0] )
                        m_bEqualWorkAreas = false;
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "workarea %d: %ldx%ld+%ld+%ld\n",
                             i,
                             m_aWMWorkAreas[i].GetWidth(),
                             m_aWMWorkAreas[i].GetHeight(),
                             m_aWMWorkAreas[i].Left(),
                             m_aWMWorkAreas[i].Top() );
#endif
                }
                XFree( pProperty );
            }
            else
            {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "%ld workareas for %d desktops !\n", nItems/4, m_nDesktops );
#endif
                if( pProperty )
                {
                    XFree(pProperty);
                    pProperty = nullptr;
                }
            }
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = nullptr;
        }
    }
    else if( pProperty )
    {
        XFree( pProperty );
        pProperty = nullptr;
    }
}

/*
 *  GnomeWMAdaptor constructor
 */

GnomeWMAdaptor::GnomeWMAdaptor( SalDisplay* pSalDisplay ) :
        WMAdaptor( pSalDisplay ),
        m_bValid( false )
{
    // currently all Gnome WMs do transient like expected

    Atom                aRealType   = None;
    int                 nFormat     = 8;
    unsigned long       nItems      = 0;
    unsigned long       nBytesLeft  = 0;
    unsigned char*  pProperty   = nullptr;

    initAtoms();

    // check for GnomeWM
    if( m_aWMAtoms[ WIN_SUPPORTING_WM_CHECK ] && m_aWMAtoms[ WIN_PROTOCOLS ] )
    {
        ::Window         aWMChild    = None;
        if( XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                m_aWMAtoms[ WIN_SUPPORTING_WM_CHECK ],
                                0, 1,
                                False,
                                XA_CARDINAL,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty ) == 0
            && aRealType == XA_CARDINAL
            && nFormat == 32
            && nItems != 0
            )
        {
            aWMChild = *reinterpret_cast< ::Window* >(pProperty);
            XFree( pProperty );
            pProperty = nullptr;
            ::Window aCheckWindow = None;
            GetGenericUnixSalData()->ErrorTrapPush();
            if( XGetWindowProperty( m_pDisplay,
                                    aWMChild,
                                    m_aWMAtoms[ WIN_SUPPORTING_WM_CHECK ],
                                    0, 1,
                                    False,
                                    XA_CARDINAL,
                                    &aRealType,
                                    &nFormat,
                                    &nItems,
                                    &nBytesLeft,
                                    &pProperty ) == 0
                && aRealType == XA_CARDINAL
                && nFormat == 32
                && nItems != 0 )
            {
                if (! GetGenericUnixSalData()->ErrorTrapPop( false ) )
                {
                    GetGenericUnixSalData()->ErrorTrapPush();

                    aCheckWindow =  *reinterpret_cast< ::Window* >(pProperty);
                    XFree( pProperty );
                    pProperty = nullptr;
                    if( aCheckWindow == aWMChild )
                    {
                        m_bValid = true;
                        /*
                         *  get name of WM
                         *  this is NOT part of the GNOME WM hints, but e.g. Sawfish
                         *  already supports this part of the extended WM hints
                         */
                        m_aWMAtoms[ UTF8_STRING ] = XInternAtom( m_pDisplay, "UTF8_STRING", False );
                        getNetWmName();
                    }
                }
                else
                    GetGenericUnixSalData()->ErrorTrapPush();
            }
            GetGenericUnixSalData()->ErrorTrapPop();
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = nullptr;
        }
    }
    if( m_bValid
        && XGetWindowProperty( m_pDisplay,
                               m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                               m_aWMAtoms[ WIN_PROTOCOLS ],
                               0, 0,
                               False,
                               XA_ATOM,
                               &aRealType,
                               &nFormat,
                               &nItems,
                               &nBytesLeft,
                               &pProperty ) == 0
        && aRealType == XA_ATOM
        && nFormat == 32
        )
    {
        if( pProperty )
        {
            XFree( pProperty );
            pProperty = nullptr;
        }
        // collect supported protocols
        if( XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                m_aWMAtoms[ WIN_PROTOCOLS ],
                                0, nBytesLeft/4,
                                False,
                                XA_ATOM,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty ) == 0
            && pProperty
            )
        {
            Atom* pAtoms = reinterpret_cast<Atom*>(pProperty);
            char** pAtomNames = static_cast<char**>(alloca( sizeof(char*)*nItems ));
            if( XGetAtomNames( m_pDisplay, pAtoms, nItems, pAtomNames ) )
            {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "supported protocols:\n" );
#endif
                for( unsigned long i = 0; i < nItems; i++ )
                {
                    // #i80971# protect against invalid atoms
                    if( pAtomNames[i] == nullptr )
                        continue;

                    WMAdaptorProtocol aSearch;
                    aSearch.pProtocol = pAtomNames[i];
                    WMAdaptorProtocol* pMatch = static_cast<WMAdaptorProtocol*>(
                        bsearch( &aSearch,
                                 aProtocolTab,
                                 SAL_N_ELEMENTS( aProtocolTab ),
                                 sizeof( struct WMAdaptorProtocol ),
                                 compareProtocol ));
                    if( pMatch )
                    {
                        m_aWMAtoms[ pMatch->nProtocol ] = pAtoms[ i ];
                        if( pMatch->nProtocol == WIN_LAYER )
                            m_bEnableAlwaysOnTopWorks = true;
                    }
                    if( strncmp( "_ICEWM_TRAY", pAtomNames[i], 11 ) == 0 )
                    {
                        m_aWMName = "IceWM";
                        m_nWinGravity = NorthWestGravity;
                        m_nInitWinGravity = NorthWestGravity;
                    }
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "  %s%s\n", pAtomNames[i], ((pMatch) && (pMatch->nProtocol != -1)) ? "" : " (unsupported)" );
#endif
                    XFree( pAtomNames[i] );
                }
            }
            XFree( pProperty );
            pProperty = nullptr;
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = nullptr;
        }

        // get number of desktops
        if( m_aWMAtoms[ WIN_WORKSPACE_COUNT ]
            && XGetWindowProperty( m_pDisplay,
                                   m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                   m_aWMAtoms[ WIN_WORKSPACE_COUNT ],
                                   0, 1,
                                   False,
                                   XA_CARDINAL,
                                   &aRealType,
                                   &nFormat,
                                   &nItems,
                                   &nBytesLeft,
                                   &pProperty ) == 0
            && pProperty
            )
        {
            m_nDesktops = *reinterpret_cast<long*>(pProperty);
            XFree( pProperty );
            pProperty = nullptr;
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = nullptr;
        }
    }
    else if( pProperty )
    {
        XFree( pProperty );
        pProperty = nullptr;
    }
}

/*
 *  getNetWmName()
 */
bool WMAdaptor::getNetWmName()
{
    Atom                aRealType   = None;
    int                 nFormat     = 8;
    unsigned long       nItems      = 0;
    unsigned long       nBytesLeft  = 0;
    unsigned char*  pProperty   = nullptr;
    bool                bNetWM      = false;

    if( m_aWMAtoms[ NET_SUPPORTING_WM_CHECK ] && m_aWMAtoms[ NET_WM_NAME ] )
    {
        ::Window         aWMChild = None;
        if( XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                m_aWMAtoms[ NET_SUPPORTING_WM_CHECK ],
                                0, 1,
                                False,
                                XA_WINDOW,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty ) == 0
            && aRealType == XA_WINDOW
            && nFormat == 32
            && nItems != 0
            )
        {
            aWMChild = *reinterpret_cast< ::Window* >(pProperty);
            XFree( pProperty );
            pProperty = nullptr;
            ::Window aCheckWindow = None;
            GetGenericUnixSalData()->ErrorTrapPush();
            if( XGetWindowProperty( m_pDisplay,
                                    aWMChild,
                                    m_aWMAtoms[ NET_SUPPORTING_WM_CHECK ],
                                    0, 1,
                                    False,
                                    XA_WINDOW,
                                    &aRealType,
                                    &nFormat,
                                    &nItems,
                                    &nBytesLeft,
                                    &pProperty ) == 0
                && aRealType == XA_WINDOW
                && nFormat == 32
                && nItems != 0 )
                {
                    if ( ! GetGenericUnixSalData()->ErrorTrapPop( false ) )
                    {
                        GetGenericUnixSalData()->ErrorTrapPush();
                        aCheckWindow =  *reinterpret_cast< ::Window* >(pProperty);
                        XFree( pProperty );
                        pProperty = nullptr;
                        if( aCheckWindow == aWMChild )
                        {
                            bNetWM = true;
                            // get name of WM
                            m_aWMAtoms[ UTF8_STRING ] = XInternAtom( m_pDisplay, "UTF8_STRING", False );
                            if( XGetWindowProperty( m_pDisplay,
                                                    aWMChild,
                                                    m_aWMAtoms[ NET_WM_NAME ],
                                                    0, 256,
                                                    False,
                                                    AnyPropertyType, /* m_aWMAtoms[ UTF8_STRING ],*/
                                                    &aRealType,
                                                    &nFormat,
                                                    &nItems,
                                                    &nBytesLeft,
                                                    &pProperty ) == 0
                                && nItems != 0
                                )
                            {
                                if (aRealType == m_aWMAtoms[ UTF8_STRING ])
                                    m_aWMName = OUString( reinterpret_cast<char*>(pProperty), nItems, RTL_TEXTENCODING_UTF8 );
                                else if (aRealType == XA_STRING)
                                    m_aWMName = OUString( reinterpret_cast<char*>(pProperty), nItems, RTL_TEXTENCODING_ISO_8859_1 );

                                XFree( pProperty );
                                pProperty = nullptr;
                            }
                            else if( pProperty )
                            {
                                XFree( pProperty );
                                pProperty = nullptr;
                            }

                            // if this is metacity, check for version to enable a legacy workaround
                            if( m_aWMName == "Metacity" )
                            {
                                int nVersionMajor = 0, nVersionMinor = 0;
                                Atom nVersionAtom = XInternAtom( m_pDisplay, "_METACITY_VERSION", True );
                                if( nVersionAtom )
                                {
                                    if( XGetWindowProperty( m_pDisplay,
                                                            aWMChild,
                                                            nVersionAtom,
                                                            0, 256,
                                                            False,
                                                            m_aWMAtoms[ UTF8_STRING ],
                                                            &aRealType,
                                                            &nFormat,
                                                            &nItems,
                                                            &nBytesLeft,
                                                            &pProperty ) == 0
                                        && nItems != 0
                                        )
                                    {
                                        OUString aMetaVersion( reinterpret_cast<char*>(pProperty), nItems, RTL_TEXTENCODING_UTF8 );
                                        sal_Int32 nIdx {0};
                                        nVersionMajor = aMetaVersion.getToken(0, '.', nIdx).toInt32();
                                        nVersionMinor = aMetaVersion.getToken(0, '.', nIdx).toInt32();
                                    }
                                    if( pProperty )
                                    {
                                        XFree( pProperty );
                                        pProperty = nullptr;
                                    }
                                }
                                if( nVersionMajor < 2 || (nVersionMajor == 2 && nVersionMinor < 12) )
                                    m_bLegacyPartialFullscreen = true;
                            }
                        }
                    }
                    else
                    {
                        if( pProperty )
                        {
                            XFree( pProperty );
                            pProperty = nullptr;
                        }
                        GetGenericUnixSalData()->ErrorTrapPush();
                    }
                }

            GetGenericUnixSalData()->ErrorTrapPop();
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = nullptr;
        }
    }
    return bNetWM;
}

bool WMAdaptor::getWMshouldSwitchWorkspace() const
{
    if( ! m_bWMshouldSwitchWorkspaceInit )
    {
        WMAdaptor * pWMA = const_cast<WMAdaptor*>(this);

        pWMA->m_bWMshouldSwitchWorkspace = true;
        vcl::SettingsConfigItem* pItem = vcl::SettingsConfigItem::get();
        OUString aSetting( pItem->getValue( "WM",
                                                 "ShouldSwitchWorkspace" ) );
        if( aSetting.isEmpty() )
        {
            if( m_aWMName == "awesome" )
            {
                pWMA->m_bWMshouldSwitchWorkspace = false;
            }
        }
        else
            pWMA->m_bWMshouldSwitchWorkspace = aSetting.toBoolean();
        pWMA->m_bWMshouldSwitchWorkspaceInit = true;
    }
    return m_bWMshouldSwitchWorkspace;
}

/*
 *  WMAdaptor::isValid()
 */
bool WMAdaptor::isValid() const
{
    return true;
}

/*
 *  NetWMAdaptor::isValid()
 */
bool NetWMAdaptor::isValid() const
{
    // some necessary sanity checks; there are WMs out there
    // which implement some of the WM hints spec without
    // real functionality
    return
        m_aWMAtoms[ NET_SUPPORTED ]
        && m_aWMAtoms[ NET_SUPPORTING_WM_CHECK ]
        && m_aWMAtoms[ NET_WM_NAME ]
        && m_aWMAtoms[ NET_WM_WINDOW_TYPE_NORMAL ]
        && m_aWMAtoms[ NET_WM_WINDOW_TYPE_DIALOG ]
        ;
}

/*
 *  GnomeWMAdaptor::isValid()
 */
bool GnomeWMAdaptor::isValid() const
{
    return m_bValid;
}

/*
 *  WMAdaptor::initAtoms
 */

void WMAdaptor::initAtoms()
{
    // get basic atoms
    for(const WMAdaptorProtocol & i : aAtomTab)
        m_aWMAtoms[ i.nProtocol ] = XInternAtom( m_pDisplay, i.pProtocol, False );
    m_aWMAtoms[ NET_SUPPORTING_WM_CHECK ]   = XInternAtom( m_pDisplay, "_NET_SUPPORTING_WM_CHECK", True );
    m_aWMAtoms[ NET_WM_NAME ]               = XInternAtom( m_pDisplay, "_NET_WM_NAME", True );
}

/*
 *  NetWMAdaptor::initAtoms
 */

void NetWMAdaptor::initAtoms()
{
    WMAdaptor::initAtoms();

    m_aWMAtoms[ NET_SUPPORTED ]             = XInternAtom( m_pDisplay, "_NET_SUPPORTED", True );
}

/*
 *  GnomeWMAdaptor::initAtoms
 */

void GnomeWMAdaptor::initAtoms()
{
    WMAdaptor::initAtoms();

    m_aWMAtoms[ WIN_PROTOCOLS ]             = XInternAtom( m_pDisplay, "_WIN_PROTOCOLS", True );
    m_aWMAtoms[ WIN_SUPPORTING_WM_CHECK ]   = XInternAtom( m_pDisplay, "_WIN_SUPPORTING_WM_CHECK", True );
}

/*
 *  WMAdaptor::setWMName
 *  sets WM_NAME
 *       WM_ICON_NAME
 */

void WMAdaptor::setWMName( X11SalFrame* pFrame, const OUString& rWMName ) const
{
    OString aTitle(OUStringToOString(rWMName,
        osl_getThreadTextEncoding()));

    OString aWMLocale;
    rtl_Locale* pLocale = nullptr;
    osl_getProcessLocale( &pLocale );
    if( pLocale )
    {
        OUString aLocaleString( LanguageTag( *pLocale).getGlibcLocaleString( OUString()));
        aWMLocale = OUStringToOString( aLocaleString, RTL_TEXTENCODING_ISO_8859_1 );
    }
    else
    {
        static const char* pLang = getenv( "LANG" );
        aWMLocale = pLang ? pLang : "C";
    }

    char* pT = const_cast<char*>(aTitle.getStr());
    XTextProperty aProp = { nullptr, None, 0, 0 };
    XmbTextListToTextProperty( m_pDisplay,
                               &pT,
                               1,
                               XStdICCTextStyle,
                               &aProp );

    unsigned char const * pData = aProp.nitems ? aProp.value : reinterpret_cast<unsigned char const *>(aTitle.getStr());
    Atom nType = aProp.nitems ? aProp.encoding : XA_STRING;
    int nFormat = aProp.nitems ? aProp.format : 8;
    int nBytes = aProp.nitems ? aProp.nitems : aTitle.getLength();
    const SystemEnvData* pEnv = pFrame->GetSystemData();
    XChangeProperty( m_pDisplay,
                     static_cast<::Window>(pEnv->aShellWindow),
                     XA_WM_NAME,
                     nType,
                     nFormat,
                     PropModeReplace,
                     pData,
                     nBytes );
    XChangeProperty( m_pDisplay,
                     static_cast<::Window>(pEnv->aShellWindow),
                     XA_WM_ICON_NAME,
                     nType,
                     nFormat,
                     PropModeReplace,
                     pData,
                     nBytes );
    XChangeProperty( m_pDisplay,
                     static_cast<::Window>(pEnv->aShellWindow),
                     m_aWMAtoms[ WM_LOCALE_NAME ],
                     XA_STRING,
                     8,
                     PropModeReplace,
                     reinterpret_cast<unsigned char const *>(aWMLocale.getStr()),
                     aWMLocale.getLength() );
    if (aProp.value != nullptr)
        XFree( aProp.value );
}

/*
 *  NetWMAdaptor::setWMName
 *  sets WM_NAME
 *       _NET_WM_NAME
 *       WM_ICON_NAME
 *       _NET_WM_ICON_NAME
 */
void NetWMAdaptor::setWMName( X11SalFrame* pFrame, const OUString& rWMName ) const
{
    WMAdaptor::setWMName( pFrame, rWMName );

    OString aTitle(OUStringToOString(rWMName, RTL_TEXTENCODING_UTF8));
    const SystemEnvData* pEnv = pFrame->GetSystemData();
    if( m_aWMAtoms[ NET_WM_NAME ] )
        XChangeProperty( m_pDisplay,
                         static_cast<::Window>(pEnv->aShellWindow),
                         m_aWMAtoms[ NET_WM_NAME ],
                         m_aWMAtoms[ UTF8_STRING ],
                         8,
                         PropModeReplace,
                         reinterpret_cast<unsigned char const *>(aTitle.getStr()),
                         aTitle.getLength() );
    if( m_aWMAtoms[ NET_WM_ICON_NAME ] )
        XChangeProperty( m_pDisplay,
                         static_cast<::Window>(pEnv->aShellWindow),
                         m_aWMAtoms[ NET_WM_ICON_NAME ],
                         m_aWMAtoms[ UTF8_STRING ],
                         8,
                         PropModeReplace,
                         reinterpret_cast<unsigned char const *>(aTitle.getStr()),
                         aTitle.getLength() );
}

/*
 *  NetWMAdaptor::setNetWMState
 *  sets _NET_WM_STATE
 */
void NetWMAdaptor::setNetWMState( X11SalFrame* pFrame ) const
{
    if( m_aWMAtoms[ NET_WM_STATE ] )
    {
        Atom aStateAtoms[ 10 ];
        int nStateAtoms = 0;

        // set NET_WM_STATE_MODAL
        if( pFrame->mbMaximizedVert
            && m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_VERT ] )
            aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_VERT ];
        if( pFrame->mbMaximizedHorz
            && m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_HORZ ] )
            aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_HORZ ];
        if( pFrame->bAlwaysOnTop_ && m_aWMAtoms[ NET_WM_STATE_STAYS_ON_TOP ] )
            aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_STAYS_ON_TOP ];
        if( pFrame->mbShaded && m_aWMAtoms[ NET_WM_STATE_SHADED ] )
            aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_SHADED ];
        if( pFrame->mbFullScreen && m_aWMAtoms[ NET_WM_STATE_FULLSCREEN ] )
            aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_FULLSCREEN ];
        if( pFrame->meWindowType == WMWindowType::Utility && m_aWMAtoms[ NET_WM_STATE_SKIP_TASKBAR ] )
            aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_SKIP_TASKBAR ];

        if( nStateAtoms )
        {
            XChangeProperty( m_pDisplay,
                             pFrame->GetShellWindow(),
                             m_aWMAtoms[ NET_WM_STATE ],
                             XA_ATOM,
                             32,
                             PropModeReplace,
                             reinterpret_cast<unsigned char*>(aStateAtoms),
                             nStateAtoms
                             );
        }
        else
            XDeleteProperty( m_pDisplay,
                             pFrame->GetShellWindow(),
                             m_aWMAtoms[ NET_WM_STATE ] );
        if( pFrame->mbMaximizedHorz
           && pFrame->mbMaximizedVert
           && ! ( pFrame->nStyle_ & SalFrameStyleFlags::SIZEABLE ) )
        {
            /*
             *  for maximizing use NorthWestGravity (including decoration)
             */
            XSizeHints  hints;
            long        supplied;
            bool bHint = false;
            if( XGetWMNormalHints( m_pDisplay,
                                   pFrame->GetShellWindow(),
                                   &hints,
                                   &supplied ) )
            {
                bHint = true;
                hints.flags |= PWinGravity;
                hints.win_gravity = NorthWestGravity;
                XSetWMNormalHints( m_pDisplay,
                                   pFrame->GetShellWindow(),
                                   &hints );
                XSync( m_pDisplay, False );
            }

            // SetPosSize necessary to set width/height, min/max w/h
            sal_Int32 nCurrent = 0;
            /*
             *  get current desktop here if work areas have different size
             *  (does this happen on any platform ?)
             */
            if( ! m_bEqualWorkAreas )
            {
                nCurrent = getCurrentWorkArea();
                if( nCurrent < 0 )
                    nCurrent = 0;
            }
            tools::Rectangle aPosSize = m_aWMWorkAreas[nCurrent];
            const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
            aPosSize = tools::Rectangle( Point( aPosSize.Left() + rGeom.nLeftDecoration,
                                         aPosSize.Top()  + rGeom.nTopDecoration ),
                                  Size( aPosSize.GetWidth()
                                        - rGeom.nLeftDecoration
                                        - rGeom.nRightDecoration,
                                        aPosSize.GetHeight()
                                        - rGeom.nTopDecoration
                                        - rGeom.nBottomDecoration )
                                  );
            pFrame->SetPosSize( aPosSize );

            /*
             *  reset gravity hint to static gravity
             *  (this should not move window according to ICCCM)
             */
            if( bHint && pFrame->nShowState_ != SHOWSTATE_UNKNOWN )
            {
                hints.win_gravity = StaticGravity;
                XSetWMNormalHints( m_pDisplay,
                                   pFrame->GetShellWindow(),
                                   &hints );
            }
        }
    }
}

/*
 *  GnomeWMAdaptor::setNetWMState
 *  sets _WIN_STATE
 */
void GnomeWMAdaptor::setGnomeWMState( X11SalFrame* pFrame ) const
{
    if( m_aWMAtoms[ WIN_STATE ] )
    {
        sal_uInt32 nWinWMState = 0;

        if( pFrame->mbMaximizedVert )
            nWinWMState |= 1 << 2;
        if( pFrame->mbMaximizedHorz )
            nWinWMState |= 1 << 3;
        if( pFrame->mbShaded )
            nWinWMState |= 1 << 5;

        XChangeProperty( m_pDisplay,
                         pFrame->GetShellWindow(),
                         m_aWMAtoms[ WIN_STATE ],
                         XA_CARDINAL,
                         32,
                         PropModeReplace,
                         reinterpret_cast<unsigned char*>(&nWinWMState),
                         1
                         );
        if( pFrame->mbMaximizedHorz
           && pFrame->mbMaximizedVert
           && ! ( pFrame->nStyle_ & SalFrameStyleFlags::SIZEABLE ) )
        {
            /*
             *  for maximizing use NorthWestGravity (including decoration)
             */
            XSizeHints  hints;
            long        supplied;
            bool bHint = false;
            if( XGetWMNormalHints( m_pDisplay,
                                   pFrame->GetShellWindow(),
                                   &hints,
                                   &supplied ) )
            {
                bHint = true;
                hints.flags |= PWinGravity;
                hints.win_gravity = NorthWestGravity;
                XSetWMNormalHints( m_pDisplay,
                                   pFrame->GetShellWindow(),
                                   &hints );
                XSync( m_pDisplay, False );
            }

            // SetPosSize necessary to set width/height, min/max w/h
            sal_Int32 nCurrent = 0;
            /*
             *  get current desktop here if work areas have different size
             *  (does this happen on any platform ?)
             */
            if( ! m_bEqualWorkAreas )
            {
                nCurrent = getCurrentWorkArea();
                if( nCurrent < 0 )
                    nCurrent = 0;
            }
            tools::Rectangle aPosSize = m_aWMWorkAreas[nCurrent];
            const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
            aPosSize = tools::Rectangle( Point( aPosSize.Left() + rGeom.nLeftDecoration,
                                         aPosSize.Top()  + rGeom.nTopDecoration ),
                                  Size( aPosSize.GetWidth()
                                        - rGeom.nLeftDecoration
                                        - rGeom.nRightDecoration,
                                        aPosSize.GetHeight()
                                        - rGeom.nTopDecoration
                                        - rGeom.nBottomDecoration )
                                  );
            pFrame->SetPosSize( aPosSize );

            /*
             *  reset gravity hint to static gravity
             *  (this should not move window according to ICCCM)
             */
            if( bHint && pFrame->nShowState_ != SHOWSTATE_UNKNOWN )
            {
                hints.win_gravity = StaticGravity;
                XSetWMNormalHints( m_pDisplay,
                                   pFrame->GetShellWindow(),
                                   &hints );
            }
        }
    }
}

/*
 *  WMAdaptor::setFrameDecoration
 *  sets _MOTIF_WM_HINTS
 *       WM_TRANSIENT_FOR
 */

void WMAdaptor::setFrameTypeAndDecoration( X11SalFrame* pFrame, WMWindowType eType, int nDecorationFlags, X11SalFrame* pReferenceFrame ) const
{
    pFrame->meWindowType        = eType;

    if( ! pFrame->mbFullScreen )
    {
        // set mwm hints
        struct _mwmhints {
            unsigned long flags, func, deco;
            long input_mode;
            unsigned long status;
        } aHint;

        aHint.flags = 15; /* flags for functions, decoration, input mode and status */
        aHint.deco = 0;
        aHint.func = 1 << 2;
        aHint.status = 0;
        aHint.input_mode = 0;

        // evaluate decoration flags
        if( nDecorationFlags & decoration_All )
        {
            aHint.deco = 1;
            aHint.func = 1;
        }
        else
        {
            if( nDecorationFlags & decoration_Title )
                aHint.deco |= 1 << 3;
            if( nDecorationFlags & decoration_Border )
                aHint.deco |= 1 << 1;
            if( nDecorationFlags & decoration_Resize )
            {
                aHint.deco |= 1 << 2;
                aHint.func |= 1 << 1;
            }
            if( nDecorationFlags & decoration_MinimizeBtn )
            {
                aHint.deco |= 1 << 5;
                aHint.func |= 1 << 3;
            }
            if( nDecorationFlags & decoration_MaximizeBtn )
            {
                aHint.deco |= 1 << 6;
                aHint.func |= 1 << 4;
            }
            if( nDecorationFlags & decoration_CloseBtn )
            {
                aHint.deco |= 1 << 4;
                aHint.func |= 1 << 5;
            }
        }

        // set the hint
        XChangeProperty( m_pDisplay,
                         pFrame->GetShellWindow(),
                         m_aWMAtoms[ MOTIF_WM_HINTS ],
                         m_aWMAtoms[ MOTIF_WM_HINTS ],
                         32,
                         PropModeReplace,
                         reinterpret_cast<unsigned char*>(&aHint),
                         5 );
    }

    // set transientFor hint
    /*  #91030# dtwm will not map a dialogue if the transient
     *  window is iconified. This is deemed undesirable because
     *  message boxes do not get mapped, so use the root as transient
     *  instead.
     */
    if( pReferenceFrame )
    {
        XSetTransientForHint( m_pDisplay,
                              pFrame->GetShellWindow(),
                              pReferenceFrame->bMapped_ ?
                              pReferenceFrame->GetShellWindow() :
                              m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() )
                              );
        if( ! pReferenceFrame->bMapped_ )
            pFrame->mbTransientForRoot = true;
    }
}

/*
 *  NetWMAdaptor::setFrameDecoration
 *  sets _MOTIF_WM_HINTS
 *       _NET_WM_WINDOW_TYPE
 *       _NET_WM_STATE
 *       WM_TRANSIENT_FOR
 */

void NetWMAdaptor::setFrameTypeAndDecoration( X11SalFrame* pFrame, WMWindowType eType, int nDecorationFlags, X11SalFrame* pReferenceFrame ) const
{
    WMAdaptor::setFrameTypeAndDecoration( pFrame, eType, nDecorationFlags, pReferenceFrame );

    setNetWMState( pFrame );

    // set NET_WM_WINDOW_TYPE
    if( m_aWMAtoms[ NET_WM_WINDOW_TYPE ] )
    {
        Atom aWindowTypes[4];
        int nWindowTypes = 0;
        switch( eType )
        {
            case WMWindowType::Utility:
                aWindowTypes[nWindowTypes++] =
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_UTILITY ] ?
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_UTILITY ] :
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_DIALOG ];
                break;
            case WMWindowType::ModelessDialogue:
                aWindowTypes[nWindowTypes++] =
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_DIALOG ];
                break;
            case WMWindowType::Splash:
                aWindowTypes[nWindowTypes++] =
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_SPLASH ] ?
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_SPLASH ] :
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_NORMAL ];
                break;
            case WMWindowType::Toolbar:
                if( m_aWMAtoms[ KDE_NET_WM_WINDOW_TYPE_OVERRIDE ] )
                    aWindowTypes[nWindowTypes++] = m_aWMAtoms[ KDE_NET_WM_WINDOW_TYPE_OVERRIDE ];
                aWindowTypes[nWindowTypes++] =
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_TOOLBAR ] ?
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_TOOLBAR ] :
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_NORMAL];
                break;
            case WMWindowType::Dock:
                aWindowTypes[nWindowTypes++] =
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_DOCK ] ?
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_DOCK ] :
                    m_aWMAtoms[ NET_WM_WINDOW_TYPE_NORMAL];
                break;
            default:
                aWindowTypes[nWindowTypes++] = m_aWMAtoms[ NET_WM_WINDOW_TYPE_NORMAL ];
                break;
        }
        XChangeProperty( m_pDisplay,
                         pFrame->GetShellWindow(),
                         m_aWMAtoms[ NET_WM_WINDOW_TYPE ],
                         XA_ATOM,
                         32,
                         PropModeReplace,
                         reinterpret_cast<unsigned char*>(aWindowTypes),
                         nWindowTypes );
    }
    if( ( eType == WMWindowType::ModelessDialogue )
        && ! pReferenceFrame )
    {
        XSetTransientForHint( m_pDisplay,
                              pFrame->GetShellWindow(),
                              m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ) );
        pFrame->mbTransientForRoot = true;
    }
}

/*
 *  WMAdaptor::maximizeFrame
 */

void WMAdaptor::maximizeFrame( X11SalFrame* pFrame, bool bHorizontal, bool bVertical ) const
{
    pFrame->mbMaximizedVert = bVertical;
    pFrame->mbMaximizedHorz = bHorizontal;

    const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );

    // discard pending configure notifies for this frame
    XSync( m_pDisplay, False );
    XEvent aDiscard;
    while( XCheckTypedWindowEvent( m_pDisplay,
                                   pFrame->GetShellWindow(),
                                   ConfigureNotify,
                                   &aDiscard ) )
        ;
    while( XCheckTypedWindowEvent( m_pDisplay,
                                   pFrame->GetWindow(),
                                   ConfigureNotify,
                                   &aDiscard ) )
        ;

    if( bHorizontal || bVertical )
    {
        Size aScreenSize( m_pSalDisplay->GetScreenSize( pFrame->GetScreenNumber() ) );
        Point aTL( rGeom.nLeftDecoration, rGeom.nTopDecoration );
        if( m_pSalDisplay->IsXinerama() )
        {
            Point aMed( aTL.X() + rGeom.nWidth/2, aTL.Y() + rGeom.nHeight/2 );
            const std::vector< tools::Rectangle >& rScreens = m_pSalDisplay->GetXineramaScreens();
            for(const auto & rScreen : rScreens)
                if( rScreen.IsInside( aMed ) )
                {
                    aTL += rScreen.TopLeft();
                    aScreenSize = rScreen.GetSize();
                    break;
                }
        }
        tools::Rectangle aTarget( aTL,
                           Size( aScreenSize.Width() - rGeom.nLeftDecoration - rGeom.nTopDecoration,
                                 aScreenSize.Height() - rGeom.nTopDecoration - rGeom.nBottomDecoration )
                           );
        if( ! bHorizontal )
        {
            aTarget.SetSize(
                            Size(
                                 pFrame->maRestorePosSize.IsEmpty() ?
                                 rGeom.nWidth : pFrame->maRestorePosSize.GetWidth(),
                                 aTarget.GetHeight()
                                 )
                            );
            aTarget.SetLeft(
                pFrame->maRestorePosSize.IsEmpty() ?
                rGeom.nX : pFrame->maRestorePosSize.Left() );
        }
        else if( ! bVertical )
        {
            aTarget.SetSize(
                            Size(
                                 aTarget.GetWidth(),
                                 pFrame->maRestorePosSize.IsEmpty() ?
                                 rGeom.nHeight : pFrame->maRestorePosSize.GetHeight()
                                 )
                            );
            aTarget.SetTop(
                pFrame->maRestorePosSize.IsEmpty() ?
                rGeom.nY : pFrame->maRestorePosSize.Top() );
        }

        tools::Rectangle aRestore( Point( rGeom.nX, rGeom.nY ), Size( rGeom.nWidth, rGeom.nHeight ) );
        if( pFrame->bMapped_ )
        {
            XSetInputFocus( m_pDisplay,
                            pFrame->GetShellWindow(),
                            RevertToNone,
                            CurrentTime
                            );
        }

        if( pFrame->maRestorePosSize.IsEmpty() )
            pFrame->maRestorePosSize = aRestore;

        pFrame->SetPosSize( aTarget );
        pFrame->nWidth_     = aTarget.GetWidth();
        pFrame->nHeight_    = aTarget.GetHeight();
        XRaiseWindow( m_pDisplay,
                      pFrame->GetShellWindow()
                      );
        if( pFrame->GetStackingWindow() )
            XRaiseWindow( m_pDisplay,
                          pFrame->GetStackingWindow()
                          );

    }
    else
    {
        pFrame->SetPosSize( pFrame->maRestorePosSize );
        pFrame->maRestorePosSize = tools::Rectangle();
        pFrame->nWidth_             = rGeom.nWidth;
        pFrame->nHeight_            = rGeom.nHeight;
    }
}

/*
 *  NetWMAdaptor::maximizeFrame
 *  changes _NET_WM_STATE by sending a client message
 */

void NetWMAdaptor::maximizeFrame( X11SalFrame* pFrame, bool bHorizontal, bool bVertical ) const
{
    pFrame->mbMaximizedVert = bVertical;
    pFrame->mbMaximizedHorz = bHorizontal;

    if( m_aWMAtoms[ NET_WM_STATE ]
        && m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_VERT ]
        && m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_HORZ ]
        && ( pFrame->nStyle_ & ~SalFrameStyleFlags::DEFAULT )
        )
    {
        if( pFrame->bMapped_ )
        {
            // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->GetShellWindow();
            aEvent.xclient.message_type = m_aWMAtoms[ NET_WM_STATE ];
            aEvent.xclient.format       = 32;
            aEvent.xclient.data.l[0]    = bHorizontal ? 1 : 0;
            aEvent.xclient.data.l[1]    = m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_HORZ ];
            aEvent.xclient.data.l[2]    = bHorizontal == bVertical ? m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_VERT ] : 0;
            aEvent.xclient.data.l[3]    = 0;
            aEvent.xclient.data.l[4]    = 0;
            XSendEvent( m_pDisplay,
                        m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ),
                        False,
                        SubstructureNotifyMask | SubstructureRedirectMask,
                        &aEvent
                        );
            if( bHorizontal != bVertical )
            {
                aEvent.xclient.data.l[0]= bVertical ? 1 : 0;
                aEvent.xclient.data.l[1]= m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_VERT ];
                aEvent.xclient.data.l[2]= 0;
                XSendEvent( m_pDisplay,
                            m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ),
                            False,
                            SubstructureNotifyMask | SubstructureRedirectMask,
                            &aEvent
                            );
            }
        }
        else
        {
            // window not mapped yet, set _NET_WM_STATE directly
            setNetWMState( pFrame );
        }
        if( !bHorizontal && !bVertical )
            pFrame->maRestorePosSize = tools::Rectangle();
        else if( pFrame->maRestorePosSize.IsEmpty() )
        {
            const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
            pFrame->maRestorePosSize =
                tools::Rectangle( Point( rGeom.nX, rGeom.nY ), Size( rGeom.nWidth, rGeom.nHeight ) );
        }
    }
    else
        WMAdaptor::maximizeFrame( pFrame, bHorizontal, bVertical );
}

/*
 *  GnomeWMAdaptor::maximizeFrame
 *  changes _WIN_STATE by sending a client message
 */

void GnomeWMAdaptor::maximizeFrame( X11SalFrame* pFrame, bool bHorizontal, bool bVertical ) const
{
    pFrame->mbMaximizedVert = bVertical;
    pFrame->mbMaximizedHorz = bHorizontal;

    if( m_aWMAtoms[ WIN_STATE ]
        && ( pFrame->nStyle_ & ~SalFrameStyleFlags::DEFAULT )
        )
    {
        if( pFrame->bMapped_ )
        {
             // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->GetShellWindow();
            aEvent.xclient.message_type = m_aWMAtoms[ WIN_STATE ];
            aEvent.xclient.format       = 32;
            aEvent.xclient.data.l[0]    = (1<<2)|(1<<3);
            aEvent.xclient.data.l[1]    =
                (bVertical ? (1<<2) : 0)
                | (bHorizontal ? (1<<3) : 0);
            aEvent.xclient.data.l[2]    = 0;
            aEvent.xclient.data.l[3]    = 0;
            aEvent.xclient.data.l[4]    = 0;
            XSendEvent( m_pDisplay,
                        m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ),
                        False,
                        SubstructureNotifyMask,
                        &aEvent
                        );
        }
        else
            // window not mapped yet, set _WIN_STATE directly
            setGnomeWMState( pFrame );

        if( !bHorizontal && !bVertical )
            pFrame->maRestorePosSize = tools::Rectangle();
        else if( pFrame->maRestorePosSize.IsEmpty() )
        {
            const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
            pFrame->maRestorePosSize =
                tools::Rectangle( Point( rGeom.nX, rGeom.nY ), Size( rGeom.nWidth, rGeom.nHeight ) );
        }
    }
    else
        WMAdaptor::maximizeFrame( pFrame, bHorizontal, bVertical );
}

/*
 *  WMAdaptor::enableAlwaysOnTop
 */
void WMAdaptor::enableAlwaysOnTop( X11SalFrame*, bool /*bEnable*/ ) const
{
}

/*
 *  NetWMAdaptor::enableAlwaysOnTop
 */
void NetWMAdaptor::enableAlwaysOnTop( X11SalFrame* pFrame, bool bEnable ) const
{
    pFrame->bAlwaysOnTop_ = bEnable;
    if( m_aWMAtoms[ NET_WM_STATE_STAYS_ON_TOP ] )
    {
        if( pFrame->bMapped_ )
        {
            // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->GetShellWindow();
            aEvent.xclient.message_type = m_aWMAtoms[ NET_WM_STATE ];
            aEvent.xclient.format       = 32;
            aEvent.xclient.data.l[0]    = bEnable ? 1 : 0;
            aEvent.xclient.data.l[1]    = m_aWMAtoms[ NET_WM_STATE_STAYS_ON_TOP ];
            aEvent.xclient.data.l[2]    = 0;
            aEvent.xclient.data.l[3]    = 0;
            aEvent.xclient.data.l[4]    = 0;
            XSendEvent( m_pDisplay,
                        m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ),
                        False,
                        SubstructureNotifyMask | SubstructureRedirectMask,
                        &aEvent
                        );
        }
        else
            setNetWMState( pFrame );
    }
}

/*
 *  GnomeWMAdaptor::enableAlwaysOnTop
 */
void GnomeWMAdaptor::enableAlwaysOnTop( X11SalFrame* pFrame, bool bEnable ) const
{
    pFrame->bAlwaysOnTop_ = bEnable;
    if( m_aWMAtoms[ WIN_LAYER ] )
    {
        if( pFrame->bMapped_ )
        {
            // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->GetShellWindow();
            aEvent.xclient.message_type = m_aWMAtoms[ WIN_LAYER ];
            aEvent.xclient.format       = 32;
            aEvent.xclient.data.l[0]    = bEnable ? 6 : 4;
            aEvent.xclient.data.l[1]    = 0;
            aEvent.xclient.data.l[2]    = 0;
            aEvent.xclient.data.l[3]    = 0;
            aEvent.xclient.data.l[4]    = 0;
            XSendEvent( m_pDisplay,
                        m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ),
                        False,
                        SubstructureNotifyMask | SubstructureRedirectMask,
                        &aEvent
                        );
        }
        else
        {
            sal_uInt32 nNewLayer = bEnable ? 6 : 4;
            XChangeProperty( m_pDisplay,
                             pFrame->GetShellWindow(),
                             m_aWMAtoms[ WIN_LAYER ],
                             XA_CARDINAL,
                             32,
                             PropModeReplace,
                             reinterpret_cast<unsigned char*>(&nNewLayer),
                             1
                             );
        }
    }
}

/*
 *  WMAdaptor::changeReferenceFrame
 */
void WMAdaptor::changeReferenceFrame( X11SalFrame* pFrame, X11SalFrame const * pReferenceFrame ) const
{
    if( ! ( pFrame->nStyle_ & SalFrameStyleFlags::PLUG )
        && ! pFrame->IsOverrideRedirect()
        && ! pFrame->IsFloatGrabWindow()
        )
    {
        ::Window aTransient = pFrame->pDisplay_->GetRootWindow( pFrame->GetScreenNumber() );
        pFrame->mbTransientForRoot = true;
        if( pReferenceFrame )
        {
            aTransient = pReferenceFrame->GetShellWindow();
            pFrame->mbTransientForRoot = false;
        }
        XSetTransientForHint( m_pDisplay,
                              pFrame->GetShellWindow(),
                              aTransient );
    }
}

/*
 *  WMAdaptor::handlePropertyNotify
 */
int WMAdaptor::handlePropertyNotify( X11SalFrame*, XPropertyEvent* ) const
{
    return 0;
}

/*
 *  NetWMAdaptor::handlePropertyNotify
 */
int NetWMAdaptor::handlePropertyNotify( X11SalFrame* pFrame, XPropertyEvent* pEvent ) const
{
    int nHandled = 1;
    if( pEvent->atom == m_aWMAtoms[ NET_WM_STATE ] )
    {
        pFrame->mbMaximizedHorz = pFrame->mbMaximizedVert = false;
        pFrame->mbShaded = false;

        if( pEvent->state == PropertyNewValue )
        {
            Atom nType, *pStates;
            int nFormat;
            unsigned long nItems, nBytesLeft;
            unsigned char* pData = nullptr;
            long nOffset = 0;
            do
            {
                XGetWindowProperty( m_pDisplay,
                                    pEvent->window,
                                    m_aWMAtoms[ NET_WM_STATE ],
                                    nOffset, 64,
                                    False,
                                    XA_ATOM,
                                    &nType,
                                    &nFormat,
                                    &nItems, &nBytesLeft,
                                    &pData );
                if( pData )
                {
                    if( nType == XA_ATOM && nFormat == 32 && nItems > 0 )
                    {
                        pStates = reinterpret_cast<Atom*>(pData);
                        for( unsigned long i = 0; i < nItems; i++ )
                        {
                            if( pStates[i] == m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_VERT ] && m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_VERT ] )
                                pFrame->mbMaximizedVert = true;
                            else if( pStates[i] == m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_HORZ ] && m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_HORZ ] )
                                pFrame->mbMaximizedHorz = true;
                            else if( pStates[i] == m_aWMAtoms[ NET_WM_STATE_SHADED ] && m_aWMAtoms[ NET_WM_STATE_SHADED ] )
                                pFrame->mbShaded = true;
                        }
                    }
                    XFree( pData );
                    pData = nullptr;
                    nOffset += nItems * nFormat / 32;
                }
                else
                    break;
            } while( nBytesLeft > 0 );
        }

        if( ! (pFrame->mbMaximizedHorz || pFrame->mbMaximizedVert ) )
            pFrame->maRestorePosSize = tools::Rectangle();
        else
        {
            const SalFrameGeometry& rGeom = pFrame->GetUnmirroredGeometry();
            // the current geometry may already be changed by the corresponding
            // ConfigureNotify, but this cannot be helped
            pFrame->maRestorePosSize =
                tools::Rectangle( Point( rGeom.nX, rGeom.nY ),
                           Size( rGeom.nWidth, rGeom.nHeight ) );
        }
    }
    else if( pEvent->atom == m_aWMAtoms[ NET_WM_DESKTOP ] )
    {
        pFrame->m_nWorkArea = getWindowWorkArea( pFrame->GetShellWindow() );
    }
    else
        nHandled = 0;

    return nHandled;
}

/*
 *  GnomeWMAdaptor::handlePropertyNotify
 */
int GnomeWMAdaptor::handlePropertyNotify( X11SalFrame* pFrame, XPropertyEvent* pEvent ) const
{
    int nHandled = 1;
    if( pEvent->atom == m_aWMAtoms[ WIN_STATE ] )
    {
        pFrame->mbMaximizedHorz = pFrame->mbMaximizedVert = false;
        pFrame->mbShaded = false;

        if( pEvent->state == PropertyNewValue )
        {
            Atom nType;
            int nFormat = 0;
            unsigned long nItems = 0;
            unsigned long nBytesLeft = 0;
            unsigned char* pData = nullptr;
            XGetWindowProperty( m_pDisplay,
                                pEvent->window,
                                m_aWMAtoms[ WIN_STATE ],
                                0, 1,
                                False,
                                XA_CARDINAL,
                                &nType,
                                &nFormat,
                                &nItems, &nBytesLeft,
                                &pData );
            if( pData )
            {
                if( nType == XA_CARDINAL && nFormat == 32 && nItems == 1 )
                {
                    sal_uInt32 nWinState = *reinterpret_cast<sal_uInt32*>(pData);
                    if( nWinState & (1<<2) )
                        pFrame->mbMaximizedVert = true;
                    if( nWinState & (1<<3) )
                        pFrame->mbMaximizedHorz = true;
                    if( nWinState & (1<<5) )
                        pFrame->mbShaded = true;
                }
                XFree( pData );
            }
        }

        if( ! (pFrame->mbMaximizedHorz || pFrame->mbMaximizedVert ) )
            pFrame->maRestorePosSize = tools::Rectangle();
        else
        {
            const SalFrameGeometry& rGeom = pFrame->GetUnmirroredGeometry();
            // the current geometry may already be changed by the corresponding
            // ConfigureNotify, but this cannot be helped
            pFrame->maRestorePosSize =
                tools::Rectangle( Point( rGeom.nX, rGeom.nY ),
                           Size( rGeom.nWidth, rGeom.nHeight ) );
        }
    }
    else if( pEvent->atom == m_aWMAtoms[ NET_WM_DESKTOP ] )
    {
        pFrame->m_nWorkArea = getWindowWorkArea( pFrame->GetShellWindow() );
    }
    else
        nHandled = 0;

    return nHandled;
}

/*
 * WMAdaptor::shade
 */
void WMAdaptor::shade( X11SalFrame*, bool /*bToShaded*/ ) const
{
}

/*
 * NetWMAdaptor::shade
 */
void NetWMAdaptor::shade( X11SalFrame* pFrame, bool bToShaded ) const
{
    if( m_aWMAtoms[ NET_WM_STATE ]
        && m_aWMAtoms[ NET_WM_STATE_SHADED ]
        && ( pFrame->nStyle_ & ~SalFrameStyleFlags::DEFAULT )
        )
    {
        pFrame->mbShaded = bToShaded;
        if( pFrame->bMapped_ )
        {
            // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->GetShellWindow();
            aEvent.xclient.message_type = m_aWMAtoms[ NET_WM_STATE ];
            aEvent.xclient.format       = 32;
            aEvent.xclient.data.l[0]    = bToShaded ? 1 : 0;
            aEvent.xclient.data.l[1]    = m_aWMAtoms[ NET_WM_STATE_SHADED ];
            aEvent.xclient.data.l[2]    = 0;
            aEvent.xclient.data.l[3]    = 0;
            aEvent.xclient.data.l[4]    = 0;
            XSendEvent( m_pDisplay,
                        m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ),
                        False,
                        SubstructureNotifyMask | SubstructureRedirectMask,
                        &aEvent
                        );
        }
        else
        {
            // window not mapped yet, set _NET_WM_STATE directly
            setNetWMState( pFrame );
        }
    }
}

/*
 *  GnomeWMAdaptor::shade
 */
void GnomeWMAdaptor::shade( X11SalFrame* pFrame, bool bToShaded ) const
{
    if( m_aWMAtoms[ WIN_STATE ] )
    {
        pFrame->mbShaded = bToShaded;
        if( pFrame->bMapped_ )
        {
            // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->GetShellWindow();
            aEvent.xclient.message_type = m_aWMAtoms[ WIN_STATE ];
            aEvent.xclient.format       = 32;
            aEvent.xclient.data.l[0]    = (1<<5);
            aEvent.xclient.data.l[1]    = bToShaded ? (1<<5) : 0;
            aEvent.xclient.data.l[2]    = 0;
            aEvent.xclient.data.l[3]    = 0;
            aEvent.xclient.data.l[4]    = 0;
            XSendEvent( m_pDisplay,
                        m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ),
                        False,
                        SubstructureNotifyMask | SubstructureRedirectMask,
                        &aEvent
                        );
        }
        else
            setGnomeWMState( pFrame );
    }
}

/*
 *  WMAdaptor::showFullScreen
 */
void WMAdaptor::showFullScreen( X11SalFrame* pFrame, bool bFullScreen ) const
{
    pFrame->mbFullScreen = bFullScreen;
    maximizeFrame( pFrame, bFullScreen, bFullScreen );
}

/*
 *  NetWMAdaptor::showFullScreen
 */
void NetWMAdaptor::showFullScreen( X11SalFrame* pFrame, bool bFullScreen ) const
{
    if( m_aWMAtoms[ NET_WM_STATE_FULLSCREEN ] )
    {
        pFrame->mbFullScreen = bFullScreen;
        if( bFullScreen )
        {
            if( m_aWMAtoms[ MOTIF_WM_HINTS ] )
            {
                XDeleteProperty( m_pDisplay,
                                 pFrame->GetShellWindow(),
                                m_aWMAtoms[ MOTIF_WM_HINTS ] );
            }
        }
        if( pFrame->bMapped_ )
        {
            // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->GetShellWindow();
            aEvent.xclient.message_type = m_aWMAtoms[ NET_WM_STATE ];
            aEvent.xclient.format       = 32;
            aEvent.xclient.data.l[0]    = bFullScreen ? 1 : 0;
            aEvent.xclient.data.l[1]    = m_aWMAtoms[ NET_WM_STATE_FULLSCREEN ];
            aEvent.xclient.data.l[2]    = 0;
            aEvent.xclient.data.l[3]    = 0;
            aEvent.xclient.data.l[4]    = 0;
            XSendEvent( m_pDisplay,
                        m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ),
                        False,
                        SubstructureNotifyMask | SubstructureRedirectMask,
                        &aEvent
                        );
        }
        else
        {
            // window not mapped yet, set _NET_WM_STATE directly
            setNetWMState( pFrame );
        }
        // #i42750# guess size before resize event shows up
        if( bFullScreen )
        {
            if( m_pSalDisplay->IsXinerama() )
            {
                ::Window aRoot, aChild;
                int root_x = 0, root_y = 0, lx, ly;
                unsigned int mask;
                XQueryPointer( m_pDisplay,
                m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ),
                &aRoot, &aChild,
                &root_x, &root_y, &lx, &ly, &mask );
                const std::vector< tools::Rectangle >& rScreens = m_pSalDisplay->GetXineramaScreens();
                Point aMousePoint( root_x, root_y );
                for(const auto & rScreen : rScreens)
                {
                    if( rScreen.IsInside( aMousePoint ) )
                    {
                        pFrame->maGeometry.nX       = rScreen.Left();
                        pFrame->maGeometry.nY       = rScreen.Top();
                        pFrame->maGeometry.nWidth   = rScreen.GetWidth();
                        pFrame->maGeometry.nHeight  = rScreen.GetHeight();
                        break;
                    }
                }
            }
            else
            {
                Size aSize = m_pSalDisplay->GetScreenSize( pFrame->GetScreenNumber() );
                pFrame->maGeometry.nX       = 0;
                pFrame->maGeometry.nY       = 0;
                pFrame->maGeometry.nWidth   = aSize.Width();
                pFrame->maGeometry.nHeight  = aSize.Height();
            }
            pFrame->CallCallback( SalEvent::MoveResize, nullptr );
        }
    }
    else WMAdaptor::showFullScreen( pFrame, bFullScreen );
}

/*
 *  WMAdaptor::getCurrentWorkArea
 */
// FIXME: multiscreen case
int WMAdaptor::getCurrentWorkArea() const
{
    int nCurrent = -1;
    if( m_aWMAtoms[ NET_CURRENT_DESKTOP ] )
    {
        Atom                aRealType   = None;
        int                 nFormat     = 8;
        unsigned long       nItems      = 0;
        unsigned long       nBytesLeft  = 0;
        unsigned char*  pProperty   = nullptr;
        if( XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                                m_aWMAtoms[ NET_CURRENT_DESKTOP ],
                                0, 1,
                                False,
                                XA_CARDINAL,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty ) == 0
                                && pProperty
        )
        {
            nCurrent = int(*reinterpret_cast<sal_Int32*>(pProperty));
            XFree( pProperty );
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = nullptr;
        }
    }
    return nCurrent;
}

/*
 *  WMAdaptor::getWindowWorkArea
 */
int WMAdaptor::getWindowWorkArea( ::Window aWindow ) const
{
    int nCurrent = -1;
    if( m_aWMAtoms[ NET_WM_DESKTOP ] )
    {
        Atom                aRealType   = None;
        int                 nFormat     = 8;
        unsigned long       nItems      = 0;
        unsigned long       nBytesLeft  = 0;
        unsigned char*  pProperty   = nullptr;
        if( XGetWindowProperty( m_pDisplay,
                                aWindow,
                                m_aWMAtoms[ NET_WM_DESKTOP ],
                                0, 1,
                                False,
                                XA_CARDINAL,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty ) == 0
                                && pProperty
        )
        {
            nCurrent = int(*reinterpret_cast<sal_Int32*>(pProperty));
            XFree( pProperty );
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = nullptr;
        }
    }
    return nCurrent;
}

/*
 *  WMAdaptor::getCurrentWorkArea
 */
// fixme: multi screen case
void WMAdaptor::switchToWorkArea( int nWorkArea ) const
{
    if( ! getWMshouldSwitchWorkspace() )
        return;

    if( !m_aWMAtoms[ NET_CURRENT_DESKTOP ] )
        return;

    XEvent aEvent;
    aEvent.type                 = ClientMessage;
    aEvent.xclient.display      = m_pDisplay;
    aEvent.xclient.window       = m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() );
    aEvent.xclient.message_type = m_aWMAtoms[ NET_CURRENT_DESKTOP ];
    aEvent.xclient.format       = 32;
    aEvent.xclient.data.l[0]    = nWorkArea;
    aEvent.xclient.data.l[1]    = 0;
    aEvent.xclient.data.l[2]    = 0;
    aEvent.xclient.data.l[3]    = 0;
    aEvent.xclient.data.l[4]    = 0;
    XSendEvent( m_pDisplay,
                m_pSalDisplay->GetRootWindow( m_pSalDisplay->GetDefaultXScreen() ),
                False,
                SubstructureNotifyMask | SubstructureRedirectMask,
                &aEvent
                );

}

/*
 * WMAdaptor::frameIsMapping
 */
void WMAdaptor::frameIsMapping( X11SalFrame* ) const
{
}

/*
 * NetWMAdaptor::frameIsMapping
 */
void NetWMAdaptor::frameIsMapping( X11SalFrame* pFrame ) const
{
    setNetWMState( pFrame );
}

/*
 * WMAdaptor::setUserTime
 */
void WMAdaptor::setUserTime( X11SalFrame*, long ) const
{
}

/*
 * NetWMAdaptor::setUserTime
 */
void NetWMAdaptor::setUserTime( X11SalFrame* i_pFrame, long i_nUserTime ) const
{
    if( m_aWMAtoms[NET_WM_USER_TIME] )
    {
        XChangeProperty( m_pDisplay,
                         i_pFrame->GetShellWindow(),
                         m_aWMAtoms[NET_WM_USER_TIME],
                         XA_CARDINAL,
                         32,
                         PropModeReplace,
                         reinterpret_cast<unsigned char*>(&i_nUserTime),
                         1
                         );
    }
}

/*
 * WMAdaptor::setPID
 */
void WMAdaptor::setPID( X11SalFrame const * i_pFrame ) const
{
    if( m_aWMAtoms[NET_WM_PID] )
    {
        long nPID = static_cast<long>(getpid());
        XChangeProperty( m_pDisplay,
                         i_pFrame->GetShellWindow(),
                         m_aWMAtoms[NET_WM_PID],
                         XA_CARDINAL,
                         32,
                         PropModeReplace,
                         reinterpret_cast<unsigned char*>(&nPID),
                         1
                         );
    }
}

/*
* WMAdaptor::setClientMachine
*/
void WMAdaptor::setClientMachine( X11SalFrame const * i_pFrame ) const
{
    OString aWmClient( OUStringToOString( GetGenericUnixSalData()->GetHostname(), RTL_TEXTENCODING_ASCII_US ) );
    XTextProperty aClientProp = { reinterpret_cast<unsigned char *>(const_cast<char *>(aWmClient.getStr())), XA_STRING, 8, sal::static_int_cast<unsigned long>( aWmClient.getLength() ) };
    XSetWMClientMachine( m_pDisplay, i_pFrame->GetShellWindow(), &aClientProp );
}

void WMAdaptor::answerPing( X11SalFrame const * i_pFrame, XClientMessageEvent const * i_pEvent ) const
{
    if( m_aWMAtoms[NET_WM_PING] &&
        i_pEvent->message_type == m_aWMAtoms[ WM_PROTOCOLS ] &&
        static_cast<Atom>(i_pEvent->data.l[0]) == m_aWMAtoms[ NET_WM_PING ] )
    {
        XEvent aEvent;
        aEvent.xclient = *i_pEvent;
        aEvent.xclient.window = m_pSalDisplay->GetRootWindow( i_pFrame->GetScreenNumber() );
        XSendEvent( m_pDisplay,
                    m_pSalDisplay->GetRootWindow( i_pFrame->GetScreenNumber() ),
                    False,
                    SubstructureNotifyMask | SubstructureRedirectMask,
                    &aEvent
                    );
        XFlush( m_pDisplay );
    }
}

void WMAdaptor::activateWindow( X11SalFrame const *pFrame, Time nTimestamp )
{
    if (!pFrame->bMapped_)
        return;

    XEvent aEvent;

    aEvent.xclient.type = ClientMessage;
    aEvent.xclient.window = pFrame->GetShellWindow();
    aEvent.xclient.message_type = m_aWMAtoms[ NET_ACTIVE_WINDOW ];
    aEvent.xclient.format = 32;
    aEvent.xclient.data.l[0] = 1;
    aEvent.xclient.data.l[1] = nTimestamp;
    aEvent.xclient.data.l[2] = None;
    aEvent.xclient.data.l[3] = 0;
    aEvent.xclient.data.l[4] = 0;

    XSendEvent( m_pDisplay,
                m_pSalDisplay->GetRootWindow( pFrame->GetScreenNumber() ),
                False,
                SubstructureNotifyMask | SubstructureRedirectMask,
                &aEvent );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
