/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <comphelper/configuration.hxx>
#include <sal/log.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/virdev.hxx>
#include <tools/poly.hxx>
#include "dxf2mtf.hxx"

#include <algorithm>
#include <cmath>

namespace
{
// The longest polygon handed to the metafile, tools::Polygon has a 16-bit point count
// and a curve segment takes three points there.
constexpr sal_uInt32 MAX_POLYGON_POINTS = 16000;

// The larger side of the drawing in metafile units
constexpr double DEVICE_SIZE = 10000.0;

// Space around the drawing in metafile units, so that lines on the border are not clipped
constexpr double DEVICE_MARGIN = 50.0;

// The widest line weight of the DXF format, in 1/100 mm like the metafile
constexpr tools::Long MAX_LINE_WEIGHT = 211;

// The shortest line type pattern drawn in metafile units, a denser one looks solid anyway
constexpr double MIN_PATTERN_PERIOD = 10.0;

basegfx::B2DPoint Project(const DXFTransform& rTransform, const DXFVector& rVector)
{
    DXFVector aResult;
    rTransform.Transform(rVector, aResult);
    return basegfx::B2DPoint(aResult.fx, aResult.fy);
}

// Projects a polygon lying in the z = fZ plane of the entity coordinate system.
// Curve control points are transformed like the points: the projection is affine.
basegfx::B2DPolygon Project(const DXFTransform& rTransform, const basegfx::B2DPolygon& rPolygon,
                            double fZ)
{
    basegfx::B2DPolygon aResult;
    const sal_uInt32 nCount = rPolygon.count();
    for (sal_uInt32 i = 0; i < nCount; ++i)
    {
        const basegfx::B2DPoint aPoint = rPolygon.getB2DPoint(i);
        aResult.append(Project(rTransform, DXFVector(aPoint.getX(), aPoint.getY(), fZ)));
    }
    if (rPolygon.areControlPointsUsed())
    {
        for (sal_uInt32 i = 0; i < nCount; ++i)
        {
            if (rPolygon.isPrevControlPointUsed(i))
            {
                const basegfx::B2DPoint aPoint = rPolygon.getPrevControlPoint(i);
                aResult.setPrevControlPoint(
                    i, Project(rTransform, DXFVector(aPoint.getX(), aPoint.getY(), fZ)));
            }
            if (rPolygon.isNextControlPointUsed(i))
            {
                const basegfx::B2DPoint aPoint = rPolygon.getNextControlPoint(i);
                aResult.setNextControlPoint(
                    i, Project(rTransform, DXFVector(aPoint.getX(), aPoint.getY(), fZ)));
            }
        }
    }
    aResult.setClosed(rPolygon.isClosed());
    return aResult;
}

bool IsFinite(const basegfx::B2DRange& rRange)
{
    return std::isfinite(rRange.getMinX()) && std::isfinite(rRange.getMinY())
           && std::isfinite(rRange.getMaxX()) && std::isfinite(rRange.getMaxY());
}
}

void DXF2GDIMetaFile::ResetState()
{
    moBlockColor = ConvertColor(7);
    aBlockDXFLineInfo = DXFLineInfo();
    mnBlockLineWeight = DXF_LINEWEIGHT_DEFAULT;
    mpParentLayer = nullptr;

    aActLineColor = pVirDev->GetLineColor();
    aActFillColor = pVirDev->GetFillColor();
    aActFont = pVirDev->GetFont();
}

Color DXF2GDIMetaFile::ConvertColor(tools::Long nColor) const
{
    if (nColor < 0)
        nColor = -nColor;
    if (nColor < 1 || nColor > 255)
        nColor = 7;
    const sal_uInt8 nIndex = static_cast<sal_uInt8>(nColor);
    return Color(
        pDXF->aPalette.GetRed( nIndex ),
        pDXF->aPalette.GetGreen( nIndex ),
        pDXF->aPalette.GetBlue( nIndex ) );
}

const DXFLayer * DXF2GDIMetaFile::GetEntityLayer(const DXFBasicEntity & rE) const
{
    // the entities on layer 0 of a block are on the layer of the insert
    if (rE.m_sLayer == "0" && mpParentLayer != nullptr)
        return mpParentLayer;
    return pDXF->aTables.SearchLayer(rE.m_sLayer);
}

