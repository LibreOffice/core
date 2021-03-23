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

#pragma once

#include <vcl/bitmapex.hxx>
#include <tools/color.hxx>

#include <memory>

namespace vcl { class Font; }

namespace sd::slidesorter::controller { class Properties; }

namespace sd::slidesorter::view {

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
        ColorType_Size_
    };
    Color GetColor (const ColorType eType);

    enum GradientColorType {
        Gradient_NormalPage,
        Gradient_SelectedPage,
        Gradient_SelectedAndFocusedPage,
        Gradient_MouseOverPage,
        Gradient_MouseOverSelected,
        Gradient_MouseOverSelectedAndFocusedPage,
        Gradient_FocusedPage,
        GradientColorType_Size_
    };
    enum class GradientColorClass {
        Border1,
        Border2,
        Fill1,
        Fill2
    };
    Color GetGradientColor (
        const GradientColorType eType,
        const GradientColorClass eClass);
    void SetGradient (
        const GradientColorType eType,
        const Color aBaseColor,
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
        IconType_Size_
    };
    const BitmapEx& GetIcon (const IconType eType);

private:
    class GradientDescriptor
    {
    public:
        Color maFillColor1;
        Color maFillColor2;
        Color maBorderColor1;
        Color maBorderColor2;
    };
    Color maBackgroundColor;
    ::std::vector<GradientDescriptor> maGradients;
    ::std::vector<BitmapEx> maIcons;
    ::std::vector<Color> maColor;

    GradientDescriptor& GetGradient (const GradientColorType eType);
    /** Guarded initialization of the specified icon in the maIcons
        container.
    */
    void InitializeIcon(const IconType eType, const OUString& rResourceId);
};

} // end of namespace ::sd::slidesorter::view

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
