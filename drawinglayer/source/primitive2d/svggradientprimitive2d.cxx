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
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <cmath>
#include <vcl/skia/SkiaHelper.hxx>

using namespace com::sun::star;


namespace
{
    sal_uInt32 calculateStepsForSvgGradient(const basegfx::BColor& rColorA, const basegfx::BColor& rColorB, double fDelta, double fDiscreteUnit)
    {
        // use color distance, assume to do every color step (full quality)
        sal_uInt32 nSteps(basegfx::fround(rColorA.getDistance(rColorB) * 255.0));

        if(nSteps)
        {
            // calc discrete length to change color all 1.5 discrete units (pixels)
            const sal_uInt32 nDistSteps(basegfx::fround(fDelta / (fDiscreteUnit * 1.5)));

            nSteps = std::min(nSteps, nDistSteps);
        }

        // roughly cut when too big or too small
        nSteps = std::min(nSteps, sal_uInt32(255));
        nSteps = std::max(nSteps, sal_uInt32(1));

        return nSteps;
    }
} // end of anonymous namespace


namespace drawinglayer::primitive2d
{
        void SvgGradientHelper::createSingleGradientEntryFill(Primitive2DContainer& rContainer) const
        {
            const SvgGradientEntryVector& rEntries = getGradientEntries();
            const sal_uInt32 nCount(rEntries.size());

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
                        Primitive2DContainer aContent { xRef };

                        xRef = Primitive2DReference(
                            new UnifiedTransparencePrimitive2D(
                                std::move(aContent),
                                1.0 - fOpacity));
                    }

