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

#include <osl/diagnose.h>

#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drangeclipper.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drectangle.hxx>

#include <o3tl/vector_pool.hxx>

#include <algorithm>
#include <list>
#include <iterator>

namespace basegfx
{
    namespace
    {
        // Generating a poly-polygon from a bunch of rectangles

        // Helper functionality for sweep-line algorithm
        // ====================================================

        class ImplPolygon;
        typedef o3tl::vector_pool<ImplPolygon> VectorOfPolygons;

        /** This class represents an active edge

            As the sweep line traverses across the overall area,
            rectangle edges parallel to it generate events, and
            rectangle edges orthogonal to it generate active
            edges. This class represents the latter.
         */
        class ActiveEdge
        {
        public:

            enum EdgeDirection {
                /// edge proceeds to the left
                PROCEED_LEFT=0,
                /// edge proceeds to the right
                PROCEED_RIGHT=1
            };

            /** Create active edge

                @param rRect
                Rectangle this edge is part of

                @param fInvariantCoord
                The invariant coordinate value of this edge

                @param eEdgeType
                Is fInvariantCoord the lower or the higher value, for
                this rect?
             */
            ActiveEdge( const B2DRectangle& rRect,
                        const double&       fInvariantCoord,
                        std::ptrdiff_t      nPolyIdx,
                        EdgeDirection       eEdgeDirection ) :
                mfInvariantCoord(fInvariantCoord),
                mpAssociatedRect( &rRect ),
                mnPolygonIdx( nPolyIdx ),
                meEdgeDirection( eEdgeDirection )
            {}

            double              getInvariantCoord() const { return mfInvariantCoord; }
            const B2DRectangle& getRect() const { return *mpAssociatedRect; }
            std::ptrdiff_t      getTargetPolygonIndex() const { return mnPolygonIdx; }
            void                setTargetPolygonIndex( std::ptrdiff_t nIdx ) { mnPolygonIdx = nIdx; }
            EdgeDirection       getEdgeDirection() const { return meEdgeDirection; }

        private:
            /** The invariant coordinate value of this edge (e.g. the
                common y value, for a horizontal edge)
             */
            double              mfInvariantCoord;

            /** Associated rectangle

                This on the one hand saves some storage space (the
                vector of rectangles is persistent, anyway), and on
                the other hand provides an identifier to match active
                edges and x events (see below)

                Ptr because class needs to be assignable
             */
            const B2DRectangle* mpAssociatedRect;

            /** Index of the polygon this edge is currently involved
                with.

                Note that this can change for some kinds of edge
                intersection, as the algorithm tends to swap
                associated polygons there.

                -1 denotes no assigned polygon
             */
            std::ptrdiff_t      mnPolygonIdx;

            /// 'left' or 'right'
            EdgeDirection       meEdgeDirection;
        };

        // Needs to be list - various places hold ptrs to elements
        typedef std::list< ActiveEdge > ListOfEdges;

        /** Element of the sweep line event list

            As the sweep line traverses across the overall area,
            rectangle edges parallel to it generate events, and
            rectangle edges orthogonal to it generate active
            edges. This class represents the former.

            The class defines an element of the sweep line list. The
            sweep line's position jumps in steps defined by the
            coordinates of the sorted SweepLineEvent entries.
         */
        class SweepLineEvent
        {
        public:
            /** The two possible sweep line rectangle edges differ by
                one coordinate value - the starting edge has the
                lower, the finishing edge the higher value.
             */
            enum EdgeType {
                /// edge with lower coordinate value
                STARTING_EDGE=0,
                /// edge with higher coordinate value
                FINISHING_EDGE=1
            };

            /** The two possible sweep line directions
             */
            enum EdgeDirection {
                PROCEED_UP=0,
                PROCEED_DOWN=1
            };

            /** Create sweep line event

                @param fPos
                Coordinate position of the event

                @param rRect
                Rectangle this event is generated for.

                @param eEdgeType
                Is fPos the lower or the higher value, for the
                rectangle this event is generated for?
             */
            SweepLineEvent( double              fPos,
                            const B2DRectangle& rRect,
                            EdgeType            eEdgeType,
                            EdgeDirection       eDirection) :
                mfPos( fPos ),
                mpAssociatedRect( &rRect ),
                meEdgeType( eEdgeType ),
                meEdgeDirection( eDirection )
            {}

