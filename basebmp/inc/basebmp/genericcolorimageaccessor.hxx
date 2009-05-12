/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: genericcolorimageaccessor.hxx,v $
 * $Revision: 1.3 $
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
