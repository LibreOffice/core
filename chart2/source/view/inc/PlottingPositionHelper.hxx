/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PlottingPositionHelper.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:10:57 $
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
#ifndef _CHART2_PLOTTINGPOSITIONHELPER_HXX
#define _CHART2_PLOTTINGPOSITIONHELPER_HXX

#include "LabelAlignment.hxx"

#ifndef _BGFX_RANGE_B2DRECTANGLE_HXX
#include <basegfx/range/b2drectangle.hxx>
#endif
#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTRANSFORMATION_HPP_
#include <com/sun/star/chart2/XTransformation.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DIRECTION3D_HPP_
#include <com/sun/star/drawing/Direction3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX_HPP_
#include <com/sun/star/drawing/HomogenMatrix.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

/*
//for WeakImplHelper1
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif
*/
//.............................................................................
namespace chart
{
//.............................................................................

class ShapeFactory;

//-----------------------------------------------------------------------------
/**
*/

class PlottingPositionHelper
{
public:
    PlottingPositionHelper();
    PlottingPositionHelper( const PlottingPositionHelper& rSource );
    virtual ~PlottingPositionHelper();

    virtual PlottingPositionHelper* clone() const;
    virtual PlottingPositionHelper* createSecondaryPosHelper( const ::com::sun::star::chart2::ExplicitScaleData& rSecondaryScale );

    virtual void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix);

    virtual void setScales( const ::com::sun::star::uno::Sequence<
            ::com::sun::star::chart2::ExplicitScaleData >& rScales
            , sal_Bool bSwapXAndYAxis );
    const ::com::sun::star::uno::Sequence<
            ::com::sun::star::chart2::ExplicitScaleData >& getScales() const;

    //better performance for big data
    inline void   setCoordinateSystemResolution( const ::com::sun::star::uno::Sequence< sal_Int32 >& rCoordinateSystemResolution );
    inline bool   isSameForGivenResolution( double fX, double fY, double fZ
                                , double fX2, double fY2, double fZ2 );

    inline bool   isLogicVisible( double fX, double fY, double fZ ) const;
    inline void   doLogicScaling( double* pX, double* pY, double* pZ, bool bClip=false ) const;
    inline void   clipLogicValues( double* pX, double* pY, double* pZ ) const;
           void   clipScaledLogicValues( double* pX, double* pY, double* pZ ) const;
    inline bool   clipYRange( double& rMin, double& rMax ) const;

    inline void   doLogicScaling( ::com::sun::star::drawing::Position3D& rPos, bool bClip=false ) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >
                  getTransformationScaledLogicToScene() const;

    virtual ::com::sun::star::drawing::Position3D
            transformLogicToScene( double fX, double fY, double fZ, bool bClip ) const;

    virtual ::com::sun::star::drawing::Position3D
            transformScaledLogicToScene( double fX, double fY, double fZ, bool bClip ) const;

    void    transformScaledLogicToScene( ::com::sun::star::drawing::PolyPolygonShape3D& rPoly ) const;

    static com::sun::star::awt::Point transformSceneToScreenPosition(
                  const com::sun::star::drawing::Position3D& rScenePosition3D
                , const com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& xSceneTarget
                , ShapeFactory* pShapeFactory, sal_Int32 nDimensionCount );

    inline double getLogicMinX() const;
    inline double getLogicMinY() const;
    inline double getLogicMinZ() const;
    inline double getLogicMaxX() const;
    inline double getLogicMaxY() const;
    inline double getLogicMaxZ() const;

    inline bool isMathematicalOrientationX() const;
    inline bool isMathematicalOrientationY() const;
    inline bool isMathematicalOrientationZ() const;

    ::basegfx::B2DRectangle     getScaledLogicClipDoubleRect() const;
    ::com::sun::star::drawing::Direction3D getScaledLogicWidth() const;

    inline bool isSwapXAndY() const;

    bool isPercentY() const;

    double getBaseValueY() const;