            double              getPos() const { return mfPos; }
            const B2DRectangle& getRect() const { return *mpAssociatedRect; }
            EdgeType            getEdgeType() const { return meEdgeType; }
            EdgeDirection       getEdgeDirection() const { return meEdgeDirection; }

            /// For STL sort
            bool operator<( const SweepLineEvent& rRHS ) const { return mfPos < rRHS.mfPos; }

        private:
            /// position of the event, in the direction of the line sweep
            double                mfPos;

            /** Rectangle this event is generated for

                This on the one hand saves some storage space (the
                vector of rectangles is persistent, anyway), and on
                the other hand provides an identifier to match active
                edges and events (see below)

                Ptr because class needs to be assignable
             */
            const B2DRectangle*   mpAssociatedRect;

            /// 'upper' or 'lower' edge of original rectangle.
            EdgeType              meEdgeType;

            /// 'up' or 'down'
            EdgeDirection         meEdgeDirection;
        };

        typedef std::vector< SweepLineEvent > VectorOfEvents;

        /** Smart point container for B2DMultiRange::getPolyPolygon()

            This class provides methods needed only here, and is used
            as a place to store some additional information per
            polygon. Also, most of the intersection logic is
            implemented here.
         */
        class ImplPolygon
        {
        public:
            /** Create polygon
             */
            ImplPolygon() :
                mpLeadingRightEdge(nullptr),
                mnIdx(-1),
                maPoints(),
                mbIsFinished(false)
            {
                // completely ad-hoc. but what the hell.
                maPoints.reserve(11);
            }

            void setPolygonPoolIndex( std::ptrdiff_t nIdx ) { mnIdx = nIdx; }

            /// Add point to the end of the existing points
            void append( const B2DPoint& rPoint )
            {
                OSL_PRECOND( maPoints.empty() ||
                             maPoints.back().getX() == rPoint.getX() ||
                             maPoints.back().getY() == rPoint.getY(),
                             "ImplPolygon::append(): added point violates 90 degree line angle constraint!" );

                if( maPoints.empty() ||
                    maPoints.back() != rPoint )
                {
                    // avoid duplicate points
                    maPoints.push_back( rPoint );
                }
            }

            /** Perform the intersection of this polygon with an
                active edge.

                @param rEvent
                The vertical line event that generated the
                intersection

                @param rActiveEdge
                The active edge that generated the intersection

                @param rPolygonPool
                Polygon pool, we sometimes need to allocate a new one

                @param bIsFinishingEdge
                True, when this is hitting the last edge of the
                vertical sweep - every vertical sweep starts and ends
                with upper and lower edge of the _same_ rectangle.

                @return the new current polygon (that's the one
                processing must proceed with, when going through the
                list of upcoming active edges).
             */
            std::ptrdiff_t intersect( SweepLineEvent const & rEvent,
                                      ActiveEdge&            rActiveEdge,
                                      VectorOfPolygons&      rPolygonPool,
                                      B2DPolyPolygon&        rRes,
                                      bool                   isFinishingEdge )
            {
                OSL_PRECOND( !mbIsFinished,
                             "ImplPolygon::intersect(): called on already finished polygon!" );
                OSL_PRECOND( !isFinishingEdge || &rEvent.getRect() == &rActiveEdge.getRect(),
                             "ImplPolygon::intersect(): inconsistent ending!" );

                const B2DPoint aIntersectionPoint( rEvent.getPos(),
                                                   rActiveEdge.getInvariantCoord() );

                // intersection point, goes to our polygon
                // unconditionally
                append(aIntersectionPoint);

                if( isFinishingEdge )
                {
                    // isSweepLineEnteringRect ?
                    if( rEvent.getEdgeType() == SweepLineEvent::STARTING_EDGE)
                        handleFinalOwnRightEdge(rActiveEdge);
                    else
                        handleFinalOwnLeftEdge(rActiveEdge,
                                               rPolygonPool,
                                               rRes);

                    // we're done with this rect & sweep line
                    return -1;
                }
                else if( metOwnEdge(rEvent,rActiveEdge) )
                {
                    handleInitialOwnEdge(rEvent, rActiveEdge);

                    // point already added, all init done, continue
                    // with same poly
                    return mnIdx;
                }
                else
                {
                    OSL_ENSURE( rActiveEdge.getTargetPolygonIndex() != -1,
                                "ImplPolygon::intersect(): non-trivial intersection hit empty polygon!" );

                    const bool isHittingLeftEdge(
                        rActiveEdge.getEdgeDirection() == ActiveEdge::PROCEED_LEFT);

                    if( isHittingLeftEdge )
                        return handleComplexLeftEdge(rActiveEdge,
                                                     aIntersectionPoint,
                                                     rPolygonPool,
                                                     rRes);
                    else
                        return handleComplexRightEdge(rActiveEdge,
                                                      aIntersectionPoint,
                                                      rPolygonPool);
                }
            }

