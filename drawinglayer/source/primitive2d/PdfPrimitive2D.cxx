/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/primitive2d/PdfPrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/ViewDependentTools.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <basegfx/utils/tools.hxx>
#include <vcl/filter/PDFiumLibrary.hxx>
#include <vcl/bitmap.hxx>

namespace drawinglayer::primitive2d
{
PdfPrimitive2D::PdfPrimitive2D(BinaryDataContainer const& rDataContainer, sal_Int32 nPageIndex,
                               basegfx::B2DHomMatrix const& rTransform)
    : maDataContainer(rDataContainer)
    , mnPageIndex(nPageIndex)
    , maTransform(rTransform)
    , mfPreviousDiscreteSizeX(0.0)
    , mfPreviousDiscreteSizeY(0.0)
{
    activateFlushOnTimer();
}

bool PdfPrimitive2D::ensurePdfium() const
{
    if (mpPdfium && mpPdfDocument && mpPdfPage)
        return true;

    if (!mpPdfium)
    {
        mpPdfium = vcl::pdf::PDFiumLibrary::get();
        if (!mpPdfium)
            return false;
    }

    if (!mpPdfDocument)
    {
        mpPdfDocument = mpPdfium->openDocument(maDataContainer.getData(), maDataContainer.getSize(),
                                               OString());
        if (!mpPdfDocument)
            return false;
    }

    if (!mpPdfPage)
    {
        mpPdfPage = mpPdfDocument->openPage(mnPageIndex);
        if (!mpPdfPage)
            return false;
    }

    return true;
}

vcl::pdf::PDFiumDocument* PdfPrimitive2D::getPdfDocument() const
{
    ensurePdfium();
    return mpPdfDocument.get();
}

vcl::pdf::PDFiumPage* PdfPrimitive2D::getPdfPage() const
{
    ensurePdfium();
    return mpPdfPage.get();
}

bool PdfPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const PdfPrimitive2D& rCompare = static_cast<const PdfPrimitive2D&>(rPrimitive);

        return (getDataContainer().getData() == rCompare.getDataContainer().getData()
                && getDataContainer().getSize() == rCompare.getDataContainer().getSize()
                && getPageIndex() == rCompare.getPageIndex()
                && getTransform() == rCompare.getTransform());
    }

    return false;
}

basegfx::B2DRange
PdfPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
    aRetval.transform(getTransform());
    return aRetval;
}

Primitive2DReference
PdfPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
{
    if (!ensurePdfium())
        return nullptr;

    // calculate discrete sizes
    basegfx::B2DRange aDiscreteRange;
    basegfx::B2DRange aVisibleDiscreteRange;
    basegfx::B2DRange aUnitVisibleRange;
    calculateDiscreteVisibleRanges(rViewInformation, getTransform(), aDiscreteRange,
                                   aVisibleDiscreteRange, aUnitVisibleRange);

    const double fDeviceScale = rViewInformation.getDeviceScaleFactor();

    // full page size in pixels (for PDFium startX/startY calculation)
    sal_Int32 nFullWidth = basegfx::fround(aDiscreteRange.getWidth() * fDeviceScale);
    sal_Int32 nFullHeight = basegfx::fround(aDiscreteRange.getHeight() * fDeviceScale);

    // render only the visible portion at full resolution
    sal_Int32 nVisibleWidth = basegfx::fround(aVisibleDiscreteRange.getWidth() * fDeviceScale);
    sal_Int32 nVisibleHeight = basegfx::fround(aVisibleDiscreteRange.getHeight() * fDeviceScale);

    // if view information is not available, fall back to a fixed size based on the page dimensions
    if (nFullWidth <= 0 || nFullHeight <= 0)
    {
        // render at approximately 96 DPI
        nFullWidth = basegfx::fround(mpPdfPage->getWidth() * 96.0 / 72.0);
        nFullHeight = basegfx::fround(mpPdfPage->getHeight() * 96.0 / 72.0);
        nVisibleWidth = nFullWidth;
        nVisibleHeight = nFullHeight;
        aUnitVisibleRange = basegfx::B2DRange(0.0, 0.0, 1.0, 1.0);
    }

    constexpr sal_Int32 nMaxPixels = 4096; // max allowed pixel width and height
    if (nVisibleWidth > nMaxPixels || nVisibleHeight > nMaxPixels)
    {
        const double fScale = double(nMaxPixels) / std::max(nVisibleWidth, nVisibleHeight);
        nVisibleWidth = basegfx::fround(nVisibleWidth * fScale);
        nVisibleHeight = basegfx::fround(nVisibleHeight * fScale);
    }

    if (nVisibleWidth <= 0 || nVisibleHeight <= 0)
        return nullptr;

    int nBitmapWidth = nVisibleWidth;
    int nBitmapHeight = nVisibleHeight;
    std::unique_ptr<vcl::pdf::PDFiumBitmap> pPdfBitmap
        = mpPdfium->createBitmap(nBitmapWidth, nBitmapHeight, /*nAlpha=*/1);
    if (!pPdfBitmap)
        return nullptr;

    // always use transparent background - if the PDF page has an opaque
    // background, PDFium will render it.
    pPdfBitmap->fillRect(0, 0, nVisibleWidth, nVisibleHeight, 0x00000000);

    // offset so the visible portion starts at (0,0) in the bitmap
    const int nStartX = -basegfx::fround(aUnitVisibleRange.getMinX() * nFullWidth);
    const int nStartY = -basegfx::fround(aUnitVisibleRange.getMinY() * nFullHeight);
    pPdfBitmap->renderPageBitmap(mpPdfDocument.get(), mpPdfPage.get(), nStartX, nStartY, nFullWidth,
                                 nFullHeight);

    Bitmap aBitmap = pPdfBitmap->createBitmapFromBuffer();
    if (aBitmap.IsEmpty())
        return nullptr;

    // compute the transform for the visible portion only
    // map from unit visible range back to world coordinates via the object transform
    const double fUnitMinX = aUnitVisibleRange.getMinX();
    const double fUnitMinY = aUnitVisibleRange.getMinY();
    const double fUnitW = aUnitVisibleRange.getWidth();
    const double fUnitH = aUnitVisibleRange.getHeight();

    basegfx::B2DHomMatrix aVisibleTransform;
    aVisibleTransform.scale(fUnitW, fUnitH);
    aVisibleTransform.translate(fUnitMinX, fUnitMinY);
    aVisibleTransform = getTransform() * aVisibleTransform;

    return new BitmapPrimitive2D(std::move(aBitmap), aVisibleTransform);
}

void PdfPrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                                        const geometry::ViewInformation2D& rViewInformation) const
{
    basegfx::B2DRange aDiscreteRange;
    basegfx::B2DRange aUnitVisibleRange;
    bool bNeedToDecompose(false);
    bool bDiscreteSizesAreCalculated(false);

    if (hasBuffered2DDecomposition())
    {
        basegfx::B2DRange aVisibleDiscreteRange;
        calculateDiscreteVisibleRanges(rViewInformation, getTransform(), aDiscreteRange,
                                       aVisibleDiscreteRange, aUnitVisibleRange);
        bDiscreteSizesAreCalculated = true;

        if (!maPreviousUnitVisiblePart.isInside(aUnitVisibleRange))
        {
            bNeedToDecompose = true;
        }

        if (!bNeedToDecompose && mfPreviousDiscreteSizeX > 0.0 && mfPreviousDiscreteSizeY > 0.0)
        {
            const double fWidthRatio = aDiscreteRange.getWidth() / mfPreviousDiscreteSizeX;
            const double fHeightRatio = aDiscreteRange.getHeight() / mfPreviousDiscreteSizeY;

            if (fWidthRatio > 1.1 || fWidthRatio < 0.9 || fHeightRatio > 1.1 || fHeightRatio < 0.9)
            {
                bNeedToDecompose = true;
            }
        }
    }

    if (bNeedToDecompose)
    {
        const_cast<PdfPrimitive2D*>(this)->setBuffered2DDecomposition(nullptr);
    }

    if (!hasBuffered2DDecomposition())
    {
        if (!bDiscreteSizesAreCalculated)
        {
            basegfx::B2DRange aVisibleDiscreteRange;
            calculateDiscreteVisibleRanges(rViewInformation, getTransform(), aDiscreteRange,
                                           aVisibleDiscreteRange, aUnitVisibleRange);
        }

        PdfPrimitive2D* pModifiable = const_cast<PdfPrimitive2D*>(this);
        pModifiable->mfPreviousDiscreteSizeX = aDiscreteRange.getWidth();
        pModifiable->mfPreviousDiscreteSizeY = aDiscreteRange.getHeight();
        pModifiable->maPreviousUnitVisiblePart = aUnitVisibleRange;
    }

    BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
}

sal_uInt32 PdfPrimitive2D::getPrimitive2DID() const { return PRIMITIVE2D_ID_PDFPRIMITIVE2D; }

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
