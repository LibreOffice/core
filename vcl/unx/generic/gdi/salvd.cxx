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

#include <vcl/opengl/OpenGLHelper.hxx>
#include <opengl/x11/salvd.hxx>

SalVirtualDevice* X11SalInstance::CreateX11VirtualDevice(SalGraphics* pGraphics,
        long &nDX, long &nDY, DeviceFormat eFormat, const SystemGraphicsData *pData,
        X11SalGraphics* pNewGraphics)
{
    assert(pNewGraphics);
    if (OpenGLHelper::isVCLOpenGLEnabled())
        return new X11OpenGLSalVirtualDevice( pGraphics, nDX, nDY, eFormat, pData, pNewGraphics );
    else
        return new X11SalVirtualDevice(pGraphics, nDX, nDY, eFormat, pData, pNewGraphics);
}

SalVirtualDevice* X11SalInstance::CreateVirtualDevice(SalGraphics* pGraphics,
        long &nDX, long &nDY, DeviceFormat eFormat, const SystemGraphicsData *pData)
{
    return CreateX11VirtualDevice(pGraphics, nDX, nDY, eFormat, pData, new X11SalGraphics());
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

    m_pVDev      = pDevice;
    m_pFrame     = nullptr;

    bWindow_     = pDisplay->IsDisplay();
    bVirDev_     = true;

    const Drawable aVdevDrawable = pDevice->GetDrawable();
    SetDrawable( aVdevDrawable, m_nXScreen );
    mxImpl->Init();
}

X11SalVirtualDevice::X11SalVirtualDevice(SalGraphics* pGraphics, long &nDX, long &nDY,
                                         DeviceFormat eFormat, const SystemGraphicsData *pData,
                                         X11SalGraphics* pNewGraphics) :
    pGraphics_(pNewGraphics),
    m_nXScreen(0),
    bGraphics_(false)
{
    SalColormap* pColormap = nullptr;
    bool bDeleteColormap = false;

    sal_uInt16 nBitCount;
    switch (eFormat)
    {
        case DeviceFormat::BITMASK:
            nBitCount = 1;
            break;
        case DeviceFormat::GRAYSCALE:
            nBitCount = 8;
            break;
        default:
            nBitCount = pGraphics->GetBitCount();
            break;

    }

    pDisplay_               = vcl_sal::getSalDisplay(GetGenericData());
    nDepth_                 = nBitCount;

    if( pData && pData->hDrawable != None )
    {
        ::Window aRoot;
        int x, y;
        unsigned int w = 0, h = 0, bw, d;
        Display* pDisp = pDisplay_->GetDisplay();
        XGetGeometry( pDisp, pData->hDrawable,
                      &aRoot, &x, &y, &w, &h, &bw, &d );
        int nScreen = 0;
        while( nScreen < ScreenCount( pDisp ) )
        {
            if( RootWindow( pDisp, nScreen ) == aRoot )
                break;
            nScreen++;
        }
        nDX_ = (long)w;
        nDY_ = (long)h;
        nDX = nDX_;
        nDY = nDY_;
        m_nXScreen = SalX11Screen( nScreen );
        hDrawable_ = pData->hDrawable;
        bExternPixmap_ = true;
    }
    else
    {
        nDX_ = nDX;
        nDY_ = nDY;
        m_nXScreen = pGraphics ? static_cast<X11SalGraphics*>(pGraphics)->GetScreenNumber() :
                                 vcl_sal::getSalDisplay(GetGenericData())->GetDefaultXScreen();
        hDrawable_ = limitXCreatePixmap( GetXDisplay(),
                                         pDisplay_->GetDrawable( m_nXScreen ),
                                         nDX_, nDY_,
                                         GetDepth() );
        bExternPixmap_ = false;
    }

    XRenderPictFormat* pXRenderFormat = pData ? static_cast<XRenderPictFormat*>(pData->pXRenderFormat) : nullptr;
    if( pXRenderFormat )
    {
        pGraphics_->SetXRenderFormat( pXRenderFormat );
        if( pXRenderFormat->colormap )
            pColormap = new SalColormap( pDisplay_, pXRenderFormat->colormap, m_nXScreen );
        else
            pColormap = new SalColormap( nBitCount );
         bDeleteColormap = true;
    }
    else if( nBitCount != pDisplay_->GetVisual( m_nXScreen ).GetDepth() )
    {
        pColormap = new SalColormap( nBitCount );
        bDeleteColormap = true;
    }

    pGraphics_->SetLayout( SalLayoutFlags::NONE ); // by default no! mirroring for VirtualDevices, can be enabled with EnableRTL()
    pGraphics_->Init( this, pColormap, bDeleteColormap );
}

X11SalVirtualDevice::~X11SalVirtualDevice()
{
    delete pGraphics_;
    pGraphics_ = nullptr;

    if( GetDrawable() && !bExternPixmap_ )
        XFreePixmap( GetXDisplay(), GetDrawable() );
}

SalGraphics* X11SalVirtualDevice::AcquireGraphics()
{
    if( bGraphics_ )
        return nullptr;

    if( pGraphics_ )
        bGraphics_ = true;

    return pGraphics_;
}

void X11SalVirtualDevice::ReleaseGraphics( SalGraphics* )
{ bGraphics_ = false; }

bool X11SalVirtualDevice::SetSize( long nDX, long nDY )
{
    if( bExternPixmap_ )
        return false;

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
        return false;
    }

    if( GetDrawable() )
        XFreePixmap( GetXDisplay(), GetDrawable() );
    hDrawable_ = h;

    nDX_ = nDX;
    nDY_ = nDY;

    if( pGraphics_ )
        pGraphics_->Init( this );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
