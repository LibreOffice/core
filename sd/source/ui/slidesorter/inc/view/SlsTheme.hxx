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

#ifndef SD_SLIDESORTER_VIEW_THEME_HXX
#define SD_SLIDESORTER_VIEW_THEME_HXX

#include "model/SlsVisualState.hxx"

#include <vcl/bitmapex.hxx>
#include <vcl/font.hxx>
#include <vcl/gradient.hxx>
#include <tools/color.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>


namespace sd { namespace slidesorter { namespace controller {
class Properties;
} } }

namespace sd { namespace slidesorter { namespace view {


/** Collection of colors and styles that are used to paint the slide sorter
    view.
*/
class Theme
{
public:
    Theme (const ::boost::shared_ptr<controller::Properties>& rpProperties);

    /** Call this method to update some colors as response to a change of
        a system color change.
    */
    void Update (
        const ::boost::shared_ptr<controller::Properties>& rpProperties);

    //    BitmapEx GetInsertIndicatorIcon (void) const;

    enum FontType {
        Font_PageNumber,
        Font_PageCount,
        Font_Button
    };
    static ::boost::shared_ptr<Font> GetFont (
        const FontType eType,
        const OutputDevice& rDevice);

    enum ColorType {
        Color_Background,
        Color_ButtonBackground,
        Color_ButtonText,
        Color_ButtonTextHover,
        Color_PageNumberDefault,
        Color_PageNumberHover,
        Color_PageNumberHighContrast,
        Color_PageNumberBrightBackground,
        Color_PageNumberDarkBackground,
        Color_Selection,
        Color_PreviewBorder,
        Color_PageCountFontColor,
        _ColorType_Size_
    };
    ColorData GetColor (const ColorType eType);
    void SetColor (const ColorType eType, const ColorData aColorData);

    enum GradientColorType {
        Gradient_NormalPage,
        Gradient_SelectedPage,
        Gradient_SelectedAndFocusedPage,
        Gradient_MouseOverPage,
        Gradient_MouseOverSelectedAndFocusedPage,
        Gradient_FocusedPage,
        Gradient_ButtonBackground,
        _GradientColorType_Size_
    };
    enum GradientColorClass {
        Border1,
        Border2,
        Fill1,
        Fill2,
        Base
    };
    ColorData GetGradientColor (
        const GradientColorType eType,
        const GradientColorClass eClass);
    sal_Int32 GetGradientOffset (
        const GradientColorType eType,
        const GradientColorClass eClass);
    void SetGradient (
        const GradientColorType eType,
        const ColorData aBaseColor,
        const sal_Int32 nSaturationOverride,
        const sal_Int32 nBrightnessOverride,
        const sal_Int32 nFillStartOffset,
        const sal_Int32 nFillEndOffset,
        const sal_Int32 nBorderStartOffset,
        const sal_Int32 nBorderEndOffset);
    sal_Int32 GetGradientSaturationOverride (const GradientColorType eType);
    sal_Int32 GetGradientBrightnessOverride (const GradientColorType eType);
    void SetGradientSaturationOverride (const GradientColorType eType, const sal_Int32 nValue);
    void SetGradientBrightnessOverride (const GradientColorType eType, const sal_Int32 nValue);

    enum IconType
    {
        Icon_RawShadow,
        Icon_RawInsertShadow,
        Icon_HideSlideOverlay,
        Icon_FocusBorder,
        Icon_ButtonBarLarge,
        Icon_ButtonBarMedium,
        Icon_ButtonBarSmall,
        Icon_Command1Large,
        Icon_Command1LargeHover,
        Icon_Command1Medium,
        Icon_Command1MediumHover,
        Icon_Command1Small,
        Icon_Command1SmallHover,
        Icon_Command2Large,
        Icon_Command2LargeHover,
        Icon_Command2Medium,
        Icon_Command2MediumHover,
        Icon_Command2Small,
        Icon_Command2SmallHover,
        Icon_Command2BLarge,
        Icon_Command2BLargeHover,
        Icon_Command2BMedium,
        Icon_Command2BMediumHover,
        Icon_Command2BSmall,
        Icon_Command2BSmallHover,
        Icon_Command3Large,
        Icon_Command3LargeHover,
        Icon_Command3Medium,
        Icon_Command3MediumHover,
        Icon_Command3Small,
        Icon_Command3SmallHover,
        _IconType_Size_
    };
    const BitmapEx& GetIcon (const IconType eType);

    enum IntegerValueType
    {
        Integer_ButtonCornerRadius,
        Integer_ButtonMaxAlpha,
        Integer_ButtonBarMaxAlpha,
        Integer_ButtonPaintType,
        Integer_ButtonBorder,
        Integer_ButtonGap,
        Integer_ButtonFadeInDelay,
        Integer_ButtonFadeInDuration,
        Integer_ButtonFadeOutDelay,
        Integer_ButtonFadeOutDuration,
        Integer_ToolTipDelay,
        Integer_FocusIndicatorWidth,
        _IntegerValueType_Size_
    };
    sal_Int32 GetIntegerValue (const IntegerValueType eType) const;
    void SetIntegerValue (const IntegerValueType eType, const sal_Int32 nValue);

    enum StringType
    {
        String_Unhide,
        String_DragAndDropPages,
        String_DragAndDropSlides,
        String_Command1,
        String_Command2,
        String_Command2B,
        String_Command3,
        _StringType_Size_
    };
    ::rtl::OUString GetString (const StringType eType) const;

private:
    bool mbIsHighContrastMode;
    class GradientDescriptor
    {
    public:
        ColorData maBaseColor;

        sal_Int32 mnSaturationOverride;
        sal_Int32 mnBrightnessOverride;

        ColorData maFillColor1;
        ColorData maFillColor2;
        ColorData maBorderColor1;
        ColorData maBorderColor2;

        sal_Int32 mnFillOffset1;
        sal_Int32 mnFillOffset2;
        sal_Int32 mnBorderOffset1;
        sal_Int32 mnBorderOffset2;
    };
    ColorData maBackgroundColor;
    ColorData maPageBackgroundColor;
    ::std::vector<GradientDescriptor> maGradients;
    ::std::vector<BitmapEx> maIcons;
    ::std::vector<ColorData> maColor;
    ::std::vector<sal_Int32> maIntegerValues;
    ::std::vector<rtl::OUString> maStrings;

    GradientDescriptor& GetGradient (const GradientColorType eType);
    /** Guarded initialization of the specified icon in the maIcons
        container.  Call only while a LocalResource object is active.
    */
    void InitializeIcon (const IconType eType, sal_uInt16 nResourceId);
};


} } } // end of namespace ::sd::slidesorter::view

#endif
