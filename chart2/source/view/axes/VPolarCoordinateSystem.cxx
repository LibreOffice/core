/*************************************************************************
 *
 *  $RCSfile: VPolarCoordinateSystem.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: iha $ $Date: 2004-01-17 13:09:59 $
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
#include "VPolarCoordinateSystem.hxx"
#include "VPolarGrid.hxx"
#include "VPolarAxis.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

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
    for( sal_Int32 nDim = 0; nDim < 3; nDim++ )
        fCoordinateOrigin[nDim] = this->getOriginByDimension( nDim );
    //create angle axis (dimension index 0)
    nDim = 0;
    {
        uno::Reference< XAxis > xAxis = this->getAxisByDimension(0);
        AxisProperties aAxisProperties;
        aAxisProperties.m_xAxisModel = xAxis;
        aAxisProperties.m_pfExrtaLinePositionAtOtherAxis =
            new double(nDim==1?fCoordinateOrigin[0]:fCoordinateOrigin[1]);
        aAxisProperties.m_aReferenceSize = rReferenceSize;
        //-------------------
        VPolarAxis aAxis(aAxisProperties,pNumberFormatterWrapper);
        aAxis.setMeterData( m_aExplicitScales[nDim], m_aExplicitIncrements[nDim] );
        aAxis.init(m_xLogicTargetForAxes,m_xFinalTarget,m_xShapeFactory);
        if(2==nDimensionCount)
            aAxis.setTransformationSceneToScreen( m_aMatrixSceneToScreen );
        aAxis.setScales( m_aExplicitScales );
        aAxis.createShapes();
    }

    /*
    for( nDim = 0; nDim < 3; nDim++ )
    {
        uno::Reference< XAxis > xAxis = this->getAxisByDimension(nDim);
        if(xAxis.is()
            &&2==nDimensionCount) //@todo remove this restriction if 3D axes are available
        {
            AxisProperties aAxisProperties;
            aAxisProperties.m_xAxisModel = xAxis;
            aAxisProperties.m_pfExrtaLinePositionAtOtherAxis =
                new double(nDim==1?fCoordinateOrigin[0]:fCoordinateOrigin[1]);
            aAxisProperties.m_aReferenceSize = rReferenceSize;
            //-------------------
            VAxis aAxis(aAxisProperties,pNumberFormatterWrapper);
            aAxis.setMeterData( m_aExplicitScales[nDim], m_aExplicitIncrements[nDim] );
            aAxis.init(m_xLogicTargetForAxes,m_xFinalTarget,m_xShapeFactory);
            if(2==nDimensionCount)
                aAxis.setTransformationSceneToScreen( m_aMatrixSceneToScreen );
            aAxis.setScales( m_aExplicitScales );
            aAxis.createShapes();
        }
    }
    */
}

//.............................................................................
} //namespace chart
//.............................................................................
