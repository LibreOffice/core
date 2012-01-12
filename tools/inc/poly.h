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



#ifndef _POLY_H
#define _POLY_H

#include <tools/gen.hxx>

#define MAX_64KPOINTS       ((((sal_uInt16)0xFFFF)-32)/sizeof(Point))

// -------------------
// - ImplPolygonData -
// -------------------

class ImplPolygonData
{
public:
    Point*          mpPointAry;
    sal_uInt8*           mpFlagAry;
    sal_uInt16          mnPoints;
    sal_uIntPtr           mnRefCount;
};

// ---------------
// - ImplPolygon -
// ---------------

class ImplPolygon  : public ImplPolygonData
{
public:
                    ImplPolygon( sal_uInt16 nInitSize, sal_Bool bFlags = sal_False );
                    ImplPolygon( sal_uInt16 nPoints, const Point* pPtAry, const sal_uInt8* pInitFlags = NULL );
                    ImplPolygon( const ImplPolygon& rImplPoly );
                    ~ImplPolygon();

    void            ImplSetSize( sal_uInt16 nSize, sal_Bool bResize = sal_True );
    void            ImplCreateFlagArray();
    void            ImplSplit( sal_uInt16 nPos, sal_uInt16 nSpace, ImplPolygon* pInitPoly = NULL );
    void            ImplRemove( sal_uInt16 nPos, sal_uInt16 nCount );
};

// -------------------
// - ImplPolyPolygon -
// -------------------

#define MAX_POLYGONS        ((sal_uInt16)0x3FF0)

class Polygon;
typedef Polygon* SVPPOLYGON;

class ImplPolyPolygon
{
public:
    SVPPOLYGON*     mpPolyAry;
    sal_uIntPtr           mnRefCount;
    sal_uInt16          mnCount;
    sal_uInt16          mnSize;
    sal_uInt16          mnResize;

                    ImplPolyPolygon( sal_uInt16 nInitSize, sal_uInt16 nResize )
                        { mpPolyAry = NULL; mnCount = 0; mnRefCount = 1;
                          mnSize = nInitSize; mnResize = nResize; }
                    ImplPolyPolygon( sal_uInt16 nInitSize );
                    ImplPolyPolygon( const ImplPolyPolygon& rImplPolyPoly );
                    ~ImplPolyPolygon();
};

#endif // _SV_POLY_H
