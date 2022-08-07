/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <tools/gen.hxx>
#include <tools/long.hxx>

#include <vcl/dllapi.h>
#include <vcl/rendercontext/MappingMetrics.hxx>

class VCL_DLLPUBLIC Geometry
{
public:
    Geometry();

    bool IsMapModeEnabled() const;
    void EnableMapMode(bool bEnable = true);

    Size GetSize() const;
    tools::Long GetWidth() const;
    tools::Long GetHeight() const;
    void SetSize(Size const& rSize);
    void SetWidth(tools::Long nWidth);
    void SetHeight(tools::Long nHeight);

    tools::Long GetXOffsetFromOriginInPixels() const;
    void SetXOffsetFromOriginInPixels(tools::Long nOffsetFromOriginXpx);
    tools::Long GetYOffsetFromOriginInPixels() const;
    void SetYOffsetFromOriginInPixels(tools::Long nOffsetFromOriginYpx);
    tools::Long GetXOffsetFromOriginInLogicalUnits() const;
    void SetXOffsetFromOriginInLogicalUnits(tools::Long nOffsetFromOriginXInLogicalUnits);
    tools::Long GetYOffsetFromOriginInLogicalUnits() const;
    void SetYOffsetFromOriginInLogicalUnits(tools::Long nOffsetFromOriginYInLogicalUnits);

    Point GetFrameOffset() const;
    tools::Long GetXFrameOffset() const;
    void SetXFrameOffset(tools::Long nOffset);
    tools::Long GetYFrameOffset() const;
    void SetYFrameOffset(tools::Long nOffset);

    MappingMetrics GetMapMetrics() const;

    Point GetMappingOffset() const;
    void SetMappingXOffset(tools::Long nX);
    void SetMappingYOffset(tools::Long nY);

    tools::Long GetMapScalingXNumerator() const;
    void SetMapScalingXNumerator(tools::Long nX);
    tools::Long GetMapScalingXDenominator() const;
    void SetMapScalingXDenominator(tools::Long nX);
    tools::Long GetMapScalingYNumerator() const;
    void SetMapScalingYNumerator(tools::Long nY);
    tools::Long GetMapScalingYDenominator() const;
    void SetMapScalingYDenominator(tools::Long nY);

    sal_Int32 GetDPIX() const;
    sal_Int32 GetDPIY() const;
    void SetDPIX(sal_Int32 nDPIX);
    void SetDPIY(sal_Int32 nDPIY);
    float GetDPIScaleFactor() const;
    sal_Int32 GetDPIScalePercentage() const;
    void SetDPIScalePercentage(sal_Int32 nPercentage);

private:
    bool mbMap;

    tools::Long mnWidth;
    tools::Long mnHeight;
    tools::Long mnOffsetOriginX;
    tools::Long mnOffsetOriginY;
    tools::Long mnOffsetOriginLogicX;
    tools::Long mnOffsetOriginLogicY;

    // Output offset for device output in pixels (pseduo window offset within window
    // system's frames)
    tools::Long mnFrameOffsetX;
    tools::Long mnFrameOffsetY;

    sal_Int32 mnDPIX;
    sal_Int32 mnDPIY;
    sal_Int32
        mnDPIScalePercentage; ///< For HiDPI displays, we want to draw elements for a percentage larger

    MappingMetrics maMapMetrics;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
