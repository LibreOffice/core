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

#ifndef INCLUDED_VCL_INC_CANVASBITMAP_HXX
#define INCLUDED_VCL_INC_CANVASBITMAP_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XBitmapPalette.hpp>

#include <vcl/bitmapex.hxx>
#include <vcl/bitmapaccess.hxx>

namespace vcl
{
namespace unotools
{
    class VCL_DLLPUBLIC VclCanvasBitmap :
        public cppu::WeakImplHelper< css::rendering::XIntegerReadOnlyBitmap,
                                      css::rendering::XBitmapPalette,
                                      css::rendering::XIntegerBitmapColorSpace >
    {
    private:
        BitmapEx                                       m_aBmpEx;
        ::Bitmap                                       m_aBitmap;
        ::Bitmap                                       m_aAlpha;
        BitmapReadAccess*                              m_pBmpAcc;
        BitmapReadAccess*                              m_pAlphaAcc;
        css::uno::Sequence<sal_Int8>                   m_aComponentTags;
        css::uno::Sequence<sal_Int32>                  m_aComponentBitCounts;
        css::rendering::IntegerBitmapLayout            m_aLayout;
        sal_Int32                                      m_nBitsPerInputPixel;
        sal_Int32                                      m_nBitsPerOutputPixel;
        sal_Int32                                      m_nRedIndex;
        sal_Int32                                      m_nGreenIndex;
        sal_Int32                                      m_nBlueIndex;
        sal_Int32                                      m_nAlphaIndex;
        sal_Int32                                      m_nIndexIndex;
        sal_Int8                                       m_nEndianness;
        bool                                           m_bPalette;

        SAL_DLLPRIVATE void setComponentInfo( sal_uInt32 redShift, sal_uInt32 greenShift, sal_uInt32 blueShift );

        virtual ~VclCanvasBitmap();

    public:
        // XBitmap
        virtual css::geometry::IntegerSize2D SAL_CALL getSize() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL hasAlpha(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::rendering::XBitmap > SAL_CALL getScaledBitmap( const css::geometry::RealSize2D& newSize, sal_Bool beFast ) throw (css::uno::RuntimeException, std::exception) override;

        // XIntegerReadOnlyBitmap
        virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getData( css::rendering::IntegerBitmapLayout& bitmapLayout, const css::geometry::IntegerRectangle2D& rect ) throw (css::lang::IndexOutOfBoundsException, css::rendering::VolatileContentDestroyedException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getPixel( css::rendering::IntegerBitmapLayout& bitmapLayout, const css::geometry::IntegerPoint2D& pos ) throw (css::lang::IndexOutOfBoundsException, css::rendering::VolatileContentDestroyedException, css::uno::RuntimeException, std::exception) override;
        css::uno::Reference< css::rendering::XBitmapPalette > SAL_CALL getPalette(  ) throw (css::uno::RuntimeException);
        virtual css::rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XBitmapPalette
        virtual sal_Int32 SAL_CALL getNumberOfEntries() throw (css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL getIndex( css::uno::Sequence< double >& entry, ::sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::uno::RuntimeException, std::exception) override;
        virtual sal_Bool SAL_CALL setIndex( const css::uno::Sequence< double >& color, sal_Bool transparency, ::sal_Int32 nIndex ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Reference< css::rendering::XColorSpace > SAL_CALL getColorSpace(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XIntegerBitmapColorSpace
        virtual ::sal_Int8 SAL_CALL getType(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getComponentTags(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getProperties(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< double > SAL_CALL convertColorSpace( const css::uno::Sequence< double >& deviceColor, const css::uno::Reference< css::rendering::XColorSpace >& targetColorSpace ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::rendering::RGBColor > SAL_CALL convertToRGB( const css::uno::Sequence< double >& deviceColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::rendering::ARGBColor > SAL_CALL convertToARGB( const css::uno::Sequence< double >& deviceColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::rendering::ARGBColor > SAL_CALL convertToPARGB( const css::uno::Sequence< double >& deviceColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< double > SAL_CALL convertFromRGB( const css::uno::Sequence< css::rendering::RGBColor >& rgbColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< double > SAL_CALL convertFromARGB( const css::uno::Sequence< css::rendering::ARGBColor >& rgbColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< double > SAL_CALL convertFromPARGB( const css::uno::Sequence< css::rendering::ARGBColor >& rgbColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int32 SAL_CALL getBitsPerPixel(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< ::sal_Int32 > SAL_CALL getComponentBitCounts(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual ::sal_Int8 SAL_CALL getEndianness(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence<double> SAL_CALL convertFromIntegerColorSpace( const css::uno::Sequence< ::sal_Int8 >& deviceColor, const css::uno::Reference< css::rendering::XColorSpace >& targetColorSpace ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL convertToIntegerColorSpace( const css::uno::Sequence< ::sal_Int8 >& deviceColor, const css::uno::Reference< css::rendering::XIntegerBitmapColorSpace >& targetColorSpace ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::rendering::RGBColor > SAL_CALL convertIntegerToRGB( const css::uno::Sequence< ::sal_Int8 >& deviceColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::rendering::ARGBColor > SAL_CALL convertIntegerToARGB( const css::uno::Sequence< ::sal_Int8 >& deviceColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::rendering::ARGBColor > SAL_CALL convertIntegerToPARGB( const css::uno::Sequence< ::sal_Int8 >& deviceColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromRGB( const css::uno::Sequence< css::rendering::RGBColor >& rgbColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromARGB( const css::uno::Sequence< css::rendering::ARGBColor >& rgbColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromPARGB( const css::uno::Sequence< css::rendering::ARGBColor >& rgbColor ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;

        /** Create API wrapper for given BitmapEx

            @param rBitmap
            Bitmap to wrap. As usual, changes to the original bitmap
            are not reflected in this object (copy on write).
         */
        explicit VclCanvasBitmap( const BitmapEx& rBitmap );

        /// Retrieve contained bitmap. Call me with locked Solar mutex!
        BitmapEx getBitmapEx() const { return m_aBmpEx;}
   };
}
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
