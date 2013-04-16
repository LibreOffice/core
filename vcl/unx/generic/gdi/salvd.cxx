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


#include <vcl/sysdata.hxx>

#include <prex.h>
#include <X11/extensions/Xrender.h>
#include <postx.h>

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/salvd.h>
#include <unx/x11/xlimits.hxx>

#include <salinst.hxx>

SalVirtualDevice* X11SalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
                                                       long nDX, long nDY,
                                                       sal_uInt16 nBitCount, const SystemGraphicsData *pData )
{
    X11SalVirtualDevice *pVDev = new X11SalVirtualDevice();
    if( !nBitCount && pGraphics )
        nBitCount = pGraphics->GetBitCount();

    if( pData && pData->hDrawable != None )
    {
        XLIB_Window aRoot;
        int x, y;
        unsigned int w = 0, h = 0, bw, d;
        Display* pDisp = GetGenericData()->GetSalDisplay()->GetDisplay();
        XGetGeometry( pDisp, pData->hDrawable,
                      &aRoot, &x, &y, &w, &h, &bw, &d );
        int nScreen = 0;
        while( nScreen < ScreenCount( pDisp ) )
        {
            if( RootWindow( pDisp, nScreen ) == aRoot )
                break;
            nScreen++;
        }
        nDX = (long)w;
        nDY = (long)h;
        if( !pVDev->Init( GetGenericData()->GetSalDisplay(), nDX, nDY, nBitCount,
                          SalX11Screen( nScreen ), pData->hDrawable,
                static_cast< XRenderPictFormat* >( pData->pXRenderFormat )) )
        {
            delete pVDev;
            return NULL;
        }
    }
    else if( !pVDev->Init( GetGenericData()->GetSalDisplay(), nDX, nDY, nBitCount,
                           pGraphics ? static_cast<X11SalGraphics*>(pGraphics)->GetScreenNumber() :
                                       GetGenericData()->GetSalDisplay()->GetDefaultXScreen() ) )
    {
        delete pVDev;
        return NULL;
    }

    pVDev->InitGraphics( pVDev );
    return pVDev;
}

void X11SalInstance::DestroyVirtualDevice( SalVirtualDevice* pDevice )
{
    delete pDevice;
}

void X11SalGraphics::Init( X11SalVirtualDevice *pDevice, SalColormap* pColormap,
                           bool bDeleteColormap )
{
    SalColormap *pOrigDeleteColormap = m_pDeleteColormap;

    SalDisplay *pDisplay  = pDevice->GetDisplay();
    m_nXScreen = pDevice->GetXScreenNumber();

    int nVisualDepth = pDisplay->GetColormap( m_nXScreen ).GetVisual().GetDepth();
    int nDeviceDepth = pDevice->GetDepth();

    if( pColormap )
    {
        m_pColormap = pColormap;
        if( bDeleteColormap )
            m_pDeleteColormap = pColormap;
    }
    else if( nDeviceDepth == nVisualDepth )
        m_pColormap = &pDisplay->GetColormap( m_nXScreen );
    else if( nDeviceDepth == 1 )
        m_pColormap = m_pDeleteColormap = new SalColormap();

    if (m_pDeleteColormap != pOrigDeleteColormap)
        delete pOrigDeleteColormap;

    const Drawable aVdevDrawable = pDevice->GetDrawable();
    SetDrawable( aVdevDrawable, m_nXScreen );

    m_pVDev      = pDevice;
    m_pFrame     = NULL;

    bWindow_     = pDisplay->IsDisplay();
    bVirDev_     = sal_True;
}

sal_Bool X11SalVirtualDevice::Init( SalDisplay *pDisplay,
                                    long nDX, long nDY,
                                    sal_uInt16 nBitCount,
                                    SalX11Screen nXScreen,
                                    Pixmap hDrawable,
                                    XRenderPictFormat* pXRenderFormat )
{
    SalColormap* pColormap = NULL;
    bool bDeleteColormap = false;

    pDisplay_               = pDisplay;
    pGraphics_              = new X11SalGraphics();
    m_nXScreen              = nXScreen;
    if( pXRenderFormat ) {
        pGraphics_->SetXRenderFormat( pXRenderFormat );
        if( pXRenderFormat->colormap )
            pColormap = new SalColormap( pDisplay, pXRenderFormat->colormap, m_nXScreen );
        else
            pColormap = new SalColormap( nBitCount );
         bDeleteColormap = true;
    }
    else if( nBitCount != pDisplay->GetVisual( m_nXScreen ).GetDepth() )
    {
        pColormap = new SalColormap( nBitCount );
        bDeleteColormap = true;
    }
    pGraphics_->SetLayout( 0 ); // by default no! mirroring for VirtualDevices, can be enabled with EnableRTL()
    nDX_                    = nDX;
    nDY_                    = nDY;
    nDepth_                 = nBitCount;

    if( hDrawable == None )
        hDrawable_          = limitXCreatePixmap( GetXDisplay(),
                                             pDisplay_->GetDrawable( m_nXScreen ),
                                             nDX_, nDY_,
                                             GetDepth() );
    else
    {
        hDrawable_ = hDrawable;
        bExternPixmap_ = sal_True;
    }

    pGraphics_->Init( this, pColormap, bDeleteColormap );

    return hDrawable_ != None ? sal_True : sal_False;
}

X11SalVirtualDevice::X11SalVirtualDevice() :
    m_nXScreen( 0 )
{
    pDisplay_               = NULL;
    pGraphics_              = NULL;
    hDrawable_              = None;
    nDX_                    = 0;
    nDY_                    = 0;
    nDepth_                 = 0;
    bGraphics_              = sal_False;
    bExternPixmap_          = sal_False;
}

X11SalVirtualDevice::~X11SalVirtualDevice()
{
    if( pGraphics_ )
        delete pGraphics_;
    pGraphics_ = NULL;

    if( GetDrawable() && !bExternPixmap_ )
        XFreePixmap( GetXDisplay(), GetDrawable() );
}

SalGraphics* X11SalVirtualDevice::GetGraphics()
{
    if( bGraphics_ )
        return NULL;

    if( pGraphics_ )
        bGraphics_ = sal_True;

    return pGraphics_;
}

void X11SalVirtualDevice::ReleaseGraphics( SalGraphics* )
{ bGraphics_ = sal_False; }

sal_Bool X11SalVirtualDevice::SetSize( long nDX, long nDY )
{
    if( bExternPixmap_ )
        return sal_False;

    if( !nDX ) nDX = 1;
    if( !nDY ) nDY = 1;

    Pixmap h = limitXCreatePixmap( GetXDisplay(),
                              pDisplay_->GetDrawable( m_nXScreen ),
                              nDX, nDY, nDepth_ );

    if( !h )
    {
        if( !GetDrawable() )
        {
            hDrawable_ = limitXCreatePixmap( GetXDisplay(),
                                        pDisplay_->GetDrawable( m_nXScreen ),
                                        1, 1, nDepth_ );
            nDX_ = 1;
            nDY_ = 1;
        }
        return sal_False;
    }

    if( GetDrawable() )
        XFreePixmap( GetXDisplay(), GetDrawable() );
    hDrawable_ = h;

    nDX_ = nDX;
    nDY_ = nDY;

    if( pGraphics_ )
        InitGraphics( this );

    return sal_True;
}

void X11SalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
    rWidth  = GetWidth();
    rHeight = GetHeight();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
