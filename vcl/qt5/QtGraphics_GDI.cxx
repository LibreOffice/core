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

#include <QtGraphics.hxx>

#include <QtBitmap.hxx>
#include <QtPainter.hxx>

#include <sal/log.hxx>

#include <QtGui/QPainter>
#include <QtGui/QScreen>
#include <QtGui/QWindow>
#include <QtWidgets/QWidget>

#include <numeric>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

QtGraphicsBackend::QtGraphicsBackend(QtFrame* pFrame, QImage* pQImage)
    : m_pFrame(pFrame)
    , m_pQImage(pQImage)
    , m_aLineColor(0x00, 0x00, 0x00)
    , m_aFillColor(0xFF, 0xFF, 0XFF)
    , m_eCompositionMode(QPainter::CompositionMode_SourceOver)
{
    ResetClipRegion();
}

QtGraphicsBackend::~QtGraphicsBackend() {}

const basegfx::B2DPoint aHalfPointOfs(0.5, 0.5);

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

bool QtGraphicsBackend::setClipRegion(const vcl::Region& rRegion)
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
        for (const auto& rRect : aRectangles)
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
        AddPolyPolygonToPath(aPath, aPolyClip, !getAntiAlias(), false);
        m_aClipPath.swap(aPath);
        if (!m_aClipRegion.isEmpty())
        {
            QRegion aRegion;
            m_aClipRegion.swap(aRegion);
        }
    }
    return true;
}

void QtGraphicsBackend::ResetClipRegion()
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

void QtGraphicsBackend::drawPixel(tools::Long nX, tools::Long nY)
{
    QtPainter aPainter(*this);
    aPainter.drawPoint(nX, nY);
    aPainter.update(nX, nY, 1, 1);
}

void QtGraphicsBackend::drawPixel(tools::Long nX, tools::Long nY, Color nColor)
{
    QtPainter aPainter(*this);
    aPainter.setPen(toQColor(nColor));
    aPainter.setPen(Qt::SolidLine);
    aPainter.drawPoint(nX, nY);
    aPainter.update(nX, nY, 1, 1);
}

void QtGraphicsBackend::drawLine(tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2)
{
    QtPainter aPainter(*this);
    aPainter.drawLine(nX1, nY1, nX2, nY2);

    tools::Long tmp;
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

void QtGraphicsBackend::drawRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                 tools::Long nHeight)
{
    if (SALCOLOR_NONE == m_aFillColor && SALCOLOR_NONE == m_aLineColor)
        return;

    QtPainter aPainter(*this, true);
    if (SALCOLOR_NONE != m_aFillColor)
        aPainter.fillRect(nX, nY, nWidth, nHeight, aPainter.brush());
    if (SALCOLOR_NONE != m_aLineColor)
        aPainter.drawRect(nX, nY, nWidth - 1, nHeight - 1);
    aPainter.update(nX, nY, nWidth, nHeight);
}

void QtGraphicsBackend::drawPolyLine(sal_uInt32 nPoints, const Point* pPtAry)
{
    if (0 == nPoints)
        return;

    QtPainter aPainter(*this);
    QPoint* pPoints = new QPoint[nPoints];
    QPoint aTopLeft(pPtAry->getX(), pPtAry->getY());
    QPoint aBottomRight = aTopLeft;
    for (sal_uInt32 i = 0; i < nPoints; ++i, ++pPtAry)
    {
        pPoints[i] = QPoint(pPtAry->getX(), pPtAry->getY());
        if (pPtAry->getX() < aTopLeft.x())
            aTopLeft.setX(pPtAry->getX());
        if (pPtAry->getY() < aTopLeft.y())
            aTopLeft.setY(pPtAry->getY());
        if (pPtAry->getX() > aBottomRight.x())
            aBottomRight.setX(pPtAry->getX());
        if (pPtAry->getY() > aBottomRight.y())
            aBottomRight.setY(pPtAry->getY());
    }
    aPainter.drawPolyline(pPoints, nPoints);
    delete[] pPoints;
    aPainter.update(QRect(aTopLeft, aBottomRight));
}

