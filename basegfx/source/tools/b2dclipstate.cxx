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

#include <basegfx/utils/b2dclipstate.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2dpolyrange.hxx>
#include <basegfx/range/b2drangeclipper.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>


namespace basegfx
{
namespace utils
{
    class ImplB2DClipState
    {
    public:
        enum Operation {UNION, INTERSECT, XOR, SUBTRACT};

        ImplB2DClipState() :
            maPendingPolygons(),
            maPendingRanges(),
            maClipPoly(),
            mePendingOps(UNION)
        {}

        explicit ImplB2DClipState( const B2DPolyPolygon& rPoly ) :
            maPendingPolygons(),
            maPendingRanges(),
            maClipPoly(rPoly),
            mePendingOps(UNION)
        {}

        bool isCleared() const
        {
            return !maClipPoly.count()
                && !maPendingPolygons.count()
                && !maPendingRanges.count();
        }

        bool isNullClipPoly() const
        {
            return maClipPoly.count() == 1
                && !maClipPoly.getB2DPolygon(0).count();
        }

        bool isNull() const
        {
            return !maPendingPolygons.count()
                && !maPendingRanges.count()
                && isNullClipPoly();
        }

        void makeNull()
        {
            maPendingPolygons.clear();
            maPendingRanges.clear();
            maClipPoly.clear();
            maClipPoly.append(B2DPolygon());
            mePendingOps = UNION;
        }

        bool operator==(const ImplB2DClipState& rRHS) const
        {
            return maPendingPolygons == rRHS.maPendingPolygons
                && maPendingRanges == rRHS.maPendingRanges
                && maClipPoly == rRHS.maClipPoly
                && mePendingOps == rRHS.mePendingOps;
        }

        void addRange(const B2DRange& rRange, Operation eOp)
        {
            if( rRange.isEmpty() )
                return;

            commitPendingPolygons();
            if( mePendingOps != eOp )
                commitPendingRanges();

            mePendingOps = eOp;
            maPendingRanges.appendElement(
                rRange,
                B2VectorOrientation::Positive);
        }

        void addPolyPolygon(const B2DPolyPolygon& aPoly, Operation eOp)
        {
            commitPendingRanges();
            if( mePendingOps != eOp )
                commitPendingPolygons();

            mePendingOps = eOp;
            maPendingPolygons.append(aPoly);
        }

        void unionRange(const B2DRange& rRange)
        {
            if( isCleared() )
                return;

            addRange(rRange,UNION);
        }

        void unionPolyPolygon(const B2DPolyPolygon& rPolyPoly)
        {
            if( isCleared() )
                return;

            addPolyPolygon(rPolyPoly,UNION);
        }

        void intersectRange(const B2DRange& rRange)
        {
            if( isNull() )
                return;

            addRange(rRange,INTERSECT);
        }

        void intersectPolyPolygon(const B2DPolyPolygon& rPolyPoly)
        {
            if( isNull() )
                return;

            addPolyPolygon(rPolyPoly,INTERSECT);
        }

        void subtractRange(const B2DRange& rRange )
        {
            if( isNull() )
                return;

            addRange(rRange,SUBTRACT);
        }

        void subtractPolyPolygon(const B2DPolyPolygon& rPolyPoly)
        {
            if( isNull() )
                return;

            addPolyPolygon(rPolyPoly,SUBTRACT);
        }

        void xorRange(const B2DRange& rRange)
        {
            addRange(rRange,XOR);
        }

        void xorPolyPolygon(const B2DPolyPolygon& rPolyPoly)
        {
            addPolyPolygon(rPolyPoly,XOR);
        }

        void transform(const basegfx::B2DHomMatrix& rTranslate)
        {
            maPendingRanges.transform(rTranslate);
            maPendingPolygons.transform(rTranslate);
            maClipPoly.transform(rTranslate);
        }

        B2DPolyPolygon const & getClipPoly() const
        {
            commitPendingRanges();
            commitPendingPolygons();

            return maClipPoly;
        }

