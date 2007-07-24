/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svpvd.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:29:08 $
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

#ifndef _SVP_SVPVD_HXX
#define _SVP_SVPVD_HXX

#include <vcl/salvd.hxx>
#include "svpelement.hxx"

#include <list>

class SvpSalGraphics;

class SvpSalVirtualDevice : public SalVirtualDevice, public SvpElement
{
    USHORT                              m_nBitCount;
    basebmp::BitmapDeviceSharedPtr      m_aDevice;
    std::list< SvpSalGraphics* >        m_aGraphics;

public:
    SvpSalVirtualDevice( USHORT nBitCount ) : SvpElement(), m_nBitCount(nBitCount) {}
    virtual ~SvpSalVirtualDevice();

    // SvpElement
    virtual const basebmp::BitmapDeviceSharedPtr& getDevice() const { return m_aDevice; }

    // SalVirtualDevice
    virtual SalGraphics*    GetGraphics();
    virtual void            ReleaseGraphics( SalGraphics* pGraphics );

    virtual BOOL            SetSize( long nNewDX, long nNewDY );
    virtual void            GetSize( long& rWidth, long& rHeight );
};

#endif // _SVP_SVPVD_HXX
