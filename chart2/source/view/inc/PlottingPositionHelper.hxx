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
#pragma once

#include <sal/config.h>

#include <memory>

#include <chartview/ExplicitScaleValues.hxx>

#include <basegfx/range/b2drectangle.hxx>
#include <tools/long.hxx>
#include <com/sun/star/drawing/Direction3D.hpp>
#include <com/sun/star/drawing/Position3D.hpp>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ref.hxx>
#include <svx/unoshape.hxx>

namespace com::sun::star::drawing { struct HomogenMatrix; }
namespace com::sun::star::drawing { struct PolyPolygonShape3D; }

namespace chart
{

/** allows the transformation of numeric values from one
     coordinate-system into another.  Values may be transformed using
     any mapping.
     This is a non-UNO variant of the css::chart2::XTransformation interface,
     but using more efficient calling and returning types.
  */
class XTransformation2
{
public:
    virtual ~XTransformation2();
     /** transforms the given input data tuple, given in the source
         coordinate system, according to the internal transformation
         rules, into a tuple of transformed coordinates in the
         destination coordinate system.

         <p>Note that both coordinate systems may have different
         dimensions, e.g., if a transformation does simply a projection
         into a lower-dimensional space.</p>

         @param aValues a source tuple of data that is to be
                transformed.  The length of this sequence must be
                equivalent to the dimension of the source coordinate
                system.

         @return the transformed data tuple.  The length of this
                 sequence is equal to the dimension of the output
                 coordinate system.

         @throws ::com::sun::star::lang::IllegalArgumentException
                if the dimension of the input vector is not equal to the
                dimension given in getSourceDimension().
      */
    virtual css::drawing::Position3D transform(
        const css::drawing::Position3D& rSourceValues ) const = 0;
    virtual css::drawing::Position3D transform(
        const css::uno::Sequence< double >& rSourceValues ) const = 0;
};


class PlottingPositionHelper
{
public:
    PlottingPositionHelper();
    PlottingPositionHelper( const PlottingPositionHelper& rSource );
    virtual ~PlottingPositionHelper();

    virtual std::unique_ptr<PlottingPositionHelper> clone() const;
    std::unique_ptr<PlottingPositionHelper> createSecondaryPosHelper( const ExplicitScaleData& rSecondaryScale );

    virtual void setTransformationSceneToScreen( const css::drawing::HomogenMatrix& rMatrix);

    virtual void setScales( std::vector< ExplicitScaleData >&& rScales, bool bSwapXAndYAxis );
    const std::vector< ExplicitScaleData >& getScales() const { return m_aScales;}

    //better performance for big data
    inline void   setCoordinateSystemResolution( const css::uno::Sequence< sal_Int32 >& rCoordinateSystemResolution );
    inline bool   isSameForGivenResolution( double fX, double fY, double fZ
                                , double fX2, double fY2, double fZ2 );

    inline bool   isStrongLowerRequested( sal_Int32 nDimensionIndex ) const;
    inline bool   isLogicVisible( double fX, double fY, double fZ ) const;
    inline void   doLogicScaling( double* pX, double* pY, double* pZ ) const;
    inline void   doUnshiftedLogicScaling( double* pX, double* pY, double* pZ ) const;
    inline void   clipLogicValues( double* pX, double* pY, double* pZ ) const;
           void   clipScaledLogicValues( double* pX, double* pY, double* pZ ) const;
    inline bool   clipYRange( double& rMin, double& rMax ) const;

    inline void   doLogicScaling( css::drawing::Position3D& rPos ) const;

    virtual ::chart::XTransformation2*
                  getTransformationScaledLogicToScene() const;

    virtual css::drawing::Position3D
            transformLogicToScene( double fX, double fY, double fZ, bool bClip ) const;

    virtual css::drawing::Position3D
            transformScaledLogicToScene( double fX, double fY, double fZ, bool bClip ) const;

    void    transformScaledLogicToScene( css::drawing::PolyPolygonShape3D& rPoly ) const;
    void    transformScaledLogicToScene( std::vector<std::vector<css::drawing::Position3D>>& rPoly ) const;

    static css::awt::Point transformSceneToScreenPosition(
                  const css::drawing::Position3D& rScenePosition3D
                , const rtl::Reference<SvxShapeGroupAnyD>& xSceneTarget
                , sal_Int32 nDimensionCount );

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
    css::drawing::Direction3D getScaledLogicWidth() const;

    inline bool isSwapXAndY() const;

    bool isPercentY() const;

    double getBaseValueY() const;

    inline bool maySkipPointsInRegressionCalculation() const;

