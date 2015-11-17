/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/sysdata.hxx>

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/saldisp.hxx>
#include <unx/salgdi.h>
#include <unx/salvd.h>

#include <opengl/x11/salvd.hxx>

void X11SalGraphics::Init( X11OpenGLSalVirtualDevice *pDevice )
{
    SalDisplay *pDisplay  = pDevice->GetDisplay();

    m_nXScreen = pDevice->GetXScreenNumber();
    m_pColormap = &pDisplay->GetColormap( m_nXScreen );

    m_pVDev      = pDevice;
    m_pFrame     = nullptr;

    bWindow_     = pDisplay->IsDisplay();
    bVirDev_     = true;

    mxImpl->Init();
}

X11OpenGLSalVirtualDevice::X11OpenGLSalVirtualDevice( SalGraphics* pGraphics,
                                                      long &nDX, long &nDY,
                                                      DeviceFormat eFormat,
                                                      const SystemGraphicsData *pData,
                                                      X11SalGraphics* pNewGraphics) :
    mpGraphics(pNewGraphics),
    mbGraphics( false ),
    mnXScreen( 0 )
{
    assert(mpGraphics);

    // TODO Do we really need the requested bit count?
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

    // TODO Check where a VirtualDevice is created from SystemGraphicsData
    assert( pData == nullptr ); (void)pData;

    mpDisplay  = vcl_sal::getSalDisplay(GetGenericData());
    mnDepth    = nBitCount;
    mnXScreen  = pGraphics ? static_cast<X11SalGraphics*>(pGraphics)->GetScreenNumber() :
                             vcl_sal::getSalDisplay(GetGenericData())->GetDefaultXScreen();
    mnWidth    = nDX;
    mnHeight   = nDY;
    mpGraphics->Init( this );
}

X11OpenGLSalVirtualDevice::~X11OpenGLSalVirtualDevice()
{
    delete mpGraphics;
}

SalGraphics* X11OpenGLSalVirtualDevice::AcquireGraphics()
{
    if( mbGraphics )
        return nullptr;

    if( mpGraphics )
        mbGraphics = true;

    return mpGraphics;
}

void X11OpenGLSalVirtualDevice::ReleaseGraphics( SalGraphics* )
{
    mbGraphics = false;
}


bool X11OpenGLSalVirtualDevice::SetSize( long nDX, long nDY )
{
    if( !nDX ) nDX = 1;
    if( !nDY ) nDY = 1;

    mnWidth = nDX;
    mnHeight = nDY;
    if( mpGraphics )
        mpGraphics->Init( this );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
