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

#include <rtl/ustring.hxx>

namespace svgio::svgreader
    {
        // SVG token mapper with hashing
        enum class SVGToken
        {
            Unknown = 0,

            // diverse attribute tokens
            Width,
            Height,
            ViewBox,
            Transform,
            Style,
            Display, // #i121656#
            D,
            X,
            Y,
            Xmlns,
            Version,
            Id,
            In,
            Rx,
            Ry,
            Points,
            Dx,
            Dy,
            Rotate,
            TextLength,
            LengthAdjust,
            Font,
            FontFamily,
            FontSize,
            FontSizeAdjust,
            FontStretch,
            FontStyle,
            FontVariant,
            FontWeight,
            Direction,
            LetterSpacing,
            TextDecoration,
            UnicodeBidi,
            WordSpacing,
            Character, // not in the hash, just for simple text handling in SvgCharacterNode
            Tspan,
            Tref,
            TextPath,
            StartOffset,
            Method,
            Spacing,
            StdDeviation,
            TextAlign,
            PathLength,
            Type,
            Class,
            TextAnchor,
            XmlSpace,
            Color,
            ClipPathNode,
            ClipPathProperty,
            FeColorMatrix,
            FeDropShadow,
            FeFlood,
            FeImage,
            FeGaussianBlur,
            FeOffset,
            Filter,
            FloodColor,
            FloodOpacity,
            Mask,
            ClipPathUnits,
            MaskUnits,
            MaskContentUnits,
            ClipRule,
            Marker,
            MarkerStart,
            MarkerMid,
            MarkerEnd,
            RefX,
            RefY,
            MarkerUnits,
            MarkerWidth,
            MarkerHeight,
            Orient,
            Pattern,
            PatternUnits,
            PatternContentUnits,
            PatternTransform,
            Opacity,
            Visibility,
            Title,
            Desc,
            Overflow,

            // AspectRatio and params
            PreserveAspectRatio,
            Defer,
            None,
            XMinYMin,
            XMidYMin,
            XMaxYMin,
            XMinYMid,
            XMidYMid,
            XMaxYMid,
            XMinYMax,
            XMidYMax,
            XMaxYMax,
            Meet,
            Slice,
            Values,

            // structural elements
            Defs,
            G,
            Svg,
            Symbol,
            Switch,
            Use,
            A,

            // shape elements
            Circle,
            Ellipse,
            Line,
            Path,
            Polygon,
            Polyline,
            Rect,
            Image,

            // gradient elements and tokens
            LinearGradient,
            RadialGradient,
            Stop,
            Offset,
            X1,
            Y1,
            X2,
            Y2,
            Cx,
            Cy,
            Fx,
            Fy,
            R,
            GradientUnits,
            GradientTransform,
            SpreadMethod,
            Href,
            XlinkHref,
            StopColor,
            StopOpacity,

            // fill tokens
            Fill,
            FillOpacity,
            FillRule,

            // stroke tokens
            Stroke,
            StrokeDasharray,
            StrokeDashoffset,
            StrokeLinecap,
            StrokeLinejoin,
            StrokeMiterlimit,
            StrokeOpacity,
            StrokeWidth,

            // text tokens
            Text,
            BaselineShift,
            DominantBaseline
        };

        SVGToken StrToSVGToken(std::u16string_view rStr, bool bIgnoreCase);
        OUString SVGTokenToStr(const SVGToken& rToken);

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
