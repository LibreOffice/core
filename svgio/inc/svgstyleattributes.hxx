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

#ifndef INCLUDED_SVGIO_INC_SVGSTYLEATTRIBUTES_HXX
#define INCLUDED_SVGIO_INC_SVGSTYLEATTRIBUTES_HXX

#include "svgpaint.hxx"
#include "svgnode.hxx"
#include <vcl/vclenum.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>


// predefines

namespace svgio { namespace svgreader {
    class SvgGradientNode;
    class SvgPatternNode;
    class SvgMarkerNode;
    class SvgClipPathNode;
    class SvgMaskNode;
}}


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

        enum FontSize
        {
            FontSize_notset,
            FontSize_xx_small,
            FontSize_x_small,
            FontSize_small,
            FontSize_smaller,
            FontSize_medium,
            FontSize_large,
            FontSize_larger,
            FontSize_x_large,
            FontSize_xx_large,
            FontSize_initial
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

        enum FillRule
        {
            FillRule_notset,
            FillRule_nonzero,
            FillRule_evenodd
        };

        enum BaselineShift
        {
            BaselineShift_Baseline,
            BaselineShift_Sub,
            BaselineShift_Super,
            BaselineShift_Percentage,
            BaselineShift_Length
        };

        enum Visibility
        {
            Visibility_notset,
            Visibility_visible,
            Visibility_hidden,
            Visibility_collapse,
            Visibility_inherit
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
            FontSize                    maFontSize;
            SvgNumber                   maFontSizeNumber;
            FontStretch                 maFontStretch;
            FontStyle                   maFontStyle;
            FontWeight                  maFontWeight;
            TextAlign                   maTextAlign;
            TextDecoration              maTextDecoration;
            TextAnchor                  maTextAnchor;
            SvgPaint                    maColor;
            SvgNumber                   maOpacity;
            Visibility                  maVisibility;
            OUString               maTitle;
            OUString               maDesc;

            /// link to content. If set, the node can be fetched on demand
            OUString               maClipPathXLink;
            const SvgClipPathNode* mpClipPathXLink;
            OUString               maMaskXLink;
            const SvgMaskNode*     mpMaskXLink;

            /// link to markers. If set, the node can be fetched on demand
            OUString               maMarkerStartXLink;
            const SvgMarkerNode*        mpMarkerStartXLink;
            OUString               maMarkerMidXLink;
            const SvgMarkerNode*        mpMarkerMidXLink;
            OUString               maMarkerEndXLink;
            const SvgMarkerNode*        mpMarkerEndXLink;

            /// fill rule
            FillRule                    maFillRule;

            // ClipRule setting (only valid when mbIsClipPathContent == true, default is FillRule_nonzero)
            FillRule                    maClipRule;

            // BaselineShift: Type and number (in case of BaselineShift_Percentage or BaselineShift_Length)
            BaselineShift               maBaselineShift;
            SvgNumber                   maBaselineShiftNumber;

            mutable bool                mbResolvingParent;

            // defines if this attributes are part of a ClipPath. If yes,
            // rough geometry will be created on decomposition by patching
            // values for fill, stroke, strokeWidth and others
            bool                        mbIsClipPathContent : 1;

            // #121221# Defines if evtl. an empty array *is* set
            bool                        mbStrokeDasharraySet : 1;

            /// internal helpers
            void add_fillGradient(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                const SvgGradientNode& rFillGradient,
                const basegfx::B2DRange& rGeoRange) const;
            void add_fillPatternTransform(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                const SvgPatternNode& rFillGradient,
                const basegfx::B2DRange& rGeoRange) const;
            void add_fillPattern(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                const SvgPatternNode& rFillGradient,
                const basegfx::B2DRange& rGeoRange) const;
            void add_fill(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                const basegfx::B2DRange& rGeoRange) const;
            void add_stroke(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                const basegfx::B2DRange& rGeoRange) const;
            bool prepare_singleMarker(
                drawinglayer::primitive2d::Primitive2DContainer& rMarkerPrimitives,
                basegfx::B2DHomMatrix& rMarkerTransform,
                basegfx::B2DRange& rClipRange,
                const SvgMarkerNode& rMarker) const;
            void add_markers(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                const basegfx::utils::PointIndexSet* pHelpPointIndices) const;

        public:
            /// local attribute scanner
            void parseStyleAttribute(SVGToken aSVGToken, const OUString& rContent,
                                     bool bCaseIndependent);

            /// helper which does the necessary with a given path
            void add_text(
                drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                drawinglayer::primitive2d::Primitive2DContainer const & rSource) const;
            void add_path(
                const basegfx::B2DPolyPolygon& rPath,
                drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                const basegfx::utils::PointIndexSet* pHelpPointIndices) const;
            void add_postProcess(
                drawinglayer::primitive2d::Primitive2DContainer& rTarget,
                const drawinglayer::primitive2d::Primitive2DContainer& rSource,
                const basegfx::B2DHomMatrix* pTransform) const;

            /// helper to set mpCssStyleParent temporarily for CSS style hierarchies
            void setCssStyleParent(const SvgStyleAttributes* pNew) { mpCssStyleParent = pNew; }
            const SvgStyleAttributes* getCssStyleParent() const { return mpCssStyleParent; }

            /// scan helpers
            void readCssStyle(const OUString& rCandidate);
            const SvgStyleAttributes* getParentStyle() const;

            SvgStyleAttributes(SvgNode& rOwner);
            ~SvgStyleAttributes();

            /// fill content
            bool isFillSet() const; // #i125258# ask if fill is a direct hard attribute (no hierarchy)
            const basegfx::BColor* getFill() const;
            void setFill(const SvgPaint& rFill) { maFill = rFill; }

            /// stroke content
            const basegfx::BColor* getStroke() const;

            /// stop color content
            const basegfx::BColor& getStopColor() const;

            /// stroke-width content
            SvgNumber getStrokeWidth() const;

            /// stop opacity content
            SvgNumber getStopOpacity() const;

            /// access to evtl. set fill gradient
            const SvgGradientNode* getSvgGradientNodeFill() const;

            /// access to evtl. set fill pattern
            const SvgPatternNode* getSvgPatternNodeFill() const;

            /// access to evtl. set stroke gradient
            const SvgGradientNode* getSvgGradientNodeStroke() const;

            /// access to evtl. set stroke pattern
            const SvgPatternNode* getSvgPatternNodeStroke() const;

            /// fill opacity content
            SvgNumber getFillOpacity() const;

            /// fill rule content
            FillRule getFillRule() const;

            /// fill StrokeDasharray content
            const SvgNumberVector& getStrokeDasharray() const;

            /// StrokeDashOffset content
            SvgNumber getStrokeDashOffset() const;

            /// StrokeLinecap content
            StrokeLinecap getStrokeLinecap() const;
            void setStrokeLinecap(const StrokeLinecap aStrokeLinecap) { maStrokeLinecap = aStrokeLinecap; }

            /// StrokeLinejoin content
            StrokeLinejoin getStrokeLinejoin() const;
            void setStrokeLinejoin(const StrokeLinejoin aStrokeLinejoin) { maStrokeLinejoin = aStrokeLinejoin; }

            /// StrokeMiterLimit content
            SvgNumber getStrokeMiterLimit() const;

            /// StrokeOpacity content
            SvgNumber getStrokeOpacity() const;

            /// Font content
            const SvgStringVector& getFontFamily() const;

            /// FontSize content
            void setFontSize(const FontSize aFontSize) { maFontSize = aFontSize; }
            SvgNumber getFontSizeNumber() const;

            /// FontStretch content
            FontStretch getFontStretch() const;
            void setFontStretch(const FontStretch aFontStretch) { maFontStretch = aFontStretch; }

            /// FontStyle content
            FontStyle getFontStyle() const;
            void setFontStyle(const FontStyle aFontStyle) { maFontStyle = aFontStyle; }

            /// FontWeight content
            FontWeight getFontWeight() const;
            void setFontWeight(const FontWeight aFontWeight) { maFontWeight = aFontWeight; }

            /// TextAlign content
            TextAlign getTextAlign() const;
            void setTextAlign(const TextAlign aTextAlign) { maTextAlign = aTextAlign; }

            /// TextDecoration content
            const SvgStyleAttributes* getTextDecorationDefiningSvgStyleAttributes() const;
            TextDecoration getTextDecoration() const;
            void setTextDecoration(const TextDecoration aTextDecoration) { maTextDecoration = aTextDecoration; }

            /// TextAnchor content
            TextAnchor getTextAnchor() const;
            void setTextAnchor(const TextAnchor aTextAnchor) { maTextAnchor = aTextAnchor; }

            /// Color content
            const basegfx::BColor* getColor() const;

            /// Resolve current color (defaults to black if no color is specified)
            const basegfx::BColor* getCurrentColor() const;

            /// Opacity content
            SvgNumber getOpacity() const;
            void setOpacity(const SvgNumber& rOpacity) { maOpacity = rOpacity; }

            /// Visibility
            Visibility getVisibility() const;
            void setVisibility(const Visibility aVisibility) { maVisibility = aVisibility; }

            // Title content
            const OUString& getTitle() const { return maTitle; }

            // Desc content
            const OUString& getDesc() const { return maDesc; }

            // ClipPathXLink content
            OUString const & getClipPathXLink() const;
            const SvgClipPathNode* accessClipPathXLink() const;

            // MaskXLink content
            OUString getMaskXLink() const;
            const SvgMaskNode* accessMaskXLink() const;

            // MarkerStartXLink content
            OUString getMarkerStartXLink() const;
            const SvgMarkerNode* accessMarkerStartXLink() const;

            // MarkerMidXLink content
            OUString getMarkerMidXLink() const;
            const SvgMarkerNode* accessMarkerMidXLink() const;

            // MarkerEndXLink content
            OUString getMarkerEndXLink() const;
            const SvgMarkerNode* accessMarkerEndXLink() const;

            // BaselineShift
            void setBaselineShift(const BaselineShift aBaselineShift) { maBaselineShift = aBaselineShift; }
            BaselineShift getBaselineShift() const { return maBaselineShift; }
            SvgNumber getBaselineShiftNumber() const;
        };
    } // end of namespace svgreader
} // end of namespace svgio

#endif // INCLUDED_SVGIO_INC_SVGSTYLEATTRIBUTES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
