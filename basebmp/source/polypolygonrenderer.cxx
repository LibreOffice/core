/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polypolygonrenderer.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: thb $ $Date: 2006-07-27 11:35:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "basebmp/polypolygonrenderer.hxx"

#include <algorithm>


namespace basebmp
{
namespace detail
{
    sal_uInt32 setupGlobalEdgeTable( VectorOfVectorOfVertices&      rGET,
                                     basegfx::B2DPolyPolygon const& rPolyPoly,
                                     sal_Int32                      nMinY )
    {
        sal_uInt32 const nNumScanlines( rGET.size() );

        // add all polygons to GET
        for( sal_uInt32 i(0), nCount(rPolyPoly.count());
             i<nCount;
             ++i )
        {
            // add all vertices to GET
            const basegfx::B2DPolygon& rPoly( rPolyPoly.getB2DPolygon(i) );
            for( sal_uInt32 k(0), nVertices(rPoly.count());
                 k<nVertices;
                 ++k )
            {
                const basegfx::B2DPoint& rP1( rPoly.getB2DPoint(k) );
                const basegfx::B2DPoint& rP2( rPoly.getB2DPoint( (k + 1) % nVertices ) );

                const sal_Int32 nVertexYP1( basegfx::fround(rP1.getY()) );
                const sal_Int32 nVertexYP2( basegfx::fround(rP2.getY()) );

                // insert only vertices which are not strictly
                // horizontal. Strictly horizontal vertices don't add
                // any information that is not already present - due
                // to their adjacent vertices.
                if(nVertexYP1 != nVertexYP2)
                {
                    if( nVertexYP2 < nVertexYP1 )
                    {
                        const sal_Int32 nStartScanline(nVertexYP2 - nMinY);

                        // edge direction is upwards - add with swapped vertices
                        if( nStartScanline < nNumScanlines )
                            rGET[ nStartScanline ].push_back( Vertex(rP2, rP1, false) );
                    }
                    else
                    {
                        const sal_Int32 nStartScanline(nVertexYP1 - nMinY);

                        if( nStartScanline < nNumScanlines )
                            rGET[ nStartScanline ].push_back( Vertex(rP1, rP2, true) );
                    }
                }
            }
        }

        // now sort all scanlines individually, with increasing x
        // coordinates
        VectorOfVectorOfVertices::iterator       aIter( rGET.begin() );
        const VectorOfVectorOfVertices::iterator aEnd( rGET.end() );
        sal_uInt32                               nVertexCount(0);
        RasterConvertVertexComparator            aComp;
        while( aIter != aEnd )
        {
            std::sort( aIter->begin(),
                       aIter->end(),
                       aComp );
            nVertexCount += aIter->size();

            ++aIter;
        }

        return nVertexCount;
    }

    void sortAET( VectorOfVertexPtr& rAETSrc,
                  VectorOfVertexPtr& rAETDest )
    {
        static RasterConvertVertexComparator aComp;

        rAETDest.clear();

        // prune AET from ended edges
        VectorOfVertexPtr::iterator iter( rAETSrc.begin() );
        VectorOfVertexPtr::iterator const end( rAETSrc.end() );
        while( iter != end )
        {
            if( (*iter)->mnYCounter > 0 )
                rAETDest.push_back( *iter );
            ++iter;
        }

        // stable sort is necessary, to avoid segment crossing where
        // none was intended.
        std::stable_sort( rAETDest.begin(), rAETDest.end(), aComp );
    }

} // namespace detail
} // namespace basebmp