        private:
            void handleInitialOwnEdge(SweepLineEvent const & rEvent,
                                      ActiveEdge&            rActiveEdge) const
            {
                const bool isActiveEdgeProceedLeft(
                    rActiveEdge.getEdgeDirection() == ActiveEdge::PROCEED_LEFT);
                const bool isSweepLineEnteringRect(
                    rEvent.getEdgeType() == SweepLineEvent::STARTING_EDGE);

                OSL_ENSURE( isSweepLineEnteringRect == isActiveEdgeProceedLeft,
                            "ImplPolygon::intersect(): sweep initial own edge hit: wrong polygon order" );

                OSL_ENSURE( isSweepLineEnteringRect ||
                            mpLeadingRightEdge == &rActiveEdge,
                            "ImplPolygon::intersect(): sweep initial own edge hit: wrong leading edge" );
            }

            void handleFinalOwnRightEdge(ActiveEdge& rActiveEdge)
            {
                OSL_ENSURE( rActiveEdge.getEdgeDirection() == ActiveEdge::PROCEED_RIGHT,
                            "ImplPolygon::handleInitialOwnRightEdge(): start edge wrong polygon order" );

                rActiveEdge.setTargetPolygonIndex(mnIdx);
                mpLeadingRightEdge = &rActiveEdge;
            }

            void handleFinalOwnLeftEdge(ActiveEdge const & rActiveEdge,
                                        VectorOfPolygons&  rPolygonPool,
                                        B2DPolyPolygon&    rRes)
            {
                OSL_ENSURE( rActiveEdge.getEdgeDirection() == ActiveEdge::PROCEED_LEFT,
                            "ImplPolygon::handleFinalOwnLeftEdge(): end edge wrong polygon order" );

                const bool isHittingOurTail(
                    rActiveEdge.getTargetPolygonIndex() == mnIdx);

                if( isHittingOurTail )
                    finish(rRes); // just finish. no fuss.
                else
                {
                    // temp poly hits final left edge
                    const std::ptrdiff_t nTmpIdx=rActiveEdge.getTargetPolygonIndex();
                    ImplPolygon& rTmp=rPolygonPool.get(nTmpIdx);

                    // active edge's polygon has points
                    // already. ours need to go in front of them.
                    maPoints.insert(maPoints.end(),
                                    rTmp.maPoints.begin(),
                                    rTmp.maPoints.end());

                    // adjust leading edges, we're switching the polygon
                    ActiveEdge* const pFarEdge=rTmp.mpLeadingRightEdge;

                    mpLeadingRightEdge = pFarEdge;
                    pFarEdge->setTargetPolygonIndex(mnIdx);

                    // nTmpIdx is an empty shell, get rid of it
                    rPolygonPool.free(nTmpIdx);
                }
            }

