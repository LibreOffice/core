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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSTHEME_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESORTER_INC_VIEW_SLSTHEME_HXX

#include "model/SlsVisualState.hxx"

#include <vcl/bitmapex.hxx>
#include <vcl/font.hxx>
#include <vcl/gradient.hxx>
#include <tools/color.hxx>

#include <memory>

namespace sd { namespace slidesorter { namespace controller {
class Properties;
} } }

namespace sd { namespace slidesorter { namespace view {

const int Theme_FocusIndicatorWidth = 3;

/** Collection of colors and styles that are used to paint the slide sorter
    view.
*/
class Theme
{
public:
    Theme (const std::shared_ptr<controller::Properties>& rpProperties);

    /** Call this method to update some colors as response to a change of
        a system color change.
    */
    void Update (
        const std::shared_ptr<controller::Properties>& rpProperties);

    //    BitmapEx GetInsertIndicatorIcon() const;

    enum FontType {
        Font_PageNumber,
        Font_PageCount
    };
    static std::shared_ptr<vcl::Font> GetFont (
        const FontType eType,
        const OutputDevice& rDevice);

    enum ColorType {
        Color_Background,
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

    enum GradientColorType {
        Gradient_NormalPage,
        Gradient_SelectedPage,
        Gradient_SelectedAndFocusedPage,
        Gradient_MouseOverPage,
        Gradient_MouseOverSelected,
        Gradient_MouseOverSelectedAndFocusedPage,
        Gradient_FocusedPage,
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
    void SetGradient (
        const GradientColorType eType,
        const ColorData aBaseColor,
        const sal_Int32 nSaturationOverride,
        const sal_Int32 nBrightnessOverride,
        const sal_Int32 nFillStartOffset,
        const sal_Int32 nFillEndOffset,
        const sal_Int32 nBorderStartOffset,
        const sal_Int32 nBorderEndOffset);

    enum IconType
    {
        Icon_RawShadow,
        Icon_RawInsertShadow,
        Icon_HideSlideOverlay,
        Icon_FocusBorder,
        _IconType_Size_
    };
    const BitmapEx& GetIcon (const IconType eType);

private:
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

    GradientDescriptor& GetGradient (const GradientColorType eType);
    /** Guarded initialization of the specified icon in the maIcons
        container.  Call only while a LocalResource object is active.
    */
    void InitializeIcon (const IconType eType, sal_uInt16 nResourceId);
};

} } } // end of namespace ::sd::slidesorter::view

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
