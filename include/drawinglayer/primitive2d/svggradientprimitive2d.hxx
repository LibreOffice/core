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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vector>


// SvgGradientEntry class

namespace drawinglayer::primitive2d
{
        /// a single GradientStop defining a color and opacity at a distance
        class SvgGradientEntry
        {
        private:
            double              mfOffset;
            basegfx::BColor     maColor;
            double              mfOpacity;

        public:
            SvgGradientEntry(double fOffset, const basegfx::BColor& rColor, double fOpacity)
            :   mfOffset(fOffset),
                maColor(rColor),
                mfOpacity(fOpacity)
            {
            }

            double getOffset() const { return mfOffset; }
            const basegfx::BColor& getColor() const { return maColor; }
            double getOpacity() const { return mfOpacity; }

            bool operator==(const SvgGradientEntry& rCompare) const
            {
                return (getOffset() == rCompare.getOffset()
                    && getColor() == rCompare.getColor()
                    && getOpacity() == rCompare.getOpacity());
            }

            bool operator<(const SvgGradientEntry& rCompare) const
            {
                return getOffset() < rCompare.getOffset();
            }
        };

        typedef ::std::vector< SvgGradientEntry > SvgGradientEntryVector;

        // SvgGradientHelper class

        enum class SpreadMethod
        {
            Pad,
            Reflect,
            Repeat
        };

        /*  helper for linear and radial gradient, both get derived from this
            to share common definitions and functionality
         **/
        class SAL_LOPLUGIN_ANNOTATE("crosscast") SvgGradientHelper
        {
        private:
            /// the extra gradient transform
            basegfx::B2DHomMatrix       maGradientTransform;

            /// geometric definition, the geometry to be filled
            basegfx::B2DPolyPolygon     maPolyPolygon;

            /// the gradient definition
            SvgGradientEntryVector      maGradientEntries;

            // internal helper for case SpreadMethod::Reflect
            SvgGradientEntryVector      maMirroredGradientEntries;

            /// start and/or center point
            basegfx::B2DPoint           maStart;

            /// how to spread
            SpreadMethod                maSpreadMethod;

            bool                        mbCreatesContent : 1;
            bool                        mbSingleEntry : 1;
            bool                        mbFullyOpaque : 1;

            // true = interpret in unit coordinate system -> object aspect ratio will scale result
            // false = interpret in object coordinate system -> object aspect ratio will not scale result
            // (related to SVG's gradientUnits (userSpaceOnUse|objectBoundingBox)
            bool                        mbUseUnitCoordinates : 1;

            /// local helpers
            const SvgGradientEntryVector& getMirroredGradientEntries() const;
            void createMirroredGradientEntries();
            const SvgGradientEntry& FindEntryLessOrEqual(sal_Int32& rInt, const double fFrac) const;
            const SvgGradientEntry& FindEntryMore(sal_Int32& rInt,const double fFrac) const;

        protected:
            /// local helpers
            Primitive2DReference createSingleGradientEntryFill() const;
            virtual void createAtom(
                Primitive2DContainer& rTargetColor,
                Primitive2DContainer& rTargetOpacity,
                const SvgGradientEntry& rFrom,
                const SvgGradientEntry& rTo,
                sal_Int32 nOffsetFrom,
                sal_Int32 nOffsetTo) const = 0;
            void createRun(
                Primitive2DContainer& rTargetColor,
                Primitive2DContainer& rTargetOpacity,
                double fStart,
                double fEnd) const;
            virtual void checkPreconditions();
            Primitive2DReference createResult(
                Primitive2DContainer aTargetColor,
                Primitive2DContainer aTargetOpacity,
                const basegfx::B2DHomMatrix& rUnitGradientToObject,
                bool bInvert = false) const;

            void setSingleEntry() { mbSingleEntry = true; }
            bool getFullyOpaque() const { return mbFullyOpaque; }

        public:
            bool getCreatesContent() const { return mbCreatesContent; }
            bool getSingleEntry() const { return mbSingleEntry; }

            /// constructor
            SvgGradientHelper(
                basegfx::B2DHomMatrix aGradientTransform,
                basegfx::B2DPolyPolygon aPolyPolygon,
                SvgGradientEntryVector&& rGradientEntries,
                const basegfx::B2DPoint& rStart,
                bool bUseUnitCoordinates,
                SpreadMethod aSpreadMethod);
            virtual ~SvgGradientHelper();

            /// data read access
            const basegfx::B2DHomMatrix& getGradientTransform() const { return maGradientTransform; }
            const basegfx::B2DPolyPolygon& getPolyPolygon() const { return maPolyPolygon; }
            const SvgGradientEntryVector& getGradientEntries() const { return maGradientEntries; }
            const basegfx::B2DPoint& getStart() const { return maStart; }
            bool getUseUnitCoordinates() const { return mbUseUnitCoordinates; }
            SpreadMethod getSpreadMethod() const { return maSpreadMethod; }

            /// compare operator
            bool operator==(const SvgGradientHelper& rSvgGradientHelper) const;

            /// create transformation from UnitGrandient to ObjectTransform
            virtual basegfx::B2DHomMatrix createUnitGradientToObjectTransformation() const = 0;
        };

        /// the basic linear gradient primitive
        class DRAWINGLAYER_DLLPUBLIC SvgLinearGradientPrimitive2D final : public BufferedDecompositionPrimitive2D, public SvgGradientHelper
        {
        private:
            /// the end point for linear gradient
            basegfx::B2DPoint                       maEnd;

            /// local helpers
            virtual void createAtom(
                Primitive2DContainer& rTargetColor,
                Primitive2DContainer& rTargetOpacity,
                const SvgGradientEntry& rFrom,
                const SvgGradientEntry& rTo,
                sal_Int32 nOffsetFrom,
                sal_Int32 nOffsetTo) const override;
            virtual void checkPreconditions() override;

