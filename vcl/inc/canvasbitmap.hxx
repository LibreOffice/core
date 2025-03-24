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

#pragma once

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>
#include <com/sun/star/rendering/XBitmapPalette.hpp>

#include <vcl/bitmapex.hxx>
#include <vcl/BitmapReadAccess.hxx>

namespace vcl::unotools
{
    class SAL_DLLPUBLIC_RTTI VclCanvasBitmap final :
        public cppu::WeakImplHelper< css::rendering::XIntegerReadOnlyBitmap,
                                     css::rendering::XBitmapPalette,
                                     css::rendering::XIntegerBitmapColorSpace >
    {
    private:
        BitmapEx                                       m_aBmpEx;
        ::Bitmap                                       m_aBitmap;
        ::Bitmap                                       m_aAlpha;
        BitmapScopedInfoAccess                         m_pBmpAcc;
        BitmapScopedInfoAccess                         m_pAlphaAcc;
        std::optional<BitmapScopedReadAccess>          m_pBmpReadAcc;
        std::optional<BitmapScopedReadAccess>          m_pAlphaReadAcc;
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
        bool                                           m_bPalette;

        void setComponentInfo( sal_uInt32 redShift, sal_uInt32 greenShift, sal_uInt32 blueShift );
        BitmapScopedReadAccess& getBitmapReadAccess();
        BitmapScopedReadAccess& getAlphaReadAccess();

        virtual ~VclCanvasBitmap() override;

    public:
        // XBitmap
        VCL_DLLPUBLIC virtual css::geometry::IntegerSize2D SAL_CALL getSize() override;
        VCL_DLLPUBLIC virtual sal_Bool SAL_CALL hasAlpha(  ) override;
        VCL_DLLPUBLIC virtual css::uno::Reference< css::rendering::XBitmap > SAL_CALL getScaledBitmap( const css::geometry::RealSize2D& newSize, sal_Bool beFast ) override;

        // XIntegerReadOnlyBitmap
        VCL_DLLPUBLIC virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getData( css::rendering::IntegerBitmapLayout& bitmapLayout, const css::geometry::IntegerRectangle2D& rect ) override;
        VCL_DLLPUBLIC virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL getPixel( css::rendering::IntegerBitmapLayout& bitmapLayout, const css::geometry::IntegerPoint2D& pos ) override;
        /// @throws css::uno::RuntimeException
        VCL_DLLPUBLIC css::uno::Reference< css::rendering::XBitmapPalette > getPalette(  );
        virtual css::rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout(  ) override;

        // XBitmapPalette
        virtual sal_Int32 SAL_CALL getNumberOfEntries() override;
        virtual sal_Bool SAL_CALL getIndex( css::uno::Sequence< double >& entry, ::sal_Int32 nIndex ) override;
        virtual sal_Bool SAL_CALL setIndex( const css::uno::Sequence< double >& color, sal_Bool transparency, ::sal_Int32 nIndex ) override;
        virtual css::uno::Reference< css::rendering::XColorSpace > SAL_CALL getColorSpace(  ) override;

        // XIntegerBitmapColorSpace
        virtual ::sal_Int8 SAL_CALL getType(  ) override;
        virtual css::uno::Sequence< sal_Int8 > SAL_CALL getComponentTags(  ) override;
        virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getProperties(  ) override;
        virtual css::uno::Sequence< double > SAL_CALL convertColorSpace( const css::uno::Sequence< double >& deviceColor, const css::uno::Reference< css::rendering::XColorSpace >& targetColorSpace ) override;
        virtual css::uno::Sequence< css::rendering::RGBColor > SAL_CALL convertToRGB( const css::uno::Sequence< double >& deviceColor ) override;
        virtual css::uno::Sequence< css::rendering::ARGBColor > SAL_CALL convertToARGB( const css::uno::Sequence< double >& deviceColor ) override;
        virtual css::uno::Sequence< css::rendering::ARGBColor > SAL_CALL convertToPARGB( const css::uno::Sequence< double >& deviceColor ) override;
        virtual css::uno::Sequence< double > SAL_CALL convertFromRGB( const css::uno::Sequence< css::rendering::RGBColor >& rgbColor ) override;
        virtual css::uno::Sequence< double > SAL_CALL convertFromARGB( const css::uno::Sequence< css::rendering::ARGBColor >& rgbColor ) override;
        virtual css::uno::Sequence< double > SAL_CALL convertFromPARGB( const css::uno::Sequence< css::rendering::ARGBColor >& rgbColor ) override;
        virtual ::sal_Int32 SAL_CALL getBitsPerPixel(  ) override;
        virtual css::uno::Sequence< ::sal_Int32 > SAL_CALL getComponentBitCounts(  ) override;
        virtual ::sal_Int8 SAL_CALL getEndianness(  ) override;
        virtual css::uno::Sequence<double> SAL_CALL convertFromIntegerColorSpace( const css::uno::Sequence< ::sal_Int8 >& deviceColor, const css::uno::Reference< css::rendering::XColorSpace >& targetColorSpace ) override;
        virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL convertToIntegerColorSpace( const css::uno::Sequence< ::sal_Int8 >& deviceColor, const css::uno::Reference< css::rendering::XIntegerBitmapColorSpace >& targetColorSpace ) override;
        VCL_DLLPUBLIC virtual css::uno::Sequence< css::rendering::RGBColor > SAL_CALL convertIntegerToRGB( const css::uno::Sequence< ::sal_Int8 >& deviceColor ) override;
        VCL_DLLPUBLIC virtual css::uno::Sequence< css::rendering::ARGBColor > SAL_CALL convertIntegerToARGB( const css::uno::Sequence< ::sal_Int8 >& deviceColor ) override;
        virtual css::uno::Sequence< css::rendering::ARGBColor > SAL_CALL convertIntegerToPARGB( const css::uno::Sequence< ::sal_Int8 >& deviceColor ) override;
        VCL_DLLPUBLIC virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromRGB( const css::uno::Sequence< css::rendering::RGBColor >& rgbColor ) override;
        VCL_DLLPUBLIC virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromARGB( const css::uno::Sequence< css::rendering::ARGBColor >& rgbColor ) override;
        virtual css::uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromPARGB( const css::uno::Sequence< css::rendering::ARGBColor >& rgbColor ) override;

        /** Create API wrapper for given BitmapEx

            @param rBitmap
            Bitmap to wrap. As usual, changes to the original bitmap
            are not reflected in this object (copy on write).
         */
        VCL_DLLPUBLIC  explicit VclCanvasBitmap( const BitmapEx& rBitmap );

        /// Retrieve contained bitmap. Call me with locked Solar mutex!
        const BitmapEx& getBitmapEx() const { return m_aBmpEx; }
   };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
