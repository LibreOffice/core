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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGSTYLEATTRIBUTES_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGSTYLEATTRIBUTES_HXX

#include <svgio/svgreader/svgpaint.hxx>
#include <svgio/svgreader/svgnode.hxx>
#include <vcl/vclenum.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace svgio { namespace svgreader {
    class SvgGradientNode;
    class SvgPatternNode;
    class SvgMarkerNode;
}}

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
        enum StrokeLinecap
        {
            StrokeLinecap_notset,
            StrokeLinecap_butt,
            StrokeLinecap_round,
            StrokeLinecap_square
        };

        enum StrokeLinejoin
        {
            StrokeLinejoin_notset,
            StrokeLinejoin_miter,
            StrokeLinejoin_round,
            StrokeLinejoin_bevel
        };

        enum FontStretch
        {
            FontStretch_notset,
            FontStretch_normal,
            FontStretch_wider,
            FontStretch_narrower,
            FontStretch_ultra_condensed,
            FontStretch_extra_condensed,
            FontStretch_condensed,
            FontStretch_semi_condensed,
            FontStretch_semi_expanded,
            FontStretch_expanded,
            FontStretch_extra_expanded,
            FontStretch_ultra_expanded
        };

        FontStretch getWider(FontStretch aSource);
        FontStretch getNarrower(FontStretch aSource);

        enum FontStyle
        {
            FontStyle_notset,
            FontStyle_normal,
            FontStyle_italic,
            FontStyle_oblique
        };

        enum FontVariant
        {
            FontVariant_notset,
            FontVariant_normal,
            FontVariant_small_caps
        };

        enum FontWeight
        {
            FontWeight_notset,
            FontWeight_100,
            FontWeight_200,
            FontWeight_300,
            FontWeight_400, // same as FontWeight_normal
            FontWeight_500,
            FontWeight_600,
            FontWeight_700, // same as FontWeight_bold
            FontWeight_800,
            FontWeight_900,
            FontWeight_bolder,
            FontWeight_lighter,
        };

        FontWeight getBolder(FontWeight aSource);
        FontWeight getLighter(FontWeight aSource);
        ::FontWeight getVclFontWeight(FontWeight aSource);

        enum TextAlign
        {
            TextAlign_notset,
            TextAlign_left,
            TextAlign_right,
            TextAlign_center,
            TextAlign_justify
        };

        enum TextDecoration
        {
            TextDecoration_notset,
            TextDecoration_none,
            TextDecoration_underline,
            TextDecoration_overline,
            TextDecoration_line_through,
            TextDecoration_blink
        };

        enum TextAnchor
        {
            TextAnchor_notset,
            TextAnchor_start,
            TextAnchor_middle,
            TextAnchor_end
        };

        class SvgStyleAttributes
        {
        private:
            SvgNode&                    mrOwner;
            const SvgStyleAttributes*   mpCssStyleParent;

            SvgPaint                    maFill;
            SvgPaint                    maStroke;
            SvgPaint                    maStopColor;
            SvgNumber                   maStrokeWidth;
            SvgNumber                   maStopOpacity;
            const SvgGradientNode*      mpSvgGradientNodeFill;
            const SvgGradientNode*      mpSvgGradientNodeStroke;
            const SvgPatternNode*       mpSvgPatternNodeFill;
            const SvgPatternNode*       mpSvgPatternNodeStroke;
            SvgNumber                   maFillOpacity;
            SvgNumberVector             maStrokeDasharray;
            SvgNumber                   maStrokeDashOffset;
            StrokeLinecap               maStrokeLinecap;
            StrokeLinejoin              maStrokeLinejoin;
            SvgNumber                   maStrokeMiterLimit;
            SvgNumber                   maStrokeOpacity;
            SvgStringVector             maFontFamily;
            SvgNumber                   maFontSize;
            FontStretch                 maFontStretch;
            FontStyle                   maFontStyle;
            FontVariant                 maFontVariant;
            FontWeight                  maFontWeight;
            TextAlign                   maTextAlign;
            TextDecoration              maTextDecoration;
            TextAnchor                  maTextAnchor;
            SvgPaint                    maColor;
            SvgNumber                   maOpacity;
            rtl::OUString               maTitle;
            rtl::OUString               maDesc;

            /// link to content. If set, the node can be fetched on demand
            rtl::OUString               maClipPathXLink;
            rtl::OUString               maMaskXLink;

            /// link to markers. If set, the node can be fetched on demand
            rtl::OUString               maMarkerStartXLink;
            const SvgMarkerNode*        mpMarkerStartXLink;
            rtl::OUString               maMarkerMidXLink;
            const SvgMarkerNode*        mpMarkerMidXLink;
            rtl::OUString               maMarkerEndXLink;
            const SvgMarkerNode*        mpMarkerEndXLink;

            /// bitfield
            bool                        maFillRule : 1; // true: NonZero, false: EvenOdd
            bool                        maFillRuleSet : 1;

            // defines if this attributes are part of a ClipPath. If yes,
            // rough geometry will be created on decomposition by patching
            // vaules for fill, stroke, strokeWidth and others
            bool                        mbIsClipPathContent : 1;

            // ClipRule setting (only valid wne mbIsClipPathContent == true)
            bool                        mbClipRule : 1; // true == nonzero(default), false == evenodd

            /// internal helpers
            void add_fillGradient(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                const SvgGradientNode& rFillGradient,
                const basegfx::B2DRange& rGeoRange) const;
            void add_fillPatternTransform(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                const SvgPatternNode& rFillGradient,
                const basegfx::B2DRange& rGeoRange) const;
            void add_fillPattern(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                const SvgPatternNode& rFillGradient,
                const basegfx::B2DRange& rGeoRange) const;
            void add_fill(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                const basegfx::B2DRange& rGeoRange) const;
            void add_stroke(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                const basegfx::B2DRange& rGeoRange) const;
            bool prepare_singleMarker(
                drawinglayer::primitive2d::Primitive2DSequence& rMarkerPrimitives,
                basegfx::B2DHomMatrix& rMarkerTransform,
                basegfx::B2DRange& rClipRange,
                const SvgMarkerNode& rMarker) const;
            void add_singleMarker(
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                const drawinglayer::primitive2d::Primitive2DSequence& rMarkerPrimitives,
                const basegfx::B2DHomMatrix& rMarkerTransform,
                const basegfx::B2DRange& rClipRange,
                const SvgMarkerNode& rMarker,
                const basegfx::B2DPolygon& rCandidate,
                const sal_uInt32 nIndex) const;
            void add_markers(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DSequence& rTarget) const;

        public:
            /// local attribute scanner
            void parseStyleAttribute(const rtl::OUString& rTokenName, SVGToken aSVGToken, const rtl::OUString& aContent);

            /// helper which does the necessary with a given path
            void add_text(
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                drawinglayer::primitive2d::Primitive2DSequence& rSource) const;
            void add_path(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DSequence& rTarget) const;
            void add_postProcess(
                drawinglayer::primitive2d::Primitive2DSequence& rTarget,
                const drawinglayer::primitive2d::Primitive2DSequence& rSource,
                const basegfx::B2DHomMatrix* pTransform) const;

            /// helper to evtl. link to css style
            void checkForCssStyle(const rtl::OUString& rClassStr) const;

            /// scan helpers
            void readStyle(const rtl::OUString& rCandidate);
            const SvgStyleAttributes* getParentStyle() const;

            SvgStyleAttributes(SvgNode& rOwner);
            ~SvgStyleAttributes();

            /// fill content
            const basegfx::BColor* getFill() const;
            void setFill(const SvgPaint& rFill) { maFill = rFill; }

            /// stroke content
            const basegfx::BColor* getStroke() const;
            void setStroke(const SvgPaint& rStroke) { maStroke = rStroke; }

            /// stop color content
            const basegfx::BColor& getStopColor() const;
            void setStopColor(const SvgPaint& rStopColor) { maStopColor = rStopColor; }

            /// stroke-width content
            const SvgNumber getStrokeWidth() const;
            void setStrokeWidth(const SvgNumber& rStrokeWidth = SvgNumber()) { maStrokeWidth = rStrokeWidth; }

            /// stop opacity content
            const SvgNumber getStopOpacity() const;
            void setStopOpacity(const SvgNumber& rStopOpacity = SvgNumber()) { maStopOpacity = rStopOpacity; }

            /// access to evtl. set fill gradient
            const SvgGradientNode* getSvgGradientNodeFill() const;
            void setSvgGradientNodeFill(const SvgGradientNode* pNew) { mpSvgGradientNodeFill = pNew; }

            /// access to evtl. set fill pattern
            const SvgPatternNode* getSvgPatternNodeFill() const;
            void setSvgPatternNodeFill(const SvgPatternNode* pNew) { mpSvgPatternNodeFill = pNew; }

            /// access to evtl. set stroke gradient
            const SvgGradientNode* getSvgGradientNodeStroke() const;
            void setSvgGradientNodeStroke(const SvgGradientNode* pNew) { mpSvgGradientNodeStroke = pNew; }

            /// access to evtl. set stroke pattern
            const SvgPatternNode* getSvgPatternNodeStroke() const;
            void setSvgPatternNodeStroke(const SvgPatternNode* pNew) { mpSvgPatternNodeStroke = pNew; }

            /// fill opacity content
            const SvgNumber getFillOpacity() const;
            void setFillOpacity(const SvgNumber& rFillOpacity = SvgNumber()) { maFillOpacity = rFillOpacity; }

            /// fill rule content
            bool getFillRule() const;

            /// fill StrokeDasharray content
            const SvgNumberVector& getStrokeDasharray() const;
            void setStrokeDasharray(const SvgNumberVector& rStrokeDasharray = SvgNumberVector()) { maStrokeDasharray = rStrokeDasharray; }

            /// StrokeDashOffset content
            const SvgNumber getStrokeDashOffset() const;
            void setStrokeDashOffset(const SvgNumber& rStrokeDashOffset = SvgNumber()) { maStrokeDashOffset = rStrokeDashOffset; }

            /// StrokeLinecap content
            StrokeLinecap getStrokeLinecap() const;
            void setStrokeLinecap(const StrokeLinecap aStrokeLinecap = StrokeLinecap_notset) { maStrokeLinecap = aStrokeLinecap; }

            /// StrokeLinejoin content
            StrokeLinejoin getStrokeLinejoin() const;
            void setStrokeLinejoin(const StrokeLinejoin aStrokeLinejoin = StrokeLinejoin_notset) { maStrokeLinejoin = aStrokeLinejoin; }

            /// StrokeMiterLimit content
            const SvgNumber getStrokeMiterLimit() const;
            void setStrokeMiterLimit(const SvgNumber& rStrokeMiterLimit = SvgNumber()) { maStrokeMiterLimit = rStrokeMiterLimit; }

            /// StrokeOpacity content
            const SvgNumber getStrokeOpacity() const;
            void setStrokeOpacity(const SvgNumber& rStrokeOpacity = SvgNumber()) { maStrokeOpacity = rStrokeOpacity; }

            /// Font content
            const SvgStringVector& getFontFamily() const;
            void setFontFamily(const SvgStringVector& rSvgStringVector = SvgStringVector()) { maFontFamily = rSvgStringVector; }

            /// FontSize content
            const SvgNumber getFontSize() const;
            void setFontSize(const SvgNumber& rFontSize = SvgNumber()) { maFontSize = rFontSize; }

            /// FontStretch content
            FontStretch getFontStretch() const;
            void setFontStretch(const FontStretch aFontStretch = FontStretch_notset) { maFontStretch = aFontStretch; }

            /// FontStyle content
            FontStyle getFontStyle() const;
            void setFontStyle(const FontStyle aFontStyle = FontStyle_notset) { maFontStyle = aFontStyle; }

            /// FontVariant content
            FontVariant getFontVariant() const;
            void setFontVariant(const FontVariant aFontVariant = FontVariant_notset) { maFontVariant = aFontVariant; }

            /// FontWeight content
            FontWeight getFontWeight() const;
            void setFontWeight(const FontWeight aFontWeight = FontWeight_notset) { maFontWeight = aFontWeight; }

            /// TextAlign content
            TextAlign getTextAlign() const;
            void setTextAlign(const TextAlign aTextAlign = TextAlign_notset) { maTextAlign = aTextAlign; }

            /// TextDecoration content
            const SvgStyleAttributes* getTextDecorationDefiningSvgStyleAttributes() const;
            TextDecoration getTextDecoration() const;
            void setTextDecoration(const TextDecoration aTextDecoration = TextDecoration_notset) { maTextDecoration = aTextDecoration; }

            /// TextAnchor content
            TextAnchor getTextAnchor() const;
            void setTextAnchor(const TextAnchor aTextAnchor = TextAnchor_notset) { maTextAnchor = aTextAnchor; }

            /// Color content
            const basegfx::BColor* getColor() const;
            void setColor(const SvgPaint& rColor) { maColor = rColor; }

            /// Opacity content
            const SvgNumber getOpacity() const { return maOpacity; }
            void setOpacity(const SvgNumber& rOpacity = SvgNumber()) { maOpacity = rOpacity; }

            // Title content
            const rtl::OUString& getTitle() const { return maTitle; }
            void setTitle(const rtl::OUString& rNew) { maTitle = rNew; }

            // Desc content
            const rtl::OUString& getDesc() const { return maDesc; }
            void setDesc(const rtl::OUString& rNew) { maDesc = rNew; }

            // ClipPathXLink content
            const rtl::OUString getClipPathXLink() const { return maClipPathXLink; }
            void setClipPathXLink(const rtl::OUString& rNew) { maClipPathXLink = rNew; }

            // MaskXLink content
            const rtl::OUString getMaskXLink() const { return maMaskXLink; }
            void setMaskXLink(const rtl::OUString& rNew) { maMaskXLink = rNew; }

            // MarkerStartXLink content
            const rtl::OUString getMarkerStartXLink() const;
            const SvgMarkerNode* accessMarkerStartXLink() const;
            void setMarkerStartXLink(const rtl::OUString& rNew) { maMarkerStartXLink = rNew; }

            // MarkerMidXLink content
            const rtl::OUString getMarkerMidXLink() const;
            const SvgMarkerNode* accessMarkerMidXLink() const;
            void setMarkerMidXLink(const rtl::OUString& rNew) { maMarkerMidXLink = rNew; }

            // MarkerEndXLink content
            const rtl::OUString getMarkerEndXLink() const;
            const SvgMarkerNode* accessMarkerEndXLink() const;
            void setMarkerEndXLink(const rtl::OUString& rNew) { maMarkerEndXLink = rNew; }

        };
    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGSTYLEATTRIBUTES_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
