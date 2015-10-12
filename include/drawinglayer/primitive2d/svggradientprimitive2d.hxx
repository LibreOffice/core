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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SVGGRADIENTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SVGGRADIENTPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vector>


// SvgGradientEntry class

namespace drawinglayer
{
    namespace primitive2d
    {
        /// a single GradientStop defining a color and opacity at a distance
        class SvgGradientEntry
        {
        private:
            double              mfOffset;
            basegfx::BColor     maColor;
            double              mfOpacity;

        public:
            SvgGradientEntry(double fOffset, const basegfx::BColor& rColor = basegfx::BColor(0.0, 0.0, 0.0), double fOpacity = 1.0)
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

    } // end of namespace primitive2d
} // end of namespace drawinglayer


// SvgGradientHelper class

namespace drawinglayer
{
    namespace primitive2d
    {
        enum SpreadMethod
        {
            Spread_pad = 0,
            Spread_reflect,
            Spread_repeat
        };

        /*  helper for linear and radial gradient, both get derived from this
            to share common definitions and functionality
         **/
        class SvgGradientHelper
        {
        private:
            /// the extra gradient transform
            basegfx::B2DHomMatrix       maGradientTransform;

            /// geometric definition, the geometry to be filled
            basegfx::B2DPolyPolygon     maPolyPolygon;

            /// the gradient definition
            SvgGradientEntryVector      maGradientEntries;

            /// start and/or center point
            basegfx::B2DPoint           maStart;

            /// how to spread
            SpreadMethod                maSpreadMethod;

            /// bitfield
            bool                        mbPreconditionsChecked : 1;
            bool                        mbCreatesContent : 1;
            bool                        mbSingleEntry : 1;
            bool                        mbFullyOpaque : 1;

            // true = interpret in unit coordinate system -> object aspect ratio will scale result
            // false = interpret in object coordinate system -> object aspect ratio will not scale result
            // (related to SVG's gradientUnits (userSpaceOnUse|objectBoundingBox)
            bool                        mbUseUnitCoordinates : 1;

        protected:
            /// local helpers
            Primitive2DSequence createSingleGradientEntryFill() const;
            virtual void createAtom(
                Primitive2DVector& rTargetColor,
                Primitive2DVector& rTargetOpacity,
                const SvgGradientEntry& rFrom,
                const SvgGradientEntry& rTo,
                sal_Int32 nOffset) const = 0;
            double createRun(
                Primitive2DVector& rTargetColor,
                Primitive2DVector& rTargetOpacity,
                double fPos,
                double fMax,
                const SvgGradientEntryVector& rEntries,
                sal_Int32 nOffset) const;
            virtual void checkPreconditions();
            Primitive2DSequence createResult(
                const Primitive2DVector& rTargetColor,
                const Primitive2DVector& rTargetOpacity,
                const basegfx::B2DHomMatrix& rUnitGradientToObject,
                bool bInvert = false) const;
            bool getCreatesContent() const { return mbCreatesContent; }
            bool getSingleEntry() const { return mbSingleEntry; }
            void setSingleEntry() { mbSingleEntry = true; }
            bool getPreconditionsChecked() const { return mbPreconditionsChecked; }
            bool getFullyOpaque() const { return mbFullyOpaque; }

        public:
            /// constructor
            SvgGradientHelper(
                const basegfx::B2DHomMatrix& rGradientTransform,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const SvgGradientEntryVector& rGradientEntries,
                const basegfx::B2DPoint& rStart,
                bool bUseUnitCoordinates,
                SpreadMethod aSpreadMethod = Spread_pad);
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
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// SvgLinearGradientPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /// the basic linear gradient primitive
        class DRAWINGLAYER_DLLPUBLIC SvgLinearGradientPrimitive2D : public BufferedDecompositionPrimitive2D, public SvgGradientHelper
        {
        private:
            /// the end point for linear gradient
            basegfx::B2DPoint                       maEnd;

        protected:
            /// local helpers
            virtual void createAtom(
                Primitive2DVector& rTargetColor,
                Primitive2DVector& rTargetOpacity,
                const SvgGradientEntry& rFrom,
                const SvgGradientEntry& rTo,
                sal_Int32 nOffset) const override;
            virtual void checkPreconditions() override;

            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            SvgLinearGradientPrimitive2D(
                const basegfx::B2DHomMatrix& rGradientTransform,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const SvgGradientEntryVector& rGradientEntries,
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                bool bUseUnitCoordinates,
                SpreadMethod aSpreadMethod = Spread_pad);
            virtual ~SvgLinearGradientPrimitive2D();

