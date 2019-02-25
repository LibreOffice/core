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

#include <Qt5Graphics.hxx>

#include <Qt5Bitmap.hxx>
#include <Qt5Painter.hxx>

#include <sal/log.hxx>

#include <QtGui/QPainter>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtWidgets/QWidget>

#include <basegfx/polygon/b2dpolygontools.hxx>

static const basegfx::B2DPoint aHalfPointOfs(0.5, 0.5);

static void AddPolygonToPath(QPainterPath& rPath, const basegfx::B2DPolygon& rPolygon,
                             bool bClosePath, bool bPixelSnap, bool bLineDraw)
{
    const int nPointCount = rPolygon.count();
    // short circuit if there is nothing to do
    if (nPointCount == 0)
        return;

    const bool bHasCurves = rPolygon.areControlPointsUsed();
    for (int nPointIdx = 0, nPrevIdx = 0;; nPrevIdx = nPointIdx++)
    {
        int nClosedIdx = nPointIdx;
        if (nPointIdx >= nPointCount)
        {
            // prepare to close last curve segment if needed
            if (bClosePath && (nPointIdx == nPointCount))
                nClosedIdx = 0;
            else
                break;
        }

        basegfx::B2DPoint aPoint = rPolygon.getB2DPoint(nClosedIdx);

        if (bPixelSnap)
        {
            // snap device coordinates to full pixels
            aPoint.setX(basegfx::fround(aPoint.getX()));
            aPoint.setY(basegfx::fround(aPoint.getY()));
        }

        if (bLineDraw)
            aPoint += aHalfPointOfs;
        if (!nPointIdx)
        {
            // first point => just move there
            rPath.moveTo(aPoint.getX(), aPoint.getY());
            continue;
        }

        bool bPendingCurve = false;
        if (bHasCurves)
        {
            bPendingCurve = rPolygon.isNextControlPointUsed(nPrevIdx);
            bPendingCurve |= rPolygon.isPrevControlPointUsed(nClosedIdx);
        }

        if (!bPendingCurve) // line segment
            rPath.lineTo(aPoint.getX(), aPoint.getY());
        else // cubic bezier segment
        {
            basegfx::B2DPoint aCP1 = rPolygon.getNextControlPoint(nPrevIdx);
            basegfx::B2DPoint aCP2 = rPolygon.getPrevControlPoint(nClosedIdx);
            if (bLineDraw)
            {
                aCP1 += aHalfPointOfs;
                aCP2 += aHalfPointOfs;
            }
            rPath.cubicTo(aCP1.getX(), aCP1.getY(), aCP2.getX(), aCP2.getY(), aPoint.getX(),
                          aPoint.getY());
        }
    }

    if (bClosePath)
        rPath.closeSubpath();
}

static bool AddPolyPolygonToPath(QPainterPath& rPath, const basegfx::B2DPolyPolygon& rPolyPoly,
                                 bool bPixelSnap, bool bLineDraw)
{
    if (rPolyPoly.count() == 0)
        return false;
    for (auto const& rPolygon : rPolyPoly)
    {
        AddPolygonToPath(rPath, rPolygon, true, bPixelSnap, bLineDraw);
    }
    return true;
}

bool Qt5Graphics::setClipRegion(const vcl::Region& rRegion)
{
    if (rRegion.IsRectangle())
    {
        m_aClipRegion = toQRect(rRegion.GetBoundRect());
        if (!m_aClipPath.isEmpty())
        {
            QPainterPath aPath;
            m_aClipPath.swap(aPath);
        }
    }
    else if (!rRegion.HasPolyPolygonOrB2DPolyPolygon())
    {
        QRegion aQRegion;
        RectangleVector aRectangles;
        rRegion.GetRegionRectangles(aRectangles);
        for (auto& rRect : aRectangles)
            aQRegion += toQRect(rRect);
        m_aClipRegion = aQRegion;
        if (!m_aClipPath.isEmpty())
        {
            QPainterPath aPath;
            m_aClipPath.swap(aPath);
        }
    }
    else
    {
        QPainterPath aPath;
        const basegfx::B2DPolyPolygon aPolyClip(rRegion.GetAsB2DPolyPolygon());
        AddPolyPolygonToPath(aPath, aPolyClip, !getAntiAliasB2DDraw(), false);
        m_aClipPath.swap(aPath);
        if (!m_aClipRegion.isEmpty())
        {
            QRegion aRegion;
            m_aClipRegion.swap(aRegion);
        }
    }
    return true;
}

