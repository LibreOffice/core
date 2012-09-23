/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


#include "view/SlsTheme.hxx"
#include "SlsResource.hxx"
#include "controller/SlsProperties.hxx"
#include "sdresid.hxx"
#include <tools/color.hxx>
#include <vcl/outdev.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
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




Theme::Theme (const ::boost::shared_ptr<controller::Properties>& rpProperties)
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




void Theme::Update (const ::boost::shared_ptr<controller::Properties>& rpProperties)
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
    SetGradient(Gradient_SelectedPage, aSelectionColor, 50, 50, +100,+100, +50,+25);
    SetGradient(Gradient_MouseOverPage, aSelectionColor, 75, 75, +100,+100, +50,+25);
    SetGradient(Gradient_SelectedAndFocusedPage, aSelectionColor, 50, 50, +100,+100, -50,-75);
    SetGradient(Gradient_MouseOverSelectedAndFocusedPage, aSelectionColor, 75, 75, +100,+100, -50,-75);
    SetGradient(Gradient_FocusedPage, aSelectionColor, -1,-1, 0,0, -50,-75);

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




::boost::shared_ptr<Font> Theme::GetFont (
    const FontType eType,
    const OutputDevice& rDevice)
{
    ::boost::shared_ptr<Font> pFont;

    switch (eType)
    {
        case Font_PageNumber:
            pFont.reset(new Font(Application::GetSettings().GetStyleSettings().GetAppFont()));
            pFont->SetTransparent(sal_True);
            pFont->SetWeight(WEIGHT_BOLD);
            break;

        case Font_PageCount:
            pFont.reset(new Font(Application::GetSettings().GetStyleSettings().GetAppFont()));
            pFont->SetTransparent(sal_True);
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
        default: OSL_ASSERT(false); // fall through
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
