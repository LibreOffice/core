/*************************************************************************
 *
 *  $RCSfile: wmadaptor.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: pl $ $Date: 2001-09-10 17:54:45 $
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

#include <stdio.h>
#include <stdlib.h>
#ifdef SOLARIS
#include <alloca.h>
#endif

#ifndef _VCL_WMADAPTOR_HXX_
#include <wmadaptor.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALGGDI_HXX
#include <salgdi.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#include <prex.h>
#include <X11/X.h>
#include <X11/Xatom.h>
#include <postx.h>

#ifdef DEBUG
#include <stdio.h>
#endif

namespace vcl_sal {

class NetWMAdaptor : public WMAdaptor
{
    void setNetWMState( SalFrame* pFrame ) const;
    void initAtoms();
    virtual bool isValid() const;
public:
    NetWMAdaptor( SalDisplay* );
    virtual ~NetWMAdaptor();

    virtual void setWMName( SalFrame* pFrame, const String& rWMName ) const;
    virtual void maximizeFrame( SalFrame* pFrame, bool bHorizontal = true, bool bVertical = true ) const;
    virtual void setFrameTypeAndDecoration( SalFrame* pFrame, WMWindowType eType, int nDecorationFlags, SalFrame* pTransientFrame = NULL ) const;
    virtual bool supportsICCCMPos() const;
    virtual void enableAlwaysOnTop( SalFrame* pFrame, bool bEnable ) const;
};

class GnomeWMAdaptor : public WMAdaptor
{
    bool m_bValid;

    void setGnomeWMState( SalFrame* pFrame ) const;
    void initAtoms();
    virtual bool isValid() const;
public:
    GnomeWMAdaptor( SalDisplay * );
    virtual ~GnomeWMAdaptor();

    virtual void maximizeFrame( SalFrame* pFrame, bool bHorizontal = true, bool bVertical = true ) const;
    virtual void enableAlwaysOnTop( SalFrame* pFrame, bool bEnable ) const;
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
    { "_NET_WM_WINDOW_TYPE_TOOLBAR", WMAdaptor::NET_WM_WINDOW_TYPE_TOOLBAR },
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
    { "SAL_QUITEVENT", WMAdaptor::SAL_QUITEVENT },
    { "SAL_USEREVENT", WMAdaptor::SAL_USEREVENT },
    { "SAL_EXTTEXTEVENT", WMAdaptor::SAL_EXTTEXTEVENT },
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
#ifdef DEBUG
    else
        fprintf( stderr, "WM supports extended WM hints\n" );
#endif

    // try a GnomeWM
    if( ! pAdaptor )
    {
        pAdaptor = new GnomeWMAdaptor( pSalDisplay );
        if( ! pAdaptor->isValid() )
            delete pAdaptor, pAdaptor = NULL;
#ifdef DEBUG
        else
            fprintf( stderr, "WM supports GNOME WM hints\n" );
#endif
    }

    if( ! pAdaptor )
        pAdaptor = new WMAdaptor( pSalDisplay );

#ifdef DEBUG
    fprintf( stderr, "Window Manager's name is \"%s\"\n",
             ByteString( pAdaptor->getWindowManagerName(), RTL_TEXTENCODING_ISO_8859_1 ).GetBuffer() );
#endif
    return pAdaptor;
}


/*
 *  WMAdaptor constructor
 */

WMAdaptor::WMAdaptor( SalDisplay* pDisplay ) :
        m_pSalDisplay( pDisplay )
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
            if (*(XLIB_Boolean*)pProperty)
            {
                m_aWMName = String(RTL_CONSTASCII_USTRINGPARAM("Dtwm"));
            }
            XFree (pProperty);
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
                                &pProperty ) == 0 )
        {
            Atom* pAtoms = (Atom*)pProperty;
            char** pAtomNames = (char**)alloca( sizeof(char*)*nItems );
            if( XGetAtomNames( m_pDisplay, pAtoms, nItems, pAtomNames ) )
            {
#ifdef DEBUG
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
                    }
#ifdef DEBUG
                    fprintf( stderr, "  %s%s\n", pAtomNames[i], nProtocol != -1 ? "" : " (unsupported)" );
#endif

                    XFree( pAtomNames[i] );
                }
            }
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
#ifdef DEBUG
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
#ifdef DEBUG
                fprintf( stderr, "%d workareas for %d desktops !\n", nItems/4, m_nDesktops );
