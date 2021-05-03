/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <filter/pdfimporter/PDFImporter.hxx>

#include <sal/log.hxx>
#include <tools/UnitConversion.hxx>
#include <tools/color.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/BitmapTools.hxx>

#include <cmath>

#include <toolkit/helper/vclunohelper.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolyPolygonStrokePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>

PDFImporter::PDFImporter(BinaryDataContainer& rDataContainer)
    : mpPDFium(vcl::pdf::PDFiumLibrary::get())
{
    auto* pData = rDataContainer.getData();
    sal_Int32 nSize = rDataContainer.getSize();
    OString aEmptyPassword;
    mpPdfDocument = mpPDFium->openDocument(pData, nSize, aEmptyPassword);
}

namespace
{
void setupPage(drawinglayer::primitive2d::Primitive2DContainer& rContainer,
               basegfx::B2DSize const& rPageSize)
{
    basegfx::B2DRange aPageRange(0.0, 0.0, rPageSize.getWidth(), rPageSize.getHeight());

    printf("Page Size %.2fpt %.2fpt\n", rPageSize.getWidth(), rPageSize.getHeight());

    const auto aPolygon = basegfx::utils::createPolygonFromRect(aPageRange);

    const drawinglayer::primitive2d::Primitive2DReference xPage(
        new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aPolygon,
                                                                  basegfx::BColor(0.0, 0.0, 0.0)));
    const drawinglayer::primitive2d::Primitive2DReference xHidden(
        new drawinglayer::primitive2d::HiddenGeometryPrimitive2D(
            drawinglayer::primitive2d::Primitive2DContainer{ xPage }));

    rContainer.push_back(xHidden);
}

double sqrt2(double a, double b) { return sqrt(a * a + b * b); }

} // end anonymous namespace

bool PDFImporter::importPage(int nPageIndex,
                             drawinglayer::primitive2d::Primitive2DContainer& rContainer)
{
    if (!mpPdfDocument)
        return false;

    drawinglayer::primitive2d::Primitive2DContainer aContent;

    const int nPageCount = mpPdfDocument->getPageCount();
    if (!(nPageCount > 0 && nPageIndex >= 0 && nPageIndex < nPageCount))
        return false;

    mpPdfPage = mpPdfDocument->openPage(nPageIndex);
    if (!mpPdfPage)
        return false;

    basegfx::B2DSize aPageSize = mpPdfDocument->getPageSize(nPageIndex);

    setupPage(aContent, aPageSize);

    // Load the page text to extract it when we get text elements.
    auto pTextPage = mpPdfPage->getTextPage();

    const int nPageObjectCount = mpPdfPage->getObjectCount();

    for (int nPageObjectIndex = 0; nPageObjectIndex < nPageObjectCount; ++nPageObjectIndex)
    {
        auto pPageObject = mpPdfPage->getObject(nPageObjectIndex);
        importPdfObject(pPageObject, pTextPage, nPageObjectIndex, aContent);
    }

    // point to pixel conversion
    double dConversionFactor = double(conversionFract(o3tl::Length::pt, o3tl::Length::px));
    const auto aTransform = basegfx::utils::createScaleTranslateB2DHomMatrix(
        dConversionFactor, -dConversionFactor, 0.0, aPageSize.getWidth() * dConversionFactor);

    const drawinglayer::primitive2d::Primitive2DReference xTransform(
        new drawinglayer::primitive2d::TransformPrimitive2D(aTransform, std::move(aContent)));

    rContainer.push_back(xTransform);

    return true;
}

void PDFImporter::importPdfObject(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                                  std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                                  int nPageObjectIndex,
                                  drawinglayer::primitive2d::Primitive2DContainer& rContent)
{
    if (!pPageObject)
        return;

    const vcl::pdf::PDFPageObjectType ePageObjectType = pPageObject->getType();
    switch (ePageObjectType)
    {
        case vcl::pdf::PDFPageObjectType::Text:
            printf("pdf::PDFPageObjectType::Text\n");
            importText(pPageObject, pTextPage, rContent);
            break;
        case vcl::pdf::PDFPageObjectType::Path:
            printf("pdf::PDFPageObjectType::Path\n");
            importPath(pPageObject, rContent);
            break;
        case vcl::pdf::PDFPageObjectType::Image:
            printf("pdf::PDFPageObjectType::Image\n");
            importImage(pPageObject, rContent);
            break;
        case vcl::pdf::PDFPageObjectType::Shading:
            printf("pdf::PDFPageObjectType::Shading\n");
            break;
        case vcl::pdf::PDFPageObjectType::Form:
            printf("pdf::PDFPageObjectType::Form\n");
            break;
        case vcl::pdf::PDFPageObjectType::Unknown:
            SAL_WARN("filter", "Unknown PDF page object #" << nPageObjectIndex
                                                           << " of type: " << int(ePageObjectType));
            break;
    }
}