std::optional<Color> DXF2GDIMetaFile::GetEntityColor(const DXFBasicEntity & rE) const
{
    if (rE.bInvisible)
        return {};
    const DXFLayer * pLayer = GetEntityLayer(rE);
    if (pLayer != nullptr && pLayer->IsHidden())
        return {};

    if (rE.nTrueColor >= 0)
        return Color(ColorTransparency, rE.nTrueColor);
    if (rE.nColor == 0) // BYBLOCK
        return moBlockColor;
    if (rE.nColor == 256) // BYLAYER
    {
        if (pLayer == nullptr)
            return ConvertColor(7);
        if (pLayer->nTrueColor >= 0)
            return Color(ColorTransparency, pLayer->nTrueColor);
        return ConvertColor(pLayer->nColor);
    }
    return ConvertColor(rE.nColor);
}

DXFLineInfo DXF2GDIMetaFile::LTypeToDXFLineInfo(std::string_view rLineType) const
{
    const DXFLType * pLT;
    DXFLineInfo aDXFLineInfo;

    pLT = pDXF->aTables.SearchLType(rLineType);
    if (pLT==nullptr || pLT->nDashCount == 0) {
        aDXFLineInfo.eStyle = LineStyle::Solid;
    }
    else {
        aDXFLineInfo.eStyle = LineStyle::Dash;
        for (tools::Long i=0; i < (pLT->nDashCount); i++) {
            const double x = pLT->fDash[i] * pDXF->getGlobalLineTypeScale();
            if ( x >= 0.0 ) {
                if ( aDXFLineInfo.nDotCount == 0 ) {
                    aDXFLineInfo.nDotCount ++;
                    aDXFLineInfo.fDotLen = x;
                }
                else if ( aDXFLineInfo.fDotLen == x ) {
                    aDXFLineInfo.nDotCount ++;
                }
                else if ( aDXFLineInfo.nDashCount == 0 ) {
                    aDXFLineInfo.nDashCount ++;
                    aDXFLineInfo.fDashLen = x;
                }
                else if ( aDXFLineInfo.fDashLen == x ) {
                    aDXFLineInfo.nDashCount ++;
                }
                else {
                    // It is impossible to be converted.
                }
            }
            else {
                if ( aDXFLineInfo.fDistance == 0 ) {
                    aDXFLineInfo.fDistance = -1 * x;
                }
                else {
                    // It is impossible to be converted.
                }
            }

        }
    }

    return aDXFLineInfo;
}

DXFLineInfo DXF2GDIMetaFile::GetEntityDXFLineInfo(const DXFBasicEntity & rE)
{
    DXFLineInfo aDXFLineInfo;

    if (rE.m_sLineType == "BYLAYER") {
        const DXFLayer * pLayer = GetEntityLayer(rE);
        if (pLayer != nullptr)
            aDXFLineInfo = LTypeToDXFLineInfo(pLayer->m_sLineType);
    }
    else if (rE.m_sLineType == "BYBLOCK") {
        aDXFLineInfo=aBlockDXFLineInfo;
    }
    else {
        aDXFLineInfo = LTypeToDXFLineInfo(rE.m_sLineType);
    }
    return aDXFLineInfo;
}

tools::Long DXF2GDIMetaFile::GetEntityLineWeight(const DXFBasicEntity & rE) const
{
    tools::Long nLineWeight = rE.nLineWeight;
    if (nLineWeight == DXF_LINEWEIGHT_BYLAYER)
    {
        const DXFLayer * pLayer = GetEntityLayer(rE);
        nLineWeight = pLayer != nullptr ? pLayer->nLineWeight : DXF_LINEWEIGHT_DEFAULT;
    }
    else if (nLineWeight == DXF_LINEWEIGHT_BYBLOCK)
        nLineWeight = mnBlockLineWeight;
    return nLineWeight;
}

LineInfo DXF2GDIMetaFile::GetEntityLineInfo(const DXFBasicEntity & rE, const DXFTransform & rTransform)
{
    if (mbMeasure)
        return LineInfo();

    LineInfo aLineInfo;
    DXFLineInfo aDXFLineInfo = GetEntityDXFLineInfo(rE);
    if (aDXFLineInfo.eStyle != LineStyle::Solid && rE.fLineTypeScale > 0)
    {
        aDXFLineInfo.fDashLen *= rE.fLineTypeScale;
        aDXFLineInfo.fDotLen *= rE.fLineTypeScale;
        aDXFLineInfo.fDistance *= rE.fLineTypeScale;
        DXFTransform aDeviceTransform(
            rTransform, DXFTransform(mfDeviceScale, mfDeviceScale, mfDeviceScale, DXFVector()));
        aLineInfo = aDeviceTransform.Transform(aDXFLineInfo);
        // a pattern too dense to be seen is drawn as a solid line
        const double fPeriod
            = aLineInfo.GetDashCount() * (aLineInfo.GetDashLen() + aLineInfo.GetDistance())
              + aLineInfo.GetDotCount() * (aLineInfo.GetDotLen() + aLineInfo.GetDistance());
        if (!(fPeriod >= MIN_PATTERN_PERIOD))
            aLineInfo = LineInfo();
    }

    // Line weights are in 1/100 mm like the metafile, the thin ones are drawn as hairlines
    // like on the screen.
    if (pDXF->mbLineWeightDisplay)
    {
        const tools::Long nLineWeight = GetEntityLineWeight(rE);
        if (nLineWeight > 25 && nLineWeight <= MAX_LINE_WEIGHT)
            aLineInfo.SetWidth(nLineWeight);
    }
    return aLineInfo;
}

