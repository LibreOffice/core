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

#include "view/SlsTheme.hxx"
#include "SlsResource.hxx"
#include "controller/SlsProperties.hxx"
#include "sdresid.hxx"
#include <tools/color.hxx>
#include <vcl/outdev.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <svtools/colorcfg.hxx>

namespace sd { namespace slidesorter { namespace view {

const static ColorData Black = 0x000000;
const static ColorData White = 0xffffff;

ColorData ChangeLuminance (const ColorData aColorData, const int nValue)
{
    Color aColor (aColorData);
    if (nValue > 0)
        aColor.IncreaseLuminance(nValue);
    else
        aColor.DecreaseLuminance(-nValue);
    return aColor.GetColor();
}

ColorData HGBAdapt (
    const ColorData aColorData,
    const sal_Int32 nNewSaturation,
    const sal_Int32 nNewBrightness)
{
    sal_uInt16 nHue (0);
    sal_uInt16 nSaturation (0);
    sal_uInt16 nBrightness (0);
    Color(aColorData).RGBtoHSB(nHue, nSaturation, nBrightness);
    return Color::HSBtoRGB(
        nHue,
        nNewSaturation>=0 ? nNewSaturation : nSaturation,
        nNewBrightness>=0 ? nNewBrightness : nBrightness);
}

Theme::Theme (const std::shared_ptr<controller::Properties>& rpProperties)
    : maBackgroundColor(rpProperties->GetBackgroundColor().GetColor()),
      maPageBackgroundColor(COL_WHITE),
      maGradients(),
      maIcons(),
      maColor()
{
    {
        LocalResource aResource (RID_SLIDESORTER_ICONS);

        maColor.resize(_ColorType_Size_);
        maColor[Color_Background] = maBackgroundColor;
        maColor[Color_PageNumberDefault] = 0x0808080;
        maColor[Color_PageNumberHover] = 0x4c4c4c;
        maColor[Color_PageNumberHighContrast] = White;
        maColor[Color_PageNumberBrightBackground] = 0x333333;
        maColor[Color_PageNumberDarkBackground] = 0xcccccc;
        maColor[Color_PreviewBorder] = 0x949599;
    }

    Update(rpProperties);
}

void Theme::Update (const std::shared_ptr<controller::Properties>& rpProperties)
{
    // Set up colors.
    maBackgroundColor = rpProperties->GetBackgroundColor().GetColor();
    maPageBackgroundColor = svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;

    maColor[Color_Background] = maBackgroundColor;

    maGradients.resize(_GradientColorType_Size_);

    maColor[Color_Background] = maBackgroundColor;
    const ColorData aSelectionColor (rpProperties->GetSelectionColor().GetColor());
    maColor[Color_Selection] = aSelectionColor;
    if (Color(aSelectionColor).IsBright())
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
        LocalResource aResource (RID_SLIDESORTER_ICONS);
        maIcons.resize(_IconType_Size_);

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
            pFont.reset(new vcl::Font(Application::GetSettings().GetStyleSettings().GetAppFont()));
            pFont->SetTransparent(true);
            pFont->SetWeight(WEIGHT_BOLD);
            break;

        case Font_PageCount:
            pFont.reset(new vcl::Font(Application::GetSettings().GetStyleSettings().GetAppFont()));
            pFont->SetTransparent(true);
            pFont->SetWeight(WEIGHT_NORMAL);
            {
                const Size aSize (pFont->GetSize());
                pFont->SetSize(Size(aSize.Width()*5/3, aSize.Height()*5/3));
            }
            break;
    }

    if (pFont)
    {
        // Transform the point size to pixel size.
        const MapMode aFontMapMode (MAP_POINT);
        const Size aFontSize (rDevice.LogicToPixel(pFont->GetSize(), aFontMapMode));

        // Transform the font size to the logical coordinates of the device.
        pFont->SetSize(rDevice.PixelToLogic(aFontSize));
    }

    return pFont;
}

ColorData Theme::GetColor (const ColorType eType)
{
    if (eType>=0 && sal_uInt32(eType)<maColor.size())
        return maColor[eType];
    else
        return 0;
}

ColorData Theme::GetGradientColor (
    const GradientColorType eType,
    const GradientColorClass eClass)
{
    GradientDescriptor& rDescriptor (GetGradient(eType));

    switch (eClass)
    {
        case Border1: return rDescriptor.maBorderColor1;
        case Border2: return rDescriptor.maBorderColor2;
        case Fill1: return rDescriptor.maFillColor1;
        case Fill2: return rDescriptor.maFillColor2;
        default: OSL_ASSERT(false); SAL_FALLTHROUGH;
        case Base: return rDescriptor.maBaseColor;
    }
}

void Theme::SetGradient (
    const GradientColorType eType,
    const ColorData aBaseColor,
    const sal_Int32 nSaturationOverride,
    const sal_Int32 nBrightnessOverride,
    const sal_Int32 nFillStartOffset,
    const sal_Int32 nFillEndOffset,
    const sal_Int32 nBorderStartOffset,
    const sal_Int32 nBorderEndOffset)
{
    GradientDescriptor& rGradient (GetGradient(eType));

    rGradient.maBaseColor = aBaseColor;

    rGradient.mnSaturationOverride = nSaturationOverride;
    rGradient.mnBrightnessOverride = nBrightnessOverride;
    const ColorData aColor (nSaturationOverride>=0 || nBrightnessOverride>=0
        ? HGBAdapt(aBaseColor, nSaturationOverride, nBrightnessOverride)
        : aBaseColor);

    rGradient.maFillColor1 = ChangeLuminance(aColor, nFillStartOffset);
    rGradient.maFillColor2 = ChangeLuminance(aColor, nFillEndOffset);
    rGradient.maBorderColor1 = ChangeLuminance(aColor, nBorderStartOffset);
    rGradient.maBorderColor2 = ChangeLuminance(aColor, nBorderEndOffset);

    rGradient.mnFillOffset1 = nFillStartOffset;
    rGradient.mnFillOffset2 = nFillEndOffset;
    rGradient.mnBorderOffset1 = nBorderStartOffset;
    rGradient.mnBorderOffset2 = nBorderEndOffset;
}

const BitmapEx& Theme::GetIcon (const IconType eType)
{
    if (eType>=0 && size_t(eType)<maIcons.size())
        return maIcons[eType];
    else
    {
        OSL_ASSERT(eType>=0 && size_t(eType)<maIcons.size());
        return maIcons[0];
    }
}

Theme::GradientDescriptor& Theme::GetGradient (const GradientColorType eType)
{
    if (eType>=0 && size_t(eType)<maGradients.size())
        return maGradients[eType];
    else
    {
        OSL_ASSERT(eType>=0 && size_t(eType)<maGradients.size());
        return maGradients[0];
    }
}

void Theme::InitializeIcon (const IconType eType, sal_uInt16 nResourceId)
{
    if (eType>=0 && size_t(eType)<maIcons.size())
    {
        const BitmapEx aIcon (Image(SdResId(nResourceId)).GetBitmapEx());
        maIcons[eType] = aIcon;
    }
    else
    {
        OSL_ASSERT(eType>=0 && size_t(eType)<maIcons.size());
    }
}

} } } // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
