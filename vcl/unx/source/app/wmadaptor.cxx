/*************************************************************************
 *
 *  $RCSfile: wmadaptor.cxx,v $
 *
 *  $Revision: 1.50 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 16:53:24 $
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
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(SOLARIS) || defined(IRIX)
#include <alloca.h>
#endif

#ifndef _VCL_WMADAPTOR_HXX_
#include <wmadaptor.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif
#ifndef _SV_SALGGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _RTL_LOCALE_H_
#include <rtl/locale.h>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif

#include <prex.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <postx.h>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

namespace vcl_sal {

class NetWMAdaptor : public WMAdaptor
{
    void setNetWMState( X11SalFrame* pFrame ) const;
    void initAtoms();
    virtual bool isValid() const;
public:
    NetWMAdaptor( SalDisplay* );
    virtual ~NetWMAdaptor();

    virtual void setWMName( X11SalFrame* pFrame, const String& rWMName ) const;
    virtual void maximizeFrame( X11SalFrame* pFrame, bool bHorizontal = true, bool bVertical = true ) const;
    virtual void shade( X11SalFrame* pFrame, bool bToShaded ) const;
    virtual void setFrameTypeAndDecoration( X11SalFrame* pFrame, WMWindowType eType, int nDecorationFlags, X11SalFrame* pTransientFrame = NULL ) const;
    virtual bool supportsICCCMPos() const;
    virtual void enableAlwaysOnTop( X11SalFrame* pFrame, bool bEnable ) const;
    virtual int handlePropertyNotify( X11SalFrame* pFrame, XPropertyEvent* pEvent ) const;
    virtual void showFullScreen( X11SalFrame* pFrame, bool bFullScreen ) const;
};

class GnomeWMAdaptor : public WMAdaptor
{
    bool m_bValid;

    void setGnomeWMState( X11SalFrame* pFrame ) const;
    void initAtoms();
    virtual bool isValid() const;
public:
    GnomeWMAdaptor( SalDisplay * );
    virtual ~GnomeWMAdaptor();

    virtual void maximizeFrame( X11SalFrame* pFrame, bool bHorizontal = true, bool bVertical = true ) const;
    virtual void shade( X11SalFrame* pFrame, bool bToShaded ) const;
    virtual void enableAlwaysOnTop( X11SalFrame* pFrame, bool bEnable ) const;
    virtual int handlePropertyNotify( X11SalFrame* pFrame, XPropertyEvent* pEvent ) const;
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
    { "_NET_CURRENT_DESKTOP", WMAdaptor::NET_CURRENT_DESKTOP },
    { "_NET_NUMBER_OF_DESKTOPS", WMAdaptor::NET_NUMBER_OF_DESKTOPS },
    { "_NET_WM_ICON_NAME", WMAdaptor::NET_WM_ICON_NAME },
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
    { "WM_SAVE_YOURSELF", WMAdaptor::WM_SAVE_YOURSELF },
    { "WM_COMMAND", WMAdaptor::WM_COMMAND },
    { "WM_CLIENT_LEADER", WMAdaptor::WM_CLIENT_LEADER },
    { "WM_LOCALE_NAME", WMAdaptor::WM_LOCALE_NAME },
    { "WM_TRANSIENT_FOR", WMAdaptor::WM_TRANSIENT_FOR },
    { "SAL_QUITEVENT", WMAdaptor::SAL_QUITEVENT },
    { "SAL_USEREVENT", WMAdaptor::SAL_USEREVENT },
    { "SAL_EXTTEXTEVENT", WMAdaptor::SAL_EXTTEXTEVENT },
    { "VCL_SYSTEM_SETTINGS", WMAdaptor::VCL_SYSTEM_SETTINGS },
    { "DTWM_IS_RUNNING", WMAdaptor::DTWM_IS_RUNNING }
};

extern "C" {
static int compareProtocol( const void* pLeft, const void* pRight )
{
    return strcmp( ((const WMAdaptorProtocol*)pLeft)->pProtocol, ((const WMAdaptorProtocol*)pRight)->pProtocol );
}
}

WMAdaptor* WMAdaptor::createWMAdaptor( SalDisplay* pSalDisplay )
{
    WMAdaptor*          pAdaptor    = NULL;

    // try a NetWM
    pAdaptor = new NetWMAdaptor( pSalDisplay );
    if( ! pAdaptor->isValid() )
        delete pAdaptor, pAdaptor = NULL;
#if OSL_DEBUG_LEVEL > 1
    else
        fprintf( stderr, "WM supports extended WM hints\n" );
#endif

    // try a GnomeWM
    if( ! pAdaptor )
    {
        pAdaptor = new GnomeWMAdaptor( pSalDisplay );
        if( ! pAdaptor->isValid() )
            delete pAdaptor, pAdaptor = NULL;
#if OSL_DEBUG_LEVEL > 1
        else
            fprintf( stderr, "WM supports GNOME WM hints\n" );
#endif
    }

    if( ! pAdaptor )
        pAdaptor = new WMAdaptor( pSalDisplay );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "Window Manager's name is \"%s\"\n",
             ByteString( pAdaptor->getWindowManagerName(), RTL_TEXTENCODING_ISO_8859_1 ).GetBuffer() );
#endif
    return pAdaptor;
}


/*
 *  WMAdaptor constructor
 */

