/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <tools/color.hxx>
#include <tools/fontenum.hxx>
#include <i18nlangtag/lang.h>

#include <vcl/dllapi.h>
#include <vcl/DrawingInterface.hxx>
#include <vcl/font.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/outdevstate.hxx>
#include <vcl/rendercontext/ImplMapRes.hxx>
#include <vcl/rendercontext/RasterOp.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/virdev.hxx>

namespace vcl
{
class VCL_DLLPUBLIC RenderContext : public DrawingInterface
{
public:
    RenderContext();
    virtual ~RenderContext() {}

    Font const& GetFont() const { return maFont; }
    virtual void SetFont(Font const& rNewFont);

    Color const& GetTextColor() const { return maTextColor; }
    virtual void SetTextColor(Color const& rColor);

    bool IsTextFillColor() const { return !maFont.IsTransparent(); }
    Color GetTextFillColor() const;
    virtual void SetTextFillColor(Color const& rColor);

    bool IsTextLineColor() const { return !maTextLineColor.IsTransparent(); }
    Color const& GetTextLineColor() const { return maTextLineColor; }
    virtual void SetTextLineColor();
    virtual void SetTextLineColor(Color const& rColor);

    bool IsLineColor() const { return mbLineColor; }
    Color const& GetLineColor() const { return maLineColor; }
    virtual void SetLineColor();
    virtual void SetLineColor(Color const& rColor);

    bool IsFillColor() const { return mbFillColor; }
    Color const& GetFillColor() const { return maFillColor; }
    virtual void SetFillColor();
    virtual void SetFillColor(Color const& rColor);

    RasterOp GetRasterOp() const { return meRasterOp; }
    virtual void SetRasterOp(RasterOp eRasterOp);

    void EnableMapMode(bool bEnable = true);
    bool IsMapModeEnabled() const { return mbMap; }
    const MapMode& GetMapMode() const { return maMapMode; }
    virtual void SetMapMode(MapMode const& rNewMapMode);
    virtual void SetRelativeMapMode(MapMode const& rNewMapMode);

    /** Invalidate the view transformation.

    TODO: see if this can be made private

     @since AOO bug 75163 (OpenOffice.org 2.4.3 - OOH 680 milestone 212)
     */
    void ImplInvalidateViewTransform();

    TextAlign GetTextAlign() const { return maFont.GetAlignment(); }
    virtual void SetTextAlign(TextAlign eAlign);

    ComplexTextLayoutFlags GetLayoutMode() const { return mnTextLayoutMode; }
    virtual void SetLayoutMode(ComplexTextLayoutFlags nTextLayoutMode);

    LanguageType GetDigitLanguage() const { return meTextLanguage; }
    virtual void SetDigitLanguage(LanguageType);

    virtual void Push(PushFlags nFlags = PushFlags::ALL);
    virtual void Pop();
    void ClearStack();

protected:
    virtual void ImplInitMapModeObjects() {}

    Font maFont;
    Color maLineColor;
    Color maFillColor;
    Color maTextColor;
    Color maTextLineColor;
    ComplexTextLayoutFlags mnTextLayoutMode;
    LanguageType meTextLanguage;
    RasterOp meRasterOp;
    MapMode maMapMode;

    sal_Int32 mnDPIX;
    sal_Int32 mnDPIY;
    ImplMapRes maMapRes;
    /// Additional output pixel offset, applied in LogicToPixel (used by SetPixelOffset/GetPixelOffset)
    tools::Long mnOutOffOrigX;
    /// Additional output offset in _logical_ coordinates, applied in PixelToLogic (used by SetPixelOffset/GetPixelOffset)
    tools::Long mnOutOffLogicX;
    /// Additional output pixel offset, applied in LogicToPixel (used by SetPixelOffset/GetPixelOffset)
    tools::Long mnOutOffOrigY;
    /// Additional output offset in _logical_ coordinates, applied in PixelToLogic (used by SetPixelOffset/GetPixelOffset)
    tools::Long mnOutOffLogicY;
    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    tools::Long mnOutOffX;
    /// Output offset for device output in pixel (pseudo window offset within window system's frames)
    tools::Long mnOutOffY;
    tools::Long mnOutWidth;
    tools::Long mnOutHeight;

    mutable bool mbNewFont : 1;
    mutable bool mbLineColor : 1;
    mutable bool mbFillColor : 1;
    mutable bool mbMap : 1;
    mutable bool mbInitLineColor : 1;
    mutable bool mbInitFillColor : 1;
    mutable bool mbInitTextColor : 1;
    mutable bool mbInitFont;

    // TEMP TEMP TEMP
    VclPtr<VirtualDevice> mpAlphaVDev;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
