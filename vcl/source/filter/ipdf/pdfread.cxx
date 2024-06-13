/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/pdfread.hxx>
#include <pdf/pdfcompat.hxx>

#include <pdf/PdfConfig.hxx>
#include <vcl/graph.hxx>
#include <vcl/BitmapWriteAccess.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/datetime.hxx>
#include <tools/UnitConversion.hxx>

#include <vcl/filter/PDFiumLibrary.hxx>
#include <sal/log.hxx>

using namespace com::sun::star;

namespace vcl
{
size_t RenderPDFBitmaps(const void* pBuffer, int nSize, std::vector<BitmapEx>& rBitmaps,
                        const size_t nFirstPage, int nPages, const basegfx::B2DTuple* pSizeHint)
{
    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return 0;
    }

    // Load the buffer using pdfium.
    std::unique_ptr<vcl::pdf::PDFiumDocument> pPdfDocument
        = pPdfium->openDocument(pBuffer, nSize, OString());
    if (!pPdfDocument)
        return 0;

    static const double fResolutionDPI = vcl::pdf::getDefaultPdfResolutionDpi();

    const int nPageCount = pPdfDocument->getPageCount();
    if (nPages <= 0)
        nPages = nPageCount;
    const size_t nLastPage = std::min<int>(nPageCount, nFirstPage + nPages) - 1;
    for (size_t nPageIndex = nFirstPage; nPageIndex <= nLastPage; ++nPageIndex)
    {
        // Render next page.
        std::unique_ptr<vcl::pdf::PDFiumPage> pPdfPage = pPdfDocument->openPage(nPageIndex);
        if (!pPdfPage)
            break;

        // Calculate the bitmap size in points.
        double nPageWidthPoints = pPdfPage->getWidth();
        double nPageHeightPoints = pPdfPage->getHeight();
        if (pSizeHint && pSizeHint->getX() && pSizeHint->getY())
        {
            // Have a size hint, prefer that over the logic size from the PDF.
            nPageWidthPoints
                = o3tl::convert(pSizeHint->getX(), o3tl::Length::mm100, o3tl::Length::pt);
            nPageHeightPoints
                = o3tl::convert(pSizeHint->getY(), o3tl::Length::mm100, o3tl::Length::pt);
        }

        // Returned unit is points, convert that to pixel.

        int nPageWidth = std::round(vcl::pdf::pointToPixel(nPageWidthPoints, fResolutionDPI)
                                    * PDF_INSERT_MAGIC_SCALE_FACTOR);
        int nPageHeight = std::round(vcl::pdf::pointToPixel(nPageHeightPoints, fResolutionDPI)
                                     * PDF_INSERT_MAGIC_SCALE_FACTOR);
        std::unique_ptr<vcl::pdf::PDFiumBitmap> pPdfBitmap
            = pPdfium->createBitmap(nPageWidth, nPageHeight, /*nAlpha=*/1);
        if (!pPdfBitmap)
            break;

        bool bTransparent = pPdfPage->hasTransparency();
        if (pSizeHint)
        {
            // This is the PDF-in-EMF case: force transparency, even in case pdfium would tell us
            // the PDF is not transparent.
            bTransparent = true;
        }
        const sal_uInt32 nColor = bTransparent ? 0x00000000 : 0xFFFFFFFF;
        pPdfBitmap->fillRect(0, 0, nPageWidth, nPageHeight, nColor);
        pPdfBitmap->renderPageBitmap(pPdfDocument.get(), pPdfPage.get(), /*nStartX=*/0,
                                     /*nStartY=*/0, nPageWidth, nPageHeight);

        // Save the buffer as a bitmap.
        Bitmap aBitmap(Size(nPageWidth, nPageHeight), vcl::PixelFormat::N24_BPP);
        AlphaMask aMask(Size(nPageWidth, nPageHeight));
        {
            BitmapScopedWriteAccess pWriteAccess(aBitmap);
            BitmapScopedWriteAccess pMaskAccess(aMask);
            ConstScanline pPdfBuffer = pPdfBitmap->getBuffer();
            const int nStride = pPdfBitmap->getStride();
            std::vector<sal_uInt8> aScanlineAlpha(nPageWidth);
            for (int nRow = 0; nRow < nPageHeight; ++nRow)
            {
                ConstScanline pPdfLine = pPdfBuffer + (nStride * nRow);
                // pdfium byte order is BGRA.
                pWriteAccess->CopyScanline(nRow, pPdfLine, ScanlineFormat::N32BitTcBgra, nStride);
                for (int nCol = 0; nCol < nPageWidth; ++nCol)
                {
                    aScanlineAlpha[nCol] = pPdfLine[3];
                    pPdfLine += 4;
                }
                pMaskAccess->CopyScanline(nRow, aScanlineAlpha.data(), ScanlineFormat::N8BitPal,
                                          nPageWidth);
            }
        }

        if (bTransparent)
        {
            rBitmaps.emplace_back(aBitmap, aMask);
        }
        else
        {
            rBitmaps.emplace_back(std::move(aBitmap));
        }
    }

