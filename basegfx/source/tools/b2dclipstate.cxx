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
                    maPendingPolygons = solveCrossovers(maPendingPolygons);
                    maPendingPolygons = stripNeutralPolygons(maPendingPolygons);
                    maPendingPolygons = stripDispensablePolygons(maPendingPolygons);

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

                    aCollectedRanges = maPendingRanges.solveCrossovers();
                    aCollectedRanges = stripNeutralPolygons(aCollectedRanges);
                    aCollectedRanges = stripDispensablePolygons(aCollectedRanges);
                    if( bIsEmpty )
                        maClipPoly = aCollectedRanges;
                    else
                        maClipPoly = utils::solvePolygonOperationOr(
                            maClipPoly,
                            aCollectedRanges);
                    break;
                case INTERSECT:
                    assert( !bIsEmpty );

                    aCollectedRanges = maPendingRanges.solveCrossovers();
                    aCollectedRanges = stripNeutralPolygons(aCollectedRanges);
                    if( maPendingRanges.count() > 1 )
                        aCollectedRanges = stripDispensablePolygons(aCollectedRanges, true);

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
                    aCollectedRanges = maPendingRanges.solveCrossovers();
                    aCollectedRanges = stripNeutralPolygons(aCollectedRanges);
                    aCollectedRanges = stripDispensablePolygons(aCollectedRanges);

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
