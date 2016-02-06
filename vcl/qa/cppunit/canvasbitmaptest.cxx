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

// bootstrap stuff
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/util/Endianness.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>
#include <com/sun/star/rendering/XBitmapPalette.hpp>

#include <cppuhelper/implbase.hxx>
#include <tools/diagnose_ex.h>
#include <rtl/ref.hxx>

#include "vcl/svapp.hxx"
#include "vcl/canvastools.hxx"
#include "vcl/dialog.hxx"
#include "vcl/outdev.hxx"
#include "vcl/bitmapaccess.hxx"
#include "vcl/virdev.hxx"
#include "vcl/bitmapex.hxx"

#include "canvasbitmap.hxx"
#include <algorithm>

using namespace ::com::sun::star;
using namespace vcl::unotools;

namespace com { namespace sun { namespace star { namespace rendering
{

bool operator==( const RGBColor& rLHS, const ARGBColor& rRHS )
{
    return rLHS.Red == rRHS.Red && rLHS.Green == rRHS.Green && rLHS.Blue == rRHS.Blue;
}

} } } }

namespace
{

class CanvasBitmapTest : public test::BootstrapFixture
{
public:
    CanvasBitmapTest() : BootstrapFixture(true, false) {}

    void runTest();

    CPPUNIT_TEST_SUITE(CanvasBitmapTest);
    CPPUNIT_TEST(runTest);
    CPPUNIT_TEST_SUITE_END();
};

bool rangeCheck( const rendering::RGBColor& rColor )
{
    return rColor.Red < 0.0 || rColor.Red > 1.0 ||
        rColor.Green < 0.0 || rColor.Green > 1.0 ||
        rColor.Blue < 0.0 || rColor.Blue > 1.0;
}

void checkCanvasBitmap( const rtl::Reference<VclCanvasBitmap>& xBmp,
                        const char*                            msg,
                        int                                    nOriginalDepth )
{
    OSL_TRACE("-------------------------");
    OSL_TRACE("Testing %s, with depth %d", msg, nOriginalDepth);

    BitmapEx aContainedBmpEx( xBmp->getBitmapEx() );
    Bitmap   aContainedBmp( aContainedBmpEx.GetBitmap() );
    int      nDepth = nOriginalDepth;

    {
        Bitmap::ScopedReadAccess pAcc( aContainedBmp );
        nDepth = pAcc->GetBitCount();
    }

    CPPUNIT_ASSERT_MESSAGE( "Original bitmap size not (200,200)",
                            aContainedBmp.GetSizePixel() == Size(200,200));

    CPPUNIT_ASSERT_MESSAGE( "Original bitmap size via API not (200,200)",
                            xBmp->getSize().Width == 200 && xBmp->getSize().Height == 200);

    CPPUNIT_ASSERT_MESSAGE( "alpha state mismatch",
                            bool(xBmp->hasAlpha()) == aContainedBmpEx.IsTransparent());

    CPPUNIT_ASSERT_MESSAGE( "getScaledBitmap() failed",
                            xBmp->getScaledBitmap( geometry::RealSize2D(500,500), false ).is());

    rendering::IntegerBitmapLayout aLayout;
    uno::Sequence<sal_Int8> aPixelData = xBmp->getData(aLayout, geometry::IntegerRectangle2D(0,0,1,1));

    const sal_Int32 nExpectedBitsPerPixel(
        aContainedBmpEx.IsTransparent() ? std::max(8,nDepth)+8 : nDepth);
    CPPUNIT_ASSERT_MESSAGE( "# scanlines not 1",
                            aLayout.ScanLines == 1);
    CPPUNIT_ASSERT_MESSAGE( "# scanline bytes mismatch",
                            aLayout.ScanLineBytes == (nExpectedBitsPerPixel+7)/8);
    CPPUNIT_ASSERT_MESSAGE( "# scanline stride mismatch",
                            aLayout.ScanLineStride == (nExpectedBitsPerPixel+7)/8 ||
                            aLayout.ScanLineStride == -(nExpectedBitsPerPixel+7)/8);
    CPPUNIT_ASSERT_MESSAGE( "# plane stride not 0",
                            aLayout.PlaneStride == 0);

    CPPUNIT_ASSERT_MESSAGE( "Color space not there",
                            aLayout.ColorSpace.is());

    CPPUNIT_ASSERT_MESSAGE( "Palette existence does not conform to bitmap",
                            aLayout.Palette.is() == (nDepth <= 8));

    uno::Sequence<sal_Int8> aPixelData2 = xBmp->getPixel( aLayout, geometry::IntegerPoint2D(0,0) );

    CPPUNIT_ASSERT_MESSAGE( "getData and getPixel did not return same amount of data",
                            aPixelData2.getLength() == aPixelData.getLength());

    aPixelData = xBmp->getData(aLayout, geometry::IntegerRectangle2D(0,0,200,1));
    CPPUNIT_ASSERT_MESSAGE( "# scanlines not 1 for getPixel",
                            aLayout.ScanLines == 1);
    CPPUNIT_ASSERT_MESSAGE( "# scanline bytes mismatch for getPixel",
                            aLayout.ScanLineBytes == (200*nExpectedBitsPerPixel+7)/8);
    CPPUNIT_ASSERT_MESSAGE( "# scanline stride mismatch for getPixel",
                            aLayout.ScanLineStride == (200*nExpectedBitsPerPixel+7)/8 ||
                            aLayout.ScanLineStride == -(200*nExpectedBitsPerPixel+7)/8);

    uno::Sequence< rendering::RGBColor >  aRGBColors  = xBmp->convertIntegerToRGB( aPixelData );
    uno::Sequence< rendering::ARGBColor > aARGBColors = xBmp->convertIntegerToARGB( aPixelData );

    const rendering::RGBColor*  pRGBStart ( aRGBColors.getConstArray() );
    const rendering::RGBColor*  pRGBEnd   ( aRGBColors.getConstArray()+aRGBColors.getLength() );
    const rendering::ARGBColor* pARGBStart( aARGBColors.getConstArray() );
    std::pair<const rendering::RGBColor*,
        const rendering::ARGBColor*> aRes = std::mismatch( pRGBStart, pRGBEnd, pARGBStart );
    CPPUNIT_ASSERT_MESSAGE( "argb and rgb colors are not equal",
                            aRes.first == pRGBEnd);

    CPPUNIT_ASSERT_MESSAGE( "rgb colors are not within [0,1] range",
                            std::none_of(pRGBStart,pRGBEnd,&rangeCheck));

    CPPUNIT_ASSERT_MESSAGE( "First pixel is not white",
                            pRGBStart[0].Red == 1.0 && pRGBStart[0].Green == 1.0 && pRGBStart[0].Blue == 1.0);
    CPPUNIT_ASSERT_MESSAGE( "Second pixel is not opaque",
                            pARGBStart[1].Alpha == 1.0);
    if( aContainedBmpEx.IsTransparent() )
    {
        CPPUNIT_ASSERT_MESSAGE( "First pixel is not fully transparent",
                                pARGBStart[0].Alpha == 0.0);
    }

    CPPUNIT_ASSERT_MESSAGE( "Second pixel is not black",
                            pRGBStart[1].Red == 0.0 && pRGBStart[1].Green == 0.0 && pRGBStart[1].Blue == 0.0);

    if( nOriginalDepth > 8 )
    {
        const Color aCol(COL_GREEN);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Sixth pixel is not green (red component)",
            vcl::unotools::toDoubleColor(aCol.GetRed()), pRGBStart[5].Red);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Sixth pixel is not green (green component)",
            vcl::unotools::toDoubleColor(aCol.GetGreen()), pRGBStart[5].Green);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            "Sixth pixel is not green (blue component)",
            vcl::unotools::toDoubleColor(aCol.GetBlue()), pRGBStart[5].Blue);
    }
    else if( nDepth <= 8 )
    {
        uno::Reference<rendering::XBitmapPalette> xPal = xBmp->getPalette();
        CPPUNIT_ASSERT_MESSAGE( "8bit or less: missing palette",
                                xPal.is());
        CPPUNIT_ASSERT_MESSAGE( "Palette incorrect entry count",
                                xPal->getNumberOfEntries() == 1L << nOriginalDepth);
        uno::Sequence<double> aIndex;
        CPPUNIT_ASSERT_MESSAGE( "Palette is not read-only",
                                !xPal->setIndex(aIndex,true,0));
        CPPUNIT_ASSERT_MESSAGE( "Palette entry 0 is not opaque",
                                xPal->getIndex(aIndex,0));
        CPPUNIT_ASSERT_MESSAGE( "Palette has no valid color space",
                                xPal->getColorSpace().is());
    }

    CPPUNIT_ASSERT_MESSAGE( "150th pixel is not white",
                            pRGBStart[150].Red == 1.0 &&
                            pRGBStart[150].Green == 1.0 &&
                            pRGBStart[150].Blue == 1.0);

    if( nOriginalDepth > 8 )
    {
        uno::Sequence<rendering::ARGBColor> aARGBColor(1);
        uno::Sequence<rendering::RGBColor>  aRGBColor(1);
        uno::Sequence<sal_Int8> aPixel3, aPixel4;

        const Color aCol(COL_GREEN);
        aARGBColor[0].Red   = vcl::unotools::toDoubleColor(aCol.GetRed());
        aARGBColor[0].Green = vcl::unotools::toDoubleColor(aCol.GetGreen());
        aARGBColor[0].Blue  = vcl::unotools::toDoubleColor(aCol.GetBlue());
        aARGBColor[0].Alpha = 1.0;

        aRGBColor[0].Red   = vcl::unotools::toDoubleColor(aCol.GetRed());
        aRGBColor[0].Green = vcl::unotools::toDoubleColor(aCol.GetGreen());
        aRGBColor[0].Blue  = vcl::unotools::toDoubleColor(aCol.GetBlue());

        aPixel3 = xBmp->convertIntegerFromARGB( aARGBColor );
        aPixel4 = xBmp->getPixel( aLayout, geometry::IntegerPoint2D(5,0) );
        CPPUNIT_ASSERT_MESSAGE( "Green pixel from bitmap mismatch with manually converted green pixel",
                                aPixel3 == aPixel4);

        if( !aContainedBmpEx.IsTransparent() )
        {
            aPixel3 = xBmp->convertIntegerFromRGB( aRGBColor );
            CPPUNIT_ASSERT_MESSAGE( "Green pixel from bitmap mismatch with manually RGB-converted green pixel",
                                    aPixel3 == aPixel4);
        }
    }
}