void Qt5Graphics::ResetClipRegion()
{
    if (m_pQImage)
        m_aClipRegion = QRegion(m_pQImage->rect());
    else
        m_aClipRegion = QRegion();
    if (!m_aClipPath.isEmpty())
    {
        QPainterPath aPath;
        m_aClipPath.swap(aPath);
    }
}

void Qt5Graphics::drawPixel(long nX, long nY)
{
    Qt5Painter aPainter(*this);
    aPainter.drawPoint(nX, nY);
    aPainter.update(nX, nY, 1, 1);
}

void Qt5Graphics::drawPixel(long nX, long nY, Color nColor)
{
    Qt5Painter aPainter(*this);
    aPainter.setPen(toQColor(nColor));
    aPainter.setPen(Qt::SolidLine);
    aPainter.drawPoint(nX, nY);
    aPainter.update(nX, nY, 1, 1);
}

void Qt5Graphics::drawLine(long nX1, long nY1, long nX2, long nY2)
{
    Qt5Painter aPainter(*this);
    aPainter.drawLine(nX1, nY1, nX2, nY2);

    long tmp;
    if (nX1 > nX2)
    {
        tmp = nX1;
        nX1 = nX2;
        nX2 = tmp;
    }
    if (nY1 > nY2)
    {
        tmp = nY1;
        nY1 = nY2;
        nY2 = tmp;
    }
    aPainter.update(nX1, nY1, nX2 - nX1 + 1, nY2 - nY1 + 1);
}

void Qt5Graphics::drawRect(long nX, long nY, long nWidth, long nHeight)
{
    if (SALCOLOR_NONE == m_aFillColor && SALCOLOR_NONE == m_aLineColor)
        return;

    Qt5Painter aPainter(*this, true);
    if (SALCOLOR_NONE != m_aFillColor)
        aPainter.fillRect(nX, nY, nWidth, nHeight, aPainter.brush());
    if (SALCOLOR_NONE != m_aLineColor)
        aPainter.drawRect(nX, nY, nWidth, nHeight);
    aPainter.update(nX, nY, nWidth, nHeight);
}

void Qt5Graphics::drawPolyLine(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    if (0 == nPoints)
        return;

    Qt5Painter aPainter(*this);
    QPoint* pPoints = new QPoint[nPoints];
    QPoint aTopLeft(pPtAry->mnX, pPtAry->mnY);
    QPoint aBottomRight = aTopLeft;
    for (sal_uInt32 i = 0; i < nPoints; ++i, ++pPtAry)
    {
        pPoints[i] = QPoint(pPtAry->mnX, pPtAry->mnY);
        if (pPtAry->mnX < aTopLeft.x())
            aTopLeft.setX(pPtAry->mnX);
        if (pPtAry->mnY < aTopLeft.y())
            aTopLeft.setY(pPtAry->mnY);
        if (pPtAry->mnX > aBottomRight.x())
            aBottomRight.setX(pPtAry->mnX);
        if (pPtAry->mnY > aBottomRight.y())
            aBottomRight.setY(pPtAry->mnY);
    }
    aPainter.drawPolyline(pPoints, nPoints);
    delete[] pPoints;
    aPainter.update(QRect(aTopLeft, aBottomRight));
}

void Qt5Graphics::drawPolygon(sal_uInt32 nPoints, const SalPoint* pPtAry)
{
    Qt5Painter aPainter(*this, true);
    QPolygon aPolygon(nPoints);
    for (sal_uInt32 i = 0; i < nPoints; ++i, ++pPtAry)
        aPolygon.setPoint(i, pPtAry->mnX, pPtAry->mnY);
    aPainter.drawPolygon(aPolygon);
    aPainter.update(aPolygon.boundingRect());
}

