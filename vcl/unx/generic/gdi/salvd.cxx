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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <vcl/sysdata.hxx>

#include <tools/prex.h>
#include <X11/extensions/Xrender.h>
#include <tools/postx.h>

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/salvd.h>

#include <salinst.hxx>

// -=-= SalInstance =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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
        Display* pDisp = GetX11SalData()->GetDisplay()->GetDisplay();
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
        if( !pVDev->Init( GetX11SalData()->GetDisplay(), nDX, nDY, nBitCount, nScreen, pData->hDrawable, pData->pRenderFormat ) )
        {
            delete pVDev;
            return NULL;
        }
    }
    else if( !pVDev->Init( GetX11SalData()->GetDisplay(), nDX, nDY, nBitCount,
                           pGraphics ? static_cast<X11SalGraphics*>(pGraphics)->GetScreenNumber() :
                                       GetX11SalData()->GetDisplay()->GetDefaultScreenNumber() ) )
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

// -=-= SalGraphicsData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void X11SalGraphics::Init( X11SalVirtualDevice *pDevice, SalColormap* pColormap, bool bDeleteColormap )
{
    SalColormap *pOrigDeleteColormap = m_pDeleteColormap;

    SalDisplay *pDisplay  = pDevice->GetDisplay();
    m_nScreen = pDevice->GetScreenNumber();

    int nVisualDepth = pDisplay->GetColormap( m_nScreen ).GetVisual().GetDepth();
    int nDeviceDepth = pDevice->GetDepth();

    if( pColormap )
    {
        m_pColormap = pColormap;
        if( bDeleteColormap )
            m_pDeleteColormap = pColormap;
    }
    else
    if( nDeviceDepth == nVisualDepth )
        m_pColormap = &pDisplay->GetColormap( m_nScreen );
    else
    if( nDeviceDepth == 1 )
        m_pColormap = m_pDeleteColormap = new SalColormap();

    if (m_pDeleteColormap != pOrigDeleteColormap)
        delete pOrigDeleteColormap;

    const Drawable aVdevDrawable = pDevice->GetDrawable();
    SetDrawable( aVdevDrawable, m_nScreen );

    m_pVDev      = pDevice;
    m_pFrame     = NULL;

    bWindow_     = pDisplay->IsDisplay();
    bVirDev_     = sal_True;
}

// -=-= SalVirDevData / SalVirtualDevice -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
sal_Bool X11SalVirtualDevice::Init( SalDisplay *pDisplay,
                                long nDX, long nDY,
                                sal_uInt16 nBitCount,
                                int nScreen,
                                Pixmap hDrawable,
                                void* pRenderFormatVoid )
{
    SalColormap* pColormap = NULL;
    bool bDeleteColormap = false;

    pDisplay_               = pDisplay;
    pGraphics_              = new X11SalGraphics();
    m_nScreen               = nScreen;
    if( pRenderFormatVoid ) {
        XRenderPictFormat *pRenderFormat = ( XRenderPictFormat* )pRenderFormatVoid;
        pGraphics_->SetXRenderFormat( pRenderFormat );
        if( pRenderFormat->colormap )
            pColormap = new SalColormap( pDisplay, pRenderFormat->colormap, m_nScreen );
        else
            pColormap = new SalColormap( nBitCount );
         bDeleteColormap = true;
    }
    else if( nBitCount != pDisplay->GetVisual( m_nScreen ).GetDepth() )
    {
        pColormap = new SalColormap( nBitCount );
        bDeleteColormap = true;
    }
    pGraphics_->SetLayout( 0 ); // by default no! mirroring for VirtualDevices, can be enabled with EnableRTL()
    nDX_                    = nDX;
    nDY_                    = nDY;
    nDepth_                 = nBitCount;

    if( hDrawable == None )
        hDrawable_          = XCreatePixmap( GetXDisplay(),
                                             pDisplay_->GetDrawable( m_nScreen ),
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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
X11SalVirtualDevice::X11SalVirtualDevice()
{
    pDisplay_               = (SalDisplay*)ILLEGAL_POINTER;
    pGraphics_              = NULL;
    hDrawable_              = None;
    nDX_                    = 0;
    nDY_                    = 0;
    nDepth_                 = 0;
    bGraphics_              = sal_False;
    bExternPixmap_          = sal_False;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
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

    // #144688#
    // the X protocol request CreatePixmap puts an upper bound
    // of 16 bit to the size. Beyond that there may be implementation
    // limits of the Xserver; which we should catch by a failed XCreatePixmap
    // call. However extra large values should be caught here since we'd run into
    // 16 bit truncation here without noticing.
    if( nDX < 0 || nDX > 65535 ||
        nDY < 0 || nDY > 65535 )
        return sal_False;

    if( !nDX ) nDX = 1;
    if( !nDY ) nDY = 1;

    Pixmap h = XCreatePixmap( GetXDisplay(),
                              pDisplay_->GetDrawable( m_nScreen ),
                              nDX, nDY, nDepth_ );

    if( !h )
    {
        if( !GetDrawable() )
        {
            hDrawable_ = XCreatePixmap( GetXDisplay(),
                                        pDisplay_->GetDrawable( m_nScreen ),
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

