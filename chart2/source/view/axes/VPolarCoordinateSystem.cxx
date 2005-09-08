/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VPolarCoordinateSystem.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:40:36 $
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
#include "VPolarCoordinateSystem.hxx"
#include "VPolarGrid.hxx"
#include "VPolarAxis.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

VPolarCoordinateSystem::VPolarCoordinateSystem( const uno::Reference< XBoundedCoordinateSystem >& xCooSys )
    : VCoordinateSystem(xCooSys)
{
}

VPolarCoordinateSystem::~VPolarCoordinateSystem()
{
}

void VPolarCoordinateSystem::createGridShapes()
{
    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    for( sal_Int32 nDim=0; nDim<3; nDim++)
    {
        uno::Sequence< uno::Reference< XGrid > >& rGridList
            = getGridListByDimension( nDim );
        for( sal_Int32 nN=0; nN<rGridList.getLength(); nN++ )
        {
            VPolarGrid aGrid(rGridList[nN],nDimensionCount);
            aGrid.setIncrements( m_aExplicitIncrements );
            aGrid.init(m_xLogicTargetForGrids,m_xFinalTarget,m_xShapeFactory);
            if(2==nDimensionCount)
                aGrid.setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            aGrid.setScales( m_aExplicitScales );
            aGrid.createShapes();
        }
    }
}

void VPolarCoordinateSystem::createAxesShapes( const awt::Size& rReferenceSize, NumberFormatterWrapper* pNumberFormatterWrapper )
{
    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    double fCoordinateOrigin[3] = { 0.0, 0.0, 0.0 };
    sal_Int32 nDim = 0;
    for( nDim = 0; nDim < 3; nDim++ )
        fCoordinateOrigin[nDim] = this->getOriginByDimension( nDim );
    //create angle axis (dimension index 0)
    for( nDim = 0; nDim < 3; nDim++ )
    {
        uno::Reference< XAxis > xAxis = this->getAxisByDimension(nDim);
        if(!xAxis.is())
            continue;
        AxisProperties aAxisProperties(xAxis,rReferenceSize);
        aAxisProperties.init();
        //-------------------
        VPolarAxis aAxis(aAxisProperties,pNumberFormatterWrapper,nDimensionCount);
        aAxis.setMeterData( m_aExplicitScales[nDim], m_aExplicitIncrements[nDim] );
        aAxis.init(m_xLogicTargetForAxes,m_xFinalTarget,m_xShapeFactory);
        aAxis.setIncrements( m_aExplicitIncrements );
        if(2==nDimensionCount)
            aAxis.setTransformationSceneToScreen( m_aMatrixSceneToScreen );
        aAxis.setScales( m_aExplicitScales );
        aAxis.createShapes();
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
