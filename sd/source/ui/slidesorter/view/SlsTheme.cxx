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

#include "precompiled_sd.hxx"

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
    : mbIsHighContrastMode(false),
      maBackgroundColor(rpProperties->GetBackgroundColor().GetColor()),
      maPageBackgroundColor(COL_WHITE),
      maGradients(),
      maIcons(),
      maColor(),
      maIntegerValues()
{
    {
        LocalResource aResource (RID_SLIDESORTER_ICONS);

        maStrings.resize(_StringType_Size_);
        maStrings[String_DragAndDropPages] = String(SdResId(STRING_DRAG_AND_DROP_PAGES));
        maStrings[String_DragAndDropSlides] = String(SdResId(STRING_DRAG_AND_DROP_SLIDES));
        maStrings[String_Command1] = String(SdResId(STRING_COMMAND1));
        maStrings[String_Command2] = String(SdResId(STRING_COMMAND2_A));
        maStrings[String_Command2B] = String(SdResId(STRING_COMMAND2_B));
        maStrings[String_Command3] = String(SdResId(STRING_COMMAND3));

        maColor.resize(_ColorType_Size_);
        maColor[Color_Background] = maBackgroundColor;
        maColor[Color_ButtonBackground] = Black;
        maColor[Color_ButtonText] = 0xc0c0c0;
        maColor[Color_ButtonTextHover] = White;
        maColor[Color_PageNumberDefault] = 0x0808080;
        maColor[Color_PageNumberHover] = 0x4c4c4c;
        maColor[Color_PageNumberHighContrast] = White;
        maColor[Color_PageNumberBrightBackground] = 0x333333;
        maColor[Color_PageNumberDarkBackground] = 0xcccccc;
        maColor[Color_PreviewBorder] = 0x949599;

        maIntegerValues.resize(_IntegerValueType_Size_);
        maIntegerValues[Integer_ButtonCornerRadius] = 3;
        maIntegerValues[Integer_ButtonMaxAlpha] = 0;
        maIntegerValues[Integer_ButtonBarMaxAlpha] = 0;
        maIntegerValues[Integer_ButtonPaintType] = 1;
        maIntegerValues[Integer_ButtonBorder] = 4;
        maIntegerValues[Integer_ButtonGap] = 0;
        maIntegerValues[Integer_ButtonFadeInDelay] = 800;
        maIntegerValues[Integer_ButtonFadeInDuration] = 100;
        maIntegerValues[Integer_ButtonFadeOutDelay] = 0;
        maIntegerValues[Integer_ButtonFadeOutDuration] = 100;
        maIntegerValues[Integer_ToolTipDelay] = 1000;
        maIntegerValues[Integer_FocusIndicatorWidth] = 3;
    }

    Update(rpProperties);
}




