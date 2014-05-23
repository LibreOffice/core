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

#ifndef IOS

#include "headless/svpbmp.hxx"
#include "headless/svpinst.hxx"
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

SalGraphics* SvpSalVirtualDevice::AcquireGraphics()
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

bool SvpSalVirtualDevice::SetSize( long nNewDX, long nNewDY )
{
    return SetSizeUsingBuffer( nNewDX, nNewDY, basebmp::RawMemorySharedArray() );
}

bool SvpSalVirtualDevice::SetSizeUsingBuffer( long nNewDX, long nNewDY, const basebmp::RawMemorySharedArray &pBuffer )
{
    B2IVector aDevSize( nNewDX, nNewDY );
    if( aDevSize.getX() == 0 )
        aDevSize.setX( 1 );
    if( aDevSize.getY() == 0 )
        aDevSize.setY( 1 );
    if( ! m_aDevice.get() || m_aDevice->getSize() != aDevSize )
    {
        SvpSalInstance* pInst = SvpSalInstance::s_pDefaultInstance;
        assert( pInst );
        basebmp::Format nFormat = pInst->getFormatForBitCount( m_nBitCount );

        if ( m_nBitCount == 1 )
        {
            std::vector< basebmp::Color > aDevPal(2);
            aDevPal.push_back( basebmp::Color( 0, 0, 0 ) );
            aDevPal.push_back( basebmp::Color( 0xff, 0xff, 0xff ) );
            m_aDevice = createBitmapDevice( aDevSize, false, nFormat, PaletteMemorySharedVector( new std::vector< basebmp::Color >(aDevPal) ) );
        }
        else
        {
            m_aDevice = pBuffer ?
                          createBitmapDevice( aDevSize, false, nFormat, pBuffer, PaletteMemorySharedVector() )
                        : createBitmapDevice( aDevSize, false, nFormat );
        }

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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
