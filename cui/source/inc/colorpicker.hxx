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

#include <vcl/ColorDialog.hxx>
#include <vcl/customweld.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/hexcolorcontrol.hxx>
#include <o3tl/typed_flags_set.hxx>

enum class UpdateFlags
{
    NONE = 0x00,
    RGB = 0x01,
    CMYK = 0x02,
    HSB = 0x04,
    ColorChooser = 0x08,
    ColorSlider = 0x10,
    Hex = 0x20,
    All = 0x3f,
};

namespace o3tl
{
template <> struct typed_flags<UpdateFlags> : is_typed_flags<UpdateFlags, 0x3f>
{
};
}

enum class ColorComponent
{
    Red,
    Green,
    Blue,
    Hue,
    Saturation,
    Brightness,
    Cyan,
    Yellow,
    Magenta,
    Key,
};

enum ColorMode
{
    HUE,
    SATURATION,
    BRIGHTNESS,
    RED,
    GREEN,
    BLUE
};

class ColorFieldControl : public weld::CustomWidgetController
{
public:
    ColorFieldControl();

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual ~ColorFieldControl() override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void Resize() override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;

    void UpdateBitmap();
    void ShowPosition(const Point& rPos, bool bUpdate);
    void UpdatePosition();
    void Modify();

    void SetValues(sal_uInt16 nBaseValue, ColorMode eMode, double x, double y);
    double GetX() const { return mdX; }
    double GetY() const { return mdY; }

    void SetModifyHdl(const Link<ColorFieldControl&, void>& rLink) { maModifyHdl = rLink; }

private:
    ColorMode meMode;
    sal_uInt16 mnBaseValue;
    double mdX;
    double mdY;
    Point maPosition;
    VclPtr<VirtualDevice> mxBitmap;
    Link<ColorFieldControl&, void> maModifyHdl;
    std::vector<sal_uInt8> maRGB_Horiz;
    std::vector<sal_uInt16> maGrad_Horiz;
    std::vector<sal_uInt16> maPercent_Horiz;
    std::vector<sal_uInt8> maRGB_Vert;
    std::vector<sal_uInt16> maPercent_Vert;
};

class ColorPreviewControl : public weld::CustomWidgetController
{
private:
    Color m_aColor;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;

public:
    ColorPreviewControl() {}

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    void SetColor(const Color& rCol);
};

class ColorSliderControl : public weld::CustomWidgetController
{
public:
    ColorSliderControl();
    virtual ~ColorSliderControl() override;

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual bool MouseMove(const MouseEvent& rMEvt) override;
    virtual bool MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&) override;
    virtual void Resize() override;

    void UpdateBitmap();
    void ChangePosition(tools::Long nY);
    void Modify();

    void SetValue(const Color& rColor, ColorMode eMode, double dValue);
    double GetValue() const { return mdValue; }

    void SetModifyHdl(const Link<ColorSliderControl&, void>& rLink) { maModifyHdl = rLink; }

    sal_Int16 GetLevel() const { return mnLevel; }

private:
    Link<ColorSliderControl&, void> maModifyHdl;
    Color maColor;
    ColorMode meMode;
    VclPtr<VirtualDevice> mxBitmap;
    sal_Int16 mnLevel;
    double mdValue;
};

class ColorPickerDialog : public SfxDialogController
{
private:
    ColorFieldControl m_aColorField;
    ColorSliderControl m_aColorSlider;
    ColorPreviewControl m_aColorPreview;
    ColorPreviewControl m_aColorPrevious;

    std::unique_ptr<weld::CustomWeld> m_xColorField;
    std::unique_ptr<weld::CustomWeld> m_xColorSlider;
    std::unique_ptr<weld::CustomWeld> m_xColorPreview;
    std::unique_ptr<weld::CustomWeld> m_xColorPrevious;

    std::unique_ptr<weld::Widget> m_xFISliderLeft;
    std::unique_ptr<weld::Widget> m_xFISliderRight;
    std::unique_ptr<weld::RadioButton> m_xRBRed;
    std::unique_ptr<weld::RadioButton> m_xRBGreen;
    std::unique_ptr<weld::RadioButton> m_xRBBlue;
    std::unique_ptr<weld::RadioButton> m_xRBHue;
    std::unique_ptr<weld::RadioButton> m_xRBSaturation;
    std::unique_ptr<weld::RadioButton> m_xRBBrightness;

    std::unique_ptr<weld::SpinButton> m_xMFRed;
    std::unique_ptr<weld::SpinButton> m_xMFGreen;
    std::unique_ptr<weld::SpinButton> m_xMFBlue;
    std::unique_ptr<weld::HexColorControl> m_xEDHex;

    std::unique_ptr<weld::MetricSpinButton> m_xMFHue;
    std::unique_ptr<weld::MetricSpinButton> m_xMFSaturation;
    std::unique_ptr<weld::MetricSpinButton> m_xMFBrightness;

    std::unique_ptr<weld::MetricSpinButton> m_xMFCyan;
    std::unique_ptr<weld::MetricSpinButton> m_xMFMagenta;
    std::unique_ptr<weld::MetricSpinButton> m_xMFYellow;
    std::unique_ptr<weld::MetricSpinButton> m_xMFKey;

    ColorMode meMode;

    double mdRed, mdGreen, mdBlue;
    double mdHue, mdSat, mdBri;
    double mdCyan, mdMagenta, mdYellow, mdKey;

public:
    ColorPickerDialog(weld::Window* pParent, const Color& rColor, vcl::ColorPickerMode eDialogMode);

    Color GetColor() const;
    void SetColor(const Color& rColor);

private:
    void update_color(UpdateFlags n = UpdateFlags::All);

    DECL_LINK(ColorFieldControlModifydl, ColorFieldControl&, void);
    DECL_LINK(ColorSliderControlModifyHdl, ColorSliderControl&, void);
    DECL_LINK(ColorModifyMetricHdl, weld::MetricSpinButton&, void);
    DECL_LINK(ColorModifySpinHdl, weld::SpinButton&, void);
    DECL_LINK(ColorModifyEditHdl, weld::Entry&, void);
    DECL_LINK(ModeModifyHdl, weld::Toggleable&, void);

    void setColorComponent(ColorComponent nComp, double dValue);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