void QtGraphicsBackend::drawPolygon(sal_uInt32 nPoints, const Point* pPtAry)
{
    QtPainter aPainter(*this, true);
    QPolygon aPolygon(nPoints);
    for (sal_uInt32 i = 0; i < nPoints; ++i, ++pPtAry)
        aPolygon.setPoint(i, pPtAry->getX(), pPtAry->getY());
    aPainter.drawPolygon(aPolygon);
    aPainter.update(aPolygon.boundingRect());
}

void QtGraphicsBackend::drawPolyPolygon(sal_uInt32 nPolyCount, const sal_uInt32* pPoints,
                                        const Point** ppPtAry)
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
            const Point* pPtAry = ppPtAry[nPoly];
            aPath.moveTo(pPtAry->getX(), pPtAry->getY());
            pPtAry++;
            for (sal_uInt32 nPoint = 1; nPoint < nPoints; nPoint++, pPtAry++)
                aPath.lineTo(pPtAry->getX(), pPtAry->getY());
            aPath.closeSubpath();
        }
    }

    QtPainter aPainter(*this, true);
    aPainter.drawPath(aPath);
    aPainter.update(aPath.boundingRect());
}

bool QtGraphicsBackend::drawPolyPolygon(const basegfx::B2DHomMatrix& rObjectToDevice,
                                        const basegfx::B2DPolyPolygon& rPolyPolygon,
                                        double fTransparency)
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
    if (!AddPolyPolygonToPath(aPath, aPolyPolygon, !getAntiAlias(), m_aLineColor != SALCOLOR_NONE))
        return true;

    QtPainter aPainter(*this, true, 255 * (1.0 - fTransparency));
    aPainter.drawPath(aPath);
    aPainter.update(aPath.boundingRect());
    return true;
}

bool QtGraphicsBackend::drawPolyLineBezier(sal_uInt32 /*nPoints*/, const Point* /*pPtAry*/,
                                           const PolyFlags* /*pFlgAry*/)
{
    return false;
}

bool QtGraphicsBackend::drawPolygonBezier(sal_uInt32 /*nPoints*/, const Point* /*pPtAry*/,
                                          const PolyFlags* /*pFlgAry*/)
{
    return false;
}

bool QtGraphicsBackend::drawPolyPolygonBezier(sal_uInt32 /*nPoly*/, const sal_uInt32* /*pPoints*/,
                                              const Point* const* /*pPtAry*/,
                                              const PolyFlags* const* /*pFlgAry*/)
{
    return false;
}

