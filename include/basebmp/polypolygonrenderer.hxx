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

#ifndef INCLUDED_BASEBMP_POLYPOLYGONRENDERER_HXX
#define INCLUDED_BASEBMP_POLYPOLYGONRENDERER_HXX

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygonfillrule.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <vigra/diff2d.hxx>
#include <vigra/iteratortraits.hxx>

#include <vector>


namespace basebmp
{
    namespace detail
    {
        /// convert int32 to 32:32 fixed point
        inline sal_Int64 toFractional( sal_Int32 v ) { return sal_Int64(sal_uInt64(v) << 32); }
        /// convert double to 32:32 fixed point
        inline sal_Int64 toFractional( double v ) { return (sal_Int64)(v*SAL_MAX_UINT32 + (v < 0.0 ? -0.5 : 0.5 )); }
        /// convert 32:32 fixed point to int32 (truncate)
        inline sal_Int32 toInteger( sal_Int64 v ) { return (sal_Int32)(v < 0 ? ~((~v) >> 32) : v >> 32); }
        /// convert 32:32 fixed point to int32 (properly rounded)
        inline sal_Int32 toRoundedInteger( sal_Int64 v ) { return toInteger(v) + (sal_Int32)((v&0x80000000) >> 31); }

        /** internal vertex store -

            Different from B2DPoint, since we don't need floating
            point coords, but orientation of vertex and y counter
         */
        struct Vertex
        {
            sal_Int32 mnYCounter;
            sal_Int64 mnX;
            sal_Int64 mnXDelta;

            bool      mbDownwards; // needed for nonzero winding rule
                                   // fills

            Vertex() :
                mnYCounter(0),
                mnX(0),
                mnXDelta(0),
                mbDownwards(true)
            {}
            Vertex( basegfx::B2DPoint const& rPt1,
                    basegfx::B2DPoint const& rPt2,
                    bool                     bDownwards ) :
                mnYCounter( basegfx::fround(rPt2.getY()) -
                            basegfx::fround(rPt1.getY()) ),
                mnX( toFractional( basegfx::fround(rPt1.getX()) )),
                mnXDelta( toFractional(
                              ((rPt2.getX() - rPt1.getX()) /
                               (double)mnYCounter) )),
                mbDownwards(bDownwards)
            {}
        };

        typedef std::vector< std::vector<Vertex> > VectorOfVectorOfVertices;
        typedef std::vector< Vertex* >             VectorOfVertexPtr;

        /// non-templated setup of GET
        sal_uInt32 setupGlobalEdgeTable( VectorOfVectorOfVertices&      rGET,
                                         basegfx::B2DPolyPolygon const& rPoly,
                                         sal_Int32                      nMinY );
        /// sort rAETSrc, copy not-yet-ended edges over to rAETDest
        void sortAET( VectorOfVertexPtr& rAETSrc,
                      VectorOfVertexPtr& rAETDest );

        /// For the STL algorithms
        struct RasterConvertVertexComparator
        {
            RasterConvertVertexComparator() {}

            bool operator()( const Vertex& rLHS,
                             const Vertex& rRHS ) const
            {
                return rLHS.mnX < rRHS.mnX;
            }

            bool operator()( const Vertex* pLHS,
                             const Vertex* pRHS ) const
            {
                return pLHS->mnX < pRHS->mnX;
            }
        };

    } // namespace detail