            std::ptrdiff_t handleComplexLeftEdge(ActiveEdge&       rActiveEdge,
                                                 const B2DPoint&   rIntersectionPoint,
                                                 VectorOfPolygons& rPolygonPool,
                                                 B2DPolyPolygon&   rRes)
            {
                const bool isHittingOurTail(
                    rActiveEdge.getTargetPolygonIndex() == mnIdx);
                if( isHittingOurTail )
                {
                    finish(rRes);

                    // so "this" is done - need new polygon to collect
                    // further points
                    const std::ptrdiff_t nIdxNewPolygon=rPolygonPool.alloc();
                    rPolygonPool.get(nIdxNewPolygon).setPolygonPoolIndex(nIdxNewPolygon);
                    rPolygonPool.get(nIdxNewPolygon).append(rIntersectionPoint);

                    rActiveEdge.setTargetPolygonIndex(nIdxNewPolygon);

                    return nIdxNewPolygon;
                }
                else
                {
                    const std::ptrdiff_t nTmpIdx=rActiveEdge.getTargetPolygonIndex();
                    ImplPolygon& rTmp=rPolygonPool.get(nTmpIdx);

                    // active edge's polygon has points
                    // already. ours need to go in front of them.
                    maPoints.insert(maPoints.end(),
                                    rTmp.maPoints.begin(),
                                    rTmp.maPoints.end());

                    rTmp.maPoints.clear();
                    rTmp.append(rIntersectionPoint);

                    // adjust leading edges, we're switching the polygon
                    ActiveEdge* const pFarEdge=rTmp.mpLeadingRightEdge;
                    ActiveEdge* const pNearEdge=&rActiveEdge;

                    rTmp.mpLeadingRightEdge = nullptr;
                    pNearEdge->setTargetPolygonIndex(nTmpIdx);

                    mpLeadingRightEdge = pFarEdge;
                    pFarEdge->setTargetPolygonIndex(mnIdx);

                    return nTmpIdx;
                }
            }

            std::ptrdiff_t handleComplexRightEdge(ActiveEdge&       rActiveEdge,
                                                  const B2DPoint&   rIntersectionPoint,
                                                  VectorOfPolygons& rPolygonPool)
            {
                const std::ptrdiff_t nTmpIdx=rActiveEdge.getTargetPolygonIndex();
                ImplPolygon& rTmp=rPolygonPool.get(nTmpIdx);

                rTmp.append(rIntersectionPoint);

                rActiveEdge.setTargetPolygonIndex(mnIdx);
                mpLeadingRightEdge = &rActiveEdge;

                rTmp.mpLeadingRightEdge = nullptr;

                return nTmpIdx;
            }

            /// True when sweep line hits our own active edge
            static bool metOwnEdge(SweepLineEvent const & rEvent,
                                   ActiveEdge const &     rActiveEdge)
            {
                const bool bHitOwnEdge=&rEvent.getRect() == &rActiveEdge.getRect();
                return bHitOwnEdge;
            }

            /// Retrieve B2DPolygon from this object
            B2DPolygon getPolygon() const
            {
                B2DPolygon aRes;
                for (auto const& aPoint : maPoints)
                    aRes.append(aPoint, 1);
                aRes.setClosed( true );
                return aRes;
            }

            /** Finish this polygon, push to result set.
             */
            void finish(B2DPolyPolygon& rRes)
            {
                OSL_PRECOND( maPoints.empty() ||
                             maPoints.front().getX() == maPoints.back().getX() ||
                             maPoints.front().getY() == maPoints.back().getY(),
                             "ImplPolygon::finish(): first and last point violate 90 degree line angle constraint!" );

                mbIsFinished = true;
                mpLeadingRightEdge = nullptr;

                rRes.append(getPolygon());
            }

            /** Refers to the current leading edge element of this
                polygon, or NULL. The leading edge denotes the 'front'
                of the polygon vertex sequence, i.e. the coordinates
                at the polygon's leading edge are returned from
                maPoints.front()
             */
            ActiveEdge*           mpLeadingRightEdge;

            /// current index into vector pool
            std::ptrdiff_t        mnIdx;

            /// Container for the actual polygon points
            std::vector<B2DPoint> maPoints;

            /// When true, this polygon is 'done', i.e. nothing must be added anymore.
            bool                  mbIsFinished;
        };

