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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGTOKEN_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGTOKEN_HXX

#include <rtl/ustring.hxx>
#include <boost/unordered_map.hpp>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        // SVG token mapper with hashing
        enum SVGToken
        {
            SVGTokenUnknown = 0,

            // diverse attribute tokens
            SVGTokenWidth,
            SVGTokenHeight,
            SVGTokenViewBox,
            SVGTokenTransform,
            SVGTokenStyle,
            SVGTokenD,
            SVGTokenX,
            SVGTokenY,
            SVGTokenXmlns,
            SVGTokenVersion,
            SVGTokenId,
            SVGTokenRx,
            SVGTokenRy,
            SVGTokenPoints,
            SVGTokenDx,
            SVGTokenDy,
            SVGTokenRotate,
            SVGTokenTextLength,
            SVGTokenLengthAdjust,
            SVGTokenFont,
            SVGTokenFontFamily,
            SVGTokenFontSize,
            SVGTokenFontSizeAdjust,
            SVGTokenFontStretch,
            SVGTokenFontStyle,
            SVGTokenFontVariant,
            SVGTokenFontWeight,
            SVGTokenDirection,
            SVGTokenLetterSpacing,
            SVGTokenTextDecoration,
            SVGTokenUnicodeBidi,
            SVGTokenWordSpacing,
            SVGTokenCharacter, // not in the hash, just for simple text handling in SvgCharacterNode
            SVGTokenTspan,
            SVGTokenTref,
            SVGTokenTextPath,
            SVGTokenStartOffset,
            SVGTokenMethod,
            SVGTokenSpacing,
            SVGTokenTextAlign,
            SVGTokenPathLength,
            SVGTokenType,
            SVGTokenClass,
            SVGTokenTextAnchor,
            SVGTokenXmlSpace,
            SVGTokenColor,
            SVGTokenClipPathNode,
            SVGTokenClipPathProperty,
            SVGTokenMask,
            SVGTokenClipPathUnits,
            SVGTokenMaskUnits,
            SVGTokenMaskContentUnits,
            SVGTokenClipRule,
            SVGTokenMarker,
            SVGTokenMarkerStart,
            SVGTokenMarkerMid,
            SVGTokenMarkerEnd,
            SVGTokenRefX,
            SVGTokenRefY,
            SVGTokenMarkerUnits,
            SVGTokenMarkerWidth,
            SVGTokenMarkerHeight,
            SVGTokenOrient,
            SVGTokenPattern,
            SVGTokenPatternUnits,
            SVGTokenPatternContentUnits,
            SVGTokenPatternTransform,
            SVGTokenOpacity,
            SVGTokenTitle,
            SVGTokenDesc,

            // AspectRatio and params
            SVGTokenPreserveAspectRatio,
            SVGTokenDefer,
            SVGTokenNone,
            SVGTokenXMinYMin,
            SVGTokenXMidYMin,
            SVGTokenXMaxYMin,
            SVGTokenXMinYMid,
            SVGTokenXMidYMid,
            SVGTokenXMaxYMid,
            SVGTokenXMinYMax,
            SVGTokenXMidYMax,
            SVGTokenXMaxYMax,
            SVGTokenMeet,
            SVGTokenSlice,

            // structural elements
            SVGTokenDefs,
            SVGTokenG,
            SVGTokenSvg,
            SVGTokenSymbol,
            SVGTokenUse,

            // shape elements
            SVGTokenCircle,
            SVGTokenEllipse,
            SVGTokenLine,
            SVGTokenPath,
            SVGTokenPolygon,
            SVGTokenPolyline,
            SVGTokenRect,
            SVGTokenImage,

            // gradient elements and tokens
            SVGTokenLinearGradient,
            SVGTokenRadialGradient,
            SVGTokenStop,
            SVGTokenOffset,
            SVGTokenX1,
            SVGTokenY1,
            SVGTokenX2,
            SVGTokenY2,
            SVGTokenCx,
            SVGTokenCy,
            SVGTokenFx,
            SVGTokenFy,
            SVGTokenR,
            SVGTokenGradientUnits,
            SVGTokenGradientTransform,
            SVGTokenSpreadMethod,
            SVGTokenXlinkHref,
            SVGTokenStopColor,
            SVGTokenStopOpacity,

            // fill tokens
            SVGTokenFill,
            SVGTokenFillOpacity,
            SVGTokenFillRule,

            // stroke tokens
            SVGTokenStroke,
            SVGTokenStrokeDasharray,
            SVGTokenStrokeDashoffset,
            SVGTokenStrokeLinecap,
            SVGTokenStrokeLinejoin,
            SVGTokenStrokeMiterlimit,
            SVGTokenStrokeOpacity,
            SVGTokenStrokeWidth,

            // text tokens
            SVGTokenText,

            SVGTokenLast
        };

        SVGToken StrToSVGToken(const rtl::OUString& rStr);

        const rtl::OUString& getStrTitle();
        const rtl::OUString& getStrDesc();
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGTOKEN_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
