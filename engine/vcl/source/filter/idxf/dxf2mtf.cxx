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
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <comphelper/configuration.hxx>
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/gradient.hxx>
#include <vcl/hatch.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <tools/poly.hxx>
#include "dxf2mtf.hxx"

#include <algorithm>
#include <cmath>
#include <limits>

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

// Limits against drawings that would never finish: the nesting of inserts, the copies of
// an array insert, the entities and copies visited in one pass, even if they draw nothing,
// and the output of one pass, counted in points plus an overhead for each metafile action.
constexpr sal_Int32 MAX_INSERT_DEPTH = 32;
constexpr tools::Long MAX_ARRAY_COUNT = 10000;
constexpr sal_uInt64 MAX_VISITS = 4000000;
constexpr sal_uInt64 MAX_WORK = 8000000;
constexpr sal_uInt64 ACTION_WORK = 8;

// The most points and polygons of the 16-bit counts of the metafile
constexpr sal_uInt32 MAX_TOOLS_COUNT = 65535;

// Coordinates further than this are garbage, not geometry
constexpr double MAX_COORDINATE = 1e15;

// a grid of rows and columns of at most nPoints points
bool IsGrid(tools::Long nRows, tools::Long nColumns, sal_uInt64 nPoints)
{
    return nRows >= 1 && nColumns >= 1 && o3tl::make_unsigned(nRows) <= nPoints
           && o3tl::make_unsigned(nColumns) <= nPoints
           && o3tl::make_unsigned(nRows) * o3tl::make_unsigned(nColumns) <= nPoints;
}

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

// the relative luminance of an sRGB colour
double Luminance(double fRed, double fGreen, double fBlue)
{
    double fLinear[3] = { fRed / 255.0, fGreen / 255.0, fBlue / 255.0 };
    for (double& rComponent : fLinear)
        rComponent = rComponent <= 0.04045 ? rComponent / 12.92
                                           : std::pow((rComponent + 0.055) / 1.055, 2.4);
    return 0.2126 * fLinear[0] + 0.7152 * fLinear[1] + 0.0722 * fLinear[2];
}

// The colours of drawings are chosen for the dark background of CAD programs, while the drawing
// is shown on light paper. Darken the colours that would have less than a 3:1 contrast to white,
// keeping their hue, the others stay as they are.
Color DarkenForContrast(const Color& rColor)
{
    // the contrast to white is (1 + 0.05) / (luminance + 0.05)
    constexpr double fMaxLuminance = 1.05 / 3.0 - 0.05;
    const double fRed = rColor.GetRed();
    const double fGreen = rColor.GetGreen();
    const double fBlue = rColor.GetBlue();
    if (Luminance(fRed, fGreen, fBlue) <= fMaxLuminance)
        return rColor;
    double fLow = 0.0;
    double fHigh = 1.0;
    for (int i = 0; i < 20; ++i)
    {
        const double fScale = (fLow + fHigh) / 2;
        if (Luminance(fRed * fScale, fGreen * fScale, fBlue * fScale) > fMaxLuminance)
            fHigh = fScale;
        else
            fLow = fScale;
    }
    return Color(static_cast<sal_uInt8>(fRed * fLow), static_cast<sal_uInt8>(fGreen * fLow),
                 static_cast<sal_uInt8>(fBlue * fLow));
}

// Some programs write white as the true colour of colour index 7, which is black on a white
// background.
bool IsTrueColor(tools::Long nTrueColor, tools::Long nColor)
{
    return nTrueColor >= 0 && !(nTrueColor == 0xffffff && (nColor == 7 || nColor == -7));
}

// whether a polygon fits into a polygon of the metafile, a curve segment takes three points
bool FitsToolsPolygon(const basegfx::B2DPolygon& rPolygon)
{
    const sal_uInt64 nPoints = rPolygon.count() + 1;
    return (rPolygon.areControlPointsUsed() ? 3 * nPoints : nPoints) <= MAX_TOOLS_COUNT;
}

// the polygons that fit into the metafile, closed
basegfx::B2DPolyPolygon ToolsPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    basegfx::B2DPolyPolygon aResult;
    for (const basegfx::B2DPolygon& rPolygon : rPolyPolygon)
    {
        if (rPolygon.count() == 0 || aResult.count() >= MAX_TOOLS_COUNT)
            continue;
        if (!FitsToolsPolygon(rPolygon))
        {
            SAL_WARN("vcl.filter", "DXF2GDIMetaFile: too many points: " << rPolygon.count());
            continue;
        }
        basegfx::B2DPolygon aPolygon(rPolygon);
        aPolygon.setClosed(true);
        aResult.append(aPolygon);
    }
    return aResult;
}

sal_uInt64 PointCount(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    sal_uInt64 nPoints = 0;
    for (const basegfx::B2DPolygon& rPolygon : rPolyPolygon)
        nPoints += rPolygon.count();
    return nPoints;
}

bool IsSane(const basegfx::B2DRange& rRange)
{
    return !rRange.isEmpty() && std::fabs(rRange.getMinX()) < MAX_COORDINATE
           && std::fabs(rRange.getMinY()) < MAX_COORDINATE
           && std::fabs(rRange.getMaxX()) < MAX_COORDINATE
           && std::fabs(rRange.getMaxY()) < MAX_COORDINATE;
}
}

Color DXF2GDIMetaFile::EnsureContrast(const Color & rColor)
{
    // the entities of a drawing come in runs with the same colour
    if (rColor != maContrastInput)
    {
        maContrastInput = rColor;
        maContrastOutput = DarkenForContrast(rColor);
    }
    return maContrastOutput;
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
    return ResolveColor(rE, rE.nColor, rE.nTrueColor);
}