bool DXF2GDIMetaFile::SetLineAttribute(const DXFBasicEntity & rE)
{
    std::optional<Color> oColor = GetEntityColor(rE);
    if (!oColor)
        return false;
    const Color aColor = *oColor;

    if (aActLineColor!=aColor) {
        aActLineColor = aColor;
        pVirDev->SetLineColor( aActLineColor );
    }

    if (aActFillColor!=COL_TRANSPARENT) {
        aActFillColor = COL_TRANSPARENT;
        pVirDev->SetFillColor();
    }
    return true;
}


bool DXF2GDIMetaFile::SetAreaAttribute(const DXFBasicEntity & rE)
{
    std::optional<Color> oColor = GetEntityColor(rE);
    if (!oColor)
        return false;
    const Color aColor = *oColor;

    if (aActLineColor!=aColor) {
        aActLineColor = aColor;
        pVirDev->SetLineColor( aActLineColor );
    }

    if ( aActFillColor == COL_TRANSPARENT || aActFillColor != aColor) {
        aActFillColor = aColor;
        pVirDev->SetFillColor( aActFillColor );
    }
    return true;
}


bool DXF2GDIMetaFile::SetFontAttribute(const DXFBasicEntity & rE, Degree10 nOrientation,
                                       sal_Int32 nHeight)
{
    std::optional<Color> oColor = GetEntityColor(rE);
    if (!oColor)
        return false;
    const Color aColor = *oColor;
    vcl::Font aFont;

    aFont.SetColor(aColor);
    aFont.SetTransparent(true);
    aFont.SetFamily(FAMILY_SWISS);
    aFont.SetFontSize(Size(0,nHeight));
    aFont.SetAlignment(ALIGN_BASELINE);
    aFont.SetOrientation(nOrientation);
    if (aActFont!=aFont) {
        aActFont=std::move(aFont);
        pVirDev->SetFont(aActFont);
    }

    return true;
}

void DXF2GDIMetaFile::OutPolyLine(const basegfx::B2DPolygon & rPolygon, const LineInfo & rLineInfo)
{
    if (rPolygon.count() == 0)
        return;
    if (mbMeasure)
    {
        maExtents.expand(rPolygon.getB2DRange());
        return;
    }

    basegfx::B2DPolygon aPolygon(rPolygon);
    aPolygon.transform(maDeviceTransform);
    if (aPolygon.isClosed())
    {
        // draw the closing edge as part of the polyline
        aPolygon.append(aPolygon.getB2DPoint(0));
        if (aPolygon.areControlPointsUsed() && aPolygon.isPrevControlPointUsed(0))
            aPolygon.setPrevControlPoint(aPolygon.count() - 1, aPolygon.getPrevControlPoint(0));
        aPolygon.setClosed(false);
    }
    if (aPolygon.count() == 1)
        aPolygon.append(aPolygon.getB2DPoint(0));

    // split long polylines, keeping the joint point in both parts
    sal_uInt32 nStart = 0;
    const sal_uInt32 nCount = aPolygon.count();
    while (nStart + 1 < nCount)
    {
        const sal_uInt32 nEnd = std::min(nStart + MAX_POLYGON_POINTS, nCount - 1);
        basegfx::B2DPolygon aPart;
        aPart.append(aPolygon, nStart, nEnd - nStart + 1);
        pVirDev->DrawPolyLine(tools::Polygon(aPart), rLineInfo);
        nStart = nEnd;
    }
}

