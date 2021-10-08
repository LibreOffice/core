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

namespace svgio::svgreader
{
        constexpr OUStringLiteral aSVGStrWidth = u"width";
        constexpr OUStringLiteral aSVGStrHeight = u"height";
        constexpr OUStringLiteral aSVGStrViewBox = u"viewBox";
        constexpr OUStringLiteral aSVGStrTransform = u"transform";
        constexpr OUStringLiteral aSVGStrStyle = u"style";
        constexpr OUStringLiteral aSVGStrDisplay = u"display"; // #i121656#
        constexpr OUStringLiteral aSVGStrD = u"d";
        constexpr OUStringLiteral aSVGStrX = u"x";
        constexpr OUStringLiteral aSVGStrY = u"y";
        constexpr OUStringLiteral aSVGStrXmlns = u"xmlns";
        constexpr OUStringLiteral aSVGStrVersion = u"version";
        constexpr OUStringLiteral aSVGStrId = u"id";
        constexpr OUStringLiteral aSVGStrRx = u"rx";
        constexpr OUStringLiteral aSVGStrRy = u"ry";
        constexpr OUStringLiteral aSVGStrPoints = u"points";
        constexpr OUStringLiteral aSVGStrDx = u"dx";
        constexpr OUStringLiteral aSVGStrDy = u"dy";
        constexpr OUStringLiteral aSVGStrRotate = u"rotate";
        constexpr OUStringLiteral aSVGStrTextLength = u"textLength";
        constexpr OUStringLiteral aSVGStrLengthAdjust = u"lengthAdjust";
        constexpr OUStringLiteral aSVGStrFont = u"font";
        constexpr OUStringLiteral aSVGStrFontFamily = u"font-family";
        constexpr OUStringLiteral aSVGStrFontSize = u"font-size";
        constexpr OUStringLiteral aSVGStrFontSizeAdjust = u"font-size-adjust";
        constexpr OUStringLiteral aSVGStrFontStretch = u"font-stretch";
        constexpr OUStringLiteral aSVGStrFontStyle = u"font-style";
        constexpr OUStringLiteral aSVGStrFontVariant = u"font-variant";
        constexpr OUStringLiteral aSVGStrFontWeight = u"font-weight";
        constexpr OUStringLiteral aSVGStrDirection = u"direction";
        constexpr OUStringLiteral aSVGStrLetterSpacing = u"letter-spacing";
        constexpr OUStringLiteral aSVGStrTextDecoration = u"text-decoration";
        constexpr OUStringLiteral aSVGStrUnicodeBidi = u"unicode-bidi";
        constexpr OUStringLiteral aSVGStrWordSpacing = u"word-spacing";
        constexpr OUStringLiteral aSVGStrTspan = u"tspan";
        constexpr OUStringLiteral aSVGStrTref = u"tref";
        constexpr OUStringLiteral aSVGStrTextPath = u"textPath";
        constexpr OUStringLiteral aSVGStrStartOffset = u"startOffset";
        constexpr OUStringLiteral aSVGStrMethod = u"method";
        constexpr OUStringLiteral aSVGStrSpacing = u"spacing";
        constexpr OUStringLiteral aSVGStrTextAlign = u"text-align";
        constexpr OUStringLiteral aSVGStrPathLength = u"pathLength";
        constexpr OUStringLiteral aSVGStrType = u"type";
        constexpr OUStringLiteral aSVGStrClass = u"class";
        constexpr OUStringLiteral aSVGStrTextAnchor = u"text-anchor";
        constexpr OUStringLiteral aSVGStrXmlSpace = u"xml:space";
        constexpr OUStringLiteral aSVGStrColor = u"color";
        constexpr OUStringLiteral aSVGStrClipPathNode = u"clipPath";
        constexpr OUStringLiteral aSVGStrClipPathProperty = u"clip-path";
        constexpr OUStringLiteral aSVGStrMask = u"mask";
        constexpr OUStringLiteral aSVGStrClipPathUnits = u"clipPathUnits";
        constexpr OUStringLiteral aSVGStrMaskUnits = u"maskUnits";
        constexpr OUStringLiteral aSVGStrMaskContentUnits = u"maskContentUnits";
        constexpr OUStringLiteral aSVGStrClipRule = u"clip-rule";
        constexpr OUStringLiteral aSVGStrMarker = u"marker";
        constexpr OUStringLiteral aSVGStrMarkerStart = u"marker-start";
        constexpr OUStringLiteral aSVGStrMarkerMid = u"marker-mid";
        constexpr OUStringLiteral aSVGStrMarkerEnd = u"marker-end";
        constexpr OUStringLiteral aSVGStrRefX = u"refX";
        constexpr OUStringLiteral aSVGStrRefY = u"refY";
        constexpr OUStringLiteral aSVGStrMarkerUnits = u"markerUnits";
        constexpr OUStringLiteral aSVGStrMarkerWidth = u"markerWidth";
        constexpr OUStringLiteral aSVGStrMarkerHeight = u"markerHeight";
        constexpr OUStringLiteral aSVGStrOrient = u"orient";
        constexpr OUStringLiteral aSVGStrPattern = u"pattern";
        constexpr OUStringLiteral aSVGStrPatternUnits = u"patternUnits";
        constexpr OUStringLiteral aSVGStrPatternContentUnits = u"patternContentUnits";
        constexpr OUStringLiteral aSVGStrPatternTransform = u"patternTransform";
        constexpr OUStringLiteral aSVGStrOpacity = u"opacity";
        constexpr OUStringLiteral aSVGStrVisibility = u"visibility";
        constexpr OUStringLiteral aSVGStrTitle = u"title";
        constexpr OUStringLiteral aSVGStrDesc = u"desc";