void PDFImporter::importText(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                             std::unique_ptr<vcl::pdf::PDFiumTextPage> const& pTextPage,
                             drawinglayer::primitive2d::Primitive2DContainer& rRootContainer)
{
    basegfx::B2DRectangle aTextRect = pPageObject->getBounds();
    basegfx::B2DHomMatrix aMatrix = pPageObject->getMatrix();

    OUString sText = pPageObject->getText(pTextPage);

    const double dFontSize = pPageObject->getFontSize();
    double dFontSizeH = std::fabs(sqrt2(aMatrix.a(), aMatrix.c()) * dFontSize);
    double dFontSizeV = std::fabs(sqrt2(aMatrix.b(), aMatrix.d()) * dFontSize);

    OUString sFontName = pPageObject->getFontName();

    printf("TEXT: %s\n", sText.toUtf8().getStr());

    Color aTextColor(COL_TRANSPARENT);
    bool bFill = false;
    bool bUse = true;

    switch (pPageObject->getTextRenderMode())
    {
        case vcl::pdf::PDFTextRenderMode::Fill:
        case vcl::pdf::PDFTextRenderMode::FillClip:
        case vcl::pdf::PDFTextRenderMode::FillStroke:
        case vcl::pdf::PDFTextRenderMode::FillStrokeClip:
            bFill = true;
            break;
        case vcl::pdf::PDFTextRenderMode::Stroke:
        case vcl::pdf::PDFTextRenderMode::StrokeClip:
        case vcl::pdf::PDFTextRenderMode::Unknown:
            break;
        case vcl::pdf::PDFTextRenderMode::Invisible:
        case vcl::pdf::PDFTextRenderMode::Clip:
            bUse = false;
            break;
    }

    if (bUse)
    {
        Color aColor = bFill ? pPageObject->getFillColor() : pPageObject->getStrokeColor();
        if (aColor != COL_TRANSPARENT)
        {
            aTextColor = aColor.GetRGBColor();
        }
    }
}

void PDFImporter::importImage(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                              drawinglayer::primitive2d::Primitive2DContainer& rRootContainer)
{
    std::unique_ptr<vcl::pdf::PDFiumBitmap> pPdfBitmap = pPageObject->getImageBitmap();
    if (!pPdfBitmap)
    {
        SAL_WARN("filter", "Failed to get IMAGE");
        return;
    }

    const vcl::pdf::PDFBitmapType eFormat = pPdfBitmap->getFormat();
    if (eFormat == vcl::pdf::PDFBitmapType::Unknown)
    {
        SAL_WARN("filter", "Failed to get IMAGE format");
        return;
    }

    const sal_uInt8* pBuffer = pPdfBitmap->getBuffer();
    const int nWidth = pPdfBitmap->getWidth();
    const int nHeight = pPdfBitmap->getHeight();
    const int nStride = pPdfBitmap->getStride();

    BitmapEx aBitmap;

    switch (eFormat)
    {
        case vcl::pdf::PDFBitmapType::BGR:
            printf("vcl::pdf::PDFBitmapType::BGR\n");
            aBitmap = vcl::bitmap::CreateFromData(pBuffer, nWidth, nHeight, nStride, 24);
            break;
        case vcl::pdf::PDFBitmapType::BGRx:
            printf("vcl::pdf::PDFBitmapType::BGRx\n");
            aBitmap = vcl::bitmap::CreateFromData(pBuffer, nWidth, nHeight, nStride, 32);
            break;
        case vcl::pdf::PDFBitmapType::BGRA:
            printf("vcl::pdf::PDFBitmapType::BGRA\n");
            aBitmap = vcl::bitmap::CreateFromData(pBuffer, nWidth, nHeight, nStride, 32);
            break;
        case vcl::pdf::PDFBitmapType::Gray:
            // TODO
        default:
            SAL_WARN("filter", "Got IMAGE width: " << nWidth << ", height: " << nHeight
                                                   << ", stride: " << nStride
                                                   << ", format: " << int(eFormat));
            break;
    }

    basegfx::B2DRectangle aBounds = pPageObject->getBounds();

    rRootContainer.push_back(new drawinglayer::primitive2d::BitmapPrimitive2D(
        aBitmap, basegfx::utils::createScaleTranslateB2DHomMatrix(aBounds.getRange(),
                                                                  aBounds.getMinimum())));
}