bool QtGraphicsBackend::drawPolyLine(const basegfx::B2DHomMatrix& rObjectToDevice,
                                     const basegfx::B2DPolygon& rPolyLine, double fTransparency,
                                     double fLineWidth,
                                     const std::vector<double>* pStroke, // MM01
                                     basegfx::B2DLineJoin eLineJoin, css::drawing::LineCap eLineCap,
                                     double fMiterMinimumAngle, bool bPixelSnapHairline)
{
    if (SALCOLOR_NONE == m_aFillColor && SALCOLOR_NONE == m_aLineColor)
    {
        return true;
    }

    // MM01 check done for simple reasons
    if (!rPolyLine.count() || fTransparency < 0.0 || fTransparency > 1.0)
    {
        return true;
    }

    // MM01 need to do line dashing as fallback stuff here now
    const double fDotDashLength(
        nullptr != pStroke ? std::accumulate(pStroke->begin(), pStroke->end(), 0.0) : 0.0);
    const bool bStrokeUsed(0.0 != fDotDashLength);
    assert(!bStrokeUsed || (bStrokeUsed && pStroke));
    basegfx::B2DPolyPolygon aPolyPolygonLine;

    if (bStrokeUsed)
    {
        // apply LineStyle
        basegfx::utils::applyLineDashing(rPolyLine, // source
                                         *pStroke, // pattern
                                         &aPolyPolygonLine, // target for lines
                                         nullptr, // target for gaps
                                         fDotDashLength); // full length if available
    }
    else
    {
        // no line dashing, just copy
        aPolyPolygonLine.append(rPolyLine);
    }

    // Transform to DeviceCoordinates, get DeviceLineWidth, execute PixelSnapHairline
    aPolyPolygonLine.transform(rObjectToDevice);
    if (bPixelSnapHairline)
    {
        aPolyPolygonLine = basegfx::utils::snapPointsOfHorizontalOrVerticalEdges(aPolyPolygonLine);
    }

    // tdf#124848 get correct LineWidth in discrete coordinates,
    if (fLineWidth == 0) // hairline
        fLineWidth = 1.0;
    else // Adjust line width for object-to-device scale.
        fLineWidth = (rObjectToDevice * basegfx::B2DVector(fLineWidth, 0)).getLength();

    // setup poly-polygon path
    QPainterPath aPath;

    // MM01 todo - I assume that this is OKAY to be done in one run for Qt,
    // but this NEEDS to be checked/verified
    for (sal_uInt32 a(0); a < aPolyPolygonLine.count(); a++)
    {
        const basegfx::B2DPolygon aPolyLine(aPolyPolygonLine.getB2DPolygon(a));
        AddPolygonToPath(aPath, aPolyLine, aPolyLine.isClosed(), !getAntiAlias(), true);
    }

    QtPainter aPainter(*this, false, 255 * (1.0 - fTransparency));

    // setup line attributes
    QPen aPen = aPainter.pen();
    aPen.setWidth(fLineWidth);

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

bool QtGraphicsBackend::drawGradient(const tools::PolyPolygon& /*rPolyPolygon*/,
                                     const Gradient& /*rGradient*/)
{
    return false;
}

bool QtGraphicsBackend::implDrawGradient(basegfx::B2DPolyPolygon const& /*rPolyPolygon*/,
                                         SalGradient const& /*rGradient*/)
{
    return false;
}

void QtGraphicsBackend::drawScaledImage(const SalTwoRect& rPosAry, const QImage& rImage)
{
    QtPainter aPainter(*this);
    QRect aSrcRect(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
    QRect aDestRect(rPosAry.mnDestX, rPosAry.mnDestY, rPosAry.mnDestWidth, rPosAry.mnDestHeight);
    aPainter.drawImage(aDestRect, rImage, aSrcRect);
    aPainter.update(aDestRect);
}

void QtGraphicsBackend::copyArea(tools::Long nDestX, tools::Long nDestY, tools::Long nSrcX,
                                 tools::Long nSrcY, tools::Long nSrcWidth, tools::Long nSrcHeight,
                                 bool /*bWindowInvalidate*/)
{
    if (nDestX == nSrcX && nDestY == nSrcY)
        return;

    SalTwoRect aTR(nSrcX, nSrcY, nSrcWidth, nSrcHeight, nDestX, nDestY, nSrcWidth, nSrcHeight);

    QImage* pImage = m_pQImage;
    QImage aImage = pImage->copy(aTR.mnSrcX, aTR.mnSrcY, aTR.mnSrcWidth, aTR.mnSrcHeight);
    pImage = &aImage;
    aTR.mnSrcX = 0;
    aTR.mnSrcY = 0;

    drawScaledImage(aTR, *pImage);
}

void QtGraphicsBackend::copyBits(const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics)
{
    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0)
        return;

    QImage aImage, *pImage;
    SalTwoRect aPosAry = rPosAry;

    if (!pSrcGraphics)
    {
        pImage = m_pQImage;
        aImage
            = pImage->copy(rPosAry.mnSrcX, rPosAry.mnSrcY, rPosAry.mnSrcWidth, rPosAry.mnSrcHeight);
        pImage = &aImage;
        aPosAry.mnSrcX = 0;
        aPosAry.mnSrcY = 0;
    }
    else
        pImage = static_cast<QtGraphics*>(pSrcGraphics)->getQImage();

    drawScaledImage(aPosAry, *pImage);
}

void QtGraphicsBackend::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSalBitmap)
{
    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0)
        return;

    const QImage* pImage = static_cast<const QtBitmap*>(&rSalBitmap)->GetQImage();

    assert(pImage);

    drawScaledImage(rPosAry, *pImage);
}