#endif
                if( pProperty )
                {
                    XFree(pProperty);
                    pProperty = NULL;
                }
            }
        }
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
    Atom                aRealType   = None;
    int                 nFormat     = 8;
    unsigned long       nItems      = 0;
    unsigned long       nBytesLeft  = 0;
    unsigned char*  pProperty   = NULL;
    bool                bGnomeWM    = false;

    initAtoms();

    // check for GnomeWM
    if( m_aWMAtoms[ WIN_SUPPORTING_WM_CHECK ] )
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
                                &pProperty ) == 0 )
        {
            Atom* pAtoms = (Atom*)pProperty;
            char** pAtomNames = (char**)alloca( sizeof(char*)*nItems );
            if( XGetAtomNames( m_pDisplay, pAtoms, nItems, pAtomNames ) )
            {
#ifdef DEBUG
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
                    }
#ifdef DEBUG
                    fprintf( stderr, "  %s%s\n", pAtomNames[i], nProtocol != -1 ? "" : " (unsupported)" );
#endif

                    XFree( pAtomNames[i] );
                }
            }
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
            )
        {
            m_nDesktops = *(sal_uInt32*)pProperty;
            XFree( pProperty );
            pProperty = NULL;
        }
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
                }
            }
            m_pSalDisplay->GetXLib()->SetIgnoreXErrors( bIgnore );
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

}

/*
 *  NetWMAdaptor::initAtoms
 */

void NetWMAdaptor::initAtoms()
{
    WMAdaptor::initAtoms();

    m_aWMAtoms[ NET_SUPPORTED ]             = XInternAtom( m_pDisplay, "_NET_SUPPORTED", True );
    m_aWMAtoms[ NET_SUPPORTING_WM_CHECK ]   = XInternAtom( m_pDisplay, "_NET_SUPPORTING_WM_CHECK", True );
    m_aWMAtoms[ NET_WM_NAME ]               = XInternAtom( m_pDisplay, "_NET_WM_NAME", True );
}

/*
 *  GnomeWMAdaptor::initAtoms
 */

void GnomeWMAdaptor::initAtoms()
{
    WMAdaptor::initAtoms();

    m_aWMAtoms[ WIN_PROTOCOLS ]             = XInternAtom( m_pDisplay, "_WIN_PROTOCOLS", True );
    m_aWMAtoms[ WIN_SUPPORTING_WM_CHECK ]   = XInternAtom( m_pDisplay, "_WIN_SUPPORTING_WM_CHECK", True );
    m_aWMAtoms[ NET_SUPPORTING_WM_CHECK ]   = XInternAtom( m_pDisplay, "_NET_SUPPORTING_WM_CHECK", True );
    m_aWMAtoms[ NET_WM_NAME ]               = XInternAtom( m_pDisplay, "_NET_WM_NAME", True );
}

/*
 *  WMAdaptor::setWMName
 *  sets WM_NAME
 *       WM_ICON_NAME
 */

void WMAdaptor::setWMName( SalFrame* pFrame, const String& rWMName ) const
{
    ByteString aTitle( rWMName, osl_getThreadTextEncoding() );

    XChangeProperty( m_pDisplay,
                     pFrame->maFrameData.GetShellWindow(),
                     XA_WM_NAME,
                     XA_STRING,
                     8,
                     PropModeReplace,
                     (unsigned char*)aTitle.GetBuffer(),
                     aTitle.Len() );
    XChangeProperty( m_pDisplay,
                     pFrame->maFrameData.GetShellWindow(),
                     XA_WM_ICON_NAME,
                     XA_STRING,
                     8,
                     PropModeReplace,
                     (unsigned char*)aTitle.GetBuffer(),
                     aTitle.Len() );
}

/*
 *  NetWMAdaptor::setWMName
 *  sets WM_NAME
 *       _NET_WM_NAME
 *       WM_ICON_NAME
 *       _NET_WM_ICON_NAME
 */
