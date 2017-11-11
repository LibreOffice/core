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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>

#include <com/sun/star/table/BorderLineStyle.hpp>
namespace editeng { typedef sal_Int16 SvxBorderStyle; }

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BorderLine class
        Helper class holding the style definition for a single part of a full BorderLine definition.
        Line extends are for start/end and for Left/Right, seen in vector direction. If
        Left != Right that means the line has a diagonal start/end
        */
        class DRAWINGLAYER_DLLPUBLIC BorderLine
        {
        private:
            // line attribute containing Width, Color and others
            drawinglayer::attribute::LineAttribute  maLineAttribute;

            // line extends
            double              mfStartLeft;
            double              mfStartRight;
            double              mfEndLeft;
            double              mfEndRight;

            // if this is a gap, this is set to true
            bool                mbIsGap;

            // not implemented
            bool operator!=(const BorderLine& rBorderLine) const = delete;

        public:
            // Constructor for visible BorderLine segments
            BorderLine(
                const drawinglayer::attribute::LineAttribute& rLineAttribute,
                double fStartLeft = 0.0,
                double fStartRight = 0.0,
                double fEndLeft = 0.0,
                double fEndRight = 0.0);

            // Constructor for gap BorderLine segments
            BorderLine(double fWidth);

            ~BorderLine();

            const drawinglayer::attribute::LineAttribute& getLineAttribute() const { return maLineAttribute; }
            double getStartLeft() const { return mfStartLeft; }
            double getStartRight() const { return mfStartRight; }
            double getEndLeft() const { return mfEndLeft; }
            double getEndRight() const { return mfEndRight; }
            bool isGap() const { return mbIsGap; }

            /// helper to get adapted width (maximum)
            double getAdaptedWidth(double fMinWidth) const;

            /// helper to get average values Start/End
            double getStartAverage() const { return 0.5 * (mfStartLeft + mfStartRight); }
            double getEndAverage() const { return 0.5 * (mfEndLeft + mfEndRight); }

            /// compare operator
            bool operator==(const BorderLine& rBorderLine) const;
        };

        /// helper to try to merge two instances of BorderLinePrimitive2D. If it was possible,
        /// a merged version is in the returned Primitive2DReference. Lots of preconditions
        /// have to be met to allow that, see implementation (and maybe even expand)
        Primitive2DReference DRAWINGLAYER_DLLPUBLIC tryMergeBorderLinePrimitive2D(
            const Primitive2DReference& rCandidateA,
            const Primitive2DReference& rCandidateB);

        /** BorderLinePrimitive2D class

        This is the basic primitive to build frames around objects, e.g. tables.
        It defines a single or double line from Start to End using the LeftWidth,
        Distance and RightWidth definitions.
        The LineStart/End overlap is defined in the BorderLines definitions (see
        class BorderLine above).
        */
        class DRAWINGLAYER_DLLPUBLIC BorderLinePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the line definition
            basegfx::B2DPoint                               maStart;
            basegfx::B2DPoint                               maEnd;

            /// the single BorderLine style definition(s), one or three mostly used
            std::vector< BorderLine >                       maBorderLines;

            /// common style definitions
            const drawinglayer::attribute::StrokeAttribute  maStrokeAttribute;

            // for view dependent decomposition in the case with existing gaps,
            // remember the smallest allowed concrete gap distance, see get2DDecomposition
            // implementation
            double                                          mfSmallestAllowedDiscreteGapDistance;

            /// create local decomposition
            virtual Primitive2DContainer create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

            /// helper to find smallest defined gap in maBorderLines
            bool getSmallestGap(double& rfSmallestGap) const;

            /// helper to get the full width taking mfSmallestAllowedDiscreteGapDistance into account
            double getFullWidth() const;

        public:
            /// simplified constructor for BorderLine with single edge
            BorderLinePrimitive2D(
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                const std::vector< BorderLine >& rBorderLines,
                const drawinglayer::attribute::StrokeAttribute& rStrokeAttribute);

            /// data read access
            const basegfx::B2DPoint& getStart() const { return maStart; }
            const basegfx::B2DPoint& getEnd() const { return maEnd; }
            const std::vector< BorderLine >& getBorderLines() const { return maBorderLines; }
            const drawinglayer::attribute::StrokeAttribute& getStrokeAttribute() const { return maStrokeAttribute; }

            /// helper to decide if AntiAliasing should be used
            bool isHorizontalOrVertical(const geometry::ViewInformation2D& rViewInformation) const;

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// Override standard getDecomposition to be view-dependent here
            virtual Primitive2DContainer get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BORDERLINEPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