void QtGraphicsBackend::drawBitmap(const SalTwoRect& rPosAry, const SalBitmap& /*rSalBitmap*/,
                                   const SalBitmap& /*rTransparentBitmap*/)
{
    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0)
        return;

    assert(rPosAry.mnSrcWidth == rPosAry.mnDestWidth);
    assert(rPosAry.mnSrcHeight == rPosAry.mnDestHeight);
}

void QtGraphicsBackend::drawMask(const SalTwoRect& rPosAry, const SalBitmap& /*rSalBitmap*/,
                                 Color /*nMaskColor*/)
{
    if (rPosAry.mnSrcWidth <= 0 || rPosAry.mnSrcHeight <= 0 || rPosAry.mnDestWidth <= 0
        || rPosAry.mnDestHeight <= 0)
        return;

    assert(rPosAry.mnSrcWidth == rPosAry.mnDestWidth);
    assert(rPosAry.mnSrcHeight == rPosAry.mnDestHeight);
}

std::shared_ptr<SalBitmap> QtGraphicsBackend::getBitmap(tools::Long nX, tools::Long nY,
                                                        tools::Long nWidth, tools::Long nHeight)
{
    return std::make_shared<QtBitmap>(m_pQImage->copy(nX, nY, nWidth, nHeight));
}

Color QtGraphicsBackend::getPixel(tools::Long nX, tools::Long nY)
{
    return Color(ColorTransparency, m_pQImage->pixel(nX, nY));
}