std::optional<Color> DXF2GDIMetaFile::ResolveColor(const DXFBasicEntity & rE, tools::Long nColor,
                                                   tools::Long nTrueColor) const
{
    if (rE.bInvisible)
        return {};
    const DXFLayer * pLayer = GetEntityLayer(rE);
    if (pLayer != nullptr && pLayer->IsHidden())
        return {};

    if (IsTrueColor(nTrueColor, nColor))
        return Color(ColorTransparency, nTrueColor);
    if (nColor == 0) // BYBLOCK
        return moBlockColor;
    if (nColor == 256) // BYLAYER
    {
        if (pLayer == nullptr)
            return ConvertColor(7);
        if (IsTrueColor(pLayer->nTrueColor, pLayer->nColor))
            return Color(ColorTransparency, pLayer->nTrueColor);
        return ConvertColor(pLayer->nColor);
    }
    return ConvertColor(nColor);
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

    if (rE.m_sLineType.equalsIgnoreAsciiCase("BYLAYER")) {
        const DXFLayer * pLayer = GetEntityLayer(rE);
        if (pLayer != nullptr)
            aDXFLineInfo = LTypeToDXFLineInfo(pLayer->m_sLineType);
    }
    else if (rE.m_sLineType.equalsIgnoreAsciiCase("BYBLOCK")) {
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
    const Color aColor = EnsureContrast(*oColor);

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
    const Color aColor = EnsureContrast(*oColor);

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


bool DXF2GDIMetaFile::AddVisit()
{
    ++mnVisits;
    if (mnVisits <= MAX_VISITS)
        return true;
    SAL_WARN_IF(mnVisits == MAX_VISITS + 1, "vcl.filter", "DXF2GDIMetaFile: too much to visit");
    return false;
}

bool DXF2GDIMetaFile::IsOverBudget() const { return mnWork > MAX_WORK || mnVisits > MAX_VISITS; }

bool DXF2GDIMetaFile::AddWork(sal_uInt64 nWork)
{
    nWork += ACTION_WORK;
    mnWork += nWork;
    if (mnWork <= MAX_WORK)
        return true;
    SAL_WARN_IF(mnWork - nWork <= MAX_WORK, "vcl.filter", "DXF2GDIMetaFile: too much to draw");
    return false;
}

void DXF2GDIMetaFile::OutPolyLine(const basegfx::B2DPolygon & rPolygon, const LineInfo & rLineInfo)
{
    if (rPolygon.count() == 0 || !AddWork(rPolygon.count()))
        return;
    const basegfx::B2DRange aRange = rPolygon.getB2DRange();
    if (!IsSane(aRange))
        return;
    if (mbMeasure)
    {
        maExtents.expand(aRange);
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
    if (rPolyPolygon.count() == 0 || !AddWork(PointCount(rPolyPolygon)))
        return;
    const basegfx::B2DRange aRange = rPolyPolygon.getB2DRange();
    if (!IsSane(aRange))
        return;
    if (mbMeasure)
    {
        maExtents.expand(aRange);
        return;
    }

    basegfx::B2DPolyPolygon aPolyPolygon = ToolsPolyPolygon(rPolyPolygon);
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
    // Points are often stray definition points far from the drawing: they count for the
    // extents only if there is nothing else, and they are not drawn outside of them.
    if (!AddWork(1) || !IsSane(basegfx::B2DRange(rPoint)))
        return;
    if (mbMeasure)
    {
        maPointExtents.expand(rPoint);
        return;
    }
    if (!maExtents.isInside(rPoint))
        return;
    const basegfx::B2DPoint aPoint = maDeviceTransform * rPoint;
    pVirDev->DrawPixel(Point(basegfx::fround<tools::Long>(aPoint.getX()),
                             basegfx::fround<tools::Long>(aPoint.getY())));
}

DXFTextFormat DXF2GDIMetaFile::GetStyleFormat(std::string_view rStyle) const
{
    DXFTextFormat aFormat;
    const DXFStyle * pStyle = pDXF->aTables.SearchStyle(rStyle);
    if (pStyle == nullptr)
        return aFormat;
    if (!pStyle->m_sFontFamily.isEmpty())
        aFormat.maFontName = pDXF->ToOUString(pStyle->m_sFontFamily);
    else if (!pStyle->m_sPrimFontFile.isEmpty())
        aFormat.maFontName = dxf::fontNameFromFile(pDXF->ToOUString(pStyle->m_sPrimFontFile),
                                                   aFormat.mbBold, aFormat.mbItalic);
    if (pStyle->nFontFlags & 0x2000000)
        aFormat.mbBold = true;
    if (pStyle->nFontFlags & 0x1000000)
        aFormat.mbItalic = true;
    if (pStyle->fHeight > 0)
        aFormat.mfHeight = pStyle->fHeight;
    if (pStyle->fWidthFak > 0)
        aFormat.mfWidthFactor = pStyle->fWidthFak;
    aFormat.mfOblique = pStyle->fOblAngle;
    return aFormat;
}

void DXF2GDIMetaFile::DrawSingleLineText(const DXFBasicEntity & rE, const DXFSingleLineText & rText,
                                         const DXFTransform & rTransform)
{
    const OUString aText(pDXF->ToOUString(rText.maText));
    if (aText.isEmpty())
        return;

    DXFTextFormat aFormat = GetStyleFormat(rText.maStyle);
    if (rText.mfHeight > 0)
        aFormat.mfHeight = rText.mfHeight;
    aFormat.mfWidthFactor = rText.mfWidthFactor > 0 ? rText.mfWidthFactor : 1.0;
    aFormat.mfOblique = rText.mfOblique;
    double fWidth = MeasureText(aText, aFormat);

    // the start of the baseline relative to the placement point, in the text direction
    DXFVector aOrigin = rText.maPoint;
    double fAngle = rText.mfRotation;
    double fX = 0;
    double fY = 0;
    const bool bAligned = rText.mbHasAlignPoint && (rText.mnHorizontal != 0 || rText.mnVertical != 0);
    if (bAligned && (rText.mnHorizontal == 3 || rText.mnHorizontal == 5))
    {
        // between the two points: aligned scales the height, fit only the width
        const DXFVector aBaseline = rText.maAlignPoint - rText.maPoint;
        const double fLength = std::hypot(aBaseline.fx, aBaseline.fy);
        if (fLength > 0 && fWidth > 0)
        {
            fAngle = basegfx::rad2deg(atan2(aBaseline.fy, aBaseline.fx));
            if (rText.mnHorizontal == 3)
                aFormat.mfHeight *= fLength / fWidth;
            else
                aFormat.mfWidthFactor *= fLength / fWidth;
            fWidth = fLength;
        }
    }
    else if (bAligned)
    {
        aOrigin = rText.maAlignPoint;
        if (rText.mnHorizontal == 1 || rText.mnHorizontal == 4)
            fX = -fWidth / 2;
        else if (rText.mnHorizontal == 2)
            fX = -fWidth;
        const double fHeight = aFormat.mfHeight;
        if (rText.mnVertical == 1) // bottom: below the baseline is the descent
            fY = fHeight / 3;
        else if (rText.mnVertical == 2 || (rText.mnVertical == 0 && rText.mnHorizontal == 4))
            fY = -fHeight / 2;
        else if (rText.mnVertical == 3)
            fY = -fHeight;
    }

    const DXFTransform aTextTransform(DXFTransform(1.0, 1.0, 1.0, fAngle, aOrigin), rTransform);
    OutTextRun(rE, aTextTransform, fX, fY, aText, aFormat, fWidth);
}

vcl::Font DXF2GDIMetaFile::MakeFont(const DXFTextFormat & rFormat, double fSize)
{
    vcl::Font aFont;
    if (!rFormat.maFontName.isEmpty())
        aFont.SetFamilyName(rFormat.maFontName);
    aFont.SetFamily(FAMILY_SWISS);
    aFont.SetFontSize(Size(0, basegfx::fround(fSize)));
    aFont.SetWeight(rFormat.mbBold ? WEIGHT_BOLD : WEIGHT_NORMAL);
    // there is no slant angle, an oblique text is italic
    aFont.SetItalic(rFormat.mbItalic || std::fabs(rFormat.mfOblique) > 5 ? ITALIC_NORMAL
                                                                          : ITALIC_NONE);
    aFont.SetUnderline(rFormat.mbUnderline ? LINESTYLE_SINGLE : LINESTYLE_NONE);
    aFont.SetOverline(rFormat.mbOverline ? LINESTYLE_SINGLE : LINESTYLE_NONE);
    aFont.SetStrikeout(rFormat.mbStrikeout ? STRIKEOUT_SINGLE : STRIKEOUT_NONE);
    aFont.SetAlignment(ALIGN_BASELINE);
    aFont.SetTransparent(true);
    return aFont;
}

double DXF2GDIMetaFile::GetCapHeightRatio(const DXFTextFormat & rFormat)
{
    const std::pair<OUString, bool> aKey(rFormat.maFontName, rFormat.mbBold);
    std::map<std::pair<OUString, bool>, double>::const_iterator it = maCapHeightRatios.find(aKey);
    if (it != maCapHeightRatios.end())
        return it->second;

    // the height of the text is the height of its capital letters
    double fRatio = 0.7;
    mpMeasureDev->SetFont(MakeFont(rFormat, 1000));
    tools::Rectangle aRect;
    if (mpMeasureDev->GetTextBoundRect(aRect, u"H"_ustr) && aRect.GetHeight() > 100
        && aRect.GetHeight() <= 1000)
        fRatio = aRect.GetHeight() / 1000.0;
    maCapHeightRatios[aKey] = fRatio;
    return fRatio;
}

double DXF2GDIMetaFile::MeasureText(std::u16string_view aText, const DXFTextFormat & rFormat)
{
    if (aText.empty())
        return 0;
    mpMeasureDev->SetFont(MakeFont(rFormat, 1000));
    const double fSize = rFormat.mfHeight / GetCapHeightRatio(rFormat);
    return mpMeasureDev->GetTextWidth(OUString(aText)) / 1000.0 * fSize * rFormat.mfWidthFactor;
}

void DXF2GDIMetaFile::OutTextRun(const DXFBasicEntity & rE, const DXFTransform & rTextTransform,
                                 double fX, double fY, const OUString & rText,
                                 const DXFTextFormat & rFormat, double fWidth)
{
    if (rText.isEmpty() || !(rFormat.mfHeight > 0))
        return;
    std::optional<Color> oColor = rFormat.mnColorIndex >= 0 || rFormat.mnTrueColor >= 0
                                      ? ResolveColor(rE, rFormat.mnColorIndex, rFormat.mnTrueColor)
                                      : GetEntityColor(rE);
    if (!oColor || !AddWork(rText.getLength()))
        return;
    // the box of the text, one absurd text must not spoil the extents of the drawing
    const double fHeight = rFormat.mfHeight;
    basegfx::B2DRange aBox(Project(rTextTransform, DXFVector(fX, fY - fHeight / 3, 0)));
    aBox.expand(Project(rTextTransform, DXFVector(fX + fWidth, fY - fHeight / 3, 0)));
    aBox.expand(Project(rTextTransform, DXFVector(fX + fWidth, fY + fHeight, 0)));
    aBox.expand(Project(rTextTransform, DXFVector(fX, fY + fHeight, 0)));
    if (!IsSane(aBox))
        return;

    if (mbMeasure)
    {
        maExtents.expand(aBox);
        return;
    }

    const basegfx::B2DPoint aOrigin
        = maDeviceTransform * Project(rTextTransform, DXFVector(fX, fY, 0));
    const basegfx::B2DVector aBaseline(
        maDeviceTransform * Project(rTextTransform, DXFVector(fX + 1, fY, 0)) - aOrigin);
    const basegfx::B2DVector aUp(
        maDeviceTransform * Project(rTextTransform, DXFVector(fX, fY + 1, 0)) - aOrigin);

    const double fSize = rFormat.mfHeight / GetCapHeightRatio(rFormat) * aUp.getLength();
    if (!(fSize >= 1.0) || fSize > SAL_MAX_INT32)
        return;
    // the metafile y axis points down, the orientation is counter-clockwise
    const double fAngle = basegfx::rad2deg(atan2(-aBaseline.getY(), aBaseline.getX()));
    sal_Int32 nOrientation = basegfx::fround(fAngle * 10.0) % 3600;
    if (nOrientation < 0)
        nOrientation += 3600;

    vcl::Font aFont = MakeFont(rFormat, fSize);
    aFont.SetColor(EnsureContrast(*oColor));
    aFont.SetOrientation(Degree10(static_cast<sal_Int16>(nOrientation)));
    // the width factor of the text and the stretching of the text space
    const double fStretch
        = rFormat.mfWidthFactor * aBaseline.getLength() / aUp.getLength();
    // squeezed to nothing, as by an insert with a zero scale
    if (!(fStretch > 0.001))
        return;
    if (std::fabs(fStretch - 1.0) > 0.01)
    {
        mpMeasureDev->SetFont(aFont);
        const tools::Long nWidth = mpMeasureDev->GetFontMetric().GetAverageFontWidth();
        if (nWidth > 0)
            aFont.SetAverageFontWidth(basegfx::fround(nWidth * fStretch));
    }
    if (aActFont != aFont)
    {
        aActFont = aFont;
        pVirDev->SetFont(aActFont);
    }
    pVirDev->DrawText(Point(basegfx::fround<tools::Long>(aOrigin.getX()),
                            basegfx::fround<tools::Long>(aOrigin.getY())),
                      rText);
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
    OutThickness(basegfx::B2DPolygon{ aPoint }, rTransform, rE.fThickness, false);

    // $PDMODE: 0 dot, 1 nothing, 2 plus, 3 cross, 4 tick up, +32 circle, +64 square
    const tools::Long nMode = pDXF->mnPointMode;
    const tools::Long nShape = nMode & 7;
    const bool bCircle = (nMode & 32) != 0;
    const bool bSquare = (nMode & 64) != 0;
    // the definition points of dimensions are dots
    if (mbMeasure || mnDimensionDepth > 0 || (nShape == 0 && !bCircle && !bSquare))
    {
        OutPoint(aPoint);
        return;
    }
    if (!maExtents.isInside(aPoint) || (nShape == 1 && !bCircle && !bSquare))
        return;

    // $PDSIZE: in drawing units if positive, otherwise a percentage of the drawing height
    const double fSize = pDXF->mfPointSize > 0
                             ? pDXF->mfPointSize * GetDeviceScale(rTransform) / mfDeviceScale
                             : (pDXF->mfPointSize < 0 ? -pDXF->mfPointSize : 5.0) / 100.0
                                   * maExtents.getHeight();
    const double fR = fSize / 2;
    const LineInfo aLineInfo;
    const double fX = aPoint.getX();
    const double fY = aPoint.getY();
    if (nShape == 0)
        OutPoint(aPoint);
    else if (nShape == 2)
    {
        OutPolyLine(basegfx::B2DPolygon{ { fX - fR, fY }, { fX + fR, fY } }, aLineInfo);
        OutPolyLine(basegfx::B2DPolygon{ { fX, fY - fR }, { fX, fY + fR } }, aLineInfo);
    }
    else if (nShape == 3)
    {
        OutPolyLine(basegfx::B2DPolygon{ { fX - fR, fY - fR }, { fX + fR, fY + fR } }, aLineInfo);
        OutPolyLine(basegfx::B2DPolygon{ { fX - fR, fY + fR }, { fX + fR, fY - fR } }, aLineInfo);
    }
    else if (nShape == 4)
        OutPolyLine(basegfx::B2DPolygon{ { fX, fY }, { fX, fY + fR } }, aLineInfo);
    if (bCircle)
        OutPolyLine(basegfx::utils::createPolygonFromCircle(aPoint, fR), aLineInfo);
    if (bSquare)
        OutPolyLine(basegfx::utils::createPolygonFromRect(
                        basegfx::B2DRange(fX - fR, fY - fR, fX + fR, fY + fR)),
                    aLineInfo);
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
    DrawSingleLineText(rE,
                       { rE.m_sText, rE.m_sStyle, rE.aP0, rE.aAlign, rE.bHasAlign, rE.fHeight,
                         rE.fXScale, rE.fRotAngle, rE.fOblAngle, rE.nHorzJust, rE.nVertJust },
                       rTransform);
}


void DXF2GDIMetaFile::DrawBlock(const DXFBlock & rBlock, const DXFBasicEntity & rE,
                                const DXFTransform & rTransform)
{
    const DXFLayer * pLayer = GetEntityLayer(rE);
    if (rE.bInvisible || (pLayer != nullptr && (pLayer->nFlags & 1) != 0))
        return;
    // the content of an external reference is not in this file
    if ((rBlock.nFlags & 4) != 0 || mnInsertDepth >= MAX_INSERT_DEPTH)
        return;
    ++mnInsertDepth;

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
    --mnInsertDepth;
}


void DXF2GDIMetaFile::DrawInsertEntity(const DXFInsertEntity & rE, const DXFTransform & rTransform)
{
    const DXFBlock * pB;
    pB=pDXF->aBlocks.Search(rE.m_sName);
    if (pB==nullptr)
        return;

    // an array of copies, the spacing is rotated with the insert but not scaled
    const tools::Long nColumns = std::clamp<tools::Long>(rE.nColCount, 1, MAX_ARRAY_COUNT);
    const tools::Long nRows = std::clamp<tools::Long>(rE.nRowCount, 1, MAX_ARRAY_COUNT);
    const double fAngle = basegfx::deg2rad(rE.fRotAngle);
    const DXFVector aColumnStep(cos(fAngle) * rE.fColSpace, sin(fAngle) * rE.fColSpace, 0);
    const DXFVector aRowStep(-sin(fAngle) * rE.fRowSpace, cos(fAngle) * rE.fRowSpace, 0);

    const DXFTransform aToBase(1.0, 1.0, 1.0, DXFVector(0.0, 0.0, 0.0) - pB->aBasePoint);
    for (tools::Long nRow = 0; nRow < nRows && !IsOverBudget(); ++nRow)
    {
        for (tools::Long nColumn = 0; nColumn < nColumns && AddVisit(); ++nColumn)
        {
            const DXFVector aPosition
                = rE.aP0 + aColumnStep * static_cast<double>(nColumn)
                  + aRowStep * static_cast<double>(nRow);
            const DXFTransform aT(
                DXFTransform(aToBase, DXFTransform(rE.fXScale, rE.fYScale, rE.fZScale,
                                                   rE.fRotAngle, aPosition)),
                rTransform);
            DrawBlock(*pB, rE, aT);
        }
    }
}


void DXF2GDIMetaFile::DrawAttribEntity(const DXFAttribEntity & rE, const DXFTransform & rTransform)
{
    if ((rE.nAttrFlags&1)!=0)
        return;

    DrawSingleLineText(rE,
                       { rE.m_sText, rE.m_sStyle, rE.aP0, rE.aAlign, rE.bHasAlign, rE.fHeight,
                         rE.fXScale, rE.fRotAngle, rE.fOblAngle, rE.nHorzJust, rE.nVertJust },
                       rTransform);
}


double DXF2GDIMetaFile::GetDeviceScale(const DXFTransform & rTransform) const
{
    DXFVector aX, aY;
    rTransform.TransDir(DXFVector(1, 0, 0), aX);
    rTransform.TransDir(DXFVector(0, 1, 0), aY);
    return (std::hypot(aX.fx, aX.fy) + std::hypot(aY.fx, aY.fy)) / 2 * mfDeviceScale;
}

void DXF2GDIMetaFile::Draw2DPolyLine(const DXFBasicEntity & rE, const std::vector<DXFPathVertex> & rVertices,
                                     bool bClosed, double fElevation, const DXFTransform & rTransform)
{
    if (rVertices.empty())
        return;

    const basegfx::B2DPolygon aPolygon(
        Project(rTransform, dxf::createPolyline(rVertices, bClosed), fElevation));

    bool bWide = false;
    bool bConstantWidth = true;
    const size_t nSegments = bClosed ? rVertices.size() : rVertices.size() - 1;
    for (size_t i = 0; i < nSegments; ++i)
    {
        const DXFPathVertex& rVertex = rVertices[i];
        if (rVertex.mfStartWidth > 0 || rVertex.mfEndWidth > 0)
            bWide = true;
        if (rVertex.mfStartWidth != rVertices[0].mfStartWidth
            || rVertex.mfEndWidth != rVertices[0].mfStartWidth)
            bConstantWidth = false;
    }

    if (bWide && !bConstantWidth)
    {
        // each segment is filled between its start and end widths
        if (!SetAreaAttribute(rE))
            return;
        // one by one, the outlines of neighbouring segments overlap
        for (const basegfx::B2DPolygon& rOutline : dxf::createWidePolyline(rVertices, bClosed))
            OutPolygon(basegfx::B2DPolyPolygon(Project(rTransform, rOutline, fElevation)));
    }

    if (!SetLineAttribute(rE))
        return;
    LineInfo aLineInfo = GetEntityLineInfo(rE, rTransform);
    if (bWide && bConstantWidth && mbMeasure)
    {
        // the width reaches beyond the centre line
        basegfx::B2DRange aRange = aPolygon.getB2DRange();
        aRange.grow(rVertices[0].mfStartWidth * GetDeviceScale(rTransform) / mfDeviceScale / 2);
        if (IsSane(aRange))
            maExtents.expand(aRange);
    }
    if (bWide && bConstantWidth && !mbMeasure)
    {
        aLineInfo.SetWidth(rVertices[0].mfStartWidth * GetDeviceScale(rTransform));
        aLineInfo.SetLineJoin(basegfx::B2DLineJoin::Miter);
    }
    OutPolyLine(aPolygon, aLineInfo);
    OutThickness(aPolygon, rTransform, rE.fThickness, false);
}

void DXF2GDIMetaFile::DrawPolyLineEntity(const DXFPolyLineEntity & rE, const DXFTransform & rTransform)
{
    std::vector<const DXFVertexEntity*> aVertices;
    for (const DXFBasicEntity* pBE = rE.pSucc; pBE != nullptr && pBE->eType == DXF_VERTEX;
         pBE = pBE->pSucc)
        aVertices.push_back(static_cast<const DXFVertexEntity*>(pBE));
    if (aVertices.empty())
        return;

    if (rE.nFlags & 64)
    {
        // polyface mesh: the vertices, then the faces referring to them; a negative index
        // hides the edge starting at that vertex
        if (!SetLineAttribute(rE))
            return;
        std::vector<basegfx::B2DPoint> aPoints;
        for (const DXFVertexEntity* pVertex : aVertices)
            if ((pVertex->nFlags & 128) == 0 || (pVertex->nFlags & 64) != 0)
                aPoints.push_back(Project(rTransform, pVertex->aP0));
        for (const DXFVertexEntity* pVertex : aVertices)
        {
            if ((pVertex->nFlags & 128) == 0 || (pVertex->nFlags & 64) != 0)
                continue;
            std::vector<tools::Long> aIndices;
            for (tools::Long nIndex : pVertex->nFaceIndex)
                if (nIndex != 0)
                    aIndices.push_back(nIndex);
            for (size_t i = 0; i < aIndices.size(); ++i)
            {
                const tools::Long nFrom = aIndices[i];
                const tools::Long nTo = std::abs(aIndices[(i + 1) % aIndices.size()]);
                if (nFrom < 0 || o3tl::make_unsigned(nFrom) > aPoints.size()
                    || o3tl::make_unsigned(nTo) > aPoints.size())
                    continue;
                basegfx::B2DPolygon aEdge;
                aEdge.append(aPoints[nFrom - 1]);
                aEdge.append(aPoints[nTo - 1]);
                OutPolyLine(aEdge, LineInfo());
            }
        }
        return;
    }

    if (rE.nFlags & 16)
    {
        // polygon mesh: M rows of N vertices, the smoothed surface replaces the control points
        if (!SetLineAttribute(rE))
            return;
        std::vector<basegfx::B2DPoint> aPoints;
        const bool bSmooth = rE.nCSSType != 0 && rE.nMDensity > 1 && rE.nNDensity > 1;
        for (const DXFVertexEntity* pVertex : aVertices)
            if (!bSmooth || (pVertex->nFlags & 16) == 0)
                aPoints.push_back(Project(rTransform, pVertex->aP0));
        // the counts are checked against the points before they are multiplied
        const sal_uInt64 nPoints = aPoints.size();
        size_t nM = 0, nN = 0;
        if (bSmooth && IsGrid(rE.nMDensity, rE.nNDensity, nPoints)
            && o3tl::make_unsigned(rE.nMDensity) * o3tl::make_unsigned(rE.nNDensity) == nPoints)
        {
            nM = rE.nMDensity;
            nN = rE.nNDensity;
        }
        else if (IsGrid(rE.nMeshMCount, rE.nMeshNCount, nPoints))
        {
            nM = rE.nMeshMCount;
            nN = rE.nMeshNCount;
        }
        else
            return;
        for (size_t m = 0; m < nM; ++m)
        {
            basegfx::B2DPolygon aRow;
            for (size_t n = 0; n < nN; ++n)
                aRow.append(aPoints[m * nN + n]);
            aRow.setClosed((rE.nFlags & 32) != 0);
            OutPolyLine(aRow, LineInfo());
        }
        for (size_t n = 0; n < nN; ++n)
        {
            basegfx::B2DPolygon aColumn;
            for (size_t m = 0; m < nM; ++m)
                aColumn.append(aPoints[m * nN + n]);
            aColumn.setClosed((rE.nFlags & 1) != 0);
            OutPolyLine(aColumn, LineInfo());
        }
        return;
    }

    // a fitted polyline is drawn through its fit vertices, not its frame
    const bool bSplineFit = (rE.nFlags & 4) != 0;
    if (rE.nFlags & 8)
    {
        // 3D polyline
        if (!SetLineAttribute(rE))
            return;
        basegfx::B2DPolygon aPolygon;
        for (const DXFVertexEntity* pVertex : aVertices)
            if (!bSplineFit || (pVertex->nFlags & 16) == 0)
                aPolygon.append(Project(rTransform, pVertex->aP0));
        aPolygon.setClosed((rE.nFlags & 1) != 0);
        OutPolyLine(aPolygon, GetEntityLineInfo(rE, rTransform));
        return;
    }

    std::vector<DXFPathVertex> aPath;
    for (const DXFVertexEntity* pVertex : aVertices)
    {
        if (bSplineFit && (pVertex->nFlags & 16) != 0)
            continue;
        DXFPathVertex aVertex;
        aVertex.maPoint = basegfx::B2DPoint(pVertex->aP0.fx, pVertex->aP0.fy);
        aVertex.mfBulge = pVertex->fBulge;
        aVertex.mfStartWidth = pVertex->fSWidth >= 0 ? pVertex->fSWidth : rE.fSWidth;
        aVertex.mfEndWidth = pVertex->fEWidth >= 0 ? pVertex->fEWidth : rE.fEWidth;
        aPath.push_back(aVertex);
    }
    Draw2DPolyLine(rE, aPath, (rE.nFlags & 1) != 0, rE.fElevation, rTransform);
}

void DXF2GDIMetaFile::DrawLWPolyLineEntity(const DXFLWPolyLineEntity & rE, const DXFTransform & rTransform )
{
    std::vector<DXFPathVertex> aPath(rE.aVertices);
    for (DXFPathVertex& rVertex : aPath)
    {
        if (rVertex.mfStartWidth < 0)
            rVertex.mfStartWidth = rE.fConstantWidth;
        if (rVertex.mfEndWidth < 0)
            rVertex.mfEndWidth = rE.fConstantWidth;
    }
    Draw2DPolyLine(rE, aPath, (rE.nFlags & 1) != 0, rE.fElevation, rTransform);
}

void DXF2GDIMetaFile::SetColors(const Color & rLineColor, const Color & rFillColor)
{
    if (aActLineColor != rLineColor)
    {
        aActLineColor = rLineColor;
        pVirDev->SetLineColor(aActLineColor);
    }
    if (aActFillColor != rFillColor)
    {
        aActFillColor = rFillColor;
        if (rFillColor == COL_TRANSPARENT)
            pVirDev->SetFillColor();
        else
            pVirDev->SetFillColor(aActFillColor);
    }
}

void DXF2GDIMetaFile::DrawHatchEntity(const DXFHatchEntity & rE, const DXFTransform & rTransform )
{
    if (rE.aBoundaries.count() == 0)
        return;
    std::optional<Color> oColor = GetEntityColor(rE);
    if (!oColor)
        return;

    basegfx::B2DPolyPolygon aArea;
    for (const basegfx::B2DPolygon& rBoundary : rE.aBoundaries)
        aArea.append(Project(rTransform, rBoundary, rE.fElevation));
    if (mbMeasure)
    {
        OutPolygon(aArea);
        return;
    }

    if (rE.bGradient && !rE.aGradientColors.empty())
    {
        const Color aStart(ColorTransparency, rE.aGradientColors[0]);
        const Color aEnd(ColorTransparency, rE.aGradientColors.size() > 1 ? rE.aGradientColors[1]
                                                                          : rE.aGradientColors[0]);
        if (aStart != aEnd && AddWork(PointCount(aArea)) && IsSane(aArea.getB2DRange()))
        {
            // The colours change along the angle, a linear gradient changes across its angle.
            // The angle is counter-clockwise in drawing coordinates, as the one of the metafile
            // is on the screen.
            DXFVector aDirection;
            rTransform.TransDir(
                DXFVector(cos(rE.fGradientAngle), sin(rE.fGradientAngle), 0), aDirection);
            const double fAngle = basegfx::rad2deg(atan2(aDirection.fy, aDirection.fx)) + 90.0;
            sal_Int32 nAngle = basegfx::fround(fAngle * 10.0) % 3600;
            if (nAngle < 0)
                nAngle += 3600;
            Gradient aGradient(css::awt::GradientStyle_LINEAR, aStart, aEnd);
            aGradient.SetAngle(Degree10(static_cast<sal_Int16>(nAngle)));
            basegfx::B2DPolyPolygon aDevice = ToolsPolyPolygon(aArea);
            aDevice.transform(maDeviceTransform);
            if (aDevice.count() > 0)
                pVirDev->DrawGradient(tools::PolyPolygon(aDevice), aGradient);
            return;
        }
        SetColors(aStart, aStart);
        OutPolygon(aArea);
        return;
    }

    // a filled area keeps its colour, it is readable when light
    if (rE.bSolid || rE.aPatternLines.empty())
    {
        SetColors(*oColor, *oColor);
        OutPolygon(aArea);
        return;
    }

    DrawHatchPattern(rE, aArea, EnsureContrast(*oColor), rTransform);
}

void DXF2GDIMetaFile::DrawHatchPattern(const DXFHatchEntity & rE, const basegfx::B2DPolyPolygon & rArea,
                                       const Color & rColor, const DXFTransform & rTransform)
{
    // the line families in drawing coordinates
    struct Family
    {
        basegfx::B2DPoint maBase;
        basegfx::B2DVector maDirection; // unit
        basegfx::B2DVector maOffset;
        std::vector<double> maDashes;
    };
    std::vector<Family> aFamilies;
    bool bDashed = false;
    const double fLengthScale = GetDeviceScale(rTransform) / mfDeviceScale;
    for (const DXFHatchPatternLine& rLine : rE.aPatternLines)
    {
        const double fAngle = basegfx::deg2rad(rLine.fAngle);
        DXFVector aDirection, aOffset;
        rTransform.TransDir(DXFVector(cos(fAngle), sin(fAngle), 0), aDirection);
        rTransform.TransDir(DXFVector(rLine.aOffset.getX(), rLine.aOffset.getY(), 0), aOffset);
        Family aFamily;
        aFamily.maBase = Project(rTransform, DXFVector(rLine.aBase.getX(), rLine.aBase.getY(), rE.fElevation));
        aFamily.maDirection = basegfx::B2DVector(aDirection.fx, aDirection.fy);
        aFamily.maDirection.normalize();
        aFamily.maOffset = basegfx::B2DVector(aOffset.fx, aOffset.fy);
        for (double fDash : rLine.aDashes)
            aFamily.maDashes.push_back(fDash * fLengthScale);
        if (!aFamily.maDashes.empty())
            bDashed = true;
        if (!aFamily.maDirection.equalZero())
            aFamilies.push_back(aFamily);
    }
    if (aFamilies.empty())
        return;

    if (!AddWork(PointCount(rArea)) || !IsSane(rArea.getB2DRange()))
        return;

    // the distance between the lines of a family
    std::vector<double> aSpacings;
    for (const Family& rFamily : aFamilies)
    {
        const basegfx::B2DVector aNormal(-rFamily.maDirection.getY(), rFamily.maDirection.getX());
        aSpacings.push_back(std::fabs(rFamily.maOffset.scalar(aNormal)));
    }

    // Continuous lines in one direction, or in two perpendicular ones with the same spacing, are
    // a hatch of the metafile, which stays a hatch in a drawing.
    const bool bPerpendicular
        = aFamilies.size() == 2
          && std::fabs(aFamilies[0].maDirection.scalar(aFamilies[1].maDirection)) < 1e-6
          && std::fabs(aSpacings[0] - aSpacings[1]) <= 1e-6 * aSpacings[0];
    if (!bDashed && (aFamilies.size() == 1 || bPerpendicular))
    {
        const double fDistance = aSpacings[0] * mfDeviceScale;
        basegfx::B2DPolyPolygon aDeviceArea = ToolsPolyPolygon(rArea);
        aDeviceArea.transform(maDeviceTransform);
        if (fDistance >= 1.0 && fDistance < DEVICE_SIZE * 10 && aDeviceArea.count() > 0)
        {
            // the angle is counter-clockwise in drawing coordinates, as the one of the metafile
            // is on the screen
            const basegfx::B2DVector aDirection = aFamilies[0].maDirection;
            const double fAngle = basegfx::rad2deg(atan2(aDirection.getY(), aDirection.getX()));
            sal_Int32 nAngle = basegfx::fround(fAngle * 10.0) % 1800;
            if (nAngle < 0)
                nAngle += 1800;
            pVirDev->DrawHatch(tools::PolyPolygon(aDeviceArea),
                               Hatch(aFamilies.size() == 1 ? HatchStyle::Single : HatchStyle::Double,
                                     rColor, basegfx::fround<tools::Long>(fDistance),
                                     Degree10(static_cast<sal_Int16>(nAngle))));
            return;
        }
    }

    // Otherwise the lines and their dashes are cut to the area. A pattern too dense to be seen
    // is a tone instead.
    const basegfx::B2DRange aRange = rArea.getB2DRange();
    const double fMinSpacing = std::max(aRange.getWidth(), aRange.getHeight()) / 2000.0;
    constexpr size_t nMaxSegments = 100000;
    basegfx::B2DPolyPolygon aLines;
    bool bTooDense = false;
    for (size_t nFamily = 0; nFamily < aFamilies.size() && !bTooDense; ++nFamily)
    {
        const Family& rFamily = aFamilies[nFamily];
        const double fSpacing = aSpacings[nFamily];
        if (!(fSpacing > fMinSpacing))
        {
            bTooDense = true;
            break;
        }
        const basegfx::B2DVector aNormal(-rFamily.maDirection.getY(), rFamily.maDirection.getX());
        const double fNormalStep = rFamily.maOffset.scalar(aNormal);
        double fMinNormal = std::numeric_limits<double>::max(), fMaxNormal = -fMinNormal;
        for (const basegfx::B2DPoint& rCorner : { aRange.getMinimum(), aRange.getMaximum(),
                                                  basegfx::B2DPoint(aRange.getMinX(), aRange.getMaxY()),
                                                  basegfx::B2DPoint(aRange.getMaxX(), aRange.getMinY()) })
        {
            const double fNormal = basegfx::B2DVector(rCorner - rFamily.maBase).scalar(aNormal);
            fMinNormal = std::min(fMinNormal, fNormal);
            fMaxNormal = std::max(fMaxNormal, fNormal);
        }
        double fFirst = fMinNormal / fNormalStep;
        double fLast = fMaxNormal / fNormalStep;
        if (fFirst > fLast)
            std::swap(fFirst, fLast);
        // the line numbers are converted only when there are not too many of them
        if (!std::isfinite(fFirst) || !std::isfinite(fLast) || fLast - fFirst > nMaxSegments
            || std::fabs(fFirst) > 1e15)
        {
            bTooDense = true;
            break;
        }
        const sal_Int64 nFirst = static_cast<sal_Int64>(std::floor(fFirst));
        const sal_Int64 nLast = static_cast<sal_Int64>(std::ceil(fLast));

        double fPeriod = 0;
        for (double fDash : rFamily.maDashes)
            fPeriod += std::fabs(fDash);
        const bool bContinuous = rFamily.maDashes.empty() || !(fPeriod > fMinSpacing / 10);

        for (sal_Int64 nLine = nFirst; nLine <= nLast && !bTooDense; ++nLine)
        {
            basegfx::B2DPoint aOrigin = rFamily.maBase + rFamily.maOffset * static_cast<double>(nLine);
            double fStart = std::numeric_limits<double>::max(), fEnd = -fStart;
            for (const basegfx::B2DPoint& rCorner : { aRange.getMinimum(), aRange.getMaximum(),
                                                      basegfx::B2DPoint(aRange.getMinX(), aRange.getMaxY()),
                                                      basegfx::B2DPoint(aRange.getMaxX(), aRange.getMinY()) })
            {
                const double fAlong = basegfx::B2DVector(rCorner - aOrigin).scalar(rFamily.maDirection);
                fStart = std::min(fStart, fAlong);
                fEnd = std::max(fEnd, fAlong);
            }
            if (bContinuous)
            {
                aLines.append(basegfx::B2DPolygon{ aOrigin + rFamily.maDirection * fStart,
                                                   aOrigin + rFamily.maDirection * fEnd });
                continue;
            }
            // The dashes start at the origin of the line: move the origin by whole periods next
            // to the area, the positions stay small and precise.
            const double fShift = std::floor(fStart / fPeriod) * fPeriod;
            aOrigin += rFamily.maDirection * fShift;
            fStart -= fShift;
            fEnd -= fShift;
            if (!((fEnd - fStart) / fPeriod * rFamily.maDashes.size() < nMaxSegments))
            {
                bTooDense = true;
                break;
            }
            double fPosition = 0;
            while (fPosition < fEnd && !bTooDense)
            {
                if (fPosition + fPeriod == fPosition)
                {
                    bTooDense = true;
                    break;
                }
                for (double fDash : rFamily.maDashes)
                {
                    const double fLength = std::fabs(fDash);
                    if (fDash >= 0)
                    {
                        // a dot is a very short dash
                        const double fFrom = std::max(fPosition, fStart);
                        const double fTo = std::min(fPosition + std::max(fLength, fMinSpacing / 20), fEnd);
                        if (fTo > fFrom)
                            aLines.append(basegfx::B2DPolygon{ aOrigin + rFamily.maDirection * fFrom,
                                                               aOrigin + rFamily.maDirection * fTo });
                    }
                    fPosition += fLength;
                }
                if (aLines.count() > nMaxSegments)
                    bTooDense = true;
            }
            if (aLines.count() > nMaxSegments)
                bTooDense = true;
        }
    }

    // cutting the lines takes time with each line and each edge of the area
    const basegfx::B2DPolyPolygon aClip = basegfx::utils::adaptiveSubdivideByAngle(rArea);
    if (static_cast<double>(aLines.count()) * PointCount(aClip) > 20000000.0)
        bTooDense = true;

    if (bTooDense)
    {
        const Color aTone(sal_uInt8((rColor.GetRed() + 255) / 2), sal_uInt8((rColor.GetGreen() + 255) / 2),
                          sal_uInt8((rColor.GetBlue() + 255) / 2));
        SetColors(aTone, aTone);
        OutPolygon(rArea);
        return;
    }

    const basegfx::B2DPolyPolygon aClipped
        = basegfx::utils::clipPolyPolygonOnPolyPolygon(aLines, aClip, true, true);
    if (!AddWork(aClipped.count() * 2))
        return;
    SetColors(rColor, COL_TRANSPARENT);
    // few actions for all the lines: a two-point polygon without a fill is its line
    basegfx::B2DPolyPolygon aDevice(aClipped);
    aDevice.transform(maDeviceTransform);
    constexpr sal_uInt32 nChunk = 16000;
    for (sal_uInt32 nStart = 0; nStart < aDevice.count(); nStart += nChunk)
    {
        basegfx::B2DPolyPolygon aPart;
        for (sal_uInt32 i = nStart; i < std::min(nStart + nChunk, aDevice.count()); ++i)
            aPart.append(aDevice.getB2DPolygon(i));
        pVirDev->DrawPolyPolygon(tools::PolyPolygon(aPart));
    }
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

void DXF2GDIMetaFile::DrawEllipseEntity(const DXFEllipseEntity & rE, const DXFTransform & rTransform)
{
    if (!SetLineAttribute(rE))
        return;

    // the minor axis follows the extrusion direction: seen from below the ellipse is mirrored
    const DXFVector aMinor3D = (rE.aExtrusion.Unit() * rE.aMajor) * rE.fRatio;
    DXFVector aMajor, aMinor;
    rTransform.TransDir(rE.aMajor, aMajor);
    rTransform.TransDir(aMinor3D, aMinor);

    double fSweep = std::fmod(rE.fEnd - rE.fStart, 2 * M_PI);
    if (fSweep <= 1e-12)
        fSweep += 2 * M_PI;
    const bool bFull = std::fabs(fSweep - 2 * M_PI) < 1e-9;

    basegfx::B2DPolygon aPolygon;
    dxf::appendEllipseArc(aPolygon, Project(rTransform, rE.aCenter),
                          basegfx::B2DVector(aMajor.fx, aMajor.fy),
                          basegfx::B2DVector(aMinor.fx, aMinor.fy), rE.fStart, fSweep, true);
    if (bFull)
        dxf::closePolygon(aPolygon);
    OutPolyLine(aPolygon, GetEntityLineInfo(rE, rTransform));
}

void DXF2GDIMetaFile::DrawSplineEntity(const DXFSplineEntity & rE, const DXFTransform & rTransform)
{
    if (!SetLineAttribute(rE))
        return;

    // the projection is affine, the curve of the projected control points is the projected curve
    std::vector<basegfx::B2DPoint> aControlPoints, aFitPoints;
    for (const DXFVector& rPoint : rE.aControlPoints)
        aControlPoints.push_back(Project(rTransform, rPoint));
    for (const DXFVector& rPoint : rE.aFitPoints)
        aFitPoints.push_back(Project(rTransform, rPoint));
    basegfx::B2DPolygon aPolygon
        = dxf::createSpline(rE.nDegree, rE.aKnots, aControlPoints, rE.aWeights, aFitPoints);
    if (aPolygon.count() < 2)
        return;
    if ((rE.nFlags & 1) != 0)
    {
        if (aPolygon.getB2DPoint(0).equal(aPolygon.getB2DPoint(aPolygon.count() - 1)))
            dxf::closePolygon(aPolygon);
        else
            aPolygon.setClosed(true);
    }
    OutPolyLine(aPolygon, GetEntityLineInfo(rE, rTransform));
}

namespace
{
struct MTextPiece
{
    const DXFTextRun* mpRun;
    OUString maText;
    double mfWidth;
};

struct MTextLine
{
    std::vector<MTextPiece> maPieces;
    double mfWidth = 0;
    double mfHeight = 0;
};

std::u16string_view trimTrailingSpaces(std::u16string_view aText)
{
    while (!aText.empty() && aText.back() == ' ')
        aText.remove_suffix(1);
    return aText;
}
}

void DXF2GDIMetaFile::DrawMTextEntity(const DXFMTextEntity & rE, const DXFTransform & rTransform)
{
    if (!GetEntityColor(rE))
        return;

    DXFTextFormat aBase = GetStyleFormat(rE.m_sStyle);
    if (rE.fHeight > 0)
        aBase.mfHeight = rE.fHeight;
    const std::vector<DXFTextParagraph> aParagraphs
        = dxf::parseMText(pDXF->ToOUString(rE.m_sText, false), aBase,
                          pDXF->getSingleByteEncoding());

    // the text runs along x, in the plane of the extrusion direction
    const DXFVector aNormal = rE.aExtrusion.Unit();
    DXFVector aXAxis;
    if (rE.bHasXDirection && rE.aXDirection.Abs() > 0)
        aXAxis = rE.aXDirection.Unit();
    else
    {
        const double fAngle = basegfx::deg2rad(rE.fRotAngle);
        DXFTransform(rE.aExtrusion).TransDir(DXFVector(cos(fAngle), sin(fAngle), 0), aXAxis);
    }
    const DXFVector aYAxis = (aNormal * aXAxis).Unit();
    const DXFTransform aTextTransform(DXFTransform(aXAxis, aYAxis, aNormal, rE.aP0), rTransform);

    // break the paragraphs into lines at the reference width
    const double fWrapWidth = rE.fReferenceWidth;
    std::vector<MTextLine> aLines;
    for (const DXFTextParagraph& rParagraph : aParagraphs)
    {
        MTextLine aLine;
        aLine.mfHeight = rParagraph.maRuns.empty() ? aBase.mfHeight : 0;
        for (const DXFTextRun& rRun : rParagraph.maRuns)
        {
            aLine.mfHeight = std::max(aLine.mfHeight, rRun.maFormat.mfHeight);
            std::vector<OUString> aWords;
            if (fWrapWidth > 0)
            {
                sal_Int32 nStart = 0;
                for (sal_Int32 i = 0; i < rRun.maText.getLength(); ++i)
                    if (rRun.maText[i] == ' ' && (i + 1 == rRun.maText.getLength() || rRun.maText[i + 1] != ' '))
                    {
                        aWords.push_back(rRun.maText.copy(nStart, i + 1 - nStart));
                        nStart = i + 1;
                    }
                if (nStart < rRun.maText.getLength())
                    aWords.push_back(rRun.maText.copy(nStart));
            }
            else
                aWords.push_back(rRun.maText);

            for (const OUString& rWord : aWords)
            {
                const double fWidth = MeasureText(rWord, rRun.maFormat);
                const std::u16string_view aTrimmed = trimTrailingSpaces(rWord);
                if (fWrapWidth > 0 && !aLine.maPieces.empty()
                    && aLine.mfWidth
                               + (aTrimmed.size() == o3tl::make_unsigned(rWord.getLength())
                                      ? fWidth
                                      : MeasureText(aTrimmed, rRun.maFormat))
                           > fWrapWidth)
                {
                    aLines.push_back(aLine);
                    aLine = MTextLine();
                    aLine.mfHeight = rRun.maFormat.mfHeight;
                }
                if (!aLine.maPieces.empty() && aLine.maPieces.back().mpRun == &rRun)
                {
                    aLine.maPieces.back().maText += rWord;
                    aLine.maPieces.back().mfWidth += fWidth;
                }
                else
                    aLine.maPieces.push_back({ &rRun, rWord, fWidth });
                aLine.mfWidth += fWidth;
            }
        }
        aLines.push_back(aLine);
    }
    if (aLines.empty())
        return;

    // trailing spaces do not count for the alignment
    for (MTextLine& rLine : aLines)
    {
        if (rLine.maPieces.empty())
            continue;
        const MTextPiece& rLast = rLine.maPieces.back();
        const std::u16string_view aTrimmed = trimTrailingSpaces(rLast.maText);
        if (aTrimmed.size() != o3tl::make_unsigned(rLast.maText.getLength()))
            rLine.mfWidth += MeasureText(aTrimmed, rLast.mpRun->maFormat) - rLast.mfWidth;
    }

    // the line pitch is 5/3 of the text height times the line spacing factor
    const double fSpacing = rE.fLineSpacing > 0 ? rE.fLineSpacing : 1.0;
    double fTotalHeight = aLines[0].mfHeight;
    for (size_t i = 1; i < aLines.size(); ++i)
        fTotalHeight += aLines[i].mfHeight * 5.0 / 3.0 * fSpacing;

    const tools::Long nAttachment = rE.nAttachment >= 1 && rE.nAttachment <= 9 ? rE.nAttachment : 1;
    const tools::Long nRow = (nAttachment - 1) / 3;
    const tools::Long nColumn = (nAttachment - 1) % 3;
    double fY = (nRow == 0 ? 0 : nRow == 1 ? fTotalHeight / 2 : fTotalHeight) - aLines[0].mfHeight;
    for (size_t i = 0; i < aLines.size(); ++i)
    {
        const MTextLine& rLine = aLines[i];
        if (i > 0)
            fY -= rLine.mfHeight * 5.0 / 3.0 * fSpacing;
        double fX = nColumn == 0 ? 0 : nColumn == 1 ? -rLine.mfWidth / 2 : -rLine.mfWidth;
        for (const MTextPiece& rPiece : rLine.maPieces)
        {
            OutTextRun(rE, aTextTransform, fX, fY, rPiece.maText, rPiece.mpRun->maFormat,
                       rPiece.mfWidth);
            fX += rPiece.mfWidth;
        }
    }
}

void DXF2GDIMetaFile::DrawLeaderEntity(const DXFLeaderEntity & rE, const DXFTransform & rTransform)
{
    if (rE.aVertices.size() < 2 || !SetLineAttribute(rE))
        return;

    std::vector<basegfx::B2DPoint> aPoints;
    for (const DXFVector& rVertex : rE.aVertices)
        aPoints.push_back(Project(rTransform, rVertex));
    basegfx::B2DPolygon aPath;
    if (rE.bSpline)
        aPath = dxf::createSpline(0, {}, {}, {}, aPoints);
    else
        for (const basegfx::B2DPoint& rPoint : aPoints)
            aPath.append(rPoint);
    OutPolyLine(aPath, GetEntityLineInfo(rE, rTransform));

    if (!rE.bArrow)
        return;
    // the arrowhead at the first vertex: DIMASZ long, a third of it wide, times DIMSCALE
    double fArrowSize = pDXF->mfDimArrowSize;
    double fScale = pDXF->mfDimScale;
    if (const DXFDimStyle * pDimStyle = pDXF->aTables.SearchDimStyle(rE.m_sDimStyle))
    {
        if (pDimStyle->mfArrowSize > 0)
            fArrowSize = pDimStyle->mfArrowSize;
        if (pDimStyle->mfScale > 0)
            fScale = pDimStyle->mfScale;
    }
    if (!(fScale > 0))
        fScale = 1.0;
    const double fLength = fArrowSize * fScale;
    const DXFVector aTip = rE.aVertices[0];
    const DXFVector aBack = rE.aVertices[1] - aTip;
    if (!(fLength > 0) || aBack.Abs() == 0)
        return;
    const DXFVector aDirection = aBack.Unit();
    const DXFVector aSide = (rE.aExtrusion.Unit() * aDirection) * (fLength / 6);
    const DXFVector aBase = aTip + aDirection * fLength;
    basegfx::B2DPolygon aArrow{ Project(rTransform, aTip), Project(rTransform, aBase + aSide),
                                Project(rTransform, aBase - aSide) };
    aArrow.setClosed(true);
    if (SetAreaAttribute(rE))
        OutPolygon(basegfx::B2DPolyPolygon(aArrow));
}

void DXF2GDIMetaFile::DrawImageEntity(const DXFImageEntity & rE, const DXFTransform & rTransform)
{
    std::optional<Color> oColor = GetEntityColor(rE);
    if (!oColor)
        return;

    // a point given in pixels from the top left corner of the image
    const double fWidth = rE.fWidth > 0 ? rE.fWidth : 1.0;
    const double fHeight = rE.fHeight > 0 ? rE.fHeight : 1.0;
    std::vector<basegfx::B2DPoint> aPixels;
    if (rE.nClipType == 2 && rE.aClipVertices.size() >= 3)
        aPixels = rE.aClipVertices;
    else if (rE.aClipVertices.size() == 2)
    {
        const basegfx::B2DPoint& rA = rE.aClipVertices[0];
        const basegfx::B2DPoint& rB = rE.aClipVertices[1];
        aPixels = { rA, { rB.getX(), rA.getY() }, rB, { rA.getX(), rB.getY() } };
    }
    else
        aPixels = { { -0.5, -0.5 }, { fWidth - 0.5, -0.5 }, { fWidth - 0.5, fHeight - 0.5 },
                    { -0.5, fHeight - 0.5 } };

    basegfx::B2DPolygon aBoundary;
    for (const basegfx::B2DPoint& rPixel : aPixels)
        aBoundary.append(Project(rTransform, rE.aInsert + rE.aU * (rPixel.getX() + 0.5)
                                                 + rE.aV * (fHeight - rPixel.getY() - 0.5)));
    aBoundary.setClosed(true);

    if (rE.bWipeout)
    {
        // covers what is drawn before it with the background
        SetColors(COL_WHITE, COL_WHITE);
        OutPolygon(basegfx::B2DPolyPolygon(aBoundary));
        return;
    }
    // the raster data is in another file, draw the frame
    if (SetLineAttribute(rE))
        OutPolyLine(aBoundary, LineInfo());
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
    ++mnDimensionDepth;
    DrawBlock(*pB, rE, aT);
    --mnDimensionDepth;
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

    while (pE!=nullptr && !IsOverBudget() && AddVisit()) {
        if (pE->nSpace==0) {
            // the coordinates of these are in the world coordinate system
            const bool bWorld
                = pE->eType == DXF_LINE || pE->eType == DXF_POINT || pE->eType == DXF_3DFACE
                  || pE->eType == DXF_ELLIPSE || pE->eType == DXF_SPLINE
                  || pE->eType == DXF_MTEXT || pE->eType == DXF_LEADER || pE->eType == DXF_IMAGE
                  || (pE->eType == DXF_POLYLINE
                      && (static_cast<const DXFPolyLineEntity*>(pE)->nFlags & (8 | 16 | 64)) != 0);
            if (bWorld || pE->aExtrusion.fz==1.0) {
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
            case DXF_ELLIPSE:
                DrawEllipseEntity(static_cast<const DXFEllipseEntity&>(*pE),*pT);
                break;
            case DXF_SPLINE:
                DrawSplineEntity(static_cast<const DXFSplineEntity&>(*pE),*pT);
                break;
            case DXF_MTEXT:
                DrawMTextEntity(static_cast<const DXFMTextEntity&>(*pE),*pT);
                break;
            case DXF_LEADER:
                DrawLeaderEntity(static_cast<const DXFLeaderEntity&>(*pE),*pT);
                break;
            case DXF_IMAGE:
                DrawImageEntity(static_cast<const DXFImageEntity&>(*pE),*pT);
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
    , maContrastInput(COL_TRANSPARENT)
    , maContrastOutput(COL_TRANSPARENT)
    , mnWork(0)
    , mnVisits(0)
    , mnInsertDepth(0)
    , mnDimensionDepth(0)
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

    mpMeasureDev.disposeAndReset(VclPtr<VirtualDevice>::Create());

    // first pass: the extents
    pVirDev = VclPtr<VirtualDevice>::Create();
    pVirDev->EnableOutput(false);
    ResetState();
    mbMeasure = true;
    mnWork = 0;
    mnVisits = 0;
    mnInsertDepth = 0;
    mnDimensionDepth = 0;
    maExtents.reset();
    maPointExtents.reset();
    DrawEntities(pDXF->aEntities, aViewTransform);
    pVirDev.disposeAndClear();
    mbMeasure = false;
    if (maExtents.isEmpty())
        maExtents = maPointExtents;

    if (!IsSane(maExtents))
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
    mnWork = 0;
    mnVisits = 0;
    mnInsertDepth = 0;
    mnDimensionDepth = 0;
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
