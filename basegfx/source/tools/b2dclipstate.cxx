/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basegfx.hxx"
#include <basegfx/tools/b2dclipstate.hxx>

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
namespace tools
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

        explicit ImplB2DClipState( const B2DRange& rRange ) :
            maPendingPolygons(),
            maPendingRanges(),
            maClipPoly(
                tools::createPolygonFromRect(rRange)),
            mePendingOps(UNION)
        {}

        explicit ImplB2DClipState( const B2DPolygon& rPoly ) :
            maPendingPolygons(),
            maPendingRanges(),
            maClipPoly(rPoly),
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

        void makeClear()
        {
            maPendingPolygons.clear();
            maPendingRanges.clear();
            maClipPoly.clear();
            mePendingOps = UNION;
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
                ORIENTATION_POSITIVE);
        }

        void addPolygon(B2DPolygon aPoly, Operation eOp)
        {
            commitPendingRanges();
            if( mePendingOps != eOp )
                commitPendingPolygons();

            mePendingOps = eOp;
            maPendingPolygons.append(aPoly);
        }

        void addPolyPolygon(B2DPolyPolygon aPoly, Operation eOp)
        {
            commitPendingRanges();
            if( mePendingOps != eOp )
                commitPendingPolygons();

            mePendingOps = eOp;
            maPendingPolygons.append(aPoly);
        }

        void addClipState(const ImplB2DClipState& rOther, Operation eOp)
        {
            if( rOther.mePendingOps == mePendingOps
                && !rOther.maClipPoly.count()
                && !rOther.maPendingPolygons.count() )
            {
                maPendingRanges.appendPolyRange( rOther.maPendingRanges );
            }
            else
            {
                commitPendingRanges();
                commitPendingPolygons();
                rOther.commitPendingRanges();
                rOther.commitPendingPolygons();

                maPendingPolygons = rOther.maClipPoly;
                mePendingOps = eOp;
            }
        }

        void unionRange(const B2DRange& rRange)
        {
            if( isCleared() )
                return;

            addRange(rRange,UNION);
        }

        void unionPolygon(const B2DPolygon& rPoly)
        {
            if( isCleared() )
                return;

            addPolygon(rPoly,UNION);
        }

        void unionPolyPolygon(const B2DPolyPolygon& rPolyPoly)
        {
            if( isCleared() )
                return;

            addPolyPolygon(rPolyPoly,UNION);
        }

        void unionClipState(const ImplB2DClipState& rOther)
        {
            if( isCleared() )
                return;

            addClipState(rOther, UNION);
        }

        void intersectRange(const B2DRange& rRange)
        {
            if( isNull() )
                return;

            addRange(rRange,INTERSECT);
        }

        void intersectPolygon(const B2DPolygon& rPoly)
        {
            if( isNull() )
                return;

            addPolygon(rPoly,INTERSECT);
        }

        void intersectPolyPolygon(const B2DPolyPolygon& rPolyPoly)
        {
            if( isNull() )
                return;

            addPolyPolygon(rPolyPoly,INTERSECT);
        }

        void intersectClipState(const ImplB2DClipState& rOther)
        {
            if( isNull() )
                return;

            addClipState(rOther, INTERSECT);
        }

        void subtractRange(const B2DRange& rRange )
        {
            if( isNull() )
                return;

            addRange(rRange,SUBTRACT);
        }

        void subtractPolygon(const B2DPolygon& rPoly)
        {
            if( isNull() )
                return;

            addPolygon(rPoly,SUBTRACT);
        }

        void subtractPolyPolygon(const B2DPolyPolygon& rPolyPoly)
        {
            if( isNull() )
                return;

            addPolyPolygon(rPolyPoly,SUBTRACT);
        }

        void subtractClipState(const ImplB2DClipState& rOther)
        {
            if( isNull() )
                return;

            addClipState(rOther, SUBTRACT);
        }

        void xorRange(const B2DRange& rRange)
        {
            addRange(rRange,XOR);
        }

        void xorPolygon(const B2DPolygon& rPoly)
        {
            addPolygon(rPoly,XOR);
        }

        void xorPolyPolygon(const B2DPolyPolygon& rPolyPoly)
        {
            addPolyPolygon(rPolyPoly,XOR);
        }

        void xorClipState(const ImplB2DClipState& rOther)
        {
            addClipState(rOther, XOR);
        }

        B2DPolyPolygon getClipPoly() const
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
            maPendingPolygons = tools::prepareForPolygonOperation(maPendingPolygons);
            const bool bIsEmpty=isNullClipPoly();
            const bool bIsCleared=!maClipPoly.count();
            switch(mePendingOps)
            {
                case UNION:
                    OSL_ASSERT( !bIsCleared );

                    if( bIsEmpty )
                        maClipPoly = maPendingPolygons;
                    else
                        maClipPoly = tools::solvePolygonOperationOr(
                            maClipPoly,
                            maPendingPolygons);
                    break;
                case INTERSECT:
                    OSL_ASSERT( !bIsEmpty );

                    if( bIsCleared )
                        maClipPoly = maPendingPolygons;
                    else
                        maClipPoly = tools::solvePolygonOperationAnd(
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
                        maClipPoly = tools::solvePolygonOperationXor(
                            B2DPolyPolygon(
                                tools::createPolygonFromRect(B2DRange(-1E20,-1E20,1E20,1E20))),
                            maPendingPolygons);
                    }
                    else
                        maClipPoly = tools::solvePolygonOperationXor(
                            maClipPoly,
                            maPendingPolygons);
                    break;
                case SUBTRACT:
                    OSL_ASSERT( !bIsEmpty );

                    // first union all pending ones, subtract en bloc then
                    maPendingPolygons = solveCrossovers(maPendingPolygons);
                    maPendingPolygons = stripNeutralPolygons(maPendingPolygons);
                    maPendingPolygons = stripDispensablePolygons(maPendingPolygons, false);

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
                        maClipPoly = tools::solvePolygonOperationDiff(
                            B2DPolyPolygon(
                                tools::createPolygonFromRect(B2DRange(-1E20,-1E20,1E20,1E20))),
                            maPendingPolygons);
                    }
                    else
                        maClipPoly = tools::solvePolygonOperationDiff(
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
                    OSL_ASSERT( !bIsCleared );

                    aCollectedRanges = maPendingRanges.solveCrossovers();
                    aCollectedRanges = stripNeutralPolygons(aCollectedRanges);
                    aCollectedRanges = stripDispensablePolygons(aCollectedRanges, false);
                    if( bIsEmpty )
                        maClipPoly = aCollectedRanges;
                    else
                        maClipPoly = tools::solvePolygonOperationOr(
                            maClipPoly,
                            aCollectedRanges);
                    break;
                case INTERSECT:
                    OSL_ASSERT( !bIsEmpty );

                    aCollectedRanges = maPendingRanges.solveCrossovers();
                    aCollectedRanges = stripNeutralPolygons(aCollectedRanges);
                    if( maPendingRanges.count() > 1 )
                        aCollectedRanges = stripDispensablePolygons(aCollectedRanges, true);

                    if( bIsCleared )
                        maClipPoly = aCollectedRanges;
                    else
                        maClipPoly = tools::solvePolygonOperationAnd(
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
                        maClipPoly = tools::solvePolygonOperationXor(
                            B2DPolyPolygon(
                                tools::createPolygonFromRect(B2DRange(-1E20,-1E20,1E20,1E20))),
                            aCollectedRanges);
                    }
                    else
                        maClipPoly = tools::solvePolygonOperationXor(
                            maClipPoly,
                            aCollectedRanges);
                    break;
                case SUBTRACT:
                    OSL_ASSERT( !bIsEmpty );

                    // first union all pending ranges, subtract en bloc then
                    aCollectedRanges = maPendingRanges.solveCrossovers();
                    aCollectedRanges = stripNeutralPolygons(aCollectedRanges);
                    aCollectedRanges = stripDispensablePolygons(aCollectedRanges, false);

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
                        maClipPoly = tools::solvePolygonOperationDiff(
                            B2DPolyPolygon(
                                tools::createPolygonFromRect(B2DRange(-1E20,-1E20,1E20,1E20))),
                            aCollectedRanges);
                    }
                    else
                        maClipPoly = tools::solvePolygonOperationDiff(
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

    B2DClipState::B2DClipState() :
        mpImpl()
    {}

    B2DClipState::~B2DClipState()
    {}

    B2DClipState::B2DClipState( const B2DClipState& rOrig ) :
        mpImpl(rOrig.mpImpl)
    {}

    B2DClipState::B2DClipState( const B2DRange& rRange ) :
        mpImpl( ImplB2DClipState(rRange) )
    {}

    B2DClipState::B2DClipState( const B2DPolygon& rPoly ) :
        mpImpl( ImplB2DClipState(rPoly) )
    {}

    B2DClipState::B2DClipState( const B2DPolyPolygon& rPolyPoly ) :
        mpImpl( ImplB2DClipState(rPolyPoly) )
    {}

    B2DClipState& B2DClipState::operator=( const B2DClipState& rRHS )
    {
        mpImpl = rRHS.mpImpl;
        return *this;
    }

    void B2DClipState::makeUnique()
    {
        mpImpl.make_unique();
    }

    void B2DClipState::makeNull()
    {
        mpImpl->makeNull();
    }

    bool B2DClipState::isNull() const
    {
        return mpImpl->isNull();
    }

    void B2DClipState::makeClear()
    {
        mpImpl->makeClear();
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

    void B2DClipState::unionPolygon(const B2DPolygon& rPoly)
    {
        mpImpl->unionPolygon(rPoly);
    }

    void B2DClipState::unionPolyPolygon(const B2DPolyPolygon& rPolyPoly)
    {
        mpImpl->unionPolyPolygon(rPolyPoly);
    }

    void B2DClipState::unionClipState(const B2DClipState& rState)
    {
        mpImpl->unionClipState(*rState.mpImpl);
    }

    void B2DClipState::intersectRange(const B2DRange& rRange)
    {
        mpImpl->intersectRange(rRange);
    }

    void B2DClipState::intersectPolygon(const B2DPolygon& rPoly)
    {
        mpImpl->intersectPolygon(rPoly);
    }

    void B2DClipState::intersectPolyPolygon(const B2DPolyPolygon& rPolyPoly)
    {
        mpImpl->intersectPolyPolygon(rPolyPoly);
    }

    void B2DClipState::intersectClipState(const B2DClipState& rState)
    {
        mpImpl->intersectClipState(*rState.mpImpl);
    }

    void B2DClipState::subtractRange(const B2DRange& rRange)
    {
        mpImpl->subtractRange(rRange);
    }

    void B2DClipState::subtractPolygon(const B2DPolygon& rPoly)
    {
        mpImpl->subtractPolygon(rPoly);
    }

    void B2DClipState::subtractPolyPolygon(const B2DPolyPolygon& rPolyPoly)
    {
        mpImpl->subtractPolyPolygon(rPolyPoly);
    }

    void B2DClipState::subtractClipState(const B2DClipState& rState)
    {
        mpImpl->subtractClipState(*rState.mpImpl);
    }

    void B2DClipState::xorRange(const B2DRange& rRange)
    {
        mpImpl->xorRange(rRange);
    }

    void B2DClipState::xorPolygon(const B2DPolygon& rPoly)
    {
        mpImpl->xorPolygon(rPoly);
    }

    void B2DClipState::xorPolyPolygon(const B2DPolyPolygon& rPolyPoly)
    {
        mpImpl->xorPolyPolygon(rPolyPoly);
    }

    void B2DClipState::xorClipState(const B2DClipState& rState)
    {
        mpImpl->xorClipState(*rState.mpImpl);
    }

    B2DPolyPolygon B2DClipState::getClipPoly() const
    {
        return mpImpl->getClipPoly();
    }

} // end of namespace tools
} // end of namespace basegfx

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
