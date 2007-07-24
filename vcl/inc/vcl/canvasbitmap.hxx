/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: canvasbitmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 09:58:09 $
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

#ifndef _VCL_CANVASBITMAP_HXX
#define _VCL_CANVASBITMAP_HXX

#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XINTEGERBITMAP_HPP_
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#endif
#ifndef _COM_SUN_STAR_RENDERING_XBITMAPPALETTE_HPP_
#include <com/sun/star/rendering/XBitmapPalette.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _SV_BITMAPEX_HXX
#include <vcl/bitmapex.hxx>
#endif

namespace vcl
{
namespace unotools
{
    class VclCanvasBitmap : public
        cppu::WeakImplHelper3< com::sun::star::rendering::XIntegerBitmap,
                               com::sun::star::rendering::XBitmapPalette,
                               com::sun::star::lang::XUnoTunnel>
    {
        BitmapEx*                                           m_pBitmap;
        com::sun::star::rendering::IntegerBitmapLayout      m_aLayout;
        bool                                                m_bHavePalette;

    public:
        VclCanvasBitmap( const BitmapEx& rBitmap );
        virtual ~VclCanvasBitmap();

        // XBitmap
        virtual com::sun::star::geometry::IntegerSize2D SAL_CALL getSize() throw (com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasAlpha(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual com::sun::star::uno::Reference< com::sun::star::rendering::XBitmapCanvas > SAL_CALL queryBitmapCanvas() throw (com::sun::star::uno::RuntimeException);
        virtual com::sun::star::uno::Reference< com::sun::star::rendering::XBitmap > SAL_CALL getScaledBitmap( const com::sun::star::geometry::RealSize2D& newSize, sal_Bool beFast ) throw (com::sun::star::uno::RuntimeException);

        // XIntegerBitmap
        virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getData( ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout, const ::com::sun::star::geometry::IntegerRectangle2D& rect ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::rendering::VolatileContentDestroyedException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& data, const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout, const ::com::sun::star::geometry::IntegerRectangle2D& rect ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPixel( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& color, const ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout, const ::com::sun::star::geometry::IntegerPoint2D& pos ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getPixel( ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout, const ::com::sun::star::geometry::IntegerPoint2D& pos ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::rendering::VolatileContentDestroyedException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapPalette > SAL_CALL getPalette(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout(  ) throw (::com::sun::star::uno::RuntimeException);

        // XBitmapPalette
        virtual sal_Int32 SAL_CALL getNumberOfEntries() throw (com::sun::star::uno::RuntimeException);
        virtual com::sun::star::uno::Sequence< double > SAL_CALL getPaletteIndex( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL setPaletteIndex( const com::sun::star::uno::Sequence< double >& color, ::sal_Int32 nIndex ) throw (com::sun::star::lang::IndexOutOfBoundsException, com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XColorSpace > SAL_CALL getColorSpace(  ) throw (::com::sun::star::uno::RuntimeException);

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw (com::sun::star::uno::RuntimeException);
   };
}
}

#endif