void DXF2GDIMetaFile::OutPolygon(const basegfx::B2DPolyPolygon & rPolyPolygon)
{
    if (rPolyPolygon.count() == 0)
        return;
    if (mbMeasure)
    {
        maExtents.expand(rPolyPolygon.getB2DRange());
        return;
    }

    basegfx::B2DPolyPolygon aPolyPolygon;
    for (const basegfx::B2DPolygon& rPolygon : rPolyPolygon)
    {
        if (rPolygon.count() == 0 || rPolygon.count() > MAX_POLYGON_POINTS)
        {
            SAL_WARN_IF(rPolygon.count() > 0, "vcl.filter",
                        "DXF2GDIMetaFile::OutPolygon: too many points: " << rPolygon.count());
            continue;
        }
        basegfx::B2DPolygon aPolygon(rPolygon);
        aPolygon.setClosed(true);
        aPolyPolygon.append(aPolygon);
    }
    if (aPolyPolygon.count() == 0)
        return;
    aPolyPolygon.transform(maDeviceTransform);
    if (aPolyPolygon.count() == 1)
        pVirDev->DrawPolygon(tools::Polygon(aPolyPolygon.getB2DPolygon(0)));
    else
        pVirDev->DrawPolyPolygon(tools::PolyPolygon(aPolyPolygon));
}

void DXF2GDIMetaFile::OutPoint(const basegfx::B2DPoint & rPoint)
{
    if (mbMeasure)
    {
        maExtents.expand(rPoint);
        return;
    }
    const basegfx::B2DPoint aPoint = maDeviceTransform * rPoint;
    pVirDev->DrawPixel(Point(basegfx::fround<tools::Long>(aPoint.getX()),
                             basegfx::fround<tools::Long>(aPoint.getY())));
}

void DXF2GDIMetaFile::OutText(const DXFBasicEntity & rE, const DXFTransform & rTextTransform,
                              std::string_view rText)
{
    const OUString aText(pDXF->ToOUString(rText));
    if (aText.isEmpty())
        return;

    if (mbMeasure)
    {
        // There is no font in drawing units to measure with, estimate the size of the text.
        const double fWidth = 0.6 * aText.getLength();
        maExtents.expand(Project(rTextTransform, DXFVector(0, 0, 0)));
        maExtents.expand(Project(rTextTransform, DXFVector(fWidth, 0, 0)));
        maExtents.expand(Project(rTextTransform, DXFVector(fWidth, 1, 0)));
        maExtents.expand(Project(rTextTransform, DXFVector(0, 1, 0)));
        return;
    }

    const basegfx::B2DPoint aOrigin
        = maDeviceTransform * Project(rTextTransform, DXFVector(0, 0, 0));
    const basegfx::B2DVector aBaseline(
        maDeviceTransform * Project(rTextTransform, DXFVector(1, 0, 0)) - aOrigin);
    const basegfx::B2DVector aUp(
        maDeviceTransform * Project(rTextTransform, DXFVector(0, 1, 0)) - aOrigin);

    const double fHeight = aUp.getLength();
    if (!(fHeight >= 1.0) || fHeight > SAL_MAX_INT32)
        return;
    // the metafile y axis points down, the orientation is counter-clockwise
    const double fAngle = basegfx::rad2deg(atan2(-aBaseline.getY(), aBaseline.getX()));
    sal_Int32 nOrientation = basegfx::fround(fAngle * 10.0) % 3600;
    if (nOrientation < 0)
        nOrientation += 3600;
    if (!SetFontAttribute(rE, Degree10(static_cast<sal_Int16>(nOrientation)),
                          basegfx::fround(fHeight)))
        return;
    pVirDev->DrawText(Point(basegfx::fround<tools::Long>(aOrigin.getX()),
                            basegfx::fround<tools::Long>(aOrigin.getY())),
                      aText);
}

void DXF2GDIMetaFile::OutThickness(const basegfx::B2DPolygon & rBase, const DXFTransform & rTransform,
                                   double fThickness, bool bFilled)
{
    if (fThickness == 0)
        return;
    DXFVector aOffset;
    rTransform.TransDir(DXFVector(0, 0, fThickness), aOffset);
    // seen from the top the sides are not visible
    if (std::hypot(aOffset.fx, aOffset.fy) <= 1e-9 * std::fabs(fThickness))
        return;

    basegfx::B2DPolygon aTop(rBase);
    aTop.transform(basegfx::utils::createTranslateB2DHomMatrix(aOffset.fx, aOffset.fy));
    if (bFilled)
        OutPolygon(basegfx::B2DPolyPolygon(aTop));
    else
        OutPolyLine(aTop, LineInfo());
    for (sal_uInt32 i = 0; i < rBase.count(); ++i)
    {
        basegfx::B2DPolygon aSide;
        aSide.append(rBase.getB2DPoint(i));
        aSide.append(aTop.getB2DPoint(i));
        OutPolyLine(aSide, LineInfo());
    }
}