        constexpr OUStringLiteral aSVGStrPreserveAspectRatio = u"preserveAspectRatio";
        constexpr OUStringLiteral aSVGStrDefer = u"defer";
        constexpr OUStringLiteral aSVGStrNone = u"none";
        constexpr OUStringLiteral aSVGStrXMinYMin = u"xMinYMin";
        constexpr OUStringLiteral aSVGStrXMidYMin = u"xMidYMin";
        constexpr OUStringLiteral aSVGStrXMaxYMin = u"xMaxYMin";
        constexpr OUStringLiteral aSVGStrXMinYMid = u"xMinYMid";
        constexpr OUStringLiteral aSVGStrXMidYMid = u"xMidYMid";
        constexpr OUStringLiteral aSVGStrXMaxYMid = u"xMaxYMid";
        constexpr OUStringLiteral aSVGStrXMinYMax = u"xMinYMax";
        constexpr OUStringLiteral aSVGStrXMidYMax = u"xMidYMax";
        constexpr OUStringLiteral aSVGStrXMaxYMax = u"xMaxYMax";
        constexpr OUStringLiteral aSVGStrMeet = u"meet";
        constexpr OUStringLiteral aSVGStrSlice = u"slice";

        constexpr OUStringLiteral aSVGStrDefs = u"defs";
        constexpr OUStringLiteral aSVGStrG = u"g";
        constexpr OUStringLiteral aSVGStrSvg = u"svg";
        constexpr OUStringLiteral aSVGStrSymbol = u"symbol";
        constexpr OUStringLiteral aSVGStrUse = u"use";
        constexpr OUStringLiteral aSVGStrA = u"a";

        constexpr OUStringLiteral aSVGStrCircle = u"circle";
        constexpr OUStringLiteral aSVGStrEllipse = u"ellipse";
        constexpr OUStringLiteral aSVGStrLine = u"line";
        constexpr OUStringLiteral aSVGStrPath = u"path";
        constexpr OUStringLiteral aSVGStrPolygon = u"polygon";
        constexpr OUStringLiteral aSVGStrPolyline = u"polyline";
        constexpr OUStringLiteral aSVGStrRect = u"rect";
        constexpr OUStringLiteral aSVGStrImage = u"image";