                    rContainer.push_back(xRef);
                }
            }
            else
            {
                OSL_ENSURE(false, "Single gradient entry construction without entry (!)");
            }
        }

        void SvgGradientHelper::checkPreconditions()
        {
            mbPreconditionsChecked = true;
            const SvgGradientEntryVector& rEntries = getGradientEntries();

            if(rEntries.empty())
            {
                // no fill at all, done
                return;
            }

            // sort maGradientEntries by offset, small to big
            std::sort(maGradientEntries.begin(), maGradientEntries.end());

            // gradient with at least two colors
            bool bAllInvisible(true);
            bool bInvalidEntries(false);

            for(const SvgGradientEntry& rCandidate : rEntries)
            {
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

                if(!basegfx::fTools::betweenOrEqualEither(rCandidate.getOffset(), 0.0, 1.0))
                {
                    bInvalidEntries = true;
                }
            }

            if(bAllInvisible)
            {
                // all invisible, nothing to do
                return;
            }

            if(bInvalidEntries)
            {
                // invalid entries, do nothing
                SAL_WARN("drawinglayer", "SvgGradientHelper got invalid SvgGradientEntries outside [0.0 .. 1.0]");
                return;
            }

            const basegfx::B2DRange aPolyRange(getPolyPolygon().getB2DRange());

            if(aPolyRange.isEmpty())
            {
                // no range to fill, nothing to do
                return;
            }

            const double fPolyWidth(aPolyRange.getWidth());
            const double fPolyHeight(aPolyRange.getHeight());

            if(basegfx::fTools::equalZero(fPolyWidth) || basegfx::fTools::equalZero(fPolyHeight))
            {
                // no width/height to fill, nothing to do
                return;
            }

            mbCreatesContent = true;

            if(1 == rEntries.size())
            {
                // fill with single existing color
                setSingleEntry();
            }
        }

        const SvgGradientEntry& SvgGradientHelper::FindEntryLessOrEqual(
            sal_Int32& rInt,
            const double fFrac) const
        {
            const bool bMirror(SpreadMethod::Reflect == getSpreadMethod() && 0 != rInt % 2);
            const SvgGradientEntryVector& rCurrent(bMirror ? getMirroredGradientEntries() : getGradientEntries());

            for(SvgGradientEntryVector::const_reverse_iterator aIter(rCurrent.rbegin()); aIter != rCurrent.rend(); ++aIter)
            {
               if(basegfx::fTools::lessOrEqual(aIter->getOffset(), fFrac))
               {
                   return *aIter;
               }
            }

            // walk over gap to the left, be prepared for missing 0.0/1.0 entries
            rInt--;
            const bool bMirror2(SpreadMethod::Reflect == getSpreadMethod() && 0 != rInt % 2);
            const SvgGradientEntryVector& rCurrent2(bMirror2 ? getMirroredGradientEntries() : getGradientEntries());
            return rCurrent2.back();
        }

        const SvgGradientEntry& SvgGradientHelper::FindEntryMore(
            sal_Int32& rInt,
            const double fFrac) const
        {
            const bool bMirror(SpreadMethod::Reflect == getSpreadMethod() && 0 != rInt % 2);
            const SvgGradientEntryVector& rCurrent(bMirror ? getMirroredGradientEntries() : getGradientEntries());

            for(SvgGradientEntryVector::const_iterator aIter(rCurrent.begin()); aIter != rCurrent.end(); ++aIter)
            {
               if(basegfx::fTools::more(aIter->getOffset(), fFrac))
               {
                   return *aIter;
               }
            }

            // walk over gap to the right, be prepared for missing 0.0/1.0 entries
            rInt++;
            const bool bMirror2(SpreadMethod::Reflect == getSpreadMethod() && 0 != rInt % 2);
            const SvgGradientEntryVector& rCurrent2(bMirror2 ? getMirroredGradientEntries() : getGradientEntries());
            return rCurrent2.front();
        }

        // tdf#124424 Adapted creation of color runs to do in a single effort. Previous
        // version tried to do this from [0.0 .. 1.0] and to re-use transformed versions
        // in the caller if SpreadMethod was on some repeat mode, but had problems when
        // e.g. like in the bugdoc from the task a negative-only fStart/fEnd run was
        // requested in which case it did nothing. Even when reusing the spread might
        // not have been a full one from [0.0 .. 1.0].
        // This gets complicated due to mirrored runs, but also for gradient definitions
        // with missing entries for 0.0 and 1.0 in which case these have to be guessed
        // to be there with same parametrisation as their nearest existing entries. These
        // *could* have been added at checkPreconditions() but would then create unnecessary
        // spreads on zone overlaps.
        void SvgGradientHelper::createRun(
            Primitive2DContainer& rTargetColor,
            Primitive2DContainer& rTargetOpacity,
            double fStart,
            double fEnd) const
        {
            double fInt(0.0);
            double fFrac(0.0);
            double fEnd2(0.0);

            if(SpreadMethod::Pad == getSpreadMethod())
            {
                if(fStart < 0.0)
                {
                    fFrac = std::modf(fStart, &fInt);
                    const SvgGradientEntry& rFront(getGradientEntries().front());
                    const SvgGradientEntry aTemp(1.0 + fFrac, rFront.getColor(), rFront.getOpacity());
                    createAtom(rTargetColor, rTargetOpacity, aTemp, rFront, static_cast<sal_Int32>(fInt - 1), 0);
                    fStart = rFront.getOffset();
                }

                if(fEnd > 1.0)
                {
                    // change fEnd early, but create geometry later (after range below)
                    fEnd2 = fEnd;
                    fEnd = getGradientEntries().back().getOffset();
                }
            }

            while(fStart < fEnd)
            {
                fFrac = std::modf(fStart, &fInt);

                if(fFrac < 0.0)
                {
                    fInt -= 1;
                    fFrac = 1.0 + fFrac;
                }

                sal_Int32 nIntLeft(static_cast<sal_Int32>(fInt));
                sal_Int32 nIntRight(nIntLeft);

                const SvgGradientEntry& rLeft(FindEntryLessOrEqual(nIntLeft, fFrac));
                const SvgGradientEntry& rRight(FindEntryMore(nIntRight, fFrac));
                createAtom(rTargetColor, rTargetOpacity, rLeft, rRight, nIntLeft, nIntRight);

                const double fNextfStart(static_cast<double>(nIntRight) + rRight.getOffset());

                if(basegfx::fTools::more(fNextfStart, fStart))
                {
                    fStart = fNextfStart;
                }
                else
                {
                    SAL_WARN("drawinglayer", "SvgGradientHelper spread error");
                    fStart += 1.0;
                }
            }

            if(fEnd2 > 1.0)
            {
                // create end run for SpreadMethod::Pad late to keep correct creation order
                fFrac = std::modf(fEnd2, &fInt);
                const SvgGradientEntry& rBack(getGradientEntries().back());
                const SvgGradientEntry aTemp(fFrac, rBack.getColor(), rBack.getOpacity());
                createAtom(rTargetColor, rTargetOpacity, rBack, aTemp, 0, static_cast<sal_Int32>(fInt));
            }
        }

        void SvgGradientHelper::createResult(
            Primitive2DContainer& rContainer,
            const Primitive2DContainer& rTargetColor,
            const Primitive2DContainer& rTargetOpacity,
            const basegfx::B2DHomMatrix& rUnitGradientToObject,
            bool bInvert) const
        {
            Primitive2DContainer aTargetColorEntries(rTargetColor.maybeInvert(bInvert));
            Primitive2DContainer aTargetOpacityEntries(rTargetOpacity.maybeInvert(bInvert));

            if(aTargetColorEntries.empty())
                return;

            Primitive2DReference xRefContent;

            if(!aTargetOpacityEntries.empty())
            {
                const Primitive2DReference xRefOpacity = new TransparencePrimitive2D(
                    std::move(aTargetColorEntries),
                    std::move(aTargetOpacityEntries));

                xRefContent = new TransformPrimitive2D(
                    rUnitGradientToObject,
                    Primitive2DContainer { xRefOpacity });
            }
            else
            {
                xRefContent = new TransformPrimitive2D(
                    rUnitGradientToObject,
                    std::move(aTargetColorEntries));
            }

            rContainer.push_back(new MaskPrimitive2D(
                getPolyPolygon(),
                Primitive2DContainer { xRefContent }));
        }

        SvgGradientHelper::SvgGradientHelper(
            const basegfx::B2DHomMatrix& rGradientTransform,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            SvgGradientEntryVector&& rGradientEntries,
            const basegfx::B2DPoint& rStart,
            bool bUseUnitCoordinates,
            SpreadMethod aSpreadMethod)
        :   maGradientTransform(rGradientTransform),
            maPolyPolygon(rPolyPolygon),
            maGradientEntries(std::move(rGradientEntries)),
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

        const SvgGradientEntryVector& SvgGradientHelper::getMirroredGradientEntries() const
        {
            if(maMirroredGradientEntries.empty() && !getGradientEntries().empty())
            {
                const_cast< SvgGradientHelper* >(this)->createMirroredGradientEntries();
            }

            return maMirroredGradientEntries;
        }

        void SvgGradientHelper::createMirroredGradientEntries()
        {
            if(!maMirroredGradientEntries.empty() || getGradientEntries().empty())
                return;

            const sal_uInt32 nCount(getGradientEntries().size());
            maMirroredGradientEntries.clear();
            maMirroredGradientEntries.reserve(nCount);

            for(sal_uInt32 a(0); a < nCount; a++)
            {
                const SvgGradientEntry& rCandidate = getGradientEntries()[nCount - 1 - a];

                maMirroredGradientEntries.emplace_back(
                        1.0 - rCandidate.getOffset(),
                        rCandidate.getColor(),
                        rCandidate.getOpacity());
            }
        }

        bool SvgGradientHelper::operator==(const SvgGradientHelper& rSvgGradientHelper) const
        {
            const SvgGradientHelper& rCompare = rSvgGradientHelper;

            return (getGradientTransform() == rCompare.getGradientTransform()
                && getPolyPolygon() == rCompare.getPolyPolygon()
                && getGradientEntries() == rCompare.getGradientEntries()
                && getStart() == rCompare.getStart()
                && getUseUnitCoordinates() == rCompare.getUseUnitCoordinates()
                && getSpreadMethod() == rCompare.getSpreadMethod());
        }

} // end of namespace drawinglayer::primitive2d