        /** Init sweep line event list

            This method fills the event list with the sweep line
            events generated from the input rectangles, and sorts them
            with increasing x.
         */
        void setupSweepLineEventListFromRanges( VectorOfEvents& o_rEventVector,
                                                const std::vector<B2DRange>& rRanges,
                                                const std::vector<B2VectorOrientation>& rOrientations )
        {
            // we need exactly 2*rectVec.size() events: one for the
            // left, and one for the right edge of each rectangle
            o_rEventVector.clear();
            o_rEventVector.reserve( 2*rRanges.size() );

            // generate events
            // ===============

            // first pass: add all left edges in increasing order
            std::vector<B2DRange>::const_iterator aCurrRect=rRanges.begin();
            std::vector<B2VectorOrientation>::const_iterator aCurrOrientation=rOrientations.begin();
            const std::vector<B2DRange>::const_iterator aEnd=rRanges.end();
            const std::vector<B2VectorOrientation>::const_iterator aEndOrientation=rOrientations.end();
            while( aCurrRect != aEnd && aCurrOrientation != aEndOrientation )
            {
                const B2DRectangle& rCurrRect( *aCurrRect++ );

                o_rEventVector.emplace_back( rCurrRect.getMinX(),
                                    rCurrRect,
                                    SweepLineEvent::STARTING_EDGE,
                                    (*aCurrOrientation++) == B2VectorOrientation::Positive ?
                                    SweepLineEvent::PROCEED_UP : SweepLineEvent::PROCEED_DOWN );
            }

            // second pass: add all right edges in reversed order
            std::vector<B2DRange>::const_reverse_iterator aCurrRectR=rRanges.rbegin();
            std::vector<B2VectorOrientation>::const_reverse_iterator aCurrOrientationR=rOrientations.rbegin();
            const std::vector<B2DRange>::const_reverse_iterator aEndR=rRanges.rend();
            while( aCurrRectR != aEndR )
            {
                const B2DRectangle& rCurrRect( *aCurrRectR++ );

                o_rEventVector.emplace_back( rCurrRect.getMaxX(),
                                    rCurrRect,
                                    SweepLineEvent::FINISHING_EDGE,
                                    (*aCurrOrientationR++) == B2VectorOrientation::Positive ?
                                    SweepLineEvent::PROCEED_DOWN : SweepLineEvent::PROCEED_UP );
            }

            // sort events
            // ===========

            // since we use stable_sort, the order of events with the
            // same x value will not change. The elaborate two-pass
            // add above thus ensures, that for each two rectangles
            // with similar left and right x coordinates, the
            // rectangle whose left event comes first will have its
            // right event come last. This is advantageous for the
            // clip algorithm below, see handleRightEdgeCrossing().

            std::stable_sort( o_rEventVector.begin(),
                              o_rEventVector.end() );
        }

        /** Insert two active edge segments for the given rectangle.

            This method creates two active edge segments from the
            given rect, and inserts them into the active edge list,
            such that this stays sorted (if it was before).

            @param io_rEdgeList
            Active edge list to insert into

            @param io_rPolygons
            Vector of polygons. Each rectangle added creates one
            tentative result polygon in this vector, and the edge list
            entries holds a reference to that polygon (this _requires_
            that the polygon vector does not reallocate, i.e. it must
            have at least the maximal number of rectangles reserved)

            @param o_CurrentPolygon
            The then-current polygon when processing this sweep line
            event

            @param rCurrEvent
            The actual event that caused this call
         */
        void createActiveEdgesFromStartEvent( ListOfEdges &          io_rEdgeList,
                                              VectorOfPolygons &     io_rPolygonPool,
                                              SweepLineEvent const & rCurrEvent )
        {
            ListOfEdges         aNewEdges;
            const B2DRectangle& rRect=rCurrEvent.getRect();
            const bool          bGoesDown=rCurrEvent.getEdgeDirection() == SweepLineEvent::PROCEED_DOWN;

            // start event - new rect starts here, needs polygon to
            // collect points into
            const std::ptrdiff_t nIdxPolygon=io_rPolygonPool.alloc();
            io_rPolygonPool.get(nIdxPolygon).setPolygonPoolIndex(nIdxPolygon);

            // upper edge
            aNewEdges.emplace_back(
                    rRect,
                    rRect.getMinY(),
                    bGoesDown ? nIdxPolygon : -1,
                    bGoesDown ? ActiveEdge::PROCEED_LEFT : ActiveEdge::PROCEED_RIGHT );
            // lower edge
            aNewEdges.emplace_back(
                    rRect,
                    rRect.getMaxY(),
                    bGoesDown ? -1 : nIdxPolygon,
                    bGoesDown ? ActiveEdge::PROCEED_RIGHT : ActiveEdge::PROCEED_LEFT );

            // furthermore, have to respect a special tie-breaking
            // rule here, for edges which share the same y value:
            // newly added upper edges must be inserted _before_ any
            // other edge with the same y value, and newly added lower
            // edges must be _after_ all other edges with the same
            // y. This ensures that the left vertical edge processing
            // below encounters the upper edge of the current rect
            // first, and the lower edge last, which automatically
            // starts and finishes this rect correctly (as only then,
            // the polygon will have their associated active edges
            // set).
            const double                nMinY( rRect.getMinY() );
            const double                nMaxY( rRect.getMaxY() );
            ListOfEdges::iterator       aCurr( io_rEdgeList.begin() );
            const ListOfEdges::iterator aEnd ( io_rEdgeList.end() );
            while( aCurr != aEnd )
            {
                const double nCurrY( aCurr->getInvariantCoord() );

                if( nCurrY >= nMinY &&
                    aNewEdges.size() == 2 ) // only add, if not yet done.
                {
                    // insert upper edge _before_ aCurr. Thus, it will
                    // be the first entry for a range of equal y
                    // values. Using splice here, since we hold
                    // references to the moved list element!
                    io_rEdgeList.splice( aCurr,
                                         aNewEdges,
                                         aNewEdges.begin() );
                }

                if( nCurrY > nMaxY )
                {
                    // insert lower edge _before_ aCurr. Thus, it will
                    // be the last entry for a range of equal y values
                    // (aCurr is the first entry strictly larger than
                    // nMaxY). Using splice here, since we hold
                    // references to the moved list element!
                    io_rEdgeList.splice( aCurr,
                                         aNewEdges,
                                         aNewEdges.begin() );
                    // done with insertion, can early-exit here.
                    return;
                }

                ++aCurr;
            }

            // append remainder of aNewList (might still contain 2 or
            // 1 elements, depending of the contents of io_rEdgeList).
            io_rEdgeList.splice( aCurr,
                                 aNewEdges );
        }

