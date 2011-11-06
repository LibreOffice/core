/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SV_IMPVECT_HXX
#define _SV_IMPVECT_HXX

#include <tools/poly.hxx>
#include <vcl/gdimtf.hxx>

// --------------
// - Vectorizer -
// --------------

class BitmapReadAccess;
class ImplChain;
class ImplVectMap;

class ImplVectorizer
{
private:

    ImplVectMap*    ImplExpand( BitmapReadAccess* pRAcc, const Color& rColor );
    void            ImplCalculate( ImplVectMap* pMap, PolyPolygon& rPolyPoly, sal_uInt8 cReduce, sal_uLong nFlags );
    sal_Bool            ImplGetChain( ImplVectMap* pMap, const Point& rStartPt, ImplChain& rChain );
    sal_Bool            ImplIsUp( ImplVectMap* pMap, long nY, long nX ) const;
    void            ImplLimitPolyPoly( PolyPolygon& rPolyPoly );

public:

                    ImplVectorizer();
                    ~ImplVectorizer();

    sal_Bool            ImplVectorize( const Bitmap& rColorBmp, GDIMetaFile& rMtf,
                                   sal_uInt8 cReduce, sal_uLong nFlags, const Link* pProgress );
    sal_Bool            ImplVectorize( const Bitmap& rMonoBmp, PolyPolygon& rPolyPoly,
                                   sal_uLong nFlags, const Link* pProgress );
};

#endif
