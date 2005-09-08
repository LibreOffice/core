/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implcolor.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:27:21 $
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

#ifndef _CPPCANVAS_IMPLCOLOR_HXX
#define _CPPCANVAS_IMPLCOLOR_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _COM_SUN_STAR_RENDERING_XGRAPHICDEVICE_HPP__
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#endif

#ifndef _CPPCANVAS_COLOR_HXX
#include <cppcanvas/color.hxx>
#endif


/* Definition of Color class */

namespace cppcanvas
{
    namespace internal
    {
        class ImplColor : public Color
        {
        public:
            ImplColor( const ::com::sun::star::uno::Reference<
                               ::com::sun::star::rendering::XGraphicDevice >& rDevice );
            virtual ~ImplColor();

            virtual IntSRGBA                                    getIntSRGBA( ::com::sun::star::uno::Sequence< double >& rDeviceColor ) const;
            virtual ::com::sun::star::uno::Sequence< double >   getDeviceColor( IntSRGBA aSRGBA ) const;

        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XGraphicDevice > mxDevice;
        };

    }
}

#endif /* _CPPCANVAS_IMPLCOLOR_HXX */
