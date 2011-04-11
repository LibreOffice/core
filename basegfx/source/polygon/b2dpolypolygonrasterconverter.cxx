/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#include <basegfx/polygon/b2dpolypolygonrasterconverter.hxx>

#include <basegfx/numeric/ftools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <boost/mem_fn.hpp>

#include <algorithm>

namespace basegfx
{
    class radixSort {

        //! public interface
        public:

            //! default constructor
            radixSort( void );

            //! destructor
            ~radixSort( void );

            bool sort( const float *pInput, sal_uInt32 nNumElements, sal_uInt32 dwStride );

            inline sal_uInt32 *indices( void ) const { return m_indices1; }

        //! private attributes
        private:

            // current size of index list
            sal_uInt32 m_current_size;

            // last known size of index list
            sal_uInt32 m_previous_size;

            // index lists
            sal_uInt32 *m_indices1;
            sal_uInt32 *m_indices2;

            sal_uInt32 m_counter[256*4];
            sal_uInt32 m_offset[256];

        //! private methods
        private:

            bool resize( sal_uInt32 nNumElements );
            inline void reset_indices( void );
            bool prepareCounters( const float *pInput, sal_uInt32 nNumElements, sal_uInt32 dwStride );
    };

    inline radixSort::radixSort( void ) {

        m_indices1 = NULL;
        m_indices2 = NULL;
        m_current_size = 0;
        m_previous_size = 0;

        reset_indices();
    }

    inline radixSort::~radixSort( void ) {

        delete [] m_indices2;
        delete [] m_indices1;
    }

    bool radixSort::resize( sal_uInt32 nNumElements ) {

        if(nNumElements==m_previous_size)
            return true;

        if(nNumElements > m_current_size) {

            // release index lists
            if(m_indices2)
                delete [] m_indices2;
            if(m_indices1)
                delete [] m_indices1;

            // allocate new index lists
            m_indices1 = new sal_uInt32[nNumElements];
            m_indices2 = new sal_uInt32[nNumElements];

            // check for out of memory situation
            if(!m_indices1 || !m_indices2) {
                delete [] m_indices1;
                delete [] m_indices2;
                m_indices1 = NULL;
                m_indices2 = NULL;
                m_current_size = 0;
                return false;
            }

            m_current_size = nNumElements;
        }

        m_previous_size = nNumElements;

        // initialize indices
        reset_indices();

        return true;
    }

    inline void radixSort::reset_indices( void ) {

        for(sal_uInt32 i=0;i<m_current_size;i++)
            m_indices1[i] = i;
    }

    bool radixSort::prepareCounters( const float *pInput, sal_uInt32 nNumElements, sal_uInt32 dwStride ) {

        // clear counters
        sal_uInt32 *ptr = m_counter;
        for(int i=0; i<64; ++i)
        {
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
            *ptr++ = 0;
        }

        // prepare pointers to relevant memory addresses
        sal_uInt8 *p = (sal_uInt8*)pInput;
        sal_uInt8 *pe = p+(nNumElements*dwStride);
        sal_uInt32 *h0= &m_counter[0];
        sal_uInt32 *h1= &m_counter[256];
        sal_uInt32 *h2= &m_counter[512];
        sal_uInt32 *h3= &m_counter[768];

        sal_uInt32 *Indices = m_indices1;
        float previous_value = *(float *)(((sal_uInt8 *)pInput)+(m_indices1[0]*dwStride));
        bool bSorted = true;
        while(p!=pe) {
            float value = *(float *)(((sal_uInt8 *)pInput)+((*Indices++)*dwStride));
            if(value<previous_value)    {
                bSorted = false;
                break;
            }
            previous_value = value;
            h0[*p++]++;
            h1[*p++]++;
            h2[*p++]++;
            h3[*p++]++;
            p += dwStride-4;
        }
        if(bSorted)
            return true;
        while(p!=pe) {
            h0[*p++]++;
            h1[*p++]++;
            h2[*p++]++;
            h3[*p++]++;
            p += dwStride-4;
        }
        return false;
    }

