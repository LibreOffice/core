/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PlottingPositionHelper.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:36:10 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "PlottingPositionHelper.hxx"
#include "CommonConverters.hxx"
#include "ViewDefines.hxx"
#include "Linear3DTransformation.hxx"

#ifndef _COM_SUN_STAR_DRAWING_DOUBLESEQUENCE_HPP_
#include <com/sun/star/drawing/DoubleSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

PlottingPositionHelper::PlottingPositionHelper()
        : m_aScales()
        , m_aMatrixScreenToScene()
        , m_xTransformationLogicToScene(NULL)

{
}

PlottingPositionHelper::~PlottingPositionHelper()
{

}

void PlottingPositionHelper::setTransformationSceneToScreen( const drawing::HomogenMatrix& rMatrix)
{
    m_aMatrixScreenToScene = HomogenMatrixToMatrix4D(rMatrix);
    m_xTransformationLogicToScene = NULL;
}

void PlottingPositionHelper::setScales( const uno::Sequence< ExplicitScaleData >& rScales )
{
    m_aScales = rScales;
    m_xTransformationLogicToScene = NULL;
}
const uno::Sequence< ExplicitScaleData >& PlottingPositionHelper::getScales() const
{
    return m_aScales;
}

uno::Reference< XTransformation > PlottingPositionHelper::getTransformationLogicToScene() const
{
    //this is a standard transformation for a cartesian coordinate system

    //transformation from 2) to 4) //@todo 2) and 4) need a ink to a document

    //we need to apply this transformation to each geometric object because of a bug/problem
    //of the old drawing layer (the UNO_NAME_3D_EXTRUDE_DEPTH is an integer value instead of an double )
    if(!m_xTransformationLogicToScene.is())
    {
        ::basegfx::B3DHomMatrix aMatrix;
        double MinX = getLogicMinX();
        double MinY = getLogicMinY();
        double MinZ = getLogicMinZ();
        double MaxX = getLogicMaxX();
        double MaxY = getLogicMaxY();
        double MaxZ = getLogicMaxZ();

        //apply scaling
        doLogicScaling( &MinX, &MinY, &MinZ );
        doLogicScaling( &MaxX, &MaxY, &MaxZ);

        if( AxisOrientation_MATHEMATICAL==m_aScales[0].Orientation )
            aMatrix.translate(-MinX, 0.0, 0.0);
        else
            aMatrix.translate(-MaxX, 0.0, 0.0);
        if( AxisOrientation_MATHEMATICAL==m_aScales[1].Orientation )
            aMatrix.translate(0.0, -MinY, 0.0);
        else
            aMatrix.translate(0.0, -MaxY, 0.0);
        if( AxisOrientation_MATHEMATICAL==m_aScales[2].Orientation )
            aMatrix.translate(0.0, 0.0, -MaxZ);//z direction in draw is reverse mathematical direction
        else
            aMatrix.translate(0.0, 0.0, -MinZ);

        double fWidthX = MaxX - MinX;
        double fWidthY = MaxY - MinY;
        double fWidthZ = MaxZ - MinZ;

        double fScaleDirectionX = AxisOrientation_MATHEMATICAL==m_aScales[0].Orientation ? 1.0 : -1.0;
        double fScaleDirectionY = AxisOrientation_MATHEMATICAL==m_aScales[1].Orientation ? 1.0 : -1.0;
        double fScaleDirectionZ = AxisOrientation_MATHEMATICAL==m_aScales[2].Orientation ? -1.0 : 1.0;

        aMatrix.scale(fScaleDirectionX*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthX,
            fScaleDirectionY*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthY,
            fScaleDirectionZ*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthZ);

        aMatrix = m_aMatrixScreenToScene*aMatrix;

        m_xTransformationLogicToScene = new Linear3DTransformation(Matrix4DToHomogenMatrix( aMatrix ));
    }
    return m_xTransformationLogicToScene;
}

