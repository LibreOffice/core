/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: randrwrapper.cxx,v $
 *
 * $Revision: 1.3 $
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

#ifdef USE_RANDR

#include "prex.h"
#include <X11/extensions/Xrandr.h>
#include "postx.h"

#include "osl/module.h"
#include "rtl/ustring.hxx"

namespace
{
class RandRWrapper
{
    oslModule m_pRandRLib;

    // function pointers
    Bool(*m_pXRRQueryExtension)(Display*,int*,int*);
    Status(*m_pXRRQueryVersion)(Display*,int*,int*);
    XRRScreenConfiguration*(*m_pXRRGetScreenInfo)(Display*,Drawable);
    void(*m_pXRRFreeScreenConfigInfo)(XRRScreenConfiguration*);
    void(*m_pXRRSelectInput)(Display*,XLIB_Window,int);
    int(*m_pXRRUpdateConfiguration)(XEvent*);
    XRRScreenSize*(*m_pXRRSizes)(Display*,int,int*);
    XRRScreenSize*(*m_pXRRConfigSizes)(XRRScreenConfiguration*,int*);
    SizeID(*m_pXRRConfigCurrentConfiguration)(XRRScreenConfiguration*,Rotation*);

    bool m_bValid;

    void initFromModule();

    RandRWrapper(Display*);
    ~RandRWrapper();
public:
    static RandRWrapper& get(Display*);
    static void releaseWrapper();

    Bool XRRQueryExtension(Display* i_pDisp, int* o_event_base, int* o_error_base )
    {
        Bool bRet = False;
        if( m_bValid )
            bRet = m_pXRRQueryExtension( i_pDisp, o_event_base, o_error_base );
        return bRet;
    }
    Status XRRQueryVersion( Display* i_pDisp, int* o_major, int* o_minor )
    {
        return m_bValid ? m_pXRRQueryVersion( i_pDisp, o_major, o_minor ) : 0;
    }
    XRRScreenConfiguration* XRRGetScreenInfo( Display* i_pDisp, Drawable i_aDrawable )
    {
        return m_bValid ? m_pXRRGetScreenInfo( i_pDisp, i_aDrawable ) : NULL;
    }
    void XRRFreeScreenConfigInfo( XRRScreenConfiguration* i_pConfig )
    {
        if( m_bValid )
            m_pXRRFreeScreenConfigInfo( i_pConfig );
    }
    void XRRSelectInput( Display* i_pDisp, XLIB_Window i_window, int i_nMask )
    {
        if( m_bValid )
            m_pXRRSelectInput( i_pDisp, i_window, i_nMask );
    }
    int XRRUpdateConfiguration( XEvent* i_pEvent )
    {
        return m_bValid ? m_pXRRUpdateConfiguration( i_pEvent ) : 0;
    }
    XRRScreenSize* XRRSizes( Display* i_pDisp, int i_screen, int* o_nscreens )
    {
        return m_bValid ? m_pXRRSizes( i_pDisp, i_screen, o_nscreens ) : NULL;
    }
    XRRScreenSize* XRRConfigSizes( XRRScreenConfiguration* i_pConfig, int* o_nSizes )
    {
        return m_bValid ? m_pXRRConfigSizes( i_pConfig, o_nSizes ) : NULL;
    }
    SizeID XRRConfigCurrentConfiguration( XRRScreenConfiguration* i_pConfig, Rotation* o_pRot )
    {
        return m_bValid ? m_pXRRConfigCurrentConfiguration( i_pConfig, o_pRot ) : 0;
    }
};
}

