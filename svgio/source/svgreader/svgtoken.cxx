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

#include <svgtoken.hxx>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_map.h>
#include <o3tl/string_view.hxx>

namespace svgio::svgreader
{

constexpr auto aSVGTokenMap = frozen::make_unordered_map<std::u16string_view, SVGToken>(
{
    { u"width", SVGToken::Width },
    { u"height", SVGToken::Height },
    { u"viewBox", SVGToken::ViewBox },
    { u"transform", SVGToken::Transform },
    { u"style", SVGToken::Style },
    { u"display", SVGToken::Display }, // #i121656#
    { u"d", SVGToken::D },
    { u"x", SVGToken::X },
    { u"y", SVGToken::Y },
    { u"xmlns", SVGToken::Xmlns },
    { u"version", SVGToken::Version },
    { u"id", SVGToken::Id },
    { u"in", SVGToken::In },
    { u"rx", SVGToken::Rx },
    { u"ry", SVGToken::Ry },
    { u"points", SVGToken::Points },
    { u"dx", SVGToken::Dx },
    { u"dy", SVGToken::Dy },
    { u"rotate", SVGToken::Rotate },
    { u"textLength", SVGToken::TextLength },
    { u"lengthAdjust", SVGToken::LengthAdjust },
    { u"font", SVGToken::Font },
    { u"font-family", SVGToken::FontFamily },
    { u"font-size", SVGToken::FontSize },
    { u"font-size-adjust", SVGToken::FontSizeAdjust },
    { u"font-stretch", SVGToken::FontStretch },
    { u"font-style", SVGToken::FontStyle },
    { u"font-variant", SVGToken::FontVariant },
    { u"font-weight", SVGToken::FontWeight },
    { u"direction", SVGToken::Direction },
    { u"letter-spacing", SVGToken::LetterSpacing },
    { u"text-decoration", SVGToken::TextDecoration },
    { u"unicode-bidi", SVGToken::UnicodeBidi },
    { u"word-spacing", SVGToken::WordSpacing },
    { u"tspan", SVGToken::Tspan },
    { u"tref", SVGToken::Tref },
    { u"textPath", SVGToken::TextPath },
    { u"startOffset", SVGToken::StartOffset },
    { u"method", SVGToken::Method },
    { u"spacing", SVGToken::Spacing },
    { u"stdDeviation", SVGToken::StdDeviation },
    { u"text-align", SVGToken::TextAlign },
    { u"pathLength", SVGToken::PathLength },
    { u"type", SVGToken::Type },
    { u"class", SVGToken::Class },
    { u"text-anchor", SVGToken::TextAnchor },
    { u"xml:space", SVGToken::XmlSpace },
    { u"color", SVGToken::Color },
    { u"clipPath", SVGToken::ClipPathNode },
    { u"clip-path", SVGToken::ClipPathProperty },
    { u"feColorMatrix", SVGToken::FeColorMatrix },
    { u"feDropShadow", SVGToken::FeDropShadow },
    { u"feFlood", SVGToken::FeFlood },
    { u"feImage", SVGToken::FeImage },
    { u"feGaussianBlur", SVGToken::FeGaussianBlur },
    { u"feOffset", SVGToken::FeOffset },
    { u"filter", SVGToken::Filter },
    { u"flood-color", SVGToken::FloodColor },
    { u"flood-opacity", SVGToken::FloodOpacity },
    { u"mask", SVGToken::Mask },
    { u"clipPathUnits", SVGToken::ClipPathUnits },
    { u"maskUnits", SVGToken::MaskUnits },
    { u"maskContentUnits", SVGToken::MaskContentUnits },
    { u"clip-rule", SVGToken::ClipRule },
    { u"marker", SVGToken::Marker },
    { u"marker-start", SVGToken::MarkerStart },
    { u"marker-mid", SVGToken::MarkerMid },
    { u"marker-end", SVGToken::MarkerEnd },
    { u"refX", SVGToken::RefX },
    { u"refY", SVGToken::RefY },
    { u"markerUnits", SVGToken::MarkerUnits },
    { u"markerWidth", SVGToken::MarkerWidth },
    { u"markerHeight", SVGToken::MarkerHeight },
    { u"orient", SVGToken::Orient },
    { u"pattern", SVGToken::Pattern },
    { u"patternUnits", SVGToken::PatternUnits },
    { u"patternContentUnits", SVGToken::PatternContentUnits },
    { u"patternTransform", SVGToken::PatternTransform },
    { u"opacity", SVGToken::Opacity },
    { u"visibility", SVGToken::Visibility },
    { u"title", SVGToken::Title },
    { u"desc", SVGToken::Desc },
    { u"overflow", SVGToken::Overflow },
    { u"preserveAspectRatio", SVGToken::PreserveAspectRatio },
    { u"defer", SVGToken::Defer },
    { u"none", SVGToken::None },
    { u"xMinYMin", SVGToken::XMinYMin },
    { u"xMidYMin", SVGToken::XMidYMin },
    { u"xMaxYMin", SVGToken::XMaxYMin },
    { u"xMinYMid", SVGToken::XMinYMid },
    { u"xMidYMid", SVGToken::XMidYMid },
    { u"xMaxYMid", SVGToken::XMaxYMid },
    { u"xMinYMax", SVGToken::XMinYMax },
    { u"xMidYMax", SVGToken::XMidYMax },
    { u"xMaxYMax", SVGToken::XMaxYMax },
    { u"meet", SVGToken::Meet },
    { u"slice", SVGToken::Slice },
    { u"values", SVGToken::Values },

    { u"defs", SVGToken::Defs },
    { u"g", SVGToken::G },
    { u"svg", SVGToken::Svg },
    { u"symbol", SVGToken::Symbol },
    { u"switch", SVGToken::Switch },
    { u"use", SVGToken::Use },
    { u"a", SVGToken::A },

    { u"circle", SVGToken::Circle },
    { u"ellipse", SVGToken::Ellipse },
    { u"line", SVGToken::Line },
    { u"path", SVGToken::Path },
    { u"polygon", SVGToken::Polygon },
    { u"polyline", SVGToken::Polyline },
    { u"rect", SVGToken::Rect },
    { u"image", SVGToken::Image },

    { u"linearGradient", SVGToken::LinearGradient },
    { u"radialGradient", SVGToken::RadialGradient },
    { u"stop", SVGToken::Stop },
    { u"offset", SVGToken::Offset },
    { u"x1", SVGToken::X1 },
    { u"y1", SVGToken::Y1 },
    { u"x2", SVGToken::X2 },
    { u"y2", SVGToken::Y2 },
    { u"cx", SVGToken::Cx },
    { u"cy", SVGToken::Cy },
    { u"fx", SVGToken::Fx },
    { u"fy", SVGToken::Fy },
    { u"r", SVGToken::R },
    { u"gradientUnits", SVGToken::GradientUnits },
    { u"gradientTransform", SVGToken::GradientTransform },
    { u"spreadMethod", SVGToken::SpreadMethod },
    { u"href", SVGToken::Href },
    { u"xlink:href", SVGToken::XlinkHref },
    { u"stop-color", SVGToken::StopColor },
    { u"stop-opacity", SVGToken::StopOpacity },

    { u"fill", SVGToken::Fill },
    { u"fill-opacity", SVGToken::FillOpacity },
    { u"fill-rule", SVGToken::FillRule },

    { u"stroke", SVGToken::Stroke },
    { u"stroke-dasharray", SVGToken::StrokeDasharray },
    { u"stroke-dashoffset", SVGToken::StrokeDashoffset },
    { u"stroke-linecap", SVGToken::StrokeLinecap },
    { u"stroke-linejoin", SVGToken::StrokeLinejoin },
    { u"stroke-miterlimit", SVGToken::StrokeMiterlimit },
    { u"stroke-opacity", SVGToken::StrokeOpacity },
    { u"stroke-width", SVGToken::StrokeWidth },

    { u"text", SVGToken::Text },
    { u"baseline-shift", SVGToken::BaselineShift },
    { u"dominant-baseline", SVGToken::DominantBaseline }
});

SVGToken StrToSVGToken(std::u16string_view rStr, bool bIgnoreCase)
{
    std::u16string_view aStr = rStr.starts_with(u"svg:") ? rStr.substr(4) : rStr;

    // TODO: a better alternative to the bIgnoreCase would be separate maps for SVG and CSS,
    // the latter using case-insensitive hasher and comparator, with separate search functions.

    auto it = bIgnoreCase ? std::find_if(aSVGTokenMap.begin(), aSVGTokenMap.end(),
                                         [aStr](const auto& el)
                                         { return o3tl::equalsIgnoreAsciiCase(el.first, aStr); })
                          : aSVGTokenMap.find(aStr);
    if (it != aSVGTokenMap.end())
        return it->second;

    return SVGToken::Unknown;
}

OUString SVGTokenToStr(const SVGToken& rToken)
{
    auto it = std::find_if(aSVGTokenMap.begin(), aSVGTokenMap.end(),
                           [rToken](const auto& el) { return el.second == rToken; });
    if (it != aSVGTokenMap.end())
        return OUString(it->first);

    return OUString();
}

} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
