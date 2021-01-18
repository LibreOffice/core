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

#include <vcl/window.hxx>
#include <vcl/waitobj.hxx>
#include <window.h>
#include <vcl/cursor.hxx>

WaitObject::WaitObject(vcl::Window* pWindow)
    : mpWindow(pWindow)
{
    if (mpWindow)
        mpWindow->EnterWait();
}

WaitObject::~WaitObject()
{
    if (mpWindow)
        mpWindow->LeaveWait();
}

namespace vcl
{
Size Window::GetOptimalSize() const { return Size(); }

void Window::ImplAdjustNWFSizes()
{
    for (Window* pWin = GetWindow(GetWindowType::FirstChild); pWin;
         pWin = pWin->GetWindow(GetWindowType::Next))
        pWin->ImplAdjustNWFSizes();
}

void WindowOutputDevice::ImplClearFontData(bool bNewFontLists)
{
    OutputDevice::ImplClearFontData(bNewFontLists);
    for (Window* pChild = mxOwnerWindow->mpWindowImpl->mpFirstChild; pChild;
         pChild = pChild->mpWindowImpl->mpNext)
        pChild->GetOutDev()->ImplClearFontData(bNewFontLists);
}

void WindowOutputDevice::ImplRefreshFontData(bool bNewFontLists)
{
    OutputDevice::ImplRefreshFontData(bNewFontLists);
    for (Window* pChild = mxOwnerWindow->mpWindowImpl->mpFirstChild; pChild;
         pChild = pChild->mpWindowImpl->mpNext)
        pChild->GetOutDev()->ImplRefreshFontData(bNewFontLists);
}

void WindowOutputDevice::ImplInitMapModeObjects()
{
    OutputDevice::ImplInitMapModeObjects();
    if (mxOwnerWindow->mpWindowImpl->mpCursor)
        mxOwnerWindow->mpWindowImpl->mpCursor->ImplNew();
}

const Font& Window::GetFont() const { return GetOutDev()->GetFont(); }
void Window::SetFont(Font const& font) { return GetOutDev()->SetFont(font); }

float Window::approximate_char_width() const { return GetOutDev()->approximate_char_width(); }

const Wallpaper& Window::GetBackground() const { return GetOutDev()->GetBackground(); }
bool Window::IsBackground() const { return GetOutDev()->IsBackground(); }
tools::Long Window::GetTextHeight() const { return GetOutDev()->GetTextHeight(); }
tools::Long Window::GetTextWidth(const OUString& rStr, sal_Int32 nIndex, sal_Int32 nLen,
                                 vcl::TextLayoutCache const* pCache,
                                 SalLayoutGlyphs const* const pLayoutCache) const
{
    return GetOutDev()->GetTextWidth(rStr, nIndex, nLen, pCache, pLayoutCache);
}
float Window::approximate_digit_width() const { return GetOutDev()->approximate_digit_width(); }

bool Window::IsNativeControlSupported(ControlType nType, ControlPart nPart) const
{
    return GetOutDev()->IsNativeControlSupported(nType, nPart);
}

bool Window::GetNativeControlRegion(ControlType nType, ControlPart nPart,
                                    const tools::Rectangle& rControlRegion, ControlState nState,
                                    const ImplControlValue& aValue,
                                    tools::Rectangle& rNativeBoundingRegion,
                                    tools::Rectangle& rNativeContentRegion) const
{
    return GetOutDev()->GetNativeControlRegion(nType, nPart, rControlRegion, nState, aValue,
                                               rNativeBoundingRegion, rNativeContentRegion);
}

Size Window::GetOutputSizePixel() const { return GetOutDev()->GetOutputSizePixel(); }

tools::Rectangle Window::GetOutputRectPixel() const { return GetOutDev()->GetOutputRectPixel(); }

void Window::SetTextLineColor() { GetOutDev()->SetTextLineColor(); }
void Window::SetTextLineColor(const Color& rColor) { GetOutDev()->SetTextLineColor(rColor); }
void Window::SetOverlineColor() { GetOutDev()->SetOverlineColor(); }
void Window::SetOverlineColor(const Color& rColor) { GetOutDev()->SetOverlineColor(rColor); }
void Window::SetTextFillColor() { GetOutDev()->SetTextFillColor(); }
void Window::SetTextFillColor(const Color& rColor) { GetOutDev()->SetTextFillColor(rColor); }
const MapMode& Window::GetMapMode() const { return GetOutDev()->GetMapMode(); }
void Window::SetBackground() { GetOutDev()->SetBackground(); }
void Window::SetBackground(const Wallpaper& rBackground)
{
    GetOutDev()->SetBackground(rBackground);
}
void Window::EnableMapMode(bool bEnable) { GetOutDev()->EnableMapMode(bEnable); }
bool Window::IsMapModeEnabled() const { return GetOutDev()->IsMapModeEnabled(); }

void Window::SetTextColor(const Color& rColor) { GetOutDev()->SetTextColor(rColor); }
const Color& Window::GetTextColor() const { return GetOutDev()->GetTextColor(); }
const Color& Window::GetTextLineColor() const { return GetOutDev()->GetTextLineColor(); }

bool Window::IsTextLineColor() const { return GetOutDev()->IsTextLineColor(); }

Color Window::GetTextFillColor() const { return GetOutDev()->GetTextFillColor(); }

bool Window::IsTextFillColor() const { return GetOutDev()->IsTextFillColor(); }

const Color& Window::GetOverlineColor() const { return GetOutDev()->GetOverlineColor(); }
bool Window::IsOverlineColor() const { return GetOutDev()->IsOverlineColor(); }
void Window::SetTextAlign(TextAlign eAlign) { GetOutDev()->SetTextAlign(eAlign); }

float Window::GetDPIScaleFactor() const { return GetOutDev()->GetDPIScaleFactor(); }
sal_Int32 Window::GetDPIScalePercentage() const { return GetOutDev()->GetDPIScalePercentage(); }
tools::Long Window::GetOutOffXPixel() const { return GetOutDev()->GetOutOffXPixel(); }
tools::Long Window::GetOutOffYPixel() const { return GetOutDev()->GetOutOffYPixel(); }
void Window::SetOutOffXPixel(tools::Long nOutOffX)
{
    return GetOutDev()->SetOutOffXPixel(nOutOffX);
}
void Window::SetOutOffYPixel(tools::Long nOutOffY)
{
    return GetOutDev()->SetOutOffXPixel(nOutOffY);
}
void Window::SetMapMode() { GetOutDev()->SetMapMode(); }
void Window::SetMapMode(const MapMode& rNewMapMode) { GetOutDev()->SetMapMode(rNewMapMode); }
bool Window::IsRTLEnabled() const { return GetOutDev()->IsRTLEnabled(); }
TextAlign Window::GetTextAlign() const { return GetOutDev()->GetTextAlign(); }
const AllSettings& Window::GetSettings() const { return GetOutDev()->GetSettings(); }

Point Window::LogicToPixel(const Point& rLogicPt) const
{
    return GetOutDev()->LogicToPixel(rLogicPt);
}
Size Window::LogicToPixel(const Size& rLogicSize) const
{
    return GetOutDev()->LogicToPixel(rLogicSize);
}
tools::Rectangle Window::LogicToPixel(const tools::Rectangle& rLogicRect) const
{
    return GetOutDev()->LogicToPixel(rLogicRect);
}
tools::Polygon Window::LogicToPixel(const tools::Polygon& rLogicPoly) const
{
    return GetOutDev()->LogicToPixel(rLogicPoly);
}
tools::PolyPolygon Window::LogicToPixel(const tools::PolyPolygon& rLogicPoly) const
{
    return GetOutDev()->LogicToPixel(rLogicPoly);
}
basegfx::B2DPolyPolygon Window::LogicToPixel(const basegfx::B2DPolyPolygon& rLogicPolyPoly) const
{
    return GetOutDev()->LogicToPixel(rLogicPolyPoly);
}
vcl::Region Window::LogicToPixel(const vcl::Region& rLogicRegion) const
{
    return GetOutDev()->LogicToPixel(rLogicRegion);
}
Point Window::LogicToPixel(const Point& rLogicPt, const MapMode& rMapMode) const
{
    return GetOutDev()->LogicToPixel(rLogicPt, rMapMode);
}
Size Window::LogicToPixel(const Size& rLogicSize, const MapMode& rMapMode) const
{
    return GetOutDev()->LogicToPixel(rLogicSize, rMapMode);
}
tools::Rectangle Window::LogicToPixel(const tools::Rectangle& rLogicRect,
                                      const MapMode& rMapMode) const
{
    return GetOutDev()->LogicToPixel(rLogicRect, rMapMode);
}
tools::Polygon Window::LogicToPixel(const tools::Polygon& rLogicPoly, const MapMode& rMapMode) const
{
    return GetOutDev()->LogicToPixel(rLogicPoly, rMapMode);
}
basegfx::B2DPolyPolygon Window::LogicToPixel(const basegfx::B2DPolyPolygon& rLogicPolyPoly,
                                             const MapMode& rMapMode) const
{
    return GetOutDev()->LogicToPixel(rLogicPolyPoly, rMapMode);
}

Point Window::PixelToLogic(const Point& rDevicePt) const
{
    return GetOutDev()->PixelToLogic(rDevicePt);
}
Size Window::PixelToLogic(const Size& rDeviceSize) const
{
    return GetOutDev()->PixelToLogic(rDeviceSize);
}
tools::Rectangle Window::PixelToLogic(const tools::Rectangle& rDeviceRect) const
{
    return GetOutDev()->PixelToLogic(rDeviceRect);
}
tools::Polygon Window::PixelToLogic(const tools::Polygon& rDevicePoly) const
{
    return GetOutDev()->PixelToLogic(rDevicePoly);
}
tools::PolyPolygon Window::PixelToLogic(const tools::PolyPolygon& rDevicePolyPoly) const
{
    return GetOutDev()->PixelToLogic(rDevicePolyPoly);
}
basegfx::B2DPolyPolygon Window::PixelToLogic(const basegfx::B2DPolyPolygon& rDevicePolyPoly) const
{
    return GetOutDev()->PixelToLogic(rDevicePolyPoly);
}
vcl::Region Window::PixelToLogic(const vcl::Region& rDeviceRegion) const
{
    return GetOutDev()->PixelToLogic(rDeviceRegion);
}
Point Window::PixelToLogic(const Point& rDevicePt, const MapMode& rMapMode) const
{
    return GetOutDev()->PixelToLogic(rDevicePt, rMapMode);
}
Size Window::PixelToLogic(const Size& rDeviceSize, const MapMode& rMapMode) const
{
    return GetOutDev()->PixelToLogic(rDeviceSize, rMapMode);
}
tools::Rectangle Window::PixelToLogic(const tools::Rectangle& rDeviceRect,
                                      const MapMode& rMapMode) const
{
    return GetOutDev()->PixelToLogic(rDeviceRect, rMapMode);
}
tools::Polygon Window::PixelToLogic(const tools::Polygon& rDevicePoly,
                                    const MapMode& rMapMode) const
{
    return GetOutDev()->PixelToLogic(rDevicePoly, rMapMode);
}
basegfx::B2DPolygon Window::PixelToLogic(const basegfx::B2DPolygon& rDevicePoly,
                                         const MapMode& rMapMode) const
{
    return GetOutDev()->PixelToLogic(rDevicePoly, rMapMode);
}
basegfx::B2DPolyPolygon Window::PixelToLogic(const basegfx::B2DPolyPolygon& rDevicePolyPoly,
                                             const MapMode& rMapMode) const
{
    return GetOutDev()->PixelToLogic(rDevicePolyPoly, rMapMode);
}

Point Window::LogicToLogic(const Point& rPtSource, const MapMode* pMapModeSource,
                           const MapMode* pMapModeDest) const
{
    return GetOutDev()->LogicToLogic(rPtSource, pMapModeSource, pMapModeDest);
}
Size Window::LogicToLogic(const Size& rSzSource, const MapMode* pMapModeSource,
                          const MapMode* pMapModeDest) const
{
    return GetOutDev()->LogicToLogic(rSzSource, pMapModeSource, pMapModeDest);
}
tools::Rectangle Window::LogicToLogic(const tools::Rectangle& rRectSource,
                                      const MapMode* pMapModeSource,
                                      const MapMode* pMapModeDest) const
{
    return GetOutDev()->LogicToLogic(rRectSource, pMapModeSource, pMapModeDest);
}

tools::Rectangle Window::GetTextRect(const tools::Rectangle& rRect, const OUString& rStr,
                                     DrawTextFlags nStyle, TextRectInfo* pInfo,
                                     const vcl::ITextLayout* _pTextLayout) const
{
    return GetOutDev()->GetTextRect(rRect, rStr, nStyle, pInfo, _pTextLayout);
}

void Window::SetSettings(const AllSettings& rSettings) { GetOutDev()->SetSettings(rSettings); }
void Window::SetSettings(const AllSettings& rSettings, bool bChild)
{
    static_cast<vcl::WindowOutputDevice*>(GetOutDev())->SetSettings(rSettings, bChild);
}

Color Window::GetBackgroundColor() const { return GetOutDev()->GetBackgroundColor(); }

void Window::EnableRTL(bool bEnable) { GetOutDev()->EnableRTL(bEnable); }

} /* namespace vcl */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
