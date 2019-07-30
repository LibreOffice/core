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
            static SVGTokenMapper aSVGTokenMapperList;

            if(aSVGTokenMapperList.empty())
            {
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrWidth, SVGTokenWidth));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrHeight, SVGTokenHeight));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrViewBox, SVGTokenViewBox));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrTransform, SVGTokenTransform));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStyle, SVGTokenStyle));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrDisplay, SVGTokenDisplay)); // #i121656#
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrD, SVGTokenD));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrX, SVGTokenX));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrY, SVGTokenY));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXmlns, SVGTokenXmlns));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrVersion, SVGTokenVersion));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrId, SVGTokenId));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrRx, SVGTokenRx));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrRy, SVGTokenRy));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrPoints, SVGTokenPoints));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrDx, SVGTokenDx));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrDy, SVGTokenDy));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrRotate, SVGTokenRotate));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrTextLength, SVGTokenTextLength));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrLengthAdjust, SVGTokenLengthAdjust));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFont, SVGTokenFont));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFontFamily, SVGTokenFontFamily));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFontSize, SVGTokenFontSize));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFontSizeAdjust, SVGTokenFontSizeAdjust));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFontStretch, SVGTokenFontStretch));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFontStyle, SVGTokenFontStyle));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFontVariant, SVGTokenFontVariant));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFontWeight, SVGTokenFontWeight));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrDirection, SVGTokenDirection));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrLetterSpacing, SVGTokenLetterSpacing));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrTextDecoration, SVGTokenTextDecoration));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrUnicodeBidi, SVGTokenUnicodeBidi));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrWordSpacing, SVGTokenWordSpacing));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrTspan, SVGTokenTspan));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrTref, SVGTokenTref));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrTextPath, SVGTokenTextPath));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStartOffset, SVGTokenStartOffset));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMethod, SVGTokenMethod));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrSpacing, SVGTokenSpacing));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrTextAlign, SVGTokenTextAlign));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrPathLength, SVGTokenPathLength));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrType, SVGTokenType));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrClass, SVGTokenClass));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrTextAnchor, SVGTokenTextAnchor));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXmlSpace, SVGTokenXmlSpace));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrColor, SVGTokenColor));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrClipPathNode, SVGTokenClipPathNode));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrClipPathProperty, SVGTokenClipPathProperty));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMask, SVGTokenMask));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrClipPathUnits, SVGTokenClipPathUnits));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMaskUnits, SVGTokenMaskUnits));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMaskContentUnits, SVGTokenMaskContentUnits));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrClipRule, SVGTokenClipRule));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMarker, SVGTokenMarker));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMarkerStart, SVGTokenMarkerStart));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMarkerMid, SVGTokenMarkerMid));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMarkerEnd, SVGTokenMarkerEnd));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrRefX, SVGTokenRefX));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrRefY, SVGTokenRefY));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMarkerUnits, SVGTokenMarkerUnits));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMarkerWidth, SVGTokenMarkerWidth));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMarkerHeight, SVGTokenMarkerHeight));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrOrient, SVGTokenOrient));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrPattern, SVGTokenPattern));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrPatternUnits, SVGTokenPatternUnits));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrPatternContentUnits, SVGTokenPatternContentUnits));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrPatternTransform, SVGTokenPatternTransform));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrOpacity, SVGTokenOpacity));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrVisibility, SVGTokenVisibility));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrTitle, SVGTokenTitle));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrDesc, SVGTokenDesc));

                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrPreserveAspectRatio, SVGTokenPreserveAspectRatio));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrDefer, SVGTokenDefer));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrNone, SVGTokenNone));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXMinYMin, SVGTokenXMinYMin));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXMidYMin, SVGTokenXMidYMin));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXMaxYMin, SVGTokenXMaxYMin));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXMinYMid, SVGTokenXMinYMid));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXMidYMid, SVGTokenXMidYMid));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXMaxYMid, SVGTokenXMaxYMid));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXMinYMax, SVGTokenXMinYMax));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXMidYMax, SVGTokenXMidYMax));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXMaxYMax, SVGTokenXMaxYMax));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrMeet, SVGTokenMeet));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrSlice, SVGTokenSlice));

                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrDefs, SVGTokenDefs));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrG, SVGTokenG));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrSvg, SVGTokenSvg));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrSymbol, SVGTokenSymbol));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrUse, SVGTokenUse));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrA, SVGTokenA));

                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrCircle, SVGTokenCircle));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrEllipse, SVGTokenEllipse));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrLine, SVGTokenLine));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrPath, SVGTokenPath));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrPolygon, SVGTokenPolygon));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrPolyline, SVGTokenPolyline));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrRect, SVGTokenRect));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrImage, SVGTokenImage));

                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrLinearGradient, SVGTokenLinearGradient));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrRadialGradient, SVGTokenRadialGradient));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStop, SVGTokenStop));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrOffset, SVGTokenOffset));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrX1, SVGTokenX1));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrY1, SVGTokenY1));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrX2, SVGTokenX2));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrY2, SVGTokenY2));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrCx, SVGTokenCx));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrCy, SVGTokenCy));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFx, SVGTokenFx));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFy, SVGTokenFy));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrR, SVGTokenR));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrGradientUnits, SVGTokenGradientUnits));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrGradientTransform, SVGTokenGradientTransform));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrSpreadMethod, SVGTokenSpreadMethod));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrXlinkHref, SVGTokenXlinkHref));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStopColor, SVGTokenStopColor));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStopOpacity, SVGTokenStopOpacity));

                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFill, SVGTokenFill));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFillOpacity, SVGTokenFillOpacity));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFillRule, SVGTokenFillRule));

                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStroke, SVGTokenStroke));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStrokeDasharray, SVGTokenStrokeDasharray));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStrokeDashoffset, SVGTokenStrokeDashoffset));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStrokeLinecap, SVGTokenStrokeLinecap));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStrokeLinejoin, SVGTokenStrokeLinejoin));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStrokeMiterlimit, SVGTokenStrokeMiterlimit));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStrokeOpacity, SVGTokenStrokeOpacity));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStrokeWidth, SVGTokenStrokeWidth));

                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrText, SVGTokenText));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrBaselineShift, SVGTokenBaselineShift));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrFlowRoot, SVGTokenFlowRoot));
            }

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

        const OUString getStrTitle()
        {
            return aSVGStrTitle;
        }

        const OUString getStrDesc()
        {
            return aSVGStrDesc;
        }
    } // end of namespace svgreader
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
