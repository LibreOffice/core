/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/Primitive2DContainer.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>

#include <basegfx/color/bcolor.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <boost/container/vector.hpp>
#include <i18nlangtag/lang.h>
#include <rtl/ustring.hxx>
#include <vcl/rendercontext/State.hxx>

#include <optional>
#include <vector>

namespace vcl
{
class Font;
}

namespace drawinglayer::primitive2d
{
/// What a text draw takes from the device it draws on, besides the font: the colors, the layout
/// mode for the writing direction, and the language for the digits and the locale.
struct TextDrawState
{
    basegfx::BColor maTextColor;
    /// The color of an overline, or none to take the text color.
    std::optional<basegfx::BColor> moOverlineColor;
    /// The color of an underline and a strikeout, or none to take the text color.
    std::optional<basegfx::BColor> moTextLineColor;
    /// The color behind the text, or none for no fill.
    std::optional<basegfx::BColor> moTextFillColor;
    vcl::text::ComplexTextLayoutFlags meLayoutMode = vcl::text::ComplexTextLayoutFlags::Default;
    LanguageType meLanguage = LANGUAGE_SYSTEM;
    /// Where the text lies in the text it was laid out in, or none for text that is not part of
    /// laid-out paragraph text.
    std::optional<TextPortionLayout> moLayout;
};

/** The primitive of a text drawn at rStart in rFont with the advances in rDXArray: a plain text
    portion, a decorated one when the font has lines, emphasis marks, relief or a shadow or draws
    its lines under the words only, and a group with the fill behind the text when the state names
    a fill color. The text portion carries the layout that the state names. Empty for an empty
    range.
 */
DRAWINGLAYER_DLLPUBLIC Primitive2DReference createTextPrimitiveFromFont(
    const basegfx::B2DPoint& rStart, const OUString& rText, sal_Int32 nTextStart,
    sal_Int32 nTextLength, std::vector<double>&& rDXArray,
    boost::container::vector<bool>&& rKashidaArray, const vcl::Font& rFont,
    const TextDrawState& rState);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
