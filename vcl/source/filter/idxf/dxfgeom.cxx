/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dxfgeom.hxx"

#include <basegfx/polygon/b2dpolygontools.hxx>

#include <algorithm>
#include <cmath>

namespace
{
basegfx::B2DPoint ellipsePoint(const basegfx::B2DPoint& rCenter, const basegfx::B2DVector& rU,
                               const basegfx::B2DVector& rV, double fT)
{
    return rCenter + rU * std::cos(fT) + rV * std::sin(fT);
}

basegfx::B2DVector ellipseDerivative(const basegfx::B2DVector& rU, const basegfx::B2DVector& rV,
                                     double fT)
{
    return rV * std::cos(fT) - rU * std::sin(fT);
}

// The segment of a polyline starting at vertex nIndex
basegfx::B2DPolygon createSegment(const std::vector<DXFPathVertex>& rVertices, size_t nIndex)
{
    basegfx::B2DPolygon aSegment;
    aSegment.append(rVertices[nIndex].maPoint);
    dxf::appendBulge(aSegment, rVertices[(nIndex + 1) % rVertices.size()].maPoint,
                     rVertices[nIndex].mfBulge);
    return aSegment;
}

// The outline of a segment whose width changes linearly from fStartWidth to fEndWidth
basegfx::B2DPolygon createWideSegment(const basegfx::B2DPolygon& rSegment, double fStartWidth,
                                      double fEndWidth)
{
    const basegfx::B2DPolygon aFlat = rSegment.areControlPointsUsed()
                                          ? basegfx::utils::adaptiveSubdivideByAngle(rSegment)
                                          : rSegment;
    const sal_uInt32 nCount = aFlat.count();
    basegfx::B2DPolygon aOutline;
    if (nCount < 2)
        return aOutline;

    std::vector<double> aLengths(nCount, 0.0);
    for (sal_uInt32 i = 1; i < nCount; ++i)
        aLengths[i]
            = aLengths[i - 1]
              + basegfx::B2DVector(aFlat.getB2DPoint(i) - aFlat.getB2DPoint(i - 1)).getLength();
    const double fLength = aLengths[nCount - 1];
    if (fLength <= 0)
        return aOutline;

    std::vector<basegfx::B2DPoint> aLeft;
    std::vector<basegfx::B2DPoint> aRight;
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        // the normal at a point is the average of the normals of the adjacent edges
        basegfx::B2DVector aDirection;
        if (i > 0)
            aDirection
                += basegfx::B2DVector(aFlat.getB2DPoint(i) - aFlat.getB2DPoint(i - 1)).normalize();
        if (i + 1 < nCount)
            aDirection
                += basegfx::B2DVector(aFlat.getB2DPoint(i + 1) - aFlat.getB2DPoint(i)).normalize();
        aDirection.normalize();
        const basegfx::B2DVector aNormal(-aDirection.getY(), aDirection.getX());
        const double fWidth = fStartWidth + (fEndWidth - fStartWidth) * aLengths[i] / fLength;
        aLeft.push_back(aFlat.getB2DPoint(i) + aNormal * (fWidth / 2));
        aRight.push_back(aFlat.getB2DPoint(i) - aNormal * (fWidth / 2));
    }
    for (const basegfx::B2DPoint& rPoint : aLeft)
        aOutline.append(rPoint);
    for (std::vector<basegfx::B2DPoint>::const_reverse_iterator it = aRight.rbegin();
         it != aRight.rend(); ++it)
        aOutline.append(*it);
    aOutline.setClosed(true);
    return aOutline;
}
}

