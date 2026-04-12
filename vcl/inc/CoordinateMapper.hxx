/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#pragma once

#include <sal/types.h>
#include <tools/long.hxx>

#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>

class CoordinateMapper
{
private:
    bool mbMap = false;
    MapMode maMapMode;
    ImplMapRes maMapRes;

    // #i75163#
    mutable basegfx::B2DHomMatrix* mpViewTransform = nullptr;
    mutable basegfx::B2DHomMatrix* mpInverseViewTransform = nullptr;

    sal_Int32 mnDPIX = 0;
    sal_Int32 mnDPIY = 0;
    sal_Int32 mnDPIScalePercentage = 100;

    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    tools::Long mnOutOffX = 0;
    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    tools::Long mnOutOffY = 0;

    /// Additional output pixel offset, applied in LogicToPixel (used by SetPixelOffset/GetPixelOffset)
    tools::Long mnOutOffOrigX = 0;
    /// Additional output pixel offset, applied in LogicToPixel (used by SetPixelOffset/GetPixelOffset)
    tools::Long mnOutOffOrigY = 0;

    /// Additional output offset in _logical_ coordinates, applied in PixelToLogic (used by SetPixelOffset/GetPixelOffset)
    tools::Long mnOutOffLogicX = 0;
    /// Additional output offset in _logical_ coordinates, applied in PixelToLogic (used by SetPixelOffset/GetPixelOffset)
    tools::Long mnOutOffLogicY = 0;

    tools::Long mnOutWidth = 0;
    tools::Long mnOutHeight = 0;

public:
    bool IsMapModeEnabled() const { return mbMap; }
    void EnableMapMode(bool bEnable = true) { mbMap = bEnable; }

    sal_Int32 GetDPIX() const;
    sal_Int32 GetDPIY() const;

    void SetDPIX(sal_Int32 nDPIX);
    void SetDPIY(sal_Int32 nDPIY);

    sal_Int32 GetDPIScalePercentage() const;
    float GetDPIScaleFactor() const;
    void SetDPIScalePercentage(sal_Int32 nPercentage);

    tools::Long GetDeviceOriginX() const;
    tools::Long GetDeviceOriginY() const;

    void SetDeviceOriginX(tools::Long nOutOffX);
    void SetOutOffYPixel(tools::Long nOutOffY);

    Point GetOutputOffPixel() const;

    tools::Long GetOutputWidthPixel() const;
    tools::Long GetOutputHeightPixel() const;
    Size GetOutputSizePixel() const;

    Size GetPixelOffset() const { return Size(mnOutOffOrigX, mnOutOffOrigY); }
    void SetPixelOffset(const Size& rSize);
    tools::Long GetPixelXOffset() const { return mnOutOffOrigX; }
    tools::Long GetPixelYOffset() const { return mnOutOffOrigY; }

    void SetLogicalOffset(const Size& rSize);
    tools::Long GetLogicalXOffset() const { return mnOutOffLogicX; }
    tools::Long GetLogicalYOffset() const { return mnOutOffLogicY; }

    void SetOutputWidthPixel(tools::Long nWidth);
    void SetOutputHeightPixel(tools::Long nHeight);

    const MapMode& GetMapMode() const { return maMapMode; }
    bool IsDefaultMapMode() const { return maMapMode.IsDefault(); }
    void ResetMapMode() { maMapMode = MapMode(); }
    void ResetMapMode(const MapMode& rMapMode) { maMapMode = rMapMode; }
    MapUnit GetMapUnit() const { return maMapMode.GetMapUnit(); }

    double GetScaleX() const { return maMapMode.GetScaleX(); }
    double GetScaleY() const { return maMapMode.GetScaleY(); }
    void SetScaleX(double nX) { maMapMode.SetScaleX(nX); }
    void SetScaleY(double nY) { maMapMode.SetScaleY(nY); }

    void SetOrigin(const Point& rPt) { maMapMode.SetOrigin(rPt); }

    tools::Long GetMappingXOffset() const { return maMapRes.mnMapOfsX; }
    tools::Long GetMappingYOffset() const { return maMapRes.mnMapOfsY; }
    double GetMapResolutionScaleX() const { return maMapRes.mfMapScX; }
    double GetMapResolutionScaleY() const { return maMapRes.mfMapScY; }