void Qt5Graphics::drawPolyPolygon(sal_uInt32 nPolyCount, const sal_uInt32* pPoints,
                                  PCONSTSALPOINT* ppPtAry)
{
    // ignore invisible polygons
    if (SALCOLOR_NONE == m_aFillColor && SALCOLOR_NONE == m_aLineColor)
        return;

    QPainterPath aPath;
    for (sal_uInt32 nPoly = 0; nPoly < nPolyCount; nPoly++)
    {
        const sal_uInt32 nPoints = pPoints[nPoly];
        if (nPoints > 1)
        {
            const SalPoint* pPtAry = ppPtAry[nPoly];
            aPath.moveTo(pPtAry->mnX, pPtAry->mnY);
            pPtAry++;
            for (sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++)
                aPath.lineTo(pPtAry->mnX, pPtAry->mnY);
            aPath.closeSubpath();
        }
    }

    Qt5Painter aPainter(*this, true);
    aPainter.drawPath(aPath);
    aPainter.update(aPath.boundingRect());
}

bool Qt5Graphics::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                  const basegfx::B2DPolyPolygon& rPolyPolygon, double fTransparency)
{
    // ignore invisible polygons
    if (SALCOLOR_NONE == m_aFillColor && SALCOLOR_NONE == m_aLineColor)
        return true;
    if ((fTransparency >= 1.0) || (fTransparency < 0))
        return true;

    // Fallback: Transform to DeviceCoordinates
    basegfx::B2DPolyPolygon aPolyPolygon(rPolyPolygon);
    aPolyPolygon.transform(rObjectToDevice);

    QPainterPath aPath;
    // ignore empty polygons
    if (!AddPolyPolygonToPath(aPath, aPolyPolygon, !getAntiAliasB2DDraw(),
                              m_aLineColor != SALCOLOR_NONE))
        return true;

    Qt5Painter aPainter(*this, true, 255 * (1.0 - fTransparency));
    aPainter.drawPath(aPath);
    aPainter.update(aPath.boundingRect());
    return true;
}

bool Qt5Graphics::drawPolyLineBezier(sal_uInt32 /*nPoints*/, const SalPoint* /*pPtAry*/,
                                     const PolyFlags* /*pFlgAry*/)
{
    return false;
}

bool Qt5Graphics::drawPolygonBezier(sal_uInt32 /*nPoints*/, const SalPoint* /*pPtAry*/,
                                    const PolyFlags* /*pFlgAry*/)
{
    return false;
}

bool Qt5Graphics::drawPolyPolygonBezier(sal_uInt32 /*nPoly*/, const sal_uInt32* /*pPoints*/,
                                        const SalPoint* const* /*pPtAry*/,
                                        const PolyFlags* const* /*pFlgAry*/)
{
    return false;
}

