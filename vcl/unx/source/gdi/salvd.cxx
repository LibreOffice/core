/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salvd.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-06 10:07:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <prex.h>
#include <X11/extensions/Xrender.h>
#include <postx.h>

#include <salunx.h>

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALDISP_HXX
#include <saldisp.hxx>
#endif
#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALVD_H
#include <salvd.h>
#endif
#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif

// -=-= SalInstance =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
SalVirtualDevice* X11SalInstance::CreateVirtualDevice( SalGraphics* pGraphics,
                                                       long nDX, long nDY,
                                                       USHORT nBitCount, const SystemGraphicsData *pData )
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

    hDrawable_   = pDevice->GetDrawable();
    m_pVDev      = pDevice;
    m_pFrame     = NULL;

    bWindow_     = pDisplay->IsDisplay();
    bVirDev_     = TRUE;

    nPenPixel_   = GetPixel( nPenColor_ );
    nTextPixel_  = GetPixel( nTextColor_ );
    nBrushPixel_ = GetPixel( nBrushColor_ );
}

// -=-= SalVirDevData / SalVirtualDevice -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
BOOL X11SalVirtualDevice::Init( SalDisplay *pDisplay,
                                long nDX, long nDY,
                                USHORT nBitCount,
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
        bExternPixmap_ = TRUE;
    }

    pGraphics_->Init( this, pColormap, bDeleteColormap );

    return hDrawable_ != None ? TRUE : FALSE;
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
    bGraphics_              = FALSE;
    bExternPixmap_          = FALSE;
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
        bGraphics_ = TRUE;

    return pGraphics_;
}

void X11SalVirtualDevice::ReleaseGraphics( SalGraphics* )
{ bGraphics_ = FALSE; }

BOOL X11SalVirtualDevice::SetSize( long nDX, long nDY )
{
    if( bExternPixmap_ )
        return FALSE;

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
        return FALSE;
    }

    if( GetDrawable() )
        XFreePixmap( GetXDisplay(), GetDrawable() );
    hDrawable_ = h;

    nDX_ = nDX;
    nDY_ = nDY;

    if( pGraphics_ )
        InitGraphics( this );

    return TRUE;
}

void X11SalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
    rWidth  = GetWidth();
    rHeight = GetHeight();
}