WMAdaptor::WMAdaptor( SalDisplay* pDisplay ) :
        m_pSalDisplay( pDisplay ),
        m_bTransientBehaviour( true ),
        m_bEnableAlwaysOnTopWorks( false ),
        m_nWinGravity( StaticGravity ),
        m_nInitWinGravity( StaticGravity )
{
    Atom                aRealType   = None;
    int                 nFormat     = 8;
    unsigned long       nItems      = 0;
    unsigned long       nBytesLeft  = 0;
    unsigned char*  pProperty   = NULL;

    // default desktops
    m_nDesktops = 1;
    m_aWMWorkAreas = ::std::vector< Rectangle >
        ( 1, Rectangle( Point( 0, 0 ), m_pSalDisplay->GetScreenSize() ) );
    m_bEqualWorkAreas = true;

    memset( m_aWMAtoms, 0, sizeof( m_aWMAtoms ) );
    m_pDisplay = m_pSalDisplay->GetDisplay();

    initAtoms();
    getNetWmName(); // try to discover e.g. Sawfish

    // check for dtwm running
    if( m_aWMAtoms[ DTWM_IS_RUNNING ] )
    {
        if (   (XGetWindowProperty( m_pDisplay,
                                    m_pSalDisplay->GetRootWindow(),
                                    m_aWMAtoms[ DTWM_IS_RUNNING ],
                                    0, 1,
                                    False,
                                    XA_INTEGER,
                                    &aRealType,
                                    &nFormat,
                                    &nItems,
                                    &nBytesLeft,
                                    &pProperty) == 0
                && nItems)
               || (XGetWindowProperty( m_pDisplay,
                                       m_pSalDisplay->GetRootWindow(),
                                       m_aWMAtoms[ DTWM_IS_RUNNING ],
                                       0, 1,
                                       False,
                                       m_aWMAtoms[ DTWM_IS_RUNNING ],
                                       &aRealType,
                                       &nFormat,
                                       &nItems,
                                       &nBytesLeft,
                                       &pProperty) == 0
                   && nItems))
        {
            if (*pProperty)
            {
                m_aWMName = String(RTL_CONSTASCII_USTRINGPARAM("Dtwm"));
                m_bTransientBehaviour = false;
                m_nWinGravity = CenterGravity;
            }
            XFree (pProperty);
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }
    }
    if( m_aWMName.Len() == 0 )
    {
        // check for window maker - needs different gravity
        Atom aWMakerRunning = XInternAtom( m_pDisplay, "_WINDOWMAKER_WM_PROTOCOLS", True );
        if( aWMakerRunning != None &&
            XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow(),
                                aWMakerRunning,
                                0, 32,
                                False,
                                XA_ATOM,
                                &aRealType,
                                &nFormat,
                                &nItems,
                                &nBytesLeft,
                                &pProperty ) == 0 )
        {
            if( aRealType == XA_ATOM )
                m_aWMName = String( RTL_CONSTASCII_USTRINGPARAM("Windowmaker" ) );
            XFree( pProperty );
            m_nInitWinGravity = NorthWestGravity;
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }
    }
    if( m_aWMName.Len() == 0 )
    {
        if( XInternAtom( m_pDisplay, "_OL_WIN_ATTR", True ) )
        {
            m_aWMName = String( RTL_CONSTASCII_USTRINGPARAM( "Olwm" ) );
            m_nInitWinGravity = NorthWestGravity;
        }
    }
    if( m_aWMName.Len() == 0 )
    {
        // check for ReflectionX wm (as it needs a workaround in Windows mode
        Atom aRwmRunning = XInternAtom( m_pDisplay, "RWM_RUNNING", True );
        if( aRwmRunning != None &&
            XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow(),
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
                m_aWMName = String( RTL_CONSTASCII_USTRINGPARAM("ReflectionX" ) );
            XFree( pProperty );
        }
        else if( (aRwmRunning = XInternAtom( m_pDisplay, "_WRQ_WM_RUNNING", True )) != None &&
            XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow(),
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
                m_aWMName = String( RTL_CONSTASCII_USTRINGPARAM( "ReflectionX Windows" ) );
            XFree( pProperty );
        }
    }

#ifdef MACOSX
        /* Apple's X11 needs NW gravity with OOo 1.1 */
        m_nWinGravity = NorthWestGravity;
        m_nInitWinGravity = NorthWestGravity;