void DXF2GDIMetaFile::DrawLineEntity(const DXFLineEntity & rE, const DXFTransform & rTransform)
{
    if (!SetLineAttribute(rE))
        return;

    basegfx::B2DPolygon aLine;
    aLine.append(Project(rTransform, rE.aP0));
    aLine.append(Project(rTransform, rE.aP1));
    OutPolyLine(aLine, GetEntityLineInfo(rE, rTransform));
    OutThickness(aLine, rTransform, rE.fThickness, false);
}


void DXF2GDIMetaFile::DrawPointEntity(const DXFPointEntity & rE, const DXFTransform & rTransform)
{
    if (!SetLineAttribute(rE))
        return;

    const basegfx::B2DPoint aPoint(Project(rTransform, rE.aP0));
    OutPoint(aPoint);
    OutThickness(basegfx::B2DPolygon{ aPoint }, rTransform, rE.fThickness, false);
}


void DXF2GDIMetaFile::DrawCircleEntity(const DXFCircleEntity & rE, const DXFTransform & rTransform)
{
    if (!SetLineAttribute(rE))
        return;

    const basegfx::B2DPolygon aCircle(Project(
        rTransform,
        basegfx::utils::createPolygonFromCircle(basegfx::B2DPoint(rE.aP0.fx, rE.aP0.fy),
                                                rE.fRadius),
        rE.aP0.fz));
    OutPolyLine(aCircle, GetEntityLineInfo(rE, rTransform));
    OutThickness(aCircle, rTransform, rE.fThickness, false);
}

void DXF2GDIMetaFile::DrawArcEntity(const DXFArcEntity & rE, const DXFTransform & rTransform)
{
    if (!SetLineAttribute(rE))
        return;

    // counter-clockwise from the start to the end angle
    const double fStart = basegfx::deg2rad(rE.fStart);
    double fSweep = fmod(rE.fEnd - rE.fStart, 360.0);
    if (fSweep <= 0)
        fSweep += 360.0;
    const basegfx::B2DPolygon aArc(Project(
        rTransform,
        basegfx::utils::createPolygonFromEllipseSegment(
            basegfx::B2DPoint(rE.aP0.fx, rE.aP0.fy), rE.fRadius, rE.fRadius, fStart,
            fStart + basegfx::deg2rad(fSweep)),
        rE.aP0.fz));
    OutPolyLine(aArc, GetEntityLineInfo(rE, rTransform));
    OutThickness(aArc, rTransform, rE.fThickness, false);
}

void DXF2GDIMetaFile::DrawTraceEntity(const DXFTraceEntity & rE, const DXFTransform & rTransform)
{
    if (!SetAreaAttribute(rE))
        return;

    // the corners are in the order 1, 2, 4, 3
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(Project(rTransform, rE.aP0));
    aPolygon.append(Project(rTransform, rE.aP1));
    aPolygon.append(Project(rTransform, rE.aP3));
    aPolygon.append(Project(rTransform, rE.aP2));
    aPolygon.setClosed(true);
    OutPolygon(basegfx::B2DPolyPolygon(aPolygon));
    OutThickness(aPolygon, rTransform, rE.fThickness, true);
}


void DXF2GDIMetaFile::DrawSolidEntity(const DXFSolidEntity & rE, const DXFTransform & rTransform)
{
    if (!SetAreaAttribute(rE))
        return;

    // the corners are in the order 1, 2, 4, 3; with only three corners the fourth one
    // repeats the third one
    basegfx::B2DPolygon aPolygon;
    aPolygon.append(Project(rTransform, rE.aP0));
    aPolygon.append(Project(rTransform, rE.aP1));
    aPolygon.append(Project(rTransform, rE.aP3));
    if (!(rE.aP2 == rE.aP3))
        aPolygon.append(Project(rTransform, rE.aP2));
    aPolygon.setClosed(true);
    OutPolygon(basegfx::B2DPolyPolygon(aPolygon));
    OutThickness(aPolygon, rTransform, rE.fThickness, true);
}


void DXF2GDIMetaFile::DrawTextEntity(const DXFTextEntity & rE, const DXFTransform & rTransform)
{
    DXFTransform aT(DXFTransform(rE.fXScale * rE.fHeight, rE.fHeight, 1.0, rE.fRotAngle, rE.aP0),
                    rTransform);
    OutText(rE, aT, rE.m_sText);
}


