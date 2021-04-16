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

#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>

#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>
#include <com/sun/star/rendering/VolatileContentDestroyedException.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/range/b2irectangle.hxx>

#include <sal/log.hxx>
#include <tools/helpers.hxx>
#include <tools/diagnose_ex.h>

#include <vcl/bitmapex.hxx>

#include <canvasbitmap.hxx>
#include <vcl/canvastools.hxx>
#include <bitmap/BitmapWriteAccess.hxx>

using namespace ::com::sun::star;

namespace vcl::unotools
{
        uno::Reference< rendering::XBitmap > xBitmapFromBitmapEx(const ::BitmapEx& inputBitmap )
        {
            SAL_INFO( "vcl.helper", "vcl::unotools::xBitmapFromBitmapEx()" );

            return new vcl::unotools::VclCanvasBitmap( inputBitmap );
        }

        namespace
        {
            bool equalsLayout( const rendering::IntegerBitmapLayout& rLHS,
                                    const rendering::IntegerBitmapLayout& rRHS )
            {
                return
                    rLHS.ScanLineBytes       == rRHS.ScanLineBytes &&
                    rLHS.ScanLineStride      == rRHS.ScanLineStride &&
                    rLHS.PlaneStride         == rRHS.PlaneStride &&
                    rLHS.ColorSpace          == rRHS.ColorSpace &&
                    rLHS.Palette             == rRHS.Palette &&
                    rLHS.IsMsbFirst          == rRHS.IsMsbFirst;
            }
            bool readBmp( sal_Int32                                                  nWidth,
                          sal_Int32                                                  nHeight,
                          const rendering::IntegerBitmapLayout&                      rLayout,
                          const uno::Reference< rendering::XIntegerReadOnlyBitmap >& xInputBitmap,
                          BitmapScopedWriteAccess&                                   rWriteAcc,
                          BitmapScopedWriteAccess&                                   rAlphaAcc )
            {
                rendering::IntegerBitmapLayout      aCurrLayout;
                geometry::IntegerRectangle2D        aRect;
                uno::Sequence<sal_Int8>             aPixelData;
                uno::Sequence<rendering::RGBColor>  aRGBColors;
                uno::Sequence<rendering::ARGBColor> aARGBColors;

                for( aRect.Y1=0; aRect.Y1<nHeight; ++aRect.Y1 )
                {
                    aRect.X1 = 0; aRect.X2 = nWidth; aRect.Y2 = aRect.Y1+1;
                    try
                    {
                        aPixelData = xInputBitmap->getData(aCurrLayout,aRect);
                    }
                    catch( rendering::VolatileContentDestroyedException& )
                    {
                        // re-read bmp from the start
                        return false;
                    }
                    if( !equalsLayout(aCurrLayout, rLayout) )
                        return false; // re-read bmp from the start

                    Scanline pScanline = rWriteAcc->GetScanline( aRect.Y1 );
                    if( rAlphaAcc.get() )
                    {
                        Scanline pScanlineAlpha = rAlphaAcc->GetScanline( aRect.Y1 );
                        // read ARGB color
                        aARGBColors = rLayout.ColorSpace->convertIntegerToARGB(aPixelData);

                        if( rWriteAcc->HasPalette() )
                        {
                            for( sal_Int32 x=0; x<nWidth; ++x )
                            {
                                const rendering::ARGBColor& rColor=aARGBColors[x];
                                rWriteAcc->SetPixelOnData( pScanline, x,
                                                     BitmapColor(static_cast<sal_uInt8>(rWriteAcc->GetBestPaletteIndex(
                                                         BitmapColor( toByteColor(rColor.Red),
                                                                      toByteColor(rColor.Green),
                                                                      toByteColor(rColor.Blue))))) );
                                rAlphaAcc->SetPixelOnData( pScanlineAlpha, x,
                                                     BitmapColor( toByteColor(rColor.Alpha) ));
                            }
                        }
                        else
                        {
                            for( sal_Int32 x=0; x<nWidth; ++x )
                            {
                                const rendering::ARGBColor& rColor=aARGBColors[x];
                                rWriteAcc->SetPixelOnData( pScanline, x,
                                                     BitmapColor( toByteColor(rColor.Red),
                                                                  toByteColor(rColor.Green),
                                                                  toByteColor(rColor.Blue) ));
                                rAlphaAcc->SetPixelOnData( pScanlineAlpha, x,
                                                     BitmapColor( toByteColor(rColor.Alpha) ));
                            }
                        }
                    }
                    else
                    {
                        // read RGB color
                        aRGBColors = rLayout.ColorSpace->convertIntegerToRGB(aPixelData);
                        if( rWriteAcc->HasPalette() )
                        {
                            for( sal_Int32 x=0; x<nWidth; ++x )
                            {
                                const rendering::RGBColor& rColor=aRGBColors[x];
                                rWriteAcc->SetPixelOnData( pScanline, x,
                                                     BitmapColor(static_cast<sal_uInt8>(rWriteAcc->GetBestPaletteIndex(
                                                         BitmapColor( toByteColor(rColor.Red),
                                                                      toByteColor(rColor.Green),
                                                                      toByteColor(rColor.Blue))))) );
                            }
                        }
                        else
                        {
                            for( sal_Int32 x=0; x<nWidth; ++x )
                            {
                                const rendering::RGBColor& rColor=aRGBColors[x];
                                rWriteAcc->SetPixelOnData( pScanline, x,
                                                     BitmapColor( toByteColor(rColor.Red),
                                                                  toByteColor(rColor.Green),
                                                                  toByteColor(rColor.Blue) ));
                            }
                        }
                    }
                }

                return true;
            }
        }

