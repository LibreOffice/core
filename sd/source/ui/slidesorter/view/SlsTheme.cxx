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

#include <bitmaps.hlst>
#include <view/SlsTheme.hxx>
#include <controller/SlsProperties.hxx>
#include <tools/color.hxx>
#include <vcl/outdev.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <osl/diagnose.h>

namespace sd::slidesorter::view {

const Color Black(0x000000);
const Color White(0xffffff);

static Color ChangeLuminance (Color aColor, const int nValue)
{
    if (nValue > 0)
        aColor.IncreaseLuminance(nValue);
    else
        aColor.DecreaseLuminance(-nValue);
    return aColor;
}

static Color HGBAdapt (
    const Color aColor,
    const sal_Int32 nNewSaturation,
    const sal_Int32 nNewBrightness)
{
    sal_uInt16 nHue (0);
    sal_uInt16 nSaturation (0);
    sal_uInt16 nBrightness (0);
    aColor.RGBtoHSB(nHue, nSaturation, nBrightness);
    return Color::HSBtoRGB(
        nHue,
        nNewSaturation>=0 ? nNewSaturation : nSaturation,
        nNewBrightness>=0 ? nNewBrightness : nBrightness);
}

Theme::Theme (const std::shared_ptr<controller::Properties>& rpProperties)
    : maBackgroundColor(rpProperties->GetBackgroundColor()),
      maGradients(),
      maIcons(),
      maColor()
{
    maColor.resize(ColorType_Size_);
    maColor[Color_Background] = maBackgroundColor;
    maColor[Color_PageNumberDefault] = Color(0x0808080);
    maColor[Color_PageNumberHover] = Color(0x4c4c4c);
    maColor[Color_PageNumberHighContrast] = White;
    maColor[Color_PageNumberBrightBackground] = Color(0x333333);
    maColor[Color_PageNumberDarkBackground] = Color(0xcccccc);
    maColor[Color_PreviewBorder] = Color(0x949599);

    Update(rpProperties);
}

void Theme::Update (const std::shared_ptr<controller::Properties>& rpProperties)
{
    // Set up colors.
    maBackgroundColor = rpProperties->GetBackgroundColor();

    maColor[Color_Background] = maBackgroundColor;

    maGradients.resize(GradientColorType_Size_);

    maColor[Color_Background] = maBackgroundColor;
    const Color aSelectionColor (rpProperties->GetSelectionColor());
    maColor[Color_Selection] = aSelectionColor;
    if (aSelectionColor.IsBright())
        maColor[Color_PageCountFontColor] = Black;
    else
        maColor[Color_PageCountFontColor] = White;

    // Set up gradients.
    SetGradient(Gradient_MouseOverPage, aSelectionColor, 0, 60, +80,+100, +50,+25);
    SetGradient(Gradient_SelectedPage, aSelectionColor, 50, 50, +80,+100, +50,+25);
    SetGradient(Gradient_FocusedPage, aSelectionColor, -1,-1, 0,0, -50,-75);
    SetGradient(Gradient_MouseOverSelected, aSelectionColor, 55, 60, +80,+100, +50,+25);
    SetGradient(Gradient_SelectedAndFocusedPage, aSelectionColor, 50, 50, +80,+100, -50,-75);
    SetGradient(Gradient_MouseOverSelectedAndFocusedPage, aSelectionColor, 55, 60, +80,+100, -50,-75);

    SetGradient(Gradient_NormalPage, maBackgroundColor, -1,-1, 0,0, 0,0);

    // The focused gradient needs special handling because its fill color is
    // like that of the NormalPage gradient.
    GetGradient(Gradient_FocusedPage).maFillColor1 = GetGradient(Gradient_NormalPage).maFillColor1;
    GetGradient(Gradient_FocusedPage).maFillColor2 = GetGradient(Gradient_NormalPage).maFillColor2;

    // Set up icons.
    if (maIcons.empty())
    {
        maIcons.resize(IconType_Size_);

        InitializeIcon(Icon_RawShadow, IMAGE_SHADOW);
        InitializeIcon(Icon_RawInsertShadow, IMAGE_INSERT_SHADOW);
        InitializeIcon(Icon_HideSlideOverlay, IMAGE_HIDE_SLIDE_OVERLAY);
        InitializeIcon(Icon_FocusBorder, IMAGE_FOCUS_BORDER);
    }
}

std::shared_ptr<vcl::Font> Theme::GetFont (
    const FontType eType,
    const OutputDevice& rDevice)
{
    std::shared_ptr<vcl::Font> pFont;

    switch (eType)
    {
        case Font_PageNumber:
            pFont = std::make_shared<vcl::Font>(Application::GetSettings().GetStyleSettings().GetAppFont());
            pFont->SetTransparent(true);
            pFont->SetWeight(WEIGHT_BOLD);
            break;

        case Font_PageCount:
            pFont = std::make_shared<vcl::Font>(Application::GetSettings().GetStyleSettings().GetAppFont());
            pFont->SetTransparent(true);
            pFont->SetWeight(WEIGHT_NORMAL);
            {
                const Size aSize (pFont->GetFontSize());
                pFont->SetFontSize(Size(aSize.Width()*5/3, aSize.Height()*5/3));
            }
            break;
    }

    if (pFont)
    {
        // Transform the point size to pixel size.
        const MapMode aFontMapMode (MapUnit::MapPoint);
        const Size aFontSize (rDevice.LogicToPixel(pFont->GetFontSize(), aFontMapMode));

        // Transform the font size to the logical coordinates of the device.
        pFont->SetFontSize(rDevice.PixelToLogic(aFontSize));
    }

    return pFont;
}

Color Theme::GetColor (const ColorType eType)
{
    if (sal_uInt32(eType)<maColor.size())
        return maColor[eType];
    else
        return Color(0);
}

Color Theme::GetGradientColor (
    const GradientColorType eType,
    const GradientColorClass eClass)
{
    GradientDescriptor& rDescriptor (GetGradient(eType));

    switch (eClass)
    {
        case GradientColorClass::Border1: return rDescriptor.maBorderColor1;
        case GradientColorClass::Border2: return rDescriptor.maBorderColor2;
        case GradientColorClass::Fill1: return rDescriptor.maFillColor1;
        case GradientColorClass::Fill2: return rDescriptor.maFillColor2;
    }
    return Color(0);
}

void Theme::SetGradient (
    const GradientColorType eType,
    const Color aBaseColor,
    const sal_Int32 nSaturationOverride,
    const sal_Int32 nBrightnessOverride,
    const sal_Int32 nFillStartOffset,
    const sal_Int32 nFillEndOffset,
    const sal_Int32 nBorderStartOffset,
    const sal_Int32 nBorderEndOffset)
{
    GradientDescriptor& rGradient (GetGradient(eType));

    const Color aColor (nSaturationOverride>=0 || nBrightnessOverride>=0
        ? HGBAdapt(aBaseColor, nSaturationOverride, nBrightnessOverride)
        : aBaseColor);

    rGradient.maFillColor1 = ChangeLuminance(aColor, nFillStartOffset);
    rGradient.maFillColor2 = ChangeLuminance(aColor, nFillEndOffset);
    rGradient.maBorderColor1 = ChangeLuminance(aColor, nBorderStartOffset);
    rGradient.maBorderColor2 = ChangeLuminance(aColor, nBorderEndOffset);
}

const BitmapEx& Theme::GetIcon (const IconType eType)
{
    if (size_t(eType)<maIcons.size())
        return maIcons[eType];
    else
    {
        OSL_ASSERT(eType>=0 && size_t(eType)<maIcons.size());
        return maIcons[0];
    }
}

Theme::GradientDescriptor& Theme::GetGradient (const GradientColorType eType)
{
    if (size_t(eType)<maGradients.size())
        return maGradients[eType];
    else
    {
        OSL_ASSERT(eType>=0 && size_t(eType)<maGradients.size());
        return maGradients[0];
    }
}

void Theme::InitializeIcon(const IconType eType, const OUString& rResourceId)
{
    if (size_t(eType)<maIcons.size())
    {
        const BitmapEx aIcon(rResourceId);
        maIcons[eType] = aIcon;
    }
    else
    {
        OSL_ASSERT(eType>=0 && size_t(eType)<maIcons.size());
    }
}

} // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