namespace drawinglayer::primitive2d
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
            Primitive2DContainer& rTargetColor,
            Primitive2DContainer& rTargetOpacity,
            const SvgGradientEntry& rFrom,
            const SvgGradientEntry& rTo,
            sal_Int32 nOffsetFrom,
            sal_Int32 nOffsetTo) const
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
                        rFrom.getColor(), rFrom.getOffset() + nOffsetFrom,
                        rTo.getColor(), rTo.getOffset() + nOffsetTo));

                if(!getFullyOpaque())
                {
                    const double fTransFrom(1.0 - rFrom.getOpacity());
                    const double fTransTo(1.0 - rTo.getOpacity());
                    const basegfx::BColor aColorFrom(fTransFrom, fTransFrom, fTransFrom);
                    const basegfx::BColor aColorTo(fTransTo, fTransTo, fTransTo);

                    rTargetOpacity.push_back(
                        new SvgLinearAtomPrimitive2D(
                            aColorFrom, rFrom.getOffset() + nOffsetFrom,
                            aColorTo, rTo.getOffset() + nOffsetTo));
                }
            }
        }

        void SvgLinearGradientPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(!getPreconditionsChecked())
            {
                const_cast< SvgLinearGradientPrimitive2D* >(this)->checkPreconditions();
            }

            if(getSingleEntry())
            {
                // fill with last existing color
                createSingleGradientEntryFill(rContainer);
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
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
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

                    aUnitGradientToObject *= getGradientTransform();

                    // create full transform from unit gradient coordinates to object coordinates
                    // including the SvgGradient transformation
                    aUnitGradientToObject *= aObjectTransform;
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

                    aUnitGradientToObject *= getGradientTransform();
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
                Primitive2DContainer aTargetColor;
                Primitive2DContainer aTargetOpacity;

                if(basegfx::fTools::more(aUnitRange.getWidth(), 0.0))
                {
                    // add a pre-multiply to aUnitGradientToObject to allow
                    // multiplication of the polygon(xl, 0.0, xr, 1.0)
                    const basegfx::B2DHomMatrix aPreMultiply(
                        basegfx::utils::createScaleTranslateB2DHomMatrix(
                            1.0, aUnitRange.getHeight(), 0.0, aUnitRange.getMinY()));
                    aUnitGradientToObject = aUnitGradientToObject * aPreMultiply;

                    // create full color run, including all SpreadMethod variants
                    createRun(
                        aTargetColor,
                        aTargetOpacity,
                        aUnitRange.getMinX(),
                        aUnitRange.getMaxX());
                }

                createResult(rContainer, aTargetColor, aTargetOpacity, aUnitGradientToObject);
            }
        }

        SvgLinearGradientPrimitive2D::SvgLinearGradientPrimitive2D(
            const basegfx::B2DHomMatrix& rGradientTransform,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            SvgGradientEntryVector&& rGradientEntries,
            const basegfx::B2DPoint& rStart,
            const basegfx::B2DPoint& rEnd,
            bool bUseUnitCoordinates,
            SpreadMethod aSpreadMethod)
        :   SvgGradientHelper(rGradientTransform, rPolyPolygon, std::move(rGradientEntries), rStart, bUseUnitCoordinates, aSpreadMethod),
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
        sal_uInt32 SvgLinearGradientPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SVGLINEARGRADIENTPRIMITIVE2D;
        }

} // end of namespace drawinglayer::primitive2d