        ::BitmapEx bitmapExFromXBitmap( const uno::Reference< rendering::XIntegerReadOnlyBitmap >& xInputBitmap )
        {
            SAL_INFO( "vcl.helper", "vcl::unotools::bitmapExFromXBitmap()" );

            if( !xInputBitmap.is() )
                return ::BitmapEx();

            // tunnel directly for known implementation
            VclCanvasBitmap* pImplBitmap = dynamic_cast<VclCanvasBitmap*>(xInputBitmap.get());
            if( pImplBitmap )
                return pImplBitmap->getBitmapEx();

            // retrieve data via UNO interface

            // volatile bitmaps are a bit more complicated to read
            // from...

            // loop a few times, until successfully read (for XVolatileBitmap)
            for( int i=0; i<10; ++i )
            {
                sal_Int32 nDepth=0;
                sal_Int32 nAlphaDepth=0;
                const rendering::IntegerBitmapLayout aLayout(
                    xInputBitmap->getMemoryLayout());

                OSL_ENSURE(aLayout.ColorSpace.is(),
                           "Cannot convert image without color space!");
                if( !aLayout.ColorSpace.is() )
                    return ::BitmapEx();

                nDepth = aLayout.ColorSpace->getBitsPerPixel();

                if( xInputBitmap->hasAlpha() )
                {
                    // determine alpha channel depth
                    const uno::Sequence<sal_Int8> aTags(
                        aLayout.ColorSpace->getComponentTags() );
                    const sal_Int8* pStart(aTags.getConstArray());
                    const std::size_t  nLen(aTags.getLength());
                    const sal_Int8* pEnd(pStart+nLen);

                    const std::ptrdiff_t nAlphaIndex =
                        std::find(pStart,pEnd,
                                  rendering::ColorComponentTag::ALPHA) - pStart;

                    if( nAlphaIndex < sal::static_int_cast<std::ptrdiff_t>(nLen) )
                    {
                        nAlphaDepth = aLayout.ColorSpace->getComponentBitCounts()[nAlphaIndex] > 1 ? 8 : 1;
                        nDepth -= nAlphaDepth;
                    }
                }

                BitmapPalette aPalette;
                if( aLayout.Palette.is() )
                {
                    uno::Reference< rendering::XColorSpace > xPaletteColorSpace(
                        aLayout.Palette->getColorSpace());
                    ENSURE_OR_THROW(xPaletteColorSpace.is(),
                                    "Palette without color space");

                    const sal_Int32 nEntryCount( aLayout.Palette->getNumberOfEntries() );
                    if( nEntryCount <= 256 )
                    {
                        if( nEntryCount <= 2 )
                            nDepth = 1;
                        else
                            nDepth = 8;

                        const sal_uInt16 nPaletteEntries(
                            sal::static_int_cast<sal_uInt16>(
                                std::min(sal_Int32(255), nEntryCount)));

                        // copy palette entries
                        aPalette.SetEntryCount(nPaletteEntries);
                        uno::Reference<rendering::XBitmapPalette> xPalette( aLayout.Palette );
                        uno::Reference<rendering::XColorSpace>    xPalColorSpace( xPalette->getColorSpace() );

                        uno::Sequence<double> aPaletteEntry;
                        for( sal_uInt16 j=0; j<nPaletteEntries; ++j )
                        {
                            if( !xPalette->getIndex(aPaletteEntry,j) &&
                                nAlphaDepth == 0 )
                            {
                                nAlphaDepth = 1;
                            }
                            uno::Sequence<rendering::RGBColor> aColors=xPalColorSpace->convertToRGB(aPaletteEntry);
                            ENSURE_OR_THROW(aColors.getLength() == 1,
                                            "Palette returned more or less than one entry");
                            const rendering::RGBColor& rColor=aColors[0];
                            aPalette[j] = BitmapColor(toByteColor(rColor.Red),
                                                      toByteColor(rColor.Green),
                                                      toByteColor(rColor.Blue));
                        }
                    }
                }

                const ::Size aPixelSize(
                    sizeFromIntegerSize2D(xInputBitmap->getSize()));

                // normalize bitcount
                auto ePixelFormat =
                    ( nDepth <= 1 ) ? vcl::PixelFormat::N1_BPP :
                    ( nDepth <= 8 ) ? vcl::PixelFormat::N8_BPP :
                                      vcl::PixelFormat::N24_BPP;
                auto eAlphaPixelFormat =
                    ( nAlphaDepth <= 1 ) ? vcl::PixelFormat::N1_BPP :
                                           vcl::PixelFormat::N8_BPP;

                ::Bitmap aBitmap( aPixelSize,
                                  ePixelFormat,
                                  aLayout.Palette.is() ? &aPalette : nullptr );
                ::Bitmap aAlpha;
                if( nAlphaDepth )
                    aAlpha = Bitmap(aPixelSize,
                                      eAlphaPixelFormat,
                                       &Bitmap::GetGreyPalette(
                                           sal::static_int_cast<sal_uInt16>(1 << nAlphaDepth)) );

                { // limit scoped access
                    BitmapScopedWriteAccess pWriteAccess( aBitmap );
                    BitmapScopedWriteAccess pAlphaWriteAccess( nAlphaDepth ? aAlpha.AcquireWriteAccess() : nullptr,
                                                               aAlpha );

                    ENSURE_OR_THROW(pWriteAccess.get() != nullptr,
                                    "Cannot get write access to bitmap");

                    const sal_Int32 nWidth(aPixelSize.Width());
                    const sal_Int32 nHeight(aPixelSize.Height());

                    if( !readBmp(nWidth,nHeight,aLayout,xInputBitmap,
                                 pWriteAccess,pAlphaWriteAccess) )
                        continue;
                } // limit scoped access

                if( nAlphaDepth )
                    return ::BitmapEx( aBitmap,
                                       AlphaMask( aAlpha ) );
                else
                    return ::BitmapEx( aBitmap );
            }

            // failed to read data 10 times - bail out
            return ::BitmapEx();
        }

