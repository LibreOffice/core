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

#ifndef INCLUDED_SVGIO_INC_SVGTOKEN_HXX
#define INCLUDED_SVGIO_INC_SVGTOKEN_HXX

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
            TextAlign,
            PathLength,
            Type,
            Class,
            TextAnchor,
            XmlSpace,
            Color,
            ClipPathNode,
            ClipPathProperty,
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

            // structural elements
            Defs,
            G,
            Svg,
            Symbol,
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

            FlowRoot
        };

        SVGToken StrToSVGToken(const OUString& rStr, bool bCaseIndependent);

        OUString getStrTitle();
        OUString getStrDesc();

} // end of namespace svgio::svgreader

#endif // INCLUDED_SVGIO_INC_SVGTOKEN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
