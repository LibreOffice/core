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



#ifndef _SV_REGION_H
#define _SV_REGION_H

#include <vcl/regband.hxx>
#include <tools/poly.hxx>
#include <vcl/region.hxx>

// -----------------
// - Hilfsmethoden -
// -----------------

#ifdef DBG_UTIL
const char* ImplDbgTestRegion( const void* pObj );
#endif

// --------------------
// - ImplRegionHandle -
// --------------------

struct ImplRegionHandle
{
    Region*             mpRegion;
    ImplRegionBand*     mpCurrRectBand;
    ImplRegionBandSep*  mpCurrRectBandSep;
    sal_Bool                mbFirst;
};

// ------------------
// - ImplRegionInfo -
// ------------------

struct ImplRegionInfo
{
    void*               mpVoidCurrRectBand;
    void*               mpVoidCurrRectBandSep;
};

// --------------
// - ImplRegion -
// --------------

struct ImplRegionBase
{
public:
    ImplRegionBase( int nCount = 1 ); // TODO: replace manual refcounting
    virtual ~ImplRegionBase();
public:
    sal_uIntPtr             mnRefCount;
    sal_uIntPtr             mnRectCount;
    PolyPolygon*        mpPolyPoly;
    basegfx::B2DPolyPolygon* mpB2DPolyPoly;
};

class ImplRegion : public ImplRegionBase
{
    friend class Region;

private:
    ImplRegionBand*     mpFirstBand;        // root of the list with y-bands
    ImplRegionBand*     mpLastCheckedBand;

public:
                        ImplRegion();
                        ImplRegion( const PolyPolygon& rPolyPoly );
                        ImplRegion( const basegfx::B2DPolyPolygon& );
                        ImplRegion( const ImplRegion& rImplRegion );
                        ~ImplRegion();

    ImplRegionBand*     ImplGetFirstRegionBand() const { return mpFirstBand; }
    PolyPolygon*        ImplGetPolyPoly() const { return mpPolyPoly; }

    void                CreateBandRange( long nYTop, long nYBottom );
    void                InsertBands( long nYTop, long nYBottom );
    sal_Bool                InsertSingleBand( ImplRegionBand* mpImplRegionBand,
                                          long nYBandPosition );
    sal_Bool                InsertLine( const Point & rFirstPoint,
                                    const Point & rSecondPoint,
                                    long nLineID );
    sal_Bool                InsertPoint( const Point &rPoint,
                                     long nLineID,
                                     sal_Bool bEndPoint, LineType eLineType );

    /** Insert one band either after another band or as the first or only
        band.  Both the forward as well as the backward links are updated.
        @param pPreviousBand
            When <NULL/> then pBandToInsert is inserted as first band or as
            only band when there are no other bands.
            When not <NULL/> then pBandToInsert is inserted directly after
            pPreviousBand.
        @param pBandToInsert
            The band to insert.
    */
    void                InsertBand (ImplRegionBand* pPreviousBand,
                                    ImplRegionBand* pBandToInsert);

    void                Union( long nLeft, long nTop, long nRight, long nBottom );
    void                Exclude( long nLeft, long nTop, long nRight, long nBottom );
    void                XOr( long nLeft, long nTop, long nRight, long nBottom );

                        // remove emtpy rects
    sal_Bool                OptimizeBandList();

    friend SvStream&    operator>>( SvStream& rIStm, Region& rRegion );
    friend SvStream&    operator<<( SvStream& rOStm, const Region& rRegion );
};

#endif  // _SV_REGION_H