bool Qt5Graphics::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                               const basegfx::B2DPolygon& rPolyLine, double fTransparency,
                               const basegfx::B2DVector& rLineWidths,
                               basegfx::B2DLineJoin eLineJoin, css::drawing::LineCap eLineCap,
                               double fMiterMinimumAngle, bool bPixelSnapHairline)
{
    if (SALCOLOR_NONE == m_aFillColor && SALCOLOR_NONE == m_aLineColor)
        return true;

    // short circuit if there is nothing to do
    if (0 == rPolyLine.count())
    {
        return true;
    }

    // Transform to DeviceCoordinates, get DeviceLineWidth, execute PixelSnapHairline
    basegfx::B2DPolygon aPolyLine(rPolyLine);
    aPolyLine.transform(rObjectToDevice);
    if (bPixelSnapHairline)
    {
        aPolyLine = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges(aPolyLine);
    }
    const basegfx::B2DVector aLineWidths(rObjectToDevice * rLineWidths);

    // setup poly-polygon path
    QPainterPath aPath;
    AddPolygonToPath(aPath, aPolyLine, aPolyLine.isClosed(), !getAntiAliasB2DDraw(), true);

    Qt5Painter aPainter(*this, false, 255 * (1.0 - fTransparency));

    // setup line attributes
    QPen aPen = aPainter.pen();
    aPen.setWidth(aLineWidths.getX());

    switch (eLineJoin)
    {
        case basegfx::B2DLineJoin::Bevel:
            aPen.setJoinStyle(Qt::BevelJoin);
            break;
        case basegfx::B2DLineJoin::Round:
            aPen.setJoinStyle(Qt::RoundJoin);
            break;
        case basegfx::B2DLineJoin::NONE:
        case basegfx::B2DLineJoin::Miter:
            aPen.setMiterLimit(1.0 / sin(fMiterMinimumAngle / 2.0));
            aPen.setJoinStyle(Qt::MiterJoin);
            break;
    }

    switch (eLineCap)
    {
        default: // css::drawing::LineCap_BUTT:
            aPen.setCapStyle(Qt::FlatCap);
            break;
        case css::drawing::LineCap_ROUND:
            aPen.setCapStyle(Qt::RoundCap);
            break;
        case css::drawing::LineCap_SQUARE:
            aPen.setCapStyle(Qt::SquareCap);
            break;
    }

    aPainter.setPen(aPen);
    aPainter.drawPath(aPath);
    aPainter.update(aPath.boundingRect());
    return true;
}

bool Qt5Graphics::drawGradient(const tools::PolyPolygon&, const Gradient&) { return false; }

void Qt5Graphics::drawScaledImage(const SalTwoRect& rPosAry, const QImage& rImage)
{
    Qt5Painter aPainter(*this);
    QRect aSrcRect(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
    QRect aDestRect(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    aPainter.drawImage(aDestRect, rImage, aSrcRect);
    aPainter.update(aDestRect);
}

void Qt5Graphics::copyArea(long nDestX, long nDestY, long nSrcX, long nSrcY, long nSrcWidth,
                           long nSrcHeight, bool /*bWindowInvalidate*/)
{
    if (nDestX == nSrcX && nDestY == nSrcY)
        return;

    SalTwoRect aTR(nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight);
    copyBits(aTR, this);
}

void Qt5Graphics::copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics)
{
    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0)
        return;

    QImage aImage, *pImage;
    SalTwoRect aPosAry = rPosAry;
    if (!pSrcGraphics || this == pSrcGraphics)
    {
        pImage = m_pQImage;
        aImage
            = pImage->copy(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
        pImage = &aImage;
        aPosAry.mnSrcX = 0;
        aPosAry.mnSrcY = 0;
    }
    else
        pImage = static_cast<Qt5Graphics*>(pSrcGraphics)->m_pQImage;

    drawScaledImage(aPosAry, *pImage);
}

void Qt5Graphics::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0)
        return;

    Qt5Bitmap aRGBABitmap;
    if (rSalBitmap.GetBitCount() == 4)
        aRGBABitmap.Create(rSalBitmap, 32);
    const QImage* pImage = (rSalBitmap.GetBitCount() != 4)
                               ? static_cast<const Qt5Bitmap*>(&rSalBitmap)->GetQImage()
                               : aRGBABitmap.GetQImage();
    assert(pImage);

    drawScaledImage(rPosAry, *pImage);
}

void Qt5Graphics::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& /*rSalBitmap*/,
                             const SalBitmap& /*rTransparentBitmap*/)
{
    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0)
        return;

    assert(rPosAry.mnSrcWidth == rPosAry.mnDestWidth);
    assert(rPosAry.mnSrcHeight == rPosAry.mnDestHeight);
}

void Qt5Graphics::drawMask(const SalTwoRect& rPosAry, const SalBitmap& /*rSalBitmap*/,
                           Color /*nMaskColor*/)
{
    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0)
        return;

    assert(rPosAry.mnSrcWidth == rPosAry.mnDestWidth);
    assert(rPosAry.mnSrcHeight == rPosAry.mnDestHeight);
}

std::shared_ptr<SalBitmap> Qt5Graphics::getBitmap(long nX, long nY, long nWidth, long nHeight)
{
    return std::make_shared<Qt5Bitmap>(m_pQImage->copy(nX, nY, nWidth, nHeight));
}

