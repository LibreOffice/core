/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


// bootstrap stuff
#include <sal/main.h>
#include <rtl/bootstrap.hxx>
#include <rtl/ref.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/util/Endianness.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>
#include <com/sun/star/rendering/XBitmapPalette.hpp>

#include <cppuhelper/compbase3.hxx>

#include <tools/diagnose_ex.h>
#include <tools/extendapplicationenvironment.hxx>

#include "vcl/svapp.hxx"
#include "vcl/canvastools.hxx"
#include "vcl/canvasbitmap.hxx"
#include "vcl/dialog.hxx"
#include "vcl/outdev.hxx"
#include "vcl/bmpacc.hxx"
#include "vcl/virdev.hxx"
#include "vcl/bitmapex.hxx"


using namespace ::com::sun::star;
using namespace ::vcl::unotools;

// -----------------------------------------------------------------------

void Main();

// -----------------------------------------------------------------------

SAL_IMPLEMENT_MAIN()
{
    tools::extendApplicationEnvironment();

    uno::Reference< lang::XMultiServiceFactory > xMS;
    xMS = cppu::createRegistryServiceFactory(
        rtl::OUString( "applicat.rdb"  ),
        sal_True );

    InitVCL( xMS );
    ::Main();
    DeInitVCL();

    return 0;
}

// -----------------------------------------------------------------------

namespace com { namespace sun { namespace star { namespace rendering
{

bool operator==( const RGBColor& rLHS, const ARGBColor& rRHS )
{
    return rLHS.Red == rRHS.Red && rLHS.Green == rRHS.Green && rLHS.Blue == rRHS.Blue;
}
bool operator==( const ARGBColor& rLHS, const RGBColor& rRHS )
{
    return rLHS.Red == rRHS.Red && rLHS.Green == rRHS.Green && rLHS.Blue == rRHS.Blue;
}

} } } }

//----------------------------------------------------------------------------------

namespace
{

class TestApp : public Application
{
public:
    virtual void   Main();
    virtual sal_uInt16 Exception( sal_uInt16 nError );
};

class TestWindow : public Dialog
{
    public:
        TestWindow() : Dialog( (Window *) NULL )
        {
            SetText( rtl::OUString("CanvasBitmap test harness") );
            SetSizePixel( Size( 1024, 1024 ) );
            EnablePaint( true );
            Show();
        }

        virtual ~TestWindow() {}
        virtual void Paint( const Rectangle& rRect );
};

//----------------------------------------------------------------------------------

static bool g_failure=false;

void test( bool bResult, const char* msg )
{
    if( bResult )
    {
        OSL_TRACE("Testing: %s - PASSED", msg);
    }
    else
    {
        g_failure = true;
        OSL_TRACE("Testing: %s - FAILED", msg);
    }
}

//----------------------------------------------------------------------------------

bool rangeCheck( const rendering::RGBColor& rColor )
{
    return rColor.Red < 0.0 || rColor.Red > 1.0 ||
        rColor.Green < 0.0 || rColor.Green > 1.0 ||
        rColor.Blue < 0.0 || rColor.Blue > 1.0;
}

//----------------------------------------------------------------------------------

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

    test( aContainedBmp.GetSizePixel() == Size(200,200),
          "Original bitmap size" );

    test( xBmp->getSize().Width == 200 && xBmp->getSize().Height == 200,
          "Original bitmap size via API" );

    test( xBmp->hasAlpha() == aContainedBmpEx.IsTransparent(),
          "Correct alpha state" );

    test( xBmp->getScaledBitmap( geometry::RealSize2D(500,500), sal_False ).is(),
          "getScaledBitmap()" );

    rendering::IntegerBitmapLayout aLayout;
    uno::Sequence<sal_Int8> aPixelData = xBmp->getData(aLayout, geometry::IntegerRectangle2D(0,0,1,1));

