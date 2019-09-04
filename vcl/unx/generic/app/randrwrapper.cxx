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

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

namespace
{

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

} // namespace

#endif

#include <unx/saldisp.hxx>
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

void SalDisplay::processRandREvent( XEvent* pEvent )
{
#ifdef USE_RANDR
    XConfigureEvent* pCnfEvent=reinterpret_cast<XConfigureEvent*>(pEvent);
    if( m_bUseRandRWrapper && pWrapper && pWrapper->XRRRootToScreen(GetDisplay(),pCnfEvent->window) != -1 )
    {
        int nRet = pWrapper->XRRUpdateConfiguration( pEvent );
        if( nRet == 1 && pEvent->type != ConfigureNotify) // this should then be a XRRScreenChangeNotifyEvent
        {
            // update screens
            bool bNotify = false;
            for(ScreenData & rScreen : m_aScreens)
            {
                if( rScreen.m_bInit )
                {
                    XRRScreenConfiguration *pConfig = nullptr;
                    XRRScreenSize *pSizes = nullptr;
                    int nSizes = 0;
                    Rotation nRot = 0;
                    SizeID nId = 0;

                    pConfig = pWrapper->XRRGetScreenInfo( GetDisplay(), rScreen.m_aRoot );
                    nId = pWrapper->XRRConfigCurrentConfiguration( pConfig, &nRot );
                    pSizes = pWrapper->XRRConfigSizes( pConfig, &nSizes );
                    XRRScreenSize *pTargetSize = pSizes + nId;

                    bNotify = bNotify ||
                              rScreen.m_aSize.Width() != pTargetSize->width ||
                              rScreen.m_aSize.Height() != pTargetSize->height;

                    rScreen.m_aSize = Size( pTargetSize->width, pTargetSize->height );

                    pWrapper->XRRFreeScreenConfigInfo( pConfig );

                    #if OSL_DEBUG_LEVEL > 1
                    fprintf( stderr, "screen %d changed to size %dx%d\n", (int)nId, (int)pTargetSize->width, (int)pTargetSize->height );
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