drawing::Position3D PlottingPositionHelper::transformLogicToScene(
    double fX, double fY, double fZ, bool bClip ) const
{
    if(bClip)
        this->clipLogicValues( &fX,&fY,&fZ );
    this->doLogicScaling( &fX,&fY,&fZ );
    drawing::Position3D aPos( fX, fY, fZ);

    uno::Reference< XTransformation > xTransformation =
        this->getTransformationLogicToScene();
    uno::Sequence< double > aSeq =
        xTransformation->transform( Position3DToSequence(aPos) );
    return SequenceToPosition3D(aSeq);
}

void PlottingPositionHelper::transformScaledLogicToScene( drawing::PolyPolygonShape3D& rPolygon ) const
{
    drawing::Position3D aScenePosition;
    for( sal_Int32 nS = rPolygon.SequenceX.getLength(); nS--;)
    {
        drawing::DoubleSequence& xValues = rPolygon.SequenceX[nS];
        drawing::DoubleSequence& yValues = rPolygon.SequenceY[nS];
        drawing::DoubleSequence& zValues = rPolygon.SequenceZ[nS];
        for( sal_Int32 nP = xValues.getLength(); nP--; )
        {
            double& fX = xValues[nP];
            double& fY = yValues[nP];
            double& fZ = zValues[nP];
            aScenePosition = this->transformLogicToScene( fX,fY,fZ,true );
            fX = aScenePosition.PositionX;
            fY = aScenePosition.PositionY;
            fZ = aScenePosition.PositionZ;
        }
    }
}