namespace drawinglayer::primitive2d
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
            Primitive2DContainer& rTargetColor,
            Primitive2DContainer& rTargetOpacity,
            const SvgGradientEntry& rFrom,
            const SvgGradientEntry& rTo,
            sal_Int32 nOffsetFrom,
            sal_Int32 nOffsetTo) const
        {
            // create gradient atom [rFrom.getOffset() .. rTo.getOffset()] with (rFrom.getOffset() > rTo.getOffset())
            if(rFrom.getOffset() == rTo.getOffset())
            {
                OSL_ENSURE(false, "SvgGradient Atom creation with no step width (!)");
            }
            else
            {
                const double fScaleFrom(rFrom.getOffset() + nOffsetFrom);
                const double fScaleTo(rTo.getOffset() + nOffsetTo);

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

        void SvgRadialGradientPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(!getPreconditionsChecked())
            {
                const_cast< SvgRadialGradientPrimitive2D* >(this)->checkPreconditions();
            }

            if(getSingleEntry())
            {
                // fill with last existing color
                createSingleGradientEntryFill(rContainer);
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
                    basegfx::utils::createScaleTranslateB2DHomMatrix(
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

                    aUnitGradientToObject *= getGradientTransform();
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
                Primitive2DContainer aTargetColor;
                Primitive2DContainer aTargetOpacity;

                if(0.0 < fMax)
                {
                    // prepare maFocalVector
                    if(isFocalSet())
                    {
                        const_cast< SvgRadialGradientPrimitive2D* >(this)->maFocalLength = fMax;
                    }

                    // create full color run, including all SpreadMethod variants
                    createRun(
                        aTargetColor,
                        aTargetOpacity,
                        0.0,
                        fMax);
                }

                createResult(rContainer, aTargetColor, aTargetOpacity, aUnitGradientToObject, true);
            }
        }

        SvgRadialGradientPrimitive2D::SvgRadialGradientPrimitive2D(
            const basegfx::B2DHomMatrix& rGradientTransform,
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            SvgGradientEntryVector&& rGradientEntries,
            const basegfx::B2DPoint& rStart,
            double fRadius,
            bool bUseUnitCoordinates,
            SpreadMethod aSpreadMethod,
            const basegfx::B2DPoint* pFocal)
        :   SvgGradientHelper(rGradientTransform, rPolyPolygon, std::move(rGradientEntries), rStart, bUseUnitCoordinates, aSpreadMethod),
            mfRadius(fRadius),
            maFocal(rStart),
            maFocalVector(0.0, 0.0),
            maFocalLength(0.0),
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
        sal_uInt32 SvgRadialGradientPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SVGRADIALGRADIENTPRIMITIVE2D;
        }

} // end of namespace drawinglayer::primitive2d