    return rBitmaps.size();
}

bool importPdfVectorGraphicData(SvStream& rStream,
                                std::shared_ptr<VectorGraphicData>& rVectorGraphicData)
{
    BinaryDataContainer aDataContainer = vcl::pdf::createBinaryDataContainer(rStream);
    if (aDataContainer.isEmpty())
    {
        SAL_WARN("vcl.filter", "ImportPDF: empty PDF data array");
        return false;
    }

    rVectorGraphicData
        = std::make_shared<VectorGraphicData>(aDataContainer, VectorGraphicDataType::Pdf);

    return true;
}

bool ImportPDF(SvStream& rStream, Graphic& rGraphic)
{
    std::shared_ptr<VectorGraphicData> pVectorGraphicData;
    if (!importPdfVectorGraphicData(rStream, pVectorGraphicData))
        return false;
    rGraphic = Graphic(pVectorGraphicData);
    return true;
}

namespace
{
basegfx::B2DPoint convertFromPDFInternalToHMM(basegfx::B2DPoint const& rInputPoint,
                                              basegfx::B2DSize const& rPageSize)
{
    double x = convertPointToMm100(rInputPoint.getX());
    double y = convertPointToMm100(rPageSize.getHeight() - rInputPoint.getY());
    return { x, y };
}

std::vector<PDFGraphicAnnotation>
findAnnotations(const std::unique_ptr<vcl::pdf::PDFiumPage>& pPage, basegfx::B2DSize aPageSize)
{
    std::vector<PDFGraphicAnnotation> aPDFGraphicAnnotations;
    if (!pPage)
    {
        return aPDFGraphicAnnotations;
    }

    for (int nAnnotation = 0; nAnnotation < pPage->getAnnotationCount(); nAnnotation++)
    {
        auto pAnnotation = pPage->getAnnotation(nAnnotation);
        if (pAnnotation)
        {
            auto eSubtype = pAnnotation->getSubType();

            if (eSubtype == vcl::pdf::PDFAnnotationSubType::Text
                || eSubtype == vcl::pdf::PDFAnnotationSubType::FreeText
                || eSubtype == vcl::pdf::PDFAnnotationSubType::Polygon
                || eSubtype == vcl::pdf::PDFAnnotationSubType::Circle
                || eSubtype == vcl::pdf::PDFAnnotationSubType::Square
                || eSubtype == vcl::pdf::PDFAnnotationSubType::Ink
                || eSubtype == vcl::pdf::PDFAnnotationSubType::Highlight
                || eSubtype == vcl::pdf::PDFAnnotationSubType::Line
                || eSubtype == vcl::pdf::PDFAnnotationSubType::Stamp)
            {
                basegfx::B2DRectangle rRectangle = pAnnotation->getRectangle();
                basegfx::B2DRectangle rRectangleHMM(
                    convertPointToMm100(rRectangle.getMinX()),
                    convertPointToMm100(aPageSize.getHeight() - rRectangle.getMinY()),
                    convertPointToMm100(rRectangle.getMaxX()),
                    convertPointToMm100(aPageSize.getHeight() - rRectangle.getMaxY()));

                OUString sDateTimeString
                    = pAnnotation->getString(vcl::pdf::constDictionaryKeyModificationDate);
                OUString sISO8601String = vcl::pdf::convertPdfDateToISO8601(sDateTimeString);

                css::util::DateTime aDateTime;
                if (!sISO8601String.isEmpty())
                {
                    utl::ISO8601parseDateTime(sISO8601String, aDateTime);
                }

                Color aColor = pAnnotation->getColor();

                aPDFGraphicAnnotations.emplace_back();

                auto& rPDFGraphicAnnotation = aPDFGraphicAnnotations.back();
                rPDFGraphicAnnotation.maRectangle = rRectangleHMM;
                rPDFGraphicAnnotation.maAuthor
                    = pAnnotation->getString(vcl::pdf::constDictionaryKeyTitle);
                rPDFGraphicAnnotation.maText
                    = pAnnotation->getString(vcl::pdf::constDictionaryKeyContents);
                rPDFGraphicAnnotation.maDateTime = aDateTime;
                rPDFGraphicAnnotation.meSubType = eSubtype;
                rPDFGraphicAnnotation.maColor = aColor;

                if (eSubtype == vcl::pdf::PDFAnnotationSubType::Polygon)
                {
                    auto const& rVertices = pAnnotation->getVertices();
                    if (!rVertices.empty())
                    {
                        auto pMarker = std::make_shared<vcl::pdf::PDFAnnotationMarkerPolygon>();
                        rPDFGraphicAnnotation.mpMarker = pMarker;
                        for (auto const& rVertex : rVertices)
                        {
                            auto aPoint = convertFromPDFInternalToHMM(rVertex, aPageSize);
                            pMarker->maPolygon.append(aPoint);
                        }
                        pMarker->maPolygon.setClosed(true);
                        pMarker->mnWidth = convertPointToMm100(pAnnotation->getBorderWidth());
                        if (pAnnotation->hasKey(vcl::pdf::constDictionaryKeyInteriorColor))
                            pMarker->maFillColor = pAnnotation->getInteriorColor();
                    }
                }
                else if (eSubtype == vcl::pdf::PDFAnnotationSubType::Square)
                {
                    auto pMarker = std::make_shared<vcl::pdf::PDFAnnotationMarkerSquare>();
                    rPDFGraphicAnnotation.mpMarker = pMarker;
                    pMarker->mnWidth = convertPointToMm100(pAnnotation->getBorderWidth());
                    if (pAnnotation->hasKey(vcl::pdf::constDictionaryKeyInteriorColor))
                        pMarker->maFillColor = pAnnotation->getInteriorColor();
                }
                else if (eSubtype == vcl::pdf::PDFAnnotationSubType::Circle)
                {
                    auto pMarker = std::make_shared<vcl::pdf::PDFAnnotationMarkerCircle>();
                    rPDFGraphicAnnotation.mpMarker = pMarker;
                    pMarker->mnWidth = convertPointToMm100(pAnnotation->getBorderWidth());
                    if (pAnnotation->hasKey(vcl::pdf::constDictionaryKeyInteriorColor))
                        pMarker->maFillColor = pAnnotation->getInteriorColor();
                }
                else if (eSubtype == vcl::pdf::PDFAnnotationSubType::Ink)
                {
                    auto const& rStrokesList = pAnnotation->getInkStrokes();
                    if (!rStrokesList.empty())
                    {
                        auto pMarker = std::make_shared<vcl::pdf::PDFAnnotationMarkerInk>();
                        rPDFGraphicAnnotation.mpMarker = pMarker;
                        for (auto const& rStrokes : rStrokesList)
                        {
                            basegfx::B2DPolygon aPolygon;
                            for (auto const& rVertex : rStrokes)
                            {
                                auto aPoint = convertFromPDFInternalToHMM(rVertex, aPageSize);
                                aPolygon.append(aPoint);
                            }
                            pMarker->maStrokes.push_back(aPolygon);
                        }
                        float fWidth = pAnnotation->getBorderWidth();
                        pMarker->mnWidth = convertPointToMm100(fWidth);
                        if (pAnnotation->hasKey(vcl::pdf::constDictionaryKeyInteriorColor))
                            pMarker->maFillColor = pAnnotation->getInteriorColor();
                    }
                }
                else if (eSubtype == vcl::pdf::PDFAnnotationSubType::Highlight)
                {
                    size_t nCount = pAnnotation->getAttachmentPointsCount();
                    if (nCount > 0)
                    {
                        auto pMarker = std::make_shared<vcl::pdf::PDFAnnotationMarkerHighlight>(
                            vcl::pdf::PDFTextMarkerType::Highlight);
                        rPDFGraphicAnnotation.mpMarker = pMarker;
                        for (size_t i = 0; i < nCount; ++i)
                        {
                            auto aAttachmentPoints = pAnnotation->getAttachmentPoints(i);
                            if (!aAttachmentPoints.empty())
                            {
                                basegfx::B2DPolygon aPolygon;
                                aPolygon.setClosed(true);

                                auto aPoint1
                                    = convertFromPDFInternalToHMM(aAttachmentPoints[0], aPageSize);
                                aPolygon.append(aPoint1);
                                auto aPoint2
                                    = convertFromPDFInternalToHMM(aAttachmentPoints[1], aPageSize);
                                aPolygon.append(aPoint2);
                                auto aPoint3
                                    = convertFromPDFInternalToHMM(aAttachmentPoints[3], aPageSize);
                                aPolygon.append(aPoint3);
                                auto aPoint4
                                    = convertFromPDFInternalToHMM(aAttachmentPoints[2], aPageSize);
                                aPolygon.append(aPoint4);

                                pMarker->maQuads.push_back(aPolygon);
                            }
                        }
                    }
                }
                else if (eSubtype == vcl::pdf::PDFAnnotationSubType::Line)
                {
                    auto const& rLineGeometry = pAnnotation->getLineGeometry();
                    if (!rLineGeometry.empty())
                    {
                        auto pMarker = std::make_shared<vcl::pdf::PDFAnnotationMarkerLine>();
                        rPDFGraphicAnnotation.mpMarker = pMarker;

                        auto aPoint1 = convertFromPDFInternalToHMM(rLineGeometry[0], aPageSize);
                        pMarker->maLineStart = aPoint1;

                        auto aPoint2 = convertFromPDFInternalToHMM(rLineGeometry[1], aPageSize);
                        pMarker->maLineEnd = aPoint2;

                        float fWidth = pAnnotation->getBorderWidth();
                        pMarker->mnWidth = convertPointToMm100(fWidth);
                    }
                }
                else if (eSubtype == vcl::pdf::PDFAnnotationSubType::FreeText)
                {
                    auto pMarker = std::make_shared<vcl::pdf::PDFAnnotationMarkerFreeText>();
                    rPDFGraphicAnnotation.mpMarker = pMarker;
                }
                else if (eSubtype == vcl::pdf::PDFAnnotationSubType::Stamp)
                {
                    auto pMarker = std::make_shared<vcl::pdf::PDFAnnotationMarkerStamp>();
                    rPDFGraphicAnnotation.mpMarker = pMarker;

                    auto nObjects = pAnnotation->getObjectCount();

                    for (int nIndex = 0; nIndex < nObjects; nIndex++)
                    {
                        auto pPageObject = pAnnotation->getObject(nIndex);
                        if (pPageObject->getType() == vcl::pdf::PDFPageObjectType::Image)
                        {
                            std::unique_ptr<vcl::pdf::PDFiumBitmap> pBitmap
                                = pPageObject->getImageBitmap();
                            pMarker->maBitmapEx = pBitmap->createBitmapFromBuffer();
                        }
                    }
                }
            }
        }
    }
    return aPDFGraphicAnnotations;
}

} // end anonymous namespace