    /** Raster-convert a poly-polygon.

        This algorithm does not perform antialiasing, and thus
        internally works with integer vertex coordinates.

        @param begin
        Left, top edge of the destination bitmap. This position is
        considered (0,0) relative to all polygon vertices

        @param ad
        Accessor to set pixel values

        @param fillColor
        Color to use for filling

        @param rClipRect
        Clipping rectangle, relative to the begin iterator. No pixel outside
        this clip rect will be modified.

        @param rPoly
        Polygon to fill
     */
    template< class DestIterator, class DestAccessor, typename T >
    void renderClippedPolyPolygon( DestIterator                   begin,
                                   DestAccessor                   ad,
                                   T                              fillColor,
                                   const basegfx::B2IBox&       rClipRect,
                                   basegfx::B2DPolyPolygon const& rPoly,
                                   basegfx::FillRule              eFillRule )
    {
        const sal_Int32 nClipX1( std::max((sal_Int32)0,rClipRect.getMinX()) );
        const sal_Int32 nClipX2( rClipRect.getMaxX() );
        const sal_Int32 nClipY1( std::max((sal_Int32)0,rClipRect.getMinY()) );
        const sal_Int32 nClipY2( rClipRect.getMaxY() );
        const sal_Int64 nClipX1_frac( detail::toFractional(nClipX1) );
        const sal_Int64 nClipX2_frac( detail::toFractional(nClipX2) );

        basegfx::B2DRange const aPolyBounds( basegfx::tools::getRange(rPoly) );

        const sal_Int32 nMinY( basegfx::fround(aPolyBounds.getMinY()) );
        const sal_Int32 nMaxY(
            std::min(
                nClipY2-1,
                basegfx::fround(aPolyBounds.getMaxY())));

        if( nMinY > nMaxY )
            return; // really, nothing to do then.

        detail::VectorOfVectorOfVertices aGET; // the Global Edge Table
        aGET.resize( nMaxY - nMinY + 1 );

        sal_uInt32 const nVertexCount(
            detail::setupGlobalEdgeTable( aGET, rPoly, nMinY ) );


        // Perform actual scan conversion
        //----------------------------------------------------------------------

        if( aGET.empty() )
            return;

        detail::VectorOfVertexPtr      aAET1; // the Active Edge Table
        detail::VectorOfVertexPtr      aAET2;
        detail::VectorOfVertexPtr*     pAET = &aAET1;
        detail::VectorOfVertexPtr*     pAETOther = &aAET2;
        aAET1.reserve( nVertexCount );
        aAET2.reserve( nVertexCount );

        // current scanline - initially, points to first scanline
        // within the clip rect, or to the polygon's first scanline
        // (whichever is greater)
        DestIterator aScanline( begin +
                                vigra::Diff2D(
                                    0,
                                    std::max(nMinY,
                                             nClipY1)) );
        detail::RasterConvertVertexComparator aComp;


        // now process each of the nMaxY - nMinY + 1 scanlines
        // ------------------------------------------------------------

        for( sal_Int32 y=nMinY; y <= nMaxY; ++y )
        {
            if( !aGET[y-nMinY].empty() )
            {
                // merge AET with current scanline's new vertices (both
                // are already correctly sorted)
                detail::VectorOfVectorOfVertices::value_type::iterator       vertex=aGET[y-nMinY].begin();
                detail::VectorOfVectorOfVertices::value_type::iterator const end=aGET[y-nMinY].end();
                while( vertex != end )
                {
                    // find insertion pos by binary search, and put ptr
                    // into active edge vector
                    pAET->insert( std::lower_bound( pAET->begin(),
                                                    pAET->end(),
                                                    &(*vertex),
                                                    aComp ),
                                  &(*vertex) );

                    ++vertex;
                }
            }

            // with less than two active edges, no fill visible
            if( pAET->size() >= 2 )
            {
                typename vigra::IteratorTraits<DestIterator>::row_iterator
                    rowIter( aScanline.rowIterator() );

                // process each span in current scanline, with
                // even-odd fill rule
                detail::VectorOfVertexPtr::iterator       currVertex( pAET->begin() );
                detail::VectorOfVertexPtr::iterator const lastVertex( pAET->end()-1 );
                sal_uInt32                                nCrossedEdges(0);
                sal_Int32                                 nWindingNumber(0);
                while( currVertex != lastVertex )
                {
                    // TODO(P1): might be worth a try to extend the
                    // size()==2 case also to the actual filling
                    // here. YMMV.
                    detail::Vertex&       rV1( **currVertex );
                    detail::Vertex const& rV2( **++currVertex );

                    nWindingNumber += -1 + 2*rV1.mbDownwards;

                    // calc fill status for both rules. might save a
                    // few percent runtime to specialize here...
                    const bool bEvenOddFill(
                        eFillRule == basegfx::FillRule_EVEN_ODD && !(nCrossedEdges & 0x01) );
                    const bool bNonZeroWindingFill(
                        eFillRule == basegfx::FillRule_NONZERO_WINDING_NUMBER && nWindingNumber != 0 );

                    // is span visible?
                    if( (bEvenOddFill || bNonZeroWindingFill) &&
                        y >= nClipY1 &&
                        rV1.mnX < nClipX2_frac &&
                        rV2.mnX > nClipX1_frac )
                    {
                        // clip span to horizontal bounds
                        sal_Int32 const nStartX(
                            std::max( nClipX1,
                                      std::min( nClipX2-1,
                                                detail::toRoundedInteger(rV1.mnX) )));
                        sal_Int32 const nEndX(
                            std::max( nClipX1,
                                      std::min( nClipX2,
                                                detail::toRoundedInteger(rV2.mnX) )));

                        typename vigra::IteratorTraits<DestIterator>::row_iterator
                            currPix( rowIter + nStartX);
                        typename vigra::IteratorTraits<DestIterator>::row_iterator
                            rowEnd( rowIter + nEndX );

                        // TODO(P2): Provide specialized span fill methods on the
                        // iterator/accessor
                        while( currPix != rowEnd )
                            ad.set(fillColor, currPix++);
                    }

                    // step vertices
                    rV1.mnX += rV1.mnXDelta;
                    --rV1.mnYCounter;

                    ++nCrossedEdges;
                }

                // step vertex also for the last one
                detail::Vertex& rLastV( **currVertex );
                rLastV.mnX += rLastV.mnXDelta;
                --rLastV.mnYCounter;


                // prune AET from ended edges, and keep it sorted
                // ---------------------------------------------------------

                pAETOther->clear();
                if( pAET->size() == 2 )
                {
                    // the case of exactly two active edges is both
                    // sufficiently common (all 'simple' polygons have
                    // it), and further more would complicate the
                    // generic case below (which works with a sliding
                    // triple of vertices).
                    if( !aComp(*(*pAET)[0], *(*pAET)[1]) )
                        std::swap(*(*pAET)[0], *(*pAET)[1]);

                    if( (*pAET)[0]->mnYCounter > 0 )
                        pAETOther->push_back( (*pAET)[0] );
                    if( (*pAET)[1]->mnYCounter > 0 )
                        pAETOther->push_back( (*pAET)[1] );
                }
                else
                {
                    bool bFallbackTaken(false);
                    currVertex = pAET->begin();
                    detail::VectorOfVertexPtr::iterator prevVertex( currVertex );
                    while( currVertex != lastVertex )
                    {
                        // try to get away with one linear swoop and
                        // simple neighbor swapping. this is an
                        // overwhelmingly common case - polygons with
                        // excessively crisscrossing edges (which on
                        // top of that cross more than one other edge
                        // per scanline) are seldom. And even if we
                        // get such a beast here, this extra loop has
                        // still only linear cost
                        if( aComp(**(currVertex+1),**currVertex) )
                        {
                            std::swap(*currVertex, *(currVertex+1));

                            if( aComp(**currVertex,**prevVertex) )
                            {
                                // one swap was not sufficient -
                                // fallback to generic sort algo, then
                                detail::sortAET(*pAET, *pAETOther);
                                bFallbackTaken = true;
                                break;
                            }
                        }

                        if( (*currVertex)->mnYCounter > 0 )
                            pAETOther->push_back( *currVertex );

                        prevVertex = currVertex++;
                    }

                    // don't forget to add last vertex (loop above
                    // only deals with n-1 vertices)
                    if( !bFallbackTaken && (*currVertex)->mnYCounter > 0 )
                        pAETOther->push_back( *currVertex );
                }

                std::swap( pAET, pAETOther );
            }

            if( y >= nClipY1 )
                ++aScanline.y;
        }
    }

} // namespace basebmp

#endif /* INCLUDED_BASEBMP_POLYPOLYGONRENDERER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