Color Qt5Graphics::getPixel(long nX, long nY) { return m_pQImage->pixel(nX, nY); }

void Qt5Graphics::invert(long nX, long nY, long nWidth, long nHeight, SalInvert nFlags)
{
    Qt5Painter aPainter(*this);
    if (SalInvert::N50 & nFlags)
    {
        aPainter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
        QBrush aBrush(Qt::white, Qt::Dense4Pattern);
        aPainter.fillRect(nX, nY, nWidth, nHeight, aBrush);
    }
    else
    {
        if (SalInvert::TrackFrame & nFlags)
        {
            aPainter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
            QPen aPen(Qt::white);
            aPen.setStyle(Qt::DotLine);
            aPainter.setPen(aPen);
            aPainter.drawRect(nX, nY, nWidth, nHeight);
        }
        else
        {
            aPainter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
            aPainter.fillRect(nX, nY, nWidth, nHeight, Qt::white);
        }
    }
    aPainter.update(nX, nY, nWidth, nHeight);
}

void Qt5Graphics::invert(sal_uInt32 /*nPoints*/, const SalPoint* /*pPtAry*/, SalInvert /*nFlags*/)
{
}

bool Qt5Graphics::drawEPS(long /*nX*/, long /*nY*/, long /*nWidth*/, long /*nHeight*/,
                          void* /*pPtr*/, sal_uInt32 /*nSize*/)
{
    return false;
}

bool Qt5Graphics::blendBitmap(const SalTwoRect&, const SalBitmap& /*rBitmap*/) { return false; }

bool Qt5Graphics::blendAlphaBitmap(const SalTwoRect&, const SalBitmap& /*rSrcBitmap*/,
                                   const SalBitmap& /*rMaskBitmap*/,
                                   const SalBitmap& /*rAlphaBitmap*/)
{
    return false;
}

static bool getAlphaImage(const SalBitmap& rSourceBitmap, const SalBitmap& rAlphaBitmap,
                          QImage& rAlphaImage)
{
    if (rAlphaBitmap.GetBitCount() != 8 && rAlphaBitmap.GetBitCount() != 1)
    {
        SAL_WARN("vcl.gdi", "unsupported alpha depth case: " << rAlphaBitmap.GetBitCount());
        return false;
    }

    Qt5Bitmap aRGBABitmap;
    if (rSourceBitmap.GetBitCount() == 4)
        aRGBABitmap.Create(rSourceBitmap, 32);
    const QImage* pBitmap = (rSourceBitmap.GetBitCount() != 4)
                                ? static_cast<const Qt5Bitmap*>(&rSourceBitmap)->GetQImage()
                                : aRGBABitmap.GetQImage();
    const QImage* pAlpha = static_cast<const Qt5Bitmap*>(&rAlphaBitmap)->GetQImage();
    rAlphaImage = pBitmap->convertToFormat(Qt5_DefaultFormat32);

    if (rAlphaBitmap.GetBitCount() == 8)
    {
        for (int y = 0; y < rAlphaImage.height(); ++y)
        {
            uchar* image_line = rAlphaImage.scanLine(y);
            const uchar* alpha_line = pAlpha->scanLine(y);
            for (int x = 0; x < rAlphaImage.width(); ++x, image_line += 4)
                image_line[3] = 255 - alpha_line[x];
        }
    }
    else
    {
        for (int y = 0; y < rAlphaImage.height(); ++y)
        {
            uchar* image_line = rAlphaImage.scanLine(y);
            const uchar* alpha_line = pAlpha->scanLine(y);
            for (int x = 0; x < rAlphaImage.width(); ++x, image_line += 4)
            {
                if (x && !(x % 8))
                    ++alpha_line;
                if (0 != (*alpha_line & (1 << (7 - x % 8))))
                    image_line[3] = 0;
            }
        }
    }

    return true;
}

