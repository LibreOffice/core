/*************************************************************************
 *
 *  $RCSfile: PlottingPositionHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-19 13:14:50 $
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

        aMatrix = aMatrix*m_aMatrixScreenToScene;

        m_xTransformationLogicToScene = new Linear3DTransformation(Matrix4DToHomogenMatrix( aMatrix ));
    }
    return m_xTransformationLogicToScene;
}

void PlottingPositionHelper::getScreenValuesForMinimum( uno::Sequence< double >& rSeq ) const
{
    double fX = this->getLogicMinX();
    double fY = this->getLogicMinY();
    double fZ = this->getLogicMinZ();

    this->doLogicScaling( &fX,&fY,&fZ );
    drawing::Position3D aPos( fX, fY, fZ);

    uno::Reference< XTransformation > xTransformation =
            this->getTransformationLogicToScene();
    rSeq = xTransformation->transform( Position3DToSequence(aPos) );
}

void PlottingPositionHelper::getScreenValuesForMaximum( uno::Sequence< double >& rSeq ) const
{
    double fX = this->getLogicMaxX();
    double fY = this->getLogicMaxY();
    double fZ = this->getLogicMaxZ();

    this->doLogicScaling( &fX,&fY,&fZ );
    drawing::Position3D aPos( fX, fY, fZ);

    uno::Reference< XTransformation > xTransformation =
            this->getTransformationLogicToScene();
    rSeq = xTransformation->transform( Position3DToSequence(aPos) );
}

void PlottingPositionHelper::getLogicMinimum( ::com::sun::star::uno::Sequence< double >& rSeq ) const
{
    rSeq.realloc(3);
    rSeq[0] = this->getLogicMinX();
    rSeq[1] = this->getLogicMinY();
    rSeq[2] = this->getLogicMinZ();
}
void PlottingPositionHelper::getLogicMaximum( ::com::sun::star::uno::Sequence< double >& rSeq ) const
{
    rSeq.realloc(3);
    rSeq[0] = this->getLogicMaxX();
    rSeq[1] = this->getLogicMaxY();
    rSeq[2] = this->getLogicMaxZ();
}

//.............................................................................
} //namespace chart
//.............................................................................
