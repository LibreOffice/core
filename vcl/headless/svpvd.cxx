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

#include "headless/svpbmp.hxx"
#include "headless/svpvd.hxx"
#include "headless/svpgdi.hxx"

#include <basegfx/vector/b2ivector.hxx>
#include <basebmp/scanlineformats.hxx>

#include "stdio.h"

using namespace basegfx;
using namespace basebmp;

SvpSalVirtualDevice::~SvpSalVirtualDevice()
{
}

SalGraphics* SvpSalVirtualDevice::GetGraphics()
{
    SvpSalGraphics* pGraphics = new SvpSalGraphics();
    pGraphics->setDevice( m_aDevice );
    m_aGraphics.push_back( pGraphics );
    return pGraphics;
}

void SvpSalVirtualDevice::ReleaseGraphics( SalGraphics* pGraphics )
{
    m_aGraphics.remove( dynamic_cast<SvpSalGraphics*>(pGraphics) );
    delete pGraphics;
}

sal_Bool SvpSalVirtualDevice::SetSize( long nNewDX, long nNewDY )
{
    return SetSizeUsingBuffer( nNewDX, nNewDY, basebmp::RawMemorySharedArray() );
}

sal_Bool SvpSalVirtualDevice::SetSizeUsingBuffer( long nNewDX, long nNewDY, const basebmp::RawMemorySharedArray &pBuffer )
{
    B2IVector aDevSize( nNewDX, nNewDY );
    if( aDevSize.getX() == 0 )
        aDevSize.setX( 1 );
    if( aDevSize.getY() == 0 )
        aDevSize.setY( 1 );
    if( ! m_aDevice.get() || m_aDevice->getSize() != aDevSize )
    {
        basebmp::Format nFormat = SVP_DEFAULT_BITMAP_FORMAT;
        std::vector< basebmp::Color > aDevPal;
        switch( m_nBitCount )
        {
            case 1: nFormat = FORMAT_ONE_BIT_MSB_PAL;
                aDevPal.reserve(2);
                aDevPal.push_back( basebmp::Color( 0, 0, 0 ) );
                aDevPal.push_back( basebmp::Color( 0xff, 0xff, 0xff ) );
                break;
            case 4: nFormat = FORMAT_FOUR_BIT_MSB_PAL; break;
            case 8: nFormat = FORMAT_EIGHT_BIT_PAL; break;
#ifdef OSL_BIGENDIAN
            case 16: nFormat = FORMAT_SIXTEEN_BIT_MSB_TC_MASK; break;
#else
            case 16: nFormat = FORMAT_SIXTEEN_BIT_LSB_TC_MASK; break;
#endif
            case 24: nFormat = FORMAT_TWENTYFOUR_BIT_TC_MASK; break;
            case 32: nFormat = FORMAT_THIRTYTWO_BIT_TC_MASK_BGRA; break;
#if defined(ANDROID) || defined(IOS)
            case 0:  nFormat = FORMAT_THIRTYTWO_BIT_TC_MASK_RGBA; break;
#else
            case 0:  nFormat = FORMAT_TWENTYFOUR_BIT_TC_MASK; break;
#endif
        }
        m_aDevice = aDevPal.empty()
                    ? ( pBuffer
                        ? createBitmapDevice( aDevSize, false, nFormat, pBuffer, PaletteMemorySharedVector() )
                        : createBitmapDevice( aDevSize, false, nFormat )
                       )
                    : createBitmapDevice( aDevSize, false, nFormat, PaletteMemorySharedVector( new std::vector< basebmp::Color >(aDevPal) ) );

        // update device in existing graphics
        for( std::list< SvpSalGraphics* >::iterator it = m_aGraphics.begin();
             it != m_aGraphics.end(); ++it )
             (*it)->setDevice( m_aDevice );

    }
    return true;
}

void SvpSalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
    if( m_aDevice.get() )
    {
        B2IVector aDevSize( m_aDevice->getSize() );
        rWidth = aDevSize.getX();
        rHeight = aDevSize.getY();
    }
    else
        rWidth = rHeight = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
