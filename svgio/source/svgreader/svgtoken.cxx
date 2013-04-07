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

#include <svgio/svgreader/svgtoken.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        static OUString aSVGStrWidth(OUString::createFromAscii("width"));
        static OUString aSVGStrHeight(OUString::createFromAscii("height"));
        static OUString aSVGStrViewBox(OUString::createFromAscii("viewBox"));
        static OUString aSVGStrTransform(OUString::createFromAscii("transform"));
        static OUString aSVGStrStyle(OUString::createFromAscii("style"));
        static OUString aSVGStrD(OUString::createFromAscii("d"));
        static OUString aSVGStrX(OUString::createFromAscii("x"));
        static OUString aSVGStrY(OUString::createFromAscii("y"));
        static OUString aSVGStrXmlns(OUString::createFromAscii("xmlns"));
        static OUString aSVGStrVersion(OUString::createFromAscii("version"));
        static OUString aSVGStrId(OUString::createFromAscii("id"));
        static OUString aSVGStrRx(OUString::createFromAscii("rx"));
        static OUString aSVGStrRy(OUString::createFromAscii("ry"));
        static OUString aSVGStrPoints(OUString::createFromAscii("points"));
        static OUString aSVGStrDx(OUString::createFromAscii("dx"));
        static OUString aSVGStrDy(OUString::createFromAscii("dy"));
        static OUString aSVGStrRotate(OUString::createFromAscii("rotate"));
        static OUString aSVGStrTextLength(OUString::createFromAscii("textLength"));
        static OUString aSVGStrLengthAdjust(OUString::createFromAscii("lengthAdjust"));
        static OUString aSVGStrFont(OUString::createFromAscii("font"));
        static OUString aSVGStrFontFamily(OUString::createFromAscii("font-family"));
        static OUString aSVGStrFontSize(OUString::createFromAscii("font-size"));
        static OUString aSVGStrFontSizeAdjust(OUString::createFromAscii("font-size-adjust"));
        static OUString aSVGStrFontStretch(OUString::createFromAscii("font-stretch"));
        static OUString aSVGStrFontStyle(OUString::createFromAscii("font-style"));
        static OUString aSVGStrFontVariant(OUString::createFromAscii("font-variant"));
        static OUString aSVGStrFontWeight(OUString::createFromAscii("font-weight"));
        static OUString aSVGStrDirection(OUString::createFromAscii("direction"));
        static OUString aSVGStrLetterSpacing(OUString::createFromAscii("letter-spacing"));
        static OUString aSVGStrTextDecoration(OUString::createFromAscii("text-decoration"));
        static OUString aSVGStrUnicodeBidi(OUString::createFromAscii("unicode-bidi"));
        static OUString aSVGStrWordSpacing(OUString::createFromAscii("word-spacing"));
        static OUString aSVGStrTspan(OUString::createFromAscii("tspan"));
        static OUString aSVGStrTref(OUString::createFromAscii("tref"));
        static OUString aSVGStrTextPath(OUString::createFromAscii("textPath"));
        static OUString aSVGStrStartOffset(OUString::createFromAscii("startOffset"));
        static OUString aSVGStrMethod(OUString::createFromAscii("method"));
        static OUString aSVGStrSpacing(OUString::createFromAscii("spacing"));
        static OUString aSVGStrTextAlign(OUString::createFromAscii("text-align"));
        static OUString aSVGStrPathLength(OUString::createFromAscii("pathLength"));
        static OUString aSVGStrType(OUString::createFromAscii("type"));
        static OUString aSVGStrClass(OUString::createFromAscii("class"));
        static OUString aSVGStrTextAnchor(OUString::createFromAscii("text-anchor"));
        static OUString aSVGStrXmlSpace(OUString::createFromAscii("xml:space"));
        static OUString aSVGStrColor(OUString::createFromAscii("color"));
        static OUString aSVGStrClipPathNode(OUString::createFromAscii("clipPath"));
        static OUString aSVGStrClipPathProperty(OUString::createFromAscii("clip-path"));
        static OUString aSVGStrMask(OUString::createFromAscii("mask"));
        static OUString aSVGStrClipPathUnits(OUString::createFromAscii("clipPathUnits"));
        static OUString aSVGStrMaskUnits(OUString::createFromAscii("maskUnits"));
        static OUString aSVGStrMaskContentUnits(OUString::createFromAscii("maskContentUnits"));
        static OUString aSVGStrClipRule(OUString::createFromAscii("clip-rule"));
        static OUString aSVGStrMarker(OUString::createFromAscii("marker"));
        static OUString aSVGStrMarkerStart(OUString::createFromAscii("marker-start"));
        static OUString aSVGStrMarkerMid(OUString::createFromAscii("marker-mid"));
        static OUString aSVGStrMarkerEnd(OUString::createFromAscii("marker-end"));
        static OUString aSVGStrRefX(OUString::createFromAscii("refX"));
        static OUString aSVGStrRefY(OUString::createFromAscii("refY"));
        static OUString aSVGStrMarkerUnits(OUString::createFromAscii("markerUnits"));
        static OUString aSVGStrMarkerWidth(OUString::createFromAscii("markerWidth"));
        static OUString aSVGStrMarkerHeight(OUString::createFromAscii("markerHeight"));
        static OUString aSVGStrOrient(OUString::createFromAscii("orient"));
        static OUString aSVGStrPattern(OUString::createFromAscii("pattern"));
        static OUString aSVGStrPatternUnits(OUString::createFromAscii("patternUnits"));
        static OUString aSVGStrPatternContentUnits(OUString::createFromAscii("patternContentUnits"));
        static OUString aSVGStrPatternTransform(OUString::createFromAscii("patternTransform"));
        static OUString aSVGStrOpacity(OUString::createFromAscii("opacity"));
        static OUString aSVGStrTitle(OUString::createFromAscii("title"));
        static OUString aSVGStrDesc(OUString::createFromAscii("desc"));

        static OUString aSVGStrPreserveAspectRatio(OUString::createFromAscii("preserveAspectRatio"));
        static OUString aSVGStrDefer(OUString::createFromAscii("defer"));
        static OUString aSVGStrNone(OUString::createFromAscii("none"));
        static OUString aSVGStrXMinYMin(OUString::createFromAscii("xMinYMin"));
        static OUString aSVGStrXMidYMin(OUString::createFromAscii("xMidYMin"));
        static OUString aSVGStrXMaxYMin(OUString::createFromAscii("xMaxYMin"));
        static OUString aSVGStrXMinYMid(OUString::createFromAscii("xMinYMid"));
        static OUString aSVGStrXMidYMid(OUString::createFromAscii("xMidYMid"));
        static OUString aSVGStrXMaxYMid(OUString::createFromAscii("xMaxYMid"));
        static OUString aSVGStrXMinYMax(OUString::createFromAscii("xMinYMax"));
        static OUString aSVGStrXMidYMax(OUString::createFromAscii("xMidYMax"));
        static OUString aSVGStrXMaxYMax(OUString::createFromAscii("xMaxYMax"));
        static OUString aSVGStrMeet(OUString::createFromAscii("meet"));
        static OUString aSVGStrSlice(OUString::createFromAscii("slice"));

        static OUString aSVGStrDefs(OUString::createFromAscii("defs"));
        static OUString aSVGStrG(OUString::createFromAscii("g"));
        static OUString aSVGStrSvg(OUString::createFromAscii("svg"));
        static OUString aSVGStrSymbol(OUString::createFromAscii("symbol"));
        static OUString aSVGStrUse(OUString::createFromAscii("use"));

        static OUString aSVGStrCircle(OUString::createFromAscii("circle"));
        static OUString aSVGStrEllipse(OUString::createFromAscii("ellipse"));
        static OUString aSVGStrLine(OUString::createFromAscii("line"));
        static OUString aSVGStrPath(OUString::createFromAscii("path"));
        static OUString aSVGStrPolygon(OUString::createFromAscii("polygon"));
        static OUString aSVGStrPolyline(OUString::createFromAscii("polyline"));
        static OUString aSVGStrRect(OUString::createFromAscii("rect"));
        static OUString aSVGStrImage(OUString::createFromAscii("image"));

        static OUString aSVGStrLinearGradient(OUString::createFromAscii("linearGradient"));
        static OUString aSVGStrRadialGradient(OUString::createFromAscii("radialGradient"));
        static OUString aSVGStrStop(OUString::createFromAscii("stop"));
        static OUString aSVGStrOffset(OUString::createFromAscii("offset"));
        static OUString aSVGStrX1(OUString::createFromAscii("x1"));
        static OUString aSVGStrY1(OUString::createFromAscii("y1"));
        static OUString aSVGStrX2(OUString::createFromAscii("x2"));
        static OUString aSVGStrY2(OUString::createFromAscii("y2"));
        static OUString aSVGStrCx(OUString::createFromAscii("cx"));
        static OUString aSVGStrCy(OUString::createFromAscii("cy"));
        static OUString aSVGStrFx(OUString::createFromAscii("fx"));
        static OUString aSVGStrFy(OUString::createFromAscii("fy"));
        static OUString aSVGStrR(OUString::createFromAscii("r"));
        static OUString aSVGStrGradientUnits(OUString::createFromAscii("gradientUnits"));
        static OUString aSVGStrGradientTransform(OUString::createFromAscii("gradientTransform"));
        static OUString aSVGStrSpreadMethod(OUString::createFromAscii("spreadMethod"));
        static OUString aSVGStrXlinkHref(OUString::createFromAscii("xlink:href"));
        static OUString aSVGStrStopColor(OUString::createFromAscii("stop-color"));
        static OUString aSVGStrStopOpacity(OUString::createFromAscii("stop-opacity"));

        static OUString aSVGStrFill(OUString::createFromAscii("fill"));
        static OUString aSVGStrFillOpacity(OUString::createFromAscii("fill-opacity"));
        static OUString aSVGStrFillRule(OUString::createFromAscii("fill-rule"));

        static OUString aSVGStrStroke(OUString::createFromAscii("stroke"));
        static OUString aSVGStrStrokeDasharray(OUString::createFromAscii("stroke-dasharray"));
        static OUString aSVGStrStrokeDashoffset(OUString::createFromAscii("stroke-dashoffset"));
        static OUString aSVGStrStrokeLinecap(OUString::createFromAscii("stroke-linecap"));
        static OUString aSVGStrStrokeLinejoin(OUString::createFromAscii("stroke-linejoin"));
        static OUString aSVGStrStrokeMiterlimit(OUString::createFromAscii("stroke-miterlimit"));
        static OUString aSVGStrStrokeOpacity(OUString::createFromAscii("stroke-opacity"));
        static OUString aSVGStrStrokeWidth(OUString::createFromAscii("stroke-width"));

        static OUString aSVGStrText(OUString::createFromAscii("text"));

        SVGToken StrToSVGToken(const OUString& rStr)
        {
            typedef boost::unordered_map< OUString, SVGToken, OUStringHash,::std::equal_to< OUString >  > SVGTokenMapper;
            typedef std::pair< OUString, SVGToken > SVGTokenValueType;
            static SVGTokenMapper aSVGTokenMapperList;

            if(aSVGTokenMapperList.empty())
            {
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrWidth, SVGTokenWidth));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrHeight, SVGTokenHeight));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrViewBox, SVGTokenViewBox));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrTransform, SVGTokenTransform));
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrStyle, SVGTokenStyle));
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
            }

            const SVGTokenMapper::const_iterator aResult(aSVGTokenMapperList.find(rStr.startsWith("svg:") ? rStr.copy(4) : rStr));

            if(aResult == aSVGTokenMapperList.end())
            {
                return SVGTokenUnknown;
            }
            else
            {
                return aResult->second;
            }
        }

        const OUString& getStrTitle()
        {
            return aSVGStrTitle;
        }

        const OUString& getStrDesc()
        {
            return aSVGStrDesc;
        }
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