        geometry::RealSize2D size2DFromSize( const Size& rSize )
        {
            return geometry::RealSize2D( rSize.Width(),
                                         rSize.Height() );
        }

        Size sizeFromRealSize2D( const geometry::RealSize2D& rSize )
        {
            return Size( static_cast<tools::Long>(rSize.Width + .5),
                         static_cast<tools::Long>(rSize.Height + .5) );
        }

        ::Size sizeFromB2DSize( const basegfx::B2DVector& rVec )
        {
            return ::Size( FRound( rVec.getX() ),
                           FRound( rVec.getY() ) );
        }

        ::Point pointFromB2DPoint( const basegfx::B2DPoint& rPoint )
        {
            return pointFromB2IPoint(basegfx::fround(rPoint));
        }

        ::tools::Rectangle rectangleFromB2DRectangle( const basegfx::B2DRange& rRect )
        {
            return rectangleFromB2IRectangle(basegfx::fround(rRect));
        }

        Point pointFromB2IPoint( const basegfx::B2IPoint& rPoint )
        {
            return ::Point( rPoint.getX(),
                            rPoint.getY() );
        }

        basegfx::B2IPoint b2IPointFromPoint(Point const& rPoint)
        {
            return basegfx::B2IPoint(rPoint.X(), rPoint.Y());
        }