#endif
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
    m_bTransientBehaviour = true;

    Atom                aRealType   = None;
    int                 nFormat     = 8;
    unsigned long       nItems      = 0;
    unsigned long       nBytesLeft  = 0;
    unsigned char*  pProperty   = NULL;
    bool                bNetWM      = false;

    initAtoms();

    // check for NetWM
    bNetWM = getNetWmName();
    if( bNetWM
        && XGetWindowProperty( m_pDisplay,
                               m_pSalDisplay->GetRootWindow(),
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
            pProperty = NULL;
        }
        // collect supported protocols
        if( XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow(),
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
            Atom* pAtoms = (Atom*)pProperty;
            char** pAtomNames = (char**)alloca( sizeof(char*)*nItems );
            if( XGetAtomNames( m_pDisplay, pAtoms, nItems, pAtomNames ) )
            {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "supported protocols:\n" );
#endif
                for( int i = 0; i < nItems; i++ )
                {
                    int nProtocol = -1;
                    WMAdaptorProtocol aSearch;
                    aSearch.pProtocol = pAtomNames[i];
                    WMAdaptorProtocol* pMatch = (WMAdaptorProtocol*)
                        bsearch( &aSearch,
                                 aProtocolTab,
                                 sizeof( aProtocolTab )/sizeof( aProtocolTab[0] ),
                                 sizeof( struct WMAdaptorProtocol ),
                                 compareProtocol );
                    if( pMatch )
                    {
                        nProtocol = pMatch->nProtocol;
                        m_aWMAtoms[ nProtocol ] = pAtoms[ i ];
                        if( pMatch->nProtocol == NET_WM_STATE_STAYS_ON_TOP )
                            m_bEnableAlwaysOnTopWorks = true;
                    }
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "  %s%s\n", pAtomNames[i], nProtocol != -1 ? "" : " (unsupported)" );
#endif

                    XFree( pAtomNames[i] );
                }
            }
            XFree( pProperty );
            pProperty = NULL;
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }

        // get number of desktops
        if( m_aWMAtoms[ NET_NUMBER_OF_DESKTOPS ]
            && XGetWindowProperty( m_pDisplay,
                                   m_pSalDisplay->GetRootWindow(),
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
            m_nDesktops = *(sal_uInt32*)pProperty;
            XFree( pProperty );
            pProperty = NULL;
            // get work areas
            if( m_aWMAtoms[ NET_WORKAREA ]
                && XGetWindowProperty( m_pDisplay,
                                       m_pSalDisplay->GetRootWindow(),
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
                && nItems == 4*m_nDesktops
                )
            {
                m_aWMWorkAreas = ::std::vector< Rectangle > ( m_nDesktops );
                sal_uInt32* pValues = (sal_uInt32*)pProperty;
                for( int i = 0; i < m_nDesktops; i++ )
                {
                    Point aPoint( pValues[4*i],
                                  pValues[4*i+1] );
                    Size aSize( pValues[4*i+2],
                                pValues[4*i+3] );
                    Rectangle aWorkArea( aPoint, aSize );
                    m_aWMWorkAreas[i] = aWorkArea;
                    if( aWorkArea != m_aWMWorkAreas[0] )
                        m_bEqualWorkAreas = false;
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "workarea %d: %dx%d+%d+%d\n",
                             i,
                             m_aWMWorkAreas[i].GetWidth(),
                             m_aWMWorkAreas[i].GetHeight(),
                             m_aWMWorkAreas[i].Left(),
                             m_aWMWorkAreas[i].Top() );
#endif
                }
            }
            else
            {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "%d workareas for %d desktops !\n", nItems/4, m_nDesktops );
#endif
                if( pProperty )
                {
                    XFree(pProperty);
                    pProperty = NULL;
                }
            }
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }
    }
    else if( pProperty )
    {
        XFree( pProperty );
        pProperty = NULL;
    }
}

/*
 *  NetWMAdaptor destructor
 */
NetWMAdaptor::~NetWMAdaptor()
{
}

/*
 *  GnomeWMAdaptor constructor
 */

