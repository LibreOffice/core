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

#ifdef USE_RANDR

#include <prex.h>
#include <X11/extensions/Xrandr.h>
#include <postx.h>

#include "osl/module.h"
#include "rtl/ustring.hxx"

namespace
{

# ifdef XRANDR_DLOPEN

class RandRWrapper
{
    oslModule m_pRandRLib;

    // function pointers
    Bool(*m_pXRRQueryExtension)(Display*,int*,int*);
    XRRScreenConfiguration*(*m_pXRRGetScreenInfo)(Display*,Drawable);
    void(*m_pXRRFreeScreenConfigInfo)(XRRScreenConfiguration*);
    void(*m_pXRRSelectInput)(Display*,::Window,int);
    int(*m_pXRRUpdateConfiguration)(XEvent*);
    XRRScreenSize*(*m_pXRRConfigSizes)(XRRScreenConfiguration*,int*);
    SizeID(*m_pXRRConfigCurrentConfiguration)(XRRScreenConfiguration*,Rotation*);
    int(*m_pXRRRootToScreen)(Display*, ::Window);

    bool m_bValid;

    void initFromModule();

    explicit RandRWrapper(Display*);
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
    XRRScreenConfiguration* XRRGetScreenInfo( Display* i_pDisp, Drawable i_aDrawable )
    {
        return m_bValid ? m_pXRRGetScreenInfo( i_pDisp, i_aDrawable ) : NULL;
    }
    void XRRFreeScreenConfigInfo( XRRScreenConfiguration* i_pConfig )
    {
        if( m_bValid )
            m_pXRRFreeScreenConfigInfo( i_pConfig );
    }
    void XRRSelectInput( Display* i_pDisp, ::Window i_window, int i_nMask )
    {
        if( m_bValid )
            m_pXRRSelectInput( i_pDisp, i_window, i_nMask );
    }
    int XRRUpdateConfiguration( XEvent* i_pEvent )
    {
        return m_bValid ? m_pXRRUpdateConfiguration( i_pEvent ) : 0;
    }
    XRRScreenSize* XRRConfigSizes( XRRScreenConfiguration* i_pConfig, int* o_nSizes )
    {
        return m_bValid ? m_pXRRConfigSizes( i_pConfig, o_nSizes ) : NULL;
    }
    SizeID XRRConfigCurrentConfiguration( XRRScreenConfiguration* i_pConfig, Rotation* o_pRot )
    {
        return m_bValid ? m_pXRRConfigCurrentConfiguration( i_pConfig, o_pRot ) : 0;
    }
    int XRRRootToScreen( Display *dpy, ::Window root )
    {
        return m_bValid ? m_pXRRRootToScreen( dpy, root ) : -1;
    }
};

void RandRWrapper::initFromModule()
{
    m_pXRRQueryExtension = (Bool(*)(Display*,int*,int*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRQueryExtension" );
    m_pXRRGetScreenInfo = (XRRScreenConfiguration*(*)(Display*,Drawable))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRGetScreenInfo" );
    m_pXRRFreeScreenConfigInfo = (void(*)(XRRScreenConfiguration*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRFreeScreenConfigInfo" );
    m_pXRRSelectInput = (void(*)(Display*,::Window,int))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRSelectInput" );
    m_pXRRUpdateConfiguration = (int(*)(XEvent*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRUpdateConfiguration" );
    m_pXRRConfigSizes = (XRRScreenSize*(*)(XRRScreenConfiguration*,int*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRConfigSizes" );
    m_pXRRConfigCurrentConfiguration = (SizeID(*)(XRRScreenConfiguration*,Rotation*))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRConfigCurrentConfiguration" );
    m_pXRRRootToScreen = (int(*)(Display*,::Window))osl_getAsciiFunctionSymbol( m_pRandRLib, "XRRRootToScreen" );

    m_bValid = m_pXRRQueryExtension             &&
               m_pXRRGetScreenInfo              &&
               m_pXRRFreeScreenConfigInfo       &&
               m_pXRRSelectInput                &&
               m_pXRRUpdateConfiguration        &&
               m_pXRRConfigSizes                &&
               m_pXRRConfigCurrentConfiguration &&
               m_pXRRRootToScreen
               ;
}

RandRWrapper::RandRWrapper( Display* pDisplay ) :
        m_pRandRLib( NULL ),
        m_pXRRQueryExtension( NULL ),
        m_pXRRGetScreenInfo( NULL ),
        m_pXRRFreeScreenConfigInfo( NULL ),
        m_pXRRSelectInput( NULL ),
        m_pXRRUpdateConfiguration( NULL ),
        m_pXRRConfigSizes( NULL ),
        m_pXRRConfigCurrentConfiguration( NULL ),
        m_pXRRRootToScreen( NULL ),
        m_bValid( false )
{
    // first try in process space (e.g. gtk links that ?)
    initFromModule();
    if( ! m_bValid )
    {
        // load and resolve dependencies immediately
        // rationale: there are older distributions where libXrandr.so.2 is not linked
        // with libXext.so, resulting in a missing symbol and terminating the office
        // obviously they expected libXext to be linked in global symbolspace (that is
        // linked by the application), which is not the case with us (because we want
        // to be able to run in headless mode even without an installed X11 library)
        m_pRandRLib = osl_loadModule( "libXrandr.so.2", SAL_LOADMODULE_DEFAULT | SAL_LOADMODULE_NOW );
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

# else

class RandRWrapper
{
    bool m_bValid;

    explicit RandRWrapper(Display*);
public:
    static RandRWrapper& get(Display*);
    static void releaseWrapper();

    Bool XRRQueryExtension(Display* i_pDisp, int* o_event_base, int* o_error_base )
    {
        Bool bRet = False;
        if( m_bValid )
            bRet = ::XRRQueryExtension( i_pDisp, o_event_base, o_error_base );
        return bRet;
    }
    XRRScreenConfiguration* XRRGetScreenInfo( Display* i_pDisp, Drawable i_aDrawable )
    {
        return m_bValid ? ::XRRGetScreenInfo( i_pDisp, i_aDrawable ) : nullptr;
    }
    void XRRFreeScreenConfigInfo( XRRScreenConfiguration* i_pConfig )
    {
        if( m_bValid )
            ::XRRFreeScreenConfigInfo( i_pConfig );
    }
    void XRRSelectInput( Display* i_pDisp, ::Window i_window, int i_nMask )
    {
        if( m_bValid )
            ::XRRSelectInput( i_pDisp, i_window, i_nMask );
    }
    int XRRUpdateConfiguration( XEvent* i_pEvent )
    {
        return m_bValid ? ::XRRUpdateConfiguration( i_pEvent ) : 0;
    }
    XRRScreenSize* XRRConfigSizes( XRRScreenConfiguration* i_pConfig, int* o_nSizes )
    {
        return m_bValid ? ::XRRConfigSizes( i_pConfig, o_nSizes ) : nullptr;
    }
    SizeID XRRConfigCurrentConfiguration( XRRScreenConfiguration* i_pConfig, Rotation* o_pRot )
    {
        return m_bValid ? ::XRRConfigCurrentConfiguration( i_pConfig, o_pRot ) : 0;
    }
    int XRRRootToScreen( Display *dpy, ::Window root )
    {
        return m_bValid ? ::XRRRootToScreen( dpy, root ) : -1;
    }
};

RandRWrapper::RandRWrapper( Display* pDisplay ) :
    m_bValid( true )
{
    int nEventBase = 0, nErrorBase = 0;
    if( !XRRQueryExtension( pDisplay, &nEventBase, &nErrorBase ) )
        m_bValid = false;
}

static RandRWrapper* pWrapper = nullptr;

RandRWrapper& RandRWrapper::get( Display* i_pDisplay )
{
    if( ! pWrapper )
        pWrapper = new RandRWrapper( i_pDisplay );
    return *pWrapper;
}

void RandRWrapper::releaseWrapper()
{
    delete pWrapper;
    pWrapper = nullptr;
}

#endif

} // namespace

#endif

#include "unx/saldisp.hxx"
#include "unx/salframe.h"
#if OSL_DEBUG_LEVEL > 1
#include <cstdio>
#endif

void SalDisplay::InitRandR( ::Window aRoot ) const
{
    #ifdef USE_RANDR
    if( m_bUseRandRWrapper )
        RandRWrapper::get( GetDisplay() ).XRRSelectInput( GetDisplay(), aRoot, RRScreenChangeNotifyMask );
    #else
    (void)this;
    (void)aRoot;
    #endif
}

void SalDisplay::DeInitRandR()
{
    #ifdef USE_RANDR
    if( m_bUseRandRWrapper )
        RandRWrapper::releaseWrapper();
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "SalDisplay::DeInitRandR()\n" );
#endif
    #else
    (void)this;
    #endif
}

int SalDisplay::processRandREvent( XEvent* pEvent )
{
    int nRet = 0;
    #ifdef USE_RANDR
    XConfigureEvent* pCnfEvent=reinterpret_cast<XConfigureEvent*>(pEvent);
    if( m_bUseRandRWrapper && pWrapper && pWrapper->XRRRootToScreen(GetDisplay(),pCnfEvent->window) != -1 )
    {
        nRet = pWrapper->XRRUpdateConfiguration( pEvent );
        if( nRet == 1 && pEvent->type != ConfigureNotify) // this should then be a XRRScreenChangeNotifyEvent
        {
            // update screens
            bool bNotify = false;
            for( size_t i = 0; i < m_aScreens.size(); i++ )
            {
                if( m_aScreens[i].m_bInit )
                {
                    XRRScreenConfiguration *pConfig = nullptr;
                    XRRScreenSize *pSizes = nullptr;
                    int nSizes = 0;
                    Rotation nRot = 0;
                    SizeID nId = 0;

                    pConfig = pWrapper->XRRGetScreenInfo( GetDisplay(), m_aScreens[i].m_aRoot );
                    nId = pWrapper->XRRConfigCurrentConfiguration( pConfig, &nRot );
                    pSizes = pWrapper->XRRConfigSizes( pConfig, &nSizes );
                    XRRScreenSize *pTargetSize = pSizes + nId;

                    bNotify = bNotify ||
                              m_aScreens[i].m_aSize.Width() != pTargetSize->width ||
                              m_aScreens[i].m_aSize.Height() != pTargetSize->height;

                    m_aScreens[i].m_aSize = Size( pTargetSize->width, pTargetSize->height );

                    pWrapper->XRRFreeScreenConfigInfo( pConfig );

                    #if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "screen %d changed to size %dx%d\n", (int)i, (int)pTargetSize->width, (int)pTargetSize->height );
                    #endif
                }
            }
            if( bNotify )
                emitDisplayChanged();
        }
    }
    #else
    (void)this;
    (void)pEvent;
    #endif
    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
