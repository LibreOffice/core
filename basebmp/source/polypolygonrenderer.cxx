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

#include <basebmp/polypolygonrenderer.hxx>

#include <algorithm>


namespace basebmp
{
namespace detail
{
    sal_uInt32 setupGlobalEdgeTable( VectorOfVectorOfVertices&      rGET,
                                     basegfx::B2DPolyPolygon const& rPolyPoly,
                                     sal_Int32                      nMinY )
    {
        sal_Int32 const nNumScanlines( (sal_Int32)rGET.size() );

        
        for( sal_uInt32 i(0), nCount(rPolyPoly.count());
             i<nCount;
             ++i )
        {
            
            const basegfx::B2DPolygon& rPoly( rPolyPoly.getB2DPolygon(i) );
            for( sal_uInt32 k(0), nVertices(rPoly.count());
                 k<nVertices;
                 ++k )
            {
                const basegfx::B2DPoint& rP1( rPoly.getB2DPoint(k) );
                const basegfx::B2DPoint& rP2( rPoly.getB2DPoint( (k + 1) % nVertices ) );

                const sal_Int32 nVertexYP1( basegfx::fround(rP1.getY()) );
                const sal_Int32 nVertexYP2( basegfx::fround(rP2.getY()) );

                
                
                
                
                if(nVertexYP1 != nVertexYP2)
                {
                    if( nVertexYP2 < nVertexYP1 )
                    {
                        const sal_Int32 nStartScanline(nVertexYP2 - nMinY);

                        
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

        
        VectorOfVertexPtr::iterator iter( rAETSrc.begin() );
        VectorOfVertexPtr::iterator const end( rAETSrc.end() );
        while( iter != end )
        {
            if( (*iter)->mnYCounter > 0 )
                rAETDest.push_back( *iter );
            ++iter;
        }

        
        
        std::stable_sort( rAETDest.begin(), rAETDest.end(), aComp );
    }

} 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