void DXF2GDIMetaFile::DrawBlock(const DXFBlock & rBlock, const DXFBasicEntity & rE,
                                const DXFTransform & rTransform)
{
    const DXFLayer * pLayer = GetEntityLayer(rE);
    if (rE.bInvisible || (pLayer != nullptr && (pLayer->nFlags & 1) != 0))
        return;

    std::optional<Color> oSavedBlockColor = moBlockColor;
    DXFLineInfo aSavedBlockDXFLineInfo = aBlockDXFLineInfo;
    const tools::Long nSavedBlockLineWeight = mnBlockLineWeight;
    const DXFLayer * pSavedParentLayer = mpParentLayer;
    // with the insert on a layer that is off, only the entities on layer 0 are hidden
    moBlockColor = GetEntityColor(rE);
    aBlockDXFLineInfo = GetEntityDXFLineInfo(rE);
    mnBlockLineWeight = GetEntityLineWeight(rE);
    mpParentLayer = pLayer;
    DrawEntities(rBlock, rTransform);
    moBlockColor = std::move(oSavedBlockColor);
    aBlockDXFLineInfo = aSavedBlockDXFLineInfo;
    mnBlockLineWeight = nSavedBlockLineWeight;
    mpParentLayer = pSavedParentLayer;
}


void DXF2GDIMetaFile::DrawInsertEntity(const DXFInsertEntity & rE, const DXFTransform & rTransform)
{
    const DXFBlock * pB;
    pB=pDXF->aBlocks.Search(rE.m_sName);
    if (pB==nullptr)
        return;

    DXFTransform aDXFTransform1(1.0,1.0,1.0,DXFVector(0.0,0.0,0.0)-pB->aBasePoint);
    DXFTransform aDXFTransform2(rE.fXScale,rE.fYScale,rE.fZScale,rE.fRotAngle,rE.aP0);
    DXFTransform aT(
        DXFTransform( aDXFTransform1, aDXFTransform2 ),
        rTransform
    );
    DrawBlock(*pB, rE, aT);
}


void DXF2GDIMetaFile::DrawAttribEntity(const DXFAttribEntity & rE, const DXFTransform & rTransform)
{
    if ((rE.nAttrFlags&1)!=0)
        return;

    DXFTransform aT(DXFTransform(rE.fXScale * rE.fHeight, rE.fHeight, 1.0, rE.fRotAngle, rE.aP0),
                    rTransform);
    OutText(rE, aT, rE.m_sText);
}


void DXF2GDIMetaFile::DrawPolyLineEntity(const DXFPolyLineEntity & rE, const DXFTransform & rTransform)
{
    if (!SetLineAttribute(rE))
        return;

    basegfx::B2DPolygon aPolygon;
    for (const DXFBasicEntity* pBE = rE.pSucc; pBE != nullptr && pBE->eType == DXF_VERTEX;
         pBE = pBE->pSucc)
        aPolygon.append(Project(rTransform, static_cast<const DXFVertexEntity*>(pBE)->aP0));
    if (aPolygon.count() < 2)
        return;
    aPolygon.setClosed((rE.nFlags & 1) != 0);

    OutPolyLine(aPolygon, GetEntityLineInfo(rE, rTransform));
    OutThickness(aPolygon, rTransform, rE.fThickness, false);
}

void DXF2GDIMetaFile::DrawLWPolyLineEntity(const DXFLWPolyLineEntity & rE, const DXFTransform & rTransform )
{
    if (rE.aP.empty() || !SetLineAttribute(rE))
        return;

    basegfx::B2DPolygon aPolygon;
    for (const DXFVector& rPoint : rE.aP)
        aPolygon.append(Project(rTransform, rPoint));
    aPolygon.setClosed((rE.nFlags & 1) != 0);
    OutPolyLine(aPolygon, GetEntityLineInfo(rE, rTransform));
}

void DXF2GDIMetaFile::DrawHatchEntity(const DXFHatchEntity & rE, const DXFTransform & rTransform )
{
    if (rE.aBoundaryPathData.empty())
        return;

    if (!SetAreaAttribute(rE))
        return;
    basegfx::B2DPolyPolygon aPolyPolygon;
    for (const DXFBoundaryPathData& rPathData : rE.aBoundaryPathData)
    {
        basegfx::B2DPolygon aPolygon;
        if ( rPathData.bIsPolyLine )
        {
            for (const auto& a : rPathData.aP)
                aPolygon.append(Project(rTransform, a));
        }
        else
        {
            for ( auto& rEdge : rPathData.aEdges )
            {
                const DXFEdgeType* pEdge = rEdge.get();
                switch( pEdge->nEdgeType )
                {
                    case 1 :
                    {
                        aPolygon.append(Project(rTransform, static_cast<const DXFEdgeTypeLine*>(pEdge)->aStartPoint));
                        aPolygon.append(Project(rTransform, static_cast<const DXFEdgeTypeLine*>(pEdge)->aEndPoint));
                    }
                    break;
                    case 2 :
                    case 3 :
                    case 4 :
                    break;
                }
            }
        }
        if (aPolygon.count() > 0)
        {
            aPolygon.setClosed(true);
            aPolyPolygon.append(aPolygon);
        }
    }
    OutPolygon(aPolyPolygon);
}

