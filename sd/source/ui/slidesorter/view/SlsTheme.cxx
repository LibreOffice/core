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




Theme::Theme (const ::boost::shared_ptr<controller::Properties>& rpProperties)
    : maBackgroundColor(rpProperties->GetBackgroundColor().GetColor()),
      maPageBackgroundColor(COL_WHITE),
      maNormalGradient(),
      maSelectedGradient(),
      maSelectedAndFocusedGradient(),
      maMouseOverGradient(),
      maRawShadow(),
      maRawInsertShadow(),
      maHideSlideOverlay(),
      maStartPresentationIcon(),
      maShowSlideIcon(),
      maDuplicateSlideIcon(),
      maColor(PreviewBorder+1),
      mnButtonCornerRadius(3),
      mnButtonMaxAlpha(255 * 20/100),
      mnButtonPaintType(0),
      msUnhide(),
      msDragAndDropPages(),
      msDragAndDropSlides()

{
    LocalResource aResource (IMG_ICONS);

    maRawShadow = Image(SdResId(IMAGE_SHADOW)).GetBitmapEx();
    maRawInsertShadow = Image(SdResId(IMAGE_INSERT_SHADOW)).GetBitmapEx();
    maHideSlideOverlay  = Image(SdResId(IMAGE_HIDE_SLIDE_OVERLAY)).GetBitmapEx();
    maStartPresentationIcon = Image(SdResId(IMAGE_PRESENTATION)).GetBitmapEx();
    maShowSlideIcon = Image(SdResId(IMAGE_SHOW_SLIDE)).GetBitmapEx();
    maDuplicateSlideIcon = Image(SdResId(IMAGE_NEW_SLIDE)).GetBitmapEx();
    msUnhide = String(SdResId(STRING_UNHIDE));
    msDragAndDropPages = String(SdResId(STRING_DRAG_AND_DROP_PAGES));
    msDragAndDropSlides = String(SdResId(STRING_DRAG_AND_DROP_SLIDES));

    maColor.resize(PreviewBorder+1);
    maColor[Background] = maBackgroundColor;
    maColor[PageBackground] = AirForceBlue;
    maColor[ButtonBackground] = AirForceBlue;
    maColor[ButtonText] = AntiqueWhite;
    maColor[MouseOverColor] = gnMouseOverColor;
    maColor[PageNumberBorder] = Azure;
    maColor[PageNumberColor] = 0x0848a8f;
    maColor[Selection] = StellaBlue;
    maColor[PreviewBorder] = 0x949599;

    Update(rpProperties);
}




void Theme::Update (const ::boost::shared_ptr<controller::Properties>& rpProperties)
{
    maBackgroundColor = rpProperties->GetBackgroundColor().GetColor();
    maPageBackgroundColor = svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;

    maColor[Background] = maBackgroundColor;

#ifdef USE_SYSTEM_SELECTION_COLOR
    const ColorData aSelectionColor (rpProperties->GetSelectionColor().GetColor());

    SetGradient(SelectedPage, aSelectionColor, +50,-10, -10,-30);
    SetGradient(SelectedAndFocusedPage, aSelectionColor, +30,-30, -30,-50);
    SetGradient(MouseOverPage, aSelectionColor, +90,+30, +10,+30);

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

    SetGradient(NormalPage, maBackgroundColor, 0,0, 0,0);
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
    const sal_Int32 nFillStartOffset,
    const sal_Int32 nFillEndOffset,
    const sal_Int32 nBorderStartOffset,
    const sal_Int32 nBorderEndOffset)
{
    GradientDescriptor& rGradient (GetGradient(eType));

    rGradient.maBaseColor = aBaseColor;

    rGradient.maFillColor1 = ChangeLuminance(aBaseColor, nFillStartOffset);
    rGradient.maFillColor2 = ChangeLuminance(aBaseColor, nFillEndOffset);
    rGradient.maBorderColor1 = ChangeLuminance(aBaseColor, nBorderStartOffset);
    rGradient.maBorderColor2 = ChangeLuminance(aBaseColor, nBorderEndOffset);

    rGradient.mnFillOffset1 = nFillStartOffset;
    rGradient.mnFillOffset2 = nFillEndOffset;
    rGradient.mnBorderOffset1 = nBorderStartOffset;
    rGradient.mnBorderOffset2 = nBorderEndOffset;
}




BitmapEx Theme::GetIcon (const IconType eType)
{
    switch (eType)
    {
        case Icon_RawShadow:
            return maRawShadow;

        case Icon_RawInsertShadow:
            return maRawInsertShadow;

        case Icon_HideSlideOverlay:
            return maHideSlideOverlay;

        case Icon_StartPresentation:
            return maStartPresentationIcon;

        case Icon_ShowSlide:
            return maShowSlideIcon;

        case Icon_DuplicateSlide:
            return maDuplicateSlideIcon;

        default:
            return BitmapEx();
    }
}




sal_Int32 Theme::GetIntegerValue (const IntegerValueType eType) const
{
    switch (eType)
    {
        case ButtonCornerRadius:
            return mnButtonCornerRadius;

        case ButtonMaxAlpha:
            return mnButtonMaxAlpha;

        case ButtonPaintType:
            return mnButtonPaintType;

        default:
            return 0;
    }
}




void Theme::SetIntegerValue (const IntegerValueType eType, const sal_Int32 nValue)
{
    switch (eType)
    {
        case ButtonCornerRadius:
            mnButtonCornerRadius = nValue;
            break;

        case ButtonMaxAlpha:
            mnButtonMaxAlpha = nValue;
            break;

        case ButtonPaintType:
            mnButtonPaintType = nValue;
            break;

        default:
            break;
    }
}




::rtl::OUString Theme::GetString (const StringType eType) const
{
    switch (eType)
    {
        case String_Unhide: return msUnhide;
        case String_DragAndDropPages: return msDragAndDropPages;
        case String_DragAndDropSlides: return msDragAndDropSlides;
        default: return ::rtl::OUString();
    }
}




Theme::GradientDescriptor& Theme::GetGradient (const GradientColorType eType)
{
    switch(eType)
    {
        default:
            OSL_ASSERT(false);
            // fall through

        case NormalPage:
            return maNormalGradient;

        case SelectedPage:
            return maSelectedGradient;

        case SelectedAndFocusedPage:
            return maSelectedAndFocusedGradient;

        case MouseOverPage:
            return maMouseOverGradient;
    }
}



} } } // end of namespace ::sd::slidesorter::view