    const sal_Int32 nExpectedBitsPerPixel(
        aContainedBmpEx.IsTransparent() ? std::max(8,nDepth)+8 : nDepth);
    test( aLayout.ScanLines == 1,
          "# scanlines" );
    test( aLayout.ScanLineBytes == (nExpectedBitsPerPixel+7)/8,
          "# scanline bytes" );
    test( aLayout.ScanLineStride == (nExpectedBitsPerPixel+7)/8 ||
          aLayout.ScanLineStride == -(nExpectedBitsPerPixel+7)/8,
          "# scanline stride" );
    test( aLayout.PlaneStride == 0,
          "# plane stride" );

    test( aLayout.ColorSpace.is(),
          "Color space there" );

    test( aLayout.Palette.is() == (nDepth <= 8),
          "Palette existance conforms to bitmap" );

    uno::Sequence<sal_Int8> aPixelData2 = xBmp->getPixel( aLayout, geometry::IntegerPoint2D(0,0) );

    test( aPixelData2.getLength() == aPixelData.getLength(),
          "getData and getPixel return same amount of data" );

    aPixelData = xBmp->getData(aLayout, geometry::IntegerRectangle2D(0,0,200,1));
    test( aLayout.ScanLines == 1,
          "# scanlines" );
    test( aLayout.ScanLineBytes == (200*nExpectedBitsPerPixel+7)/8,
          "# scanline bytes" );
    test( aLayout.ScanLineStride == (200*nExpectedBitsPerPixel+7)/8 ||
          aLayout.ScanLineStride == -(200*nExpectedBitsPerPixel+7)/8,
          "# scanline stride" );

    uno::Sequence< rendering::RGBColor >  aRGBColors  = xBmp->convertIntegerToRGB( aPixelData );
    uno::Sequence< rendering::ARGBColor > aARGBColors = xBmp->convertIntegerToARGB( aPixelData );

    const rendering::RGBColor*  pRGBStart ( aRGBColors.getConstArray() );
    const rendering::RGBColor*  pRGBEnd   ( aRGBColors.getConstArray()+aRGBColors.getLength() );
    const rendering::ARGBColor* pARGBStart( aARGBColors.getConstArray() );
    std::pair<const rendering::RGBColor*,
        const rendering::ARGBColor*> aRes = std::mismatch( pRGBStart, pRGBEnd, pARGBStart );
    test( aRes.first == pRGBEnd,
          "argb and rgb colors are equal" );

    test( std::find_if(pRGBStart,pRGBEnd,&rangeCheck) == pRGBEnd,
          "rgb colors are within [0,1] range" );

    test( pRGBStart[0].Red == 1.0 && pRGBStart[0].Green == 1.0 && pRGBStart[0].Blue == 1.0,
          "First pixel is white" );
    test( pARGBStart[1].Alpha == 1.0,
          "Second pixel is opaque" );
    if( aContainedBmpEx.IsTransparent() )
    {
        test( pARGBStart[0].Alpha == 0.0,
              "First pixel is fully transparent" );
    }

    test( pRGBStart[1].Red == 0.0 && pRGBStart[1].Green == 0.0 && pRGBStart[1].Blue == 0.0,
          "Second pixel is black" );

    if( nOriginalDepth > 8 )
    {
        const Color aCol(COL_GREEN);
        test( pRGBStart[5].Red == vcl::unotools::toDoubleColor(aCol.GetRed()) &&
              pRGBStart[5].Green == vcl::unotools::toDoubleColor(aCol.GetGreen()) &&
              pRGBStart[5].Blue == vcl::unotools::toDoubleColor(aCol.GetBlue()),
              "Sixth pixel is green" );
    }
    else if( nDepth <= 8 )
    {
        uno::Reference<rendering::XBitmapPalette> xPal = xBmp->getPalette();
        test( xPal.is(),
              "8bit or less: needs palette" );
        test( xPal->getNumberOfEntries() == 1L << nOriginalDepth,
              "Palette has correct entry count" );
        uno::Sequence<double> aIndex;
        test( xPal->setIndex(aIndex,sal_True,0) == sal_False,
              "Palette is read-only" );
        test( xPal->getIndex(aIndex,0),
              "Palette entry 0 is opaque" );
        test( xPal->getColorSpace().is(),
              "Palette has a valid color space" );
    }

