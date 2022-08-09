/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_vcl.h>

#include <vcl/Geometry.hxx>
#include <vcl/lineinfo.hxx>

#include <maptools.hxx>

Geometry::Geometry()
    : mbMap(false)
    , mnWidth(0)
    , mnHeight(0)
    , mnOffsetOriginX(0)
    , mnOffsetOriginY(0)
    , mnOffsetOriginLogicX(0)
    , mnOffsetOriginLogicY(0)
    , mnFrameOffsetX(0)
    , mnFrameOffsetY(0)
    , mnDPIX(0)
    , mnDPIY(0)
    , mnDPIScalePercentage(100)
{
}

bool Geometry::IsMapModeEnabled() const { return mbMap; }

void Geometry::EnableMapMode(bool bEnable) { mbMap = bEnable; }

tools::Long Geometry::GetWidth() const { return mnWidth; }

void Geometry::SetWidth(tools::Long nWidth) { mnWidth = nWidth; }

tools::Long Geometry::GetHeight() const { return mnHeight; }

void Geometry::SetHeight(tools::Long nHeight) { mnHeight = nHeight; }

tools::Long Geometry::GetXOffsetFromOriginInPixels() const { return mnOffsetOriginX; }

void Geometry::SetXOffsetFromOriginInPixels(tools::Long nOffsetFromOriginXpx)
{
    mnOffsetOriginX = nOffsetFromOriginXpx;
}

tools::Long Geometry::GetYOffsetFromOriginInPixels() const { return mnOffsetOriginY; }

void Geometry::SetYOffsetFromOriginInPixels(tools::Long nOffsetFromOriginYpx)
{
    mnOffsetOriginY = nOffsetFromOriginYpx;
}

tools::Long Geometry::GetXOffsetFromOriginInLogicalUnits() const { return mnOffsetOriginLogicX; }

void Geometry::SetXOffsetFromOriginInLogicalUnits(tools::Long nOffsetFromOriginXInLogicalUnits)
{
    mnOffsetOriginLogicX = nOffsetFromOriginXInLogicalUnits;
}

tools::Long Geometry::GetYOffsetFromOriginInLogicalUnits() const { return mnOffsetOriginLogicX; }

void Geometry::SetYOffsetFromOriginInLogicalUnits(tools::Long nOffsetFromOriginYInLogicalUnits)
{
    mnOffsetOriginLogicY = nOffsetFromOriginYInLogicalUnits;
}

Point Geometry::GetFrameOffset() const { return Point(mnFrameOffsetX, mnFrameOffsetY); }

tools::Long Geometry::GetXFrameOffset() const { return mnFrameOffsetX; }

void Geometry::SetXFrameOffset(tools::Long nOffset) { mnFrameOffsetX = nOffset; }

tools::Long Geometry::GetYFrameOffset() const { return mnFrameOffsetY; }

void Geometry::SetYFrameOffset(tools::Long nOffset) { mnFrameOffsetY = nOffset; }

sal_Int32 Geometry::GetDPIX() const { return mnDPIX; }

sal_Int32 Geometry::GetDPIY() const { return mnDPIY; }

void Geometry::SetDPIX(sal_Int32 nDPIX) { mnDPIX = nDPIX; }

void Geometry::SetDPIY(sal_Int32 nDPIY) { mnDPIY = nDPIY; }

float Geometry::GetDPIScaleFactor() const { return mnDPIScalePercentage / 100.0f; }

sal_Int32 Geometry::GetDPIScalePercentage() const { return mnDPIScalePercentage; }

void Geometry::SetDPIScalePercentage(sal_Int32 nPercentage) { mnDPIScalePercentage = nPercentage; }

MappingMetrics Geometry::GetMapMetrics() const { return maMapMetrics; }

Point Geometry::GetMappingOffset() const
{
    return Point(maMapMetrics.mnMappingXOffset, maMapMetrics.mnMappingYOffset);
}

