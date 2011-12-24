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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/svggradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SvgGradientHelper::createSingleGradientEntryFill() const
        {
            const SvgGradientEntryVector& rEntries = getGradientEntries();
            const sal_uInt32 nCount(rEntries.size());
            Primitive2DSequence xRetval;

            if(nCount)
            {
                const SvgGradientEntry& rSingleEntry = rEntries[nCount - 1];
                const double fOpacity(rSingleEntry.getOpacity());

                if(fOpacity > 0.0)
                {
                    Primitive2DReference xRef(
                        new PolyPolygonColorPrimitive2D(
                            getPolyPolygon(),
                            rSingleEntry.getColor()));

                    if(fOpacity < 1.0)
                    {
                        const Primitive2DSequence aContent(&xRef, 1);

                        xRef = Primitive2DReference(
                            new UnifiedTransparencePrimitive2D(
                                aContent,
                                1.0 - fOpacity));
                    }

                    xRetval = Primitive2DSequence(&xRef, 1);
                }
            }
            else
            {
                OSL_ENSURE(false, "Single gradient entry construction without entry (!)");
            }

            return xRetval;
        }

        void SvgGradientHelper::checkPreconditions()
        {
            mbPreconditionsChecked = true;
            const SvgGradientEntryVector& rEntries = getGradientEntries();

            if(rEntries.empty())
            {
                // no fill at all
            }
            else
            {
                const sal_uInt32 nCount(rEntries.size());

                if(1 == nCount)
                {
                    // fill with single existing color
                    setSingleEntry();
                }
                else
                {
                    // sort maGradientEntries when more than one
                    std::sort(maGradientEntries.begin(), maGradientEntries.end());

                    // gradient with at least two colors
                    bool bAllInvisible(true);

                    for(sal_uInt32 a(0); a < nCount; a++)
                    {
                        const SvgGradientEntry& rCandidate = rEntries[a];

                        if(basegfx::fTools::equalZero(rCandidate.getOpacity()))
                        {
                            // invisible
                            mbFullyOpaque = false;
                        }
                        else if(basegfx::fTools::equal(rCandidate.getOpacity(), 1.0))
                        {
                            // completely opaque
                            bAllInvisible = false;
                        }
                        else
                        {
                            // opacity
                            bAllInvisible = false;
                            mbFullyOpaque = false;
                        }
                    }

                    if(bAllInvisible)
                    {
                        // all invisible, nothing to do
                    }
                    else
                    {
                        const basegfx::B2DRange aPolyRange(getPolyPolygon().getB2DRange());

                        if(aPolyRange.isEmpty())
                        {
                            // no range to fill, nothing to do
                        }
                        else
                        {
                            const double fPolyWidth(aPolyRange.getWidth());
                            const double fPolyHeight(aPolyRange.getHeight());

                            if(basegfx::fTools::equalZero(fPolyWidth) || basegfx::fTools::equalZero(fPolyHeight))
                            {
                                // no width/height to fill, nothing to do
                            }
                            else
                            {
                                mbCreatesContent = true;
                            }
                        }
                    }
                }
            }
        }

        double SvgGradientHelper::createRun(
            Primitive2DVector& rTargetColor,
            Primitive2DVector& rTargetOpacity,
            double fPos,
            double fMax,
            const SvgGradientEntryVector& rEntries,
            sal_Int32 nOffset) const
        {
            const sal_uInt32 nCount(rEntries.size());

            if(nCount)
            {
                const SvgGradientEntry& rStart = rEntries[0];
                const bool bCreateStartPad(fPos < 0.0 && Spread_pad == getSpreadMethod());
                const bool bCreateStartFill(rStart.getOffset() > 0.0);
                sal_uInt32 nIndex(0);

                if(bCreateStartPad || bCreateStartFill)
                {
                    const SvgGradientEntry aTemp(bCreateStartPad ? fPos : 0.0, rStart.getColor(), rStart.getOpacity());

                    createAtom(rTargetColor, rTargetOpacity, aTemp, rStart, nOffset);
                    fPos = rStart.getOffset();
                }

                while(fPos < 1.0 && nIndex + 1 < nCount)
                {
                    const SvgGradientEntry& rCandidateA = rEntries[nIndex++];
                    const SvgGradientEntry& rCandidateB = rEntries[nIndex];

                    createAtom(rTargetColor, rTargetOpacity, rCandidateA, rCandidateB, nOffset);
                    fPos = rCandidateB.getOffset();
                }

                const SvgGradientEntry& rEnd = rEntries[nCount - 1];
                const bool bCreateEndPad(fPos < fMax && Spread_pad == getSpreadMethod());
                const bool bCreateEndFill(rEnd.getOffset() < 1.0);

                if(bCreateEndPad || bCreateEndFill)
                {
                    fPos = bCreateEndPad ? fMax : 1.0;
                    const SvgGradientEntry aTemp(fPos, rEnd.getColor(), rEnd.getOpacity());

                    createAtom(rTargetColor, rTargetOpacity, rEnd, aTemp, nOffset);
                }
            }
            else
            {
                OSL_ENSURE(false, "GradientAtom creation without ColorStops (!)");
                fPos = fMax;
            }

            return fPos;
        }

        Primitive2DSequence SvgGradientHelper::createResult(
            const Primitive2DVector& rTargetColor,
            const Primitive2DVector& rTargetOpacity,
            const basegfx::B2DHomMatrix& rUnitGradientToObject,
            bool bInvert) const
        {
            Primitive2DSequence xRetval;
            const Primitive2DSequence aTargetColorEntries(Primitive2DVectorToPrimitive2DSequence(rTargetColor, bInvert));
            const Primitive2DSequence aTargetOpacityEntries(Primitive2DVectorToPrimitive2DSequence(rTargetOpacity, bInvert));

            if(aTargetColorEntries.hasElements())
            {
                Primitive2DReference xRefContent;

                if(aTargetOpacityEntries.hasElements())
                {
                    const Primitive2DReference xRefOpacity = new TransparencePrimitive2D(
                        aTargetColorEntries,
                        aTargetOpacityEntries);

                    xRefContent = new TransformPrimitive2D(
                        rUnitGradientToObject,
                        Primitive2DSequence(&xRefOpacity, 1));
                }
                else
                {
                    xRefContent = new TransformPrimitive2D(
                        rUnitGradientToObject,
                        aTargetColorEntries);
                }

                xRefContent = new MaskPrimitive2D(
                    getPolyPolygon(),
                    Primitive2DSequence(&xRefContent, 1));

                xRetval = Primitive2DSequence(&xRefContent, 1);
            }

            return xRetval;
        }

        SvgGradientHelper::SvgGradientHelper(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const SvgGradientEntryVector& rGradientEntries,
            const basegfx::B2DPoint& rStart,
            SpreadMethod aSpreadMethod,
            double fOverlapping)
        :   maPolyPolygon(rPolyPolygon),
            maGradientEntries(rGradientEntries),
            maStart(rStart),
            maSpreadMethod(aSpreadMethod),
            mfOverlapping(fOverlapping),
            mbPreconditionsChecked(false),
            mbCreatesContent(false),
            mbSingleEntry(false),
            mbFullyOpaque(true)
        {
        }

        bool SvgGradientHelper::operator==(const SvgGradientHelper& rSvgGradientHelper) const
        {
            const SvgGradientHelper& rCompare = static_cast< const SvgGradientHelper& >(rSvgGradientHelper);

            return (getPolyPolygon() == rCompare.getPolyPolygon()
                && getGradientEntries() == rCompare.getGradientEntries()
                && getStart() == rCompare.getStart()
                && getSpreadMethod() == rCompare.getSpreadMethod()
                && getOverlapping() == rCompare.getOverlapping());
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        void SvgLinearGradientPrimitive2D::checkPreconditions()
        {
            // call parent
            SvgGradientHelper::checkPreconditions();

            if(getCreatesContent())
            {
                // Check Vector
                const basegfx::B2DVector aVector(getEnd() - getStart());

                if(basegfx::fTools::equalZero(aVector.getX()) && basegfx::fTools::equalZero(aVector.getY()))
                {
                    // fill with single color using last stop color
                    setSingleEntry();
                }
            }
        }

        void SvgLinearGradientPrimitive2D::ensureGeometry(
            basegfx::B2DPolyPolygon& rPolyPolygon,
            const SvgGradientEntry& rFrom,
            const SvgGradientEntry& rTo,
            sal_Int32 nOffset) const
        {
            if(!rPolyPolygon.count())
            {
                rPolyPolygon.append(
                    basegfx::tools::createPolygonFromRect(
                        basegfx::B2DRange(
                            rFrom.getOffset() - getOverlapping() + nOffset,
                            0.0,
                            rTo.getOffset() + getOverlapping() + nOffset,
                            1.0)));
            }
        }

        void SvgLinearGradientPrimitive2D::createAtom(
            Primitive2DVector& rTargetColor,
            Primitive2DVector& rTargetOpacity,
            const SvgGradientEntry& rFrom,
            const SvgGradientEntry& rTo,
            sal_Int32 nOffset) const
        {
            // create gradient atom [rFrom.getOffset() .. rTo.getOffset()] with (rFrom.getOffset() > rTo.getOffset())
            if(rFrom.getOffset() == rTo.getOffset())
            {
                OSL_ENSURE(false, "SvgGradient Atom creation with no step width (!)");
            }
            else
            {
                const bool bColorChange(rFrom.getColor() != rTo.getColor());
                const bool bOpacityChange(rFrom.getOpacity() != rTo.getOpacity());
                basegfx::B2DPolyPolygon aPolyPolygon;

                if(bColorChange)
                {
                    rTargetColor.push_back(
                        new SvgLinearAtomPrimitive2D(
                            rFrom.getColor(), rFrom.getOffset() + nOffset,
                            rTo.getColor(), rTo.getOffset() + nOffset,
                            getOverlapping()));
                }
                else
                {
                    ensureGeometry(aPolyPolygon, rFrom, rTo, nOffset);
                    rTargetColor.push_back(
                        new PolyPolygonColorPrimitive2D(
                            aPolyPolygon,
                            rFrom.getColor()));
                }

                if(bOpacityChange)
                {
                    const double fTransFrom(1.0 - rFrom.getOpacity());
                    const double fTransTo(1.0 - rTo.getOpacity());

                    rTargetOpacity.push_back(
                        new SvgLinearAtomPrimitive2D(
                            basegfx::BColor(fTransFrom, fTransFrom, fTransFrom), rFrom.getOffset() + nOffset,
                            basegfx::BColor(fTransTo,fTransTo, fTransTo), rTo.getOffset() + nOffset,
                            getOverlapping()));
                }
                else if(!getFullyOpaque())
                {
                    const double fTransparence(1.0 - rFrom.getOpacity());

                    ensureGeometry(aPolyPolygon, rFrom, rTo, nOffset);
                    rTargetOpacity.push_back(
                        new PolyPolygonColorPrimitive2D(
                            aPolyPolygon,
                            basegfx::BColor(fTransparence, fTransparence, fTransparence)));
                }
            }
        }

        Primitive2DSequence SvgLinearGradientPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;

            if(!getPreconditionsChecked())
            {
                const_cast< SvgLinearGradientPrimitive2D* >(this)->checkPreconditions();
            }

            if(getSingleEntry())
            {
                // fill with last existing color
                xRetval = createSingleGradientEntryFill();
            }
            else if(getCreatesContent())
            {
                // at least two color stops in range [0.0 .. 1.0], sorted, non-null vector, not completely
                // invisible, width and height to fill are not empty
                const basegfx::B2DRange aPolyRange(getPolyPolygon().getB2DRange());
                const double fPolyWidth(aPolyRange.getWidth());
                const double fPolyHeight(aPolyRange.getHeight());

                // create ObjectTransform based on polygon range
                const basegfx::B2DHomMatrix aObjectTransform(
                    basegfx::tools::createScaleTranslateB2DHomMatrix(
                        fPolyWidth, fPolyHeight,
                        aPolyRange.getMinX(), aPolyRange.getMinY()));

                // create unit transform from unit vector [0.0 .. 1.0] along the X-Axis to given
                // gradient vector defined by Start,End
                const basegfx::B2DVector aVector(getEnd() - getStart());
                const double fVectorLength(aVector.getLength());
                basegfx::B2DHomMatrix aUnitGradientToGradient;

                aUnitGradientToGradient.scale(fVectorLength, 1.0);
                aUnitGradientToGradient.rotate(atan2(aVector.getY(), aVector.getX()));
                aUnitGradientToGradient.translate(getStart().getX(), getStart().getY());

                // create full transform from unit gradient coordinates to object coordinates
                // including the SvgGradient transformation
                basegfx::B2DHomMatrix aUnitGradientToObject(aObjectTransform * aUnitGradientToGradient);

                // create inverse from it
                basegfx::B2DHomMatrix aObjectToUnitGradient(aUnitGradientToObject);
                aObjectToUnitGradient.invert();

                // back-transform polygon to unit gradient coordinates and get
                // UnitRage. This is the range the gradient has to cover
                basegfx::B2DPolyPolygon aUnitPoly(getPolyPolygon());
                aUnitPoly.transform(aObjectToUnitGradient);
                const basegfx::B2DRange aUnitRange(aUnitPoly.getB2DRange());

                // prepare result vectors
                Primitive2DVector aTargetColor;
                Primitive2DVector aTargetOpacity;

                if(basegfx::fTools::more(aUnitRange.getWidth(), 0.0))
                {
                    // add a pre-multiply to aUnitGradientToObject to allow
                    // multiplication of the polygon(xl, 0.0, xr, 1.0)
                    const basegfx::B2DHomMatrix aPreMultiply(
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            1.0, aUnitRange.getHeight(), 0.0, aUnitRange.getMinY()));
                    aUnitGradientToObject = aUnitGradientToObject * aPreMultiply;

                    // create central run, may also already do all necessary when
                    // Spread_pad is set as SpreadMethod and/or the range is smaller
                    double fPos(createRun(aTargetColor, aTargetOpacity, aUnitRange.getMinX(), aUnitRange.getMaxX(), getGradientEntries(), 0));

                    if(fPos < aUnitRange.getMaxX())
                    {
                        // can only happen when SpreadMethod is Spread_reflect or Spread_repeat,
                        // else the start and end pads are already created and fPos == aUnitRange.getMaxX().
                        // Its possible to express the repeated linear gradient by adding the
                        // transformed central run. Crete it this way
                        Primitive2DSequence aTargetColorEntries(Primitive2DVectorToPrimitive2DSequence(aTargetColor));
                        Primitive2DSequence aTargetOpacityEntries(Primitive2DVectorToPrimitive2DSequence(aTargetOpacity));
                        aTargetColor.clear();
                        aTargetOpacity.clear();

                        if(aTargetColorEntries.hasElements())
                        {
                            // add original central run as group primitive
                            aTargetColor.push_back(new GroupPrimitive2D(aTargetColorEntries));

                            if(aTargetOpacityEntries.hasElements())
                            {
                                aTargetOpacity.push_back(new GroupPrimitive2D(aTargetOpacityEntries));
                            }

                            // add negative runs
                            fPos = 0.0;
                            sal_Int32 nOffset(0);

                            while(fPos > aUnitRange.getMinX())
                            {
                                fPos -= 1.0;
                                nOffset++;

                                basegfx::B2DHomMatrix aTransform;
                                const bool bMirror(Spread_reflect == getSpreadMethod() && (nOffset % 2));

                                if(bMirror)
                                {
                                    aTransform.scale(-1.0, 1.0);
                                    aTransform.translate(fPos + 1.0, 0.0);
                                }
                                else
                                {
                                    aTransform.translate(fPos, 0.0);
                                }

                                aTargetColor.push_back(new TransformPrimitive2D(aTransform, aTargetColorEntries));

                                if(aTargetOpacityEntries.hasElements())
                                {
                                    aTargetOpacity.push_back(new TransformPrimitive2D(aTransform, aTargetOpacityEntries));
                                }
                            }

                            // add positive runs
                            fPos = 1.0;
                            nOffset = 1;

                            while(fPos < aUnitRange.getMaxX())
                            {
                                basegfx::B2DHomMatrix aTransform;
                                const bool bMirror(Spread_reflect == getSpreadMethod() && (nOffset % 2));

                                if(bMirror)
                                {
                                    aTransform.scale(-1.0, 1.0);
                                    aTransform.translate(fPos + 1.0, 0.0);
                                }
                                else
                                {
                                    aTransform.translate(fPos, 0.0);
                                }

                                aTargetColor.push_back(new TransformPrimitive2D(aTransform, aTargetColorEntries));

                                if(aTargetOpacityEntries.hasElements())
                                {
                                    aTargetOpacity.push_back(new TransformPrimitive2D(aTransform, aTargetOpacityEntries));
                                }

                                fPos += 1.0;
                                nOffset++;
                            }
                        }
                    }
                }

                xRetval = createResult(aTargetColor, aTargetOpacity, aUnitGradientToObject);
            }

            return xRetval;
        }

        SvgLinearGradientPrimitive2D::SvgLinearGradientPrimitive2D(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const SvgGradientEntryVector& rGradientEntries,
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            SpreadMethod aSpreadMethod,
            double fOverlapping)
        :   BufferedDecompositionPrimitive2D(),
            SvgGradientHelper(rPolyPolygon, rGradientEntries, rStart, aSpreadMethod, fOverlapping),
            maEnd(rEnd)
        {
        }

        bool SvgLinearGradientPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            const SvgGradientHelper* pSvgGradientHelper = dynamic_cast< const SvgGradientHelper* >(&rPrimitive);

            if(pSvgGradientHelper && SvgGradientHelper::operator==(*pSvgGradientHelper))
            {
                const SvgLinearGradientPrimitive2D& rCompare = static_cast< const SvgLinearGradientPrimitive2D& >(rPrimitive);

                return (getEnd() == rCompare.getEnd());
            }

            return false;
        }

        basegfx::B2DRange SvgLinearGradientPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return ObjectRange
            return getPolyPolygon().getB2DRange();
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SvgLinearGradientPrimitive2D, PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        void SvgRadialGradientPrimitive2D::checkPreconditions()
        {
            // call parent
            SvgGradientHelper::checkPreconditions();

            if(getCreatesContent())
            {
                // Check Radius
                if(basegfx::fTools::equalZero(getRadius()))
                {
                    // fill with single color using last stop color
                    setSingleEntry();
                }
            }
        }

        void SvgRadialGradientPrimitive2D::ensureGeometry(
            basegfx::B2DPolyPolygon& rPolyPolygon,
            const SvgGradientEntry& rFrom,
            const SvgGradientEntry& rTo,
            sal_Int32 nOffset) const
        {
            if(!rPolyPolygon.count())
            {
                basegfx::B2DPolygon aPolygonA(basegfx::tools::createPolygonFromUnitCircle());
                basegfx::B2DPolygon aPolygonB(basegfx::tools::createPolygonFromUnitCircle());
                double fScaleFrom(rFrom.getOffset() + nOffset);
                const double fScaleTo(rTo.getOffset() + nOffset);

                if(fScaleFrom > getOverlapping())
                {
                    fScaleFrom -= getOverlapping();
                }

                if(isFocalSet())
                {
                    const basegfx::B2DVector aTranslateFrom(maFocalVector * (maFocalLength - fScaleFrom));
                    const basegfx::B2DVector aTranslateTo(maFocalVector * (maFocalLength - fScaleTo));

                    aPolygonA.transform(
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            fScaleFrom,
                            fScaleFrom,
                            aTranslateFrom.getX(),
                            aTranslateFrom.getY()));
                    aPolygonB.transform(
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            fScaleTo,
                            fScaleTo,
                            aTranslateTo.getX(),
                            aTranslateTo.getY()));
                }
                else
                {
                    aPolygonA.transform(
                        basegfx::tools::createScaleB2DHomMatrix(
                            fScaleFrom,
                            fScaleFrom));
                    aPolygonB.transform(
                        basegfx::tools::createScaleB2DHomMatrix(
                            fScaleTo,
                            fScaleTo));
                }

                // add the outer polygon first
                rPolyPolygon.append(aPolygonB);
                rPolyPolygon.append(aPolygonA);
            }
        }

        void SvgRadialGradientPrimitive2D::createAtom(
            Primitive2DVector& rTargetColor,
            Primitive2DVector& rTargetOpacity,
            const SvgGradientEntry& rFrom,
            const SvgGradientEntry& rTo,
            sal_Int32 nOffset) const
        {
            // create gradient atom [rFrom.getOffset() .. rTo.getOffset()] with (rFrom.getOffset() > rTo.getOffset())
            if(rFrom.getOffset() == rTo.getOffset())
            {
                OSL_ENSURE(false, "SvgGradient Atom creation with no step width (!)");
            }
            else
            {
                const bool bColorChange(rFrom.getColor() != rTo.getColor());
                const bool bOpacityChange(rFrom.getOpacity() != rTo.getOpacity());
                basegfx::B2DPolyPolygon aPolyPolygon;

                if(bColorChange)
                {
                    const double fScaleFrom(rFrom.getOffset() + nOffset);
                    const double fScaleTo(rTo.getOffset() + nOffset);

                    if(isFocalSet())
                    {
                        const basegfx::B2DVector aTranslateFrom(maFocalVector * (maFocalLength - fScaleFrom));
                        const basegfx::B2DVector aTranslateTo(maFocalVector * (maFocalLength - fScaleTo));

                        rTargetColor.push_back(
                            new SvgRadialAtomPrimitive2D(
                                rFrom.getColor(), fScaleFrom, aTranslateFrom,
                                rTo.getColor(), fScaleTo, aTranslateTo,
                                getOverlapping()));
                    }
                    else
                    {
                        rTargetColor.push_back(
                            new SvgRadialAtomPrimitive2D(
                                rFrom.getColor(), fScaleFrom,
                                rTo.getColor(), fScaleTo,
                                getOverlapping()));
                    }
                }
                else
                {
                    ensureGeometry(aPolyPolygon, rFrom, rTo, nOffset);
                    rTargetColor.push_back(
                        new PolyPolygonColorPrimitive2D(
                            aPolyPolygon,
                            rFrom.getColor()));
                }

                if(bOpacityChange)
                {
                    const double fTransFrom(1.0 - rFrom.getOpacity());
                    const double fTransTo(1.0 - rTo.getOpacity());
                    const basegfx::BColor aColorFrom(fTransFrom, fTransFrom, fTransFrom);
                    const basegfx::BColor aColorTo(fTransTo, fTransTo, fTransTo);
                    const double fScaleFrom(rFrom.getOffset() + nOffset);
                    const double fScaleTo(rTo.getOffset() + nOffset);

                    if(isFocalSet())
                    {
                        const basegfx::B2DVector aTranslateFrom(maFocalVector * (maFocalLength - fScaleFrom));
                        const basegfx::B2DVector aTranslateTo(maFocalVector * (maFocalLength - fScaleTo));

                        rTargetOpacity.push_back(
                            new SvgRadialAtomPrimitive2D(
                                aColorFrom, fScaleFrom, aTranslateFrom,
                                aColorTo, fScaleTo, aTranslateTo,
                                getOverlapping()));
                    }
                    else
                    {
                        rTargetOpacity.push_back(
                            new SvgRadialAtomPrimitive2D(
                                aColorFrom, fScaleFrom,
                                aColorTo, fScaleTo,
                                getOverlapping()));
                    }
                }
                else if(!getFullyOpaque())
                {
                    const double fTransparence(1.0 - rFrom.getOpacity());

                    ensureGeometry(aPolyPolygon, rFrom, rTo, nOffset);
                    rTargetOpacity.push_back(
                        new PolyPolygonColorPrimitive2D(
                            aPolyPolygon,
                            basegfx::BColor(fTransparence, fTransparence, fTransparence)));
                }
            }
        }

        const SvgGradientEntryVector& SvgRadialGradientPrimitive2D::getMirroredGradientEntries() const
        {
            if(maMirroredGradientEntries.empty() && !getGradientEntries().empty())
            {
                const_cast< SvgRadialGradientPrimitive2D* >(this)->createMirroredGradientEntries();
            }

            return maMirroredGradientEntries;
        }

        void SvgRadialGradientPrimitive2D::createMirroredGradientEntries()
        {
            if(maMirroredGradientEntries.empty() && !getGradientEntries().empty())
            {
                const sal_uInt32 nCount(getGradientEntries().size());
                maMirroredGradientEntries.clear();
                maMirroredGradientEntries.reserve(nCount);

                for(sal_uInt32 a(0); a < nCount; a++)
                {
                    const SvgGradientEntry& rCandidate = getGradientEntries()[nCount - 1 - a];

                    maMirroredGradientEntries.push_back(
                        SvgGradientEntry(
                            1.0 - rCandidate.getOffset(),
                            rCandidate.getColor(),
                            rCandidate.getOpacity()));
                }
            }
        }

        Primitive2DSequence SvgRadialGradientPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;

            if(!getPreconditionsChecked())
            {
                const_cast< SvgRadialGradientPrimitive2D* >(this)->checkPreconditions();
            }

            if(getSingleEntry())
            {
                // fill with last existing color
                xRetval = createSingleGradientEntryFill();
            }
            else if(getCreatesContent())
            {
                // at least two color stops in range [0.0 .. 1.0], sorted, non-null vector, not completely
                // invisible, width and height to fill are not empty
                const basegfx::B2DRange aPolyRange(getPolyPolygon().getB2DRange());
                const double fPolyWidth(aPolyRange.getWidth());
                const double fPolyHeight(aPolyRange.getHeight());

                // create ObjectTransform based on polygon range
                const basegfx::B2DHomMatrix aObjectTransform(
                    basegfx::tools::createScaleTranslateB2DHomMatrix(
                        fPolyWidth, fPolyHeight,
                        aPolyRange.getMinX(), aPolyRange.getMinY()));

                // create unit transform from unit vector to given linear gradient vector
                basegfx::B2DHomMatrix aUnitGradientToGradient;

                aUnitGradientToGradient.scale(getRadius(), getRadius());
                aUnitGradientToGradient.translate(getStart().getX(), getStart().getY());

                // create full transform from unit gradient coordinates to object coordinates
                // including the SvgGradient transformation
                basegfx::B2DHomMatrix aUnitGradientToObject(aObjectTransform * aUnitGradientToGradient);

                // create inverse from it
                basegfx::B2DHomMatrix aObjectToUnitGradient(aUnitGradientToObject);
                aObjectToUnitGradient.invert();

                // back-transform polygon to unit gradient coordinates and get
                // UnitRage. This is the range the gradient has to cover
                basegfx::B2DPolyPolygon aUnitPoly(getPolyPolygon());
                aUnitPoly.transform(aObjectToUnitGradient);
                const basegfx::B2DRange aUnitRange(aUnitPoly.getB2DRange());

                // create range which the gradient has to cover to cover the whole given geometry.
                // For circle, go from 0.0 to max radius in all directions (the corners)
                double fMax(basegfx::B2DVector(aUnitRange.getMinimum()).getLength());
                fMax = std::max(fMax, basegfx::B2DVector(aUnitRange.getMaximum()).getLength());
                fMax = std::max(fMax, basegfx::B2DVector(aUnitRange.getMinX(), aUnitRange.getMaxY()).getLength());
                fMax = std::max(fMax, basegfx::B2DVector(aUnitRange.getMaxX(), aUnitRange.getMinY()).getLength());

                // prepare result vectors
                Primitive2DVector aTargetColor;
                Primitive2DVector aTargetOpacity;

                if(0.0 < fMax)
                {
                    // prepare maFocalVector
                    if(isFocalSet())
                    {
                        const_cast< SvgRadialGradientPrimitive2D* >(this)->maFocalLength = fMax;
                    }

                    // create central run, may also already do all necessary when
                    // Spread_pad is set as SpreadMethod and/or the range is smaller
                    double fPos(createRun(aTargetColor, aTargetOpacity, 0.0, fMax, getGradientEntries(), 0));

                    if(fPos < fMax)
                    {
                        // can only happen when SpreadMethod is Spread_reflect or Spread_repeat,
                        // else the start and end pads are already created and fPos == fMax.
                        // For radial there is no way to transform the already created
                        // central run, it needs to be created from 1.0 to fMax
                        sal_Int32 nOffset(1);

                        while(fPos < fMax)
                        {
                            const bool bMirror(Spread_reflect == getSpreadMethod() && (nOffset % 2));

                            if(bMirror)
                            {
                                createRun(aTargetColor, aTargetOpacity, 0.0, fMax, getMirroredGradientEntries(), nOffset);
                            }
                            else
                            {
                                createRun(aTargetColor, aTargetOpacity, 0.0, fMax, getGradientEntries(), nOffset);
                            }

                            nOffset++;
                            fPos += 1.0;
                        }
                    }
                }

                xRetval = createResult(aTargetColor, aTargetOpacity, aUnitGradientToObject, true);
            }

            return xRetval;
        }

        SvgRadialGradientPrimitive2D::SvgRadialGradientPrimitive2D(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const SvgGradientEntryVector& rGradientEntries,
            const basegfx::B2DPoint& rStart,
            double fRadius,
            SpreadMethod aSpreadMethod,
            const basegfx::B2DPoint* pFocal,
            double fOverlapping)
        :   BufferedDecompositionPrimitive2D(),
            SvgGradientHelper(rPolyPolygon, rGradientEntries, rStart, aSpreadMethod, fOverlapping),
            mfRadius(fRadius),
            maFocal(rStart),
            maFocalVector(0.0, 0.0),
            maFocalLength(0.0),
            maMirroredGradientEntries(),
            mbFocalSet(false)
        {
            if(pFocal)
            {
                maFocal = *pFocal;
                maFocalVector = maFocal - getStart();
                mbFocalSet = true;
            }
        }

        bool SvgRadialGradientPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            const SvgGradientHelper* pSvgGradientHelper = dynamic_cast< const SvgGradientHelper* >(&rPrimitive);

            if(pSvgGradientHelper && SvgGradientHelper::operator==(*pSvgGradientHelper))
            {
                const SvgRadialGradientPrimitive2D& rCompare = static_cast< const SvgRadialGradientPrimitive2D& >(rPrimitive);

                if(getRadius() == rCompare.getRadius())
                {
                    if(isFocalSet() == rCompare.isFocalSet())
                    {
                        if(isFocalSet())
                        {
                            return getFocal() == rCompare.getFocal();
                        }
                        else
                        {
                            return true;
                        }
                    }
                }
            }

            return false;
        }

        basegfx::B2DRange SvgRadialGradientPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return ObjectRange
            return getPolyPolygon().getB2DRange();
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SvgRadialGradientPrimitive2D, PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// SvgLinearAtomPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SvgLinearAtomPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;
            const double fDelta(getOffsetB() - getOffsetA());

            if(!basegfx::fTools::equalZero(fDelta))
            {
                if(getColorA() == getColorB())
                {
                    const basegfx::B2DPolygon aPolygon(
                        basegfx::tools::createPolygonFromRect(
                            basegfx::B2DRange(
                                getOffsetA() - getOverlapping(),
                                0.0,
                                getOffsetB() + getOverlapping(),
                                1.0)));

                    xRetval.realloc(1);
                    xRetval[0] = new PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(aPolygon),
                        getColorA());
                }
                else
                {
                    // calc discrete length to change color all 2.5 pixels
                    sal_uInt32 nSteps(basegfx::fround(fDelta / (getDiscreteUnit() * 2.5)));

                    // use color distance, assume to do every 3rd
                    const double fColorDistance(getColorA().getDistance(getColorB()));
                    const sal_uInt32 nColorSteps(basegfx::fround(fColorDistance * (255.0 * 0.3)));
                    nSteps = std::min(nSteps, nColorSteps);

                    // roughly cut when too big
                    nSteps = std::min(nSteps, sal_uInt32(100));
                    nSteps = std::max(nSteps, sal_uInt32(1));

                    // preapare iteration
                    double fStart(0.0);
                    double fStep(fDelta / nSteps);

                    xRetval.realloc(nSteps);

                    for(sal_uInt32 a(0); a < nSteps; a++, fStart += fStep)
                    {
                        const double fLeft(getOffsetA() + fStart);
                        const double fRight(fLeft + fStep);
                        const basegfx::B2DPolygon aPolygon(
                            basegfx::tools::createPolygonFromRect(
                                basegfx::B2DRange(
                                    fLeft - getOverlapping(),
                                    0.0,
                                    fRight + getOverlapping(),
                                    1.0)));

                        xRetval[a] = new PolyPolygonColorPrimitive2D(
                            basegfx::B2DPolyPolygon(aPolygon),
                            basegfx::interpolate(getColorA(), getColorB(), fStart/fDelta));
                    }
                }
            }

            return xRetval;
        }

        bool SvgLinearAtomPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
            {
                const SvgLinearAtomPrimitive2D& rCompare = static_cast< const SvgLinearAtomPrimitive2D& >(rPrimitive);

                return (getColorA() == rCompare.getColorA()
                    && getColorB() == rCompare.getColorB()
                    && getOffsetA() == rCompare.getOffsetA()
                    && getOffsetB() == rCompare.getOffsetB()
                    && getOverlapping() == rCompare.getOverlapping());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SvgLinearAtomPrimitive2D, PRIMITIVE2D_ID_SVGLINEARATOMPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// SvgRadialAtomPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SvgRadialAtomPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;
            const double fDeltaScale(getScaleB() - getScaleA());

            if(!basegfx::fTools::equalZero(fDeltaScale))
            {
                if(getColorA() == getColorB())
                {
                    basegfx::B2DPolygon aPolygonA(basegfx::tools::createPolygonFromUnitCircle());
                    basegfx::B2DPolygon aPolygonB(basegfx::tools::createPolygonFromUnitCircle());
                    double fScaleA(getScaleA());
                    const double fScaleB(getScaleB());

                    if(fScaleA > getOverlapping())
                    {
                        fScaleA -= getOverlapping();
                    }

                    const bool bUseA(basegfx::fTools::equalZero(fScaleA));

                    if(getTranslateSet())
                    {
                        if(bUseA)
                        {
                            aPolygonA.transform(
                                basegfx::tools::createScaleTranslateB2DHomMatrix(
                                    fScaleA,
                                    fScaleA,
                                    getTranslateA().getX(),
                                    getTranslateA().getY()));
                        }

                        aPolygonB.transform(
                            basegfx::tools::createScaleTranslateB2DHomMatrix(
                                fScaleB,
                                fScaleB,
                                getTranslateB().getX(),
                                getTranslateB().getY()));
                    }
                    else
                    {
                        if(bUseA)
                        {
                            aPolygonA.transform(
                                basegfx::tools::createScaleB2DHomMatrix(
                                    fScaleA,
                                    fScaleA));
                        }

                        aPolygonB.transform(
                            basegfx::tools::createScaleB2DHomMatrix(
                                fScaleB,
                                fScaleB));
                    }

                    basegfx::B2DPolyPolygon aPolyPolygon(aPolygonB);

                    if(bUseA)
                    {
                        aPolyPolygon.append(aPolygonA);
                    }

                    xRetval.realloc(1);

                    xRetval[0] = new PolyPolygonColorPrimitive2D(
                        aPolyPolygon,
                        getColorA());
                }
                else
                {
                    // calc discrete length to change color all 2.5 pixels
                    sal_uInt32 nSteps(basegfx::fround(fDeltaScale / (getDiscreteUnit() * 2.5)));

                    // use color distance, assume to do every 3rd
                    const double fColorDistance(getColorA().getDistance(getColorB()));
                    const sal_uInt32 nColorSteps(basegfx::fround(fColorDistance * (255.0 * 0.3)));
                    nSteps = std::min(nSteps, nColorSteps);

                    // roughly cut when too big
                    nSteps = std::min(nSteps, sal_uInt32(100));
                    nSteps = std::max(nSteps, sal_uInt32(1));

                    // preapare iteration
                    double fStartScale(0.0);
                    double fStepScale(fDeltaScale / nSteps);

                    xRetval.realloc(nSteps);

                    for(sal_uInt32 a(0); a < nSteps; a++, fStartScale += fStepScale)
                    {
                        double fScaleA(getScaleA() + fStartScale);
                        const double fScaleB(fScaleA + fStepScale);
                        const double fUnitScale(fStartScale/fDeltaScale);
                        basegfx::B2DPolygon aPolygonA(basegfx::tools::createPolygonFromUnitCircle());
                        basegfx::B2DPolygon aPolygonB(basegfx::tools::createPolygonFromUnitCircle());

                        if(fScaleA > getOverlapping())
                        {
                            fScaleA -= getOverlapping();
                        }

                        const bool bUseA(basegfx::fTools::equalZero(fScaleA));

                        if(getTranslateSet())
                        {
                            const double fUnitScaleEnd((fStartScale + fStepScale)/fDeltaScale);
                            const basegfx::B2DVector aTranslateB(basegfx::interpolate(getTranslateA(), getTranslateB(), fUnitScaleEnd));

                            if(bUseA)
                            {
                                const basegfx::B2DVector aTranslateA(basegfx::interpolate(getTranslateA(), getTranslateB(), fUnitScale));

                                aPolygonA.transform(
                                    basegfx::tools::createScaleTranslateB2DHomMatrix(
                                        fScaleA,
                                        fScaleA,
                                        aTranslateA.getX(),
                                        aTranslateA.getY()));
                            }

                            aPolygonB.transform(
                                basegfx::tools::createScaleTranslateB2DHomMatrix(
                                    fScaleB,
                                    fScaleB,
                                    aTranslateB.getX(),
                                    aTranslateB.getY()));
                        }
                        else
                        {
                            if(bUseA)
                            {
                                aPolygonA.transform(
                                    basegfx::tools::createScaleB2DHomMatrix(
                                        fScaleA,
                                        fScaleA));
                            }

                            aPolygonB.transform(
                                basegfx::tools::createScaleB2DHomMatrix(
                                    fScaleB,
                                    fScaleB));
                        }

                        basegfx::B2DPolyPolygon aPolyPolygon(aPolygonB);

                        if(bUseA)
                        {
                            aPolyPolygon.append(aPolygonA);
                        }

                        xRetval[nSteps - 1 - a] = new PolyPolygonColorPrimitive2D(
                            aPolyPolygon,
                            basegfx::interpolate(getColorA(), getColorB(), fUnitScale));
                    }
                }
            }

            return xRetval;
        }

        bool SvgRadialAtomPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
            {
                const SvgRadialAtomPrimitive2D& rCompare = static_cast< const SvgRadialAtomPrimitive2D& >(rPrimitive);

                return (getColorA() == rCompare.getColorA()
                    && getColorB() == rCompare.getColorB()
                    && getScaleA() == rCompare.getScaleA()
                    && getScaleB() == rCompare.getScaleB()
                    && getTranslateA() == rCompare.getTranslateA()
                    && getTranslateB() == rCompare.getTranslateB()
                    && getOverlapping() == rCompare.getOverlapping());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(SvgRadialAtomPrimitive2D, PRIMITIVE2D_ID_SVGRADIALATOMPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
