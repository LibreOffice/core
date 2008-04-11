/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: region.h,v $
 * $Revision: 1.3 $
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
    BOOL                mbFirst;
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
    ULONG               mnRefCount;
    ULONG               mnRectCount;
    PolyPolygon*        mpPolyPoly;
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
                        ImplRegion( const ImplRegion& rImplRegion );
                        ~ImplRegion();

    ImplRegionBand*     ImplGetFirstRegionBand() const { return mpFirstBand; }
    PolyPolygon*        ImplGetPolyPoly() const { return mpPolyPoly; }

    void                CreateBandRange( long nYTop, long nYBottom );
    void                InsertBands( long nYTop, long nYBottom );
    BOOL                InsertSingleBand( ImplRegionBand* mpImplRegionBand,
                                          long nYBandPosition );
    BOOL                InsertLine( const Point & rFirstPoint,
                                    const Point & rSecondPoint,
                                    long nLineID );
    BOOL                InsertPoint( const Point &rPoint,
                                     long nLineID,
                                     BOOL bEndPoint, LineType eLineType );

    void                Union( long nLeft, long nTop, long nRight, long nBottom );
    void                Exclude( long nLeft, long nTop, long nRight, long nBottom );
    void                XOr( long nLeft, long nTop, long nRight, long nBottom );

                        // remove emtpy rects
    BOOL                OptimizeBandList();

    friend SvStream&    operator>>( SvStream& rIStm, Region& rRegion );
    friend SvStream&    operator<<( SvStream& rOStm, const Region& rRegion );
};

#endif  // _SV_REGION_H