        constexpr OUStringLiteral aSVGStrLinearGradient = u"linearGradient";
        constexpr OUStringLiteral aSVGStrRadialGradient = u"radialGradient";
        constexpr OUStringLiteral aSVGStrStop = u"stop";
        constexpr OUStringLiteral aSVGStrOffset = u"offset";
        constexpr OUStringLiteral aSVGStrX1 = u"x1";
        constexpr OUStringLiteral aSVGStrY1 = u"y1";
        constexpr OUStringLiteral aSVGStrX2 = u"x2";
        constexpr OUStringLiteral aSVGStrY2 = u"y2";
        constexpr OUStringLiteral aSVGStrCx = u"cx";
        constexpr OUStringLiteral aSVGStrCy = u"cy";
        constexpr OUStringLiteral aSVGStrFx = u"fx";
        constexpr OUStringLiteral aSVGStrFy = u"fy";
        constexpr OUStringLiteral aSVGStrR = u"r";
        constexpr OUStringLiteral aSVGStrGradientUnits = u"gradientUnits";
        constexpr OUStringLiteral aSVGStrGradientTransform = u"gradientTransform";
        constexpr OUStringLiteral aSVGStrSpreadMethod = u"spreadMethod";
        constexpr OUStringLiteral aSVGStrXlinkHref = u"xlink:href";
        constexpr OUStringLiteral aSVGStrStopColor = u"stop-color";
        constexpr OUStringLiteral aSVGStrStopOpacity = u"stop-opacity";

        constexpr OUStringLiteral aSVGStrFill = u"fill";
        constexpr OUStringLiteral aSVGStrFillOpacity = u"fill-opacity";
        constexpr OUStringLiteral aSVGStrFillRule = u"fill-rule";

        constexpr OUStringLiteral aSVGStrStroke = u"stroke";
        constexpr OUStringLiteral aSVGStrStrokeDasharray = u"stroke-dasharray";
        constexpr OUStringLiteral aSVGStrStrokeDashoffset = u"stroke-dashoffset";
        constexpr OUStringLiteral aSVGStrStrokeLinecap = u"stroke-linecap";
        constexpr OUStringLiteral aSVGStrStrokeLinejoin = u"stroke-linejoin";
        constexpr OUStringLiteral aSVGStrStrokeMiterlimit = u"stroke-miterlimit";
        constexpr OUStringLiteral aSVGStrStrokeOpacity = u"stroke-opacity";
        constexpr OUStringLiteral aSVGStrStrokeWidth = u"stroke-width";

        constexpr OUStringLiteral aSVGStrText = u"text";
        constexpr OUStringLiteral aSVGStrBaselineShift = u"baseline-shift";

        constexpr OUStringLiteral aSVGStrFlowRoot = u"flowRoot";