    private:
        void commitPendingPolygons() const
        {
            if( !maPendingPolygons.count() )
                return;

            // assumption: maClipPoly has kept polygons prepared for
            // clipping; i.e. no neutral polygons & correct
            // orientation
            maPendingPolygons = utils::prepareForPolygonOperation(maPendingPolygons);
            const bool bIsEmpty=isNullClipPoly();
            const bool bIsCleared=!maClipPoly.count();
            switch(mePendingOps)
            {
                case UNION:
                    assert( !bIsCleared );

                    if( bIsEmpty )
                        maClipPoly = maPendingPolygons;
                    else
                        maClipPoly = utils::solvePolygonOperationOr(
                            maClipPoly,
                            maPendingPolygons);
                    break;
                case INTERSECT:
                    assert( !bIsEmpty );

                    if( bIsCleared )
                        maClipPoly = maPendingPolygons;
                    else
                        maClipPoly = utils::solvePolygonOperationAnd(
                            maClipPoly,
                            maPendingPolygons);
                    break;
                case XOR:
                    if( bIsEmpty )
                        maClipPoly = maPendingPolygons;
                    else if( bIsCleared )
                    {
                        // not representable, strictly speaking,
                        // using polygons with the common even/odd
                        // or nonzero winding number fill rule. If
                        // we'd want to represent it, fill rule
                        // would need to be "non-negative winding
                        // number" (and we then would return
                        // 'holes' here)

                        // going for an ugly hack meanwhile
                        maClipPoly = utils::solvePolygonOperationXor(
                            B2DPolyPolygon(
                                utils::createPolygonFromRect(B2DRange(-1E20,-1E20,1E20,1E20))),
                            maPendingPolygons);
                    }
                    else
                        maClipPoly = utils::solvePolygonOperationXor(
                            maClipPoly,
                            maPendingPolygons);
                    break;
                case SUBTRACT:
                    assert( !bIsEmpty );

                    // first union all pending ones, subtract en bloc then
                    maPendingPolygons = utils::solvePolygonOperationOr(
                        utils::packInB2DPolyPolygonVector(maPendingPolygons));

                    if( bIsCleared )
                    {
                        // not representable, strictly speaking,
                        // using polygons with the common even/odd
                        // or nonzero winding number fill rule. If
                        // we'd want to represent it, fill rule
                        // would need to be "non-negative winding
                        // number" (and we then would return
                        // 'holes' here)

                        // going for an ugly hack meanwhile
                        maClipPoly = utils::solvePolygonOperationDiff(
                            B2DPolyPolygon(
                                utils::createPolygonFromRect(B2DRange(-1E20,-1E20,1E20,1E20))),
                            maPendingPolygons);
                    }
                    else
                        maClipPoly = utils::solvePolygonOperationDiff(
                            maClipPoly,
                            maPendingPolygons);
                    break;
            }

            maPendingPolygons.clear();
            mePendingOps = UNION;
        }