    inline bool maySkipPointsInRegressionCalculation() const;

protected: //member
    ::com::sun::star::uno::Sequence<
            ::com::sun::star::chart2::ExplicitScaleData >   m_aScales;
    ::basegfx::B3DHomMatrix                                 m_aMatrixScreenToScene;

    //this is calculated based on m_aScales and m_aMatrixScreenToScene
    mutable ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTransformation >     m_xTransformationLogicToScene;

    bool    m_bSwapXAndY;//e.g. true for bar chart and false for column chart

    sal_Int32 m_nXResolution;
    sal_Int32 m_nYResolution;
    sal_Int32 m_nZResolution;

    bool m_bMaySkipPointsInRegressionCalculation;
};

//describes wich axis of the drawinglayer scene or sreen axis are the normal axis
enum NormalAxis
{
      NormalAxis_X
    , NormalAxis_Y
    , NormalAxis_Z
};

class PolarPlottingPositionHelper : public PlottingPositionHelper
    /*
                                  , public ::cppu::WeakImplHelper1<
                                ::com::sun::star::chart2::XTransformation >
                                */
{
public:
    PolarPlottingPositionHelper( NormalAxis eNormalAxis=NormalAxis_Z );
    PolarPlottingPositionHelper( const PolarPlottingPositionHelper& rSource );
    virtual ~PolarPlottingPositionHelper();

    virtual PlottingPositionHelper* clone() const;

    virtual void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix);
    virtual void setScales( const ::com::sun::star::uno::Sequence<
            ::com::sun::star::chart2::ExplicitScaleData >& rScales
            , sal_Bool bSwapXAndYAxis );

    ::basegfx::B3DHomMatrix getUnitCartesianToScene() const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >
                  getTransformationScaledLogicToScene() const;

    //the resulting values should be used for input to the transformation
    //received with 'getTransformationScaledLogicToScene'
    double  transformToRadius( double fLogicValueOnRadiusAxis, bool bDoScaling=true ) const;
    double  transformToAngleDegree( double fLogicValueOnAngleAxis, bool bDoScaling=true ) const;
    double  getWidthAngleDegree( double& fStartLogicValueOnAngleAxis, double& fEndLogicValueOnAngleAxis ) const;
    //

    virtual ::com::sun::star::drawing::Position3D
            transformLogicToScene( double fX, double fY, double fZ, bool bClip ) const;
    virtual ::com::sun::star::drawing::Position3D
            transformScaledLogicToScene( double fX, double fY, double fZ, bool bClip ) const;
    ::com::sun::star::drawing::Position3D
            transformAngleRadiusToScene( double fLogicValueOnAngleAxis, double fLogicValueOnRadiusAxis, double fLogicZ, bool bDoScaling=true ) const;
    ::com::sun::star::drawing::Position3D
            transformUnitCircleToScene( double fUnitAngleDegree, double fUnitRadius, double fLogicZ, bool bDoScaling=true ) const;

    using PlottingPositionHelper::transformScaledLogicToScene;

    double  getInnerLogicRadius() const;
    double  getOuterLogicRadius() const;

    inline bool isMathematicalOrientationAngle() const;
    inline bool isMathematicalOrientationRadius() const;

    /*
    // ____ XTransformation ____
    /// @see ::com::sun::star::chart2::XTransformation
    virtual ::com::sun::star::uno::Sequence< double > SAL_CALL transform(
        const ::com::sun::star::uno::Sequence< double >& rSourceValues )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    /// @see ::com::sun::star::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getSourceDimension()
        throw (::com::sun::star::uno::RuntimeException);
    /// @see ::com::sun::star::chart2::XTransformation
    virtual sal_Int32 SAL_CALL getTargetDimension()
        throw (::com::sun::star::uno::RuntimeException);
        */
public:
    //Offset for radius axis in absolute logic scaled values (1.0 == 1 category)
    double      m_fRadiusOffset;
    //Offset for angle axis in real degree
    double      m_fAngleDegreeOffset;

