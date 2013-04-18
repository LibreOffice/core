/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SV_REGION_HXX
#define _SV_REGION_HXX

#include <tools/gen.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>

#include <basegfx/polygon/b2dpolypolygon.hxx>

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
    SAL_DLLPRIVATE void             ImplCreatePolyPolyRegion( const basegfx::B2DPolyPolygon& rPolyPoly );
    SAL_DLLPRIVATE void             ImplPolyPolyRegionToBandRegionFunc();
    SAL_DLLPRIVATE inline void      ImplPolyPolyRegionToBandRegion();
    SAL_DLLPRIVATE const ImplRegion*    ImplGetImplRegion() const { return mpImplRegion; }
    SAL_DLLPRIVATE ImplRegion*      ImplGetImplRegion() { return mpImplRegion; }
    SAL_DLLPRIVATE void             ImplBeginAddRect( );
    SAL_DLLPRIVATE sal_Bool             ImplAddRect( const Rectangle& rRect );
    SAL_DLLPRIVATE void             ImplEndAddRect( );
    SAL_DLLPRIVATE void             ImplIntersectWithPolyPolygon( const Region& );
    SAL_DLLPRIVATE void             ImplExcludePolyPolygon( const Region& );
    SAL_DLLPRIVATE void             ImplUnionPolyPolygon( const Region& );
    SAL_DLLPRIVATE void             ImplXOrPolyPolygon( const Region& );

public: // public within vcl
    VCL_PLUGIN_PUBLIC bool              ImplGetFirstRect( ImplRegionInfo& rImplRegionInfo,
                                          long& nX, long& nY, long& nWidth, long& nHeight ) const;
    VCL_PLUGIN_PUBLIC bool              ImplGetNextRect( ImplRegionInfo& rImplRegionInfo,
                                         long& nX, long& nY, long& nWidth, long& nHeight ) const;
#ifdef DBG_UTIL
    friend const char*  ImplDbgTestRegion( const void* pObj );
#endif

public:
    explicit        Region();
    explicit        Region( RegionType eType );
    explicit        Region( const Rectangle& rRect );
    explicit        Region( const Polygon& rPolygon );
    explicit        Region( const PolyPolygon& rPolyPoly );
    explicit        Region( const basegfx::B2DPolyPolygon& );
                    Region( const Region& rRegion );
                    ~Region();

    void            Move( long nHorzMove, long nVertMove );
    void            Scale( double fScaleX, double fScaleY );
    void            Union( const Rectangle& rRegion );
    void            Intersect( const Rectangle& rRegion );
    void            Exclude( const Rectangle& rRegion );
    void            XOr( const Rectangle& rRegion );
    void            Union( const Region& rRegion );
    void            Intersect( const Region& rRegion );
    void            Exclude( const Region& rRegion );
    void            XOr( const Region& rRegion );

    RegionType      GetType() const;
    sal_Bool            IsEmpty() const { return GetType() == REGION_EMPTY; };
    sal_Bool            IsNull() const { return GetType() == REGION_NULL; };

    void            SetEmpty();
    void            SetNull();

    Rectangle       GetBoundRect() const;

    sal_Bool            HasPolyPolygon() const;
    PolyPolygon     GetPolyPolygon() const;
    // returns an empty polypolygon in case HasPolyPolygon is sal_False
    const basegfx::B2DPolyPolygon GetB2DPolyPolygon() const;
    // returns a PolyPolygon either copied from the set PolyPolygon region
    // or created from the constituent rectangles
    basegfx::B2DPolyPolygon ConvertToB2DPolyPolygon();

    sal_uLong           GetRectCount() const;
    RegionHandle    BeginEnumRects();
    sal_Bool            GetEnumRects( RegionHandle hRegionHandle, Rectangle& rRect );
    sal_Bool            GetNextEnumRect( RegionHandle hRegionHandle, Rectangle& rRect )
                        { return GetEnumRects( hRegionHandle, rRect ); }
    void            EndEnumRects( RegionHandle hRegionHandle );

    sal_Bool            IsInside( const Point& rPoint ) const;
    sal_Bool            IsInside( const Rectangle& rRect ) const;
    sal_Bool            IsOver( const Rectangle& rRect ) const;

    Region&         operator=( const Region& rRegion );
    Region&         operator=( const Rectangle& rRect );

    sal_Bool            operator==( const Region& rRegion ) const;
    sal_Bool            operator!=( const Region& rRegion ) const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
