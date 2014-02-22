/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <rtl/math.hxx>

#include <basegfx/tuple/b2dtuple.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2dpolyrange.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <o3tl/vector_pool.hxx>
#include <boost/bind.hpp>
#include <boost/utility.hpp>

#include <algorithm>
#include <deque>
#include <list>


namespace basegfx
{
    namespace
    {
        
        //
        
        

        typedef std::vector<B2DRange> VectorOfRanges;

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
            /** The two possible active rectangle edges differ by one
                coordinate value - the upper edge has the lower, the
                lower edge the higher value.
             */
            enum EdgeType {
                
                UPPER=0,
                
                LOWER=1
            };

            enum EdgeDirection {
                
                PROCEED_LEFT=0,
                
                PROCEED_RIGHT=1
            };

            /** Create active edge

                @param rRect
                Rectangle this edge is part of

                @param fInvariantCoord
                The invariant ccordinate value of this edge

                @param eEdgeType
                Is fInvariantCoord the lower or the higher value, for
                this rect?
             */
            ActiveEdge( const B2DRectangle& rRect,
                        const double&       fInvariantCoord,
                        std::ptrdiff_t      nPolyIdx,
                        EdgeType            eEdgeType,
                        EdgeDirection       eEdgeDirection ) :
                mfInvariantCoord(fInvariantCoord),
                mpAssociatedRect( &rRect ),
                mnPolygonIdx( nPolyIdx ),
                meEdgeType( eEdgeType ),
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

            
            EdgeType            meEdgeType;

            
            EdgeDirection       meEdgeDirection;
        };

        
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
                
                STARTING_EDGE=0,
                
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

            
            bool operator<( const SweepLineEvent& rRHS ) const { return mfPos < rRHS.mfPos; }

        private:
            
            double                mfPos;

            /** Rectangle this event is generated for

                This on the one hand saves some storage space (the
                vector of rectangles is persistent, anyway), and on
                the other hand provides an identifier to match active
                edges and events (see below)

                Ptr because class needs to be assignable
             */
            const B2DRectangle*   mpAssociatedRect;

            
            EdgeType              meEdgeType;

            
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
                mpLeadingRightEdge(NULL),
                mnIdx(-1),
                maPoints(),
                mbIsFinished(false)
            {
                
                maPoints.reserve(11);
            }

