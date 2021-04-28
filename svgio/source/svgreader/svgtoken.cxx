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
        const char aSVGStrWidth[] = "width";
        const char aSVGStrHeight[] = "height";
        const char aSVGStrViewBox[] = "viewBox";
        const char aSVGStrTransform[] = "transform";
        const char aSVGStrStyle[] = "style";
        const char aSVGStrDisplay[] = "display"; // #i121656#
        const char aSVGStrD[] = "d";
        const char aSVGStrX[] = "x";
        const char aSVGStrY[] = "y";
        const char aSVGStrXmlns[] = "xmlns";
        const char aSVGStrVersion[] = "version";
        const char aSVGStrId[] = "id";
        const char aSVGStrRx[] = "rx";
        const char aSVGStrRy[] = "ry";
        const char aSVGStrPoints[] = "points";
        const char aSVGStrDx[] = "dx";
        const char aSVGStrDy[] = "dy";
        const char aSVGStrRotate[] = "rotate";
        const char aSVGStrTextLength[] = "textLength";
        const char aSVGStrLengthAdjust[] = "lengthAdjust";
        const char aSVGStrFont[] = "font";
        const char aSVGStrFontFamily[] = "font-family";
        const char aSVGStrFontSize[] = "font-size";
        const char aSVGStrFontSizeAdjust[] = "font-size-adjust";
        const char aSVGStrFontStretch[] = "font-stretch";
        const char aSVGStrFontStyle[] = "font-style";
        const char aSVGStrFontVariant[] = "font-variant";
        const char aSVGStrFontWeight[] = "font-weight";
        const char aSVGStrDirection[] = "direction";
        const char aSVGStrLetterSpacing[] = "letter-spacing";
        const char aSVGStrTextDecoration[] = "text-decoration";
        const char aSVGStrUnicodeBidi[] = "unicode-bidi";
        const char aSVGStrWordSpacing[] = "word-spacing";
        const char aSVGStrTspan[] = "tspan";
        const char aSVGStrTref[] = "tref";
        const char aSVGStrTextPath[] = "textPath";
        const char aSVGStrStartOffset[] = "startOffset";
        const char aSVGStrMethod[] = "method";
        const char aSVGStrSpacing[] = "spacing";
        const char aSVGStrTextAlign[] = "text-align";
        const char aSVGStrPathLength[] = "pathLength";
        const char aSVGStrType[] = "type";
        const char aSVGStrClass[] = "class";
        const char aSVGStrTextAnchor[] = "text-anchor";
        const char aSVGStrXmlSpace[] = "xml:space";
        const char aSVGStrColor[] = "color";
        const char aSVGStrClipPathNode[] = "clipPath";
        const char aSVGStrClipPathProperty[] = "clip-path";
        const char aSVGStrMask[] = "mask";
        const char aSVGStrClipPathUnits[] = "clipPathUnits";
        const char aSVGStrMaskUnits[] = "maskUnits";
        const char aSVGStrMaskContentUnits[] = "maskContentUnits";
        const char aSVGStrClipRule[] = "clip-rule";
        const char aSVGStrMarker[] = "marker";
        const char aSVGStrMarkerStart[] = "marker-start";
        const char aSVGStrMarkerMid[] = "marker-mid";
        const char aSVGStrMarkerEnd[] = "marker-end";
        const char aSVGStrRefX[] = "refX";
        const char aSVGStrRefY[] = "refY";
        const char aSVGStrMarkerUnits[] = "markerUnits";
        const char aSVGStrMarkerWidth[] = "markerWidth";
        const char aSVGStrMarkerHeight[] = "markerHeight";
        const char aSVGStrOrient[] = "orient";
        const char aSVGStrPattern[] = "pattern";
        const char aSVGStrPatternUnits[] = "patternUnits";
        const char aSVGStrPatternContentUnits[] = "patternContentUnits";
        const char aSVGStrPatternTransform[] = "patternTransform";
        const char aSVGStrOpacity[] = "opacity";
        const char aSVGStrVisibility[] = "visibility";
        constexpr OUStringLiteral aSVGStrTitle = u"title";
        constexpr OUStringLiteral aSVGStrDesc = u"desc";

        const char aSVGStrPreserveAspectRatio[] = "preserveAspectRatio";
        const char aSVGStrDefer[] = "defer";
        const char aSVGStrNone[] = "none";
        const char aSVGStrXMinYMin[] = "xMinYMin";
        const char aSVGStrXMidYMin[] = "xMidYMin";
        const char aSVGStrXMaxYMin[] = "xMaxYMin";
        const char aSVGStrXMinYMid[] = "xMinYMid";
        const char aSVGStrXMidYMid[] = "xMidYMid";
        const char aSVGStrXMaxYMid[] = "xMaxYMid";
        const char aSVGStrXMinYMax[] = "xMinYMax";
        const char aSVGStrXMidYMax[] = "xMidYMax";
        const char aSVGStrXMaxYMax[] = "xMaxYMax";
        const char aSVGStrMeet[] = "meet";
        const char aSVGStrSlice[] = "slice";

        const char aSVGStrDefs[] = "defs";
        const char aSVGStrG[] = "g";
        const char aSVGStrSvg[] = "svg";
        const char aSVGStrSymbol[] = "symbol";
        const char aSVGStrUse[] = "use";
        const char aSVGStrA[] = "a";

        const char aSVGStrCircle[] = "circle";
        const char aSVGStrEllipse[] = "ellipse";
        const char aSVGStrLine[] = "line";
        const char aSVGStrPath[] = "path";
        const char aSVGStrPolygon[] = "polygon";
        const char aSVGStrPolyline[] = "polyline";
        const char aSVGStrRect[] = "rect";
        const char aSVGStrImage[] = "image";

        const char aSVGStrLinearGradient[] = "linearGradient";
        const char aSVGStrRadialGradient[] = "radialGradient";
        const char aSVGStrStop[] = "stop";
        const char aSVGStrOffset[] = "offset";
        const char aSVGStrX1[] = "x1";
        const char aSVGStrY1[] = "y1";
        const char aSVGStrX2[] = "x2";
        const char aSVGStrY2[] = "y2";
        const char aSVGStrCx[] = "cx";
        const char aSVGStrCy[] = "cy";
        const char aSVGStrFx[] = "fx";
        const char aSVGStrFy[] = "fy";
        const char aSVGStrR[] = "r";
        const char aSVGStrGradientUnits[] = "gradientUnits";
        const char aSVGStrGradientTransform[] = "gradientTransform";
        const char aSVGStrSpreadMethod[] = "spreadMethod";
        const char aSVGStrXlinkHref[] = "xlink:href";
        const char aSVGStrStopColor[] = "stop-color";
        const char aSVGStrStopOpacity[] = "stop-opacity";

        const char aSVGStrFill[] = "fill";
        const char aSVGStrFillOpacity[] = "fill-opacity";
        const char aSVGStrFillRule[] = "fill-rule";

        const char aSVGStrStroke[] = "stroke";
        const char aSVGStrStrokeDasharray[] = "stroke-dasharray";
        const char aSVGStrStrokeDashoffset[] = "stroke-dashoffset";
        const char aSVGStrStrokeLinecap[] = "stroke-linecap";
        const char aSVGStrStrokeLinejoin[] = "stroke-linejoin";
        const char aSVGStrStrokeMiterlimit[] = "stroke-miterlimit";
        const char aSVGStrStrokeOpacity[] = "stroke-opacity";
        const char aSVGStrStrokeWidth[] = "stroke-width";

        const char aSVGStrText[] = "text";
        const char aSVGStrBaselineShift[] = "baseline-shift";

        const char aSVGStrFlowRoot[] = "flowRoot";

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
