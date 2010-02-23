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
#include "SlsIcons.hxx"
#include "controller/SlsProperties.hxx"
#include "sdresid.hxx"
#include <tools/color.hxx>
#include <vcl/outdev.hxx>
#include <vcl/image.hxx>

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

const static double gnCornerRadius = 4.0;


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
      maNormalGradient(),
      maSelectedGradient(),
      maSelectedAndFocusedGradient(),
      maMouseOverGradient(),
      maRawShadow(),
      maInsertionIndicator()
{
    LocalResource aResource (IMG_ICONS);

    maRawShadow = Image(SdResId(IMAGE_SHADOW)).GetBitmapEx();
    maInsertionIndicator = Image(SdResId(IMAGE_INSERTION_INDICATOR_SELECT)).GetBitmapEx();

    Update(rpProperties);
}




void Theme::Update (const ::boost::shared_ptr<controller::Properties>& rpProperties)
{
    maBackgroundColor = rpProperties->GetBackgroundColor().GetColor();
#ifdef USE_SYSTEM_SELECTION_COLOR
    const ColorData aSelectionColor (rpProperties->GetSelectionColor().GetColor());

    maSelectedGradient.maFillColor1 = ChangeLuminance(aSelectionColor, +50);
    maSelectedGradient.maFillColor2 = ChangeLuminance(aSelectionColor, -10);
    maSelectedGradient.maBorderColor1 = ChangeLuminance(aSelectionColor, -10);
    maSelectedGradient.maBorderColor2 = ChangeLuminance(aSelectionColor, -30);

    maSelectedAndFocusedGradient.maFillColor1 = ChangeLuminance(aSelectionColor, +30);
    maSelectedAndFocusedGradient.maFillColor2 = ChangeLuminance(aSelectionColor, -30);
    maSelectedAndFocusedGradient.maBorderColor1 = ChangeLuminance(aSelectionColor, -30);
    maSelectedAndFocusedGradient.maBorderColor2 = ChangeLuminance(aSelectionColor, -50);

    maMouseOverGradient.maFillColor1 = ChangeLuminance(aSelectionColor, +90);
    maMouseOverGradient.maFillColor2 = ChangeLuminance(aSelectionColor, +30);
    maMouseOverGradient.maBorderColor1 = ChangeLuminance(aSelectionColor, +10);
    maMouseOverGradient.maBorderColor2 = ChangeLuminance(aSelectionColor, +30);

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

    maNormalGradient.maFillColor1 = maBackgroundColor;
    maNormalGradient.maFillColor2 = maBackgroundColor;
    maNormalGradient.maBorderColor1 = maBackgroundColor;
    maNormalGradient.maBorderColor2 = maBackgroundColor;
}




::boost::shared_ptr<Font> Theme::CreateFont (
    const FontType eType,
    OutputDevice& rDevice) const
{
    ::boost::shared_ptr<Font> pFont;

    switch (eType)
    {
        case PageNumberFont:
            pFont.reset(new Font(rDevice.GetFont()));
            pFont->SetWeight(WEIGHT_BOLD);
            break;
    }

    return pFont;
}




ColorData Theme::GetColorForVisualState (const model::VisualState::State eState) const
{
    ColorData nColor;
    switch (eState)
    {
        case model::VisualState::VS_Selected:
            nColor = 0x80000000 | StellaBlue;
            break;

        case model::VisualState::VS_Focused:
            nColor = AndroidGreen;
            break;

        case model::VisualState::VS_Current:
            nColor = 0x80000000 | StellaBlue;
            //            aColor = mpProperties->GetSelectionColor();
            break;

        case model::VisualState::VS_Excluded:
            nColor = 0xcc929ca2;
            break;

        case model::VisualState::VS_None:
        default:
            nColor = 0x80000000 | AntiqueWhite;
            break;
    }

    return nColor;
}




ColorData Theme::GetColor (const ColorType eType)
{
    switch(eType)
    {
        case Background:
            return maBackgroundColor;

        case ButtonBackground:
            return AirForceBlue;

        case MouseOverColor:
            return gnMouseOverColor;

        case PageNumberBorder:
            return Azure;

        case PageNumberColor:
            return 0x0848a8f;

        case Selection:
            return StellaBlue;
    }
    return 0;
}




ColorData Theme::GetGradientColor (
    const GradientColorType eType,
    const GradientColorClass eClass)
{
    GradientDescriptor* pDescriptor = NULL;
    switch(eType)
    {
        case NormalPage:
            pDescriptor = &maNormalGradient;
            break;

        case SelectedPage:
            pDescriptor = &maSelectedGradient;
            break;

        case SelectedAndFocusedPage:
            pDescriptor = &maSelectedAndFocusedGradient;
            break;

        case MouseOverPage:
            pDescriptor = &maMouseOverGradient;
            break;

        default:
            OSL_ASSERT(false);
            break;
    }

    if (pDescriptor != NULL)
    {
        switch (eClass)
        {
            case Border1: return pDescriptor->maBorderColor1;
            case Border2: return pDescriptor->maBorderColor2;
            case Fill1: return pDescriptor->maFillColor1;
            case Fill2: return pDescriptor->maFillColor2;

            default:
                OSL_ASSERT(false);
                break;
        }
    }
    return 0;
}




BitmapEx Theme::GetIcon (const IconType eType)
{
    switch (eType)
    {
        case InsertionIndicator:
            return maInsertionIndicator;

        case RawShadow:
            return maRawShadow;

        default:
            return BitmapEx();
    }
}

} } } // end of namespace ::sd::slidesorter::view