void QtGraphicsBackend::invert(tools::Long nX, tools::Long nY, tools::Long nWidth,
                               tools::Long nHeight, SalInvert nFlags)
{
    QtPainter aPainter(*this);
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

void QtGraphicsBackend::invert(sal_uInt32 /*nPoints*/, const Point* /*pPtAry*/,
                               SalInvert /*nFlags*/)
{
}

bool QtGraphicsBackend::drawEPS(tools::Long /*nX*/, tools::Long /*nY*/, tools::Long /*nWidth*/,
                                tools::Long /*nHeight*/, void* /*pPtr*/, sal_uInt32 /*nSize*/)
{
    return false;
}

bool QtGraphicsBackend::blendBitmap(const SalTwoRect&, const SalBitmap& /*rBitmap*/)
{
    return false;
}

bool QtGraphicsBackend::blendAlphaBitmap(const SalTwoRect&, const SalBitmap& /*rSrcBitmap*/,
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

    const QImage* pBitmap = static_cast<const QtBitmap*>(&rSourceBitmap)->GetQImage();
    const QImage* pAlpha = static_cast<const QtBitmap*>(&rAlphaBitmap)->GetQImage();
    rAlphaImage = pBitmap->convertToFormat(Qt_DefaultFormat32);

    if (rAlphaBitmap.GetBitCount() == 8)
    {
        for (int y = 0; y < rAlphaImage.height(); ++y)
        {
            uchar* image_line = rAlphaImage.scanLine(y);
            const uchar* alpha_line = pAlpha->scanLine(y);
            for (int x = 0; x < rAlphaImage.width(); ++x, image_line += 4)
                image_line[3] = alpha_line[x];
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

bool QtGraphicsBackend::drawAlphaBitmap(const SalTwoRect& rPosAry, const SalBitmap& rSourceBitmap,
                                        const SalBitmap& rAlphaBitmap)
{
    QImage aImage;
    if (!getAlphaImage(rSourceBitmap, rAlphaBitmap, aImage))
        return false;
    drawScaledImage(rPosAry, aImage);
    return true;
}

bool QtGraphicsBackend::drawTransformedBitmap(const basegfx::B2DPoint& rNull,
                                              const basegfx::B2DPoint& rX,
                                              const basegfx::B2DPoint& rY,
                                              const SalBitmap& rSourceBitmap,
                                              const SalBitmap* pAlphaBitmap, double fAlpha)
{
    if (fAlpha != 1.0)
        return false;
    QImage aImage;
    if (pAlphaBitmap && !getAlphaImage(rSourceBitmap, *pAlphaBitmap, aImage))
        return false;
    else
    {
        const QImage* pBitmap = static_cast<const QtBitmap*>(&rSourceBitmap)->GetQImage();
        aImage = pBitmap->convertToFormat(Qt_DefaultFormat32);
    }

    QtPainter aPainter(*this);
    const basegfx::B2DVector aXRel = rX - rNull;
    const basegfx::B2DVector aYRel = rY - rNull;
    aPainter.setTransform(QTransform(aXRel.getX() / aImage.width(), aXRel.getY() / aImage.width(),
                                     aYRel.getX() / aImage.height(), aYRel.getY() / aImage.height(),
                                     rNull.getX(), rNull.getY()));
    aPainter.drawImage(QPoint(0, 0), aImage);
    aPainter.update(aImage.rect());
    return true;
}

bool QtGraphicsBackend::hasFastDrawTransformedBitmap() const { return false; }

bool QtGraphicsBackend::drawAlphaRect(tools::Long nX, tools::Long nY, tools::Long nWidth,
                                      tools::Long nHeight, sal_uInt8 nTransparency)
{
    if (SALCOLOR_NONE == m_aFillColor && SALCOLOR_NONE == m_aLineColor)
        return true;
    assert(nTransparency <= 100);
    if (nTransparency > 100)
        nTransparency = 100;
    QtPainter aPainter(*this, true, (100 - nTransparency) * (255.0 / 100));
    if (SALCOLOR_NONE != m_aFillColor)
        aPainter.fillRect(nX, nY, nWidth, nHeight, aPainter.brush());
    if (SALCOLOR_NONE != m_aLineColor)
        aPainter.drawRect(nX, nY, nWidth - 1, nHeight - 1);
    aPainter.update(nX, nY, nWidth, nHeight);
    return true;
}

sal_uInt16 QtGraphicsBackend::GetBitCount() const { return getFormatBits(m_pQImage->format()); }

tools::Long QtGraphicsBackend::GetGraphicsWidth() const { return m_pQImage->width(); }

void QtGraphicsBackend::SetLineColor() { m_aLineColor = SALCOLOR_NONE; }

void QtGraphicsBackend::SetLineColor(Color nColor) { m_aLineColor = nColor; }

void QtGraphicsBackend::SetFillColor() { m_aFillColor = SALCOLOR_NONE; }

void QtGraphicsBackend::SetFillColor(Color nColor) { m_aFillColor = nColor; }

void QtGraphicsBackend::SetXORMode(bool bSet, bool)
{
    if (bSet)
        m_eCompositionMode = QPainter::CompositionMode_Xor;
    else
        m_eCompositionMode = QPainter::CompositionMode_SourceOver;
}

void QtGraphicsBackend::SetROPLineColor(SalROPColor /*nROPColor*/) {}

void QtGraphicsBackend::SetROPFillColor(SalROPColor /*nROPColor*/) {}

bool QtGraphicsBackend::supportsOperation(OutDevSupportType eType) const
{
    switch (eType)
    {
        case OutDevSupportType::B2DDraw:
        case OutDevSupportType::TransparentRect:
            return true;
        default:
            return false;
    }
}

void QtGraphics::GetResolution(sal_Int32& rDPIX, sal_Int32& rDPIY)
{
    char* pForceDpi;
    if ((pForceDpi = getenv("SAL_FORCEDPI")))
    {
        OString sForceDPI(pForceDpi);
        rDPIX = rDPIY = sForceDPI.toInt32();
        return;
    }

    if (!m_pFrame)
        return;

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QScreen* pScreen = m_pFrame->GetQWidget()->screen();
#else
    if (!m_pFrame->GetQWidget()->window()->windowHandle())
        return;

    QScreen* pScreen = m_pFrame->GetQWidget()->window()->windowHandle()->screen();
#endif
    rDPIX = pScreen->logicalDotsPerInchX() * pScreen->devicePixelRatio() + 0.5;
    rDPIY = pScreen->logicalDotsPerInchY() * pScreen->devicePixelRatio() + 0.5;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
