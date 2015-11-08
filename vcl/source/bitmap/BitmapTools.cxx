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

using namespace css;

using drawinglayer::primitive2d::Primitive2DSequence;
using drawinglayer::primitive2d::Primitive2DReference;

namespace vcl
{

void BitmapTools::loadFromSvg(SvStream& rStream, const OUString& sPath, BitmapEx& rBitmapEx, double fScalingFactor, const Size& aSize)
{
    uno::Reference<uno::XComponentContext> xContext(comphelper::getProcessComponentContext());
    const uno::Reference<graphic::XSvgParser> xSvgParser = graphic::SvgTools::create(xContext);

    sal_Size nSize = rStream.remainingSize();
    std::vector<sal_Int8> pBuffer(nSize + 1);
    rStream.Read(pBuffer.data(), nSize);
    pBuffer[nSize] = 0;

    uno::Sequence<sal_Int8> aData(pBuffer.data(), nSize + 1);
    uno::Reference<io::XInputStream> aInputStream(new comphelper::SequenceInputStream(aData));

    Primitive2DSequence aPrimitiveSequence = xSvgParser->getDecomposition(aInputStream, sPath);

    if (aPrimitiveSequence.hasElements())
    {
        uno::Sequence<beans::PropertyValue> aViewParameters;

        const sal_Int32 nCount(aPrimitiveSequence.getLength());
        geometry::RealRectangle2D aRealRect;
        basegfx::B2DRange aRange;
        for (sal_Int32 a = 0L; a < nCount; ++a)
        {
            const Primitive2DReference xReference(aPrimitiveSequence[a]);

            if (xReference.is())
            {
                aRealRect = xReference->getRange(aViewParameters);
                aRange.expand(basegfx::B2DRange(aRealRect.X1, aRealRect.Y1, aRealRect.X2, aRealRect.Y2));
            }
        }

        bool bIsSizeEmpty = (aSize.Width() == 0 && aSize.Height() == 0);

        aRealRect.X1 = 0;
        aRealRect.Y1 = 0;
        aRealRect.X2 = bIsSizeEmpty ? (aSize.Width() * 2540 / 90)  : aRange.getMaxX() - aRange.getMinX();
        aRealRect.Y2 = bIsSizeEmpty ? (aSize.Height() * 2540 / 90) : aRange.getMaxY() - aRange.getMinY();

        double nDPI = 90 * fScalingFactor;

        const css::uno::Reference<css::graphic::XPrimitive2DRenderer> xPrimitive2DRenderer = css::graphic::Primitive2DTools::create(xContext);
        const css::uno::Reference<css::rendering::XBitmap> xBitmap(
            xPrimitive2DRenderer->rasterize(aPrimitiveSequence, aViewParameters, nDPI, nDPI, aRealRect, 256*256));

        printf("%f %f %f %f\n", aRealRect.X1, aRealRect.Y1, aRealRect.X2, aRealRect.Y2);

        if (xBitmap.is())
        {
            const css::uno::Reference<css::rendering::XIntegerReadOnlyBitmap> xIntBmp(xBitmap, uno::UNO_QUERY_THROW);

            if (xIntBmp.is())
            {
                rBitmapEx = vcl::unotools::bitmapExFromXBitmap(xIntBmp);
                printf("Size: %d %d\n", rBitmapEx.GetSizePixel().Width(), rBitmapEx.GetSizePixel().Height());
            }
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
