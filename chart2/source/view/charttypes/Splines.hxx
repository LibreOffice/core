#ifndef _CHART2_SPLINECALCULATOR_HXX
#define _CHART2_SPLINECALCULATOR_HXX

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

class SplineCalculater
{
public:
    static void CalculateCubicSplines(
            const ::com::sun::star::drawing::PolyPolygonShape3D& rPoints
            , ::com::sun::star::drawing::PolyPolygonShape3D& rResult
            , sal_Int32 nGranularity );

    static void CalculateBSplines(
            const ::com::sun::star::drawing::PolyPolygonShape3D& rPoints
            , ::com::sun::star::drawing::PolyPolygonShape3D& rResult
            , sal_Int32 nGranularity
            , sal_Int32 nSplineDepth );
};


//.............................................................................
} //namespace chart
//.............................................................................
#endif