void DXF2GDIMetaFile::Draw3DFaceEntity(const DXF3DFaceEntity & rE, const DXFTransform & rTransform)
{
    if (!SetLineAttribute(rE))
        return;

    std::vector<basegfx::B2DPoint> aCorners;
    aCorners.push_back(Project(rTransform, rE.aP0));
    aCorners.push_back(Project(rTransform, rE.aP1));
    aCorners.push_back(Project(rTransform, rE.aP2));
    if (!(rE.aP2 == rE.aP3))
        aCorners.push_back(Project(rTransform, rE.aP3));
    const size_t nN = aCorners.size();

    if ((rE.nIEFlags & 0x0f) == 0)
    {
        basegfx::B2DPolygon aPolygon;
        for (const basegfx::B2DPoint& rCorner : aCorners)
            aPolygon.append(rCorner);
        aPolygon.setClosed(true);
        OutPolyLine(aPolygon, LineInfo());
        return;
    }
    // only the visible edges; a triangle has an empty third edge, its closing edge is the fourth
    for (size_t i = 0; i < nN; ++i)
    {
        const tools::Long nEdgeFlag = nN == 3 && i == 2 ? 8 : static_cast<tools::Long>(1) << i;
        if ((rE.nIEFlags & nEdgeFlag) == 0)
        {
            basegfx::B2DPolygon aEdge;
            aEdge.append(aCorners[i]);
            aEdge.append(aCorners[(i + 1) % nN]);
            OutPolyLine(aEdge, LineInfo());
        }
    }
}

void DXF2GDIMetaFile::DrawDimensionEntity(const DXFDimensionEntity & rE, const DXFTransform & rTransform)
{
    const DXFBlock * pB;
    pB=pDXF->aBlocks.Search(rE.m_sPseudoBlock);
    if (pB==nullptr)
        return;

    DXFTransform aT(
        DXFTransform(1.0,1.0,1.0,DXFVector(0.0,0.0,0.0)-pB->aBasePoint),
        rTransform
    );
    DrawBlock(*pB, rE, aT);
}


void DXF2GDIMetaFile::DrawEntities(const DXFEntities & rEntities,
                                   const DXFTransform & rTransform)
{
    if (rEntities.mbBeingDrawn)
        return;
    rEntities.mbBeingDrawn = true;

    DXFTransform aET;
    const DXFTransform * pT;

    const DXFBasicEntity * pE=rEntities.pFirst;

    while (pE!=nullptr) {
        if (pE->nSpace==0) {
            if (pE->aExtrusion.fz==1.0) {
                pT=&rTransform;
            }
            else {
                aET=DXFTransform(DXFTransform(pE->aExtrusion),rTransform);
                pT=&aET;
            }
            switch (pE->eType) {
            case DXF_LINE:
                DrawLineEntity(static_cast<const DXFLineEntity&>(*pE),*pT);
                break;
            case DXF_POINT:
                DrawPointEntity(static_cast<const DXFPointEntity&>(*pE),*pT);
                break;
            case DXF_CIRCLE:
                DrawCircleEntity(static_cast<const DXFCircleEntity&>(*pE),*pT);
                break;
            case DXF_ARC:
                DrawArcEntity(static_cast<const DXFArcEntity&>(*pE),*pT);
                break;
            case DXF_TRACE:
                DrawTraceEntity(static_cast<const DXFTraceEntity&>(*pE),*pT);
                break;
            case DXF_SOLID:
                DrawSolidEntity(static_cast<const DXFSolidEntity&>(*pE),*pT);
                break;
            case DXF_TEXT:
                DrawTextEntity(static_cast<const DXFTextEntity&>(*pE),*pT);
                break;
            case DXF_INSERT:
                DrawInsertEntity(static_cast<const DXFInsertEntity&>(*pE),*pT);
                break;
            case DXF_ATTRIB:
                DrawAttribEntity(static_cast<const DXFAttribEntity&>(*pE),*pT);
                break;
            case DXF_POLYLINE:
                DrawPolyLineEntity(static_cast<const DXFPolyLineEntity&>(*pE),*pT);
                break;
            case DXF_LWPOLYLINE :
                DrawLWPolyLineEntity(static_cast<const DXFLWPolyLineEntity&>(*pE), *pT);
                break;
            case DXF_HATCH :
                DrawHatchEntity(static_cast<const DXFHatchEntity&>(*pE), *pT);
                break;
            case DXF_3DFACE:
                Draw3DFaceEntity(static_cast<const DXF3DFaceEntity&>(*pE),*pT);
                break;
            case DXF_DIMENSION:
                DrawDimensionEntity(static_cast<const DXFDimensionEntity&>(*pE),*pT);
                break;
            default:
                break;  // four other values not handled -Wall
            }
        }
        pE=pE->pSucc;
    }

    rEntities.mbBeingDrawn = false;
}