void NetWMAdaptor::setWMName( SalFrame* pFrame, const String& rWMName ) const
{
    WMAdaptor::setWMName( pFrame, rWMName );

    ByteString aTitle( rWMName, RTL_TEXTENCODING_UTF8 );
    if( m_aWMAtoms[ NET_WM_NAME ] )
        XChangeProperty( m_pDisplay,
                         pFrame->maFrameData.GetShellWindow(),
                         m_aWMAtoms[ NET_WM_NAME ],
                         m_aWMAtoms[ UTF8_STRING ],
                         8,
                         PropModeReplace,
                         (unsigned char*)aTitle.GetBuffer(),
                         aTitle.Len()+1 );
    if( m_aWMAtoms[ NET_WM_ICON_NAME ] )
        XChangeProperty( m_pDisplay,
                         pFrame->maFrameData.GetShellWindow(),
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
void NetWMAdaptor::setNetWMState( SalFrame* pFrame ) const
{
    if( m_aWMAtoms[ NET_WM_STATE ] )
    {
        Atom aStateAtoms[ 5 ];
        int nStateAtoms = 0;

        // set NET_WM_STATE_MODAL
        if( m_aWMAtoms[ NET_WM_STATE_MODAL ]
            && pFrame->maFrameData.meWindowType == windowType_ModalDialogue )
        {
            aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_MODAL ];
            /*
             *  #90998# NET_WM_STATE_SKIP_TASKBAR set on a frame will
             *  cause kwin not to give it the focus on map request
             *  this seems to be a bug in kwin
             *  aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_SKIP_TASKBAR ];
             */
        }
        if( pFrame->maFrameData.mbMaximizedVert
            && m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_VERT ] )
            aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_VERT ];
        if( pFrame->maFrameData.mbMaximizedHorz
            && m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_HORZ ] )
            aStateAtoms[ nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_HORZ ];
        if( pFrame->maFrameData.bAlwaysOnTop_ && m_aWMAtoms[ NET_WM_STATE_STAYS_ON_TOP ] )
            aStateAtoms[nStateAtoms++ ] = m_aWMAtoms[ NET_WM_STATE_STAYS_ON_TOP ];
        if( nStateAtoms )
        {
            XChangeProperty( m_pDisplay,
                             pFrame->maFrameData.GetShellWindow(),
                             m_aWMAtoms[ NET_WM_STATE ],
                             XA_ATOM,
                             32,
                             PropModeReplace,
                             (unsigned char*)aStateAtoms,
                             nStateAtoms
                             );
        }
        if( pFrame->maFrameData.mbMaximizedHorz
           && pFrame->maFrameData.mbMaximizedVert
           && ! ( pFrame->maFrameData.nStyle_ & SAL_FRAME_STYLE_SIZEABLE ) )
        {
            /*
             *  for maximizing use NorthWestGravity (including decoration)
             */
            XSizeHints  hints;
            long        supplied;
            bool bHint = false;
            if( XGetWMNormalHints( m_pDisplay,
                                   pFrame->maFrameData.GetShellWindow(),
                                   &hints,
                                   &supplied ) )
            {
                bHint = true;
                hints.flags |= PWinGravity;
                hints.win_gravity = NorthWestGravity;
                XSetWMNormalHints( m_pDisplay,
                                   pFrame->maFrameData.GetShellWindow(),
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
                    )
                {
                    nCurrent = *(sal_Int32*)pProperty;
                    XFree( pProperty );
                }
            }
            Rectangle aPosSize = m_aWMWorkAreas[nCurrent];
            aPosSize = Rectangle( Point( aPosSize.Left() + pFrame->maFrameData.nLeft_,
                                         aPosSize.Top()  + pFrame->maFrameData.nTop_ ),
                                  Size( aPosSize.GetWidth()
                                        - pFrame->maFrameData.nLeft_
                                        - pFrame->maFrameData.nRight_,
                                        aPosSize.GetHeight()
                                        - pFrame->maFrameData.nTop_
                                        - pFrame->maFrameData.nBottom_ )
                                  );
            pFrame->maFrameData.SetPosSize( aPosSize );

            /*
             *  reset gravity hint to static gravity
             *  (this should not move window according to ICCCM)
             */
            if( bHint && pFrame->maFrameData.nShowState_ != SHOWSTATE_UNKNOWN )
            {
                hints.win_gravity = StaticGravity;
                XSetWMNormalHints( m_pDisplay,
                                   pFrame->maFrameData.GetShellWindow(),
                                   &hints );
            }
        }
    }
}