        SVGToken StrToSVGToken(const OUString& rStr, bool bCaseIndependent)
        {
            typedef std::unordered_map< OUString, SVGToken > SVGTokenMapper;
            typedef std::pair< OUString, SVGToken > SVGTokenValueType;
            static SVGTokenMapper aSVGTokenMapperList {
                { aSVGStrWidth, SVGToken::Width },
                { aSVGStrHeight, SVGToken::Height },
                { aSVGStrViewBox, SVGToken::ViewBox },
                { aSVGStrTransform, SVGToken::Transform },
                { aSVGStrStyle, SVGToken::Style },
                { aSVGStrDisplay, SVGToken::Display }, // #i121656#
                { aSVGStrD, SVGToken::D },
                { aSVGStrX, SVGToken::X },
                { aSVGStrY, SVGToken::Y },
                { aSVGStrXmlns, SVGToken::Xmlns },
                { aSVGStrVersion, SVGToken::Version },
                { aSVGStrId, SVGToken::Id },
                { aSVGStrRx, SVGToken::Rx },
                { aSVGStrRy, SVGToken::Ry },
                { aSVGStrPoints, SVGToken::Points },
                { aSVGStrDx, SVGToken::Dx },
                { aSVGStrDy, SVGToken::Dy },
                { aSVGStrRotate, SVGToken::Rotate },
                { aSVGStrTextLength, SVGToken::TextLength },
                { aSVGStrLengthAdjust, SVGToken::LengthAdjust },
                { aSVGStrFont, SVGToken::Font },
                { aSVGStrFontFamily, SVGToken::FontFamily },
                { aSVGStrFontSize, SVGToken::FontSize },
                { aSVGStrFontSizeAdjust, SVGToken::FontSizeAdjust },
                { aSVGStrFontStretch, SVGToken::FontStretch },
                { aSVGStrFontStyle, SVGToken::FontStyle },
                { aSVGStrFontVariant, SVGToken::FontVariant },
                { aSVGStrFontWeight, SVGToken::FontWeight },
                { aSVGStrDirection, SVGToken::Direction },
                { aSVGStrLetterSpacing, SVGToken::LetterSpacing },
                { aSVGStrTextDecoration, SVGToken::TextDecoration },
                { aSVGStrUnicodeBidi, SVGToken::UnicodeBidi },
                { aSVGStrWordSpacing, SVGToken::WordSpacing },
                { aSVGStrTspan, SVGToken::Tspan },
                { aSVGStrTref, SVGToken::Tref },
                { aSVGStrTextPath, SVGToken::TextPath },
                { aSVGStrStartOffset, SVGToken::StartOffset },
                { aSVGStrMethod, SVGToken::Method },
                { aSVGStrSpacing, SVGToken::Spacing },
                { aSVGStrTextAlign, SVGToken::TextAlign },
                { aSVGStrPathLength, SVGToken::PathLength },
                { aSVGStrType, SVGToken::Type },
                { aSVGStrClass, SVGToken::Class },
                { aSVGStrTextAnchor, SVGToken::TextAnchor },
                { aSVGStrXmlSpace, SVGToken::XmlSpace },
                { aSVGStrColor, SVGToken::Color },
                { aSVGStrClipPathNode, SVGToken::ClipPathNode },
                { aSVGStrClipPathProperty, SVGToken::ClipPathProperty },
                { aSVGStrMask, SVGToken::Mask },
                { aSVGStrClipPathUnits, SVGToken::ClipPathUnits },
                { aSVGStrMaskUnits, SVGToken::MaskUnits },
                { aSVGStrMaskContentUnits, SVGToken::MaskContentUnits },
                { aSVGStrClipRule, SVGToken::ClipRule },
                { aSVGStrMarker, SVGToken::Marker },
                { aSVGStrMarkerStart, SVGToken::MarkerStart },
                { aSVGStrMarkerMid, SVGToken::MarkerMid },
                { aSVGStrMarkerEnd, SVGToken::MarkerEnd },
                { aSVGStrRefX, SVGToken::RefX },
                { aSVGStrRefY, SVGToken::RefY },
                { aSVGStrMarkerUnits, SVGToken::MarkerUnits },
                { aSVGStrMarkerWidth, SVGToken::MarkerWidth },
                { aSVGStrMarkerHeight, SVGToken::MarkerHeight },
                { aSVGStrOrient, SVGToken::Orient },
                { aSVGStrPattern, SVGToken::Pattern },
                { aSVGStrPatternUnits, SVGToken::PatternUnits },
                { aSVGStrPatternContentUnits, SVGToken::PatternContentUnits },
                { aSVGStrPatternTransform, SVGToken::PatternTransform },
                { aSVGStrOpacity, SVGToken::Opacity },
                { aSVGStrVisibility, SVGToken::Visibility },
                { aSVGStrTitle, SVGToken::Title },
                { aSVGStrDesc, SVGToken::Desc },

                { aSVGStrPreserveAspectRatio, SVGToken::PreserveAspectRatio },
                { aSVGStrDefer, SVGToken::Defer },
                { aSVGStrNone, SVGToken::None },
                { aSVGStrXMinYMin, SVGToken::XMinYMin },
                { aSVGStrXMidYMin, SVGToken::XMidYMin },
                { aSVGStrXMaxYMin, SVGToken::XMaxYMin },
                { aSVGStrXMinYMid, SVGToken::XMinYMid },
                { aSVGStrXMidYMid, SVGToken::XMidYMid },
                { aSVGStrXMaxYMid, SVGToken::XMaxYMid },
                { aSVGStrXMinYMax, SVGToken::XMinYMax },
                { aSVGStrXMidYMax, SVGToken::XMidYMax },
                { aSVGStrXMaxYMax, SVGToken::XMaxYMax },
                { aSVGStrMeet, SVGToken::Meet },
                { aSVGStrSlice, SVGToken::Slice },

                { aSVGStrDefs, SVGToken::Defs },
                { aSVGStrG, SVGToken::G },
                { aSVGStrSvg, SVGToken::Svg },
                { aSVGStrSymbol, SVGToken::Symbol },
                { aSVGStrUse, SVGToken::Use },
                { aSVGStrA, SVGToken::A },

                { aSVGStrCircle, SVGToken::Circle },
                { aSVGStrEllipse, SVGToken::Ellipse },
                { aSVGStrLine, SVGToken::Line },
                { aSVGStrPath, SVGToken::Path },
                { aSVGStrPolygon, SVGToken::Polygon },
                { aSVGStrPolyline, SVGToken::Polyline },
                { aSVGStrRect, SVGToken::Rect },
                { aSVGStrImage, SVGToken::Image },

                { aSVGStrLinearGradient, SVGToken::LinearGradient },
                { aSVGStrRadialGradient, SVGToken::RadialGradient },
                { aSVGStrStop, SVGToken::Stop },
                { aSVGStrOffset, SVGToken::Offset },
                { aSVGStrX1, SVGToken::X1 },
                { aSVGStrY1, SVGToken::Y1 },
                { aSVGStrX2, SVGToken::X2 },
                { aSVGStrY2, SVGToken::Y2 },
                { aSVGStrCx, SVGToken::Cx },
                { aSVGStrCy, SVGToken::Cy },
                { aSVGStrFx, SVGToken::Fx },
                { aSVGStrFy, SVGToken::Fy },
                { aSVGStrR, SVGToken::R },
                { aSVGStrGradientUnits, SVGToken::GradientUnits },
                { aSVGStrGradientTransform, SVGToken::GradientTransform },
                { aSVGStrSpreadMethod, SVGToken::SpreadMethod },
                { aSVGStrXlinkHref, SVGToken::XlinkHref },
                { aSVGStrStopColor, SVGToken::StopColor },
                { aSVGStrStopOpacity, SVGToken::StopOpacity },

                { aSVGStrFill, SVGToken::Fill },
                { aSVGStrFillOpacity, SVGToken::FillOpacity },
                { aSVGStrFillRule, SVGToken::FillRule },

                { aSVGStrStroke, SVGToken::Stroke },
                { aSVGStrStrokeDasharray, SVGToken::StrokeDasharray },
                { aSVGStrStrokeDashoffset, SVGToken::StrokeDashoffset },
                { aSVGStrStrokeLinecap, SVGToken::StrokeLinecap },
                { aSVGStrStrokeLinejoin, SVGToken::StrokeLinejoin },
                { aSVGStrStrokeMiterlimit, SVGToken::StrokeMiterlimit },
                { aSVGStrStrokeOpacity, SVGToken::StrokeOpacity },
                { aSVGStrStrokeWidth, SVGToken::StrokeWidth },

                { aSVGStrText, SVGToken::Text },
                { aSVGStrBaselineShift, SVGToken::BaselineShift },
                { aSVGStrFlowRoot, SVGToken::FlowRoot }
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
                        return SVGToken::Unknown;
                    }
                    else
                    {
                        return aResult2->second;
                    }
                }

                return SVGToken::Unknown;
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
} // end of namespace svgio

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
