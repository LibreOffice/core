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
#include <unordered_map>

namespace svgio
{
    namespace svgreader
    {
        static const char aSVGStrWidth[] = "width";
        static const char aSVGStrHeight[] = "height";
        static const char aSVGStrViewBox[] = "viewBox";
        static const char aSVGStrTransform[] = "transform";
        static const char aSVGStrStyle[] = "style";
        static const char aSVGStrDisplay[] = "display"; // #i121656#
        static const char aSVGStrD[] = "d";
        static const char aSVGStrX[] = "x";
        static const char aSVGStrY[] = "y";
        static const char aSVGStrXmlns[] = "xmlns";
        static const char aSVGStrVersion[] = "version";
        static const char aSVGStrId[] = "id";
        static const char aSVGStrRx[] = "rx";
        static const char aSVGStrRy[] = "ry";
        static const char aSVGStrPoints[] = "points";
        static const char aSVGStrDx[] = "dx";
        static const char aSVGStrDy[] = "dy";
        static const char aSVGStrRotate[] = "rotate";
        static const char aSVGStrTextLength[] = "textLength";
        static const char aSVGStrLengthAdjust[] = "lengthAdjust";
        static const char aSVGStrFont[] = "font";
        static const char aSVGStrFontFamily[] = "font-family";
        static const char aSVGStrFontSize[] = "font-size";
        static const char aSVGStrFontSizeAdjust[] = "font-size-adjust";
        static const char aSVGStrFontStretch[] = "font-stretch";
        static const char aSVGStrFontStyle[] = "font-style";
        static const char aSVGStrFontVariant[] = "font-variant";
        static const char aSVGStrFontWeight[] = "font-weight";
        static const char aSVGStrDirection[] = "direction";
        static const char aSVGStrLetterSpacing[] = "letter-spacing";
        static const char aSVGStrTextDecoration[] = "text-decoration";
        static const char aSVGStrUnicodeBidi[] = "unicode-bidi";
        static const char aSVGStrWordSpacing[] = "word-spacing";
        static const char aSVGStrTspan[] = "tspan";
        static const char aSVGStrTref[] = "tref";
        static const char aSVGStrTextPath[] = "textPath";
        static const char aSVGStrStartOffset[] = "startOffset";
        static const char aSVGStrMethod[] = "method";
        static const char aSVGStrSpacing[] = "spacing";
        static const char aSVGStrTextAlign[] = "text-align";
        static const char aSVGStrPathLength[] = "pathLength";
        static const char aSVGStrType[] = "type";
        static const char aSVGStrClass[] = "class";
        static const char aSVGStrTextAnchor[] = "text-anchor";
        static const char aSVGStrXmlSpace[] = "xml:space";
        static const char aSVGStrColor[] = "color";
        static const char aSVGStrClipPathNode[] = "clipPath";
        static const char aSVGStrClipPathProperty[] = "clip-path";
        static const char aSVGStrMask[] = "mask";
        static const char aSVGStrClipPathUnits[] = "clipPathUnits";
        static const char aSVGStrMaskUnits[] = "maskUnits";
        static const char aSVGStrMaskContentUnits[] = "maskContentUnits";
        static const char aSVGStrClipRule[] = "clip-rule";
        static const char aSVGStrMarker[] = "marker";
        static const char aSVGStrMarkerStart[] = "marker-start";
        static const char aSVGStrMarkerMid[] = "marker-mid";
        static const char aSVGStrMarkerEnd[] = "marker-end";
        static const char aSVGStrRefX[] = "refX";
        static const char aSVGStrRefY[] = "refY";
        static const char aSVGStrMarkerUnits[] = "markerUnits";
        static const char aSVGStrMarkerWidth[] = "markerWidth";
        static const char aSVGStrMarkerHeight[] = "markerHeight";
        static const char aSVGStrOrient[] = "orient";
        static const char aSVGStrPattern[] = "pattern";
        static const char aSVGStrPatternUnits[] = "patternUnits";
        static const char aSVGStrPatternContentUnits[] = "patternContentUnits";
        static const char aSVGStrPatternTransform[] = "patternTransform";
        static const char aSVGStrOpacity[] = "opacity";
        static const char aSVGStrVisibility[] = "visibility";
        static const char aSVGStrTitle[] = "title";
        static const char aSVGStrDesc[] = "desc";