GnomeWMAdaptor::GnomeWMAdaptor( SalDisplay* pSalDisplay ) :
        WMAdaptor( pSalDisplay ),
        m_bValid( false )
{
    // currently all Gnome WMs do transient like expected
    m_bTransientBehaviour = true;

    Atom                aRealType   = None;
    int                 nFormat     = 8;
    unsigned long       nItems      = 0;
    unsigned long       nBytesLeft  = 0;
    unsigned char*  pProperty   = NULL;

    initAtoms();

    // check for GnomeWM
    if( m_aWMAtoms[ WIN_SUPPORTING_WM_CHECK ] && m_aWMAtoms[ WIN_PROTOCOLS ] )
    {
        XLIB_Window         aWMChild    = None;
        if( XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow(),
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
            aWMChild = *(XLIB_Window*)pProperty;
            XFree( pProperty );
            pProperty = NULL;
            XLIB_Window aCheckWindow = None;
            BOOL bIgnore = m_pSalDisplay->GetXLib()->GetIgnoreXErrors();
            m_pSalDisplay->GetXLib()->SetIgnoreXErrors( TRUE );
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
                && nItems != 0
                && ! m_pSalDisplay->GetXLib()->WasXError()
                )
            {
                aCheckWindow =  *(XLIB_Window*)pProperty;
                XFree( pProperty );
                pProperty = NULL;
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
            m_pSalDisplay->GetXLib()->SetIgnoreXErrors( bIgnore );
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }
    }
    if( m_bValid
        && XGetWindowProperty( m_pDisplay,
                               m_pSalDisplay->GetRootWindow(),
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
            pProperty = NULL;
        }
        // collect supported protocols
        if( XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow(),
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
            Atom* pAtoms = (Atom*)pProperty;
            char** pAtomNames = (char**)alloca( sizeof(char*)*nItems );
            if( XGetAtomNames( m_pDisplay, pAtoms, nItems, pAtomNames ) )
            {
#if OSL_DEBUG_LEVEL > 1
                fprintf( stderr, "supported protocols:\n" );
#endif
                for( int i = 0; i < nItems; i++ )
                {
                    int nProtocol = -1;
                    WMAdaptorProtocol aSearch;
                    aSearch.pProtocol = pAtomNames[i];
                    WMAdaptorProtocol* pMatch = (WMAdaptorProtocol*)
                        bsearch( &aSearch,
                                 aProtocolTab,
                                 sizeof( aProtocolTab )/sizeof( aProtocolTab[0] ),
                                 sizeof( struct WMAdaptorProtocol ),
                                 compareProtocol );
                    if( pMatch )
                    {
                        nProtocol = pMatch->nProtocol;
                        m_aWMAtoms[ nProtocol ] = pAtoms[ i ];
                        if( pMatch->nProtocol == WIN_LAYER )
                            m_bEnableAlwaysOnTopWorks = true;
                    }
                    if( strncmp( "_ICEWM_TRAY", pAtomNames[i], 11 ) == 0 )
                    {
                        m_aWMName = String(RTL_CONSTASCII_USTRINGPARAM("IceWM" ));
                        m_nWinGravity = NorthWestGravity;
                        m_nInitWinGravity = NorthWestGravity;
                    }
#if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "  %s%s\n", pAtomNames[i], nProtocol != -1 ? "" : " (unsupported)" );
#endif

                    XFree( pAtomNames[i] );
                }
            }
            XFree( pProperty );
            pProperty = NULL;
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }

        // get number of desktops
        if( m_aWMAtoms[ WIN_WORKSPACE_COUNT ]
            && XGetWindowProperty( m_pDisplay,
                                   m_pSalDisplay->GetRootWindow(),
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
            m_nDesktops = *(sal_uInt32*)pProperty;
            XFree( pProperty );
            pProperty = NULL;
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }
    }
    else if( pProperty )
    {
        XFree( pProperty );
        pProperty = NULL;
    }
}

/*
 *  GnomeWMAdaptor destructor
 */
GnomeWMAdaptor::~GnomeWMAdaptor()
{
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
    unsigned char*  pProperty   = NULL;
    bool                bNetWM      = false;

    if( m_aWMAtoms[ NET_SUPPORTING_WM_CHECK ] && m_aWMAtoms[ NET_WM_NAME ] )
    {
        XLIB_Window         aWMChild = None;
        if( XGetWindowProperty( m_pDisplay,
                                m_pSalDisplay->GetRootWindow(),
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
            aWMChild = *(XLIB_Window*)pProperty;
            XFree( pProperty );
            pProperty = NULL;
            XLIB_Window aCheckWindow = None;
            BOOL bIgnore = m_pSalDisplay->GetXLib()->GetIgnoreXErrors();
            m_pSalDisplay->GetXLib()->SetIgnoreXErrors( TRUE );
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
                && nItems != 0
                && ! m_pSalDisplay->GetXLib()->WasXError()
                )
            {
                aCheckWindow =  *(XLIB_Window*)pProperty;
                XFree( pProperty );
                pProperty = NULL;
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
                        {
                            m_aWMName = String( (sal_Char*)pProperty, nItems, RTL_TEXTENCODING_UTF8 );
                        }
                        else
                        if (aRealType == XA_STRING)
                        {
                            m_aWMName = String( (sal_Char*)pProperty, nItems, RTL_TEXTENCODING_ISO_8859_1 );
                        }

                        XFree( pProperty );
                        pProperty = NULL;
                    }
                    else if( pProperty )
                    {
                        XFree( pProperty );
                        pProperty = NULL;
                    }
                }
            }
            else if( pProperty )
            {
                XFree( pProperty );
                pProperty = NULL;
            }
            m_pSalDisplay->GetXLib()->SetIgnoreXErrors( bIgnore );
        }
        else if( pProperty )
        {
            XFree( pProperty );
            pProperty = NULL;
        }
    }
    return bNetWM;
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
    for( int i = 0; i < sizeof( aAtomTab )/sizeof( aAtomTab[0] ); i++ )
        m_aWMAtoms[ aAtomTab[i].nProtocol ] = XInternAtom( m_pDisplay, aAtomTab[i].pProtocol, False );
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

