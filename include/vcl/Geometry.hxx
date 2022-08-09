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
#include <tools/poly.hxx>

#include <vcl/dllapi.h>
#include <vcl/devicecoordinate.hxx>
#include <vcl/region.hxx>
#include <vcl/rendercontext/MappingMetrics.hxx>

class LineInfo;

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

    static tools::Long LogicToPixel(tools::Long n, tools::Long nDPI, tools::Long nMapNum,
                                    tools::Long nMapDenom);

    static double LogicToSubPixel(tools::Long n, tools::Long nDPI, tools::Long nMapNum,
                                  tools::Long nMapDenom);

    static tools::Long PixelToLogic(tools::Long n, tools::Long nDPI, tools::Long nMapNum,
                                    tools::Long nMapDenom);

    /** Convert a logical X coordinate to a device pixel's X coordinate.

     To get the device's X coordinate, it must calculate the mapping offset
     coordinate X position (if there is one - if not then it just adds
     the pseudo-window offset to the logical X coordinate), the X-DPI of
     the device and the mapping's X scaling factor.

     @param nX Logical X coordinate

     @returns Device's X pixel coordinate
     */
    tools::Long LogicXToDevicePixel(tools::Long nX) const;

    /** Convert a logical Y coordinate to a device pixel's Y coordinate.

     To get the device's Y coordinate, it must calculate the mapping offset
     coordinate Y position (if there is one - if not then it just adds
     the pseudo-window offset to the logical Y coordinate), the Y-DPI of
     the device and the mapping's Y scaling factor.

     @param nY Logical Y coordinate

     @returns Device's Y pixel coordinate
     */
    tools::Long LogicYToDevicePixel(tools::Long nY) const;

    /** Convert a logical width to a width in units of device pixels.

     To get the number of device pixels, it must calculate the X-DPI of the device and
     the map scaling factor. If there is no mapping, then it just returns the
     width as nothing more needs to be done.

     @param nWidth Logical width

     @returns Width in units of device pixels.
     */

    tools::Long LogicWidthToDevicePixel(tools::Long nWidth) const;

    /** Convert a logical height to a height in units of device pixels.

     To get the number of device pixels, it must calculate the Y-DPI of the device and
     the map scaling factor. If there is no mapping, then it just returns the
     height as nothing more needs to be done.

     @param nHeight Logical height

     @returns Height in units of device pixels.
     */
    tools::Long LogicHeightToDevicePixel(tools::Long nHeight) const;

    /** Convert device pixels to a width in logical units.

     To get the logical width, it must calculate the X-DPI of the device and the
     map scaling factor.

     @param nWidth Width in device pixels

     @returns Width in logical units.
     */
    tools::Long DevicePixelToLogicWidth(tools::Long nWidth) const;

    /** Convert device pixels to a height in logical units.

     To get the logical height, it must calculate the Y-DPI of the device and the
     map scaling factor.

     @param nHeight Height in device pixels

     @returns Height in logical units.
     */
    tools::Long DevicePixelToLogicHeight(tools::Long nHeight) const;

    /** Convert a logical rectangle to a rectangle in physical device pixel units.

     @param rLogicRect Const reference to a rectangle in logical units

     @returns Rectangle based on physical device pixel coordinates and units.
     */
    tools::Rectangle LogicToDevicePixel(tools::Rectangle const& rLogicRect) const;

    /** Convert a logical point to a physical point on the device.

     @param rLogicPt Const reference to a point in logical units.

     @returns Physical point on the device.
     */
    Point LogicToDevicePixel(Point const& rLogicPt) const;

    /** Convert a logical size to the size on the physical device.

     @param rLogicSize Const reference to a size in logical units

     @returns Physical size on the device.
     */
    Size LogicToDevicePixel(Size const& rLogicSize) const;

    /** Convert a logical polygon to a polygon in physical device pixel units.

     @param rLogicPoly Const reference to a polygon in logical units

     @returns Polygon based on physical device pixel coordinates and units.
     */
    tools::Polygon LogicToDevicePixel(tools::Polygon const& rLogicPoly) const;

    /** Convert a logical B2DPolygon to a B2DPolygon in physical device pixel units.

     @param rLogicSize Const reference to a B2DPolygon in logical units

     @returns B2DPolyPolygon based on physical device pixel coordinates and units.
     */
    ::basegfx::B2DPolygon LogicToDevicePixel(::basegfx::B2DPolygon const& rLogicPoly) const;

    /** Convert a logical polypolygon to a polypolygon in physical device pixel units.

     @param rLogicPolyPoly Const reference to a polypolygon in logical units

     @returns Polypolygon based on physical device pixel coordinates and units.
     */
    tools::PolyPolygon LogicToDevicePixel(tools::PolyPolygon const& rLogicPolyPoly) const;

    /** Convert a line in logical units to a line in physical device pixel units.

     @param rLineInfo Const reference to a line in logical units

     @returns Line based on physical device pixel coordinates and units.
     */
    LineInfo LogicToDevicePixel(LineInfo const& rLineInfo) const;

    /** Convert a rectangle in physical pixel units to a rectangle in physical pixel units and coords.

     @param rPixelRect Const reference to rectangle in logical units and coords.

     @returns Rectangle based on logical coordinates and units.
     */
    tools::Rectangle DevicePixelToLogic(tools::Rectangle const& rPixelRect) const;

    /** Convert a region in pixel units to a region in device pixel units and coords.

     @param rRegion Const reference to region.

     @returns vcl::Region based on device pixel coordinates and units.
     */
    vcl::Region PixelToDevicePixel(vcl::Region const& rRegion) const;

    DevicePoint LogicToDeviceSubPixel(Point const& rLogicPt) const;

    double LogicWidthToDeviceSubPixel(tools::Long nWidth) const;
    double LogicHeightToDeviceSubPixel(tools::Long nHeight) const;

    DeviceCoordinate LogicWidthToDeviceCoordinate(tools::Long nWidth) const;

    Point SubPixelToLogic(DevicePoint const& rDevicePt, bool bCheck) const;

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
