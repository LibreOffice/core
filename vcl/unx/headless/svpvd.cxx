/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "svpvd.hxx"
#include "svpgdi.hxx"

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
    B2IVector aDevSize( nNewDX, nNewDY );
    if( aDevSize.getX() == 0 )
        aDevSize.setX( 1 );
    if( aDevSize.getY() == 0 )
        aDevSize.setY( 1 );
    if( ! m_aDevice.get() || m_aDevice->getSize() != aDevSize )
    {
        sal_uInt32 nFormat = SVP_DEFAULT_BITMAP_FORMAT;
        std::vector< basebmp::Color > aDevPal;
        switch( m_nBitCount )
        {
            case 1: nFormat = Format::ONE_BIT_MSB_PAL;
                aDevPal.reserve(2);
                aDevPal.push_back( basebmp::Color( 0, 0, 0 ) );
                aDevPal.push_back( basebmp::Color( 0xff, 0xff, 0xff ) );
                break;
            case 4: nFormat = Format::FOUR_BIT_MSB_PAL; break;
            case 8: nFormat = Format::EIGHT_BIT_PAL; break;
#ifdef OSL_BIGENDIAN
            case 16: nFormat = Format::SIXTEEN_BIT_MSB_TC_MASK; break;
#else
            case 16: nFormat = Format::SIXTEEN_BIT_LSB_TC_MASK; break;
#endif
            case 0:
            case 24: nFormat = Format::TWENTYFOUR_BIT_TC_MASK; break;
            case 32: nFormat = Format::THIRTYTWO_BIT_TC_MASK; break;
        }
        m_aDevice = aDevPal.empty()
                    ? createBitmapDevice( aDevSize, false, nFormat )
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
