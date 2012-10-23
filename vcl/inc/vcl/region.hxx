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

#ifndef _SV_REGION_HXX
#define _SV_REGION_HXX

#include <tools/gen.hxx>
#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <boost/shared_ptr.hpp>

class ImplRegionBand;
class RegionBand;
class Polygon;
class PolyPolygon;

//////////////////////////////////////////////////////////////////////////////

typedef boost::shared_ptr< RegionBand > RegionBandPtr;
typedef boost::shared_ptr< PolyPolygon > PolyPolygonPtr;
typedef boost::shared_ptr< basegfx::B2DPolyPolygon > B2DPolyPolygonPtr;
typedef std::vector< Rectangle > RectangleVector;

//////////////////////////////////////////////////////////////////////////////

class VCL_DLLPUBLIC Region
{
private:
    friend class OutputDevice;
    friend class Window;
    friend class Bitmap;

    // possible contents
    B2DPolyPolygonPtr           mpB2DPolyPolygon;
    PolyPolygonPtr              mpPolyPolygon;
    RegionBandPtr               mpRegionBand;

    /// bitfield
    bool                        mbIsNull : 1;

    // helpers
    SAL_DLLPRIVATE void ImplCreatePolyPolyRegion( const PolyPolygon& rPolyPoly );
    SAL_DLLPRIVATE void ImplCreatePolyPolyRegion( const basegfx::B2DPolyPolygon& rPolyPoly );

    SAL_DLLPRIVATE PolyPolygon ImplCreatePolyPolygonFromRegionBand() const;
    SAL_DLLPRIVATE basegfx::B2DPolyPolygon ImplCreateB2DPolyPolygonFromRegionBand() const;

public:
    Region(bool bIsNull = false); // default creates empty region, with true a null region is created
    Region(const Rectangle& rRect);
    Region(const Polygon& rPolygon);
    Region(const PolyPolygon& rPolyPoly);
    Region(const basegfx::B2DPolyPolygon&);
    Region(const Region& rRegion);
    ~Region();

    // direct access to contents
    const basegfx::B2DPolyPolygon* getB2DPolyPolygon() const { return mpB2DPolyPolygon.get(); }
    const PolyPolygon* getPolyPolygon() const { return mpPolyPolygon.get(); }
    const RegionBand* getRegionBand() const { return mpRegionBand.get(); }

    // access with converters, the asked data will be created from the most
    // valuable data, buffered and returned
    const PolyPolygon GetAsPolyPolygon() const;
    const basegfx::B2DPolyPolygon GetAsB2DPolyPolygon() const;
    const RegionBand* GetAsRegionBand() const;

    // manipulators
    void Move( long nHorzMove, long nVertMove );
    void Scale( double fScaleX, double fScaleY );
    bool Union( const Rectangle& rRegion );
    bool Intersect( const Rectangle& rRegion );
    bool Exclude( const Rectangle& rRegion );
    bool XOr( const Rectangle& rRegion );
    bool Union( const Region& rRegion );
    bool Intersect( const Region& rRegion );
    bool Exclude( const Region& rRegion );
    bool XOr( const Region& rRegion );

    bool IsEmpty() const;
    bool IsNull() const;

    void SetEmpty();
    void SetNull();

    Rectangle GetBoundRect() const;
    bool HasPolyPolygonOrB2DPolyPolygon() const { return (getB2DPolyPolygon() || getPolyPolygon()); }
    void GetRegionRectangles(RectangleVector& rTarget) const;

    bool IsInside( const Point& rPoint ) const;
    bool IsInside( const Rectangle& rRect ) const;
    bool IsOver( const Rectangle& rRect ) const;

    Region& operator=( const Region& rRegion );
    Region& operator=( const Rectangle& rRect );

    bool operator==( const Region& rRegion ) const;
    bool operator!=( const Region& rRegion ) const { return !(Region::operator==( rRegion )); }

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

//////////////////////////////////////////////////////////////////////////////
// eof