DXF2GDIMetaFile::DXF2GDIMetaFile()
    : pVirDev(nullptr)
    , pDXF(nullptr)
    , mbMeasure(false)
    , mfDeviceScale(1.0)
    , mnBlockLineWeight(DXF_LINEWEIGHT_DEFAULT)
    , mpParentLayer(nullptr)
{
}


DXF2GDIMetaFile::~DXF2GDIMetaFile()
{
}


bool DXF2GDIMetaFile::Convert(const DXFRepresentation & rDXF, GDIMetaFile & rMTF)
{
    pDXF    = &rDXF;

    // A viewport looking at the drawing from another direction than the top gives the
    // projection, the extents always come from what is drawn.
    DXFTransform aViewTransform;
    const DXFVPort * pVPort = pDXF->aTables.SearchVPort("*ACTIVE");
    if (pVPort != nullptr && (pVPort->aDirection.fx != 0 || pVPort->aDirection.fy != 0))
        aViewTransform = DXFTransform(pVPort->aDirection, pVPort->aTarget);

    // first pass: the extents
    pVirDev = VclPtr<VirtualDevice>::Create();
    pVirDev->EnableOutput(false);
    ResetState();
    mbMeasure = true;
    maExtents.reset();
    DrawEntities(pDXF->aEntities, aViewTransform);
    pVirDev.disposeAndClear();
    mbMeasure = false;

    if (maExtents.isEmpty() || !IsFinite(maExtents))
        return false;

    double fWidth = maExtents.getWidth();
    double fHeight = maExtents.getHeight();
    if (!(std::max(fWidth, fHeight) > 0))
        return false;
    // Far from the origin, a drawing too small for the precision of its coordinates is shown
    // as a small part of a larger area, so that the metafile coordinates stay in range.
    const double fMagnitude
        = std::max({ 1.0, std::fabs(maExtents.getMinX()), std::fabs(maExtents.getMaxX()),
                     std::fabs(maExtents.getMinY()), std::fabs(maExtents.getMaxY()) });
    const double fSize = std::max({ fWidth, fHeight, fMagnitude * 1e-9 });
    // a drawing that is only a horizontal or vertical line still gets an area
    fWidth = std::max(fWidth, fSize / 1000.0);
    fHeight = std::max(fHeight, fSize / 1000.0);
    mfDeviceScale = DEVICE_SIZE / fSize;
    // half of the widest line weight on the border is drawn outside of the extents
    const double fMargin
        = DEVICE_MARGIN + (pDXF->mbLineWeightDisplay ? MAX_LINE_WEIGHT / 2.0 : 0.0);
    maDeviceTransform = basegfx::utils::createScaleTranslateB2DHomMatrix(
        mfDeviceScale, -mfDeviceScale, fMargin - maExtents.getMinX() * mfDeviceScale,
        fMargin + maExtents.getMaxY() * mfDeviceScale);

    // second pass: the metafile
    pVirDev = VclPtr<VirtualDevice>::Create();
    pVirDev->EnableOutput(false);
    if (!comphelper::IsFuzzing()) // for fuzzing don't bother recording the drawing
        rMTF.Record(pVirDev);
    ResetState();
    DrawEntities(pDXF->aEntities, aViewTransform);
    rMTF.Stop();

    Size aPrefSize(basegfx::fround<tools::Long>(fWidth * mfDeviceScale + 2 * fMargin),
                   basegfx::fround<tools::Long>(fHeight * mfDeviceScale + 2 * fMargin));
    rMTF.SetPrefSize( aPrefSize );
    rMTF.SetPrefMapMode( MapMode( MapUnit::Map100thMM ) );

    pVirDev.disposeAndClear();
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
