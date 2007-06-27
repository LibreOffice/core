/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: genericcolorimageaccessor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 12:40:23 $
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

#ifndef INCLUDED_BASEBMP_GENERICCOLORIMAGEACCESSOR_HXX
#define INCLUDED_BASEBMP_GENERICCOLORIMAGEACCESSOR_HXX

#include <basebmp/color.hxx>
#include <basebmp/bitmapdevice.hxx>

namespace basebmp
{
    /** Access a BitmapDevice generically

        This accessor deals with an opaque BitmapDevice generically,
        via getPixel()/setPixel() at the published interface.
     */
    class GenericColorImageAccessor
    {
        BitmapDeviceSharedPtr mpDevice;
        DrawMode              meDrawMode;

    public:
        typedef Color value_type;

        explicit GenericColorImageAccessor( BitmapDeviceSharedPtr const& rTarget ) :
            mpDevice(rTarget),
            meDrawMode(DrawMode_PAINT)
        {}

        GenericColorImageAccessor( BitmapDeviceSharedPtr const& rTarget,
                                   DrawMode                     eDrawMode ) :
            mpDevice(rTarget),
            meDrawMode(eDrawMode)
        {}

        template< typename Iterator >
        Color operator()( Iterator const& i ) const
        { return mpDevice->getPixel( basegfx::B2IPoint( i->x,i->y ) ); }

        template< typename Iterator, typename Difference >
        Color operator()( Iterator const& i, Difference const& diff) const
        { return mpDevice->getPixel( basegfx::B2IPoint( i[diff]->x,
                                                        i[diff]->y ) ); }

        template< typename Iterator >
        void set(Color const& value, Iterator const& i) const
        { return mpDevice->setPixel( basegfx::B2IPoint( i->x,i->y ),
                                     value, meDrawMode ); }

        template< class Iterator, class Difference >
        void set(value_type const& value, Iterator const& i, Difference const& diff) const
        { return mpDevice->setPixel( basegfx::B2IPoint( i[diff]->x,
                                                        i[diff]->y ),
                                     value, meDrawMode ); }
    };
}

#endif /* INCLUDED_BASEBMP_GENERICCOLORIMAGEACCESSOR_HXX */