// SvgLinearAtomPrimitive2D class

namespace drawinglayer::primitive2d
{
        void SvgLinearAtomPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            const double fDelta(getOffsetB() - getOffsetA());

            if(basegfx::fTools::equalZero(fDelta))
                return;

            // use one discrete unit for overlap (one pixel)
            const double fDiscreteUnit(getDiscreteUnit());

            // use color distance and discrete lengths to calculate step count
            const sal_uInt32 nSteps(calculateStepsForSvgGradient(getColorA(), getColorB(), fDelta, fDiscreteUnit));

            // HACK: Splitting a gradient into adjacent polygons with gradually changing color is silly.
            // If antialiasing is used to draw them, the AA-ed adjacent edges won't line up perfectly
            // because of the AA (see SkiaSalGraphicsImpl::mergePolyPolygonToPrevious()).
            // Make the polygons a bit wider, so they the partial overlap "fixes" this.
            const double fixup = SkiaHelper::isVCLSkiaEnabled() ? fDiscreteUnit / 2 : 0;

            // tdf#117949 Use a small amount of discrete overlap at the edges. Usually this
            // should be exactly 0.0 and 1.0, but there were cases when this gets clipped
            // against the mask polygon which got numerically problematic.
            // This change is unnecessary in that respect, but avoids that numerical havoc
            // by at the same time doing no real harm AFAIK
            // TTTT: Remove again when clipping is fixed (!)

            // prepare polygon in needed width at start position (with discrete overlap)
            const basegfx::B2DPolygon aPolygon(
                basegfx::utils::createPolygonFromRect(
                    basegfx::B2DRange(
                        getOffsetA() - fDiscreteUnit,
                        -0.0001, // TTTT -> should be 0.0, see comment above
                        getOffsetA() + (fDelta / nSteps) + fDiscreteUnit + fixup,
                        1.0001))); // TTTT -> should be 1.0, see comment above

            // prepare loop (inside to outside, [0.0 .. 1.0[)
            double fUnitScale(0.0);
            const double fUnitStep(1.0 / nSteps);

