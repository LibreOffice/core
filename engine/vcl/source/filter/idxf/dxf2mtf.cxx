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
#include <o3tl/safeint.hxx>
#include <sal/log.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/metric.hxx>
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

    if (nTrueColor >= 0)
        return Color(ColorTransparency, nTrueColor);
    if (nColor == 0) // BYBLOCK
        return moBlockColor;
    if (nColor == 256) // BYLAYER
    {
        if (pLayer == nullptr)
            return ConvertColor(7);
        if (pLayer->nTrueColor >= 0)
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
    // Points are often stray definition points far from the drawing: they count for the
    // extents only if there is nothing else, and they are not drawn outside of them.
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

vcl::Font DXF2GDIMetaFile::MakeFont(const DXFTextFormat & rFormat, double fSize)
{
    vcl::Font aFont;
    if (!rFormat.maFontName.isEmpty())
        aFont.SetFamilyName(rFormat.maFontName);
    aFont.SetFamily(FAMILY_SWISS);
    aFont.SetFontSize(Size(0, basegfx::fround(fSize)));
    aFont.SetWeight(rFormat.mbBold ? WEIGHT_BOLD : WEIGHT_NORMAL);
    aFont.SetItalic(rFormat.mbItalic ? ITALIC_NORMAL : ITALIC_NONE);
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
    if (!oColor)
        return;

    if (mbMeasure)
    {
        const double fHeight = rFormat.mfHeight;
        maExtents.expand(Project(rTextTransform, DXFVector(fX, fY - fHeight / 3, 0)));
        maExtents.expand(Project(rTextTransform, DXFVector(fX + fWidth, fY - fHeight / 3, 0)));
        maExtents.expand(Project(rTextTransform, DXFVector(fX + fWidth, fY + fHeight, 0)));
        maExtents.expand(Project(rTextTransform, DXFVector(fX, fY + fHeight, 0)));
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
    aFont.SetColor(*oColor);
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

    DXFTextFormat aBase;
    aBase.mfHeight = rE.fHeight > 0 ? rE.fHeight : 1.0;
    const std::vector<DXFTextParagraph> aParagraphs
        = dxf::parseMText(pDXF->ToOUString(rE.m_sText, false), aBase, pDXF->getTextEncoding());

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
            // the coordinates of these are in the world coordinate system
            const bool bWorld
                = pE->eType == DXF_LINE || pE->eType == DXF_POINT || pE->eType == DXF_3DFACE
                  || pE->eType == DXF_ELLIPSE || pE->eType == DXF_SPLINE
                  || pE->eType == DXF_MTEXT
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

    mpMeasureDev.disposeAndReset(VclPtr<VirtualDevice>::Create());

    // first pass: the extents
    pVirDev = VclPtr<VirtualDevice>::Create();
    pVirDev->EnableOutput(false);
    ResetState();
    mbMeasure = true;
    maExtents.reset();
    maPointExtents.reset();
    DrawEntities(pDXF->aEntities, aViewTransform);
    pVirDev.disposeAndClear();
    mbMeasure = false;
    if (maExtents.isEmpty())
        maExtents = maPointExtents;

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