        bool isSameRect(ActiveEdge const &        rEdge,
                               basegfx::B2DRange const & rRect)
        {
            return &rEdge.getRect() == &rRect;
        }

        // wow what a hack. necessary because stl's list::erase does
        // not eat reverse_iterator
        template<typename Cont, typename Iter> Iter eraseFromList(Cont&, const Iter&);
        template<> ListOfEdges::iterator eraseFromList(
            ListOfEdges& rList, const ListOfEdges::iterator& aIter)
        {
            return rList.erase(aIter);
        }
        template<> ListOfEdges::reverse_iterator eraseFromList(
            ListOfEdges& rList, const ListOfEdges::reverse_iterator& aIter)
        {
            return ListOfEdges::reverse_iterator(
                    rList.erase(std::prev(aIter.base())));
        }

        template<int bPerformErase,
                 typename Iterator> void processActiveEdges(
            Iterator          first,
            Iterator          last,
            ListOfEdges&      rActiveEdgeList,
            SweepLineEvent const & rCurrEvent,
            VectorOfPolygons& rPolygonPool,
            B2DPolyPolygon&   rRes )
        {
            const basegfx::B2DRange& rCurrRect=rCurrEvent.getRect();

            // fast-forward to rCurrEvent's first active edge (holds
            // for both starting and finishing sweep line events, a
            // rect is regarded _outside_ any rects whose events have
            // started earlier
            first = std::find_if(first, last,
                                 [&rCurrRect](ActiveEdge& anEdge) { return isSameRect(anEdge, rCurrRect); });

            if(first == last)
                return;

            int nCount=0;
            std::ptrdiff_t nCurrPolyIdx=-1;
            while(first != last)
            {
                if( nCurrPolyIdx == -1 )
                    nCurrPolyIdx=first->getTargetPolygonIndex();

                OSL_ASSERT(nCurrPolyIdx != -1);

                // second encounter of my rect -> second edge
                // encountered, done
                const bool bExit=
                    nCount &&
                    isSameRect(*first,
                               rCurrRect);

                // deal with current active edge
                nCurrPolyIdx =
                    rPolygonPool.get(nCurrPolyIdx).intersect(
                        rCurrEvent,
                        *first,
                        rPolygonPool,
                        rRes,
                        bExit);

                // prune upper & lower active edges, if requested
                if( bPerformErase && (bExit || !nCount) )
                    first = eraseFromList(rActiveEdgeList,first);
                else
                    ++first;

                // delayed exit, had to prune first
                if( bExit )
                    return;

                ++nCount;
            }
        }