class TestBitmap : public cppu::WeakImplHelper< rendering::XIntegerReadOnlyBitmap,
                                                 rendering::XBitmapPalette,
                                                 rendering::XIntegerBitmapColorSpace >
{
private:
    geometry::IntegerSize2D        maSize;
    uno::Sequence<sal_Int8>        maComponentTags;
    uno::Sequence<sal_Int32>       maComponentBitCounts;
    rendering::IntegerBitmapLayout maLayout;
    const sal_Int32                mnBitsPerPixel;

    // XBitmap
    virtual geometry::IntegerSize2D SAL_CALL getSize() throw (uno::RuntimeException, std::exception) override { return maSize; }
    virtual sal_Bool SAL_CALL hasAlpha(  ) throw (uno::RuntimeException, std::exception) override { return mnBitsPerPixel != 8; }
    virtual uno::Reference< rendering::XBitmap > SAL_CALL getScaledBitmap( const geometry::RealSize2D&,
                                                                           sal_Bool ) throw (uno::RuntimeException, std::exception) override { return this; }

    // XIntegerReadOnlyBitmap
    virtual uno::Sequence< ::sal_Int8 > SAL_CALL getData( rendering::IntegerBitmapLayout&     bitmapLayout,
                                                          const geometry::IntegerRectangle2D& rect )
        throw (lang::IndexOutOfBoundsException,
               rendering::VolatileContentDestroyedException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE( "X1 out of bounds", rect.X1 >= 0 );
        CPPUNIT_ASSERT_MESSAGE( "Y1 out of bounds", rect.Y1 >= 0 );
        CPPUNIT_ASSERT_MESSAGE( "X2 out of bounds", rect.X2 <= maSize.Width );
        CPPUNIT_ASSERT_MESSAGE( "Y2 out of bounds", rect.Y2 <= maSize.Height );

        bitmapLayout = getMemoryLayout();

        const sal_Int32 nWidth  = rect.X2-rect.X1;
        const sal_Int32 nHeight = rect.Y2-rect.Y1;
        const sal_Int32 nScanlineLen = (nWidth * mnBitsPerPixel + 7)/8;
        uno::Sequence<sal_Int8> aRes( nScanlineLen * nHeight );
        sal_Int8* pOut = aRes.getArray();

        bitmapLayout.ScanLines     = nHeight;
        bitmapLayout.ScanLineBytes =
        bitmapLayout.ScanLineStride= nScanlineLen;

        if( mnBitsPerPixel == 8 )
        {
            for( sal_Int32 y=0; y<nHeight; ++y )
            {
                for( sal_Int32 x=0; x<nWidth; ++x )
                    pOut[ y*nScanlineLen + x ] = sal_Int8(x);
            }
        }
        else
        {
            for( sal_Int32 y=0; y<nHeight; ++y )
            {
                for( sal_Int32 x=0; x<nWidth; ++x )
                {
                    pOut[ y*nScanlineLen + 4*x     ] = sal_Int8(rect.X1);
                    pOut[ y*nScanlineLen + 4*x + 1 ] = sal_Int8(rect.Y2);
                    pOut[ y*nScanlineLen + 4*x + 2 ] = sal_Int8(x);
                    pOut[ y*nScanlineLen + 4*x + 3 ] = sal_Int8(rect.Y1);
                }
            }
        }

        return aRes;
    }

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL getPixel( rendering::IntegerBitmapLayout&,
                                                           const geometry::IntegerPoint2D&  )
        throw (lang::IndexOutOfBoundsException,
               rendering::VolatileContentDestroyedException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("getPixel: method not implemented", false);
        return uno::Sequence< sal_Int8 >();
    }

    uno::Reference< rendering::XBitmapPalette > SAL_CALL getPalette(  ) throw (uno::RuntimeException)
    {
        uno::Reference< XBitmapPalette > aRet;
        if( mnBitsPerPixel == 8 )
            aRet.set(this);
        return aRet;
    }

    virtual rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout(  ) throw (uno::RuntimeException, std::exception) override
    {
        rendering::IntegerBitmapLayout aLayout( maLayout );

        const sal_Int32 nScanlineLen = (maSize.Width * mnBitsPerPixel + 7)/8;

        aLayout.ScanLines     = maSize.Height;
        aLayout.ScanLineBytes =
        aLayout.ScanLineStride= nScanlineLen;
        aLayout.Palette = getPalette();
        aLayout.ColorSpace.set( this );

        return aLayout;
    }

    // XBitmapPalette
    virtual sal_Int32 SAL_CALL getNumberOfEntries() throw (uno::RuntimeException, std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE( "Got palette getNumberOfEntries interface call without handing out palette",
                                getPalette().is() );

        return 255;
    }

    virtual sal_Bool SAL_CALL getIndex( uno::Sequence< double >& entry,
                                          ::sal_Int32 nIndex )
        throw (lang::IndexOutOfBoundsException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE( "Got palette getIndex interface call without handing out palette",
                                getPalette().is() );
        CPPUNIT_ASSERT_MESSAGE( "getIndex: index out of range",
                                nIndex >= 0 && nIndex < 256 );
        entry = colorToStdColorSpaceSequence(
            Color(sal_uInt8(nIndex),
                  sal_uInt8(nIndex),
                  sal_uInt8(nIndex)) );

        return true; // no palette transparency here.
    }

    virtual sal_Bool SAL_CALL setIndex( const uno::Sequence< double >&,
                                          sal_Bool,
                                          ::sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException,
                                                                      lang::IllegalArgumentException,
                                                                      uno::RuntimeException,
                                                                      std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE( "Got palette setIndex interface call without handing out palette",
                                getPalette().is());
        CPPUNIT_ASSERT_MESSAGE( "setIndex: index out of range",
                                nIndex >= 0 && nIndex < 256);
        return false;
    }

    struct PaletteColorSpaceHolder: public rtl::StaticWithInit<uno::Reference<rendering::XColorSpace>,
                                                               PaletteColorSpaceHolder>
    {
        uno::Reference<rendering::XColorSpace> operator()()
        {
            return vcl::unotools::createStandardColorSpace();
        }
    };

    virtual uno::Reference< rendering::XColorSpace > SAL_CALL getColorSpace(  ) throw (uno::RuntimeException, std::exception) override
    {
        // this is the method from XBitmapPalette. Return palette color
        // space here
        return PaletteColorSpaceHolder::get();
    }

    // XIntegerBitmapColorSpace
    virtual ::sal_Int8 SAL_CALL getType(  ) throw (uno::RuntimeException, std::exception) override
    {
        return rendering::ColorSpaceType::RGB;
    }

    virtual uno::Sequence< sal_Int8 > SAL_CALL getComponentTags(  ) throw (uno::RuntimeException, std::exception) override
    {
        return maComponentTags;
    }

    virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) throw (uno::RuntimeException, std::exception) override
    {
        return rendering::RenderingIntent::PERCEPTUAL;
    }

    virtual uno::Sequence< beans::PropertyValue > SAL_CALL getProperties()
        throw (uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("getProperties: method not implemented", false );
        return uno::Sequence< ::beans::PropertyValue >();
    }

    virtual uno::Sequence< double > SAL_CALL convertColorSpace( const uno::Sequence< double >&,
                                                                const uno::Reference< rendering::XColorSpace >& )
        throw (uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertColorSpace: method not implemented", false);
        return uno::Sequence< double >();
    }

    virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertToRGB( const uno::Sequence< double >& )
        throw (lang::IllegalArgumentException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertToRGB: method not implemented", false);
        return uno::Sequence< rendering::RGBColor >();
    }

    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToARGB( const uno::Sequence< double >& )
        throw (lang::IllegalArgumentException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertToARGB: method not implemented", false);
        return uno::Sequence< rendering::ARGBColor >();
    }

    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToPARGB( const uno::Sequence< double >& )
        throw (lang::IllegalArgumentException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertToPARGB: method not implemented", false);
        return uno::Sequence< rendering::ARGBColor >();
    }

    virtual uno::Sequence< double > SAL_CALL convertFromRGB( const uno::Sequence< rendering::RGBColor >& )
        throw (lang::IllegalArgumentException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertFromRGB: method not implemented", false);
        return uno::Sequence< double >();
    }

    virtual uno::Sequence< double > SAL_CALL convertFromARGB( const uno::Sequence< rendering::ARGBColor >& )
        throw (lang::IllegalArgumentException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertFromARGB: this method is not expected to be called!", false);
        return uno::Sequence< double >();
    }

    virtual uno::Sequence< double > SAL_CALL convertFromPARGB( const uno::Sequence< rendering::ARGBColor >& )
        throw (lang::IllegalArgumentException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertFromPARGB: this method is not expected to be called!", false);
        return uno::Sequence< double >();
    }

    virtual ::sal_Int32 SAL_CALL getBitsPerPixel(  ) throw (uno::RuntimeException, std::exception) override
    {
        return mnBitsPerPixel;
    }

    virtual uno::Sequence< ::sal_Int32 > SAL_CALL getComponentBitCounts(  ) throw (uno::RuntimeException, std::exception) override
    {
        return maComponentBitCounts;
    }

    virtual ::sal_Int8 SAL_CALL getEndianness(  ) throw (uno::RuntimeException, std::exception) override
    {
        return util::Endianness::LITTLE;
    }

    virtual uno::Sequence< double > SAL_CALL convertFromIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& ,
                                                                           const uno::Reference< rendering::XColorSpace >& )
        throw (lang::IllegalArgumentException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertFromIntegerColorSpace: method not implemented", false);
        return uno::Sequence< double >();
    }

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertToIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& ,
                                                                             const uno::Reference< rendering::XIntegerBitmapColorSpace >& )
        throw (lang::IllegalArgumentException,
               uno::RuntimeException,
               std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertToIntegerColorSpace: method not implemented", false);
        return uno::Sequence< sal_Int8 >();
    }

    virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertIntegerToRGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) throw (lang::IllegalArgumentException,
                                                                                                                                       uno::RuntimeException, std::exception) override
    {
        const uno::Sequence< rendering::ARGBColor > aTemp( convertIntegerToARGB(deviceColor) );
        const sal_Size nLen(aTemp.getLength());
        uno::Sequence< rendering::RGBColor > aRes( nLen );
        rendering::RGBColor* pOut = aRes.getArray();
        for( sal_Size i=0; i<nLen; ++i )
        {
            *pOut++ = rendering::RGBColor(aTemp[i].Red,
                                          aTemp[i].Green,
                                          aTemp[i].Blue);
        }

        return aRes;
    }

    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToARGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) throw (lang::IllegalArgumentException,
                                                                                                                                         uno::RuntimeException, std::exception) override
    {
        const sal_Size  nLen( deviceColor.getLength() );
        const sal_Int32 nBytesPerPixel(mnBitsPerPixel == 8 ? 1 : 4);
        CPPUNIT_ASSERT_MESSAGE("number of channels no multiple of pixel element count",
                               nLen%nBytesPerPixel==0);

        uno::Sequence< rendering::ARGBColor > aRes( nLen / nBytesPerPixel );
        rendering::ARGBColor* pOut( aRes.getArray() );

        if( getPalette().is() )
        {
            for( sal_Size i=0; i<nLen; ++i )
            {
                *pOut++ = rendering::ARGBColor(
                    1.0,
                    vcl::unotools::toDoubleColor(deviceColor[i]),
                    vcl::unotools::toDoubleColor(deviceColor[i]),
                    vcl::unotools::toDoubleColor(deviceColor[i]));
            }
        }
        else
        {
            for( sal_Size i=0; i<nLen; i+=4 )
            {
                *pOut++ = rendering::ARGBColor(
                    vcl::unotools::toDoubleColor(deviceColor[i+3]),
                    vcl::unotools::toDoubleColor(deviceColor[i+0]),
                    vcl::unotools::toDoubleColor(deviceColor[i+1]),
                    vcl::unotools::toDoubleColor(deviceColor[i+2]));
            }
        }

        return aRes;
    }

    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToPARGB(
        const uno::Sequence< ::sal_Int8 >& deviceColor)
            throw (lang::IllegalArgumentException,
                   uno::RuntimeException,
                   std::exception) override
    {
        const sal_Size  nLen( deviceColor.getLength() );
        const sal_Int32 nBytesPerPixel(mnBitsPerPixel == 8 ? 1 : 4);
        CPPUNIT_ASSERT_MESSAGE("number of channels no multiple of pixel element count",
                               nLen%nBytesPerPixel==0);

        uno::Sequence< rendering::ARGBColor > aRes( nLen / nBytesPerPixel );
        rendering::ARGBColor* pOut( aRes.getArray() );

        if( getPalette().is() )
        {
            for( sal_Size i=0; i<nLen; ++i )
            {
                *pOut++ = rendering::ARGBColor(
                    1.0,
                    vcl::unotools::toDoubleColor(deviceColor[i]),
                    vcl::unotools::toDoubleColor(deviceColor[i]),
                    vcl::unotools::toDoubleColor(deviceColor[i]));
            }
        }
        else
        {
            for( sal_Size i=0; i<nLen; i+=4 )
            {
                const double fAlpha=vcl::unotools::toDoubleColor(deviceColor[i+3]);
                *pOut++ = rendering::ARGBColor(
                    fAlpha,
                    fAlpha*vcl::unotools::toDoubleColor(deviceColor[i+0]),
                    fAlpha*vcl::unotools::toDoubleColor(deviceColor[i+1]),
                    fAlpha*vcl::unotools::toDoubleColor(deviceColor[i+2]));
            }
        }

        return aRes;
    }

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromRGB(
        const uno::Sequence< rendering::RGBColor >&)
            throw (lang::IllegalArgumentException,
                   uno::RuntimeException,
                   std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertIntegerFromRGB: method not implemented", false);
        return uno::Sequence< sal_Int8 >();
    }

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromARGB( const uno::Sequence< rendering::ARGBColor >& ) throw (lang::IllegalArgumentException,
                                                                                                                               uno::RuntimeException,
                                  std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertIntegerFromARGB: method not implemented", false);
        return uno::Sequence< sal_Int8 >();
    }

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromPARGB( const uno::Sequence< rendering::ARGBColor >& )
        throw (lang::IllegalArgumentException,
        uno::RuntimeException,
        std::exception) override
    {
        CPPUNIT_ASSERT_MESSAGE("convertIntegerFromPARGB: method not implemented", false);
        return uno::Sequence< sal_Int8 >();
    }