private:
    ::basegfx::B3DHomMatrix m_aUnitCartesianToScene;
    NormalAxis  m_eNormalAxis;

    ::basegfx::B3DHomMatrix impl_calculateMatrixUnitCartesianToScene( const ::basegfx::B3DHomMatrix& rMatrixScreenToScene ) const;
};

bool PolarPlottingPositionHelper::isMathematicalOrientationAngle() const
{
    const ::com::sun::star::chart2::ExplicitScaleData& rScale = m_bSwapXAndY ? m_aScales[1] : m_aScales[2];
    if( ::com::sun::star::chart2::AxisOrientation_MATHEMATICAL==rScale.Orientation )
        return true;
    return false;
}
bool PolarPlottingPositionHelper::isMathematicalOrientationRadius() const
{
    const ::com::sun::star::chart2::ExplicitScaleData& rScale = m_bSwapXAndY ? m_aScales[0] : m_aScales[1];
    if( ::com::sun::star::chart2::AxisOrientation_MATHEMATICAL==rScale.Orientation )
        return true;
    return false;
}

//better performance for big data
void PlottingPositionHelper::setCoordinateSystemResolution( const ::com::sun::star::uno::Sequence< sal_Int32 >& rCoordinateSystemResolution )
{
    m_nXResolution = 1000;
    m_nYResolution = 1000;
    m_nZResolution = 1000;
    if( rCoordinateSystemResolution.getLength() > 0 )
        m_nXResolution = rCoordinateSystemResolution[0];
    if( rCoordinateSystemResolution.getLength() > 1 )
        m_nYResolution = rCoordinateSystemResolution[1];
    if( rCoordinateSystemResolution.getLength() > 2 )
        m_nZResolution = rCoordinateSystemResolution[2];
}

bool PlottingPositionHelper::isSameForGivenResolution( double fX, double fY, double fZ
                                , double fX2, double fY2, double fZ2 /*these values are all expected tp be scaled already*/ )
{
    if( !::rtl::math::isFinite(fX) || !::rtl::math::isFinite(fY) || !::rtl::math::isFinite(fZ)
        || !::rtl::math::isFinite(fX2) || !::rtl::math::isFinite(fY2) || !::rtl::math::isFinite(fZ2) )
        return false;

    double fScaledMinX = getLogicMinX();
    double fScaledMinY = getLogicMinY();
    double fScaledMinZ = getLogicMinZ();
    double fScaledMaxX = getLogicMaxX();
    double fScaledMaxY = getLogicMaxY();
    double fScaledMaxZ = getLogicMaxZ();

    doLogicScaling( &fScaledMinX, &fScaledMinY, &fScaledMinZ );
    doLogicScaling( &fScaledMaxX, &fScaledMaxY, &fScaledMaxZ);

    bool bSameX = ( static_cast<sal_Int32>(m_nXResolution*(fX - fScaledMinX)/(fScaledMaxX-fScaledMinX))
                == static_cast<sal_Int32>(m_nXResolution*(fX2 - fScaledMinX)/(fScaledMaxX-fScaledMinX)) );

    bool bSameY = ( static_cast<sal_Int32>(m_nYResolution*(fY - fScaledMinY)/(fScaledMaxY-fScaledMinY))
                == static_cast<sal_Int32>(m_nYResolution*(fY2 - fScaledMinY)/(fScaledMaxY-fScaledMinY)) );

    bool bSameZ = ( static_cast<sal_Int32>(m_nZResolution*(fZ - fScaledMinZ)/(fScaledMaxZ-fScaledMinZ))
                == static_cast<sal_Int32>(m_nZResolution*(fZ2 - fScaledMinZ)/(fScaledMaxZ-fScaledMinZ)) );

    return (bSameX && bSameY && bSameZ);
}

bool PlottingPositionHelper::isLogicVisible(
    double fX, double fY, double fZ ) const
{
    return fX >= m_aScales[0].Minimum && fX <= m_aScales[0].Maximum
        && fY >= m_aScales[1].Minimum && fY <= m_aScales[1].Maximum
        && fZ >= m_aScales[2].Minimum && fZ <= m_aScales[2].Maximum;
}