            /// local decomposition.
            virtual Primitive2DReference create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            SvgLinearGradientPrimitive2D(
                const basegfx::B2DHomMatrix& rGradientTransform,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                SvgGradientEntryVector&& rGradientEntries,
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                bool bUseUnitCoordinates,
                SpreadMethod aSpreadMethod);
            virtual ~SvgLinearGradientPrimitive2D() override;

            /// data read access
            const basegfx::B2DPoint& getEnd() const { return maEnd; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;

            /// create transformation from UnitGrandient to ObjectTransform
            virtual basegfx::B2DHomMatrix createUnitGradientToObjectTransformation() const override;
        };

        /// the basic radial gradient primitive
        class DRAWINGLAYER_DLLPUBLIC SvgRadialGradientPrimitive2D final : public BufferedDecompositionPrimitive2D, public SvgGradientHelper
        {
        private:
            /// the geometric definition
            double                                  mfRadius;

            /// Focal only used when focal is set at all, see constructors
            basegfx::B2DPoint                       maFocal;
            double                                  maFocalLength;

            /// local helpers
            virtual void createAtom(
                Primitive2DContainer& rTargetColor,
                Primitive2DContainer& rTargetOpacity,
                const SvgGradientEntry& rFrom,
                const SvgGradientEntry& rTo,
                sal_Int32 nOffsetFrom,
                sal_Int32 nOffsetTo) const override;
            virtual void checkPreconditions() override;

            /// local decomposition.
            virtual Primitive2DReference create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            SvgRadialGradientPrimitive2D(
                const basegfx::B2DHomMatrix& rGradientTransform,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                SvgGradientEntryVector&& rGradientEntries,
                const basegfx::B2DPoint& rStart,
                double fRadius,
                bool bUseUnitCoordinates,
                SpreadMethod aSpreadMethod,
                const basegfx::B2DPoint* pFocal);
            virtual ~SvgRadialGradientPrimitive2D() override;

            /// data read access
            double getRadius() const { return mfRadius; }
            const basegfx::B2DPoint& getFocal() const { return maFocal; }
            bool isFocalSet() const { return !maFocal.equal(getStart()); }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;

            /// create transformation from UnitGrandient to ObjectTransform
            virtual basegfx::B2DHomMatrix createUnitGradientToObjectTransformation() const override;
        };

        // SvgLinearAtomPrimitive2D class

        /*  basic primitive for a single linear GradientRun in unit coordinates.
            It's derived from DiscreteMetricDependentPrimitive2D to allow view-dependent
            decompositions allowing reduced color steps
         **/
        class SvgLinearAtomPrimitive2D final : public DiscreteMetricDependentPrimitive2D
        {
        private:
            /// the geometric definition in unit coordinates
            basegfx::BColor             maColorA;
            basegfx::BColor             maColorB;
            double                      mfOffsetA;
            double                      mfOffsetB;

            /// local decomposition.
            virtual Primitive2DReference create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            SvgLinearAtomPrimitive2D(
                const basegfx::BColor& aColorA, double fOffsetA,
                const basegfx::BColor& aColorB, double fOffsetB);

            /// data read access
            const basegfx::BColor& getColorA() const { return maColorA; }
            const basegfx::BColor& getColorB() const { return maColorB; }
            double getOffsetA() const { return mfOffsetA; }
            double getOffsetB() const { return mfOffsetB; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };

        // SvgRadialAtomPrimitive2D class

        /*  basic primitive for a single radial GradientRun in unit coordinates.
            It's derived from DiscreteMetricDependentPrimitive2D to allow view-dependent
            decompositions allowing reduced color steps
         **/
        class SvgRadialAtomPrimitive2D final : public DiscreteMetricDependentPrimitive2D
        {
        private:
            /// the geometric definition in unit coordinates
            basegfx::BColor             maColorA;
            basegfx::BColor             maColorB;
            double                      mfScaleA;
            double                      mfScaleB;

            // helper to hold translation vectors when given (for focal)
            struct VectorPair
            {
                basegfx::B2DVector          maTranslateA;
                basegfx::B2DVector          maTranslateB;

                VectorPair(const basegfx::B2DVector& rTranslateA, const basegfx::B2DVector& rTranslateB)
                :   maTranslateA(rTranslateA),
                    maTranslateB(rTranslateB)
                {
                }
            };

            /// Only used when focal is set
            std::unique_ptr<VectorPair> mpTranslate;

            /// local decomposition.
            virtual Primitive2DReference create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            SvgRadialAtomPrimitive2D(
                const basegfx::BColor& aColorA, double fScaleA, const basegfx::B2DVector& rTranslateA,
                const basegfx::BColor& aColorB, double fScaleB, const basegfx::B2DVector& rTranslateB);
            SvgRadialAtomPrimitive2D(
                const basegfx::BColor& aColorA, double fScaleA,
                const basegfx::BColor& aColorB, double fScaleB);
            virtual ~SvgRadialAtomPrimitive2D() override;

            /// data read access
            const basegfx::BColor& getColorA() const { return maColorA; }
            const basegfx::BColor& getColorB() const { return maColorB; }
            double getScaleA() const { return mfScaleA; }
            double getScaleB() const { return mfScaleB; }
            bool isTranslateSet() const { return (nullptr != mpTranslate); }
            basegfx::B2DVector getTranslateA() const { if(mpTranslate) return mpTranslate->maTranslateA; return basegfx::B2DVector(); }
            basegfx::B2DVector getTranslateB() const { if(mpTranslate) return mpTranslate->maTranslateB; return basegfx::B2DVector(); }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
