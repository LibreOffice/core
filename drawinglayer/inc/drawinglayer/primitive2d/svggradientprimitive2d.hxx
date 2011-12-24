/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http:\\www.apache.org\licenses\LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SVGGRADIENTPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SVGGRADIENTPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
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
                    && getColor() == getColor()
                    && getOpacity() == getOpacity());
            }

            bool operator<(const SvgGradientEntry& rCompare) const
            {
                return getOffset() < rCompare.getOffset();
            }
        };

        typedef ::std::vector< SvgGradientEntry > SvgGradientEntryVector;

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// SvgGradientHelper class
#define DEFAULT_OVERLAPPING_VALUE (1.0/512.0)

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
            /// geometric definition, the geometry to be filled
            basegfx::B2DPolyPolygon     maPolyPolygon;

            /// the gradient definition
            SvgGradientEntryVector      maGradientEntries;

            /// start and/or center point
            basegfx::B2DPoint           maStart;

            /// how to spread
            SpreadMethod                maSpreadMethod;

            /*  allows to set an overlapping value to be able to create
                slightly overlapping PolyPolygons/Polygons for the decomposition.
                This is needed since when creating geometrically correct decompositions
                many visualisations will show artefacts at the borders, even when these
                borders are absolutely correctly defined. It is possible to define a
                useful value for this since the coordinate system we are working in is
                in unit coordinates so that the whole gradient is from [0.0 .. 1.0] range.
                This explains the default value which is 1/512 by a maximum possible
                color count of 255 steps per GradientStop.
                Give a 0.0 here to go to geometrically correct gradient decompositions if
                needed.
                An alternative would be to create no PolyPolygons from outside to inside
                (example for radial), but this leads to up to 255 filled polygons per
                GradientStop and thus to enormous rendering costs.
             **/
            double                      mfOverlapping;

            /// bitfield
            bool                        mbPreconditionsChecked : 1;
            bool                        mbCreatesContent : 1;
            bool                        mbSingleEntry : 1;
            bool                        mbFullyOpaque : 1;

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
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const SvgGradientEntryVector& rGradientEntries,
                const basegfx::B2DPoint& rStart,
                SpreadMethod aSpreadMethod = Spread_pad,
                double fOverlapping = DEFAULT_OVERLAPPING_VALUE);

            /// data read access
            const basegfx::B2DPolyPolygon& getPolyPolygon() const { return maPolyPolygon; }
            const SvgGradientEntryVector& getGradientEntries() const { return maGradientEntries; }
            const basegfx::B2DPoint& getStart() const { return maStart; }
            SpreadMethod getSpreadMethod() const { return maSpreadMethod; }
            double getOverlapping() const { return mfOverlapping; }

            /// compare operator
            virtual bool operator==(const SvgGradientHelper& rSvgGradientHelper) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
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

            /// local helpers
            void ensureGeometry(
                basegfx::B2DPolyPolygon& rPolyPolygon,
                const SvgGradientEntry& rFrom,
                const SvgGradientEntry& rTo,
                sal_Int32 nOffset) const;

        protected:
            /// local helpers
            virtual void createAtom(
                Primitive2DVector& rTargetColor,
                Primitive2DVector& rTargetOpacity,
                const SvgGradientEntry& rFrom,
                const SvgGradientEntry& rTo,
                sal_Int32 nOffset) const;
            virtual void checkPreconditions();

            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            SvgLinearGradientPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const SvgGradientEntryVector& rGradientEntries,
                const basegfx::B2DPoint& rStart,
                const basegfx::B2DPoint& rEnd,
                SpreadMethod aSpreadMethod = Spread_pad,
                double fOverlapping = DEFAULT_OVERLAPPING_VALUE);

            /// data read access
            const basegfx::B2DPoint& getEnd() const { return maEnd; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
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
            void ensureGeometry(
                basegfx::B2DPolyPolygon& rPolyPolygon,
                const SvgGradientEntry& rFrom,
                const SvgGradientEntry& rTo,
                sal_Int32 nOffset) const;

        protected:
            /// local helpers
            virtual void createAtom(
                Primitive2DVector& rTargetColor,
                Primitive2DVector& rTargetOpacity,
                const SvgGradientEntry& rFrom,
                const SvgGradientEntry& rTo,
                sal_Int32 nOffset) const;
            virtual void checkPreconditions();

            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            SvgRadialGradientPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const SvgGradientEntryVector& rGradientEntries,
                const basegfx::B2DPoint& rStart,
                double fRadius,
                SpreadMethod aSpreadMethod = Spread_pad,
                const basegfx::B2DPoint* pFocal = 0,
                double fOverlapping = DEFAULT_OVERLAPPING_VALUE);

            /// data read access
            double getRadius() const { return mfRadius; }
            const basegfx::B2DPoint& getFocal() const { return maFocal; }
            bool isFocalSet() const { return mbFocalSet; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// SvgLinearAtomPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /*  basic primitive for a single linear GradientRun in unit coordiantes.
            It's derived from DiscreteMetricDependentPrimitive2D to allow view-dependent
            decompositions allowing reduced color steps
         **/
        class DRAWINGLAYER_DLLPUBLIC SvgLinearAtomPrimitive2D : public DiscreteMetricDependentPrimitive2D
        {
        private:
            /// the geometric definition in unit coordiantes
            basegfx::BColor             maColorA;
            basegfx::BColor             maColorB;
            double                      mfOffsetA;
            double                      mfOffsetB;
            double                      mfOverlapping;

        protected:

            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            SvgLinearAtomPrimitive2D(
                const basegfx::BColor& aColorA, double fOffsetA,
                const basegfx::BColor& aColorB, double fOffsetB,
                double fOverlapping = DEFAULT_OVERLAPPING_VALUE)
            :   DiscreteMetricDependentPrimitive2D(),
                maColorA(aColorA),
                maColorB(aColorB),
                mfOffsetA(fOffsetA),
                mfOffsetB(fOffsetB),
                mfOverlapping(fOverlapping)
            {
            }

            /// data read access
            const basegfx::BColor& getColorA() const { return maColorA; }
            const basegfx::BColor& getColorB() const { return maColorB; }
            double getOffsetA() const { return mfOffsetA; }
            double getOffsetB() const { return mfOffsetB; }
            double getOverlapping() const { return mfOverlapping; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// SvgRadialAtomPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /*  basic primitive for a single radial GradientRun in unit coordiantes.
            It's derived from DiscreteMetricDependentPrimitive2D to allow view-dependent
            decompositions allowing reduced color steps
         **/
        class DRAWINGLAYER_DLLPUBLIC SvgRadialAtomPrimitive2D : public DiscreteMetricDependentPrimitive2D
        {
        private:
            /// the geometric definition in unit coordiantes
            basegfx::BColor             maColorA;
            basegfx::BColor             maColorB;
            double                      mfScaleA;
            double                      mfScaleB;

            /// Only used when focal is set
            basegfx::B2DVector          maTranslateA;
            basegfx::B2DVector          maTranslateB;

            double                      mfOverlapping;

            /// bitfield
            bool                        mbTranslateSet : 1;

        protected:

            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            SvgRadialAtomPrimitive2D(
                const basegfx::BColor& aColorA, double fScaleA, const basegfx::B2DVector& rTranslateA,
                const basegfx::BColor& aColorB, double fScaleB, const basegfx::B2DVector& rTranslateB,
                double fOverlapping = DEFAULT_OVERLAPPING_VALUE)
            :   DiscreteMetricDependentPrimitive2D(),
                maColorA(aColorA),
                maColorB(aColorB),
                mfScaleA(fScaleA),
                mfScaleB(fScaleB),
                maTranslateA(rTranslateA),
                maTranslateB(rTranslateB),
                mfOverlapping(fOverlapping),
                mbTranslateSet(true)
            {
                mbTranslateSet = !maTranslateA.equal(maTranslateB);
            }

            SvgRadialAtomPrimitive2D(
                const basegfx::BColor& aColorA, double fScaleA,
                const basegfx::BColor& aColorB, double fScaleB,
                double fOverlapping = DEFAULT_OVERLAPPING_VALUE)
            :   DiscreteMetricDependentPrimitive2D(),
                maColorA(aColorA),
                maColorB(aColorB),
                mfScaleA(fScaleA),
                mfScaleB(fScaleB),
                maTranslateA(),
                maTranslateB(),
                mfOverlapping(fOverlapping),
                mbTranslateSet(false)
            {
            }

            /// data read access
            const basegfx::BColor& getColorA() const { return maColorA; }
            const basegfx::BColor& getColorB() const { return maColorB; }
            double getScaleA() const { return mfScaleA; }
            double getScaleB() const { return mfScaleB; }
            const basegfx::B2DVector& getTranslateA() const { return maTranslateA; }
            const basegfx::B2DVector& getTranslateB() const { return maTranslateB; }
            double getOverlapping() const { return mfOverlapping; }
            bool getTranslateSet() const { return mbTranslateSet; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SVGGRADIENTPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
