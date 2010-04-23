/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlsViewCacheContext.hxx,v $
 *
 * $Revision: 1.3 $
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

#define USE_SYSTEM_SELECTION_COLOR

namespace sd { namespace slidesorter { namespace view {

// Grays
#define Black 0x000000

// Reds
#define Amber 0xff7e00

// Greens
#define AndroidGreen 0xa4c639
#define AppleGreen 0x8db600
#define Asparagus 0x87a96b

// Blues
#define Azure 0x000fff
#define DarkCerulean 0x08457e
#define StellaBlue 0x009ee1
#define AirForceBlue 0x5d8aa8

// Off white
#define OldLace 0xfdf5e6
#define AntiqueWhite 0xfaebd7

// Off grays
#define Arsenic 0x3b444b


const static ColorData BackgroundColorData = 0xffffff;

const static ColorData gnMouseOverColor = 0x59000000 | StellaBlue;


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
    USHORT nHue (0);
    USHORT nSaturation (0);
    USHORT nBrightness (0);
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
      maColor(),
      mnButtonCornerRadius(3),
      mnButtonMaxAlpha(255 * 20/100),
      mnButtonPaintType(1),
      mnButtonBorder(4),
      mnButtonGap(8)

{
    {
        LocalResource aResource (RID_SLIDESORTER_ICONS);

        maStrings.resize(_StringType_Size_);
        maStrings[String_Unhide] = String(SdResId(STRING_UNHIDE));
        maStrings[String_DragAndDropPages] = String(SdResId(STRING_DRAG_AND_DROP_PAGES));
        maStrings[String_DragAndDropSlides] = String(SdResId(STRING_DRAG_AND_DROP_SLIDES));
        maStrings[String_Command1] = String(SdResId(STRING_COMMAND1));
        maStrings[String_Command2] = String(SdResId(STRING_COMMAND2));
        maStrings[String_Command3] = String(SdResId(STRING_COMMAND3));

        maColor.resize(_ColorType_Size_);
        maColor[Background] = maBackgroundColor;
        maColor[PageBackground] = AirForceBlue;
        maColor[ButtonBackground] = Black;
        maColor[ButtonText] = AntiqueWhite;
        maColor[MouseOverColor] = gnMouseOverColor;
        maColor[PageNumberBorder] = Azure;
        maColor[PageNumberColor] = 0x0848a8f;
        maColor[Selection] = StellaBlue;
        maColor[PreviewBorder] = 0x949599;
    }

    Update(rpProperties);
}




void Theme::Update (const ::boost::shared_ptr<controller::Properties>& rpProperties)
{
    maBackgroundColor = rpProperties->GetBackgroundColor().GetColor();
    maPageBackgroundColor = svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;

    maColor[Background] = maBackgroundColor;

    maGradients.resize(_GradientColorType_Size_);

#ifdef USE_SYSTEM_SELECTION_COLOR
    const ColorData aSelectionColor (rpProperties->GetSelectionColor().GetColor());

    SetGradient(Gradient_SelectedPage, aSelectionColor, 50, 50, +100,+100, +50,+25);
    SetGradient(Gradient_MouseOverPage, aSelectionColor, 75, 75, +100,+100, +50,+25);
    SetGradient(Gradient_SelectedAndFocusedPage, aSelectionColor, 50, 50, +100,0, -50,-75);
    SetGradient(Gradient_MouseOverSelectedAndFocusedPage, aSelectionColor, 75, 75, +100,0, -50,-75);
    SetGradient(Gradient_FocusedPage, aSelectionColor, -1,-1, 0,0, -50,-75);

#else

    maSelectedGradient.maFillColor1 = 0xb7daf0;
    maSelectedGradient.maFillColor2 = 0x6db5e1;
    maSelectedGradient.maBorderColor1 = 0x6db5e1;
    maSelectedGradient.maBorderColor2 = 0x0e85cd;

    maSelectedAndFocusedGradient.maFillColor1 = 0xb7daf0;
    maSelectedAndFocusedGradient.maFillColor2 = 0x6db5e1;
    maSelectedAndFocusedGradient.maBorderColor1 = 0x6db5e1;
    maSelectedAndFocusedGradient.maBorderColor2 = 0x0e85cd;

    maMouseOverGradient.maFillColor1 = 0x0e85cd;
    maMouseOverGradient.maFillColor2 = 0x044c99;
    maMouseOverGradient.maBorderColor1 = 0x6db5e1;
    maMouseOverGradient.maBorderColor2 = 0x0e85cd;
#endif

    SetGradient(Gradient_ButtonBackground, 0x000000, -1,-1, 0,0, 0,0);
    SetGradient(Gradient_NormalPage, maBackgroundColor, -1,-1, 0,0, 0,0);

    // The focused gradient needs special handling because its fill color is
    // like that of the NormalPage gradient.
    GetGradient(Gradient_FocusedPage).maFillColor1 = GetGradient(Gradient_NormalPage).maFillColor1;
    GetGradient(Gradient_FocusedPage).maFillColor2 = GetGradient(Gradient_NormalPage).maFillColor2;

    const bool bSavedHighContrastMode (mbIsHighContrastMode);
    mbIsHighContrastMode = rpProperties->IsHighContrastModeActive();
    if (bSavedHighContrastMode != mbIsHighContrastMode)
    {
        LocalResource aResource (RID_SLIDESORTER_ICONS);

        maIcons.resize(_IconType_Size_);
        if (mbIsHighContrastMode)
        {
            InitializeIcon(Icon_RawShadow, IMAGE_SHADOW);
            InitializeIcon(Icon_RawInsertShadow, IMAGE_INSERT_SHADOW);
            InitializeIcon(Icon_HideSlideOverlay, IMAGE_HIDE_SLIDE_OVERLAY);

            InitializeIcon(Icon_Command1Regular, IMAGE_COMMAND1_REGULAR_HC);
            InitializeIcon(Icon_Command1Hover, IMAGE_COMMAND1_HOVER_HC);
            InitializeIcon(Icon_Command1Small, IMAGE_COMMAND1_SMALL_HC);
            InitializeIcon(Icon_Command1SmallHover, IMAGE_COMMAND1_SMALL_HOVER_HC);

            InitializeIcon(Icon_Command2Regular, IMAGE_COMMAND2_REGULAR_HC);
            InitializeIcon(Icon_Command2Hover, IMAGE_COMMAND2_HOVER_HC);
            InitializeIcon(Icon_Command2Small, IMAGE_COMMAND2_SMALL_HC);
            InitializeIcon(Icon_Command2SmallHover, IMAGE_COMMAND2_SMALL_HOVER_HC);

            InitializeIcon(Icon_Command3Regular, IMAGE_COMMAND3_REGULAR_HC);
            InitializeIcon(Icon_Command3Hover, IMAGE_COMMAND3_HOVER_HC);
            InitializeIcon(Icon_Command3Small, IMAGE_COMMAND3_SMALL_HC);
            InitializeIcon(Icon_Command3SmallHover, IMAGE_COMMAND3_SMALL_HOVER_HC);
        }
        else
        {
            InitializeIcon(Icon_RawShadow, IMAGE_SHADOW);
            InitializeIcon(Icon_RawInsertShadow, IMAGE_INSERT_SHADOW);
            InitializeIcon(Icon_HideSlideOverlay, IMAGE_HIDE_SLIDE_OVERLAY);

            InitializeIcon(Icon_Command1Regular, IMAGE_COMMAND1_REGULAR);
            InitializeIcon(Icon_Command1Hover, IMAGE_COMMAND1_HOVER);
            InitializeIcon(Icon_Command1Small, IMAGE_COMMAND1_SMALL);
            InitializeIcon(Icon_Command1SmallHover, IMAGE_COMMAND1_SMALL_HOVER);

            InitializeIcon(Icon_Command2Regular, IMAGE_COMMAND2_REGULAR);
            InitializeIcon(Icon_Command2Hover, IMAGE_COMMAND2_HOVER);
            InitializeIcon(Icon_Command2Small, IMAGE_COMMAND2_SMALL);
            InitializeIcon(Icon_Command2SmallHover, IMAGE_COMMAND2_SMALL_HOVER);

            InitializeIcon(Icon_Command3Regular, IMAGE_COMMAND3_REGULAR);
            InitializeIcon(Icon_Command3Hover, IMAGE_COMMAND3_HOVER);
            InitializeIcon(Icon_Command3Small, IMAGE_COMMAND3_SMALL);
            InitializeIcon(Icon_Command3SmallHover, IMAGE_COMMAND3_SMALL_HOVER);
        }
    }
}




::boost::shared_ptr<Font> Theme::GetFont (
    const FontType eType,
    const OutputDevice& rDevice)
{
    ::boost::shared_ptr<Font> pFont;

    switch (eType)
    {
        case PageNumberFont:
            pFont.reset(new Font(Application::GetSettings().GetStyleSettings().GetAppFont()));
            pFont->SetTransparent(TRUE);
            pFont->SetWeight(WEIGHT_BOLD);
            break;

        case PageCountFont:
            pFont.reset(new Font(Application::GetSettings().GetStyleSettings().GetAppFont()));
            pFont->SetTransparent(TRUE);
            pFont->SetWeight(WEIGHT_NORMAL);
            {
                const Size aSize (pFont->GetSize());
                pFont->SetSize(Size(aSize.Width()*5/3, aSize.Height()*5/3));
            }
            break;

        case ButtonFont:
            pFont.reset(new Font(Application::GetSettings().GetStyleSettings().GetAppFont()));
            pFont->SetTransparent(TRUE);
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
    const ColorData aColor (HGBAdapt(aBaseColor, nSaturationOverride, nBrightnessOverride));

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




BitmapEx Theme::GetIcon (const IconType eType)
{
    if (eType>=0 && eType<maIcons.size())
        return maIcons[eType];
    else
    {
        OSL_ASSERT(eType>=0 && eType<maIcons.size());
        return BitmapEx();
    }
}




sal_Int32 Theme::GetIntegerValue (const IntegerValueType eType) const
{
    switch (eType)
    {
        case Integer_ButtonCornerRadius:
            return mnButtonCornerRadius;

        case Integer_ButtonMaxAlpha:
            return mnButtonMaxAlpha;

        case Integer_ButtonPaintType:
            return mnButtonPaintType;

        case Integer_ButtonBorder:
            return mnButtonBorder;

        case Integer_ButtonGap:
            return mnButtonGap;

        default:
            return 0;
    }
}




void Theme::SetIntegerValue (const IntegerValueType eType, const sal_Int32 nValue)
{
    switch (eType)
    {
        case Integer_ButtonCornerRadius:
            mnButtonCornerRadius = nValue;
            break;

        case Integer_ButtonMaxAlpha:
            mnButtonMaxAlpha = nValue;
            break;

        case Integer_ButtonPaintType:
            mnButtonPaintType = nValue;
            break;

        case Integer_ButtonBorder:
            mnButtonBorder = nValue;
            break;

        case Integer_ButtonGap:
            mnButtonGap = nValue;
            break;

        default:
            break;
    }
}




::rtl::OUString Theme::GetString (const StringType eType) const
{
    if (eType>=0 && eType<maStrings.size())
        return maStrings[eType];
    else
    {
        OSL_ASSERT(eType>=0 && eType<maStrings.size());
        return ::rtl::OUString();
    }
}




Theme::GradientDescriptor& Theme::GetGradient (const GradientColorType eType)
{
    if (eType>=0 && eType<maGradients.size())
        return maGradients[eType];
    else
    {
        OSL_ASSERT(eType>=0 && eType<maGradients.size());
        return maGradients[0];
    }
}




void Theme::InitializeIcon (const IconType eType, USHORT nResourceId)
{
    if (eType>=0 && eType<maIcons.size())
        maIcons[eType] = Image(SdResId(nResourceId)).GetBitmapEx();
    else
    {
        OSL_ASSERT(eType>=0 && eType<maIcons.size());
    }
}




} } } // end of namespace ::sd::slidesorter::view
