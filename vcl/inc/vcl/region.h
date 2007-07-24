/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: region.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:01:21 $
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

#ifndef _SV_REGION_H
#define _SV_REGION_H

#ifndef _SV_SV_H
#include <sv.h>
#endif
#ifndef _SV_REGBAND_HXX
#include <vcl/regband.hxx>
#endif
#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif

#ifndef _SV_REGION_HXX
#include <vcl/region.hxx>
#endif

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