        tools::Rectangle rectangleFromB2IRectangle( const basegfx::B2IRange& rRect )
        {
            return ::tools::Rectangle( rRect.getMinX(),
                                rRect.getMinY(),
                                rRect.getMaxX(),
                                rRect.getMaxY() );
        }

        basegfx::B2IRectangle b2IRectangleFromRectangle(tools::Rectangle const& rRect)
        {
            // although B2IRange internally has separate height/width emptiness, it doesn't
            // expose any API to let us set them separately, so just do the best we can.
            if (rRect.IsWidthEmpty() && rRect.IsHeightEmpty())
                return basegfx::B2IRange( basegfx::B2ITuple( rRect.Left(), rRect.Top() ) );
            return basegfx::B2IRange( rRect.Left(),
                                  rRect.Top(),
                                  rRect.IsWidthEmpty() ? rRect.Left() : rRect.Right(),
                                  rRect.IsHeightEmpty() ? rRect.Top() : rRect.Bottom() );
        }

        basegfx::B2DVector b2DSizeFromSize( const ::Size& rSize )
        {
            return basegfx::B2DVector( rSize.Width(),
                                         rSize.Height() );
        }

        basegfx::B2DPoint b2DPointFromPoint( const ::Point& rPoint )
        {
            return basegfx::B2DPoint( rPoint.X(),
                                        rPoint.Y() );
        }

        basegfx::B2DRange b2DRectangleFromRectangle( const ::tools::Rectangle& rRect )
        {
            // although B2DRange internally has separate height/width emptiness, it doesn't
            // expose any API to let us set them separately, so just do the best we can.
            if (rRect.IsWidthEmpty() && rRect.IsHeightEmpty())
                return basegfx::B2DRange( basegfx::B2DTuple( rRect.Left(), rRect.Top() ) );
            return basegfx::B2DRectangle( rRect.Left(),
                                  rRect.Top(),
                                  rRect.IsWidthEmpty() ? rRect.Left() : rRect.Right(),
                                  rRect.IsHeightEmpty() ? rRect.Top() : rRect.Bottom() );
        }

        geometry::IntegerSize2D integerSize2DFromSize( const Size& rSize )
        {
            return geometry::IntegerSize2D( rSize.Width(),
                                            rSize.Height() );
        }

        Size sizeFromIntegerSize2D( const geometry::IntegerSize2D& rSize )
        {
            return Size( rSize.Width,
                         rSize.Height );
        }

        Point pointFromIntegerPoint2D( const geometry::IntegerPoint2D& rPoint )
        {
            return Point( rPoint.X,
                          rPoint.Y );
        }

        tools::Rectangle rectangleFromIntegerRectangle2D( const geometry::IntegerRectangle2D& rRectangle )
        {
            return tools::Rectangle( rRectangle.X1, rRectangle.Y1,
                              rRectangle.X2, rRectangle.Y2 );
        }

        namespace
        {
            class StandardColorSpace : public cppu::WeakImplHelper< css::rendering::XColorSpace >
            {
            private:
                uno::Sequence< sal_Int8 > m_aComponentTags;

