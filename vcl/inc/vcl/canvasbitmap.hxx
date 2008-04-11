/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvasbitmap.hxx,v $
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

#ifndef _VCL_CANVASBITMAP_HXX
#define _VCL_CANVASBITMAP_HXX

#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XBitmapPalette.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <vcl/bitmapex.hxx>

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