void PlottingPositionHelper::doLogicScaling( double* pX, double* pY, double* pZ, bool bClip ) const
{
    if(bClip)
        this->clipLogicValues( pX,pY,pZ );

    if(pX && m_aScales[0].Scaling.is())
        *pX = m_aScales[0].Scaling->doScaling(*pX);
    if(pY && m_aScales[1].Scaling.is())
        *pY = m_aScales[1].Scaling->doScaling(*pY);
    if(pZ && m_aScales[2].Scaling.is())
        *pZ = m_aScales[2].Scaling->doScaling(*pZ);
}

void PlottingPositionHelper::doLogicScaling( ::com::sun::star::drawing::Position3D& rPos, bool bClip ) const
{
    doLogicScaling( &rPos.PositionX, &rPos.PositionY, &rPos.PositionZ, bClip );
}

void PlottingPositionHelper::clipLogicValues( double* pX, double* pY, double* pZ ) const
{
    if(pX)
    {
        if( *pX < m_aScales[0].Minimum )
            *pX = m_aScales[0].Minimum;
        else if( *pX > m_aScales[0].Maximum )
            *pX = m_aScales[0].Maximum;
    }
    if(pY)
    {
        if( *pY < m_aScales[1].Minimum )
            *pY = m_aScales[1].Minimum;
        else if( *pY > m_aScales[1].Maximum )
            *pY = m_aScales[1].Maximum;
    }
    if(pZ)
    {
        if( *pZ < m_aScales[2].Minimum )
            *pZ = m_aScales[2].Minimum;
        else if( *pZ > m_aScales[2].Maximum )
            *pZ = m_aScales[2].Maximum;
    }
}

inline bool PlottingPositionHelper::clipYRange( double& rMin, double& rMax ) const
{
    //returns true if something remains
    if( rMin > rMax )
    {
        double fHelp = rMin;
        rMin = rMax;
        rMax = fHelp;
    }
    if( rMin > getLogicMaxY() )
        return false;
    if( rMax < getLogicMinY() )
        return false;
    if( rMin < getLogicMinY() )
        rMin = getLogicMinY();
    if( rMax > getLogicMaxY() )
        rMax = getLogicMaxY();
    if( rMin == rMax )
        return false;
    return true;
}

inline double PlottingPositionHelper::getLogicMinX() const
{
    return m_aScales[0].Minimum;
}
inline double PlottingPositionHelper::getLogicMinY() const
{
    return m_aScales[1].Minimum;
}
inline double PlottingPositionHelper::getLogicMinZ() const
{
    return m_aScales[2].Minimum;
}

inline double PlottingPositionHelper::getLogicMaxX() const
{
    return m_aScales[0].Maximum;
}
inline double PlottingPositionHelper::getLogicMaxY() const
{
    return m_aScales[1].Maximum;
}
inline double PlottingPositionHelper::getLogicMaxZ() const
{
    return m_aScales[2].Maximum;
}
inline bool PlottingPositionHelper::isMathematicalOrientationX() const
{
    return ::com::sun::star::chart2::AxisOrientation_MATHEMATICAL == m_aScales[0].Orientation;
}
inline bool PlottingPositionHelper::isMathematicalOrientationY() const
{
    return ::com::sun::star::chart2::AxisOrientation_MATHEMATICAL == m_aScales[1].Orientation;
}
inline bool PlottingPositionHelper::isMathematicalOrientationZ() const
{
    return ::com::sun::star::chart2::AxisOrientation_MATHEMATICAL == m_aScales[2].Orientation;
}
inline bool PlottingPositionHelper::isSwapXAndY() const
{
    return m_bSwapXAndY;
}
inline bool PlottingPositionHelper::maySkipPointsInRegressionCalculation() const
{
    return m_bMaySkipPointsInRegressionCalculation;
}

//.............................................................................
} //namespace chart
//.............................................................................
#endif