        void commitPendingRanges() const
        {
            if( !maPendingRanges.count() )
                return;

            // use the specialized range clipper for the win
            B2DPolyPolygon aCollectedRanges;
            const bool bIsEmpty=isNullClipPoly();
            const bool bIsCleared=!maClipPoly.count();
            switch(mePendingOps)
            {
                case UNION:
                    assert( !bIsCleared );
                    aCollectedRanges = utils::solvePolygonOperationOr(
                        utils::packInB2DPolyPolygonVector(maPendingRanges.solveCrossovers()));

                    if( bIsEmpty )
                        maClipPoly = aCollectedRanges;
                    else
                        maClipPoly = utils::solvePolygonOperationOr(
                            maClipPoly,
                            aCollectedRanges);
                    break;
                case INTERSECT:
                    assert( !bIsEmpty );
                    // This intersect of multiple ranges was highly dubious implemented
                    // using the older stripDispensablePolygons(..., true) call. When e.g.
                    // executing the test case "intersect" from basegfx/test/clipstate.cxx
                    // (part of CppunitTest_basegfx), five ranges are given (see
                    // construction of aIntersect in setUp()). The last four do
                    // intersect with the first, but not with each other, thus
                    // a 'real' intersect of all five would be empty - there is no area that
                    // resides completely inside all of them.
                    // This shows that the stacking and one-level execution of INTERSECT
                    // here is a non-working concept with the used polygon topology. If
                    // the four non-intersecting polygons are intended to form a single
                    // PolyPolygon, it would have to be expressed exactly by using such a
                    // construct - or in case of Ranges something like a 'PolyRange'.
                    // The call to maPendingRanges.solveCrossovers solves the crossovers,
                    // so there are four snippets that are inside a fifth surrounding these
                    // when we stay at the given example.
                    // What was executed here before is to keep all polygons that are inside
                    // of another one. No holes here, can be emulated by keeping all part-polygons
                    // that lie inside another one.
                    // To do that, two loops over aCollectedRanges are needed and all which
                    // lie inside another one would be part of the result.
                    // A 'real' intersect would need to start with the 1st part-polygon, intersect
                    // with the second and so forth where the order of partial polygons would be
                    // irrelevant. In the example from the test the result would
                    // obviously be empty - what is not expected by the test.
                    // What seems to have been intended is to take the 1st part-polygon, merge the others
                    // topologically to a single PolyPoylgon and intersect that two.
                    // This is nearly impossible to do when using the result of
                    // maPendingRanges.solveCrossovers, the order is not preserved and thus
                    // unknown which was the '1st' part-polygon originally.
                    // If this strange behaviour is intended it would be easiest to just use
                    // maPendingRanges as Polygons, take the 1st, merge the rest and execute a
                    // simple utils::solvePolygonOperationAnd with these.
                    // To be on the safe side, I will keep the more expensive and complicated
                    // solution with detecting 'isInside' for now - that keeps the UnitTest working.
                    aCollectedRanges = maPendingRanges.solveCrossovers();

                    if(aCollectedRanges.count() > 1)
                    {
                        B2DPolyPolygon aResult;

                        for(sal_uInt32 a(0); a < aCollectedRanges.count(); a++)
                        {
                            const B2DPolygon aCandidate(aCollectedRanges.getB2DPolygon(a));

                            for(sal_uInt32 b(0); b < aCollectedRanges.count(); b++)
                            {
                                if(b != a)
                                {
                                    const B2DPolygon aContainer(aCollectedRanges.getB2DPolygon(b));

                                    if(utils::isInside(aContainer, aCandidate, true))
                                    {
                                        aResult.append(aCandidate);
                                        break;
                                    }
                                }
                            }
                        }

                        aCollectedRanges = aResult;
                    }

                    if( bIsCleared )
                        maClipPoly = aCollectedRanges;
                    else
                        maClipPoly = utils::solvePolygonOperationAnd(
                            maClipPoly,
                            aCollectedRanges);
                    break;
                case XOR:
                    aCollectedRanges = maPendingRanges.solveCrossovers();
                    aCollectedRanges = stripNeutralPolygons(aCollectedRanges);
                    aCollectedRanges = correctOrientations(aCollectedRanges);

                    if( bIsEmpty )
                        maClipPoly = aCollectedRanges;
                    else if( bIsCleared )
                    {
                        // not representable, strictly speaking,
                        // using polygons with the common even/odd
                        // or nonzero winding number fill rule. If
                        // we'd want to represent it, fill rule
                        // would need to be "non-negative winding
                        // number" (and we then would return
                        // 'holes' here)

                        // going for an ugly hack meanwhile
                        maClipPoly = utils::solvePolygonOperationXor(
                            B2DPolyPolygon(
                                utils::createPolygonFromRect(B2DRange(-1E20,-1E20,1E20,1E20))),
                            aCollectedRanges);
                    }
                    else
                        maClipPoly = utils::solvePolygonOperationXor(
                            maClipPoly,
                            aCollectedRanges);
                    break;
                case SUBTRACT:
                    assert( !bIsEmpty );

                    // first union all pending ranges, subtract en bloc then
                    aCollectedRanges = utils::solvePolygonOperationOr(
                        utils::packInB2DPolyPolygonVector(maPendingRanges.solveCrossovers()));

                    if( bIsCleared )
                    {
                        // not representable, strictly speaking,
                        // using polygons with the common even/odd
                        // or nonzero winding number fill rule. If
                        // we'd want to represent it, fill rule
                        // would need to be "non-negative winding
                        // number" (and we then would return
                        // 'holes' here)

                        // going for an ugly hack meanwhile
                        maClipPoly = utils::solvePolygonOperationDiff(
                            B2DPolyPolygon(
                                utils::createPolygonFromRect(B2DRange(-1E20,-1E20,1E20,1E20))),
                            aCollectedRanges);
                    }
                    else
                        maClipPoly = utils::solvePolygonOperationDiff(
                            maClipPoly,
                            aCollectedRanges);
                    break;
            }

            maPendingRanges.clear();
            mePendingOps = UNION;
        }