DoubleRectangle PlottingPositionHelper::getScaledLogicClipDoubleRect() const
{
    //get logic clip values:
    double MinX = getLogicMinX();
    double MinY = getLogicMinY();
    double MinZ = getLogicMinZ();
    double MaxX = getLogicMaxX();
    double MaxY = getLogicMaxY();
    double MaxZ = getLogicMaxZ();

    //apply scaling
    doLogicScaling( &MinX, &MinY, &MinZ );
    doLogicScaling( &MaxX, &MaxY, &MaxZ);

    DoubleRectangle aRet( MinX, MaxY, MaxX, MinY );
    return aRet;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

PolarPlottingPositionHelper::PolarPlottingPositionHelper( bool bRadiusAxisMapsToFirstDimension )
    : m_fRadiusOffset(0.0)
    , m_fAngleDegreeOffset(90.0)
    , m_bRadiusAxisMapsToFirstDimension(bRadiusAxisMapsToFirstDimension)
{
}

PolarPlottingPositionHelper::~PolarPlottingPositionHelper()
{
}

uno::Reference< XTransformation > PolarPlottingPositionHelper::getTransformationLogicToScene() const
{
    //transformation from 2) to 4) //@todo 2) and 4) need a ink to a document

    //?? need to apply this transformation to each geometric object, or would group be sufficient??

    if( !m_xTransformationLogicToScene.is() )
    {
        double MinX = getLogicMinX();
        double MinY = getLogicMinY();
        double MinZ = getLogicMinZ();
        double MaxX = getLogicMaxX();
        double MaxY = getLogicMaxY();
        double MaxZ = getLogicMaxZ();

        //apply scaling
        doLogicScaling( &MinX, &MinY, &MinZ );
        doLogicScaling( &MaxX, &MaxY, &MaxZ);

        double fLogicDiameter = 2*(fabs(MaxY - MinY) + m_fRadiusOffset);
        if( m_bRadiusAxisMapsToFirstDimension )
            fLogicDiameter = 2*(fabs(MaxX - MinX) + m_fRadiusOffset);

        double fScaleDirectionZ = AxisOrientation_MATHEMATICAL==m_aScales[2].Orientation ? 1.0 : -1.0;

        ::basegfx::B3DHomMatrix aMatrix;
        //the middle of the pie circle is the middle of the diagram
        aMatrix.translate(fLogicDiameter/2.0, 0.0, 0.0);
        aMatrix.scale(FIXED_SIZE_FOR_3D_CHART_VOLUME/fLogicDiameter, 1.0, 1.0);

        aMatrix.translate(0.0, fLogicDiameter/2.0, 0.0);
        aMatrix.scale(1.0, FIXED_SIZE_FOR_3D_CHART_VOLUME/fLogicDiameter, fScaleDirectionZ*FIXED_SIZE_FOR_3D_CHART_VOLUME);
        aMatrix = m_aMatrixScreenToScene*aMatrix;

        m_xTransformationLogicToScene = new Linear3DTransformation(Matrix4DToHomogenMatrix( aMatrix ));
    }
    return m_xTransformationLogicToScene;
}

double PolarPlottingPositionHelper::getWidthAngleDegree( double& fStartLogicValueOnAngleAxis, double& fEndLogicValueOnAngleAxis ) const
{
    if( !this->isMathematicalOrientationY() )
    {
        double fHelp = fEndLogicValueOnAngleAxis;
        fEndLogicValueOnAngleAxis = fStartLogicValueOnAngleAxis;
        fStartLogicValueOnAngleAxis = fHelp;
    }

    double fStartAngleDegree = this->transformToAngleDegree( fStartLogicValueOnAngleAxis );
    double fEndAngleDegree   = this->transformToAngleDegree( fEndLogicValueOnAngleAxis );
    double fWidthAngleDegree = fEndAngleDegree - fStartAngleDegree;

    while(fWidthAngleDegree<0.0)
        fWidthAngleDegree+=360.0;
    while(fWidthAngleDegree>360.0)
        fWidthAngleDegree-=360.0;

    return fWidthAngleDegree;
}

double PolarPlottingPositionHelper::transformToAngleDegree( double fLogicValueOnAngleAxis ) const
{
    double fRet=0.0;

    double fAxisAngleScaleDirection = 1.0;
    {
        const ExplicitScaleData& rScale = m_bRadiusAxisMapsToFirstDimension ? m_aScales[1] : m_aScales[0];
        if(AxisOrientation_MATHEMATICAL != rScale.Orientation)
            fAxisAngleScaleDirection *= -1.0;
    }

    double MinAngleValue = 0.0;
    double MaxAngleValue = 0.0;
    {
        double MinX = getLogicMinX();
        double MinY = getLogicMinY();
        double MaxX = getLogicMaxX();
        double MaxY = getLogicMaxY();
        double MinZ = getLogicMinZ();
        double MaxZ = getLogicMaxZ();

        doLogicScaling( &MinX, &MinY, &MinZ );
        doLogicScaling( &MaxX, &MaxY, &MaxZ);

        MinAngleValue = m_bRadiusAxisMapsToFirstDimension ? MinY : MinX;
        MaxAngleValue = m_bRadiusAxisMapsToFirstDimension ? MaxY : MaxX;
    }

    double fScaledLogicAngleValue = 0.0;
    {
        double fX = m_bRadiusAxisMapsToFirstDimension ? getLogicMaxX() : fLogicValueOnAngleAxis;
        double fY = m_bRadiusAxisMapsToFirstDimension ? fLogicValueOnAngleAxis : getLogicMaxY();
        double fZ = getLogicMaxZ();
        clipLogicValues( &fX, &fY, &fZ );
        doLogicScaling( &fX, &fY, &fZ );
        fScaledLogicAngleValue = m_bRadiusAxisMapsToFirstDimension ? fY : fX;
    }

    fRet = m_fAngleDegreeOffset
                  + fAxisAngleScaleDirection*(fScaledLogicAngleValue-MinAngleValue)*360.0
                    /fabs(MaxAngleValue-MinAngleValue);
    while(fRet>360.0)
        fRet-=360.0;
    while(fRet<0)
        fRet+=360.0;
    return fRet;
}

double PolarPlottingPositionHelper::transformToRadius( double fLogicValueOnRadiusAxis ) const
{
    double fRet=0.0;

    double fScaledLogicRadiusValue = 0.0;
    {
        double fX = m_bRadiusAxisMapsToFirstDimension ? fLogicValueOnRadiusAxis: getLogicMaxX();
        double fY = m_bRadiusAxisMapsToFirstDimension ? getLogicMaxY() : fLogicValueOnRadiusAxis;
        doLogicScaling( &fX, &fY, 0 );

        fScaledLogicRadiusValue = m_fRadiusOffset + ( m_bRadiusAxisMapsToFirstDimension ? fX : fY );

        bool bMinIsInnerRadius = true;
        const ExplicitScaleData& rScale = m_bRadiusAxisMapsToFirstDimension ? m_aScales[0] : m_aScales[1];
        if(AxisOrientation_MATHEMATICAL != rScale.Orientation)
            bMinIsInnerRadius = false;
        if(bMinIsInnerRadius)
        {
            double MinX = getLogicMinX();
            double MinY = getLogicMinY();
            doLogicScaling( &MinX, &MinY, 0 );
            fScaledLogicRadiusValue -= ( m_bRadiusAxisMapsToFirstDimension ? MinX : MinY );
        }
        else
        {
            double MaxX = getLogicMaxX();
            double MaxY = getLogicMaxY();
            doLogicScaling( &MaxX, &MaxY, 0 );
            fScaledLogicRadiusValue -= ( m_bRadiusAxisMapsToFirstDimension ? MaxX : MaxY );
        }
    }
    return fScaledLogicRadiusValue;
}

drawing::Position3D PolarPlottingPositionHelper::transformLogicToScene( double fX, double fY, double fZ, bool bClip ) const
{
    if(bClip)
        this->clipLogicValues( &fX,&fY,&fZ );
    double fLogicValueOnAngleAxis  = m_bRadiusAxisMapsToFirstDimension ? fY : fX;
    double fLogicValueOnRadiusAxis = m_bRadiusAxisMapsToFirstDimension ? fX : fY;
    return this->transformLogicToScene( fLogicValueOnAngleAxis, fLogicValueOnRadiusAxis, fZ );
}

drawing::Position3D PolarPlottingPositionHelper::transformLogicToScene( double fLogicValueOnAngleAxis, double fLogicValueOnRadiusAxis, double fLogicZ ) const
{
    double fAngleDegree = this->transformToAngleDegree(fLogicValueOnAngleAxis);
    double fAnglePi     = fAngleDegree*F_PI/180.0;
    double fRadius      = this->transformToRadius(fLogicValueOnRadiusAxis);
    drawing::Position3D aLogicPos(fRadius*cos(fAnglePi),fRadius*sin(fAnglePi),fLogicZ+0.5);
    drawing::Position3D aScenePosition3D( SequenceToPosition3D(
        this->getTransformationLogicToScene()->transform(
            Position3DToSequence(aLogicPos) ) ) );
    return aScenePosition3D;
}

double PolarPlottingPositionHelper::getInnerLogicRadius() const
{
    const ExplicitScaleData& rScale = m_bRadiusAxisMapsToFirstDimension ? m_aScales[0] : m_aScales[1];
    if( AxisOrientation_MATHEMATICAL==rScale.Orientation )
        return rScale.Minimum;
    else
        return rScale.Maximum;
}

double PolarPlottingPositionHelper::getOuterLogicRadius() const
{
    const ExplicitScaleData& rScale = m_bRadiusAxisMapsToFirstDimension ? m_aScales[0] : m_aScales[1];
    if( AxisOrientation_MATHEMATICAL==rScale.Orientation )
        return rScale.Maximum;
    else
        return rScale.Minimum;
}

/*
// ____ XTransformation ____
uno::Sequence< double > SAL_CALL PolarPlottingPositionHelper::transform(
                        const uno::Sequence< double >& rSourceValues )
            throw (uno::RuntimeException, lang::IllegalArgumentException)
{
    uno::Sequence< double > aSourceValues(3);
    return aSourceValues;
}

sal_Int32 SAL_CALL PolarPlottingPositionHelper::getSourceDimension() throw (uno::RuntimeException)
{
    return 3;
}

sal_Int32 SAL_CALL PolarPlottingPositionHelper::getTargetDimension() throw (uno::RuntimeException)
{
    return 3;
}
*/

//.............................................................................
} //namespace chart
//.............................................................................