    test( pRGBStart[150].Red == 1.0 && pRGBStart[150].Green == 1.0 && pRGBStart[150].Blue == 1.0,
          "150th pixel is white" );

    if( nOriginalDepth > 8 )
    {
        const uno::Sequence<sal_Int8> aComponentTags( xBmp->getComponentTags() );
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
        test( aPixel3 == aPixel4,
              "Green pixel from bitmap matches with manually converted green pixel" );

        if( !aContainedBmpEx.IsTransparent() )
        {
            aPixel3 = xBmp->convertIntegerFromRGB( aRGBColor );
            test( aPixel3 == aPixel4,
                  "Green pixel from bitmap matches with manually RGB-converted green pixel" );
        }
    }
}

//----------------------------------------------------------------------------------

void checkBitmapImport( const rtl::Reference<VclCanvasBitmap>& xBmp,
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

    test( aContainedBmp.GetSizePixel() == Size(200,200),
          "Original bitmap size" );

    test( xBmp->getSize().Width == 200 && xBmp->getSize().Height == 200,
          "Original bitmap size via API" );

    test( xBmp->hasAlpha() == aContainedBmpEx.IsTransparent(),
          "Correct alpha state" );

    test( xBmp->getScaledBitmap( geometry::RealSize2D(500,500), sal_False ).is(),
          "getScaledBitmap()" );

    rendering::IntegerBitmapLayout aLayout;
    uno::Sequence<sal_Int8> aPixelData = xBmp->getData(aLayout, geometry::IntegerRectangle2D(0,0,1,1));

    const sal_Int32 nExpectedBitsPerPixel(
        aContainedBmpEx.IsTransparent() ? std::max(8,nDepth)+8 : nDepth);
    test( aLayout.ScanLines == 1,
          "# scanlines" );
    test( aLayout.ScanLineBytes == (nExpectedBitsPerPixel+7)/8,
          "# scanline bytes" );
    test( aLayout.ScanLineStride == (nExpectedBitsPerPixel+7)/8 ||
          aLayout.ScanLineStride == -(nExpectedBitsPerPixel+7)/8,
          "# scanline stride" );
    test( aLayout.PlaneStride == 0,
          "# plane stride" );

    test( aLayout.ColorSpace.is(),
          "Color space there" );

    test( aLayout.Palette.is() == (nDepth <= 8),
          "Palette existance conforms to bitmap" );

    uno::Sequence<sal_Int8> aPixelData2 = xBmp->getPixel( aLayout, geometry::IntegerPoint2D(0,0) );

    test( aPixelData2.getLength() == aPixelData.getLength(),
          "getData and getPixel return same amount of data" );

    aPixelData = xBmp->getData(aLayout, geometry::IntegerRectangle2D(0,0,200,1));
    test( aLayout.ScanLines == 1,
          "# scanlines" );
    test( aLayout.ScanLineBytes == (200*nExpectedBitsPerPixel+7)/8,
          "# scanline bytes" );
    test( aLayout.ScanLineStride == (200*nExpectedBitsPerPixel+7)/8 ||
          aLayout.ScanLineStride == -(200*nExpectedBitsPerPixel+7)/8,
          "# scanline stride" );

    uno::Sequence< rendering::RGBColor >  aRGBColors  = xBmp->convertIntegerToRGB( aPixelData );
    uno::Sequence< rendering::ARGBColor > aARGBColors = xBmp->convertIntegerToARGB( aPixelData );

    const rendering::RGBColor*  pRGBStart ( aRGBColors.getConstArray() );
    const rendering::RGBColor*  pRGBEnd   ( aRGBColors.getConstArray()+aRGBColors.getLength() );
    const rendering::ARGBColor* pARGBStart( aARGBColors.getConstArray() );
    std::pair<const rendering::RGBColor*,
        const rendering::ARGBColor*> aRes = std::mismatch( pRGBStart, pRGBEnd, pARGBStart );
    test( aRes.first == pRGBEnd,
          "argb and rgb colors are equal" );

    test( std::find_if(pRGBStart,pRGBEnd,&rangeCheck) == pRGBEnd,
          "rgb colors are within [0,1] range" );

    test( pRGBStart[0].Red == 1.0 && pRGBStart[0].Green == 1.0 && pRGBStart[0].Blue == 1.0,
          "First pixel is white" );
    test( pARGBStart[1].Alpha == 1.0,
          "Second pixel is opaque" );
    if( aContainedBmpEx.IsTransparent() )
    {
        test( pARGBStart[0].Alpha == 0.0,
              "First pixel is fully transparent" );
    }

    test( pRGBStart[1].Red == 0.0 && pRGBStart[1].Green == 0.0 && pRGBStart[1].Blue == 0.0,
          "Second pixel is black" );

    if( nOriginalDepth > 8 )
    {
        const Color aCol(COL_GREEN);
        test( pRGBStart[5].Red == vcl::unotools::toDoubleColor(aCol.GetRed()) &&
              pRGBStart[5].Green == vcl::unotools::toDoubleColor(aCol.GetGreen()) &&
              pRGBStart[5].Blue == vcl::unotools::toDoubleColor(aCol.GetBlue()),
              "Sixth pixel is green" );
    }
    else if( nDepth <= 8 )
    {
        uno::Reference<rendering::XBitmapPalette> xPal = xBmp->getPalette();
        test( xPal.is(),
              "8bit or less: needs palette" );
        test( xPal->getNumberOfEntries() == 1L << nOriginalDepth,
              "Palette has correct entry count" );
        uno::Sequence<double> aIndex;
        test( xPal->setIndex(aIndex,sal_True,0) == sal_False,
              "Palette is read-only" );
        test( xPal->getIndex(aIndex,0),
              "Palette entry 0 is opaque" );
        test( xPal->getColorSpace().is(),
              "Palette has a valid color space" );
    }

    test( pRGBStart[150].Red == 1.0 && pRGBStart[150].Green == 1.0 && pRGBStart[150].Blue == 1.0,
          "150th pixel is white" );

    if( nOriginalDepth > 8 )
    {
        const uno::Sequence<sal_Int8> aComponentTags( xBmp->getComponentTags() );
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
        test( aPixel3 == aPixel4,
              "Green pixel from bitmap matches with manually converted green pixel" );

        if( !aContainedBmpEx.IsTransparent() )
        {
            aPixel3 = xBmp->convertIntegerFromRGB( aRGBColor );
            test( aPixel3 == aPixel4,
                  "Green pixel from bitmap matches with manually RGB-converted green pixel" );
        }
    }
}