void Theme::Update (const ::boost::shared_ptr<controller::Properties>& rpProperties)
{
    const bool bSavedHighContrastMode (mbIsHighContrastMode);
    mbIsHighContrastMode = rpProperties->IsHighContrastModeActive();

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

    SetGradient(Gradient_ButtonBackground, Black, -1,-1, 0,0, 0,0);
    SetGradient(Gradient_NormalPage, maBackgroundColor, -1,-1, 0,0, 0,0);

    // The focused gradient needs special handling because its fill color is
    // like that of the NormalPage gradient.
    GetGradient(Gradient_FocusedPage).maFillColor1 = GetGradient(Gradient_NormalPage).maFillColor1;
    GetGradient(Gradient_FocusedPage).maFillColor2 = GetGradient(Gradient_NormalPage).maFillColor2;

    // Set up icons.
    if (bSavedHighContrastMode != mbIsHighContrastMode || maIcons.empty())
    {
        LocalResource aResource (RID_SLIDESORTER_ICONS);

        maIcons.resize(_IconType_Size_);
        if (mbIsHighContrastMode)
        {
            InitializeIcon(Icon_RawShadow, IMAGE_SHADOW);
            InitializeIcon(Icon_RawInsertShadow, IMAGE_INSERT_SHADOW);
            InitializeIcon(Icon_HideSlideOverlay, IMAGE_HIDE_SLIDE_OVERLAY);

            InitializeIcon(Icon_ButtonBarLarge, IMAGE_BUTTONBAR_LARGE_HC);
            InitializeIcon(Icon_ButtonBarMedium, IMAGE_BUTTONBAR_MEDIUM_HC);
            InitializeIcon(Icon_ButtonBarSmall, IMAGE_BUTTONBAR_SMALL_HC);

            InitializeIcon(Icon_Command1Large, IMAGE_COMMAND1_LARGE_HC);
            InitializeIcon(Icon_Command1LargeHover, IMAGE_COMMAND1_LARGE_HOVER_HC);
            InitializeIcon(Icon_Command1Medium, IMAGE_COMMAND1_MEDIUM_HC);
            InitializeIcon(Icon_Command1MediumHover, IMAGE_COMMAND1_MEDIUM_HOVER_HC);
            InitializeIcon(Icon_Command1Small, IMAGE_COMMAND1_SMALL_HC);
            InitializeIcon(Icon_Command1SmallHover, IMAGE_COMMAND1_SMALL_HOVER_HC);

            InitializeIcon(Icon_Command2Large, IMAGE_COMMAND2_LARGE_HC);
            InitializeIcon(Icon_Command2LargeHover, IMAGE_COMMAND2_LARGE_HOVER_HC);
            InitializeIcon(Icon_Command2Medium, IMAGE_COMMAND2_MEDIUM_HC);
            InitializeIcon(Icon_Command2MediumHover, IMAGE_COMMAND2_MEDIUM_HOVER_HC);
            InitializeIcon(Icon_Command2Small, IMAGE_COMMAND2_SMALL_HC);
            InitializeIcon(Icon_Command2SmallHover, IMAGE_COMMAND2_SMALL_HOVER_HC);

            InitializeIcon(Icon_Command2BLarge, IMAGE_COMMAND2B_LARGE_HC);
            InitializeIcon(Icon_Command2BLargeHover, IMAGE_COMMAND2B_LARGE_HOVER_HC);
            InitializeIcon(Icon_Command2BMedium, IMAGE_COMMAND2B_MEDIUM_HC);
            InitializeIcon(Icon_Command2BMediumHover, IMAGE_COMMAND2B_MEDIUM_HOVER_HC);
            InitializeIcon(Icon_Command2BSmall, IMAGE_COMMAND2B_SMALL_HC);
            InitializeIcon(Icon_Command2BSmallHover, IMAGE_COMMAND2B_SMALL_HOVER_HC);

            InitializeIcon(Icon_Command3Large, IMAGE_COMMAND3_LARGE_HC);
            InitializeIcon(Icon_Command3LargeHover, IMAGE_COMMAND3_LARGE_HOVER_HC);
            InitializeIcon(Icon_Command3Medium, IMAGE_COMMAND3_SMALL_HC);
            InitializeIcon(Icon_Command3MediumHover, IMAGE_COMMAND3_SMALL_HOVER_HC);
            InitializeIcon(Icon_Command3Small, IMAGE_COMMAND3_SMALL_HC);
            InitializeIcon(Icon_Command3SmallHover, IMAGE_COMMAND3_SMALL_HOVER_HC);
        }
        else
        {
            InitializeIcon(Icon_RawShadow, IMAGE_SHADOW);
            InitializeIcon(Icon_RawInsertShadow, IMAGE_INSERT_SHADOW);
            InitializeIcon(Icon_HideSlideOverlay, IMAGE_HIDE_SLIDE_OVERLAY);

            InitializeIcon(Icon_ButtonBarLarge, IMAGE_BUTTONBAR_LARGE);
            InitializeIcon(Icon_ButtonBarMedium, IMAGE_BUTTONBAR_MEDIUM);
            InitializeIcon(Icon_ButtonBarSmall, IMAGE_BUTTONBAR_SMALL);

            InitializeIcon(Icon_Command1Large, IMAGE_COMMAND1_LARGE);
            InitializeIcon(Icon_Command1LargeHover, IMAGE_COMMAND1_LARGE_HOVER);
            InitializeIcon(Icon_Command1Medium, IMAGE_COMMAND1_MEDIUM);
            InitializeIcon(Icon_Command1MediumHover, IMAGE_COMMAND1_MEDIUM_HOVER);
            InitializeIcon(Icon_Command1Small, IMAGE_COMMAND1_SMALL);
            InitializeIcon(Icon_Command1SmallHover, IMAGE_COMMAND1_SMALL_HOVER);

            InitializeIcon(Icon_Command2Large, IMAGE_COMMAND2_LARGE);
            InitializeIcon(Icon_Command2LargeHover, IMAGE_COMMAND2_LARGE_HOVER);
            InitializeIcon(Icon_Command2Medium, IMAGE_COMMAND2_MEDIUM);
            InitializeIcon(Icon_Command2MediumHover, IMAGE_COMMAND2_MEDIUM_HOVER);
            InitializeIcon(Icon_Command2Small, IMAGE_COMMAND2_SMALL);
            InitializeIcon(Icon_Command2SmallHover, IMAGE_COMMAND2_SMALL_HOVER);

            InitializeIcon(Icon_Command2BLarge, IMAGE_COMMAND2B_LARGE);
            InitializeIcon(Icon_Command2BLargeHover, IMAGE_COMMAND2B_LARGE_HOVER);
            InitializeIcon(Icon_Command2BMedium, IMAGE_COMMAND2B_MEDIUM);
            InitializeIcon(Icon_Command2BMediumHover, IMAGE_COMMAND2B_MEDIUM_HOVER);
            InitializeIcon(Icon_Command2BSmall, IMAGE_COMMAND2B_SMALL);
            InitializeIcon(Icon_Command2BSmallHover, IMAGE_COMMAND2B_SMALL_HOVER);

            InitializeIcon(Icon_Command3Large, IMAGE_COMMAND3_LARGE);
            InitializeIcon(Icon_Command3LargeHover, IMAGE_COMMAND3_LARGE_HOVER);
            InitializeIcon(Icon_Command3Medium, IMAGE_COMMAND3_MEDIUM);
            InitializeIcon(Icon_Command3MediumHover, IMAGE_COMMAND3_MEDIUM_HOVER);
            InitializeIcon(Icon_Command3Small, IMAGE_COMMAND3_SMALL);
            InitializeIcon(Icon_Command3SmallHover, IMAGE_COMMAND3_SMALL_HOVER);
        }
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

        case Font_Button:
            pFont.reset(new Font(Application::GetSettings().GetStyleSettings().GetAppFont()));
            pFont->SetTransparent(sal_True);
            pFont->SetWeight(WEIGHT_BOLD);
            {
                const Size aSize (pFont->GetSize());
                pFont->SetSize(Size(aSize.Width()*4/3, aSize.Height()*4/3));
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




void Theme::SetColor (
    const ColorType eType,
    const ColorData aData)
{
    if (eType>=0 && sal_uInt32(eType)<maColor.size())
        maColor[eType] = aData;
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




sal_Int32 Theme::GetGradientOffset (
    const GradientColorType eType,
    const GradientColorClass eClass)
{
    GradientDescriptor& rDescriptor (GetGradient(eType));

    switch (eClass)
    {
        case Border1: return rDescriptor.mnBorderOffset1;
        case Border2: return rDescriptor.mnBorderOffset2;
        case Fill1: return rDescriptor.mnFillOffset1;
        case Fill2: return rDescriptor.mnFillOffset2;
        default: OSL_ASSERT(false); // fall through
        case Base: return 0;
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




sal_Int32 Theme::GetGradientSaturationOverride (const GradientColorType eType)
{
    GradientDescriptor& rGradient (GetGradient(eType));
    return rGradient.mnSaturationOverride;
}




sal_Int32 Theme::GetGradientBrightnessOverride (const GradientColorType eType)
{
    GradientDescriptor& rGradient (GetGradient(eType));
    return rGradient.mnBrightnessOverride;
}




void Theme::SetGradientSaturationOverride (const GradientColorType eType, const sal_Int32 nValue)
{
    GradientDescriptor& rGradient (GetGradient(eType));
    SetGradient(
        eType,
        rGradient.maBaseColor,
        nValue,
        rGradient.mnBrightnessOverride,
        rGradient.mnFillOffset1,
        rGradient.mnFillOffset2,
        rGradient.mnBorderOffset1,
        rGradient.mnBorderOffset2);
}




void Theme::SetGradientBrightnessOverride (const GradientColorType eType, const sal_Int32 nValue)
{
    GradientDescriptor& rGradient (GetGradient(eType));
    SetGradient(eType,
        rGradient.maBaseColor,
        rGradient.mnSaturationOverride,
        nValue,
        rGradient.mnFillOffset1,
        rGradient.mnFillOffset2,
        rGradient.mnBorderOffset1,
        rGradient.mnBorderOffset2);
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




sal_Int32 Theme::GetIntegerValue (const IntegerValueType eType) const
{
    if (eType>=0 && size_t(eType)<maIntegerValues.size())
        return maIntegerValues[eType];
    else
    {
        OSL_ASSERT(eType>=0 && size_t(eType)<maIntegerValues.size());
        return 0;
    }
}




void Theme::SetIntegerValue (const IntegerValueType eType, const sal_Int32 nValue)
{
    if (eType>=0 && size_t(eType)<maIntegerValues.size())
        maIntegerValues[eType] = nValue;
    else
    {
        OSL_ASSERT(eType>=0 && size_t(eType)<maIntegerValues.size());
    }
}




::rtl::OUString Theme::GetString (const StringType eType) const
{
    if (eType>=0 && size_t(eType)<maStrings.size())
        return maStrings[eType];
    else
    {
        OSL_ASSERT(eType>=0 && size_t(eType)<maStrings.size());
        return ::rtl::OUString();
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
