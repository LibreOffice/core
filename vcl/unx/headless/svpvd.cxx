/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svpvd.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:28:59 $
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

#include "svpvd.hxx"
#include "svpgdi.hxx"

#include <basegfx/vector/b2ivector.hxx>
#include <basebmp/scanlineformats.hxx>

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

BOOL SvpSalVirtualDevice::SetSize( long nNewDX, long nNewDY )
{
    B2IVector aDevSize( nNewDX, nNewDY );
    if( aDevSize.getX() == 0 )
        aDevSize.setX( 1 );
    if( aDevSize.getY() == 0 )
        aDevSize.setY( 1 );
    if( ! m_aDevice.get() || m_aDevice->getSize() != aDevSize )
    {
        sal_uInt32 nFormat = SVP_DEFAULT_BITMAP_FORMAT;
        switch( m_nBitCount )
        {
            case 1: nFormat = Format::ONE_BIT_MSB_PAL; break;
            case 4: nFormat = Format::FOUR_BIT_MSB_PAL; break;
            case 8: nFormat = Format::EIGHT_BIT_PAL; break;
#ifdef OSL_BIGENDIAN
            case 16: nFormat = Format::SIXTEEN_BIT_MSB_TC_MASK; break;
#else
            case 16: nFormat = Format::SIXTEEN_BIT_LSB_TC_MASK; break;
#endif
            case 24: nFormat = Format::TWENTYFOUR_BIT_TC_MASK; break;
            case 32: nFormat = Format::THIRTYTWO_BIT_TC_MASK; break;
        }
        m_aDevice = createBitmapDevice( aDevSize, false, nFormat );
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