//----------------------------------------------------------------------------------

class TestBitmap : public cppu::WeakImplHelper3< rendering::XIntegerReadOnlyBitmap,
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
    virtual geometry::IntegerSize2D SAL_CALL getSize() throw (uno::RuntimeException) { return maSize; }
    virtual ::sal_Bool SAL_CALL hasAlpha(  ) throw (uno::RuntimeException) { return mnBitsPerPixel != 8; }
    virtual uno::Reference< rendering::XBitmap > SAL_CALL getScaledBitmap( const geometry::RealSize2D&,
                                                                           sal_Bool ) throw (uno::RuntimeException) { return this; }

    // XIntegerReadOnlyBitmap
    virtual uno::Sequence< ::sal_Int8 > SAL_CALL getData( rendering::IntegerBitmapLayout&     bitmapLayout,
                                                          const geometry::IntegerRectangle2D& rect ) throw (lang::IndexOutOfBoundsException,
                                                                                                            rendering::VolatileContentDestroyedException, uno::RuntimeException)
    {
        test( rect.X1 >= 0, "X1 within bounds" );
        test( rect.Y1 >= 0, "Y1 within bounds" );
        test( rect.X2 <= maSize.Width,  "X2 within bounds" );
        test( rect.Y2 <= maSize.Height, "Y2 within bounds" );

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
                                                           const geometry::IntegerPoint2D&  ) throw (lang::IndexOutOfBoundsException,
                                                                                                     rendering::VolatileContentDestroyedException, uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< sal_Int8 >();
    }

    virtual uno::Reference< rendering::XBitmapPalette > SAL_CALL getPalette(  ) throw (uno::RuntimeException)
    {
        uno::Reference< XBitmapPalette > aRet;
        if( mnBitsPerPixel == 8 )
            aRet.set(this);
        return aRet;
    }

    virtual rendering::IntegerBitmapLayout SAL_CALL getMemoryLayout(  ) throw (uno::RuntimeException)
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
    virtual sal_Int32 SAL_CALL getNumberOfEntries() throw (uno::RuntimeException)
    {
        test( getPalette().is(),
              "Got palette interface call without handing out palette?!" );

        return 255;
    }

    virtual ::sal_Bool SAL_CALL getIndex( uno::Sequence< double >& entry,
                                          ::sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException,
                                                                      uno::RuntimeException)
    {
        test( getPalette().is(),
              "Got palette interface call without handing out palette?!" );
        test( nIndex >= 0 && nIndex < 256,
              "Index out of range" );
        entry = colorToStdColorSpaceSequence(
            Color(UINT8(nIndex),
                  UINT8(nIndex),
                  UINT8(nIndex)) );

        return sal_True; // no palette transparency here.
    }

    virtual ::sal_Bool SAL_CALL setIndex( const uno::Sequence< double >&,
                                          ::sal_Bool,
                                          ::sal_Int32 nIndex ) throw (lang::IndexOutOfBoundsException,
                                                                      lang::IllegalArgumentException,
                                                                      uno::RuntimeException)
    {
        test( getPalette().is(),
              "Got palette interface call without handing out palette?!" );
        test( nIndex >= 0 && nIndex < 256,
              "Index out of range" );
        return sal_False;
    }

    struct PaletteColorSpaceHolder: public rtl::StaticWithInit<uno::Reference<rendering::XColorSpace>,
                                                               PaletteColorSpaceHolder>
    {
        uno::Reference<rendering::XColorSpace> operator()()
        {
            return vcl::unotools::createStandardColorSpace();
        }
    };

    virtual uno::Reference< rendering::XColorSpace > SAL_CALL getColorSpace(  ) throw (uno::RuntimeException)
    {
        // this is the method from XBitmapPalette. Return palette color
        // space here
        return PaletteColorSpaceHolder::get();
    }

    // XIntegerBitmapColorSpace
    virtual ::sal_Int8 SAL_CALL getType(  ) throw (uno::RuntimeException)
    {
        return rendering::ColorSpaceType::RGB;
    }

    virtual uno::Sequence< sal_Int8 > SAL_CALL getComponentTags(  ) throw (uno::RuntimeException)
    {
        return maComponentTags;
    }

    virtual ::sal_Int8 SAL_CALL getRenderingIntent(  ) throw (uno::RuntimeException)
    {
        return rendering::RenderingIntent::PERCEPTUAL;
    }

    virtual uno::Sequence< beans::PropertyValue > SAL_CALL getProperties(  ) throw (uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< ::beans::PropertyValue >();
    }

    virtual uno::Sequence< double > SAL_CALL convertColorSpace( const uno::Sequence< double >&,
                                                                const uno::Reference< rendering::XColorSpace >& ) throw (uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< double >();
    }

    virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertToRGB( const uno::Sequence< double >& ) throw (lang::IllegalArgumentException,
                                                                                                                uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< rendering::RGBColor >();
    }

    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToARGB( const uno::Sequence< double >& ) throw (lang::IllegalArgumentException,
                                                                                                                  uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< rendering::ARGBColor >();
    }

    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToPARGB( const uno::Sequence< double >& ) throw (lang::IllegalArgumentException,
                                                                                                                   uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< rendering::ARGBColor >();
    }

    virtual uno::Sequence< double > SAL_CALL convertFromRGB( const uno::Sequence< rendering::RGBColor >& ) throw (lang::IllegalArgumentException,
                                                                                                                  uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< double >();
    }

    virtual uno::Sequence< double > SAL_CALL convertFromARGB( const uno::Sequence< rendering::ARGBColor >& ) throw (lang::IllegalArgumentException,
                                                                                                                    uno::RuntimeException)
    {
        test(false, "This method is not expected to be called!");
        return uno::Sequence< double >();
    }

    virtual uno::Sequence< double > SAL_CALL convertFromPARGB( const uno::Sequence< rendering::ARGBColor >& ) throw (lang::IllegalArgumentException,
                                                                                                                    uno::RuntimeException)
    {
        test(false, "This method is not expected to be called!");
        return uno::Sequence< double >();
    }

    virtual ::sal_Int32 SAL_CALL getBitsPerPixel(  ) throw (uno::RuntimeException)
    {
        return mnBitsPerPixel;
    }

    virtual uno::Sequence< ::sal_Int32 > SAL_CALL getComponentBitCounts(  ) throw (uno::RuntimeException)
    {
        return maComponentBitCounts;
    }

    virtual ::sal_Int8 SAL_CALL getEndianness(  ) throw (uno::RuntimeException)
    {
        return util::Endianness::LITTLE;
    }

    virtual uno::Sequence< double > SAL_CALL convertFromIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& ,
                                                                           const uno::Reference< rendering::XColorSpace >& ) throw (lang::IllegalArgumentException,
                                                                                                                                    uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< double >();
    }

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertToIntegerColorSpace( const uno::Sequence< ::sal_Int8 >& ,
                                                                             const uno::Reference< rendering::XIntegerBitmapColorSpace >& ) throw (lang::IllegalArgumentException,
                                                                                                                                                   uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< sal_Int8 >();
    }

    virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertIntegerToRGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) throw (lang::IllegalArgumentException,
                                                                                                                                       uno::RuntimeException)
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
                                                                                                                                         uno::RuntimeException)
    {
        const sal_Size  nLen( deviceColor.getLength() );
        const sal_Int32 nBytesPerPixel(mnBitsPerPixel == 8 ? 1 : 4);
        test(nLen%nBytesPerPixel==0,
             "number of channels no multiple of pixel element count");

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

    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToPARGB( const uno::Sequence< ::sal_Int8 >& deviceColor ) throw (lang::IllegalArgumentException,
                                                                                                                                         uno::RuntimeException)
    {
        const sal_Size  nLen( deviceColor.getLength() );
        const sal_Int32 nBytesPerPixel(mnBitsPerPixel == 8 ? 1 : 4);
        test(nLen%nBytesPerPixel==0,
             "number of channels no multiple of pixel element count");

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

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromRGB( const uno::Sequence< rendering::RGBColor >& ) throw (lang::IllegalArgumentException,
                                                                                                                             uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< sal_Int8 >();
    }

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromARGB( const uno::Sequence< rendering::ARGBColor >& ) throw (lang::IllegalArgumentException,
                                                                                                                               uno::RuntimeException)
    {
        test(false, "Method not implemented");
        return uno::Sequence< sal_Int8 >();
    }

    virtual uno::Sequence< ::sal_Int8 > SAL_CALL convertIntegerFromPARGB( const uno::Sequence< rendering::ARGBColor >& ) throw (lang::IllegalArgumentException,
                                                                                                                                uno::RuntimeException)
    {
        test(false, "Method not implemented");
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
        maLayout.IsMsbFirst     = sal_False;
    }
};


//----------------------------------------------------------------------------------

void TestWindow::Paint( const Rectangle& )
{
    static sal_Int8 lcl_depths[]={1,4,8,16,24};

    try
    {
        // Testing VclCanvasBitmap wrapper
        // ===============================

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
                        aBlack.SetIndex( sal::static_int_cast<BYTE>(pAcc->GetBestPaletteIndex(BitmapColor(0,0,0))) );
                        aWhite.SetIndex( sal::static_int_cast<BYTE>(pAcc->GetBestPaletteIndex(BitmapColor(255,255,255))) );
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
        // ======================
        uno::Reference< rendering::XIntegerReadOnlyBitmap > xTestBmp(
            new TestBitmap( geometry::IntegerSize2D(10,10), true ));

        BitmapEx aBmp = vcl::unotools::bitmapExFromXBitmap(xTestBmp);
        test( aBmp.IsTransparent() == false,
              "Palette bitmap is not transparent" );
        test( aBmp.GetSizePixel() == Size(10,10),
              "Bitmap has size (10,10)" );
        test( aBmp.GetBitCount() == 8,
              "Bitmap has bitcount of 8" );
        {
            BitmapReadAccess* pBmpAcc   = aBmp.GetBitmap().AcquireReadAccess();

            test( pBmpAcc,
                  "Bitmap has valid BitmapReadAccess" );

            test(pBmpAcc->GetPixel(0,0) == BitmapColor(0),
                 "(0,0) correct content");
            test(pBmpAcc->GetPixel(2,2) == BitmapColor(2),
                 "(2,2) correct content");
            test(pBmpAcc->GetPixel(2,9) == BitmapColor(9),
                 "(9,2) correct content");

            aBmp.GetBitmap().ReleaseAccess(pBmpAcc);
        }

        xTestBmp.set( new TestBitmap( geometry::IntegerSize2D(10,10), false ));

        aBmp = vcl::unotools::bitmapExFromXBitmap(xTestBmp);
        test( aBmp.IsTransparent() == TRUE,
              "Palette bitmap is transparent" );
        test( aBmp.IsAlpha() == TRUE,
              "Palette bitmap has alpha" );
        test( aBmp.GetSizePixel() == Size(10,10),
              "Bitmap has size (10,10)" );
        test( aBmp.GetBitCount() == 24,
              "Bitmap has bitcount of 24" );
        {
            BitmapReadAccess* pBmpAcc   = aBmp.GetBitmap().AcquireReadAccess();
            BitmapReadAccess* pAlphaAcc = aBmp.GetAlpha().AcquireReadAccess();

            test( pBmpAcc,
                  "Bitmap has valid BitmapReadAccess" );
            test( pAlphaAcc,
                  "Bitmap has valid alpha BitmapReadAccess" );

            test(pBmpAcc->GetPixel(0,0) == BitmapColor(0,1,0),
                 "(0,0) correct content");
            test(pAlphaAcc->GetPixel(0,0) == BitmapColor(255),
                 "(0,0) correct alpha content");
            test(pBmpAcc->GetPixel(2,2) == BitmapColor(0,3,2),
                 "(2,2) correct content");
            test(pAlphaAcc->GetPixel(2,2) == BitmapColor(253),
                 "(2,2) correct alpha content");
            test(pBmpAcc->GetPixel(2,9) == BitmapColor(0,3,9),
                 "(9,2) correct content");
            test(pAlphaAcc->GetPixel(2,9) == BitmapColor(253),
                 "(9,2) correct alpha content");

            aBmp.GetAlpha().ReleaseAccess(pAlphaAcc);
            aBmp.GetBitmap().ReleaseAccess(pBmpAcc);
        }
    }
    catch( uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
        exit(2);
    }
    catch( std::exception& )
    {
        OSL_TRACE( "Caught std exception!" );
    }

    if( g_failure )
        exit(2);
}

} // namespace

void Main()
{
    TestWindow aWindow;
    aWindow.Execute();
    aWindow.SetText( rtl::OUString( "VCL - canvasbitmaptest"  ) );

    Application::Execute();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