        static const char aSVGStrPreserveAspectRatio[] = "preserveAspectRatio";
        static const char aSVGStrDefer[] = "defer";
        static const char aSVGStrNone[] = "none";
        static const char aSVGStrXMinYMin[] = "xMinYMin";
        static const char aSVGStrXMidYMin[] = "xMidYMin";
        static const char aSVGStrXMaxYMin[] = "xMaxYMin";
        static const char aSVGStrXMinYMid[] = "xMinYMid";
        static const char aSVGStrXMidYMid[] = "xMidYMid";
        static const char aSVGStrXMaxYMid[] = "xMaxYMid";
        static const char aSVGStrXMinYMax[] = "xMinYMax";
        static const char aSVGStrXMidYMax[] = "xMidYMax";
        static const char aSVGStrXMaxYMax[] = "xMaxYMax";
        static const char aSVGStrMeet[] = "meet";
        static const char aSVGStrSlice[] = "slice";

        static const char aSVGStrDefs[] = "defs";
        static const char aSVGStrG[] = "g";
        static const char aSVGStrSvg[] = "svg";
        static const char aSVGStrSymbol[] = "symbol";
        static const char aSVGStrUse[] = "use";
        static const char aSVGStrA[] = "a";

        static const char aSVGStrCircle[] = "circle";
        static const char aSVGStrEllipse[] = "ellipse";
        static const char aSVGStrLine[] = "line";
        static const char aSVGStrPath[] = "path";
        static const char aSVGStrPolygon[] = "polygon";
        static const char aSVGStrPolyline[] = "polyline";
        static const char aSVGStrRect[] = "rect";
        static const char aSVGStrImage[] = "image";

        static const char aSVGStrLinearGradient[] = "linearGradient";
        static const char aSVGStrRadialGradient[] = "radialGradient";
        static const char aSVGStrStop[] = "stop";
        static const char aSVGStrOffset[] = "offset";
        static const char aSVGStrX1[] = "x1";
        static const char aSVGStrY1[] = "y1";
        static const char aSVGStrX2[] = "x2";
        static const char aSVGStrY2[] = "y2";
        static const char aSVGStrCx[] = "cx";
        static const char aSVGStrCy[] = "cy";
        static const char aSVGStrFx[] = "fx";
        static const char aSVGStrFy[] = "fy";
        static const char aSVGStrR[] = "r";
        static const char aSVGStrGradientUnits[] = "gradientUnits";
        static const char aSVGStrGradientTransform[] = "gradientTransform";
        static const char aSVGStrSpreadMethod[] = "spreadMethod";
        static const char aSVGStrXlinkHref[] = "xlink:href";
        static const char aSVGStrStopColor[] = "stop-color";
        static const char aSVGStrStopOpacity[] = "stop-opacity";

        static const char aSVGStrFill[] = "fill";
        static const char aSVGStrFillOpacity[] = "fill-opacity";
        static const char aSVGStrFillRule[] = "fill-rule";

        static const char aSVGStrStroke[] = "stroke";
        static const char aSVGStrStrokeDasharray[] = "stroke-dasharray";
        static const char aSVGStrStrokeDashoffset[] = "stroke-dashoffset";
        static const char aSVGStrStrokeLinecap[] = "stroke-linecap";
        static const char aSVGStrStrokeLinejoin[] = "stroke-linejoin";
        static const char aSVGStrStrokeMiterlimit[] = "stroke-miterlimit";
        static const char aSVGStrStrokeOpacity[] = "stroke-opacity";
        static const char aSVGStrStrokeWidth[] = "stroke-width";

        static const char aSVGStrText[] = "text";
        static const char aSVGStrBaselineShift[] = "baseline-shift";

        static const char aSVGStrFlowRoot[] = "flowRoot";

