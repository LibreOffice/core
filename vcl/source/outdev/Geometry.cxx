/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/Geometry.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