void PDFImporter::importPath(std::unique_ptr<vcl::pdf::PDFiumPageObject> const& pPageObject,
                             drawinglayer::primitive2d::Primitive2DContainer& rRootContainer)
{
    drawinglayer::primitive2d::Primitive2DContainer aContent;

    basegfx::B2DHomMatrix aPathMatrix = pPageObject->getMatrix();

    basegfx::B2DPolyPolygon aPolyPolygon;
    basegfx::B2DPolygon aPolygon;
    std::vector<basegfx::B2DPoint> aBezier;

    const int nSegments = pPageObject->getPathSegmentCount();
    for (int nSegmentIndex = 0; nSegmentIndex < nSegments; ++nSegmentIndex)
    {
        auto pPathSegment = pPageObject->getPathSegment(nSegmentIndex);
        if (!pPathSegment)
            continue;

        basegfx::B2DPoint aB2DPoint = pPathSegment->getPoint();

        aPolygon.setClosed(pPathSegment->isClosed());

        const vcl::pdf::PDFSegmentType eSegmentType = pPathSegment->getType();
        switch (eSegmentType)
        {
            case vcl::pdf::PDFSegmentType::Lineto:
            {
                aPolygon.append(aB2DPoint);
            }
            break;

            case vcl::pdf::PDFSegmentType::Bezierto:
            {
                aBezier.emplace_back(aB2DPoint.getX(), aB2DPoint.getY());
                if (aBezier.size() == 3)
                {
                    aPolygon.appendBezierSegment(aBezier[0], aBezier[1], aBezier[2]);
                    aBezier.clear();
                }
            }
            break;

            case vcl::pdf::PDFSegmentType::Moveto:
            {
                if (aPolygon.count() > 0)
                {
                    aPolyPolygon.append(aPolygon);
                    aPolygon.clear();
                }

                aPolygon.append(aB2DPoint);
            }
            break;

            case vcl::pdf::PDFSegmentType::Unknown:
            default:
            {
                SAL_WARN("filter", "Unknown path segment type in PDF: " << int(eSegmentType));
            }
            break;
        }
    }

    if (aBezier.size() == 3)
    {
        aPolygon.appendBezierSegment(aBezier[0], aBezier[1], aBezier[2]);
        aBezier.clear();
    }

    if (aPolygon.count() > 0)
    {
        aPolyPolygon.append(aPolygon, 1);
        aPolygon.clear();
    }

    printf("PolyPoly size %d\n", aPolyPolygon.count());
    for (auto const& rPoly : aPolyPolygon)
        printf("Poly size %d\n", rPoly.count());

    double fStrokeWidth = pPageObject->getStrokeWidth();
    printf("Stroke: %f\n", fStrokeWidth);

    vcl::pdf::PDFFillMode nFillMode = vcl::pdf::PDFFillMode::Alternate;
    bool bStroke = true;

    if (!pPageObject->getDrawMode(nFillMode, bStroke))
    {
        SAL_WARN("filter", "Huh...");
    }

    Color aFillColor = pPageObject->getFillColor();
    Color aStokeColor = COL_TRANSPARENT;

    if (bStroke)
    {
        aStokeColor = pPageObject->getStrokeColor();
    }

    if (aStokeColor == COL_TRANSPARENT)
        aStokeColor = aFillColor;

    if (!bStroke)
    {
        const drawinglayer::primitive2d::Primitive2DReference xPolyPolygonColorPrimitive(
            new drawinglayer::primitive2d::PolyPolygonColorPrimitive2D(aPolyPolygon,
                                                                       aFillColor.getBColor()));
        aContent.push_back(xPolyPolygonColorPrimitive);
    }

    drawinglayer::attribute::LineAttribute aLineAttribute(aStokeColor.getBColor(), fStrokeWidth);
    const drawinglayer::primitive2d::Primitive2DReference xPolyPolygonStrokePrimitive(
        new drawinglayer::primitive2d::PolyPolygonStrokePrimitive2D(aPolyPolygon, aLineAttribute));
    aContent.push_back(xPolyPolygonStrokePrimitive);

    const drawinglayer::primitive2d::Primitive2DReference xTransform(
        new drawinglayer::primitive2d::TransformPrimitive2D(aPathMatrix, std::move(aContent)));
    rRootContainer.push_back(xTransform);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
