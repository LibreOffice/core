/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _VCL_CANVASBITMAP_HXX
#define _VCL_CANVASBITMAP_HXX

#include <cppuhelper/compbase3.hxx>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XBitmapPalette.hpp>

#include <vcl/bitmapex.hxx>
#include <vcl/bmpacc.hxx>

namespace vcl
{
namespace unotools
{
    class VCL_DLLPUBLIC VclCanvasBitmap :
        public cppu::WeakImplHelper3< com::sun::star::rendering::XIntegerReadOnlyBitmap,
                                      com::sun::star::rendering::XBitmapPalette,
                                      com::sun::star::rendering::XIntegerBitmapColorSpace >
    {
    private:
        BitmapEx                                       m_aBmpEx;
        Bitmap                                         m_aBitmap;
        Bitmap                                         m_aAlpha;
        BitmapReadAccess*                              m_pBmpAcc;
        BitmapReadAccess*                              m_pAlphaAcc;
        com::sun::star::uno::Sequence<sal_Int8>        m_aComponentTags;
        com::sun::star::uno::Sequence<sal_Int32>       m_aComponentBitCounts;
        com::sun::star::rendering::IntegerBitmapLayout m_aLayout;
        sal_Int32                                      m_nBitsPerInputPixel;
        sal_Int32                                      m_nBitsPerOutputPixel;
        sal_Int32                                      m_nRedIndex;
        sal_Int32                                      m_nGreenIndex;
        sal_Int32                                      m_nBlueIndex;
        sal_Int32                                      m_nAlphaIndex;
        sal_Int32                                      m_nIndexIndex;
        sal_Int8                                       m_nEndianness;
        bool                                           m_bPalette;

        SAL_DLLPRIVATE void setComponentInfo( sal_uLong redShift, sal_uLong greenShift, sal_uLong blueShift );

        virtual ~VclCanvasBitmap();

    public:
        // XBitmap
        virtual com::sun::star::geometry::IntegerSize2D SAL_CALL getSize() throw (com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL hasAlpha(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual com::sun::star::uno::Reference< com::sun::star::rendering::XBitmap > SAL_CALL getScaledBitmap( const com::sun::star::geometry::RealSize2D& newSize, sal_Bool beFast ) throw (com::sun::star::uno::RuntimeException);

        // XIntegerReadOnlyBitmap
        virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getData( ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout, const ::com::sun::star::geometry::IntegerRectangle2D& rect ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::rendering::VolatileContentDestroyedException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL getPixel( ::com::sun::star::rendering::IntegerBitmapLayout& bitmapLayout, const ::com::sun::star::geometry::IntegerPoint2D& pos ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::rendering::VolatileContentDestroyedException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XBitmapPalette > SAL_CALL getPalette(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout(  ) throw (::com::sun::star::uno::RuntimeException);

        // XBitmapPalette
        virtual sal_Int32 SAL_CALL getNumberOfEntries() throw (com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getIndex( ::com::sun::star::uno::Sequence< double >& entry, ::sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL setIndex( const ::com::sun::star::uno::Sequence< double >& color, ::sal_Bool transparency, ::sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XColorSpace > SAL_CALL getColorSpace(  ) throw (::com::sun::star::uno::RuntimeException);

        // XIntegerBitmapColorSpace
        virtual ::sal_Int8 SAL_CALL getType(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getComponentTags(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getProperties(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL convertColorSpace( const ::com::sun::star::uno::Sequence< double >& deviceColor, const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XColorSpace >& targetColorSpace ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::RGBColor > SAL_CALL convertToRGB( const ::com::sun::star::uno::Sequence< double >& deviceColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::ARGBColor > SAL_CALL convertToARGB( const ::com::sun::star::uno::Sequence< double >& deviceColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::ARGBColor > SAL_CALL convertToPARGB( const ::com::sun::star::uno::Sequence< double >& deviceColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL convertFromRGB( const ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::RGBColor >& rgbColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL convertFromARGB( const ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::ARGBColor >& rgbColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< double > SAL_CALL convertFromPARGB( const ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::ARGBColor >& rgbColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int32 SAL_CALL getBitsPerPixel(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::sal_Int32 > SAL_CALL getComponentBitCounts(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Int8 SAL_CALL getEndianness(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence<double> SAL_CALL convertFromIntegerColorSpace( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& deviceColor, const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XColorSpace >& targetColorSpace ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL convertToIntegerColorSpace( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& deviceColor, const ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XIntegerBitmapColorSpace >& targetColorSpace ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::RGBColor > SAL_CALL convertIntegerToRGB( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& deviceColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::ARGBColor > SAL_CALL convertIntegerToARGB( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& deviceColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::ARGBColor > SAL_CALL convertIntegerToPARGB( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& deviceColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromRGB( const ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::RGBColor >& rgbColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromARGB( const ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::ARGBColor >& rgbColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromPARGB( const ::com::sun::star::uno::Sequence< ::com::sun::star::rendering::ARGBColor >& rgbColor ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

        /** Create API wrapper for given BitmapEx

            @param rBitmap
            Bitmap to wrap. As usual, changes to the original bitmap
            are not reflected in this object (copy on write).
         */
        explicit VclCanvasBitmap( const BitmapEx& rBitmap );

        /// Retrieve contained bitmap. Call me with locked Solar mutex!
        BitmapEx getBitmapEx() const;
   };
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