            for(sal_uInt32 a(0); a < nSteps; a++, fUnitScale += fUnitStep)
            {
                basegfx::B2DPolygon aNew(aPolygon);

                aNew.transform(basegfx::utils::createTranslateB2DHomMatrix(fDelta * fUnitScale, 0.0));
                rContainer.push_back(new PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(aNew),
                    basegfx::interpolate(getColorA(), getColorB(), fUnitScale)));
            }
        }

        SvgLinearAtomPrimitive2D::SvgLinearAtomPrimitive2D(
            const basegfx::BColor& aColorA, double fOffsetA,
            const basegfx::BColor& aColorB, double fOffsetB)
        :   maColorA(aColorA),
            maColorB(aColorB),
            mfOffsetA(fOffsetA),
            mfOffsetB(fOffsetB)
        {
            if(mfOffsetA > mfOffsetB)
            {
                OSL_ENSURE(false, "Wrong offset order (!)");
                std::swap(mfOffsetA, mfOffsetB);
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
        sal_uInt32 SvgLinearAtomPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SVGLINEARATOMPRIMITIVE2D;
        }

} // end of namespace drawinglayer::primitive2d


// SvgRadialAtomPrimitive2D class

namespace drawinglayer::primitive2d
{
        void SvgRadialAtomPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            const double fDeltaScale(getScaleB() - getScaleA());

            if(basegfx::fTools::equalZero(fDeltaScale))
                return;

            // use one discrete unit for overlap (one pixel)
            const double fDiscreteUnit(getDiscreteUnit());

            // use color distance and discrete lengths to calculate step count
            const sal_uInt32 nSteps(calculateStepsForSvgGradient(getColorA(), getColorB(), fDeltaScale, fDiscreteUnit));

            // prepare loop ([0.0 .. 1.0[, full polygons, no polypolygons with holes)
            double fUnitScale(0.0);
            const double fUnitStep(1.0 / nSteps);

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

                    aTransform = basegfx::utils::createScaleTranslateB2DHomMatrix(
                        fEndScale,
                        fEndScale,
                        aTranslate.getX(),
                        aTranslate.getY());
                }
                else
                {
                    aTransform = basegfx::utils::createScaleB2DHomMatrix(
                        fEndScale,
                        fEndScale);
                }

                basegfx::B2DPolygon aNew(basegfx::utils::createPolygonFromUnitCircle());

                aNew.transform(aTransform);
                rContainer.push_back(new PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(aNew),
                    basegfx::interpolate(getColorB(), getColorA(), fUnitScale)));
            }
        }

        SvgRadialAtomPrimitive2D::SvgRadialAtomPrimitive2D(
            const basegfx::BColor& aColorA, double fScaleA, const basegfx::B2DVector& rTranslateA,
            const basegfx::BColor& aColorB, double fScaleB, const basegfx::B2DVector& rTranslateB)
        :   maColorA(aColorA),
            maColorB(aColorB),
            mfScaleA(fScaleA),
            mfScaleB(fScaleB)
        {
            // check and evtl. set translations
            if(!rTranslateA.equal(rTranslateB))
            {
                mpTranslate.reset( new VectorPair(rTranslateA, rTranslateB) );
            }

            // scale A and B have to be positive
            mfScaleA = std::max(mfScaleA, 0.0);
            mfScaleB = std::max(mfScaleB, 0.0);

            // scale B has to be bigger than scale A; swap if different
            if(mfScaleA > mfScaleB)
            {
                OSL_ENSURE(false, "Wrong offset order (!)");
                std::swap(mfScaleA, mfScaleB);

                if(mpTranslate)
                {
                    std::swap(mpTranslate->maTranslateA, mpTranslate->maTranslateB);
                }
            }
        }

        SvgRadialAtomPrimitive2D::SvgRadialAtomPrimitive2D(
            const basegfx::BColor& aColorA, double fScaleA,
            const basegfx::BColor& aColorB, double fScaleB)
        :   maColorA(aColorA),
            maColorB(aColorB),
            mfScaleA(fScaleA),
            mfScaleB(fScaleB)
        {
            // scale A and B have to be positive
            mfScaleA = std::max(mfScaleA, 0.0);
            mfScaleB = std::max(mfScaleB, 0.0);

            // scale B has to be bigger than scale A; swap if different
            if(mfScaleA > mfScaleB)
            {
                OSL_ENSURE(false, "Wrong offset order (!)");
                std::swap(mfScaleA, mfScaleB);
            }
        }

        SvgRadialAtomPrimitive2D::~SvgRadialAtomPrimitive2D()
        {
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
        sal_uInt32 SvgRadialAtomPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SVGRADIALATOMPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