void WMAdaptor::setWMName( X11SalFrame* pFrame, const String& rWMName ) const
{
    ByteString aTitle( rWMName, osl_getThreadTextEncoding() );

    if( ! rWMName.Len() && m_aWMName.EqualsAscii( "Dtwm" ) )
        aTitle = " ";

    ::rtl::OString aWMLocale;
    rtl_Locale* pLocale = NULL;
    osl_getProcessLocale( &pLocale );
    if( pLocale )
    {
        ::rtl::OUString aLocaleString( pLocale->Language );
        ::rtl::OUString aCountry( pLocale->Country );
        ::rtl::OUString aVariant( pLocale->Variant );

        if( aCountry.getLength() )
        {
            aLocaleString += ::rtl::OUString::createFromAscii( "_" );
            aLocaleString += aCountry;
        }
        if( aVariant.getLength() )
            aLocaleString += aVariant;
        aWMLocale = ::rtl::OUStringToOString( aLocaleString, RTL_TEXTENCODING_ISO_8859_1 );
    }
    else
    {
        static const char* pLang = getenv( "LANG" );
        aWMLocale = pLang ? pLang : "C";
    }

    char* pT = const_cast<char*>(aTitle.GetBuffer());
    XTextProperty aProp = { NULL, None, 0, 0 };
    XmbTextListToTextProperty( m_pDisplay,
                               &pT,
                               1,
                               XStdICCTextStyle,
                               &aProp );

    unsigned char* pData    = aProp.nitems ? aProp.value : (unsigned char*)aTitle.GetBuffer();
    Atom nType              = aProp.nitems ? aProp.encoding : XA_STRING;
    int nFormat             = aProp.nitems ? aProp.format : 8;
    int nBytes              = aProp.nitems ? aProp.nitems : aTitle.Len();
    const SystemEnvData* pEnv = pFrame->GetSystemData();
    XChangeProperty( m_pDisplay,
                     (XLIB_Window)pEnv->aShellWindow,
                     XA_WM_NAME,
                     nType,
                     nFormat,
                     PropModeReplace,
                     pData,
                     nBytes );
    XChangeProperty( m_pDisplay,
                     (XLIB_Window)pEnv->aShellWindow,
                     XA_WM_ICON_NAME,
                     nType,
                     nFormat,
                     PropModeReplace,
                     pData,
                     nBytes );
    XChangeProperty( m_pDisplay,
                     (XLIB_Window)pEnv->aShellWindow,
                     m_aWMAtoms[ WM_LOCALE_NAME ],
                     XA_STRING,
                     8,
                     PropModeReplace,
                     (unsigned char*)aWMLocale.getStr(),
                     aWMLocale.getLength() );
    if (aProp.value != NULL)
        XFree( aProp.value );
}

/*
 *  NetWMAdaptor::setWMName
 *  sets WM_NAME
 *       _NET_WM_NAME
 *       WM_ICON_NAME
 *       _NET_WM_ICON_NAME
 */
