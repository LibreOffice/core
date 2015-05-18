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

#ifndef INCLUDED_VCL_REGION_HXX
#define INCLUDED_VCL_REGION_HXX

#include <tools/gen.hxx>
#include <vcl/dllapi.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <memory>

class ImplRegionBand;
class RegionBand;
class Polygon;
namespace tools { class PolyPolygon; }
namespace vcl { class Window; }
class OutputDevice;
class Bitmap;

typedef std::shared_ptr< RegionBand > RegionBandPtr;
typedef std::shared_ptr< tools::PolyPolygon > PolyPolygonPtr;
typedef std::shared_ptr< basegfx::B2DPolyPolygon > B2DPolyPolygonPtr;
typedef std::vector< Rectangle > RectangleVector;

namespace vcl {

class VCL_DLLPUBLIC Region
{
private:
    friend class ::OutputDevice;
    friend class ::vcl::Window;
    friend class ::Bitmap;

    // possible contents
    B2DPolyPolygonPtr           mpB2DPolyPolygon;
    PolyPolygonPtr              mpPolyPolygon;
    RegionBandPtr               mpRegionBand;

    /// bitfield
    bool                        mbIsNull : 1;

    // helpers
    SAL_DLLPRIVATE void ImplCreatePolyPolyRegion( const tools::PolyPolygon& rPolyPoly );
    SAL_DLLPRIVATE void ImplCreatePolyPolyRegion( const basegfx::B2DPolyPolygon& rPolyPoly );

    SAL_DLLPRIVATE tools::PolyPolygon ImplCreatePolyPolygonFromRegionBand() const;
    SAL_DLLPRIVATE basegfx::B2DPolyPolygon ImplCreateB2DPolyPolygonFromRegionBand() const;

public:

    explicit Region(bool bIsNull = false); // default creates empty region, with true a null region is created
    explicit Region(const Rectangle& rRect);
    explicit Region(const Polygon& rPolygon);
    explicit Region(const tools::PolyPolygon& rPolyPoly);
    explicit Region(const basegfx::B2DPolyPolygon&);
    Region(const vcl::Region& rRegion);
    ~Region();

    // direct access to contents
    const basegfx::B2DPolyPolygon* getB2DPolyPolygon() const { return mpB2DPolyPolygon.get(); }
    const tools::PolyPolygon* getPolyPolygon() const { return mpPolyPolygon.get(); }
    const RegionBand* getRegionBand() const { return mpRegionBand.get(); }

    // access with converters, the asked data will be created from the most
    // valuable data, buffered and returned
    const tools::PolyPolygon GetAsPolyPolygon() const;
    const basegfx::B2DPolyPolygon GetAsB2DPolyPolygon() const;
    const RegionBand* GetAsRegionBand() const;

    // manipulators
    void Move( long nHorzMove, long nVertMove );
    void Scale( double fScaleX, double fScaleY );
    bool Union( const Rectangle& rRegion );
    bool Intersect( const Rectangle& rRegion );
    bool Exclude( const Rectangle& rRegion );
    bool XOr( const Rectangle& rRegion );
    bool Union( const vcl::Region& rRegion );
    bool Intersect( const vcl::Region& rRegion );
    bool Exclude( const vcl::Region& rRegion );
    bool XOr( const vcl::Region& rRegion );

    bool IsEmpty() const;
    bool IsNull() const { return mbIsNull;}

    void SetEmpty();
    void SetNull();

    bool IsRectangle() const;

    Rectangle GetBoundRect() const;
    bool HasPolyPolygonOrB2DPolyPolygon() const { return (getB2DPolyPolygon() || getPolyPolygon()); }
    void GetRegionRectangles(RectangleVector& rTarget) const;

    bool IsInside( const Point& rPoint ) const;
    bool IsInside( const Rectangle& rRect ) const;
    bool IsOver( const Rectangle& rRect ) const;

    vcl::Region& operator=( const vcl::Region& rRegion );
    vcl::Region& operator=( const Rectangle& rRect );

    bool operator==( const vcl::Region& rRegion ) const;
    bool operator!=( const vcl::Region& rRegion ) const { return !(Region::operator==( rRegion )); }

    friend VCL_DLLPUBLIC SvStream& ReadRegion( SvStream& rIStm, vcl::Region& rRegion );
    friend VCL_DLLPUBLIC SvStream& WriteRegion( SvStream& rOStm, const vcl::Region& rRegion );

    /* workaround: faster conversion for PolyPolygons
     * if half of the Polygons contained in rPolyPoly are actually
     * rectangles, then the returned vcl::Region will be constructed by
     * XOr'ing the contained Polygons together; in the case of
     * only Rectangles this can be up to eight times faster than
     * Region( const tools::PolyPolygon& ).
     * Caution: this is only useful if the vcl::Region is known to be
     * changed to rectangles; e.g. if being set as clip region
     */
    static vcl::Region GetRegionFromPolyPolygon( const tools::PolyPolygon& rPolyPoly );
};

}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const vcl::Region& rRegion)
{
    if (rRegion.IsEmpty())
        return stream << "EMPTY";
    if (rRegion.getB2DPolyPolygon())
        return stream << "B2DPolyPolygon("
                      << *rRegion.getB2DPolyPolygon()
                      << ")";
    if (rRegion.getPolyPolygon())
        return stream << "unimplemented";
    if (rRegion.getRegionBand())
    {   // inlined because RegionBand is private to vcl
        stream << "RegionBand(";
        RectangleVector rects;
        rRegion.GetRegionRectangles(rects);
        if (rects.empty())
            stream << "EMPTY";
        for (size_t i = 0; i < rects.size(); ++i)
            stream << "[" << i << "] " << rects[i];
        stream << ")";
    }
    return stream;
}

#endif // INCLUDED_VCL_REGION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