void Geometry::SetMappingXOffset(tools::Long nX) { maMapMetrics.mnMappingXOffset = nX; }

void Geometry::SetMappingYOffset(tools::Long nY) { maMapMetrics.mnMappingYOffset = nY; }

tools::Long Geometry::GetMapScalingXNumerator() const
{
    return maMapMetrics.mnMapScalingXNumerator;
}

tools::Long Geometry::GetMapScalingXDenominator() const
{
    return maMapMetrics.mnMapScalingXDenominator;
}

tools::Long Geometry::GetMapScalingYNumerator() const
{
    return maMapMetrics.mnMapScalingYNumerator;
}

tools::Long Geometry::GetMapScalingYDenominator() const
{
    return maMapMetrics.mnMapScalingYDenominator;
}

void Geometry::SetMapScalingXNumerator(tools::Long nX) { maMapMetrics.mnMapScalingXNumerator = nX; }

void Geometry::SetMapScalingXDenominator(tools::Long nX)
{
    maMapMetrics.mnMapScalingXDenominator = nX;
}

void Geometry::SetMapScalingYNumerator(tools::Long nY) { maMapMetrics.mnMapScalingYNumerator = nY; }

void Geometry::SetMapScalingYDenominator(tools::Long nY)
{
    maMapMetrics.mnMapScalingYDenominator = nY;
}

tools::Long Geometry::LogicToPixel(tools::Long n, tools::Long nDPI, tools::Long nMapNum,
                                   tools::Long nMapDenom)
{
    assert(nDPI > 0);
    assert(nMapDenom != 0);
    if constexpr (sizeof(tools::Long) >= 8)
    {
        assert(nMapNum >= 0);
        //detect overflows
        assert(nMapNum == 0
               || std::abs(n) < std::numeric_limits<tools::Long>::max() / nMapNum / nDPI);
    }
    sal_Int64 n64 = n;
    n64 *= nMapNum;
    n64 *= nDPI;
    if (nMapDenom == 1)
        n = static_cast<tools::Long>(n64);
    else
    {
        n64 = 2 * n64 / nMapDenom;
        if (n64 < 0)
            --n64;
        else
            ++n64;
        n = static_cast<tools::Long>(n64 / 2);
    }
    return n;
}

double Geometry::LogicToSubPixel(tools::Long n, tools::Long nDPI, tools::Long nMapNum,
                                 tools::Long nMapDenom)
{
    assert(nDPI > 0);
    assert(nMapDenom != 0);
    return static_cast<double>(n) * nMapNum * nDPI / nMapDenom;
}

tools::Long Geometry::PixelToLogic(tools::Long n, tools::Long nDPI, tools::Long nMapNum,
                                   tools::Long nMapDenom)
{
    assert(nDPI > 0);
    if (nMapNum == 0)
        return 0;
    sal_Int64 nDenom = nDPI;
    nDenom *= nMapNum;

    sal_Int64 n64 = n;
    n64 *= nMapDenom;
    if (nDenom == 1)
        n = static_cast<tools::Long>(n64);
    else
    {
        n64 = 2 * n64 / nDenom;
        if (n64 < 0)
            --n64;
        else
            ++n64;
        n = static_cast<tools::Long>(n64 / 2);
    }
    return n;
}

tools::Long Geometry::LogicXToDevicePixel(tools::Long nX) const
{
    if (!IsMapModeEnabled())
        return nX + GetXFrameOffset();

    return LogicToPixel(nX + GetMappingOffset().X(), GetDPIX(), GetMapScalingXNumerator(),
                        GetMapScalingXDenominator())
           + GetXFrameOffset() + GetXOffsetFromOriginInPixels();
}

tools::Long Geometry::LogicYToDevicePixel(tools::Long nY) const
{
    if (!IsMapModeEnabled())
        return nY + GetYFrameOffset();

    return LogicToPixel(nY + GetMappingOffset().Y(), GetDPIY(), GetMapScalingYNumerator(),
                        GetMapScalingYDenominator())
           + GetYFrameOffset() + GetYOffsetFromOriginInPixels();
}