        SVGToken StrToSVGToken(const OUString& rStr, bool bCaseIndependent)
        {
            typedef std::unordered_map< OUString, SVGToken > SVGTokenMapper;
            typedef std::pair< OUString, SVGToken > SVGTokenValueType;
            static SVGTokenMapper aSVGTokenMapperList {
                { aSVGStrWidth, SVGTokenWidth },
                { aSVGStrHeight, SVGTokenHeight },
                { aSVGStrViewBox, SVGTokenViewBox },
                { aSVGStrTransform, SVGTokenTransform },
                { aSVGStrStyle, SVGTokenStyle },
                { aSVGStrDisplay, SVGTokenDisplay }, // #i121656#
                { aSVGStrD, SVGTokenD },
                { aSVGStrX, SVGTokenX },
                { aSVGStrY, SVGTokenY },
                { aSVGStrXmlns, SVGTokenXmlns },
                { aSVGStrVersion, SVGTokenVersion },
                { aSVGStrId, SVGTokenId },
                { aSVGStrRx, SVGTokenRx },
                { aSVGStrRy, SVGTokenRy },
                { aSVGStrPoints, SVGTokenPoints },
                { aSVGStrDx, SVGTokenDx },
                { aSVGStrDy, SVGTokenDy },
                { aSVGStrRotate, SVGTokenRotate },
                { aSVGStrTextLength, SVGTokenTextLength },
                { aSVGStrLengthAdjust, SVGTokenLengthAdjust },
                { aSVGStrFont, SVGTokenFont },
                { aSVGStrFontFamily, SVGTokenFontFamily },
                { aSVGStrFontSize, SVGTokenFontSize },
                { aSVGStrFontSizeAdjust, SVGTokenFontSizeAdjust },
                { aSVGStrFontStretch, SVGTokenFontStretch },
                { aSVGStrFontStyle, SVGTokenFontStyle },
                { aSVGStrFontVariant, SVGTokenFontVariant },
                { aSVGStrFontWeight, SVGTokenFontWeight },
                { aSVGStrDirection, SVGTokenDirection },
                { aSVGStrLetterSpacing, SVGTokenLetterSpacing },
                { aSVGStrTextDecoration, SVGTokenTextDecoration },
                { aSVGStrUnicodeBidi, SVGTokenUnicodeBidi },
                { aSVGStrWordSpacing, SVGTokenWordSpacing },
                { aSVGStrTspan, SVGTokenTspan },
                { aSVGStrTref, SVGTokenTref },
                { aSVGStrTextPath, SVGTokenTextPath },
                { aSVGStrStartOffset, SVGTokenStartOffset },
                { aSVGStrMethod, SVGTokenMethod },
                { aSVGStrSpacing, SVGTokenSpacing },
                { aSVGStrTextAlign, SVGTokenTextAlign },
                { aSVGStrPathLength, SVGTokenPathLength },
                { aSVGStrType, SVGTokenType },
                { aSVGStrClass, SVGTokenClass },
                { aSVGStrTextAnchor, SVGTokenTextAnchor },
                { aSVGStrXmlSpace, SVGTokenXmlSpace },
                { aSVGStrColor, SVGTokenColor },
                { aSVGStrClipPathNode, SVGTokenClipPathNode },
                { aSVGStrClipPathProperty, SVGTokenClipPathProperty },
                { aSVGStrMask, SVGTokenMask },
                { aSVGStrClipPathUnits, SVGTokenClipPathUnits },
                { aSVGStrMaskUnits, SVGTokenMaskUnits },
                { aSVGStrMaskContentUnits, SVGTokenMaskContentUnits },
                { aSVGStrClipRule, SVGTokenClipRule },
                { aSVGStrMarker, SVGTokenMarker },
                { aSVGStrMarkerStart, SVGTokenMarkerStart },
                { aSVGStrMarkerMid, SVGTokenMarkerMid },
                { aSVGStrMarkerEnd, SVGTokenMarkerEnd },
                { aSVGStrRefX, SVGTokenRefX },
                { aSVGStrRefY, SVGTokenRefY },
                { aSVGStrMarkerUnits, SVGTokenMarkerUnits },
                { aSVGStrMarkerWidth, SVGTokenMarkerWidth },
                { aSVGStrMarkerHeight, SVGTokenMarkerHeight },
                { aSVGStrOrient, SVGTokenOrient },
                { aSVGStrPattern, SVGTokenPattern },
                { aSVGStrPatternUnits, SVGTokenPatternUnits },
                { aSVGStrPatternContentUnits, SVGTokenPatternContentUnits },
                { aSVGStrPatternTransform, SVGTokenPatternTransform },
                { aSVGStrOpacity, SVGTokenOpacity },
                { aSVGStrVisibility, SVGTokenVisibility },
                { aSVGStrTitle, SVGTokenTitle },
                { aSVGStrDesc, SVGTokenDesc },

                { aSVGStrPreserveAspectRatio, SVGTokenPreserveAspectRatio },
                { aSVGStrDefer, SVGTokenDefer },
                { aSVGStrNone, SVGTokenNone },
                { aSVGStrXMinYMin, SVGTokenXMinYMin },
                { aSVGStrXMidYMin, SVGTokenXMidYMin },
                { aSVGStrXMaxYMin, SVGTokenXMaxYMin },
                { aSVGStrXMinYMid, SVGTokenXMinYMid },
                { aSVGStrXMidYMid, SVGTokenXMidYMid },
                { aSVGStrXMaxYMid, SVGTokenXMaxYMid },
                { aSVGStrXMinYMax, SVGTokenXMinYMax },
                { aSVGStrXMidYMax, SVGTokenXMidYMax },
                { aSVGStrXMaxYMax, SVGTokenXMaxYMax },
                { aSVGStrMeet, SVGTokenMeet },
                { aSVGStrSlice, SVGTokenSlice },

                { aSVGStrDefs, SVGTokenDefs },
                { aSVGStrG, SVGTokenG },
                { aSVGStrSvg, SVGTokenSvg },
                { aSVGStrSymbol, SVGTokenSymbol },
                { aSVGStrUse, SVGTokenUse },
                { aSVGStrA, SVGTokenA },

                { aSVGStrCircle, SVGTokenCircle },
                { aSVGStrEllipse, SVGTokenEllipse },
                { aSVGStrLine, SVGTokenLine },
                { aSVGStrPath, SVGTokenPath },
                { aSVGStrPolygon, SVGTokenPolygon },
                { aSVGStrPolyline, SVGTokenPolyline },
                { aSVGStrRect, SVGTokenRect },
                { aSVGStrImage, SVGTokenImage },

                { aSVGStrLinearGradient, SVGTokenLinearGradient },
                { aSVGStrRadialGradient, SVGTokenRadialGradient },
                { aSVGStrStop, SVGTokenStop },
                { aSVGStrOffset, SVGTokenOffset },
                { aSVGStrX1, SVGTokenX1 },
                { aSVGStrY1, SVGTokenY1 },
                { aSVGStrX2, SVGTokenX2 },
                { aSVGStrY2, SVGTokenY2 },
                { aSVGStrCx, SVGTokenCx },
                { aSVGStrCy, SVGTokenCy },
                { aSVGStrFx, SVGTokenFx },
                { aSVGStrFy, SVGTokenFy },
                { aSVGStrR, SVGTokenR },
                { aSVGStrGradientUnits, SVGTokenGradientUnits },
                { aSVGStrGradientTransform, SVGTokenGradientTransform },
                { aSVGStrSpreadMethod, SVGTokenSpreadMethod },
                { aSVGStrXlinkHref, SVGTokenXlinkHref },
                { aSVGStrStopColor, SVGTokenStopColor },
                { aSVGStrStopOpacity, SVGTokenStopOpacity },

                { aSVGStrFill, SVGTokenFill },
                { aSVGStrFillOpacity, SVGTokenFillOpacity },
                { aSVGStrFillRule, SVGTokenFillRule },

                { aSVGStrStroke, SVGTokenStroke },
                { aSVGStrStrokeDasharray, SVGTokenStrokeDasharray },
                { aSVGStrStrokeDashoffset, SVGTokenStrokeDashoffset },
                { aSVGStrStrokeLinecap, SVGTokenStrokeLinecap },
                { aSVGStrStrokeLinejoin, SVGTokenStrokeLinejoin },
                { aSVGStrStrokeMiterlimit, SVGTokenStrokeMiterlimit },
                { aSVGStrStrokeOpacity, SVGTokenStrokeOpacity },
                { aSVGStrStrokeWidth, SVGTokenStrokeWidth },

                { aSVGStrText, SVGTokenText },
                { aSVGStrBaselineShift, SVGTokenBaselineShift },
                { aSVGStrFlowRoot, SVGTokenFlowRoot }
            };

            const SVGTokenMapper::const_iterator aResult(aSVGTokenMapperList.find(rStr.startsWith("svg:") ? rStr.copy(4) : rStr));

            if(aResult == aSVGTokenMapperList.end())
            {
                if(bCaseIndependent)
                {
                    static SVGTokenMapper aCaseLindependentSVGTokenMapperList;

                    if(aCaseLindependentSVGTokenMapperList.empty())
                    {
                        for(const auto& rCurrent : aSVGTokenMapperList)
                        {
                            aCaseLindependentSVGTokenMapperList.insert(
                                SVGTokenValueType(
                                    rCurrent.first.toAsciiLowerCase(),
                                    rCurrent.second));
                        }
                    }

                    const SVGTokenMapper::const_iterator aResult2(aCaseLindependentSVGTokenMapperList.find(rStr.toAsciiLowerCase()));

                    if(aResult2 == aCaseLindependentSVGTokenMapperList.end())
                    {
                        return SVGTokenUnknown;
                    }
                    else
                    {
                        return aResult2->second;
                    }
                }

                return SVGTokenUnknown;
            }
            else
            {
                return aResult->second;
            }
        }

        OUString getStrTitle()
        {
            return aSVGStrTitle;
        }

        OUString getStrDesc()
        {
            return aSVGStrDesc;
        }
    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
