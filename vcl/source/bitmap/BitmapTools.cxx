/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BitmapTools.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/seqstream.hxx>
#include <vcl/canvastools.hxx>

#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>

#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>

#include <unotools/resmgr.hxx>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <vcl/salbtype.hxx>
#include <vcl/bitmapaccess.hxx>

using namespace css;

using drawinglayer::primitive2d::Primitive2DSequence;
using drawinglayer::primitive2d::Primitive2DReference;

namespace vcl
{

namespace bitmap
{

BitmapEx loadFromName(const OUString& rFileName, const ImageLoadFlags eFlags)
{
    BitmapEx aBitmapEx;

    OUString aIconTheme = Application::GetSettings().GetStyleSettings().DetermineIconTheme();

    ImageTree::get().loadImage(rFileName, aIconTheme, aBitmapEx, true, eFlags);

    return aBitmapEx;
}

void loadFromSvg(SvStream& rStream, const OUString& sPath, BitmapEx& rBitmapEx, double fScalingFactor)
{
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    const uno::Reference<graphic::XSvgParser> xSvgParser = graphic::SvgTools::create(xContext);

    std::size_t nSize = rStream.remainingSize();
    std::vector<sal_Int8> aBuffer(nSize + 1);
    rStream.ReadBytes(aBuffer.data(), nSize);
    aBuffer[nSize] = 0;

    uno::Sequence<sal_Int8> aData(aBuffer.data(), nSize + 1);
    uno::Reference<io::XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));

    Primitive2DSequence aPrimitiveSequence = xSvgParser->getDecomposition(aInputStream, sPath);

    if (!aPrimitiveSequence.hasElements())
        return;

    uno::Sequence<beans::PropertyValue> aViewParameters;

    geometry::RealRectangle2D aRealRect;
    basegfx::B2DRange aRange;
    for (Primitive2DReference const & xReference : aPrimitiveSequence)
    {
        if (xReference.is())
        {
            aRealRect = xReference->getRange(aViewParameters);
            aRange.expand(basegfx::B2DRange(aRealRect.X1, aRealRect.Y1, aRealRect.X2, aRealRect.Y2));
        }
    }

    aRealRect.X1 = aRange.getMinX();
    aRealRect.Y1 = aRange.getMinY();
    aRealRect.X2 = aRange.getMaxX();
    aRealRect.Y2 = aRange.getMaxY();

    double nDPI = 96 * fScalingFactor;

    const css::uno::Reference<css::graphic::XPrimitive2DRenderer> xPrimitive2DRenderer = css::graphic::Primitive2DTools::create(xContext);
    const css::uno::Reference<css::rendering::XBitmap> xBitmap(
        xPrimitive2DRenderer->rasterize(aPrimitiveSequence, aViewParameters, nDPI, nDPI, aRealRect, 256*256));

    if (xBitmap.is())
    {
        const css::uno::Reference<css::rendering::XIntegerReadOnlyBitmap> xIntBmp(xBitmap, uno::UNO_QUERY_THROW);

        if (xIntBmp.is())
        {
            rBitmapEx = vcl::unotools::bitmapExFromXBitmap(xIntBmp);
        }
    }

}

/** Copy block of image data into the bitmap.
    Assumes that the Bitmap has been constructed with the desired size.

    @param pData
    The block of data to copy
    @param nStride
    The number of bytes in a scanline, must >= width
*/
BitmapEx CreateFromData( sal_uInt8 const *pData, sal_Int32 nWidth, sal_Int32 nHeight, sal_Int32 nStride, sal_uInt16 nBitCount )
{
    assert(nStride >= nWidth);
    Bitmap aBmp( Size( nWidth, nHeight ), nBitCount );

    Bitmap::ScopedWriteAccess pWrite(aBmp);
    assert(pWrite.get());
    if( pWrite.get() )
    {
        for( long y = 0; y < nHeight; ++y )
        {
            sal_uInt8 const *p = pData + y * nStride;
            Scanline pScanline = pWrite->GetScanline(y);
            for (long x = 0; x < nWidth; ++x)
            {
                BitmapColor col(p[0], p[1], p[2]);
                pWrite->SetPixelOnData(pScanline, x, col);
                p += 3;
            }
        }
    }
    return aBmp;
}

/** Copy block of image data into the bitmap.
    Assumes that the Bitmap has been constructed with the desired size.
*/
BitmapEx CreateFromData( RawBitmap&& rawBitmap )
{
    Bitmap aBmp( rawBitmap.maSize, /*nBitCount*/24 );

    Bitmap::ScopedWriteAccess pWrite(aBmp);
    assert(pWrite.get());
    if( !pWrite )
        return BitmapEx();

    auto nHeight = rawBitmap.maSize.getHeight();
    auto nWidth = rawBitmap.maSize.getWidth();
    for( long y = 0; y < nHeight; ++y )
    {
        sal_uInt8 const *p = rawBitmap.mpData.get() + (y * nWidth * 3);
        Scanline pScanline = pWrite->GetScanline(y);
        for (long x = 0; x < nWidth; ++x)
        {
            BitmapColor col(p[0], p[1], p[2]);
            pWrite->SetPixelOnData(pScanline, x, col);
            p += 3;
        }
    }
    return aBmp;
}

}} // end vcl::bitmap

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
