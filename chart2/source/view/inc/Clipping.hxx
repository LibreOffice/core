#ifndef _CHART2_CLIPPING_HXX
#define _CHART2_CLIPPING_HXX

#include "DoubleRectangle.hxx"

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class Clipping
{
    /** This class uses the Liang-Biarsky parametric line-clipping algorithm as described in:
            Computer Graphics: principles and practice, 2nd ed.,
            James D. Foley et al.,
            Section 3.12.4 on page 117.
    */

public:
    /** @descr  The intersection between an open polygon and a rectangle is
            calculated and the resulting lines are placed into the poly-polygon aResult.
        @param  rPolygon    The polygon is required to be open, ie. it's start and end point
            have different coordinates and that it is continuous, ie. has no holes.
        @param  rRectangle  The clipping area.
        @param  aResult The resulting lines that are the parts of the given polygon lying inside
            the clipping area are stored into aResult whose prior content is deleted first.
     */
    static void clipPolygonAtRectangle(
                                const ::com::sun::star::drawing::PolyPolygonShape3D& rPolygon
                                , const DoubleRectangle& rRectangle
                                , ::com::sun::star::drawing::PolyPolygonShape3D& aResult );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
