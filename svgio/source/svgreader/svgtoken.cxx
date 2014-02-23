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



namespace svgio
{
    namespace svgreader
    {
        static OUString aSVGStrWidth("width");
        static OUString aSVGStrHeight("height");
        static OUString aSVGStrViewBox("viewBox");
        static OUString aSVGStrTransform("transform");
        static OUString aSVGStrStyle("style");
        static OUString aSVGStrDisplay("display"); // #i121656#
        static OUString aSVGStrD("d");
        static OUString aSVGStrX("x");
        static OUString aSVGStrY("y");
        static OUString aSVGStrXmlns("xmlns");
        static OUString aSVGStrVersion("version");
        static OUString aSVGStrId("id");
        static OUString aSVGStrRx("rx");
        static OUString aSVGStrRy("ry");
        static OUString aSVGStrPoints("points");
        static OUString aSVGStrDx("dx");
        static OUString aSVGStrDy("dy");
        static OUString aSVGStrRotate("rotate");
        static OUString aSVGStrTextLength("textLength");
        static OUString aSVGStrLengthAdjust("lengthAdjust");
        static OUString aSVGStrFont("font");
        static OUString aSVGStrFontFamily("font-family");
        static OUString aSVGStrFontSize("font-size");
        static OUString aSVGStrFontSizeAdjust("font-size-adjust");
        static OUString aSVGStrFontStretch("font-stretch");
        static OUString aSVGStrFontStyle("font-style");
        static OUString aSVGStrFontVariant("font-variant");
        static OUString aSVGStrFontWeight("font-weight");
        static OUString aSVGStrDirection("direction");
        static OUString aSVGStrLetterSpacing("letter-spacing");
        static OUString aSVGStrTextDecoration("text-decoration");
        static OUString aSVGStrUnicodeBidi("unicode-bidi");
        static OUString aSVGStrWordSpacing("word-spacing");
        static OUString aSVGStrTspan("tspan");
        static OUString aSVGStrTref("tref");
        static OUString aSVGStrTextPath("textPath");
        static OUString aSVGStrStartOffset("startOffset");
        static OUString aSVGStrMethod("method");
        static OUString aSVGStrSpacing("spacing");
        static OUString aSVGStrTextAlign("text-align");
        static OUString aSVGStrPathLength("pathLength");
        static OUString aSVGStrType("type");
        static OUString aSVGStrClass("class");
        static OUString aSVGStrTextAnchor("text-anchor");
        static OUString aSVGStrXmlSpace("xml:space");
        static OUString aSVGStrColor("color");
        static OUString aSVGStrClipPathNode("clipPath");
        static OUString aSVGStrClipPathProperty("clip-path");
        static OUString aSVGStrMask("mask");
        static OUString aSVGStrClipPathUnits("clipPathUnits");
        static OUString aSVGStrMaskUnits("maskUnits");
        static OUString aSVGStrMaskContentUnits("maskContentUnits");
        static OUString aSVGStrClipRule("clip-rule");
        static OUString aSVGStrMarker("marker");
        static OUString aSVGStrMarkerStart("marker-start");
        static OUString aSVGStrMarkerMid("marker-mid");
        static OUString aSVGStrMarkerEnd("marker-end");
        static OUString aSVGStrRefX("refX");
        static OUString aSVGStrRefY("refY");
        static OUString aSVGStrMarkerUnits("markerUnits");
        static OUString aSVGStrMarkerWidth("markerWidth");
        static OUString aSVGStrMarkerHeight("markerHeight");
        static OUString aSVGStrOrient("orient");
        static OUString aSVGStrPattern("pattern");
        static OUString aSVGStrPatternUnits("patternUnits");
        static OUString aSVGStrPatternContentUnits("patternContentUnits");
        static OUString aSVGStrPatternTransform("patternTransform");
        static OUString aSVGStrOpacity("opacity");
        static OUString aSVGStrTitle("title");
        static OUString aSVGStrDesc("desc");

        static OUString aSVGStrPreserveAspectRatio("preserveAspectRatio");
        static OUString aSVGStrDefer("defer");
        static OUString aSVGStrNone("none");
        static OUString aSVGStrXMinYMin("xMinYMin");
        static OUString aSVGStrXMidYMin("xMidYMin");
        static OUString aSVGStrXMaxYMin("xMaxYMin");
        static OUString aSVGStrXMinYMid("xMinYMid");
        static OUString aSVGStrXMidYMid("xMidYMid");
        static OUString aSVGStrXMaxYMid("xMaxYMid");
        static OUString aSVGStrXMinYMax("xMinYMax");
        static OUString aSVGStrXMidYMax("xMidYMax");
        static OUString aSVGStrXMaxYMax("xMaxYMax");
        static OUString aSVGStrMeet("meet");
        static OUString aSVGStrSlice("slice");

        static OUString aSVGStrDefs("defs");
        static OUString aSVGStrG("g");
        static OUString aSVGStrSvg("svg");
        static OUString aSVGStrSymbol("symbol");
        static OUString aSVGStrUse("use");

        static OUString aSVGStrCircle("circle");
        static OUString aSVGStrEllipse("ellipse");
        static OUString aSVGStrLine("line");
        static OUString aSVGStrPath("path");
        static OUString aSVGStrPolygon("polygon");
        static OUString aSVGStrPolyline("polyline");
        static OUString aSVGStrRect("rect");
        static OUString aSVGStrImage("image");

        static OUString aSVGStrLinearGradient("linearGradient");
        static OUString aSVGStrRadialGradient("radialGradient");
        static OUString aSVGStrStop("stop");
        static OUString aSVGStrOffset("offset");
        static OUString aSVGStrX1("x1");
        static OUString aSVGStrY1("y1");
        static OUString aSVGStrX2("x2");
        static OUString aSVGStrY2("y2");
        static OUString aSVGStrCx("cx");
        static OUString aSVGStrCy("cy");
        static OUString aSVGStrFx("fx");
        static OUString aSVGStrFy("fy");
        static OUString aSVGStrR("r");
        static OUString aSVGStrGradientUnits("gradientUnits");
        static OUString aSVGStrGradientTransform("gradientTransform");
        static OUString aSVGStrSpreadMethod("spreadMethod");
        static OUString aSVGStrXlinkHref("xlink:href");
        static OUString aSVGStrStopColor("stop-color");
        static OUString aSVGStrStopOpacity("stop-opacity");

        static OUString aSVGStrFill("fill");
        static OUString aSVGStrFillOpacity("fill-opacity");
        static OUString aSVGStrFillRule("fill-rule");

        static OUString aSVGStrStroke("stroke");
        static OUString aSVGStrStrokeDasharray("stroke-dasharray");
        static OUString aSVGStrStrokeDashoffset("stroke-dashoffset");
        static OUString aSVGStrStrokeLinecap("stroke-linecap");
        static OUString aSVGStrStrokeLinejoin("stroke-linejoin");
        static OUString aSVGStrStrokeMiterlimit("stroke-miterlimit");
        static OUString aSVGStrStrokeOpacity("stroke-opacity");
        static OUString aSVGStrStrokeWidth("stroke-width");

        static OUString aSVGStrText("text");
        static OUString aSVGStrBaselineShift("baseline-shift");

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
                aSVGTokenMapperList.insert(SVGTokenValueType(aSVGStrBaselineShift, SVGTokenBaselineShift));
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


// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