    bool radixSort::sort( const float *pInput, sal_uInt32 nNumElements, sal_uInt32 dwStride ) {

        if(!(pInput))
            return false;
        if(!(nNumElements))
            return false;
        if(!(resize(nNumElements)))
            return false;

        // prepare radix counters, return if already sorted
        if(prepareCounters(pInput,nNumElements,dwStride))
            return true;

        // count number of negative values
        sal_uInt32 num_negatives = 0;
        sal_uInt32 *h3= &m_counter[768];
        for(sal_uInt32 i=128;i<256;i++)
            num_negatives += h3[i];

        // perform passes, one for each byte
        for(sal_uInt32 j=0;j<4;j++) {

            // ignore this pass if all values have the same byte
            bool bRun = true;
            sal_uInt32 *current_counter = &m_counter[j<<8];
            sal_uInt8 unique_value = *(((sal_uInt8*)pInput)+j);
            if(current_counter[unique_value]==nNumElements)
                bRun=false;

            // does the incoming byte contain the sign bit?
            sal_uInt32 i;
            if(j!=3) {
                if(bRun) {
                    m_offset[0] = 0;
                    for(i=1;i<256;i++)
                        m_offset[i] = m_offset[i-1] + current_counter[i-1];
                    sal_uInt8 *InputBytes = (sal_uInt8 *)pInput;
                    sal_uInt32 *Indices = m_indices1;
                    sal_uInt32 *IndicesEnd = &m_indices1[nNumElements];
                    InputBytes += j;
                    while(Indices!=IndicesEnd) {
                        sal_uInt32 id = *Indices++;
                        m_indices2[m_offset[InputBytes[id*dwStride]]++] = id;
                    }
                    sal_uInt32 *Tmp = m_indices1;
                    m_indices1 = m_indices2;
                    m_indices2 = Tmp;
                }
            }
            else {
                if(bRun) {
                    m_offset[0] = num_negatives;
                    for(i=1;i<128;i++)
                        m_offset[i] = m_offset[i-1] + current_counter[i-1];
                    m_offset[255] = 0;
                    for(i=0;i<127;i++)
                        m_offset[254-i] = m_offset[255-i] + current_counter[255-i];
                    for(i=128;i<256;i++)
                        m_offset[i] += current_counter[i];
                    for(i=0;i<nNumElements;i++) {
                        sal_uInt32 Radix = (*(sal_uInt32 *)(((sal_uInt8 *)pInput)+(m_indices1[i]*dwStride)))>>24;
                        if(Radix<128) m_indices2[m_offset[Radix]++] = m_indices1[i];
                        else m_indices2[--m_offset[Radix]] = m_indices1[i];
                    }
                    sal_uInt32 *Tmp = m_indices1;
                    m_indices1 = m_indices2;
                    m_indices2 = Tmp;
                }
                else {
                    if(unique_value>=128) {
                        for(i=0;i<nNumElements;i++)
                            m_indices2[i] = m_indices1[nNumElements-i-1];
                        sal_uInt32 *Tmp = m_indices1;
                        m_indices1 = m_indices2;
                        m_indices2 = Tmp;
                    }
                }
            }
        }

        return true;
    }

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
        public:
            sal_Int32   mnYCounter;
            float       mfXPos;
            float       mfXDelta;
            bool        mbDownwards;

        public:
            /**rP1 and rP2 must not have equal y values, when rounded
               to integer!
            */
            ImplLineNode(const B2DPoint& rP1, const B2DPoint& rP2, bool bDown) :
                mnYCounter( fround(rP2.getY()) - fround(rP1.getY()) ),
                mfXPos( (float)(rP1.getX()) ),
                mfXDelta((float) ((rP2.getX() - rP1.getX()) / mnYCounter) ),
                mbDownwards( bDown )
            {
            }

            /// get current x position
            const float& getXPos() const
            {
                return mfXPos;
            }

            /// returns true, if line ends on this Y value
            float nextLine()
            {
                if(mnYCounter>=0)
                {
                    // go one step in Y
                    mfXPos += mfXDelta;
                    --mnYCounter;
                    return mfXDelta;
                }

                return 0.0f;
            }

            bool isEnded() const
            {
                return mnYCounter<=0;
            }

            bool isDownwards() const
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

        // mickey's optimized version...
        radixSort   rs;
        std::size_t nb(0);
        std::size_t nb_previous(0);
        bool        bSort(false);