                virtual ::sal_Int8 SAL_CALL getType(  ) override
                {
                    return rendering::ColorSpaceType::RGB;
                }
                virtual uno::Sequence< ::sal_Int8 > SAL_CALL getComponentTags(  ) override
                {
                    return m_aComponentTags;
                }
                virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) override
                {
                    return rendering::RenderingIntent::PERCEPTUAL;
                }
                virtual uno::Sequence< beans::PropertyValue > SAL_CALL getProperties(  ) override
                {
                    return uno::Sequence< beans::PropertyValue >();
                }
                virtual uno::Sequence< double > SAL_CALL convertColorSpace( const uno::Sequence< double >& deviceColor,
                                                                            const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
                {
                    // TODO(P3): if we know anything about target
                    // colorspace, this can be greatly sped up
                    uno::Sequence<rendering::ARGBColor> aIntermediate(
                        convertToARGB(deviceColor));
                    return targetColorSpace->convertFromARGB(aIntermediate);
                }
                virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertToRGB( const uno::Sequence< double >& deviceColor ) override
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const std::size_t nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::RGBColor > aRes(nLen/4);
                    rendering::RGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::RGBColor(pIn[0],pIn[1],pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToARGB( const uno::Sequence< double >& deviceColor ) override
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const std::size_t nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(pIn[3],pIn[0],pIn[1],pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToPARGB( const uno::Sequence< double >& deviceColor ) override
                {
                    const double*  pIn( deviceColor.getConstArray() );
                    const std::size_t nLen( deviceColor.getLength() );
                    ENSURE_ARG_OR_THROW2(nLen%4==0,
                                         "number of channels no multiple of 4",
                                         static_cast<rendering::XColorSpace*>(this), 0);

                    uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
                    rendering::ARGBColor* pOut( aRes.getArray() );
                    for( std::size_t i=0; i<nLen; i+=4 )
                    {
                        *pOut++ = rendering::ARGBColor(pIn[3],pIn[3]*pIn[0],pIn[3]*pIn[1],pIn[3]*pIn[2]);
                        pIn += 4;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) override
                {
                    const std::size_t             nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( const auto& rIn : rgbColor )
                    {
                        *pColors++ = rIn.Red;
                        *pColors++ = rIn.Green;
                        *pColors++ = rIn.Blue;
                        *pColors++ = 1.0;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    const std::size_t              nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( const auto& rIn : rgbColor )
                    {
                        *pColors++ = rIn.Red;
                        *pColors++ = rIn.Green;
                        *pColors++ = rIn.Blue;
                        *pColors++ = rIn.Alpha;
                    }
                    return aRes;
                }
                virtual uno::Sequence< double > SAL_CALL convertFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
                {
                    const std::size_t              nLen( rgbColor.getLength() );

                    uno::Sequence< double > aRes(nLen*4);
                    double* pColors=aRes.getArray();
                    for( const auto& rIn : rgbColor )
                    {
                        *pColors++ = rIn.Red/rIn.Alpha;
                        *pColors++ = rIn.Green/rIn.Alpha;
                        *pColors++ = rIn.Blue/rIn.Alpha;
                        *pColors++ = rIn.Alpha;
                    }
                    return aRes;
                }

            public:
                StandardColorSpace() : m_aComponentTags(4)
                {
                    sal_Int8* pTags = m_aComponentTags.getArray();
                    pTags[0] = rendering::ColorComponentTag::RGB_RED;
                    pTags[1] = rendering::ColorComponentTag::RGB_GREEN;
                    pTags[2] = rendering::ColorComponentTag::RGB_BLUE;
                    pTags[3] = rendering::ColorComponentTag::ALPHA;
                }
            };
        }

        uno::Reference<rendering::XColorSpace> createStandardColorSpace()
        {
            return new StandardColorSpace();
        }

        uno::Sequence< double > colorToStdColorSpaceSequence( const Color& rColor )
        {
            return
            {
                toDoubleColor(rColor.GetRed()),
                toDoubleColor(rColor.GetGreen()),
                toDoubleColor(rColor.GetBlue()),
                toDoubleColor(rColor.GetAlpha())
            };
        }

        Color stdColorSpaceSequenceToColor( const uno::Sequence< double >& rColor        )
        {
            ENSURE_ARG_OR_THROW( rColor.getLength() == 4,
                                 "color must have 4 channels" );

            Color aColor;

            aColor.SetRed  ( toByteColor(rColor[0]) );
            aColor.SetGreen( toByteColor(rColor[1]) );
            aColor.SetBlue ( toByteColor(rColor[2]) );
            aColor.SetAlpha( toByteColor(rColor[3]) );

            return aColor;
        }

        uno::Sequence< double > colorToDoubleSequence(
            const Color&                                    rColor,
            const uno::Reference< rendering::XColorSpace >& xColorSpace )
        {
            uno::Sequence<rendering::ARGBColor> aSeq
            {
                {
                    toDoubleColor(rColor.GetAlpha()),
                    toDoubleColor(rColor.GetRed()),
                    toDoubleColor(rColor.GetGreen()),
                    toDoubleColor(rColor.GetBlue())
                }
            };

            return xColorSpace->convertFromARGB(aSeq);
        }

        Color doubleSequenceToColor(
            const uno::Sequence< double >&                  rColor,
            const uno::Reference< rendering::XColorSpace >& xColorSpace )
        {
            const rendering::ARGBColor aARGBColor(
                xColorSpace->convertToARGB(rColor)[0]);

            return Color( ColorAlpha, toByteColor(aARGBColor.Alpha),
                          toByteColor(aARGBColor.Red),
                          toByteColor(aARGBColor.Green),
                          toByteColor(aARGBColor.Blue) );
        }

} // namespace canvas

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