tools::Long Geometry::LogicWidthToDevicePixel(tools::Long nWidth) const
{
    if (!IsMapModeEnabled())
        return nWidth;

    return LogicToPixel(nWidth, GetDPIX(), GetMapScalingXNumerator(), GetMapScalingXDenominator());
}

tools::Long Geometry::LogicHeightToDevicePixel(tools::Long nHeight) const
{
    if (!IsMapModeEnabled())
        return nHeight;

    return LogicToSubPixel(nHeight, GetDPIY(), GetMapScalingYNumerator(),
                           GetMapScalingYDenominator());
}

tools::Long Geometry::DevicePixelToLogicWidth(tools::Long nWidth) const
{
    if (!IsMapModeEnabled())
        return nWidth;

    return PixelToLogic(nWidth, GetDPIX(), GetMapScalingXNumerator(), GetMapScalingXDenominator());
}

tools::Long Geometry::DevicePixelToLogicHeight(tools::Long nHeight) const
{
    if (!IsMapModeEnabled())
        return nHeight;

    return PixelToLogic(nHeight, GetDPIY(), GetMapScalingYNumerator(), GetMapScalingYDenominator());
}

Point Geometry::LogicToDevicePixel(Point const& rLogicPt) const
{
    if (!IsMapModeEnabled())
        return Point(rLogicPt.X() + GetXFrameOffset(), rLogicPt.Y() + GetYFrameOffset());

    return Point(LogicToPixel(rLogicPt.X() + GetMappingOffset().X(), GetDPIX(),
                              GetMapScalingXNumerator(), GetMapScalingXDenominator())
                     + GetXFrameOffset() + GetXOffsetFromOriginInPixels(),
                 LogicToPixel(rLogicPt.Y() + GetMappingOffset().Y(), GetDPIY(),
                              GetMapScalingYNumerator(), GetMapScalingYDenominator())
                     + GetYFrameOffset() + GetYOffsetFromOriginInPixels());
}

Size Geometry::LogicToDevicePixel(Size const& rLogicSize) const
{
    if (!IsMapModeEnabled())
        return rLogicSize;

    return Size(LogicToPixel(rLogicSize.Width(), GetDPIX(), GetMapScalingXNumerator(),
                             GetMapScalingXDenominator()),
                LogicToPixel(rLogicSize.Height(), GetDPIY(), GetMapScalingYNumerator(),
                             GetMapScalingYDenominator()));
}