public:
    TestBitmap( const geometry::IntegerSize2D& rSize, bool bPalette ) :
        maSize(rSize),
        maComponentTags(),
        maComponentBitCounts(),
        maLayout(),
        mnBitsPerPixel( bPalette ? 8 : 32 )
    {
        if( bPalette )
        {
            maComponentTags.realloc(1);
            maComponentTags[0] = rendering::ColorComponentTag::INDEX;

            maComponentBitCounts.realloc(1);
            maComponentBitCounts[0] = 8;
        }
        else
        {
            maComponentTags.realloc(4);
            sal_Int8* pTags = maComponentTags.getArray();
            pTags[0]        = rendering::ColorComponentTag::RGB_BLUE;
            pTags[1]        = rendering::ColorComponentTag::RGB_GREEN;
            pTags[2]        = rendering::ColorComponentTag::RGB_RED;
            pTags[3]        = rendering::ColorComponentTag::ALPHA;

            maComponentBitCounts.realloc(4);
            sal_Int32* pCounts = maComponentBitCounts.getArray();
            pCounts[0]         = 8;
            pCounts[1]         = 8;
            pCounts[2]         = 8;
            pCounts[3]         = 8;
        }

        maLayout.ScanLines      = 0;
        maLayout.ScanLineBytes  = 0;
        maLayout.ScanLineStride = 0;
        maLayout.PlaneStride    = 0;
        maLayout.ColorSpace.clear();
        maLayout.Palette.clear();
        maLayout.IsMsbFirst     = false;
    }
};