void RandRWrapper::initFromModule()
{
    m_pXRRQueryExtension = (Bool(*)(Display*,int*,int*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRQueryExtension" );
    m_pXRRQueryVersion = (Status(*)(Display*,int*,int*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRQueryVersion" );
    m_pXRRGetScreenInfo = (XRRScreenConfiguration*(*)(Display*,Drawable))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRGetScreenInfo" );
    m_pXRRFreeScreenConfigInfo = (void(*)(XRRScreenConfiguration*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRFreeScreenConfigInfo" );
    m_pXRRSelectInput = (void(*)(Display*,XLIB_Window,int))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRSelectInput" );
    m_pXRRUpdateConfiguration = (int(*)(XEvent*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRUpdateConfiguration" );
    m_pXRRSizes = (XRRScreenSize*(*)(Display*,int,int*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRSizes" );
    m_pXRRConfigSizes = (XRRScreenSize*(*)(XRRScreenConfiguration*,int*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRConfigSizes" );
    m_pXRRConfigCurrentConfiguration = (SizeID(*)(XRRScreenConfiguration*,Rotation*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRConfigCurrentConfiguration" );

    m_bValid = m_pXRRQueryExtension             &&
               m_pXRRQueryVersion               &&
               m_pXRRGetScreenInfo              &&
               m_pXRRFreeScreenConfigInfo       &&
               m_pXRRSelectInput                &&
               m_pXRRUpdateConfiguration        &&
               m_pXRRSizes                      &&
               m_pXRRConfigSizes                &&
               m_pXRRConfigCurrentConfiguration
               ;
}

RandRWrapper::RandRWrapper( Display* pDisplay ) :
        m_pRandRLib( NULL ),
        m_pXRRQueryExtension( NULL ),
        m_pXRRQueryVersion( NULL ),
        m_pXRRGetScreenInfo( NULL ),
        m_pXRRFreeScreenConfigInfo( NULL ),
        m_pXRRSelectInput( NULL ),
        m_pXRRUpdateConfiguration( NULL ),
        m_pXRRSizes( NULL ),
        m_pXRRConfigSizes( NULL ),
        m_pXRRConfigCurrentConfiguration( NULL ),
        m_bValid( false )
{
    // first try in process space (e.g. gtk links that ?)
    initFromModule();
    if( ! m_bValid )
    {
        rtl::OUString aLibName( RTL_CONSTASCII_USTRINGPARAM( "libXrandr.so.2" ) );
        m_pRandRLib = osl_loadModule( aLibName.pData, SAL_LOADMODULE_DEFAULT );
        initFromModule();
    }
    if( m_bValid )
    {
        int nEventBase = 0, nErrorBase = 0;
        if( ! m_pXRRQueryExtension( pDisplay, &nEventBase, &nErrorBase ) )
            m_bValid = false;
    }
}

RandRWrapper::~RandRWrapper()
{
    if( m_pRandRLib )
        osl_unloadModule( m_pRandRLib );
}

static RandRWrapper* pWrapper = NULL;

RandRWrapper& RandRWrapper::get( Display* i_pDisplay )
{
    if( ! pWrapper )
        pWrapper = new RandRWrapper( i_pDisplay );
    return *pWrapper;
}

void RandRWrapper::releaseWrapper()
{
    delete pWrapper;
    pWrapper = NULL;
}

#endif

#include "saldisp.hxx"

void SalDisplay::InitRandR( XLIB_Window aRoot ) const
{
    #ifdef USE_RANDR
    RandRWrapper::get( GetDisplay() ).XRRSelectInput( GetDisplay(), aRoot, RRScreenChangeNotifyMask );
    #else
    (void)aRoot;
    #endif
}

void SalDisplay::DeInitRandR()
{
    #ifdef USE_RANDR
    RandRWrapper::releaseWrapper();
    #endif
}

int SalDisplay::processRandREvent( XEvent* pEvent )
{
    int nRet = 0;
    #ifdef USE_RANDR
    if( pWrapper )
    {
        nRet = pWrapper->XRRUpdateConfiguration( pEvent );
        if( nRet == 1 && pEvent->type != ConfigureNotify) // this should then be a XRRScreenChangeNotifyEvent
        {
            // update screens
            for( size_t i = 0; i < m_aScreens.size(); i++ )
            {
                if( m_aScreens[i].m_bInit )
                {
                    XRRScreenConfiguration *pConfig = NULL;
                    XRRScreenSize *pSizes = NULL;
                    int nSizes = 0;
                    Rotation nRot = 0;
                    SizeID nId = 0;

                    pConfig = pWrapper->XRRGetScreenInfo( GetDisplay(), m_aScreens[i].m_aRoot );
                    nId = pWrapper->XRRConfigCurrentConfiguration( pConfig, &nRot );
                    pSizes = pWrapper->XRRConfigSizes( pConfig, &nSizes );
                    XRRScreenSize *pTargetSize = pSizes + nId;

                    m_aScreens[i].m_aSize = Size( pTargetSize->width, pTargetSize->height );

                    pWrapper->XRRFreeScreenConfigInfo( pConfig );

                    #if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "screen %d changed to size %dx%d\n", i, pTargetSize->width, pTargetSize->height );
                    #endif
                }
            }
        }
    }
    #else
    (void)pEvent;
    #endif
    return nRet;
}
