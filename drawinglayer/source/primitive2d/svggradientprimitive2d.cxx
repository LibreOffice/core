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



using namespace com::sun::star;



namespace
{
    sal_uInt32 calculateStepsForSvgGradient(const basegfx::BColor& rColorA, const basegfx::BColor& rColorB, double fDelta, double fDiscreteUnit)
    {
        // use color distance, assume to do every color step (full quality)
        sal_uInt32 nSteps(basegfx::fround(rColorA.getDistance(rColorB) * 255.0));

        if(nSteps)
        {
            // calc discrete length to change color all 1.5 disctete units (pixels)
            const sal_uInt32 nDistSteps(basegfx::fround(fDelta / (fDiscreteUnit * 1.5)));

            nSteps = std::min(nSteps, nDistSteps);
        }

        // roughly cut when too big or too small
        nSteps = std::min(nSteps, sal_uInt32(255));
        nSteps = std::max(nSteps, sal_uInt32(1));

        return nSteps;
    }
} // end of anonymous namespace



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
            const basegfx::B2DHomMatrix& rGradientTransform,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const SvgGradientEntryVector& rGradientEntries,
            const basegfx::B2DPoint& rStart,
            bool bUseUnitCoordinates,
            SpreadMethod aSpreadMethod)
        :   maGradientTransform(rGradientTransform),
            maPolyPolygon(rPolyPolygon),
            maGradientEntries(rGradientEntries),
            maStart(rStart),
            maSpreadMethod(aSpreadMethod),
            mbPreconditionsChecked(false),
            mbCreatesContent(false),
            mbSingleEntry(false),
            mbFullyOpaque(true),
            mbUseUnitCoordinates(bUseUnitCoordinates)
        {
        }

        SvgGradientHelper::~SvgGradientHelper()
        {
        }

        bool SvgGradientHelper::operator==(const SvgGradientHelper& rSvgGradientHelper) const
        {
            const SvgGradientHelper& rCompare = static_cast< const SvgGradientHelper& >(rSvgGradientHelper);

            return (getGradientTransform() == rCompare.getGradientTransform()
                && getPolyPolygon() == rCompare.getPolyPolygon()
                && getGradientEntries() == rCompare.getGradientEntries()
                && getStart() == rCompare.getStart()
                && getUseUnitCoordinates() == rCompare.getUseUnitCoordinates()
                && getSpreadMethod() == rCompare.getSpreadMethod());
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer



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
                rTargetColor.push_back(
                    new SvgLinearAtomPrimitive2D(
                        rFrom.getColor(), rFrom.getOffset() + nOffset,
                        rTo.getColor(), rTo.getOffset() + nOffset));

                if(!getFullyOpaque())
                {
                    const double fTransFrom(1.0 - rFrom.getOpacity());
                    const double fTransTo(1.0 - rTo.getOpacity());
                    const basegfx::BColor aColorFrom(fTransFrom, fTransFrom, fTransFrom);
                    const basegfx::BColor aColorTo(fTransTo, fTransTo, fTransTo);

                    rTargetOpacity.push_back(
                        new SvgLinearAtomPrimitive2D(
                            aColorFrom, rFrom.getOffset() + nOffset,
                            aColorTo, rTo.getOffset() + nOffset));
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
                basegfx::B2DHomMatrix aUnitGradientToObject;

                if(getUseUnitCoordinates())
                {
                    // interpret in unit coordinate system -> object aspect ratio will scale result
                    // create unit transform from unit vector [0.0 .. 1.0] along the X-Axis to given
                    // gradient vector defined by Start,End
                    const basegfx::B2DVector aVector(getEnd() - getStart());
                    const double fVectorLength(aVector.getLength());

                    aUnitGradientToObject.scale(fVectorLength, 1.0);
                    aUnitGradientToObject.rotate(atan2(aVector.getY(), aVector.getX()));
                    aUnitGradientToObject.translate(getStart().getX(), getStart().getY());

                    if(!getGradientTransform().isIdentity())
                    {
                        aUnitGradientToObject = getGradientTransform() * aUnitGradientToObject;
                    }

                    // create full transform from unit gradient coordinates to object coordinates
                    // including the SvgGradient transformation
                    aUnitGradientToObject = aObjectTransform * aUnitGradientToObject;
                }
                else
                {
                    // interpret in object coordinate system -> object aspect ratio will not scale result
                    const basegfx::B2DPoint aStart(aObjectTransform * getStart());
                    const basegfx::B2DPoint aEnd(aObjectTransform * getEnd());
                    const basegfx::B2DVector aVector(aEnd - aStart);

                    aUnitGradientToObject.scale(aVector.getLength(), 1.0);
                    aUnitGradientToObject.rotate(atan2(aVector.getY(), aVector.getX()));
                    aUnitGradientToObject.translate(aStart.getX(), aStart.getY());

                    if(!getGradientTransform().isIdentity())
                    {
                        aUnitGradientToObject = getGradientTransform() * aUnitGradientToObject;
                    }
                }

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
                        // transformed central run. Create it this way
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
            const basegfx::B2DHomMatrix& rGradientTransform,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const SvgGradientEntryVector& rGradientEntries,
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            bool bUseUnitCoordinates,
            SpreadMethod aSpreadMethod)
        :   BufferedDecompositionPrimitive2D(),
            SvgGradientHelper(rGradientTransform, rPolyPolygon, rGradientEntries, rStart, bUseUnitCoordinates, aSpreadMethod),
            maEnd(rEnd)
        {
        }

        SvgLinearGradientPrimitive2D::~SvgLinearGradientPrimitive2D()
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
        ImplPrimitive2DIDBlock(SvgLinearGradientPrimitive2D, PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer



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
                const double fScaleFrom(rFrom.getOffset() + nOffset);
                const double fScaleTo(rTo.getOffset() + nOffset);

                if(isFocalSet())
                {
                    const basegfx::B2DVector aTranslateFrom(maFocalVector * (maFocalLength - fScaleFrom));
                    const basegfx::B2DVector aTranslateTo(maFocalVector * (maFocalLength - fScaleTo));

                    rTargetColor.push_back(
                        new SvgRadialAtomPrimitive2D(
                            rFrom.getColor(), fScaleFrom, aTranslateFrom,
                            rTo.getColor(), fScaleTo, aTranslateTo));
                }
                else
                {
                    rTargetColor.push_back(
                        new SvgRadialAtomPrimitive2D(
                            rFrom.getColor(), fScaleFrom,
                            rTo.getColor(), fScaleTo));
                }

                if(!getFullyOpaque())
                {
                    const double fTransFrom(1.0 - rFrom.getOpacity());
                    const double fTransTo(1.0 - rTo.getOpacity());
                    const basegfx::BColor aColorFrom(fTransFrom, fTransFrom, fTransFrom);
                    const basegfx::BColor aColorTo(fTransTo, fTransTo, fTransTo);

                    if(isFocalSet())
                    {
                        const basegfx::B2DVector aTranslateFrom(maFocalVector * (maFocalLength - fScaleFrom));
                        const basegfx::B2DVector aTranslateTo(maFocalVector * (maFocalLength - fScaleTo));

                        rTargetOpacity.push_back(
                            new SvgRadialAtomPrimitive2D(
                                aColorFrom, fScaleFrom, aTranslateFrom,
                                aColorTo, fScaleTo, aTranslateTo));
                    }
                    else
                    {
                        rTargetOpacity.push_back(
                            new SvgRadialAtomPrimitive2D(
                                aColorFrom, fScaleFrom,
                                aColorTo, fScaleTo));
                    }
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
                basegfx::B2DHomMatrix aUnitGradientToObject;

                if(getUseUnitCoordinates())
                {
                    // interpret in unit coordinate system -> object aspect ratio will scale result
                    // create unit transform from unit vector to given linear gradient vector
                    aUnitGradientToObject.scale(getRadius(), getRadius());
                    aUnitGradientToObject.translate(getStart().getX(), getStart().getY());

                    if(!getGradientTransform().isIdentity())
                    {
                        aUnitGradientToObject = getGradientTransform() * aUnitGradientToObject;
                    }

                    // create full transform from unit gradient coordinates to object coordinates
                    // including the SvgGradient transformation
                    aUnitGradientToObject = aObjectTransform * aUnitGradientToObject;
                }
                else
                {
                    // interpret in object coordinate system -> object aspect ratio will not scale result
                    // use X-Axis with radius, it was already made relative to object width when coming from
                    // SVG import
                    const double fRadius((aObjectTransform * basegfx::B2DVector(getRadius(), 0.0)).getLength());
                    const basegfx::B2DPoint aStart(aObjectTransform * getStart());

                    aUnitGradientToObject.scale(fRadius, fRadius);
                    aUnitGradientToObject.translate(aStart.getX(), aStart.getY());

                    if(!getGradientTransform().isIdentity())
                    {
                        aUnitGradientToObject = getGradientTransform() * aUnitGradientToObject;
                    }
                }

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
            const basegfx::B2DHomMatrix& rGradientTransform,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const SvgGradientEntryVector& rGradientEntries,
            const basegfx::B2DPoint& rStart,
            double fRadius,
            bool bUseUnitCoordinates,
            SpreadMethod aSpreadMethod,
            const basegfx::B2DPoint* pFocal)
        :   BufferedDecompositionPrimitive2D(),
            SvgGradientHelper(rGradientTransform, rPolyPolygon, rGradientEntries, rStart, bUseUnitCoordinates, aSpreadMethod),
            mfRadius(fRadius),
            maFocal(rStart),
            maFocalVector(0.0, 0.0),
            maFocalLength(0.0),
            maMirroredGradientEntries(),
            mbFocalSet(false)
        {
            if(pFocal && !pFocal->equal(getStart()))
            {
                maFocal = *pFocal;
                maFocalVector = maFocal - getStart();
                mbFocalSet = true;
            }
        }

        SvgRadialGradientPrimitive2D::~SvgRadialGradientPrimitive2D()
        {
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
        ImplPrimitive2DIDBlock(SvgRadialGradientPrimitive2D, PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer


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
                // use one discrete unit for overlap (one pixel)
                const double fDiscreteUnit(getDiscreteUnit());

                // use color distance and discrete lengths to calculate step count
                const sal_uInt32 nSteps(calculateStepsForSvgGradient(getColorA(), getColorB(), fDelta, fDiscreteUnit));

                // prepare polygon in needed width at start position (with discrete overlap)
                const basegfx::B2DPolygon aPolygon(
                    basegfx::tools::createPolygonFromRect(
                        basegfx::B2DRange(
                            getOffsetA() - fDiscreteUnit,
                            0.0,
                            getOffsetA() + (fDelta / nSteps) + fDiscreteUnit,
                            1.0)));

                // prepare loop (inside to outside, [0.0 .. 1.0[)
                double fUnitScale(0.0);
                const double fUnitStep(1.0 / nSteps);

                // prepare result set (known size)
                xRetval.realloc(nSteps);

                for(sal_uInt32 a(0); a < nSteps; a++, fUnitScale += fUnitStep)
                {
                    basegfx::B2DPolygon aNew(aPolygon);

                    aNew.transform(basegfx::tools::createTranslateB2DHomMatrix(fDelta * fUnitScale, 0.0));
                    xRetval[a] = new PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(aNew),
                        basegfx::interpolate(getColorA(), getColorB(), fUnitScale));
                }
            }

            return xRetval;
        }

        SvgLinearAtomPrimitive2D::SvgLinearAtomPrimitive2D(
            const basegfx::BColor& aColorA, double fOffsetA,
            const basegfx::BColor& aColorB, double fOffsetB)
        :   DiscreteMetricDependentPrimitive2D(),
            maColorA(aColorA),
            maColorB(aColorB),
            mfOffsetA(fOffsetA),
            mfOffsetB(fOffsetB)
        {
            if(mfOffsetA > mfOffsetB)
            {
                OSL_ENSURE(false, "Wrong offset order (!)");
                ::std::swap(mfOffsetA, mfOffsetB);
            }
        }

        bool SvgLinearAtomPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
            {
                const SvgLinearAtomPrimitive2D& rCompare = static_cast< const SvgLinearAtomPrimitive2D& >(rPrimitive);

                return (getColorA() == rCompare.getColorA()
                    && getColorB() == rCompare.getColorB()
                    && getOffsetA() == rCompare.getOffsetA()
                    && getOffsetB() == rCompare.getOffsetB());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SvgLinearAtomPrimitive2D, PRIMITIVE2D_ID_SVGLINEARATOMPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer


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
                // use one discrete unit for overlap (one pixel)
                const double fDiscreteUnit(getDiscreteUnit());

                // use color distance and discrete lengths to calculate step count
                const sal_uInt32 nSteps(calculateStepsForSvgGradient(getColorA(), getColorB(), fDeltaScale, fDiscreteUnit));

                // prepare loop ([0.0 .. 1.0[, full polygons, no polypolygons with holes)
                double fUnitScale(0.0);
                const double fUnitStep(1.0 / nSteps);

                // prepare result set (known size)
                xRetval.realloc(nSteps);

                for(sal_uInt32 a(0); a < nSteps; a++, fUnitScale += fUnitStep)
                {
                    basegfx::B2DHomMatrix aTransform;
                    const double fEndScale(getScaleB() - (fDeltaScale * fUnitScale));

                    if(isTranslateSet())
                    {
                        const basegfx::B2DVector aTranslate(
                            basegfx::interpolate(
                                getTranslateB(),
                                getTranslateA(),
                                fUnitScale));

                        aTransform = basegfx::tools::createScaleTranslateB2DHomMatrix(
                            fEndScale,
                            fEndScale,
                            aTranslate.getX(),
                            aTranslate.getY());
                    }
                    else
                    {
                        aTransform = basegfx::tools::createScaleB2DHomMatrix(
                            fEndScale,
                            fEndScale);
                    }

                    basegfx::B2DPolygon aNew(basegfx::tools::createPolygonFromUnitCircle());

                    aNew.transform(aTransform);
                    xRetval[a] = new PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(aNew),
                        basegfx::interpolate(getColorB(), getColorA(), fUnitScale));
                }
            }

            return xRetval;
        }

        SvgRadialAtomPrimitive2D::SvgRadialAtomPrimitive2D(
            const basegfx::BColor& aColorA, double fScaleA, const basegfx::B2DVector& rTranslateA,
            const basegfx::BColor& aColorB, double fScaleB, const basegfx::B2DVector& rTranslateB)
        :   DiscreteMetricDependentPrimitive2D(),
            maColorA(aColorA),
            maColorB(aColorB),
            mfScaleA(fScaleA),
            mfScaleB(fScaleB),
            mpTranslate(nullptr)
        {
            // check and evtl. set translations
            if(!rTranslateA.equal(rTranslateB))
            {
                mpTranslate = new VectorPair(rTranslateA, rTranslateB);
            }

            // scale A and B have to be positive
            mfScaleA = ::std::max(mfScaleA, 0.0);
            mfScaleB = ::std::max(mfScaleB, 0.0);

            // scale B has to be bigger than scale A; swap if different
            if(mfScaleA > mfScaleB)
            {
                OSL_ENSURE(false, "Wrong offset order (!)");
                ::std::swap(mfScaleA, mfScaleB);

                if(mpTranslate)
                {
                    ::std::swap(mpTranslate->maTranslateA, mpTranslate->maTranslateB);
                }
            }
        }

        SvgRadialAtomPrimitive2D::SvgRadialAtomPrimitive2D(
            const basegfx::BColor& aColorA, double fScaleA,
            const basegfx::BColor& aColorB, double fScaleB)
        :   DiscreteMetricDependentPrimitive2D(),
            maColorA(aColorA),
            maColorB(aColorB),
            mfScaleA(fScaleA),
            mfScaleB(fScaleB),
            mpTranslate(nullptr)
        {
            // scale A and B have to be positive
            mfScaleA = ::std::max(mfScaleA, 0.0);
            mfScaleB = ::std::max(mfScaleB, 0.0);

            // scale B has to be bigger than scale A; swap if different
            if(mfScaleA > mfScaleB)
            {
                OSL_ENSURE(false, "Wrong offset order (!)");
                ::std::swap(mfScaleA, mfScaleB);
            }
        }

        SvgRadialAtomPrimitive2D::~SvgRadialAtomPrimitive2D()
        {
            if(mpTranslate)
            {
                delete mpTranslate;
                mpTranslate = nullptr;
            }
        }

        bool SvgRadialAtomPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
            {
                const SvgRadialAtomPrimitive2D& rCompare = static_cast< const SvgRadialAtomPrimitive2D& >(rPrimitive);

                if(getColorA() == rCompare.getColorA()
                    && getColorB() == rCompare.getColorB()
                    && getScaleA() == rCompare.getScaleA()
                    && getScaleB() == rCompare.getScaleB())
                {
                    if(isTranslateSet() && rCompare.isTranslateSet())
                    {
                        return (getTranslateA() == rCompare.getTranslateA()
                            && getTranslateB() == rCompare.getTranslateB());
                    }
                    else if(!isTranslateSet() && !rCompare.isTranslateSet())
                    {
                        return true;
                    }
                }
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SvgRadialAtomPrimitive2D, PRIMITIVE2D_ID_SVGRADIALATOMPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
