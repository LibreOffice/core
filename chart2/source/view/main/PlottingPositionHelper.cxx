/*************************************************************************
 *
 *  $RCSfile: PlottingPositionHelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: iha $ $Date: 2004-01-22 19:20:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "PlottingPositionHelper.hxx"
#include "CommonConverters.hxx"
#include "ViewDefines.hxx"
#include "Linear3DTransformation.hxx"

#ifndef _COM_SUN_STAR_DRAWING_POSITION3D_HPP_
#include <com/sun/star/drawing/Position3D.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

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

uno::Reference< XTransformation > PlottingPositionHelper::getTransformationLogicToScene() const
{
    //this is a standard transformation for a cartesian coordinate system

    //transformation from 2) to 4) //@todo 2) and 4) need a ink to a document

    //we need to apply this transformation to each geometric object because of a bug/problem
    //of the old drawing layer (the UNO_NAME_3D_EXTRUDE_DEPTH is an integer value instead of an double )
    if(!m_xTransformationLogicToScene.is())
    {
        Matrix4D aMatrix;
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
            aMatrix.TranslateX(-MinX);
        else
            aMatrix.TranslateX(-MaxX);
        if( AxisOrientation_MATHEMATICAL==m_aScales[1].Orientation )
            aMatrix.TranslateY(-MinY);
        else
            aMatrix.TranslateY(-MaxY);
        if( AxisOrientation_MATHEMATICAL==m_aScales[2].Orientation )
            aMatrix.TranslateZ(-MaxZ);//z direction in draw is reverse mathematical direction
        else
            aMatrix.TranslateY(-MinZ);

        double fWidthX = MaxX - MinX;
        double fWidthY = MaxY - MinY;
        double fWidthZ = MaxZ - MinZ;

        double fScaleDirectionX = AxisOrientation_MATHEMATICAL==m_aScales[0].Orientation ? 1.0 : -1.0;
        double fScaleDirectionY = AxisOrientation_MATHEMATICAL==m_aScales[1].Orientation ? 1.0 : -1.0;
        double fScaleDirectionZ = AxisOrientation_MATHEMATICAL==m_aScales[2].Orientation ? -1.0 : 1.0;

        aMatrix.ScaleX(fScaleDirectionX*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthX);
        aMatrix.ScaleY(fScaleDirectionY*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthY);
        aMatrix.ScaleZ(fScaleDirectionZ*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthZ);

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

Rectangle PlottingPositionHelper::getTransformedClipRect() const
{
    DoubleRectangle aDoubleRect( this->getTransformedClipDoubleRect() );

    Rectangle aRet( static_cast<long>(aDoubleRect.Left)
                  , static_cast<long>(aDoubleRect.Top)
                  , static_cast<long>(aDoubleRect.Right)
                  , static_cast<long>(aDoubleRect.Bottom) );
    return aRet;
}
DoubleRectangle PlottingPositionHelper::getTransformedClipDoubleRect() const
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

    drawing::Position3D aMimimum( MinX, MinY, MinZ);
    drawing::Position3D aMaximum( MaxX, MaxY, MaxZ);

    //transform to screen coordinates
    aMimimum = SequenceToPosition3D( getTransformationLogicToScene()
                    ->transform( Position3DToSequence(aMimimum) ) );
    aMaximum = SequenceToPosition3D( getTransformationLogicToScene()
                    ->transform( Position3DToSequence(aMaximum) ) );

    DoubleRectangle aRet( aMimimum.PositionX
                  , aMaximum.PositionY
                  , aMaximum.PositionX
                  , aMimimum.PositionY );
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

const uno::Sequence< ExplicitScaleData >& PolarPlottingPositionHelper::getScales() const
{
    return m_aScales;
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

        Matrix4D aMatrix;
        //the middle of the pie circle is the middle of the diagram
        aMatrix.TranslateX(fLogicDiameter/2.0);
        aMatrix.ScaleX(FIXED_SIZE_FOR_3D_CHART_VOLUME/fLogicDiameter);

        aMatrix.TranslateY(fLogicDiameter/2.0);
        aMatrix.ScaleY(FIXED_SIZE_FOR_3D_CHART_VOLUME/fLogicDiameter);

        aMatrix.ScaleZ(fScaleDirectionZ*FIXED_SIZE_FOR_3D_CHART_VOLUME);

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
