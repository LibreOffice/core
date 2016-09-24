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

#include <cairo.h>

using namespace basegfx;

SvpSalVirtualDevice::~SvpSalVirtualDevice()
{
    cairo_surface_destroy(m_pSurface);
}

SalGraphics* SvpSalVirtualDevice::AcquireGraphics()
{
    SvpSalGraphics* pGraphics = new SvpSalGraphics();
    pGraphics->setSurface(m_pSurface);
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
    return SetSizeUsingBuffer(nNewDX, nNewDY, nullptr);
}

bool SvpSalVirtualDevice::SetSizeUsingBuffer( long nNewDX, long nNewDY,
        sal_uInt8 *const pBuffer)
{
    B2IVector aDevSize( nNewDX, nNewDY );
    if( aDevSize.getX() == 0 )
        aDevSize.setX( 1 );
    if( aDevSize.getY() == 0 )
        aDevSize.setY( 1 );

    if (!m_pSurface || cairo_image_surface_get_width(m_pSurface) != aDevSize.getX() ||
                       cairo_image_surface_get_height(m_pSurface) != aDevSize.getY() )
    {
        if (m_pSurface)
        {
            cairo_surface_destroy(m_pSurface);
        }

        if (m_eFormat == DeviceFormat::BITMASK)
        {
            m_pSurface = cairo_image_surface_create(CAIRO_FORMAT_A1,
                                aDevSize.getX(),
                                aDevSize.getY());
        }
        else
        {
            m_pSurface = pBuffer ?
                             cairo_image_surface_create_for_data(pBuffer, CAIRO_FORMAT_ARGB32,
                                   aDevSize.getX(),
                                   aDevSize.getY(),
                                   cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, aDevSize.getX()))
                                 :
                             cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                   aDevSize.getX(),
                                   aDevSize.getY());
        }

        // update device in existing graphics
        for( std::list< SvpSalGraphics* >::iterator it = m_aGraphics.begin();
             it != m_aGraphics.end(); ++it )
            (*it)->setSurface(m_pSurface);

    }
    return true;
}

long SvpSalVirtualDevice::GetWidth() const
{
    return m_pSurface ? cairo_image_surface_get_width(m_pSurface) : 0;
}

long SvpSalVirtualDevice::GetHeight() const
{
    return m_pSurface ? cairo_image_surface_get_height(m_pSurface) : 0;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
