/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svpelement.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:26:55 $
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

#ifndef _SVP_SVPELEMENT_HXX
#define _SVP_SVPELEMENT_HXX

#ifndef INCLUDED_BASEBMP_BITMAPDEVICE_HXX
#include <basebmp/bitmapdevice.hxx>
#endif

#define SVP_DEFAULT_BITMAP_FORMAT basebmp::Format::TWENTYFOUR_BIT_TC_MASK

class SvpElement
{
    protected:
    SvpElement();
    virtual ~SvpElement();
    public:
    virtual const basebmp::BitmapDeviceSharedPtr& getDevice() const = 0;

    static sal_uInt32 getBitCountFromScanlineFormat( sal_Int32 nFormat );
};

#endif
