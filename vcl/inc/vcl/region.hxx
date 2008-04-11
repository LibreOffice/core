/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: region.hxx,v $
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

#ifndef _SV_REGION_HXX
#define _SV_REGION_HXX

#include <tools/gen.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>

class ImplRegion;
class ImplRegionBand;
class Polygon;
class PolyPolygon;
struct ImplRegionInfo;

// --------------
// - RegionType -
// --------------

enum RegionType { REGION_NULL, REGION_EMPTY, REGION_RECTANGLE, REGION_COMPLEX };
enum RegionOverlapType { REGION_INSIDE, REGION_OVER, REGION_OUTSIDE };

typedef long RegionHandle;

// ----------
// - Region -
// ----------

class VCL_DLLPUBLIC Region
{
    friend class OutputDevice;
    friend class Window;
    friend class Bitmap;

private:
    ImplRegion*         mpImplRegion;

    SAL_DLLPRIVATE void             ImplCopyData();
    SAL_DLLPRIVATE void             ImplCreateRectRegion( const Rectangle& rRect );
    SAL_DLLPRIVATE void             ImplCreatePolyPolyRegion( const PolyPolygon& rPolyPoly );
    SAL_DLLPRIVATE void             ImplPolyPolyRegionToBandRegionFunc();
    SAL_DLLPRIVATE inline void      ImplPolyPolyRegionToBandRegion();
    SAL_DLLPRIVATE const ImplRegion*    ImplGetImplRegion() const { return mpImplRegion; }
    SAL_DLLPRIVATE ImplRegion*      ImplGetImplRegion() { return mpImplRegion; }
    SAL_DLLPRIVATE BOOL             ImplGetFirstRect( ImplRegionInfo& rImplRegionInfo,
                                          long& nX, long& nY, long& nWidth, long& nHeight ) const;
    SAL_DLLPRIVATE BOOL             ImplGetNextRect( ImplRegionInfo& rImplRegionInfo,
                                         long& nX, long& nY, long& nWidth, long& nHeight ) const;
    SAL_DLLPRIVATE void             ImplBeginAddRect( );
    SAL_DLLPRIVATE BOOL             ImplAddRect( const Rectangle& rRect );
    SAL_DLLPRIVATE void             ImplEndAddRect( );

#ifdef DBG_UTIL
    friend const char*  ImplDbgTestRegion( const void* pObj );
#endif

public:
                    Region();
                    Region( RegionType eType );
                    Region( const Rectangle& rRect );
                    Region( const Polygon& rPolygon );
                    Region( const PolyPolygon& rPolyPoly );
                    Region( const Region& rRegion );
                    ~Region();

    void            Move( long nHorzMove, long nVertMove );
    void            Scale( double fScaleX, double fScaleY );
    BOOL            Union( const Rectangle& rRegion );
    BOOL            Intersect( const Rectangle& rRegion );
    BOOL            Exclude( const Rectangle& rRegion );
    BOOL            XOr( const Rectangle& rRegion );
    BOOL            Union( const Region& rRegion );
    BOOL            Intersect( const Region& rRegion );
    BOOL            Exclude( const Region& rRegion );
    BOOL            XOr( const Region& rRegion );

    RegionType      GetType() const;
    BOOL            IsEmpty() const { return GetType() == REGION_EMPTY; };
    BOOL            IsNull() const { return GetType() == REGION_NULL; };

    void            SetEmpty();
    void            SetNull();

    Rectangle       GetBoundRect() const;

    BOOL            HasPolyPolygon() const;
    PolyPolygon     GetPolyPolygon() const;

    ULONG           GetRectCount() const;
    RegionHandle    BeginEnumRects();
    BOOL            GetEnumRects( RegionHandle hRegionHandle, Rectangle& rRect );
    BOOL            GetNextEnumRect( RegionHandle hRegionHandle, Rectangle& rRect )
                        { return GetEnumRects( hRegionHandle, rRect ); }
    void            EndEnumRects( RegionHandle hRegionHandle );

    BOOL            IsInside( const Point& rPoint ) const;
    BOOL            IsInside( const Rectangle& rRect ) const;
    BOOL            IsOver( const Rectangle& rRect ) const;

    Region&         operator=( const Region& rRegion );
    Region&         operator=( const Rectangle& rRect );

    BOOL            operator==( const Region& rRegion ) const;
    BOOL            operator!=( const Region& rRegion ) const
                        { return !(Region::operator==( rRegion )); }

    friend VCL_DLLPUBLIC SvStream& operator>>( SvStream& rIStm, Region& rRegion );
    friend VCL_DLLPUBLIC SvStream& operator<<( SvStream& rOStm, const Region& rRegion );

    /* workaround: faster conversion for PolyPolygons
     * if half of the Polygons contained in rPolyPoly are actually
     * rectangles, then the returned Region will be constructed by
     * XOr'ing the contained Polygons together; in the case of
     * only Rectangles this can be up to eight times faster than
     * Region( const PolyPolygon& ).
     * Caution: this is only useful if the Region is known to be
     * changed to rectangles; e.g. if being set as clip region
     */
    static Region GetRegionFromPolyPolygon( const PolyPolygon& rPolyPoly );
};

#endif  // _SV_REGION_HXX
