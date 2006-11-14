/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PlottingPositionHelper.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:35:17 $
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

#include "DoubleRectangle.hxx"

#ifndef _COM_SUN_STAR_CHART2_EXPLICITSCALEDATA_HPP_
#include <com/sun/star/chart2/ExplicitScaleData.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTRANSFORMATION_HPP_
#include <com/sun/star/chart2/XTransformation.hpp>
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

#ifndef _BGFX_MATRIX_B3DHOMMATRIX_HXX
#include <basegfx/matrix/b3dhommatrix.hxx>
#endif

//#ifndef _B3D_HMATRIX_HXX
//#include <goodies/hmatrix.hxx>
//#endif
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

//-----------------------------------------------------------------------------
/**
*/

class PlottingPositionHelper
{
public:
    PlottingPositionHelper();
    virtual ~PlottingPositionHelper();

    void setTransformationSceneToScreen( const ::com::sun::star::drawing::HomogenMatrix& rMatrix);

    void setScales( const ::com::sun::star::uno::Sequence<
            ::com::sun::star::chart2::ExplicitScaleData >& rScales );
    const ::com::sun::star::uno::Sequence<
            ::com::sun::star::chart2::ExplicitScaleData >& getScales() const;

    inline bool   isLogicVisible( double fX, double fY, double fZ ) const;
    inline void   doLogicScaling( double* pX, double* pY, double* pZ ) const;
    inline void   clipLogicValues( double* pX, double* pY, double* pZ ) const;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >
                  getTransformationLogicToScene() const;

    virtual ::com::sun::star::drawing::Position3D
            transformLogicToScene( double fX, double fY, double fZ, bool bClip ) const;

    void    transformScaledLogicToScene( ::com::sun::star::drawing::PolyPolygonShape3D& rPoly ) const;

    inline double getLogicMinX() const;
    inline double getLogicMinY() const;
    inline double getLogicMinZ() const;
    inline double getLogicMaxX() const;
    inline double getLogicMaxY() const;
    inline double getLogicMaxZ() const;

    inline bool isMathematicalOrientationX() const;
    inline bool isMathematicalOrientationY() const;
    inline bool isMathematicalOrientationZ() const;

    DoubleRectangle     getScaledLogicClipDoubleRect() const;

protected: //member
    ::com::sun::star::uno::Sequence<
            ::com::sun::star::chart2::ExplicitScaleData >   m_aScales;
    ::basegfx::B3DHomMatrix                                 m_aMatrixScreenToScene;

    //this is calculated based on m_aScales and m_aMatrixScreenToScene
    mutable ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTransformation >     m_xTransformationLogicToScene;
};

class PolarPlottingPositionHelper : public PlottingPositionHelper
    /*
                                  , public ::cppu::WeakImplHelper1<
                                ::com::sun::star::chart2::XTransformation >
                                */
{
public:
    PolarPlottingPositionHelper( bool bRadiusAxisMapsToFirstDimension );
    virtual ~PolarPlottingPositionHelper();

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XTransformation >
                  getTransformationLogicToScene() const;

    //the resulting values should be used for input to the transformation
    //received with 'getTransformationLogicToScene'
    double  transformToRadius( double fLogicValueOnRadiusAxis ) const;
    double  transformToAngleDegree( double fLogicValueOnAngleAxis ) const;
    double  getWidthAngleDegree( double& fStartLogicValueOnAngleAxis, double& fEndLogicValueOnAngleAxis ) const;
    //

    virtual ::com::sun::star::drawing::Position3D
            transformLogicToScene( double fX, double fY, double fZ, bool bClip ) const;
    ::com::sun::star::drawing::Position3D
            transformLogicToScene( double fLogicValueOnAngleAxis, double fLogicValueOnRadiusAxis, double fLogicZ ) const;

    double  getInnerLogicRadius() const;
    double  getOuterLogicRadius() const;

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
    PolarPlottingPositionHelper();
    bool m_bRadiusAxisMapsToFirstDimension;
};

bool PlottingPositionHelper::isLogicVisible(
    double fX, double fY, double fZ ) const
{
    return fX >= m_aScales[0].Minimum && fX <= m_aScales[0].Maximum
        && fY >= m_aScales[1].Minimum && fY <= m_aScales[1].Maximum
        && fZ >= m_aScales[2].Minimum && fZ <= m_aScales[2].Maximum;
}

void PlottingPositionHelper::doLogicScaling( double* pX, double* pY, double* pZ ) const
{
    if(pX && m_aScales[0].Scaling.is())
        *pX = m_aScales[0].Scaling->doScaling(*pX);
    if(pY && m_aScales[1].Scaling.is())
        *pY = m_aScales[1].Scaling->doScaling(*pY);
    if(pZ && m_aScales[2].Scaling.is())
        *pZ = m_aScales[2].Scaling->doScaling(*pZ);
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

//.............................................................................
} //namespace chart
//.............................................................................
#endif
