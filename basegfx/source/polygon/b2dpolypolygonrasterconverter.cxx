/*************************************************************************
 *
 *  $RCSfile: b2dpolypolygonrasterconverter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 18:39:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <basegfx/polygon/b2dpolypolygonrasterconverter.hxx>

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#ifndef BOOST_MEM_FN_HPP_INCLUDED
#include <boost/mem_fn.hpp>
#endif

#include <algorithm>


namespace basegfx
{
    //************************************************************
    // Internal vertex storage of B2DPolyPolygonRasterConverter
    //************************************************************

    inline B2DPolyPolygonRasterConverter::Vertex::Vertex() :
        aP1(),
        aP2(),
        bDownwards( true )
    {
    }

    inline B2DPolyPolygonRasterConverter::Vertex::Vertex( const B2DPoint& rP1, const B2DPoint& rP2, bool bDown ) :
        aP1( rP1 ),
        aP2( rP2 ),
        bDownwards( bDown )
    {
    }


    //************************************************************
    // Helper class for holding horizontal line segments during raster
    // conversion
    //************************************************************

    namespace
    {
        class ImplLineNode
        {
            sal_Int32   mnYCounter;
            double      mfXPos;
            double      mfXDelta;
            bool        mbDownwards;

        public:
            /**rP1 and rP2 must not have equal y values, when rounded
               to integer!
            */
            ImplLineNode(const B2DPoint& rP1, const B2DPoint& rP2, bool bDown) :
                mnYCounter( fround(rP2.getY()) - fround(rP1.getY()) ),
                mfXPos( rP1.getX() ),
                mfXDelta( (rP2.getX() - rP1.getX()) / mnYCounter ),
                mbDownwards( bDown )
            {
            }

            /// get current x position
            const double& getXPos() const
            {
                return mfXPos;
            }

            /// returns true, if line ends on this Y value
            void nextLine()
            {
                if(mnYCounter>=0)
                {
                    // go one step in Y
                    mfXPos += mfXDelta;
                    --mnYCounter;
                }
            }

            bool isEnded()
            {
                return mnYCounter<=0;
            }

            bool isDownwards()
            {
                return mbDownwards;
            }
        };
    }

    typedef ::std::vector<ImplLineNode> VectorOfLineNodes;


    //************************************************************
    //   Base2D PolyPolygon Raster Converter (Rasterizer)
    //************************************************************

    namespace
    {
        struct VertexComparator
        {
            bool operator()( const B2DPolyPolygonRasterConverter::Vertex& rLHS,
                             const B2DPolyPolygonRasterConverter::Vertex& rRHS )
            {
                return rLHS.aP1.getX() < rRHS.aP1.getX();
            }
        };
    }

    void B2DPolyPolygonRasterConverter::init()
    {
        if(!maPolyPolyRectangle.isEmpty())
        {
            const sal_Int32 nMinY( fround(maPolyPolyRectangle.getMinY()) );
            const sal_Int32 nScanlines(fround(maPolyPolyRectangle.getMaxY()) - nMinY);

            maScanlines.resize( nScanlines+1 );

            // add all polygons
            for( sal_uInt32 i(0), nCount(maPolyPolygon.count());
                 i < nCount;
                 ++i )
            {
                // add all vertices
                const B2DPolygon& rPoly( maPolyPolygon.getB2DPolygon(i) );
                for( sal_uInt32 k(0), nVertices(rPoly.count());
                     k<nVertices;
                     ++k )
                {
                    const B2DPoint& rP1( rPoly.getB2DPoint(k) );
                    const B2DPoint& rP2( rPoly.getB2DPoint( (k + 1) % nVertices ) );

                    const sal_Int32 nVertexYP1( fround(rP1.getY()) );
                    const sal_Int32 nVertexYP2( fround(rP2.getY()) );

                    // insert only vertices which are not strictly
                    // horizontal. Note that the ImplLineNode relies on
                    // this.
                    if(nVertexYP1 != nVertexYP2)
                    {
                        if( nVertexYP2 < nVertexYP1 )
                        {
                            const sal_Int32 nStartScanline(nVertexYP2 - nMinY);

                            // swap edges
                            maScanlines[ nStartScanline ].push_back( Vertex(rP2, rP1, false) );
                        }
                        else
                        {
                            const sal_Int32 nStartScanline(nVertexYP1 - nMinY);

                            maScanlines[ nStartScanline ].push_back( Vertex(rP1, rP2, true) );
                        }
                    }
                }
            }

            // now sort all scanlines, with increasing x coordinates
            VectorOfVertexVectors::iterator aIter( maScanlines.begin() );
            VectorOfVertexVectors::iterator aEnd( maScanlines.end() );
            while( aIter != aEnd )
            {
                ::std::sort( aIter->begin(),
                             aIter->end(),
                             VertexComparator() );
                ++aIter;
            }
        }
    }

    B2DPolyPolygonRasterConverter::B2DPolyPolygonRasterConverter( const B2DPolyPolygon& rPolyPoly ) :
        maPolyPolygon( rPolyPoly ),
        maPolyPolyRectangle( tools::getRange( rPolyPoly ) ),
        maScanlines()
    {
        init();
    }

    namespace
    {
        B2DRectangle getCombinedBounds( const B2DPolyPolygon& rPolyPolyRaster,
                                        const B2DRectangle&   rRasterArea )
        {
            B2DRectangle aRect( tools::getRange( rPolyPolyRaster ) );
            aRect.expand( rRasterArea );

            return aRect;
        }
    }

    B2DPolyPolygonRasterConverter::B2DPolyPolygonRasterConverter( const B2DPolyPolygon& rPolyPolyRaster,
                                                                  const B2DRectangle&   rRasterArea ) :
        maPolyPolygon( rPolyPolyRaster ),
        maPolyPolyRectangle(
            getCombinedBounds( rPolyPolyRaster,
                               rRasterArea ) ),
        maScanlines()
    {
        init();
    }

    B2DPolyPolygonRasterConverter::~B2DPolyPolygonRasterConverter()
    {
    }

    namespace
    {
        class LineNodeGenerator
        {
        public:
            LineNodeGenerator( VectorOfLineNodes& rActiveVertices ) :
                mrActiveVertices( rActiveVertices )
            {
            }

            void operator()( const B2DPolyPolygonRasterConverter::Vertex& rVertex )
            {
                mrActiveVertices.push_back( ImplLineNode(rVertex.aP1,
                                                         rVertex.aP2,
                                                         rVertex.bDownwards) );
            }

        private:
            VectorOfLineNodes& mrActiveVertices;
        };

        struct LineNodeComparator
        {
            bool operator()( const ImplLineNode& rLHS, const ImplLineNode& rRHS )
            {
                return rLHS.getXPos() < rRHS.getXPos();
            }
        };
    }

    void B2DPolyPolygonRasterConverter::rasterConvert( FillRule eFillRule )
    {
        if( maScanlines.empty() )
            return; // no scanlines at all -> bail out

        const sal_Int32 nMinY( fround(maPolyPolyRectangle.getMinY()) );
        const sal_Int32 nScanlines(fround(maPolyPolyRectangle.getMaxY()) - nMinY);

        // Vector of currently active vertices. A vertex is active, if
        // it crosses or touches the current scanline.
        VectorOfLineNodes   aActiveVertices;

        // process each scanline
        for( sal_Int32 y(0); y <= nScanlines; ++y )
        {
            // buffer last sorted aActiveVertices index, to later
            // perform an inplace_merge
            ::std::size_t nLastIndex( aActiveVertices.size() );

            // add vertices which start at current scanline into
            // active vertex vector
            ::std::for_each( maScanlines[y].begin(),
                             maScanlines[y].end(),
                             LineNodeGenerator( aActiveVertices ) );

            // sort with increasing X
            ::std::inplace_merge( aActiveVertices.begin(),
                                  aActiveVertices.begin()+nLastIndex,
                                  aActiveVertices.end(),
                                  LineNodeComparator() );

            const ::std::size_t nLen( aActiveVertices.size() );
            if( !nLen )
            {
                // empty scanline - call derived with an 'off' span
                // for the full width
                span( maPolyPolyRectangle.getMinX(),
                      maPolyPolyRectangle.getMaxX(),
                      nMinY + y,
                      false );
            }
            else
            {
                const sal_Int32 nCurrY( nMinY + y );

                // scanline not empty - forward all scans to derived,
                // according to selected fill rule

                // TODO(P1): Maybe allow these 'off' span calls to be
                // switched off (or all 'on' span calls, depending on
                // use case scenario)

                // call derived with 'off' span for everything left of first active span
                if( aActiveVertices.front().getXPos() > maPolyPolyRectangle.getMinX() )
                {
                    span( maPolyPolyRectangle.getMinX(),
                          aActiveVertices.front().getXPos(),
                          nCurrY,
                          false );
                }

                switch( eFillRule )
                {
                    default:
                        OSL_ENSURE(false,
                                   "B2DPolyPolygonRasterConverter::rasterConvert(): Unexpected fill rule");
                        return;

                    case FillRule_EVEN_ODD:
                        // process each span in current scanline, with
                        // even-odd fill rule
                        for( ::std::size_t i(0), nLen(aActiveVertices.size());
                             i+1 < nLen;
                             ++i )
                        {
                            span( aActiveVertices[i].getXPos(),
                                  aActiveVertices[i+1].getXPos(),
                                  nCurrY,
                                  i % 2 == 0 );

                            aActiveVertices[i].nextLine();
                        }
                        break;

                    case FillRule_NONZERO_WINDING_NUMBER:
                        // process each span in current scanline, with
                        // non-zero winding numbe fill rule
                        sal_Int32 nWindingNumber(0);
                        for( ::std::size_t i(0), nLen(aActiveVertices.size());
                             i+1 < nLen;
                             ++i )
                        {
                            nWindingNumber += -1 + 2*aActiveVertices[i].isDownwards();

                            span( aActiveVertices[i].getXPos(),
                                  aActiveVertices[i+1].getXPos(),
                                  nCurrY,
                                  nWindingNumber != 0 );

                            aActiveVertices[i].nextLine();
                        }
                        break;
                }

                // call derived with 'off' span for everything right of last active span
                if( aActiveVertices.back().getXPos() < maPolyPolyRectangle.getMaxX() )
                {
                    span( aActiveVertices.back().getXPos()+1.0,
                          maPolyPolyRectangle.getMaxX(),
                          nCurrY,
                          false );
                }

                // also call nextLine on very last line node
                aActiveVertices.back().nextLine();
            }

            // remove line nodes which have ended on the current scanline
            aActiveVertices.erase( ::std::remove_if( aActiveVertices.begin(),
                                                     aActiveVertices.end(),
                                                     ::boost::mem_fn( &ImplLineNode::isEnded ) ),
                                   aActiveVertices.end() );

            // TODO(P2): Maybe employ another sort here - typically,
            // only a few entries will have changed order, quite
            // probably even only swapped positions.

            // sort line nodes again - the nextLine() call changed the
            // X positions!
            ::std::sort( aActiveVertices.begin(),
                         aActiveVertices.end(),
                         LineNodeComparator() );
        }
    }

}
// eof
