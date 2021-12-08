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

#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <tools/fontenum.hxx>
#include <i18nlangtag/lang.h>
#include <o3tl/typed_flags_set.hxx>

#include <vcl/rendercontext/RasterOp.hxx>
#include <vcl/font.hxx>
#include <vcl/mapmod.hxx>
#include <vcl/region.hxx>
#include <vcl/vclenum.hxx>

#include <memory>
#include <optional>

namespace vcl
{
// Flags for OutputDevice::Push() and State
enum class PushFlags
{
    NONE = 0x0000,
    LINECOLOR = 0x0001,
    FILLCOLOR = 0x0002,
    FONT = 0x0004,
    TEXTCOLOR = 0x0008,
    MAPMODE = 0x0010,
    CLIPREGION = 0x0020,
    RASTEROP = 0x0040,
    TEXTFILLCOLOR = 0x0080,
    TEXTALIGN = 0x0100,
    REFPOINT = 0x0200,
    TEXTLINECOLOR = 0x0400,
    TEXTLAYOUTMODE = 0x0800,
    TEXTLANGUAGE = 0x1000,
    OVERLINECOLOR = 0x2000,
    FILLMODE = 0x4000,
    ALL = 0xFFFF
};
}

namespace o3tl
{
template <> struct typed_flags<vcl::PushFlags> : is_typed_flags<vcl::PushFlags, 0xFFFF>
{
};
}
#define PUSH_ALLFONT                                                                               \
    (vcl::PushFlags::TEXTCOLOR | vcl::PushFlags::TEXTFILLCOLOR | vcl::PushFlags::TEXTLINECOLOR     \
     | vcl::PushFlags::OVERLINECOLOR | vcl::PushFlags::TEXTALIGN | vcl::PushFlags::TEXTLAYOUTMODE  \
     | vcl::PushFlags::TEXTLANGUAGE | vcl::PushFlags::FONT)

namespace vcl::text
{
// Layout flags for Complex Text Layout
// These are flag values, i.e they can be combined
enum class ComplexTextLayoutFlags
{
    Default = 0x0000,
    BiDiRtl = 0x0001,
    BiDiStrong = 0x0002,
    TextOriginLeft = 0x0004,
    TextOriginRight = 0x0008
};
}

namespace o3tl
{
template <>
struct typed_flags<vcl::text::ComplexTextLayoutFlags>
    : is_typed_flags<vcl::text::ComplexTextLayoutFlags, 0x000f>
{
};
}

namespace vcl
{
struct State
{
    State() = default;
    State(State&&) = default;

    std::unique_ptr<vcl::Region> mpClipRegion;
    std::optional<MapMode> mpMapMode;
    std::optional<vcl::Font> mpFont;
    std::optional<Point> mpRefPoint;
    std::optional<Color> mpLineColor;
    std::optional<Color> mpFillColor;
    std::optional<Color> mpTextColor;
    std::optional<Color> mpTextFillColor;
    std::optional<Color> mpTextLineColor;
    std::optional<Color> mpOverlineColor;
    TextAlign meTextAlign = ALIGN_TOP;
    RasterOp meRasterOp = RasterOp::OverPaint;
    text::ComplexTextLayoutFlags mnTextLayoutMode = text::ComplexTextLayoutFlags::Default;
    LanguageType meTextLanguage = LANGUAGE_SYSTEM;
    PushFlags mnFlags = PushFlags::NONE;
    bool mbMapActive = false;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