void NetWMAdaptor::setWMName( X11SalFrame* pFrame, const String& rWMName ) const
{
    WMAdaptor::setWMName( pFrame, rWMName );

    ByteString aTitle( rWMName, RTL_TEXTENCODING_UTF8 );
    const SystemEnvData* pEnv = pFrame->GetSystemData();
    if( m_aWMAtoms[ NET_WM_NAME ] )
        XChangeProperty( m_pDisplay,
                         (XLIB_Window)pEnv->aShellWindow,
                         m_aWMAtoms[ NET_WM_NAME ],
                         m_aWMAtoms[ UTF8_STRING ],
                         8,
                         PropModeReplace,
                         (unsigned char*)aTitle.GetBuffer(),
                         aTitle.Len()+1 );
    if( m_aWMAtoms[ NET_WM_ICON_NAME ] )
        XChangeProperty( m_pDisplay,
                         (XLIB_Window)pEnv->aShellWindow,
                         m_aWMAtoms[ NET_WM_ICON_NAME ],
                         m_aWMAtoms[ UTF8_STRING ],
                         8,
                         PropModeReplace,
                         (unsigned char*)aTitle.GetBuffer(),
                         aTitle.Len()+1 );
    // The +1 copies the terminating null byte. Although
    // the spec says, this should not be necessary
    // at least the kwin implementation seems to depend
    // on the null byte
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
        if( m_aWMAtoms[ NET_WM_STATE_MODAL ]
            && pFrame->meWindowType == windowType_ModalDialogue )
        {
            aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_MODAL ];
            /*
             *  #90998# NET_WM_STATE_SKIP_TASKBAR set on a frame will
             *  cause kwin not to give it the focus on map request
             *  this seems to be a bug in kwin
             *  aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_SKIP_TASKBAR ];
             */
        }
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

        if( nStateAtoms )
        {
            XChangeProperty( m_pDisplay,
                             pFrame->GetShellWindow(),
                             m_aWMAtoms[ NET_WM_STATE ],
                             XA_ATOM,
                             32,
                             PropModeReplace,
                             (unsigned char*)aStateAtoms,
                             nStateAtoms
                             );
        }
        else
            XDeleteProperty( m_pDisplay,
                             pFrame->GetShellWindow(),
                             m_aWMAtoms[ NET_WM_STATE ] );
        if( pFrame->mbMaximizedHorz
           && pFrame->mbMaximizedVert
           && ! ( pFrame->nStyle_ & SAL_FRAME_STYLE_SIZEABLE ) )
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
            if( ! m_bEqualWorkAreas
                && m_aWMAtoms[ NET_CURRENT_DESKTOP ]
                )
            {
                Atom                aRealType   = None;
                int                 nFormat     = 8;
                unsigned long       nItems      = 0;
                unsigned long       nBytesLeft  = 0;
                unsigned char*  pProperty   = NULL;
                if( XGetWindowProperty( m_pDisplay,
                                        m_pSalDisplay->GetRootWindow(),
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
                    nCurrent = *(sal_Int32*)pProperty;
                    XFree( pProperty );
                }
                else if( pProperty )
                {
                    XFree( pProperty );
                    pProperty = NULL;
                }
            }
            Rectangle aPosSize = m_aWMWorkAreas[nCurrent];
            const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
            aPosSize = Rectangle( Point( aPosSize.Left() + rGeom.nLeftDecoration,
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
                         (unsigned char*)&nWinWMState,
                         1
                         );
        if( pFrame->mbMaximizedHorz
           && pFrame->mbMaximizedVert
           && ! ( pFrame->nStyle_ & SAL_FRAME_STYLE_SIZEABLE ) )
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
            if( ! m_bEqualWorkAreas
                && m_aWMAtoms[ NET_CURRENT_DESKTOP ]
                )
            {
                Atom                aRealType   = None;
                int                 nFormat     = 8;
                unsigned long       nItems      = 0;
                unsigned long       nBytesLeft  = 0;
                unsigned char*  pProperty   = NULL;
                if( XGetWindowProperty( m_pDisplay,
                                        m_pSalDisplay->GetRootWindow(),
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
                    nCurrent = *(sal_Int32*)pProperty;
                    XFree( pProperty );
                }
                else if( pProperty )
                {
                    XFree( pProperty );
                    pProperty = NULL;
                }
            }
            Rectangle aPosSize = m_aWMWorkAreas[nCurrent];
            const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
            aPosSize = Rectangle( Point( aPosSize.Left() + rGeom.nLeftDecoration,
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
    pFrame->mnDecorationFlags   = nDecorationFlags;

    // set mwm hints
    struct _mwmhints {
        unsigned long flags, func, deco;
        long input_mode;
        unsigned long status;
    } aHint;

    aHint.flags = 7; /* flags for functions, decoration and input mode */
    aHint.deco = 0;
    aHint.func = 1L << 2;

    // evaluate decoration flags
    if( nDecorationFlags & decoration_All )
        aHint.deco = 1, aHint.func = 1;
    else
    {
        if( nDecorationFlags & decoration_Title )
            aHint.deco |= 1L << 3;
        if( nDecorationFlags & decoration_Border )
            aHint.deco |= 1L << 1;
        if( nDecorationFlags & decoration_Resize )
            aHint.deco |= 1L << 2, aHint.func |= 1L << 1;
        if( nDecorationFlags & decoration_MinimizeBtn )
            aHint.deco |= 1L << 5, aHint.func |= 1L << 3;
        if( nDecorationFlags & decoration_MaximizeBtn )
            aHint.deco |= 1L << 6, aHint.func |= 1L << 4;
        if( nDecorationFlags & decoration_CloseBtn )
            aHint.deco |= 1L << 4, aHint.func |= 1L << 5;
    }
    // evaluate window type
    switch( eType )
    {
        case windowType_ModalDialogue:
            aHint.input_mode = 1;
            break;
        default:
            aHint.input_mode = 0;
            break;
    }

    // set the hint
     XChangeProperty( m_pDisplay,
                      pFrame->GetShellWindow(),
                      m_aWMAtoms[ MOTIF_WM_HINTS ],
                      m_aWMAtoms[ MOTIF_WM_HINTS ],
                      32,
                      PropModeReplace,
                      (unsigned char*)&aHint,
                      5 );

    // set transientFor hint
    /*  #91030# dtwm will not map a dialogue if the transient
     *  window is iconified. This is deemed undesireable because
     *  message boxes do not get mapped, so use the root as transient
     *  instead.
     */
    if( pReferenceFrame )
    {
        XSetTransientForHint( m_pDisplay,
                              pFrame->GetShellWindow(),
                              pReferenceFrame->bMapped_ ?
                              pReferenceFrame->GetShellWindow() :
                              m_pSalDisplay->GetRootWindow()
                              );
        if( ! pReferenceFrame->bMapped_ )
            pFrame->mbTransientForRoot = true;
    }
    // #110333# in case no one ever sets a title prevent
    // the Dtwm taking the class instead
    if( m_aWMName.EqualsAscii( "Dtwm" ) )
        setWMName( pFrame, String() );
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
        WMAtom eWMType;
        switch( eType )
        {
            case windowType_Utility:
                eWMType = m_aWMAtoms[ NET_WM_WINDOW_TYPE_UTILITY ] ? NET_WM_WINDOW_TYPE_UTILITY : NET_WM_WINDOW_TYPE_DIALOG;
                break;
            case windowType_ModelessDialogue:
            case windowType_ModalDialogue:
                eWMType = NET_WM_WINDOW_TYPE_DIALOG;
                break;
            case windowType_Splash:
                eWMType = m_aWMAtoms[ NET_WM_WINDOW_TYPE_SPLASH ] ? NET_WM_WINDOW_TYPE_SPLASH : NET_WM_WINDOW_TYPE_NORMAL;
                break;
            default:
                eWMType = NET_WM_WINDOW_TYPE_NORMAL;
                break;
        }
        XChangeProperty( m_pDisplay,
                         pFrame->GetShellWindow(),
                         m_aWMAtoms[ NET_WM_WINDOW_TYPE ],
                         XA_ATOM,
                         32,
                             PropModeReplace,
                         (unsigned char*)&m_aWMAtoms[ eWMType ],
                         1 );
    }
    if( ( eType == windowType_ModalDialogue ||
          eType == windowType_ModelessDialogue )
        && ! pReferenceFrame )
    {
        XSetTransientForHint( m_pDisplay,
                              pFrame->GetShellWindow(),
                              m_pSalDisplay->GetRootWindow() );
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
        Size aScreenSize( m_pSalDisplay->GetScreenSize() );
        Point aTL( rGeom.nLeftDecoration, rGeom.nTopDecoration );
        if( m_pSalDisplay->IsXinerama() )
        {
            Point aMed( aTL.X() + rGeom.nWidth/2, aTL.Y() + rGeom.nHeight/2 );
            const std::vector< Rectangle >& rScreens = m_pSalDisplay->GetXineramaScreens();
            for( int i = 0; i < rScreens.size(); i++ )
                if( rScreens[i].IsInside( aMed ) )
                {
                    aTL += rScreens[i].TopLeft();
                    aScreenSize = rScreens[i].GetSize();
                    break;
                }
        }
        Rectangle aTarget( aTL,
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
            aTarget.Left() =
                pFrame->maRestorePosSize.IsEmpty() ?
                rGeom.nX : pFrame->maRestorePosSize.Left();
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
            aTarget.Top() =
                pFrame->maRestorePosSize.IsEmpty() ?
                rGeom.nY : pFrame->maRestorePosSize.Top();
        }

        Rectangle aRestore( Point( rGeom.nX, rGeom.nY ), Size( rGeom.nWidth, rGeom.nHeight ) );
        if( pFrame->bMapped_ )
        {
            XSetInputFocus( m_pDisplay,
                            pFrame->GetShellWindow(),
                            RevertToNone,
                            CurrentTime
                            );
            if( m_aWMName.EqualsAscii( "Dtwm" ) )
            {
                /*
                 *  Dtwm will only position  correctly with center gravity
                 *  and in this case the request actually changes the frame
                 *  not the shell window
                 */
                aTarget = Rectangle( Point( 0, 0 ), aScreenSize );
                aRestore.Move( -rGeom.nLeftDecoration, -rGeom.nTopDecoration );
            }
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
        pFrame->maRestorePosSize = Rectangle();
        pFrame->nWidth_             = rGeom.nWidth;
        pFrame->nHeight_            = rGeom.nHeight;
        if( m_aWMName.EqualsAscii( "Dtwm" ) && pFrame->bMapped_ )
        {
            pFrame->maGeometry.nX += rGeom.nLeftDecoration;
            pFrame->maGeometry.nY += rGeom.nTopDecoration;
        }
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
        && ( pFrame->nStyle_ & ~SAL_FRAME_STYLE_DEFAULT )
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
                        m_pSalDisplay->GetRootWindow(),
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
                            m_pSalDisplay->GetRootWindow(),
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
            pFrame->maRestorePosSize = Rectangle();
        else if( pFrame->maRestorePosSize.IsEmpty() )
        {
            const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
            pFrame->maRestorePosSize =
                Rectangle( Point( rGeom.nX, rGeom.nY ), Size( rGeom.nWidth, rGeom.nHeight ) );
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
        && ( pFrame->nStyle_ & ~SAL_FRAME_STYLE_DEFAULT )
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
                        m_pSalDisplay->GetRootWindow(),
                        False,
                        SubstructureNotifyMask,
                        &aEvent
                        );
        }
        else
            // window not mapped yet, set _WIN_STATE directly
            setGnomeWMState( pFrame );

        if( !bHorizontal && !bVertical )
            pFrame->maRestorePosSize = Rectangle();
        else if( pFrame->maRestorePosSize.IsEmpty() )
        {
            const SalFrameGeometry& rGeom( pFrame->GetUnmirroredGeometry() );
            pFrame->maRestorePosSize =
                Rectangle( Point( rGeom.nX, rGeom.nY ), Size( rGeom.nWidth, rGeom.nHeight ) );
        }
    }
    else
        WMAdaptor::maximizeFrame( pFrame, bHorizontal, bVertical );
}

/*
 *  WMAdaptor::supportsICCCMPos
 */

bool WMAdaptor::supportsICCCMPos() const
{
    return
        m_aWMName.EqualsAscii( "Sawfish" )
        || m_aWMName.EqualsAscii( "Dtwm" );
}

/*
 *  NetWMAdaptor::supportsICCCMPos
 */

bool NetWMAdaptor::supportsICCCMPos() const
{
    return true;
}


/*
 *  WMAdaptor::enableAlwaysOnTop
 */
void WMAdaptor::enableAlwaysOnTop( X11SalFrame* pFrame, bool bEnable ) const
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
                        m_pSalDisplay->GetRootWindow(),
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
                        m_pSalDisplay->GetRootWindow(),
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
                             (unsigned char*)&nNewLayer,
                             1
                             );
        }
    }
}