    void setTimeResolution( tools::Long nTimeResolution, const Date& rNullDate );
    virtual void setScaledCategoryWidth( double fScaledCategoryWidth );
    void AllowShiftXAxisPos( bool bAllowShift );
    void AllowShiftZAxisPos( bool bAllowShift );

protected: //member
    std::vector< ExplicitScaleData >  m_aScales;
    ::basegfx::B3DHomMatrix             m_aMatrixScreenToScene;

    //this is calculated based on m_aScales and m_aMatrixScreenToScene
    mutable std::unique_ptr< ::chart::XTransformation2 >  m_xTransformationLogicToScene;

    bool    m_bSwapXAndY;//e.g. true for bar chart and false for column chart

    sal_Int32 m_nXResolution;
    sal_Int32 m_nYResolution;
    sal_Int32 m_nZResolution;

    bool m_bMaySkipPointsInRegressionCalculation;

    bool m_bDateAxis;
    tools::Long m_nTimeResolution;
    Date m_aNullDate;

    double m_fScaledCategoryWidth;
    bool   m_bAllowShiftXAxisPos;
    bool   m_bAllowShiftZAxisPos;
};

class PolarPlottingPositionHelper : public PlottingPositionHelper
{
public:
    PolarPlottingPositionHelper();
    PolarPlottingPositionHelper( const PolarPlottingPositionHelper& rSource );
    virtual ~PolarPlottingPositionHelper() override;

    virtual std::unique_ptr<PlottingPositionHelper> clone() const override;

    virtual void setTransformationSceneToScreen( const css::drawing::HomogenMatrix& rMatrix) override;
    virtual void setScales( std::vector< ExplicitScaleData >&& rScales, bool bSwapXAndYAxis ) override;

    const ::basegfx::B3DHomMatrix& getUnitCartesianToScene() const { return m_aUnitCartesianToScene;}

    virtual ::chart::XTransformation2*
                  getTransformationScaledLogicToScene() const override;

    //the resulting values provided by the following 3 methods should be used
    //for input to the transformation received with
    //'getTransformationScaledLogicToScene'

    /** Given a value in the radius axis scale range, it returns the normalized
     *  value.
     */
    double  transformToRadius( double fLogicValueOnRadiusAxis, bool bDoScaling=true ) const;

    /** Given a value in the angle axis scale range (e.g. [0,1] for pie charts)
     *  this method returns the related angle in degree.
     */
    double  transformToAngleDegree( double fLogicValueOnAngleAxis, bool bDoScaling=true ) const;

    /** Given 2 values in the angle axis scale range (e.g. [0,1] for pie charts)
     *  this method returns the angle between the 2 values keeping into account
     *  the correct axis orientation; (for instance, this method is used for
     *  computing the angle width of a pie slice).
     */
    double  getWidthAngleDegree( double& fStartLogicValueOnAngleAxis, double& fEndLogicValueOnAngleAxis ) const;

    virtual css::drawing::Position3D
            transformLogicToScene( double fX, double fY, double fZ, bool bClip ) const override;
    virtual css::drawing::Position3D
            transformScaledLogicToScene( double fX, double fY, double fZ, bool bClip ) const override;
    css::drawing::Position3D
            transformAngleRadiusToScene( double fLogicValueOnAngleAxis, double fLogicValueOnRadiusAxis, double fLogicZ, bool bDoScaling=true ) const;

    /** Return the scene coordinates of the passed point: this point is
     *  described through a normalized cylindrical coordinate system, with an
     *  optional offset.
     *  (For a standard pie chart the origin of the coordinate system is the
     *  pie center; for an of-pie chart the components of the aOffset
     *  parameter are not all zero).
     */
    css::drawing::Position3D
            transformUnitCircleToScene( double fUnitAngleDegree
                    , double fUnitRadius, double fLogicZ
                    , const ::basegfx::B3DVector& aOffset = ::basegfx::B3DVector()) const;

    using PlottingPositionHelper::transformScaledLogicToScene;

    double  getOuterLogicRadius() const;

    inline bool isMathematicalOrientationAngle() const;
    inline bool isMathematicalOrientationRadius() const;
public:
    ///m_bSwapXAndY (inherited): by default the X axis (scale[0]) represents
    ///the angle axis and the Y axis (scale[1]) represents the radius axis;
    ///when this parameter is true, the opposite happens (this is the case for
    ///pie charts).

    ///Offset for radius axis in absolute logic scaled values (1.0 == 1 category)
    ///For a donut, it represents the non-normalized inner radius (see notes for
    ///transformToRadius)
    double      m_fRadiusOffset;
    ///Offset for angle axis in real degree.
    ///For a pie it represents the angle offset at which the first slice have to
    ///start;
    double      m_fAngleDegreeOffset;

private:
    ::basegfx::B3DHomMatrix m_aUnitCartesianToScene;