        template<int bPerformErase> void processActiveEdgesTopDown(
            SweepLineEvent&   rCurrEvent,
            ListOfEdges&      rActiveEdgeList,
            VectorOfPolygons& rPolygonPool,
            B2DPolyPolygon&   rRes )
        {
            processActiveEdges<bPerformErase>(
                rActiveEdgeList. begin(),
                rActiveEdgeList. end(),
                rActiveEdgeList,
                rCurrEvent,
                rPolygonPool,
                rRes);
        }

        template<int bPerformErase> void processActiveEdgesBottomUp(
            SweepLineEvent&   rCurrEvent,
            ListOfEdges&      rActiveEdgeList,
            VectorOfPolygons& rPolygonPool,
            B2DPolyPolygon&   rRes )
        {
            processActiveEdges<bPerformErase>(
                rActiveEdgeList. rbegin(),
                rActiveEdgeList. rend(),
                rActiveEdgeList,
                rCurrEvent,
                rPolygonPool,
                rRes);
        }

        enum{ NoErase=0, PerformErase=1 };

        void handleStartingEdge( SweepLineEvent&   rCurrEvent,
                                 ListOfEdges&      rActiveEdgeList,
                                 VectorOfPolygons& rPolygonPool,
                                 B2DPolyPolygon&   rRes)
        {
            // inject two new active edges for rect
            createActiveEdgesFromStartEvent( rActiveEdgeList,
                                             rPolygonPool,
                                             rCurrEvent );

            if( rCurrEvent.getEdgeDirection() == SweepLineEvent::PROCEED_DOWN )
                processActiveEdgesTopDown<NoErase>(
                    rCurrEvent, rActiveEdgeList, rPolygonPool, rRes);
            else
                processActiveEdgesBottomUp<NoErase>(
                    rCurrEvent, rActiveEdgeList, rPolygonPool, rRes);
        }

        void handleFinishingEdge( SweepLineEvent&   rCurrEvent,
                                  ListOfEdges&      rActiveEdgeList,
                                  VectorOfPolygons& rPolygonPool,
                                  B2DPolyPolygon&   rRes)
        {
            if( rCurrEvent.getEdgeDirection() == SweepLineEvent::PROCEED_DOWN )
                processActiveEdgesTopDown<PerformErase>(
                    rCurrEvent, rActiveEdgeList, rPolygonPool, rRes);
            else
                processActiveEdgesBottomUp<PerformErase>(
                    rCurrEvent, rActiveEdgeList, rPolygonPool, rRes);
        }

        void handleSweepLineEvent( SweepLineEvent&   rCurrEvent,
                                          ListOfEdges&      rActiveEdgeList,
                                          VectorOfPolygons& rPolygonPool,
                                          B2DPolyPolygon&   rRes)
        {
            if( rCurrEvent.getEdgeType() == SweepLineEvent::STARTING_EDGE )
                handleStartingEdge(rCurrEvent,rActiveEdgeList,rPolygonPool,rRes);
            else
                handleFinishingEdge(rCurrEvent,rActiveEdgeList,rPolygonPool,rRes);
        }
    }

    namespace utils
    {
        B2DPolyPolygon solveCrossovers(const std::vector<B2DRange>& rRanges,
                                       const std::vector<B2VectorOrientation>& rOrientations)
        {
            // sweep-line algorithm to generate a poly-polygon
            // from a bunch of rectangles
            // ===============================================

            // This algorithm uses the well-known sweep line
            // concept, explained in every good text book about
            // computational geometry.

            // We start with creating two structures for every
            // rectangle, one representing the left x coordinate,
            // one representing the right x coordinate (and both
            // referencing the original rect). These structs are
            // sorted with increasing x coordinates.

            // Then, we start processing the resulting list from
            // the beginning. Every entry in the list defines a
            // point in time of the line sweeping from left to
            // right across all rectangles.
            VectorOfEvents aSweepLineEvents;
            setupSweepLineEventListFromRanges( aSweepLineEvents,
                                               rRanges,
                                               rOrientations );

            B2DPolyPolygon   aRes;
            VectorOfPolygons aPolygonPool;
            ListOfEdges      aActiveEdgeList;

            // sometimes not enough, but a usable compromise
            aPolygonPool.reserve( rRanges.size() );

            for (auto& aSweepLineEvent : aSweepLineEvents)
                handleSweepLineEvent(aSweepLineEvent, aActiveEdgeList, aPolygonPool, aRes);

            return aRes;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