        // process each scanline
        for( sal_Int32 y(0); y <= nScanlines; ++y )
        {
            // add vertices which start at current scanline into
            // active vertex vector
            ::std::for_each( maScanlines[y].begin(),
                             maScanlines[y].end(),
                             LineNodeGenerator( aActiveVertices ) );
            nb = aActiveVertices.size();
            if(nb != nb_previous)
            {
                nb_previous = nb;
                bSort = true;
            }

            // sort with increasing X
            if(bSort)
            {
                bSort = false;

                if( nb )
                {
                    rs.sort(&aActiveVertices[0].mfXPos,
                            nb,
                            sizeof(ImplLineNode));
                }
            }

            const std::size_t nLen( nb );
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

                // sorting didn't change the order of the elements
                // in memory but prepared a list of indices in sorted order.
                // thus we now process the nodes with an additional indirection.
                sal_uInt32 *sorted = rs.indices();

                // call derived with 'off' span for everything left of first active span
                if( aActiveVertices[sorted[0]].getXPos() > maPolyPolyRectangle.getMinX() )
                {
                    span( maPolyPolyRectangle.getMinX(),
                          aActiveVertices[sorted[0]].getXPos(),
                          nCurrY,
                          false );
                }

                switch( eFillRule )
                {
                    default:
                        OSL_FAIL("B2DPolyPolygonRasterConverter::rasterConvert(): Unexpected fill rule");
                        return;

                    case FillRule_EVEN_ODD:
                        // process each span in current scanline, with
                        // even-odd fill rule
                        for( ::std::size_t i(0), nLength(aActiveVertices.size());
                             i+1 < nLength;
                             ++i )
                        {
                            sal_uInt32 nIndex = sorted[i];
                            sal_uInt32 nNextIndex = sorted[i+1];
                            span( aActiveVertices[nIndex].getXPos(),
                                  aActiveVertices[nNextIndex].getXPos(),
                                  nCurrY,
                                  i % 2 == 0 );

                            float delta = aActiveVertices[nIndex].nextLine();
                            if(delta > 0.0f)
                            {
                                if(aActiveVertices[nIndex].getXPos() > aActiveVertices[nNextIndex].getXPos())
                                    bSort = true;
                            }
                            else if(delta < 0.0f)
                            {
                                if(i)
                                {
                                    sal_uInt32 nPrevIndex = sorted[i-1];
                                    if(aActiveVertices[nIndex].getXPos() < aActiveVertices[nPrevIndex].getXPos())
                                        bSort = true;
                                }
                            }
                        }
                        break;

                    case FillRule_NONZERO_WINDING_NUMBER:
                        // process each span in current scanline, with
                        // non-zero winding numbe fill rule
                        sal_Int32 nWindingNumber(0);
                        for( ::std::size_t i(0), nLength(aActiveVertices.size());
                             i+1 < nLength;
                             ++i )
                        {
                            sal_uInt32 nIndex = sorted[i];
                            sal_uInt32 nNextIndex = sorted[i+1];
                            nWindingNumber += -1 + 2*aActiveVertices[nIndex].isDownwards();

                            span( aActiveVertices[nIndex].getXPos(),
                                  aActiveVertices[nNextIndex].getXPos(),
                                  nCurrY,
                                  nWindingNumber != 0 );

                            float delta = aActiveVertices[nIndex].nextLine();
                            if(delta > 0.0f)
                            {
                                if(aActiveVertices[nIndex].getXPos() > aActiveVertices[nNextIndex].getXPos())
                                    bSort = true;
                            }
                            else if(delta < 0.0f)
                            {
                                if(i)
                                {
                                    sal_uInt32 nPrevIndex = sorted[i-1];
                                    if(aActiveVertices[nIndex].getXPos() < aActiveVertices[nPrevIndex].getXPos())
                                        bSort = true;
                                }
                            }
                        }
                        break;
                }

                // call derived with 'off' span for everything right of last active span
                if( aActiveVertices[sorted[nb-1]].getXPos()+1.0 < maPolyPolyRectangle.getMaxX() )
                {
                    span( aActiveVertices[sorted[nb-1]].getXPos()+1.0,
                          maPolyPolyRectangle.getMaxX(),
                          nCurrY,
                          false );
                }

                // also call nextLine on very last line node
                sal_uInt32 nIndex = sorted[nb-1];
                float delta = aActiveVertices[nIndex].nextLine();
                if(delta < 0.0f)
                {
                    if(nb)
                    {
                        sal_uInt32 nPrevIndex = sorted[nb-2];
                        if(aActiveVertices[nIndex].getXPos() < aActiveVertices[nPrevIndex].getXPos())
                            bSort = true;
                    }
                }
            }

            // remove line nodes which have ended on the current scanline
            aActiveVertices.erase( ::std::remove_if( aActiveVertices.begin(),
                                                     aActiveVertices.end(),
                                                     ::boost::mem_fn( &ImplLineNode::isEnded ) ),
                                   aActiveVertices.end() );
            nb = aActiveVertices.size();
            if(nb != nb_previous)
            {
                nb_previous = nb;
                bSort = true;
            }
        }
    }
}
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