tools::Rectangle Geometry::LogicToDevicePixel(tools::Rectangle const& rLogicRect) const
{
    // tdf#141761 IsEmpty() removed
    // Even if rLogicRect.IsEmpty(), transform of the Position contained
    // in the Rectangle is necessary. Due to Rectangle::Right() returning
    // Left() when IsEmpty(), the code *could* stay unchanged (same for Bottom),
    // but:
    // The Rectangle constructor used with the four tools::Long values does not
    // check for IsEmpty(), so to keep that state correct there are two possibilities:
    // (1) Add a test to the Rectangle constructor in question
    // (2) Do it handish here
    // I have tried (1) first, but test Test::test_rectangle() claims that for
    //  tools::Rectangle aRect(1, 1, 1, 1);
    //    tools::Long(1) == aRect.GetWidth()
    //    tools::Long(0) == aRect.getWidth()
    // (remember: this means Left == Right == 1 -> GetWidth => 1, getWidth == 0)
    // so indeed the 1's have to go uncommented/unchecked into the data body
    // of rectangle. Switching to (2) *is* needed, doing so
    tools::Rectangle aRetval;

    if (!IsMapModeEnabled())
    {
        aRetval = tools::Rectangle(
            rLogicRect.Left() + GetXFrameOffset(), rLogicRect.Top() + GetYFrameOffset(),
            rLogicRect.IsWidthEmpty() ? 0 : rLogicRect.Right() + GetXFrameOffset(),
            rLogicRect.IsHeightEmpty() ? 0 : rLogicRect.Bottom() + GetYFrameOffset());
    }
    else
    {
        aRetval = tools::Rectangle(
            LogicToPixel(rLogicRect.Left() + GetMappingOffset().X(), GetDPIX(),
                         GetMapScalingXNumerator(), GetMapScalingXDenominator())
                + GetXFrameOffset() + GetXOffsetFromOriginInPixels(),
            LogicToPixel(rLogicRect.Top() + GetMappingOffset().Y(), GetDPIY(),
                         GetMapScalingYNumerator(), GetMapScalingYDenominator())
                + GetYFrameOffset() + GetYOffsetFromOriginInPixels(),
            rLogicRect.IsWidthEmpty()
                ? 0
                : LogicToPixel(rLogicRect.Right() + GetMappingOffset().X(), GetDPIX(),
                               GetMapScalingXNumerator(), GetMapScalingXDenominator())
                      + GetXFrameOffset() + GetXOffsetFromOriginInPixels(),
            rLogicRect.IsHeightEmpty()
                ? 0
                : LogicToPixel(rLogicRect.Bottom() + GetMappingOffset().Y(), GetDPIY(),
                               GetMapScalingYNumerator(), GetMapScalingYDenominator())
                      + GetYFrameOffset() + GetYOffsetFromOriginInPixels());
    }

    if (rLogicRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if (rLogicRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

tools::Polygon Geometry::LogicToDevicePixel(tools::Polygon const& rLogicPoly) const
{
    if (!IsMapModeEnabled() && !GetXFrameOffset() && !GetYFrameOffset())
        return rLogicPoly;

    sal_uInt16 i;
    sal_uInt16 nPoints = rLogicPoly.GetSize();
    tools::Polygon aPoly(rLogicPoly);

    // get pointer to Point-array (copy data)
    const Point* pPointAry = aPoly.GetConstPointAry();

    if (IsMapModeEnabled())
    {
        for (i = 0; i < nPoints; i++)
        {
            const Point& rPt = pPointAry[i];
            Point aPt(LogicToPixel(rPt.X() + GetMappingOffset().X(), GetDPIX(),
                                   GetMapScalingXNumerator(), GetMapScalingXDenominator())
                          + GetXFrameOffset() + GetXOffsetFromOriginInPixels(),
                      LogicToPixel(rPt.Y() + GetMappingOffset().Y(), GetDPIY(),
                                   GetMapScalingYNumerator(), GetMapScalingYDenominator())
                          + GetYFrameOffset() + GetYOffsetFromOriginInPixels());
            aPoly[i] = aPt;
        }
    }
    else
    {
        for (i = 0; i < nPoints; i++)
        {
            Point aPt = pPointAry[i];
            aPt.AdjustX(GetXFrameOffset());
            aPt.AdjustY(GetYFrameOffset());
            aPoly[i] = aPt;
        }
    }

    return aPoly;
}

basegfx::B2DPolygon Geometry::LogicToDevicePixel(basegfx::B2DPolygon const& rLogicPoly) const
{
    if (!IsMapModeEnabled() && !GetXFrameOffset() && !GetYFrameOffset())
        return rLogicPoly;

    sal_uInt32 nPoints = rLogicPoly.count();
    basegfx::B2DPolygon aPoly(rLogicPoly);

    basegfx::B2DPoint aC1;
    basegfx::B2DPoint aC2;

    if (IsMapModeEnabled())
    {
        for (sal_uInt32 i = 0; i < nPoints; ++i)
        {
            const basegfx::B2DPoint& rPt = aPoly.getB2DPoint(i);
            basegfx::B2DPoint aPt(
                LogicToPixel(rPt.getX() + GetMappingOffset().X(), GetDPIX(),
                             GetMapScalingXNumerator(), GetMapScalingXDenominator())
                    + GetXFrameOffset() + GetXOffsetFromOriginInPixels(),
                LogicToPixel(rPt.getY() + GetMappingOffset().Y(), GetDPIY(),
                             GetMapScalingYNumerator(), GetMapScalingYDenominator())
                    + GetYFrameOffset() + GetYOffsetFromOriginInPixels());

            const bool bC1 = aPoly.isPrevControlPointUsed(i);
            if (bC1)
            {
                const basegfx::B2DPoint aB2DC1(aPoly.getPrevControlPoint(i));

                aC1 = basegfx::B2DPoint(
                    LogicToPixel(aB2DC1.getX() + GetMappingOffset().X(), GetDPIX(),
                                 GetMapScalingXNumerator(), GetMapScalingXDenominator())
                        + GetXFrameOffset() + GetXOffsetFromOriginInPixels(),
                    LogicToPixel(aB2DC1.getY() + GetMappingOffset().Y(), GetDPIY(),
                                 GetMapScalingYNumerator(), GetMapScalingYDenominator())
                        + GetYFrameOffset() + GetYOffsetFromOriginInPixels());
            }

            const bool bC2 = aPoly.isNextControlPointUsed(i);
            if (bC2)
            {
                const basegfx::B2DPoint aB2DC2(aPoly.getNextControlPoint(i));

                aC2 = basegfx::B2DPoint(
                    LogicToPixel(aB2DC2.getX() + GetMappingOffset().X(), GetDPIX(),
                                 GetMapScalingXNumerator(), GetMapScalingXDenominator())
                        + GetXFrameOffset() + GetXOffsetFromOriginInPixels(),
                    LogicToPixel(aB2DC2.getY() + GetMappingOffset().Y(), GetDPIY(),
                                 GetMapScalingYNumerator(), GetMapScalingYDenominator())
                        + GetYFrameOffset() + GetYOffsetFromOriginInPixels());
            }

            aPoly.setB2DPoint(i, aPt);

            if (bC1)
                aPoly.setPrevControlPoint(i, aC1);

            if (bC2)
                aPoly.setNextControlPoint(i, aC2);
        }
    }
    else
    {
        for (sal_uInt32 i = 0; i < nPoints; ++i)
        {
            const basegfx::B2DPoint& rPt = aPoly.getB2DPoint(i);
            basegfx::B2DPoint aPt(rPt.getX() + GetXFrameOffset(), rPt.getY() + GetYFrameOffset());

            const bool bC1 = aPoly.isPrevControlPointUsed(i);
            if (bC1)
            {
                const basegfx::B2DPoint aB2DC1(aPoly.getPrevControlPoint(i));

                aC1 = basegfx::B2DPoint(aB2DC1.getX() + GetXFrameOffset(),
                                        aB2DC1.getY() + GetYFrameOffset());
            }

            const bool bC2 = aPoly.isNextControlPointUsed(i);
            if (bC2)
            {
                const basegfx::B2DPoint aB2DC2(aPoly.getNextControlPoint(i));

                aC1 = basegfx::B2DPoint(aB2DC2.getX() + GetXFrameOffset(),
                                        aB2DC2.getY() + GetYFrameOffset());
            }

            aPoly.setB2DPoint(i, aPt);

            if (bC1)
                aPoly.setPrevControlPoint(i, aC1);

            if (bC2)
                aPoly.setNextControlPoint(i, aC2);
        }
    }

    return aPoly;
}

tools::PolyPolygon Geometry::LogicToDevicePixel(tools::PolyPolygon const& rLogicPolyPoly) const
{
    if (!IsMapModeEnabled() && !GetXFrameOffset() && !GetYFrameOffset())
        return rLogicPolyPoly;

    tools::PolyPolygon aPolyPoly(rLogicPolyPoly);
    sal_uInt16 nPoly = aPolyPoly.Count();
    for (sal_uInt16 i = 0; i < nPoly; i++)
    {
        tools::Polygon& rPoly = aPolyPoly[i];
        rPoly = LogicToDevicePixel(rPoly);
    }
    return aPolyPoly;
}

LineInfo Geometry::LogicToDevicePixel(LineInfo const& rLineInfo) const
{
    LineInfo aInfo(rLineInfo);

    if (aInfo.GetStyle() == LineStyle::Dash)
    {
        if (aInfo.GetDotCount() && aInfo.GetDotLen())
            aInfo.SetDotLen(std::max(LogicWidthToDevicePixel(aInfo.GetDotLen()), tools::Long(1)));
        else
            aInfo.SetDotCount(0);

        if (aInfo.GetDashCount() && aInfo.GetDashLen())
            aInfo.SetDashLen(std::max(LogicWidthToDevicePixel(aInfo.GetDashLen()), tools::Long(1)));
        else
            aInfo.SetDashCount(0);

        aInfo.SetDistance(LogicWidthToDevicePixel(aInfo.GetDistance()));

        if ((!aInfo.GetDashCount() && !aInfo.GetDotCount()) || !aInfo.GetDistance())
            aInfo.SetStyle(LineStyle::Solid);
    }

    aInfo.SetWidth(LogicWidthToDevicePixel(aInfo.GetWidth()));

    return aInfo;
}

tools::Rectangle Geometry::DevicePixelToLogic(tools::Rectangle const& rPixelRect) const
{
    // tdf#141761 see comments above, IsEmpty() removed
    tools::Rectangle aRetval;

    if (!IsMapModeEnabled())
    {
        aRetval = tools::Rectangle(
            rPixelRect.Left() - GetXFrameOffset(), rPixelRect.Top() - GetYFrameOffset(),
            rPixelRect.IsWidthEmpty() ? 0 : rPixelRect.Right() - GetXFrameOffset(),
            rPixelRect.IsHeightEmpty() ? 0 : rPixelRect.Bottom() - GetYFrameOffset());
    }
    else
    {
        aRetval = tools::Rectangle(
            PixelToLogic(rPixelRect.Left() - GetXFrameOffset() - GetXOffsetFromOriginInPixels(),
                         GetDPIX(), GetMapScalingXNumerator(), GetMapScalingXDenominator())
                - GetMappingOffset().X(),
            PixelToLogic(rPixelRect.Top() - GetYFrameOffset() - GetYOffsetFromOriginInPixels(),
                         GetDPIY(), GetMapScalingYNumerator(), GetMapScalingYDenominator())
                - GetMappingOffset().Y(),
            rPixelRect.IsWidthEmpty()
                ? 0
                : PixelToLogic(rPixelRect.Right() - GetXFrameOffset()
                                   - GetXOffsetFromOriginInPixels(),
                               GetDPIX(), GetMapScalingXNumerator(), GetMapScalingXDenominator())
                      - GetMappingOffset().X(),
            rPixelRect.IsHeightEmpty()
                ? 0
                : PixelToLogic(rPixelRect.Bottom() - GetYFrameOffset()
                                   - GetYOffsetFromOriginInPixels(),
                               GetDPIY(), GetMapScalingYNumerator(), GetMapScalingYDenominator())
                      - GetMappingOffset().Y());
    }

    if (rPixelRect.IsWidthEmpty())
        aRetval.SetWidthEmpty();

    if (rPixelRect.IsHeightEmpty())
        aRetval.SetHeightEmpty();

    return aRetval;
}

vcl::Region Geometry::PixelToDevicePixel(vcl::Region const& rRegion) const
{
    if (!GetXFrameOffset() && !GetYFrameOffset())
        return rRegion;

    vcl::Region aRegion(rRegion);
    aRegion.Move(GetXFrameOffset() + GetXOffsetFromOriginInPixels(),
                 GetYFrameOffset() + GetYOffsetFromOriginInPixels());

    return aRegion;
}

static double lcl_LogicToSubPixel(tools::Long n, tools::Long nDPI, tools::Long nMapNum,
                                  tools::Long nMapDenom)
{
    assert(nDPI > 0);
    assert(nMapDenom != 0);
    double nRet = static_cast<double>(n) * nMapNum * nDPI / nMapDenom;
    return nRet;
}

static tools::Long lcl_SubPixelToLogic(bool bCheck, double n, tools::Long nDPI, tools::Long nMapNum,
                                       tools::Long nMapDenom)
{
    assert(nDPI > 0);
    assert(nMapNum != 0);

    double nRes = n * nMapDenom / nMapNum / nDPI;
    tools::Long nRet(std::round(nRes));
    assert(!bCheck
           || rtl_math_approxValue(n)
                  == rtl_math_approxValue(static_cast<double>(nRet) * nMapNum * nDPI / nMapDenom));
    (void)bCheck;
    return nRet;
}

DeviceCoordinate Geometry::LogicWidthToDeviceCoordinate(tools::Long nWidth) const
{
    if (!IsMapModeEnabled())
        return static_cast<DeviceCoordinate>(nWidth);

#if VCL_FLOAT_DEVICE_PIXEL
    return lcl_LogicToSubPixel(nWidth, GetDPIX(), GetMapScalingXNumerator(),
                               GetMapScalingXDenominator());
#else
    return LogicToPixel(nWidth, GetDPIX(), GetMapScalingXNumerator(), GetMapScalingXDenominator());
#endif
}

double Geometry::LogicWidthToDeviceSubPixel(tools::Long nWidth) const
{
    if (!IsMapModeEnabled())
        return nWidth;

    return lcl_LogicToSubPixel(nWidth, GetDPIX(), GetMapScalingXNumerator(),
                               GetMapScalingXDenominator());
}

double Geometry::LogicHeightToDeviceSubPixel(tools::Long nHeight) const
{
    if (!IsMapModeEnabled())
        return nHeight;

    return lcl_LogicToSubPixel(nHeight, GetDPIY(), GetMapScalingYNumerator(),
                               GetMapScalingYDenominator());
}

DevicePoint Geometry::LogicToDeviceSubPixel(const Point& rPoint) const
{
    if (!IsMapModeEnabled())
        return DevicePoint(rPoint.X() + GetXFrameOffset(), rPoint.Y() + GetYFrameOffset());

    return DevicePoint(lcl_LogicToSubPixel(rPoint.X() + GetMappingOffset().X(), GetDPIX(),
                                           GetMapScalingXNumerator(), GetMapScalingXDenominator())
                           + GetXFrameOffset() + GetXOffsetFromOriginInPixels(),
                       lcl_LogicToSubPixel(rPoint.Y() + GetMappingOffset().Y(), GetDPIY(),
                                           GetMapScalingYNumerator(), GetMapScalingYDenominator())
                           + GetYFrameOffset() + GetYOffsetFromOriginInPixels());
}

Point Geometry::SubPixelToLogic(DevicePoint const& rDevicePt, bool bCheck) const
{
    if (!IsMapModeEnabled())
    {
        assert(floor(rDevicePt.getX() == rDevicePt.getX())
               && floor(rDevicePt.getY() == rDevicePt.getY()));
        return Point(rDevicePt.getX(), rDevicePt.getY());
    }

    return Point(lcl_SubPixelToLogic(bCheck, rDevicePt.getX(), GetDPIX(), GetMapScalingXNumerator(),
                                     GetMapScalingXDenominator())
                     - GetMappingOffset().X() - GetXOffsetFromOriginInLogicalUnits(),
                 lcl_SubPixelToLogic(bCheck, rDevicePt.getY(), GetDPIY(), GetMapScalingXNumerator(),
                                     GetMapScalingXDenominator())
                     - GetMappingOffset().Y() - GetYOffsetFromOriginInLogicalUnits());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
