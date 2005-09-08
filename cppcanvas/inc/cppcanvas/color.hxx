/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: color.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:13:34 $
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

#ifndef _CPPCANVAS_COLOR_HXX
#define _CPPCANVAS_COLOR_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif


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