void CanvasBitmapTest::runTest()
{
    static const sal_Int8 lcl_depths[]={1,4,8,16,24};

    // Testing VclCanvasBitmap wrapper

    for( unsigned int i=0; i<SAL_N_ELEMENTS(lcl_depths); ++i )
    {
        const sal_Int8 nDepth( lcl_depths[i] );
        Bitmap aBitmap(Size(200,200),nDepth);
        aBitmap.Erase(COL_WHITE);
        {
            Bitmap::ScopedWriteAccess pAcc(aBitmap);
            if( pAcc.get() )
            {
                BitmapColor aBlack(0);
                BitmapColor aWhite(0);
                if( pAcc->HasPalette() )
                {
                    aBlack.SetIndex( sal::static_int_cast<sal_Int8>(pAcc->GetBestPaletteIndex(BitmapColor(0,0,0))) );
                    aWhite.SetIndex( sal::static_int_cast<sal_Int8>(pAcc->GetBestPaletteIndex(BitmapColor(255,255,255))) );
                }
                else
                {
                    aBlack = Color(COL_BLACK);
                    aWhite = Color(COL_WHITE);
                }
                pAcc->SetFillColor(COL_GREEN);
                pAcc->FillRect(Rectangle(0,0,100,100));
                pAcc->SetPixel(0,0,aWhite);
                pAcc->SetPixel(0,1,aBlack);
                pAcc->SetPixel(0,2,aWhite);
            }
        }

        rtl::Reference<VclCanvasBitmap> xBmp( new VclCanvasBitmap(aBitmap) );

        checkCanvasBitmap( xBmp, "single bitmap", nDepth );

        Bitmap aMask(Size(200,200),1);
        aMask.Erase(COL_WHITE);
        {
            Bitmap::ScopedWriteAccess pAcc(aMask);
            if( pAcc.get() )
            {
                pAcc->SetFillColor(COL_BLACK);
                pAcc->FillRect(Rectangle(0,0,100,100));
                pAcc->SetPixel(0,0,BitmapColor(1));
                pAcc->SetPixel(0,1,BitmapColor(0));
                pAcc->SetPixel(0,2,BitmapColor(1));
            }
        }

        xBmp.set( new VclCanvasBitmap(BitmapEx(aBitmap,aMask)) );

        checkCanvasBitmap( xBmp, "masked bitmap", nDepth );

        AlphaMask aAlpha(Size(200,200));
        aAlpha.Erase(255);
        {
            BitmapWriteAccess* pAcc = aAlpha.AcquireWriteAccess();
            if( pAcc )
            {
                pAcc->SetFillColor(COL_BLACK);
                pAcc->FillRect(Rectangle(0,0,100,100));
                pAcc->SetPixel(0,0,BitmapColor(255));
                pAcc->SetPixel(0,1,BitmapColor(0));
                pAcc->SetPixel(0,2,BitmapColor(255));
                aAlpha.ReleaseAccess(pAcc);
            }
        }

        xBmp.set( new VclCanvasBitmap(BitmapEx(aBitmap,aAlpha)) );

        checkCanvasBitmap( xBmp, "alpha bitmap", nDepth );
    }

    // Testing XBitmap import

    uno::Reference< rendering::XIntegerReadOnlyBitmap > xTestBmp(
        new TestBitmap( geometry::IntegerSize2D(10,10), true ));

    BitmapEx aBmp = vcl::unotools::bitmapExFromXBitmap(xTestBmp);
    CPPUNIT_ASSERT_MESSAGE( "Palette bitmap is transparent",
                            !aBmp.IsTransparent());
    CPPUNIT_ASSERT_MESSAGE( "Bitmap does not have size (10,10)",
                            aBmp.GetSizePixel() == Size(10,10));
    CPPUNIT_ASSERT_MESSAGE( "Bitmap does not have bitcount of 8",
                            aBmp.GetBitCount() == 8);
    {
        BitmapReadAccess* pBmpAcc   = aBmp.GetBitmap().AcquireReadAccess();

        CPPUNIT_ASSERT_MESSAGE( "Bitmap has invalid BitmapReadAccess",
                                pBmpAcc );

        CPPUNIT_ASSERT_MESSAGE("(0,0) incorrect content",
                               pBmpAcc->GetPixel(0,0) == BitmapColor(0));
        CPPUNIT_ASSERT_MESSAGE("(2,2) incorrect content",
                               pBmpAcc->GetPixel(2,2) == BitmapColor(2));
        CPPUNIT_ASSERT_MESSAGE("(9,2) incorrect content",
                               pBmpAcc->GetPixel(2,9) == BitmapColor(9));

        Bitmap::ReleaseAccess(pBmpAcc);
    }

    xTestBmp.set( new TestBitmap( geometry::IntegerSize2D(10,10), false ));

    aBmp = vcl::unotools::bitmapExFromXBitmap(xTestBmp);
    CPPUNIT_ASSERT_MESSAGE( "Palette bitmap is not transparent",
                            aBmp.IsTransparent());
    CPPUNIT_ASSERT_MESSAGE( "Palette bitmap has no alpha",
                            aBmp.IsAlpha());
    CPPUNIT_ASSERT_MESSAGE( "Bitmap does not have size (10,10)",
                            aBmp.GetSizePixel() == Size(10,10));
    CPPUNIT_ASSERT_MESSAGE( "Bitmap has bitcount of 24",
                            aBmp.GetBitCount() == 24);
    {
        BitmapReadAccess* pBmpAcc   = aBmp.GetBitmap().AcquireReadAccess();
        BitmapReadAccess* pAlphaAcc = aBmp.GetAlpha().AcquireReadAccess();

        CPPUNIT_ASSERT_MESSAGE( "Bitmap has invalid BitmapReadAccess",
                                pBmpAcc);
        CPPUNIT_ASSERT_MESSAGE( "Bitmap has invalid alpha BitmapReadAccess",
                                pAlphaAcc);

        CPPUNIT_ASSERT_MESSAGE("(0,0) incorrect content",
                               pBmpAcc->GetPixel(0,0) == BitmapColor(0,1,0));
        CPPUNIT_ASSERT_MESSAGE("(0,0) incorrect alpha content",
                               pAlphaAcc->GetPixel(0,0) == BitmapColor(255));
        CPPUNIT_ASSERT_MESSAGE("(2,2) incorrect content",
                               pBmpAcc->GetPixel(2,2) == BitmapColor(0,3,2));
        CPPUNIT_ASSERT_MESSAGE("(2,2) incorrect alpha content",
                               pAlphaAcc->GetPixel(2,2) == BitmapColor(253));
        CPPUNIT_ASSERT_MESSAGE("(9,2) incorrect content",
                               pBmpAcc->GetPixel(2,9) == BitmapColor(0,3,9));
        CPPUNIT_ASSERT_MESSAGE("(9,2) correct alpha content",
                               pAlphaAcc->GetPixel(2,9) == BitmapColor(253));

        aBmp.GetAlpha().ReleaseAccess(pAlphaAcc);
        Bitmap::ReleaseAccess(pBmpAcc);
    }
}

} // namespace

CPPUNIT_TEST_SUITE_REGISTRATION(CanvasBitmapTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