            /// data read access
            const basegfx::B2DPoint& getEnd() const { return maEnd; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// SvgRadialGradientPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /// the basic radial gradient primitive
        class DRAWINGLAYER_DLLPUBLIC SvgRadialGradientPrimitive2D : public BufferedDecompositionPrimitive2D, public SvgGradientHelper
        {
        private:
            /// the geometric definition
            double                                  mfRadius;

            /// Focal only used when focal is set at all, see constructors
            basegfx::B2DPoint                       maFocal;
            basegfx::B2DVector                      maFocalVector;
            double                                  maFocalLength;

            // internal helper for case Spread_reflect
            SvgGradientEntryVector                  maMirroredGradientEntries;

            /// bitfield
            bool                                    mbFocalSet : 1;

            /// local helpers
            const SvgGradientEntryVector& getMirroredGradientEntries() const;
            void createMirroredGradientEntries();

        protected:
            /// local helpers
            virtual void createAtom(
                Primitive2DVector& rTargetColor,
                Primitive2DVector& rTargetOpacity,
                const SvgGradientEntry& rFrom,
                const SvgGradientEntry& rTo,
                sal_Int32 nOffset) const override;
            virtual void checkPreconditions() override;

            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            SvgRadialGradientPrimitive2D(
                const basegfx::B2DHomMatrix& rGradientTransform,
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const SvgGradientEntryVector& rGradientEntries,
                const basegfx::B2DPoint& rStart,
                double fRadius,
                bool bUseUnitCoordinates,
                SpreadMethod aSpreadMethod = Spread_pad,
                const basegfx::B2DPoint* pFocal = 0);
            virtual ~SvgRadialGradientPrimitive2D();

            /// data read access
            double getRadius() const { return mfRadius; }
            const basegfx::B2DPoint& getFocal() const { return maFocal; }
            bool isFocalSet() const { return mbFocalSet; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// SvgLinearAtomPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /*  basic primitive for a single linear GradientRun in unit coordinates.
            It's derived from DiscreteMetricDependentPrimitive2D to allow view-dependent
            decompositions allowing reduced color steps
         **/
        class DRAWINGLAYER_DLLPUBLIC SvgLinearAtomPrimitive2D : public DiscreteMetricDependentPrimitive2D
        {
        private:
            /// the geometric definition in unit coordinates
            basegfx::BColor             maColorA;
            basegfx::BColor             maColorB;
            double                      mfOffsetA;
            double                      mfOffsetB;

        protected:

            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

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
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// SvgRadialAtomPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /*  basic primitive for a single radial GradientRun in unit coordinates.
            It's derived from DiscreteMetricDependentPrimitive2D to allow view-dependent
            decompositions allowing reduced color steps
         **/
        class DRAWINGLAYER_DLLPUBLIC SvgRadialAtomPrimitive2D : public DiscreteMetricDependentPrimitive2D
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
            VectorPair*                 mpTranslate;

        protected:

            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            SvgRadialAtomPrimitive2D(
                const basegfx::BColor& aColorA, double fScaleA, const basegfx::B2DVector& rTranslateA,
                const basegfx::BColor& aColorB, double fScaleB, const basegfx::B2DVector& rTranslateB);
            SvgRadialAtomPrimitive2D(
                const basegfx::BColor& aColorA, double fScaleA,
                const basegfx::BColor& aColorB, double fScaleB);
            virtual ~SvgRadialAtomPrimitive2D();

            /// data read access
            const basegfx::BColor& getColorA() const { return maColorA; }
            const basegfx::BColor& getColorB() const { return maColorB; }
            double getScaleA() const { return mfScaleA; }
            double getScaleB() const { return mfScaleB; }
            bool isTranslateSet() const { return (0 != mpTranslate); }
            basegfx::B2DVector getTranslateA() const { if(mpTranslate) return mpTranslate->maTranslateA; return basegfx::B2DVector(); }
            basegfx::B2DVector getTranslateB() const { if(mpTranslate) return mpTranslate->maTranslateB; return basegfx::B2DVector(); }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SVGGRADIENTPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