bool Qt5Graphics::drawAlphaBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap,
                                  const SalBitmap& rAlphaBitmap)
{
    QImage aImage;
    if (!getAlphaImage(rSourceBitmap, rAlphaBitmap, aImage))
        return false;
    drawScaledImage(rPosAry, aImage);
    return true;
}

bool Qt5Graphics::drawTransformedBitmap(const basegfx::B2DPoint& rNull, const basegfx::B2DPoint& rX,
                                        const basegfx::B2DPoint& rY, const SalBitmap& rSourceBitmap,
                                        const SalBitmap* pAlphaBitmap)
{
    QImage aImage;
    if (pAlphaBitmap && !getAlphaImage(rSourceBitmap, *pAlphaBitmap, aImage))
        return false;
    else
    {
        Qt5Bitmap aRGBABitmap;
        if (rSourceBitmap.GetBitCount() == 4)
            aRGBABitmap.Create(rSourceBitmap, 32);
        const QImage* pBitmap = (rSourceBitmap.GetBitCount() != 4)
                                    ? static_cast<const Qt5Bitmap*>(&rSourceBitmap)->GetQImage()
                                    : aRGBABitmap.GetQImage();
        aImage = pBitmap->convertToFormat(Qt5_DefaultFormat32);
    }

    Qt5Painter aPainter(*this);
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    aPainter.setTransform(QTransform(aXRel.getX() / aImage.width(), aXRel.getY() / aImage.width(),
                                     aYRel.getX() / aImage.height(), aYRel.getY() / aImage.height(),
                                     rNull.getX(), rNull.getY()));
    aPainter.drawImage(QPoint(0, 0), aImage);
    aPainter.update(aImage.rect());
    return true;
}

bool Qt5Graphics::drawAlphaRect(long nX, long nY, long nWidth, long nHeight,
                                sal_uInt8 nTransparency)
{
    if (SALCOLOR_NONE == m_aFillColor && SALCOLOR_NONE == m_aLineColor)
        return true;
    assert(nTransparency <= 100);
    if (nTransparency > 100)
        nTransparency = 100;
    Qt5Painter aPainter(*this, true, (100 - nTransparency) * (255.0 / 100));
    if (SALCOLOR_NONE != m_aFillColor)
        aPainter.fillRect(nX, nY, nWidth, nHeight, aPainter.brush());
    if (SALCOLOR_NONE != m_aLineColor)
        aPainter.drawRect(nX, nY, nWidth, nHeight);
    aPainter.update(nX, nY, nWidth, nHeight);
    return true;
}

void Qt5Graphics::GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY)
{
    char* pForceDpi;
    if ((pForceDpi = getenv("SAL_FORCEDPI")))
    {
        OString sForceDPI(pForceDpi);
        rDPIX = rDPIY = sForceDPI.toInt32();
        return;
    }

    if (!m_pFrame || !m_pFrame->GetQWidget()->window()->windowHandle())
        return;

    QScreen* pScreen = m_pFrame->GetQWidget()->window()->windowHandle()->screen();
    rDPIX = pScreen->logicalDotsPerInchX();
    rDPIY = pScreen->logicalDotsPerInchY();
}

sal_uInt16 Qt5Graphics::GetBitCount() const { return getFormatBits(m_pQImage->format()); }

long Qt5Graphics::GetGraphicsWidth() const { return m_pQImage->width(); }

void Qt5Graphics::SetLineColor() { m_aLineColor = SALCOLOR_NONE; }

void Qt5Graphics::SetLineColor(Color nColor) { m_aLineColor = nColor; }

void Qt5Graphics::SetFillColor() { m_aFillColor = SALCOLOR_NONE; }

void Qt5Graphics::SetFillColor(Color nColor) { m_aFillColor = nColor; }

void Qt5Graphics::SetXORMode(bool bSet, bool)
{
    if (bSet)
        m_eCompositionMode = QPainter::CompositionMode_Xor;
    else
        m_eCompositionMode = QPainter::CompositionMode_SourceOver;
}

void Qt5Graphics::SetROPLineColor(SalROPColor /*nROPColor*/) {}

void Qt5Graphics::SetROPFillColor(SalROPColor /*nROPColor*/) {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
