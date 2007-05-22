/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BarPositionHelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:15:03 $
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

#include "BarPositionHelper.hxx"
#include "Linear3DTransformation.hxx"
#include "ViewDefines.hxx"
#include "CommonConverters.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

BarPositionHelper::BarPositionHelper( bool bSwapXAndY )
        : CategoryPositionHelper( 1 )
{
}

BarPositionHelper::BarPositionHelper( const BarPositionHelper& rSource )
        : CategoryPositionHelper( rSource )
        , PlottingPositionHelper( rSource )
{
}

BarPositionHelper::~BarPositionHelper()
{
}

PlottingPositionHelper* BarPositionHelper::clone() const
{
    BarPositionHelper* pRet = new BarPositionHelper(*this);
    return pRet;
}

void BarPositionHelper::updateSeriesCount( double fSeriesCount )
{
    m_fSeriesCount = fSeriesCount;
}

uno::Reference< XTransformation > BarPositionHelper::getTransformationScaledLogicToScene() const
{
    //transformation from 2) to 4) //@todo 2) and 4) need a link to a document

    //we need to apply this transformation to each geometric object because of a bug/problem
    //of the old drawing layer (the UNO_NAME_3D_EXTRUDE_DEPTH is an integer value instead of an double )

    if( !m_xTransformationLogicToScene.is() )
    {
        ::basegfx::B3DHomMatrix aMatrix;

        double MinX = getLogicMinX();
        double MinY = getLogicMinY();
        double MinZ = getLogicMinZ();
        double MaxX = getLogicMaxX();
        double MaxY = getLogicMaxY();
        double MaxZ = getLogicMaxZ();

        AxisOrientation nXAxisOrientation = m_aScales[0].Orientation;
        AxisOrientation nYAxisOrientation = m_aScales[1].Orientation;
        AxisOrientation nZAxisOrientation = m_aScales[2].Orientation;

        //apply scaling
        //scaling of x axis is refused/ignored
        doLogicScaling( NULL, &MinY, &MinZ );
        doLogicScaling( NULL, &MaxY, &MaxZ);

        if(m_bSwapXAndY)
        {
            std::swap(MinX,MinY);
            std::swap(MaxX,MaxY);
            std::swap(nXAxisOrientation,nYAxisOrientation);
        }

        if( AxisOrientation_MATHEMATICAL==nXAxisOrientation )
            aMatrix.translate(-MinX,0.0,0.0);
        else
            aMatrix.translate(-MaxX,0.0,0.0);
        if( AxisOrientation_MATHEMATICAL==nYAxisOrientation )
            aMatrix.translate(0.0,-MinY,0.0);
        else
            aMatrix.translate(0.0,-MaxY,0.0);
        if( AxisOrientation_MATHEMATICAL==nZAxisOrientation )
            aMatrix.translate(0.0,0.0,-MaxZ);//z direction in draw is reverse mathematical direction
        else
            aMatrix.translate(0.0,0.0,-MinZ);

        double fWidthX = MaxX - MinX;
        double fWidthY = MaxY - MinY;
        double fWidthZ = MaxZ - MinZ;

        double fScaleDirectionX = AxisOrientation_MATHEMATICAL==nXAxisOrientation ? 1.0 : -1.0;
        double fScaleDirectionY = AxisOrientation_MATHEMATICAL==nYAxisOrientation ? 1.0 : -1.0;
        double fScaleDirectionZ = AxisOrientation_MATHEMATICAL==nZAxisOrientation ? -1.0 : 1.0;

        aMatrix.scale(fScaleDirectionX*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthX
                , fScaleDirectionY*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthY
                , fScaleDirectionZ*FIXED_SIZE_FOR_3D_CHART_VOLUME/fWidthZ);

        //if(nDim==2)
            aMatrix = m_aMatrixScreenToScene*aMatrix;

        m_xTransformationLogicToScene = new Linear3DTransformation(B3DHomMatrixToHomogenMatrix( aMatrix ),m_bSwapXAndY);
    }
    return m_xTransformationLogicToScene;
}

//.............................................................................
} //namespace chart
//.............................................................................