    void SetMappingXOffset(tools::Long nOffset) { maMapRes.mnMapOfsX = nOffset; }
    void SetMappingYOffset(tools::Long nOffset) { maMapRes.mnMapOfsY = nOffset; }
    void SetMapResolutionScaleX(double fX) { maMapRes.mfMapScX = fX; }
    void SetMapResolutionScaleY(double fY) { maMapRes.mfMapScY = fY; }

    void CalcMapResolution(const MapMode& rMapMode, tools::Long nDPIX, tools::Long nDPIY);

    ImplMapRes ResolveMapRes(const MapMode* pMode);

    /** Invalidate the view transformation.

     @since AOO bug 75163 (OpenOffice.org 2.4.3 - OOH 680 milestone 212)
     */
    void InvalidateViewTransform();
    basegfx::B2DHomMatrix GetViewTransformation() const;
    basegfx::B2DHomMatrix GetViewTransformation(const MapMode& rMapMode) const;
    basegfx::B2DHomMatrix GetInverseViewTransformation() const;
    basegfx::B2DHomMatrix GetInverseViewTransformation(const MapMode& rMapMode) const;
    basegfx::B2DHomMatrix GetDeviceTransformation() const;

    tools::Long LogicToViewDistanceX(tools::Long n, double fMapResolutionScale) const;
    tools::Long LogicToViewDistanceY(tools::Long n, double fMapResolutionScale) const;

    double LogicToViewDistanceSubPixelX(tools::Long n, double fMapResolutionScale) const;
    double LogicToViewDistanceSubPixelY(tools::Long n, double fMapResolutionScale) const;

    tools::Long ViewSubPixelToLogicDistanceX(double n, double fMapResolutionScale) const;
    tools::Long ViewSubPixelToLogicDistanceY(double n, double fMapResolutionScale) const;

    tools::Long ViewToLogicDistanceX(tools::Long n, double fMapResolutionScale) const;
    tools::Long ViewToLogicDistanceY(tools::Long n, double fMapResolutionScale) const;

    double ViewToLogicDistanceDoubleX(double n, double fMapResolutionScale) const;
    double ViewToLogicDistanceDoubleY(double n, double fMapResolutionScale) const;

    tools::Long LogicToViewDistanceX(tools::Long n) const;
    tools::Long LogicToViewDistanceY(tools::Long n) const;

    double LogicToViewDistanceSubPixelX(tools::Long n) const;
    double LogicToViewDistanceSubPixelY(tools::Long n) const;

    tools::Long ViewSubPixelToLogicDistanceX(double n) const;
    tools::Long ViewSubPixelToLogicDistanceY(double n) const;

    tools::Long ViewToLogicDistanceX(tools::Long n) const;
    tools::Long ViewToLogicDistanceY(tools::Long n) const;

    double ViewToLogicDistanceDoubleX(double n) const;
    double ViewToLogicDistanceDoubleY(double n) const;

    // Document model coordinates (logical space)
    tools::Long LogicToOffsetLogicX(tools::Long nX) const;
    tools::Long LogicToOffsetLogicY(tools::Long nY) const;

    // Viewport (view space)

    // Physical window (device space)
    tools::Long LogicToDevicePixelX(tools::Long nX) const;
    tools::Long LogicToDevicePixelY(tools::Long nY) const;

    // Device <-> Window (Strip/Apply OutputDevice screen origin)
    tools::Long DeviceToWindowUnitsX(tools::Long nX) const;
    tools::Long DeviceToWindowUnitsY(tools::Long nY) const;
    tools::Long WindowToDeviceUnitsX(tools::Long nX) const;
    tools::Long WindowToDeviceUnitsY(tools::Long nY) const;

    // Window <-> View (Strip/Apply VCL internal Pixel Offset)
    tools::Long WindowToViewUnitsX(tools::Long nX) const;
    tools::Long WindowToViewUnitsY(tools::Long nY) const;
    tools::Long ViewToWindowUnitsX(tools::Long nX) const;
    tools::Long ViewToWindowUnitsY(tools::Long nY) const;

    // View <-> LogicUnits (Scale and apply Logical Mapping Offset)
    tools::Long ViewToLogicUnitsX(tools::Long nX) const;
    tools::Long ViewToLogicUnitsY(tools::Long nY) const;
    tools::Long LogicUnitsToViewUnitsX(tools::Long nX) const;
    tools::Long LogicUnitsToViewUnitsY(tools::Long nY) const;

private:
    tools::Long ImplCalcDevicePixelX(tools::Long nX) const;
    tools::Long ImplCalcDevicePixelY(tools::Long nY) const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