            void setPolygonPoolIndex( std::ptrdiff_t nIdx ) { mnIdx = nIdx; }
            bool isFinished() const { return mbIsFinished; }

            
            void append( const B2DPoint& rPoint )
            {
                OSL_PRECOND( maPoints.empty() ||
                             maPoints.back().getX() == rPoint.getX() ||
                             maPoints.back().getY() == rPoint.getY(),
                             "ImplPolygon::append(): added point violates 90 degree line angle constraint!" );

                if( maPoints.empty() ||
                    maPoints.back() != rPoint )
                {
                    
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
            std::ptrdiff_t intersect( SweepLineEvent&   rEvent,
                                      ActiveEdge&       rActiveEdge,
                                      VectorOfPolygons& rPolygonPool,
                                      B2DPolyPolygon&   rRes,
                                      bool              isFinishingEdge )
            {
                OSL_PRECOND( !isFinished(),
                             "ImplPolygon::intersect(): called on already finished polygon!" );
                OSL_PRECOND( !isFinishingEdge
                             || (isFinishingEdge && &rEvent.getRect() == &rActiveEdge.getRect()),
                             "ImplPolygon::intersect(): inconsistent ending!" );

                const B2DPoint aIntersectionPoint( rEvent.getPos(),
                                                   rActiveEdge.getInvariantCoord() );

                
                
                append(aIntersectionPoint);

                if( isFinishingEdge )
                {
                    
                    if( rEvent.getEdgeType() == SweepLineEvent::STARTING_EDGE)
                        handleFinalOwnRightEdge(rActiveEdge);
                    else
                        handleFinalOwnLeftEdge(rActiveEdge,
                                               rPolygonPool,
                                               rRes);

                    
                    return -1;
                }
                else if( metOwnEdge(rEvent,rActiveEdge) )
                {
                    handleInitialOwnEdge(rEvent, rActiveEdge);

                    
                    
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
            void handleInitialOwnEdge(SweepLineEvent& rEvent,
                                      ActiveEdge&     rActiveEdge)
            {
                const bool isActiveEdgeProceedLeft(
                    rActiveEdge.getEdgeDirection() == ActiveEdge::PROCEED_LEFT);
                const bool isSweepLineEnteringRect(
                    rEvent.getEdgeType() == SweepLineEvent::STARTING_EDGE);
                (void)isActiveEdgeProceedLeft;
                (void)isSweepLineEnteringRect;

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

            void handleFinalOwnLeftEdge(ActiveEdge&       rActiveEdge,
                                        VectorOfPolygons& rPolygonPool,
                                        B2DPolyPolygon&   rRes)
            {
                OSL_ENSURE( rActiveEdge.getEdgeDirection() == ActiveEdge::PROCEED_LEFT,
                            "ImplPolygon::handleFinalOwnLeftEdge(): end edge wrong polygon order" );

                const bool isHittingOurTail(
                    rActiveEdge.getTargetPolygonIndex() == mnIdx);

                if( isHittingOurTail )
                    finish(rRes); 
                else
                {
                    
                    const std::ptrdiff_t nTmpIdx=rActiveEdge.getTargetPolygonIndex();
                    ImplPolygon& rTmp=rPolygonPool.get(nTmpIdx);

                    
                    
                    maPoints.insert(maPoints.end(),
                                    rTmp.maPoints.begin(),
                                    rTmp.maPoints.end());

                    
                    ActiveEdge* const pFarEdge=rTmp.mpLeadingRightEdge;

                    mpLeadingRightEdge = pFarEdge;
                    pFarEdge->setTargetPolygonIndex(mnIdx);

                    
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

                    
                    
                    maPoints.insert(maPoints.end(),
                                    rTmp.maPoints.begin(),
                                    rTmp.maPoints.end());

                    rTmp.maPoints.clear();
                    rTmp.append(rIntersectionPoint);

                    
                    ActiveEdge* const pFarEdge=rTmp.mpLeadingRightEdge;
                    ActiveEdge* const pNearEdge=&rActiveEdge;

                    rTmp.mpLeadingRightEdge = NULL;
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

                rTmp.mpLeadingRightEdge = NULL;

                return nTmpIdx;
            }

            
            bool metOwnEdge(const SweepLineEvent& rEvent,
                            ActiveEdge&           rActiveEdge)
            {
                const bool bHitOwnEdge=&rEvent.getRect() == &rActiveEdge.getRect();
                return bHitOwnEdge;
            }

            
            B2DPolygon getPolygon() const
            {
                B2DPolygon aRes;
                std::for_each( maPoints.begin(),
                               maPoints.end(),
                               boost::bind(
                     &B2DPolygon::append,
                                   boost::ref(aRes),
                                   _1,
                                   1 ) );
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
                mpLeadingRightEdge = NULL;

                rRes.append(getPolygon());
            }

            /** Refers to the current leading edge element of this
                polygon, or NULL. The leading edge denotes the 'front'
                of the polygon vertex sequence, i.e. the coordinates
                at the polygon's leading edge are returned from
                maPoints.front()
             */
            ActiveEdge*           mpLeadingRightEdge;

            
            std::ptrdiff_t        mnIdx;

            
            std::vector<B2DPoint> maPoints;

            
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
            
            
            o_rEventVector.clear();
            o_rEventVector.reserve( 2*rRanges.size() );

            
            

            
            std::vector<B2DRange>::const_iterator aCurrRect=rRanges.begin();
            std::vector<B2VectorOrientation>::const_iterator aCurrOrientation=rOrientations.begin();
            const std::vector<B2DRange>::const_iterator aEnd=rRanges.end();
            const std::vector<B2VectorOrientation>::const_iterator aEndOrientation=rOrientations.end();
            while( aCurrRect != aEnd && aCurrOrientation != aEndOrientation )
            {
                const B2DRectangle& rCurrRect( *aCurrRect++ );

                o_rEventVector.push_back(
                    SweepLineEvent( rCurrRect.getMinX(),
                                    rCurrRect,
                                    SweepLineEvent::STARTING_EDGE,
                                    (*aCurrOrientation++) == ORIENTATION_POSITIVE ?
                                    SweepLineEvent::PROCEED_UP : SweepLineEvent::PROCEED_DOWN) );
            }

            
            std::vector<B2DRange>::const_reverse_iterator aCurrRectR=rRanges.rbegin();
            std::vector<B2VectorOrientation>::const_reverse_iterator aCurrOrientationR=rOrientations.rbegin();
            const std::vector<B2DRange>::const_reverse_iterator aEndR=rRanges.rend();
            while( aCurrRectR != aEndR )
            {
                const B2DRectangle& rCurrRect( *aCurrRectR++ );

                o_rEventVector.push_back(
                    SweepLineEvent( rCurrRect.getMaxX(),
                                    rCurrRect,
                                    SweepLineEvent::FINISHING_EDGE,
                                    (*aCurrOrientationR++) == ORIENTATION_POSITIVE ?
                                    SweepLineEvent::PROCEED_DOWN : SweepLineEvent::PROCEED_UP ) );
            }

            
            

            
            
            
            
            
            
            

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
        void createActiveEdgesFromStartEvent( ListOfEdges&      io_rEdgeList,
                                              VectorOfPolygons& io_rPolygonPool,
                                              SweepLineEvent&   rCurrEvent )
        {
            ListOfEdges         aNewEdges;
            const B2DRectangle& rRect=rCurrEvent.getRect();
            const bool          bGoesDown=rCurrEvent.getEdgeDirection() == SweepLineEvent::PROCEED_DOWN;

            
            
            const std::ptrdiff_t nIdxPolygon=io_rPolygonPool.alloc();
            io_rPolygonPool.get(nIdxPolygon).setPolygonPoolIndex(nIdxPolygon);

            
            aNewEdges.push_back(
                ActiveEdge(
                    rRect,
                    rRect.getMinY(),
                    bGoesDown ? nIdxPolygon : -1,
                    ActiveEdge::UPPER,
                    bGoesDown ? ActiveEdge::PROCEED_LEFT : ActiveEdge::PROCEED_RIGHT) );
            
            aNewEdges.push_back(
                ActiveEdge(
                    rRect,
                    rRect.getMaxY(),
                    bGoesDown ? -1 : nIdxPolygon,
                    ActiveEdge::LOWER,
                    bGoesDown ? ActiveEdge::PROCEED_RIGHT : ActiveEdge::PROCEED_LEFT ) );

            
            
            
            
            
            
            
            
            
            
            
            const double                nMinY( rRect.getMinY() );
            const double                nMaxY( rRect.getMaxY() );
            ListOfEdges::iterator       aCurr( io_rEdgeList.begin() );
            const ListOfEdges::iterator aEnd ( io_rEdgeList.end() );
            while( aCurr != aEnd )
            {
                const double nCurrY( aCurr->getInvariantCoord() );

                if( nCurrY >= nMinY &&
                    aNewEdges.size() == 2 ) 
                {
                    
                    
                    
                    
                    io_rEdgeList.splice( aCurr,
                                         aNewEdges,
                                         aNewEdges.begin() );
                }

                if( nCurrY > nMaxY )
                {
                    
                    
                    
                    
                    
                    io_rEdgeList.splice( aCurr,
                                         aNewEdges,
                                         aNewEdges.begin() );
                    
                    return;
                }

                ++aCurr;
            }

            
            
            io_rEdgeList.splice( aCurr,
                                 aNewEdges );
        }

        inline bool isSameRect(ActiveEdge&              rEdge,
                               const basegfx::B2DRange& rRect)
        {
            return &rEdge.getRect() == &rRect;
        }

        
        
        template<typename Cont, typename Iter> Iter eraseFromList(Cont&, Iter);
        template<> inline ListOfEdges::iterator eraseFromList(
            ListOfEdges& rList, ListOfEdges::iterator aIter)
        {
            return rList.erase(aIter);
        }
        template<> inline ListOfEdges::reverse_iterator eraseFromList(
            ListOfEdges& rList, ListOfEdges::reverse_iterator aIter)
        {
            return ListOfEdges::reverse_iterator(
                    rList.erase(boost::prior(aIter.base())));
        }

        template<int bPerformErase,
                 typename Iterator> inline void processActiveEdges(
            Iterator          first,
            Iterator          last,
            ListOfEdges&      rActiveEdgeList,
            SweepLineEvent&   rCurrEvent,
            VectorOfPolygons& rPolygonPool,
            B2DPolyPolygon&   rRes )
        {
            const basegfx::B2DRange& rCurrRect=rCurrEvent.getRect();

            
            
            
            
            first = std::find_if(first, last,
                                 boost::bind(
                         &isSameRect,
                                     _1,
                                     boost::cref(rCurrRect)));

            if(first == last)
                return;

            int nCount=0;
            std::ptrdiff_t nCurrPolyIdx=-1;
            while(first != last)
            {
                if( nCurrPolyIdx == -1 )
                    nCurrPolyIdx=first->getTargetPolygonIndex();

                OSL_ASSERT(nCurrPolyIdx != -1);

                
                
                const bool bExit=
                    nCount &&
                    isSameRect(*first,
                               rCurrRect);

                
                nCurrPolyIdx =
                    rPolygonPool.get(nCurrPolyIdx).intersect(
                        rCurrEvent,
                        *first,
                        rPolygonPool,
                        rRes,
                        bExit);

                
                if( bPerformErase && (bExit || !nCount) )
                    first = eraseFromList(rActiveEdgeList,first);
                else
                    ++first;

                
                if( bExit )
                    return;

                ++nCount;
            }
        }

        template<int bPerformErase> inline void processActiveEdgesTopDown(
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

        template<int bPerformErase> inline void processActiveEdgesBottomUp(
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
            
            createActiveEdgesFromStartEvent( rActiveEdgeList,
                                             rPolygonPool,
                                             rCurrEvent );

            if( SweepLineEvent::PROCEED_DOWN == rCurrEvent.getEdgeDirection() )
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
            if( SweepLineEvent::PROCEED_DOWN == rCurrEvent.getEdgeDirection() )
                processActiveEdgesTopDown<PerformErase>(
                    rCurrEvent, rActiveEdgeList, rPolygonPool, rRes);
            else
                processActiveEdgesBottomUp<PerformErase>(
                    rCurrEvent, rActiveEdgeList, rPolygonPool, rRes);
        }

        inline void handleSweepLineEvent( SweepLineEvent&   rCurrEvent,
                                          ListOfEdges&      rActiveEdgeList,
                                          VectorOfPolygons& rPolygonPool,
                                          B2DPolyPolygon&   rRes)
        {
            if( SweepLineEvent::STARTING_EDGE == rCurrEvent.getEdgeType() )
                handleStartingEdge(rCurrEvent,rActiveEdgeList,rPolygonPool,rRes);
            else
                handleFinishingEdge(rCurrEvent,rActiveEdgeList,rPolygonPool,rRes);
        }
    }

    namespace tools
    {
        B2DPolyPolygon solveCrossovers(const std::vector<B2DRange>& rRanges,
                                       const std::vector<B2VectorOrientation>& rOrientations)
        {
            
            
            
            //
            
            
            
            //
            
            
            
            
            
            //
            
            
            
            
            VectorOfEvents aSweepLineEvents;
            setupSweepLineEventListFromRanges( aSweepLineEvents,
                                               rRanges,
                                               rOrientations );

            B2DPolyPolygon   aRes;
            VectorOfPolygons aPolygonPool;
            ListOfEdges      aActiveEdgeList;

            
            aPolygonPool.reserve( rRanges.size() );

            std::for_each( aSweepLineEvents.begin(),
                           aSweepLineEvents.end(),
                           boost::bind(
                               &handleSweepLineEvent,
                               _1,
                               boost::ref(aActiveEdgeList),
                               boost::ref(aPolygonPool),
                               boost::ref(aRes)) );

            return aRes;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
