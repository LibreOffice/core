/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef INCLUDED_SVGIO_SVGREADER_SVGTOKEN_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGTOKEN_HXX

#include <svgio/svgiodllapi.h>
#include <rtl/ustring.hxx>
#include <hash_map>

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
            SVGTokenDisplay, // #121656#
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