/*
 *  WMAdaptor::changeReferenceFrame
 */
void WMAdaptor::changeReferenceFrame( X11SalFrame* pFrame, X11SalFrame* pReferenceFrame ) const
{
    if( ! ( pFrame->nStyle_ & ( SAL_FRAME_STYLE_CHILD | SAL_FRAME_STYLE_FLOAT ) )
        && ! pFrame->IsOverrideRedirect()
        )
    {
        XLIB_Window aTransient = pFrame->pDisplay_->GetRootWindow();
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
int WMAdaptor::handlePropertyNotify( X11SalFrame* pFrame, XPropertyEvent* pEvent ) const
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
            unsigned char* pData;
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
                        pStates = (Atom*)pData;
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
                }
            } while( nBytesLeft > 0 );
        }

        if( ! (pFrame->mbMaximizedHorz || pFrame->mbMaximizedVert ) )
            pFrame->maRestorePosSize = Rectangle();
        else
        {
            const SalFrameGeometry& rGeom = pFrame->GetUnmirroredGeometry();
            // the current geometry may already be changed by the corresponding
            // ConfigureNotify, but this cannot be helped
            pFrame->maRestorePosSize =
                Rectangle( Point( rGeom.nX, rGeom.nY ),
                           Size( rGeom.nWidth, rGeom.nHeight ) );
        }
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
            unsigned char* pData = 0;
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
                    sal_uInt32 nWinState = *(sal_uInt32*)pData;
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
            pFrame->maRestorePosSize = Rectangle();
        else
        {
            const SalFrameGeometry& rGeom = pFrame->GetUnmirroredGeometry();
            // the current geometry may already be changed by the corresponding
            // ConfigureNotify, but this cannot be helped
            pFrame->maRestorePosSize =
                Rectangle( Point( rGeom.nX, rGeom.nY ),
                           Size( rGeom.nWidth, rGeom.nHeight ) );
        }
    }
    else
        nHandled = 0;

    return nHandled;
}

/*
 * WMAdaptor::shade
 */
void WMAdaptor::shade( X11SalFrame* pFrame, bool bToShaded ) const
{
}

/*
 * NetWMAdaptor::shade
 */
void NetWMAdaptor::shade( X11SalFrame* pFrame, bool bToShaded ) const
{
    if( m_aWMAtoms[ NET_WM_STATE ]
        && m_aWMAtoms[ NET_WM_STATE_SHADED ]
        && ( pFrame->nStyle_ & ~SAL_FRAME_STYLE_DEFAULT )
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
                        m_pSalDisplay->GetRootWindow(),
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
                        m_pSalDisplay->GetRootWindow(),
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
                        m_pSalDisplay->GetRootWindow(),
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
    else WMAdaptor::showFullScreen( pFrame, bFullScreen );
}