namespace dxf
{
void appendEllipseArc(basegfx::B2DPolygon& rPolygon, const basegfx::B2DPoint& rCenter,
                      const basegfx::B2DVector& rU, const basegfx::B2DVector& rV, double fStart,
                      double fSweep, bool bWithStart)
{
    if (!std::isfinite(fStart) || !std::isfinite(fSweep))
        return;
    if (bWithStart)
        rPolygon.append(ellipsePoint(rCenter, rU, rV, fStart));
    if (fSweep == 0)
        return;

    // at most a quarter of the ellipse per curve segment
    const double fSegments = std::ceil(std::fabs(fSweep) / M_PI_2 - 1e-9);
    const sal_uInt32 nSegments = static_cast<sal_uInt32>(std::clamp(fSegments, 1.0, 16.0));
    const double fDelta = fSweep / nSegments;
    const double fK = 4.0 / 3.0 * std::tan(fDelta / 4.0);
    for (sal_uInt32 i = 0; i < nSegments; ++i)
    {
        const double fT0 = fStart + i * fDelta;
        const double fT1 = fT0 + fDelta;
        const basegfx::B2DPoint aStart = ellipsePoint(rCenter, rU, rV, fT0);
        const basegfx::B2DPoint aEnd = ellipsePoint(rCenter, rU, rV, fT1);
        rPolygon.appendBezierSegment(aStart + ellipseDerivative(rU, rV, fT0) * fK,
                                     aEnd - ellipseDerivative(rU, rV, fT1) * fK, aEnd);
    }
}

void appendBulge(basegfx::B2DPolygon& rPolygon, const basegfx::B2DPoint& rEnd, double fBulge)
{
    const sal_uInt32 nCount = rPolygon.count();
    if (nCount == 0 || !std::isfinite(fBulge) || std::fabs(fBulge) < 1e-9
        || std::fabs(fBulge) > 1e6)
    {
        rPolygon.append(rEnd);
        return;
    }

    const basegfx::B2DPoint aStart = rPolygon.getB2DPoint(nCount - 1);
    const basegfx::B2DVector aChord(rEnd - aStart);
    const double fChord = aChord.getLength();
    if (fChord == 0)
    {
        rPolygon.append(rEnd);
        return;
    }

    // the centre is on the perpendicular bisector of the chord
    const basegfx::B2DVector aNormal(-aChord.getY() / fChord, aChord.getX() / fChord);
    const basegfx::B2DPoint aCenter
        = aStart + aChord * 0.5 + aNormal * (fChord * (1 - fBulge * fBulge) / (4 * fBulge));
    const double fRadius = fChord * (1 + fBulge * fBulge) / (4 * std::fabs(fBulge));
    const double fStartAngle
        = std::atan2(aStart.getY() - aCenter.getY(), aStart.getX() - aCenter.getX());
    appendEllipseArc(rPolygon, aCenter, basegfx::B2DVector(fRadius, 0),
                     basegfx::B2DVector(0, fRadius), fStartAngle, 4 * std::atan(fBulge), false);
    // end exactly on the next vertex
    rPolygon.setB2DPoint(rPolygon.count() - 1, rEnd);
}

basegfx::B2DPolygon createPolyline(const std::vector<DXFPathVertex>& rVertices, bool bClosed)
{
    basegfx::B2DPolygon aPolygon;
    const size_t nCount = rVertices.size();
    if (nCount == 0)
        return aPolygon;

    aPolygon.append(rVertices[0].maPoint);
    for (size_t i = 1; i < nCount; ++i)
        appendBulge(aPolygon, rVertices[i].maPoint, rVertices[i - 1].mfBulge);
    if (bClosed && nCount > 1)
    {
        appendBulge(aPolygon, rVertices[0].maPoint, rVertices[nCount - 1].mfBulge);
        // the closing segment ends on the first point, merge the two
        const sal_uInt32 nLast = aPolygon.count() - 1;
        if (aPolygon.areControlPointsUsed() && aPolygon.isPrevControlPointUsed(nLast))
            aPolygon.setPrevControlPoint(0, aPolygon.getPrevControlPoint(nLast));
        aPolygon.remove(nLast);
        aPolygon.setClosed(true);
    }
    return aPolygon;
}

basegfx::B2DPolyPolygon createWidePolyline(const std::vector<DXFPathVertex>& rVertices,
                                           bool bClosed)
{
    basegfx::B2DPolyPolygon aResult;
    const size_t nCount = rVertices.size();
    if (nCount < 2)
        return aResult;

    const size_t nSegments = bClosed ? nCount : nCount - 1;
    for (size_t i = 0; i < nSegments; ++i)
    {
        const DXFPathVertex& rVertex = rVertices[i];
        if (!(rVertex.mfStartWidth > 0) && !(rVertex.mfEndWidth > 0))
            continue;
        basegfx::B2DPolygon aOutline
            = createWideSegment(createSegment(rVertices, i), std::max(rVertex.mfStartWidth, 0.0),
                                std::max(rVertex.mfEndWidth, 0.0));
        if (aOutline.count() > 0)
            aResult.append(aOutline);
    }
    return aResult;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