        mutable B2DPolyPolygon maPendingPolygons;
        mutable B2DPolyRange   maPendingRanges;
        mutable B2DPolyPolygon maClipPoly;
        mutable Operation      mePendingOps;
    };

    B2DClipState::B2DClipState() = default;

    B2DClipState::~B2DClipState() = default;

    B2DClipState::B2DClipState( const B2DClipState& ) = default;

    B2DClipState::B2DClipState( B2DClipState&& ) = default;

    B2DClipState::B2DClipState( const B2DPolyPolygon& rPolyPoly ) :
        mpImpl( ImplB2DClipState(rPolyPoly) )
    {}

    B2DClipState& B2DClipState::operator=( const B2DClipState& ) = default;

    B2DClipState& B2DClipState::operator=( B2DClipState&& ) = default;

    void B2DClipState::makeNull()
    {
        mpImpl->makeNull();
    }

    bool B2DClipState::isCleared() const
    {
        return mpImpl->isCleared();
    }

    bool B2DClipState::operator==(const B2DClipState& rRHS) const
    {
        if(mpImpl.same_object(rRHS.mpImpl))
            return true;

        return ((*mpImpl) == (*rRHS.mpImpl));
    }

    bool B2DClipState::operator!=(const B2DClipState& rRHS) const
    {
        return !(*this == rRHS);
    }

    void B2DClipState::unionRange(const B2DRange& rRange)
    {
        mpImpl->unionRange(rRange);
    }

    void B2DClipState::unionPolyPolygon(const B2DPolyPolygon& rPolyPoly)
    {
        mpImpl->unionPolyPolygon(rPolyPoly);
    }

    void B2DClipState::intersectRange(const B2DRange& rRange)
    {
        mpImpl->intersectRange(rRange);
    }

    void B2DClipState::intersectPolyPolygon(const B2DPolyPolygon& rPolyPoly)
    {
        mpImpl->intersectPolyPolygon(rPolyPoly);
    }

    void B2DClipState::subtractRange(const B2DRange& rRange)
    {
        mpImpl->subtractRange(rRange);
    }

    void B2DClipState::subtractPolyPolygon(const B2DPolyPolygon& rPolyPoly)
    {
        mpImpl->subtractPolyPolygon(rPolyPoly);
    }

    void B2DClipState::xorRange(const B2DRange& rRange)
    {
        mpImpl->xorRange(rRange);
    }

    void B2DClipState::xorPolyPolygon(const B2DPolyPolygon& rPolyPoly)
    {
        mpImpl->xorPolyPolygon(rPolyPoly);
    }

    B2DPolyPolygon const & B2DClipState::getClipPoly() const
    {
        return mpImpl->getClipPoly();
    }

    void B2DClipState::transform(const basegfx::B2DHomMatrix& rTranslate)
    {
        return mpImpl->transform(rTranslate);
    }


} // end of namespace utils
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