/*
 *  GnomeWMAdaptor::setNetWMState
 *  sets _WIN_STATE
 */
void GnomeWMAdaptor::setGnomeWMState( SalFrame* pFrame ) const
{
    if( m_aWMAtoms[ WIN_STATE ] )
    {
        sal_uInt32 nWinWMState = 0;

        if( pFrame->maFrameData.mbMaximizedVert )
            nWinWMState |= 1 << 2;
        if( pFrame->maFrameData.mbMaximizedHorz )
            nWinWMState |= 1 << 3;

        XChangeProperty( m_pDisplay,
                         pFrame->maFrameData.GetShellWindow(),
                         m_aWMAtoms[ WIN_STATE ],
                         XA_CARDINAL,
                         32,
                         PropModeReplace,
                         (unsigned char*)&nWinWMState,
                         1
                         );
        if( pFrame->maFrameData.mbMaximizedHorz
           && pFrame->maFrameData.mbMaximizedVert
           && ! ( pFrame->maFrameData.nStyle_ & SAL_FRAME_STYLE_SIZEABLE ) )
        {
            /*
             *  for maximizing use NorthWestGravity (including decoration)
             */
            XSizeHints  hints;
            long        supplied;
            bool bHint = false;
            if( XGetWMNormalHints( m_pDisplay,
                                   pFrame->maFrameData.GetShellWindow(),
                                   &hints,
                                   &supplied ) )
            {
                bHint = true;
                hints.flags |= PWinGravity;
                hints.win_gravity = NorthWestGravity;
                XSetWMNormalHints( m_pDisplay,
                                   pFrame->maFrameData.GetShellWindow(),
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
                    )
                {
                    nCurrent = *(sal_Int32*)pProperty;
                    XFree( pProperty );
                }
            }
            Rectangle aPosSize = m_aWMWorkAreas[nCurrent];
            aPosSize = Rectangle( Point( aPosSize.Left() + pFrame->maFrameData.nLeft_,
                                         aPosSize.Top()  + pFrame->maFrameData.nTop_ ),
                                  Size( aPosSize.GetWidth()
                                        - pFrame->maFrameData.nLeft_
                                        - pFrame->maFrameData.nRight_,
                                        aPosSize.GetHeight()
                                        - pFrame->maFrameData.nTop_
                                        - pFrame->maFrameData.nBottom_ )
                                  );
            pFrame->maFrameData.SetPosSize( aPosSize );

            /*
             *  reset gravity hint to static gravity
             *  (this should not move window according to ICCCM)
             */
            if( bHint && pFrame->maFrameData.nShowState_ != SHOWSTATE_UNKNOWN )
            {
                hints.win_gravity = StaticGravity;
                XSetWMNormalHints( m_pDisplay,
                                   pFrame->maFrameData.GetShellWindow(),
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

void WMAdaptor::setFrameTypeAndDecoration( SalFrame* pFrame, WMWindowType eType, int nDecorationFlags, SalFrame* pReferenceFrame ) const
{
    pFrame->maFrameData.meWindowType        = eType;
    pFrame->maFrameData.mnDecorationFlags   = nDecorationFlags;

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
                      pFrame->maFrameData.GetShellWindow(),
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
        XSetTransientForHint( m_pDisplay,
                              pFrame->maFrameData.GetShellWindow(),
                              pReferenceFrame->maFrameData.bMapped_ ?
                              pReferenceFrame->maFrameData.GetShellWindow() :
                              m_pSalDisplay->GetRootWindow()
                              );
}

/*
 *  NetWMAdaptor::setFrameDecoration
 *  sets _MOTIF_WM_HINTS
 *       _NET_WM_WINDOW_TYPE
 *       _NET_WM_STATE
 *       WM_TRANSIENT_FOR
 */

void NetWMAdaptor::setFrameTypeAndDecoration( SalFrame* pFrame, WMWindowType eType, int nDecorationFlags, SalFrame* pReferenceFrame ) const
{
    WMAdaptor::setFrameTypeAndDecoration( pFrame, eType, nDecorationFlags, pReferenceFrame );

    setNetWMState( pFrame );

    // set NET_WM_WINDOW_TYPE
    if( m_aWMAtoms[ NET_WM_WINDOW_TYPE ] )
    {
        WMAtom eWMType;
        switch( eType )
        {
            case windowType_ModalDialogue:
                eWMType = NET_WM_WINDOW_TYPE_DIALOG;
                break;
            default:
                eWMType = NET_WM_WINDOW_TYPE_NORMAL;
                break;
        }
        XChangeProperty( m_pDisplay,
                         pFrame->maFrameData.GetShellWindow(),
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
        XSetTransientForHint( m_pDisplay,
                              pFrame->maFrameData.GetShellWindow(),
                              m_pSalDisplay->GetRootWindow() );
}

/*
 *  WMAdaptor::maximizeFrame
 */

void WMAdaptor::maximizeFrame( SalFrame* pFrame, bool bHorizontal, bool bVertical ) const
{
    pFrame->maFrameData.mbMaximizedVert = bVertical;
    pFrame->maFrameData.mbMaximizedHorz = bHorizontal;

    if( bHorizontal || bVertical )
    {
        const Size& aScreenSize( m_pSalDisplay->GetScreenSize() );
        Rectangle aTarget( Point( pFrame->maFrameData.nLeft_,
                                  pFrame->maFrameData.nTop_ ),
                           Size( aScreenSize.Width() - pFrame->maFrameData.nLeft_ - pFrame->maFrameData.nRight_,
                                 aScreenSize.Height() - pFrame->maFrameData.nTop_ - pFrame->maFrameData.nBottom_ )
                           );
        if( ! bHorizontal )
        {
            aTarget.SetSize(
                            Size(
                                 pFrame->maFrameData.aRestoreFullScreen_.IsEmpty() ?
                                 pFrame->maFrameData.aPosSize_.GetWidth() :
                                 pFrame->maFrameData.aRestoreFullScreen_.GetWidth(),
                                 aTarget.GetHeight()
                                 )
                            );
            aTarget.Left() =
                pFrame->maFrameData.aRestoreFullScreen_.IsEmpty() ?
                pFrame->maFrameData.aPosSize_.Left() :
                pFrame->maFrameData.aRestoreFullScreen_.Left();
        }
        else if( ! bVertical )
        {
            aTarget.SetSize(
                            Size(
                                 aTarget.GetWidth(),
                                 pFrame->maFrameData.aRestoreFullScreen_.IsEmpty() ?
                                 pFrame->maFrameData.aPosSize_.GetHeight() :
                                 pFrame->maFrameData.aRestoreFullScreen_.GetHeight()
                                 )
                            );
            aTarget.Top() =
                pFrame->maFrameData.aRestoreFullScreen_.IsEmpty() ?
                pFrame->maFrameData.aPosSize_.Top() :
                pFrame->maFrameData.aRestoreFullScreen_.Top();
        }
        if( pFrame->maFrameData.aRestoreFullScreen_.IsEmpty() )
            pFrame->maFrameData.aRestoreFullScreen_ = pFrame->maFrameData.aPosSize_;
        delete pFrame->maFrameData.pFreeGraphics_;
        pFrame->maFrameData.pFreeGraphics_ = NULL;

        if( pFrame->maFrameData.bMapped_ )
        {
            XSetInputFocus( m_pDisplay,
                            pFrame->maFrameData.GetShellWindow(),
                            RevertToNone,
                            CurrentTime
                            );
        }
        else if( ! pFrame->maFrameData.mpParent )
        {
            // this lets some window managers override
            // user positioning style (e.g. fvwm2)
            XSetTransientForHint( m_pDisplay,
                                  pFrame->maFrameData.GetShellWindow(),
                                  m_pSalDisplay->GetRootWindow() );
        }
        pFrame->maFrameData.SetPosSize( aTarget );
        pFrame->maFrameData.nWidth_     = aTarget.GetWidth();
        pFrame->maFrameData.nHeight_    = aTarget.GetHeight();
        XRaiseWindow( m_pDisplay,
                      pFrame->maFrameData.GetShellWindow()
                      );
        if( pFrame->maFrameData.GetStackingWindow() )
            XRaiseWindow( m_pDisplay,
                          pFrame->maFrameData.GetStackingWindow()
                          );
    }
    else
    {
        delete pFrame->maFrameData.pFreeGraphics_;
        pFrame->maFrameData.pFreeGraphics_ = NULL;

        pFrame->maFrameData.SetPosSize( pFrame->maFrameData.aRestoreFullScreen_ );
        pFrame->maFrameData.aRestoreFullScreen_ = Rectangle();
        pFrame->maFrameData.nWidth_             = pFrame->maFrameData.aPosSize_.GetWidth();
        pFrame->maFrameData.nHeight_            = pFrame->maFrameData.aPosSize_.GetHeight();
    }
}

/*
 *  NetWMAdaptor::maximizeFrame
 *  changes _NET_WM_STATE by sending a client message
 */

void NetWMAdaptor::maximizeFrame( SalFrame* pFrame, bool bHorizontal, bool bVertical ) const
{
    pFrame->maFrameData.mbMaximizedVert = bVertical;
    pFrame->maFrameData.mbMaximizedHorz = bHorizontal;

    if( m_aWMAtoms[ NET_WM_STATE ]
        && m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_VERT ]
        && m_aWMAtoms[ NET_WM_STATE_MAXIMIZED_HORZ ]
        && ( pFrame->maFrameData.nStyle_ & ~SAL_FRAME_STYLE_DEFAULT )
        )
    {
        if( pFrame->maFrameData.bMapped_ )
        {
            // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->maFrameData.GetShellWindow();
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
            pFrame->maFrameData.aRestoreFullScreen_ = Rectangle();
        else if( pFrame->maFrameData.aRestoreFullScreen_.IsEmpty() )
            pFrame->maFrameData.aRestoreFullScreen_ = pFrame->maFrameData.aPosSize_;
    }
    else
        WMAdaptor::maximizeFrame( pFrame, bHorizontal, bVertical );
}

/*
 *  GnomeWMAdaptor::maximizeFrame
 *  changes _WIN_STATE by sending a client message
 */

void GnomeWMAdaptor::maximizeFrame( SalFrame* pFrame, bool bHorizontal, bool bVertical ) const
{
    pFrame->maFrameData.mbMaximizedVert = bVertical;
    pFrame->maFrameData.mbMaximizedHorz = bHorizontal;

    if( m_aWMAtoms[ WIN_STATE ]
        && ( pFrame->maFrameData.nStyle_ & ~SAL_FRAME_STYLE_DEFAULT )
        )
    {
        if( pFrame->maFrameData.bMapped_ )
        {
             // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->maFrameData.GetShellWindow();
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
        {
            // window not mapped yet, set _NET_WM_STATE directly
            setGnomeWMState( pFrame );
        }
        if( !bHorizontal && !bVertical )
            pFrame->maFrameData.aRestoreFullScreen_ = Rectangle();
        else if( pFrame->maFrameData.aRestoreFullScreen_.IsEmpty() )
            pFrame->maFrameData.aRestoreFullScreen_ = pFrame->maFrameData.aPosSize_;
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
void WMAdaptor::enableAlwaysOnTop( SalFrame* pFrame, bool bEnable ) const
{
}

/*
 *  NetWMAdaptor::enableAlwaysOnTop
 */
void NetWMAdaptor::enableAlwaysOnTop( SalFrame* pFrame, bool bEnable ) const
{
    pFrame->maFrameData.bAlwaysOnTop_ = bEnable;
    if( m_aWMAtoms[ NET_WM_STATE_STAYS_ON_TOP ] )
    {
        if( pFrame->maFrameData.bMapped_ )
        {
            // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->maFrameData.GetShellWindow();
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
void GnomeWMAdaptor::enableAlwaysOnTop( SalFrame* pFrame, bool bEnable ) const
{
    pFrame->maFrameData.bAlwaysOnTop_ = bEnable;
    if( m_aWMAtoms[ WIN_LAYER ] )
    {
        if( pFrame->maFrameData.bMapped_ )
        {
            // window already mapped, send WM a message
            XEvent aEvent;
            aEvent.type                 = ClientMessage;
            aEvent.xclient.display      = m_pDisplay;
            aEvent.xclient.window       = pFrame->maFrameData.GetShellWindow();
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
                             pFrame->maFrameData.GetShellWindow(),
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