    ::basegfx::B3DHomMatrix impl_calculateMatrixUnitCartesianToScene( const ::basegfx::B3DHomMatrix& rMatrixScreenToScene ) const;
};

bool PolarPlottingPositionHelper::isMathematicalOrientationAngle() const
{
    const ExplicitScaleData& rScale = m_bSwapXAndY ? m_aScales[1] : m_aScales[2];
    if( css::chart2::AxisOrientation_MATHEMATICAL==rScale.Orientation )
        return true;
    return false;
}
bool PolarPlottingPositionHelper::isMathematicalOrientationRadius() const
{
    const ExplicitScaleData& rScale = m_bSwapXAndY ? m_aScales[0] : m_aScales[1];
    if( css::chart2::AxisOrientation_MATHEMATICAL==rScale.Orientation )
        return true;
    return false;
}

//better performance for big data
void PlottingPositionHelper::setCoordinateSystemResolution( const css::uno::Sequence< sal_Int32 >& rCoordinateSystemResolution )
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
    if( !std::isfinite(fX) || !std::isfinite(fY) || !std::isfinite(fZ)
        || !std::isfinite(fX2) || !std::isfinite(fY2) || !std::isfinite(fZ2) )
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

bool PlottingPositionHelper::isStrongLowerRequested( sal_Int32 nDimensionIndex ) const
{
    if( m_aScales.empty() )
        return false;
    if( 0==nDimensionIndex )
        return m_bAllowShiftXAxisPos && m_aScales[nDimensionIndex].m_bShiftedCategoryPosition;
    else if( 2==nDimensionIndex )
        return m_bAllowShiftZAxisPos && m_aScales[nDimensionIndex].m_bShiftedCategoryPosition;
    return false;
}

bool PlottingPositionHelper::isLogicVisible(
    double fX, double fY, double fZ ) const
{
    return fX >= m_aScales[0].Minimum && ( isStrongLowerRequested(0) ? fX < m_aScales[0].Maximum : fX <= m_aScales[0].Maximum )
        && fY >= m_aScales[1].Minimum && fY <= m_aScales[1].Maximum
        && fZ >= m_aScales[2].Minimum && ( isStrongLowerRequested(2) ? fZ < m_aScales[2].Maximum : fZ <= m_aScales[2].Maximum );
}

void PlottingPositionHelper::doLogicScaling( double* pX, double* pY, double* pZ ) const
{
    if(pX)
    {
        if( m_aScales[0].Scaling.is())
            *pX = m_aScales[0].Scaling->doScaling(*pX);
        if( m_bAllowShiftXAxisPos && m_aScales[0].m_bShiftedCategoryPosition )
            (*pX) += m_fScaledCategoryWidth/2.0;
    }
    if(pY && m_aScales[1].Scaling.is())
        *pY = m_aScales[1].Scaling->doScaling(*pY);
    if(pZ)
    {
        if( m_aScales[2].Scaling.is())
            *pZ = m_aScales[2].Scaling->doScaling(*pZ);
        if( m_bAllowShiftZAxisPos && m_aScales[2].m_bShiftedCategoryPosition)
            (*pZ) += 0.5;
    }
}

void PlottingPositionHelper::doUnshiftedLogicScaling( double* pX, double* pY, double* pZ ) const
{
    if(pX && m_aScales[0].Scaling.is())
        *pX = m_aScales[0].Scaling->doScaling(*pX);
    if(pY && m_aScales[1].Scaling.is())
        *pY = m_aScales[1].Scaling->doScaling(*pY);
    if(pZ && m_aScales[2].Scaling.is())
        *pZ = m_aScales[2].Scaling->doScaling(*pZ);
}

void PlottingPositionHelper::doLogicScaling( css::drawing::Position3D& rPos ) const
{
    doLogicScaling( &rPos.PositionX, &rPos.PositionY, &rPos.PositionZ );
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
        std::swap( rMin, rMax );
    if( rMin > getLogicMaxY() )
        return false;
    if( rMax < getLogicMinY() )
        return false;
    if( rMin < getLogicMinY() )
        rMin = getLogicMinY();
    if( rMax > getLogicMaxY() )
        rMax = getLogicMaxY();
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
    return css::chart2::AxisOrientation_MATHEMATICAL == m_aScales[0].Orientation;
}
inline bool PlottingPositionHelper::isMathematicalOrientationY() const
{
    return css::chart2::AxisOrientation_MATHEMATICAL == m_aScales[1].Orientation;
}
inline bool PlottingPositionHelper::isMathematicalOrientationZ() const
{
    return css::chart2::AxisOrientation_MATHEMATICAL == m_aScales[2].Orientation;
}
inline bool PlottingPositionHelper::isSwapXAndY() const
{
    return m_bSwapXAndY;
}
inline bool PlottingPositionHelper::maySkipPointsInRegressionCalculation() const
{
    return m_bMaySkipPointsInRegressionCalculation;
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
