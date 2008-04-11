/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: color.hxx,v $
 * $Revision: 1.6 $
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

#ifndef _CPPCANVAS_COLOR_HXX
#define _CPPCANVAS_COLOR_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <boost/shared_ptr.hpp>


/* Definition of Color class */

namespace cppcanvas
{
    class Color
    {
    public:
        /** Color in the sRGB color space, plus alpha channel

            The four bytes of the sal_uInt32 are allocated as follows
            to the color channels and alpha: 0xRRGGBBAA.
         */
        typedef sal_uInt32 IntSRGBA;

        virtual ~Color() {}

        virtual IntSRGBA                                    getIntSRGBA( ::com::sun::star::uno::Sequence< double >& rDeviceColor ) const = 0;
        virtual ::com::sun::star::uno::Sequence< double >   getDeviceColor( IntSRGBA aSRGBA ) const = 0;
    };

    typedef ::boost::shared_ptr< ::cppcanvas::Color > ColorSharedPtr;

    inline sal_uInt8 getRed( Color::IntSRGBA nCol )
    {
        return static_cast<sal_uInt8>( (nCol&0xFF000000U) >> 24U );
    }

    inline sal_uInt8 getGreen( Color::IntSRGBA nCol )
    {
        return static_cast<sal_uInt8>( (nCol&0x00FF0000U) >> 16U );
    }

    inline sal_uInt8 getBlue( Color::IntSRGBA nCol )
    {
        return static_cast<sal_uInt8>( (nCol&0x0000FF00U) >> 8U );
    }

    inline sal_uInt8 getAlpha( Color::IntSRGBA nCol )
    {
        return static_cast<sal_uInt8>( nCol&0x000000FFU );
    }

    inline Color::IntSRGBA makeColor( sal_uInt8 nRed, sal_uInt8 nGreen, sal_uInt8 nBlue, sal_uInt8 nAlpha )
    {
        return (nRed << 24U)|(nGreen << 16U)|(nBlue << 8U)|(nAlpha);
    }

}

#endif /* _CPPCANVAS_COLOR_HXX */