size_t ImportPDFUnloaded(const OUString& rURL, std::vector<PDFGraphicResult>& rGraphics)
{
    std::unique_ptr<SvStream> xStream(
        ::utl::UcbStreamHelper::CreateStream(rURL, StreamMode::READ | StreamMode::SHARE_DENYNONE));

    // Save the original PDF stream for later use.
    BinaryDataContainer aDataContainer = vcl::pdf::createBinaryDataContainer(*xStream);
    if (aDataContainer.isEmpty())
        return 0;

    // Prepare the link with the PDF stream.
    auto pGfxLink = std::make_shared<GfxLink>(aDataContainer, GfxLinkType::NativePdf);

    auto pPdfium = vcl::pdf::PDFiumLibrary::get();
    if (!pPdfium)
    {
        return 0;
    }

    // Load the buffer using pdfium.
    auto pPdfDocument
        = pPdfium->openDocument(pGfxLink->GetData(), pGfxLink->GetDataSize(), OString());

    if (!pPdfDocument)
        return 0;

    const int nPageCount = pPdfDocument->getPageCount();
    if (nPageCount <= 0)
        return 0;

    for (int nPageIndex = 0; nPageIndex < nPageCount; ++nPageIndex)
    {
        basegfx::B2DSize aPageSize = pPdfDocument->getPageSize(nPageIndex);
        if (aPageSize.getWidth() <= 0.0 || aPageSize.getHeight() <= 0.0)
            continue;

        // Returned unit is points

        tools::Long nPageWidth = std::round(convertPointToMm100(aPageSize.getWidth()));
        tools::Long nPageHeight = std::round(convertPointToMm100(aPageSize.getHeight()));

        // Create the Graphic with the VectorGraphicDataPtr and link the original PDF stream.
        // We swap out this Graphic as soon as possible, and a later swap in
        // actually renders the correct Bitmap on demand.
        Graphic aGraphic(pGfxLink, nPageIndex);

        auto pPage = pPdfDocument->openPage(nPageIndex);

        std::vector<PDFGraphicAnnotation> aPDFGraphicAnnotations
            = findAnnotations(pPage, aPageSize);

        rGraphics.emplace_back(std::move(aGraphic), Size(nPageWidth, nPageHeight),
                               aPDFGraphicAnnotations);
    }

    return rGraphics.size();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
