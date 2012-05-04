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
        static rtl::OUString aSVGStrWidth(rtl::OUString::createFromAscii("width"));
        static rtl::OUString aSVGStrHeight(rtl::OUString::createFromAscii("height"));
        static rtl::OUString aSVGStrViewBox(rtl::OUString::createFromAscii("viewBox"));
        static rtl::OUString aSVGStrTransform(rtl::OUString::createFromAscii("transform"));
        static rtl::OUString aSVGStrStyle(rtl::OUString::createFromAscii("style"));
        static rtl::OUString aSVGStrD(rtl::OUString::createFromAscii("d"));
        static rtl::OUString aSVGStrX(rtl::OUString::createFromAscii("x"));
        static rtl::OUString aSVGStrY(rtl::OUString::createFromAscii("y"));
        static rtl::OUString aSVGStrXmlns(rtl::OUString::createFromAscii("xmlns"));
        static rtl::OUString aSVGStrVersion(rtl::OUString::createFromAscii("version"));
        static rtl::OUString aSVGStrId(rtl::OUString::createFromAscii("id"));
        static rtl::OUString aSVGStrRx(rtl::OUString::createFromAscii("rx"));
        static rtl::OUString aSVGStrRy(rtl::OUString::createFromAscii("ry"));
        static rtl::OUString aSVGStrPoints(rtl::OUString::createFromAscii("points"));
        static rtl::OUString aSVGStrDx(rtl::OUString::createFromAscii("dx"));
        static rtl::OUString aSVGStrDy(rtl::OUString::createFromAscii("dy"));
        static rtl::OUString aSVGStrRotate(rtl::OUString::createFromAscii("rotate"));
        static rtl::OUString aSVGStrTextLength(rtl::OUString::createFromAscii("textLength"));
        static rtl::OUString aSVGStrLengthAdjust(rtl::OUString::createFromAscii("lengthAdjust"));
        static rtl::OUString aSVGStrFont(rtl::OUString::createFromAscii("font"));
        static rtl::OUString aSVGStrFontFamily(rtl::OUString::createFromAscii("font-family"));
        static rtl::OUString aSVGStrFontSize(rtl::OUString::createFromAscii("font-size"));
        static rtl::OUString aSVGStrFontSizeAdjust(rtl::OUString::createFromAscii("font-size-adjust"));
        static rtl::OUString aSVGStrFontStretch(rtl::OUString::createFromAscii("font-stretch"));
        static rtl::OUString aSVGStrFontStyle(rtl::OUString::createFromAscii("font-style"));
        static rtl::OUString aSVGStrFontVariant(rtl::OUString::createFromAscii("font-variant"));
        static rtl::OUString aSVGStrFontWeight(rtl::OUString::createFromAscii("font-weight"));
        static rtl::OUString aSVGStrDirection(rtl::OUString::createFromAscii("direction"));
        static rtl::OUString aSVGStrLetterSpacing(rtl::OUString::createFromAscii("letter-spacing"));
        static rtl::OUString aSVGStrTextDecoration(rtl::OUString::createFromAscii("text-decoration"));
        static rtl::OUString aSVGStrUnicodeBidi(rtl::OUString::createFromAscii("unicode-bidi"));
        static rtl::OUString aSVGStrWordSpacing(rtl::OUString::createFromAscii("word-spacing"));
        static rtl::OUString aSVGStrTspan(rtl::OUString::createFromAscii("tspan"));
        static rtl::OUString aSVGStrTref(rtl::OUString::createFromAscii("tref"));
        static rtl::OUString aSVGStrTextPath(rtl::OUString::createFromAscii("textPath"));
        static rtl::OUString aSVGStrStartOffset(rtl::OUString::createFromAscii("startOffset"));
        static rtl::OUString aSVGStrMethod(rtl::OUString::createFromAscii("method"));
        static rtl::OUString aSVGStrSpacing(rtl::OUString::createFromAscii("spacing"));
        static rtl::OUString aSVGStrTextAlign(rtl::OUString::createFromAscii("text-align"));
        static rtl::OUString aSVGStrPathLength(rtl::OUString::createFromAscii("pathLength"));
        static rtl::OUString aSVGStrType(rtl::OUString::createFromAscii("type"));
        static rtl::OUString aSVGStrClass(rtl::OUString::createFromAscii("class"));
        static rtl::OUString aSVGStrTextAnchor(rtl::OUString::createFromAscii("text-anchor"));
        static rtl::OUString aSVGStrXmlSpace(rtl::OUString::createFromAscii("xml:space"));
        static rtl::OUString aSVGStrColor(rtl::OUString::createFromAscii("color"));
        static rtl::OUString aSVGStrClipPathNode(rtl::OUString::createFromAscii("clipPath"));
        static rtl::OUString aSVGStrClipPathProperty(rtl::OUString::createFromAscii("clip-path"));
        static rtl::OUString aSVGStrMask(rtl::OUString::createFromAscii("mask"));
        static rtl::OUString aSVGStrClipPathUnits(rtl::OUString::createFromAscii("clipPathUnits"));
        static rtl::OUString aSVGStrMaskUnits(rtl::OUString::createFromAscii("maskUnits"));
        static rtl::OUString aSVGStrMaskContentUnits(rtl::OUString::createFromAscii("maskContentUnits"));
        static rtl::OUString aSVGStrClipRule(rtl::OUString::createFromAscii("clip-rule"));
        static rtl::OUString aSVGStrMarker(rtl::OUString::createFromAscii("marker"));
        static rtl::OUString aSVGStrMarkerStart(rtl::OUString::createFromAscii("marker-start"));
        static rtl::OUString aSVGStrMarkerMid(rtl::OUString::createFromAscii("marker-mid"));
        static rtl::OUString aSVGStrMarkerEnd(rtl::OUString::createFromAscii("marker-end"));
        static rtl::OUString aSVGStrRefX(rtl::OUString::createFromAscii("refX"));
        static rtl::OUString aSVGStrRefY(rtl::OUString::createFromAscii("refY"));
        static rtl::OUString aSVGStrMarkerUnits(rtl::OUString::createFromAscii("markerUnits"));
        static rtl::OUString aSVGStrMarkerWidth(rtl::OUString::createFromAscii("markerWidth"));
        static rtl::OUString aSVGStrMarkerHeight(rtl::OUString::createFromAscii("markerHeight"));
        static rtl::OUString aSVGStrOrient(rtl::OUString::createFromAscii("orient"));
        static rtl::OUString aSVGStrPattern(rtl::OUString::createFromAscii("pattern"));
        static rtl::OUString aSVGStrPatternUnits(rtl::OUString::createFromAscii("patternUnits"));
        static rtl::OUString aSVGStrPatternContentUnits(rtl::OUString::createFromAscii("patternContentUnits"));
        static rtl::OUString aSVGStrPatternTransform(rtl::OUString::createFromAscii("patternTransform"));
        static rtl::OUString aSVGStrOpacity(rtl::OUString::createFromAscii("opacity"));
        static rtl::OUString aSVGStrTitle(rtl::OUString::createFromAscii("title"));
        static rtl::OUString aSVGStrDesc(rtl::OUString::createFromAscii("desc"));

        static rtl::OUString aSVGStrPreserveAspectRatio(rtl::OUString::createFromAscii("preserveAspectRatio"));
        static rtl::OUString aSVGStrDefer(rtl::OUString::createFromAscii("defer"));
        static rtl::OUString aSVGStrNone(rtl::OUString::createFromAscii("none"));
        static rtl::OUString aSVGStrXMinYMin(rtl::OUString::createFromAscii("xMinYMin"));
        static rtl::OUString aSVGStrXMidYMin(rtl::OUString::createFromAscii("xMidYMin"));
        static rtl::OUString aSVGStrXMaxYMin(rtl::OUString::createFromAscii("xMaxYMin"));
        static rtl::OUString aSVGStrXMinYMid(rtl::OUString::createFromAscii("xMinYMid"));
        static rtl::OUString aSVGStrXMidYMid(rtl::OUString::createFromAscii("xMidYMid"));
        static rtl::OUString aSVGStrXMaxYMid(rtl::OUString::createFromAscii("xMaxYMid"));
        static rtl::OUString aSVGStrXMinYMax(rtl::OUString::createFromAscii("xMinYMax"));
        static rtl::OUString aSVGStrXMidYMax(rtl::OUString::createFromAscii("xMidYMax"));
        static rtl::OUString aSVGStrXMaxYMax(rtl::OUString::createFromAscii("xMaxYMax"));
        static rtl::OUString aSVGStrMeet(rtl::OUString::createFromAscii("meet"));
        static rtl::OUString aSVGStrSlice(rtl::OUString::createFromAscii("slice"));

        static rtl::OUString aSVGStrDefs(rtl::OUString::createFromAscii("defs"));
        static rtl::OUString aSVGStrG(rtl::OUString::createFromAscii("g"));
        static rtl::OUString aSVGStrSvg(rtl::OUString::createFromAscii("svg"));
        static rtl::OUString aSVGStrSymbol(rtl::OUString::createFromAscii("symbol"));
        static rtl::OUString aSVGStrUse(rtl::OUString::createFromAscii("use"));

        static rtl::OUString aSVGStrCircle(rtl::OUString::createFromAscii("circle"));
        static rtl::OUString aSVGStrEllipse(rtl::OUString::createFromAscii("ellipse"));
        static rtl::OUString aSVGStrLine(rtl::OUString::createFromAscii("line"));
        static rtl::OUString aSVGStrPath(rtl::OUString::createFromAscii("path"));
        static rtl::OUString aSVGStrPolygon(rtl::OUString::createFromAscii("polygon"));
        static rtl::OUString aSVGStrPolyline(rtl::OUString::createFromAscii("polyline"));
        static rtl::OUString aSVGStrRect(rtl::OUString::createFromAscii("rect"));
        static rtl::OUString aSVGStrImage(rtl::OUString::createFromAscii("image"));

        static rtl::OUString aSVGStrLinearGradient(rtl::OUString::createFromAscii("linearGradient"));
        static rtl::OUString aSVGStrRadialGradient(rtl::OUString::createFromAscii("radialGradient"));
        static rtl::OUString aSVGStrStop(rtl::OUString::createFromAscii("stop"));
        static rtl::OUString aSVGStrOffset(rtl::OUString::createFromAscii("offset"));
        static rtl::OUString aSVGStrX1(rtl::OUString::createFromAscii("x1"));
        static rtl::OUString aSVGStrY1(rtl::OUString::createFromAscii("y1"));
        static rtl::OUString aSVGStrX2(rtl::OUString::createFromAscii("x2"));
        static rtl::OUString aSVGStrY2(rtl::OUString::createFromAscii("y2"));
        static rtl::OUString aSVGStrCx(rtl::OUString::createFromAscii("cx"));
        static rtl::OUString aSVGStrCy(rtl::OUString::createFromAscii("cy"));
        static rtl::OUString aSVGStrFx(rtl::OUString::createFromAscii("fx"));
        static rtl::OUString aSVGStrFy(rtl::OUString::createFromAscii("fy"));
        static rtl::OUString aSVGStrR(rtl::OUString::createFromAscii("r"));
        static rtl::OUString aSVGStrGradientUnits(rtl::OUString::createFromAscii("gradientUnits"));
        static rtl::OUString aSVGStrGradientTransform(rtl::OUString::createFromAscii("gradientTransform"));
        static rtl::OUString aSVGStrSpreadMethod(rtl::OUString::createFromAscii("spreadMethod"));
        static rtl::OUString aSVGStrXlinkHref(rtl::OUString::createFromAscii("xlink:href"));
        static rtl::OUString aSVGStrStopColor(rtl::OUString::createFromAscii("stop-color"));
        static rtl::OUString aSVGStrStopOpacity(rtl::OUString::createFromAscii("stop-opacity"));

        static rtl::OUString aSVGStrFill(rtl::OUString::createFromAscii("fill"));
        static rtl::OUString aSVGStrFillOpacity(rtl::OUString::createFromAscii("fill-opacity"));
        static rtl::OUString aSVGStrFillRule(rtl::OUString::createFromAscii("fill-rule"));

        static rtl::OUString aSVGStrStroke(rtl::OUString::createFromAscii("stroke"));
        static rtl::OUString aSVGStrStrokeDasharray(rtl::OUString::createFromAscii("stroke-dasharray"));
        static rtl::OUString aSVGStrStrokeDashoffset(rtl::OUString::createFromAscii("stroke-dashoffset"));
        static rtl::OUString aSVGStrStrokeLinecap(rtl::OUString::createFromAscii("stroke-linecap"));
        static rtl::OUString aSVGStrStrokeLinejoin(rtl::OUString::createFromAscii("stroke-linejoin"));
        static rtl::OUString aSVGStrStrokeMiterlimit(rtl::OUString::createFromAscii("stroke-miterlimit"));
        static rtl::OUString aSVGStrStrokeOpacity(rtl::OUString::createFromAscii("stroke-opacity"));
        static rtl::OUString aSVGStrStrokeWidth(rtl::OUString::createFromAscii("stroke-width"));

        static rtl::OUString aSVGStrText(rtl::OUString::createFromAscii("text"));

        SVGToken StrToSVGToken(const rtl::OUString& rStr)
        {
            typedef boost::unordered_map< rtl::OUString, SVGToken, rtl::OUStringHash,::std::equal_to< ::rtl::OUString >  > SVGTokenMapper;
            typedef std::pair< rtl::OUString, SVGToken > SVGTokenValueType;
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

        const rtl::OUString& getStrTitle()
        {
            return aSVGStrTitle;
        }

        const rtl::OUString& getStrDesc()
        {
            return aSVGStrDesc;
        }
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
