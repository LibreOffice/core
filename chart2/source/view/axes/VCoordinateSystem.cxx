/*************************************************************************
 *
 *  $RCSfile: VCoordinateSystem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-15 08:51:25 $
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
#include "VCoordinateSystem.hxx"
#include "ScaleAutomatism.hxx"
#include "VSeriesPlotter.hxx"
#include "VGrid.hxx"

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::chart2;

VCoordinateSystem::VCoordinateSystem( const uno::Reference< XBoundedCoordinateSystem >& xCooSys )
    : m_xCooSysModel(xCooSys)
    , m_xAxis0(NULL)
    , m_xAxis1(NULL)
    , m_xAxis2(NULL)
    , m_xGridList0()
    , m_xGridList1()
    , m_xGridList2()
    , m_aExplicitScales(3)
    , m_aExplicitIncrements(3)
{
}
VCoordinateSystem::~VCoordinateSystem()
{

}

void VCoordinateSystem::setOrigin( double* fCoordinateOrigin )
{
    m_fCoordinateOrigin[0]=fCoordinateOrigin[0];
    m_fCoordinateOrigin[1]=fCoordinateOrigin[1];
    m_fCoordinateOrigin[2]=fCoordinateOrigin[2];
}

uno::Reference< XBoundedCoordinateSystem > VCoordinateSystem::getModel() const
{
    return m_xCooSysModel;
}

void VCoordinateSystem::addAxis( const uno::Reference< XAxis >& xAxis )
{
    if(!xAxis.is())
        return;
    sal_Int32 nDim = xAxis->getRepresentedDimension();
    if(0==nDim)
        m_xAxis0 = xAxis;
    else if(1==nDim)
        m_xAxis1 = xAxis;
    else if(2==nDim)
        m_xAxis2 = xAxis;
}

uno::Sequence< uno::Reference< XGrid > >& VCoordinateSystem::getGridListByDimension( sal_Int32 nDim )
{
    if( 0==nDim )
        return m_xGridList0;
    if( 1==nDim )
        return m_xGridList1;
    return m_xGridList2;
}

void VCoordinateSystem::addGrid( const uno::Reference< XGrid >& xGrid )
{
    if(!xGrid.is())
        return;
    sal_Int32 nDim = xGrid->getRepresentedDimension();
    uno::Sequence< uno::Reference< XGrid > >& rGridList
        = getGridListByDimension( nDim );

    rGridList.realloc(rGridList.getLength()+1);
    rGridList[rGridList.getLength()-1] = xGrid;
}

void VCoordinateSystem::createGridShapes(
              const uno::Reference< lang::XMultiServiceFactory>& xShapeFactory
            , const uno::Reference< drawing::XShapes >& xTarget
            , const drawing::HomogenMatrix& rHM_SceneToScreen )
{
    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    for( sal_Int32 nDim=0; nDim<3; nDim++)
    {
        uno::Sequence< uno::Reference< XGrid > >& rGridList
            = getGridListByDimension( nDim );
        for( sal_Int32 nN=0; nN<rGridList.getLength(); nN++ )
        {
            VGrid aGrid(rGridList[nN],nDimensionCount);
            aGrid.setMeterData( m_aExplicitScales[nDim], m_aExplicitIncrements[nDim] );

            aGrid.init(xTarget,xTarget,xShapeFactory);
            if(2==nDimensionCount)
                aGrid.setTransformationSceneToScreen( rHM_SceneToScreen );
            aGrid.setScales( m_aExplicitScales );
            aGrid.createShapes();
        }
    }
}

uno::Reference< XAxis > VCoordinateSystem::getAxisByDimension( sal_Int32 nDim  ) const
{
    uno::Reference< XAxis > xAxis(NULL);
    if(0==nDim)
        xAxis = m_xAxis0;
    else if(1==nDim)
        xAxis = m_xAxis1;
    else if(2==nDim)
        xAxis = m_xAxis2;
    return xAxis;
}

void setExplicitScaleToDefault( ExplicitScaleData& rExplicitScale )
{
    rExplicitScale.Minimum = 0.0;
    rExplicitScale.Maximum = 1.0;
    rExplicitScale.Orientation = AxisOrientation_MATHEMATICAL;
    //rExplicitScale.Scaling = ;
    //rExplicitScale.Breaks = ;
}

void VCoordinateSystem::doAutoScale( MinimumAndMaximumSupplier* pMinMaxSupplier )
{
    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    for( sal_Int32 nDim = 0; nDim < nDimensionCount; nDim++ )
    {
        uno::Reference< XScale > xScale(
            m_xCooSysModel->getScaleByDimension( nDim ),
            uno::UNO_QUERY );
        if( ! xScale.is() )
            continue;
        ScaleAutomatism aScaleAutomatism( xScale->getScaleData() );
        uno::Reference< XAxis > xAxis( this->getAxisByDimension(nDim) );
        if(xAxis.is())
        {
            uno::Reference< XIncrement > xInc( xAxis->getIncrement() );
            if( xInc.is() )
            {
                aScaleAutomatism.m_aSourceIncrement = xInc->getIncrementData();
                aScaleAutomatism.m_aSourceSubIncrementList = xInc->getSubIncrements();
            }
        }
        if(0==nDim)
        {
            aScaleAutomatism.m_fValueMinimum = pMinMaxSupplier->getMinimumX();
            aScaleAutomatism.m_fValueMaximum = pMinMaxSupplier->getMaximumX();
        }
        else if(1==nDim)
        {
            const ExplicitScaleData& rScale = m_aExplicitScales[0];
            //@todo iterate through all xSlots which belong to coordinate system dimension in this plotter
            //and iterate through all plotter for this coordinate system dimension
            sal_Int32 nXSlotIndex = 0;
            aScaleAutomatism.m_fValueMinimum = pMinMaxSupplier->getMinimumYInRange(rScale.Minimum,rScale.Maximum);
            aScaleAutomatism.m_fValueMaximum = pMinMaxSupplier->getMaximumYInRange(rScale.Minimum,rScale.Maximum);
        }
        else if(2==nDim)
        {
            aScaleAutomatism.m_fValueMinimum = pMinMaxSupplier->getMinimumZ();
            aScaleAutomatism.m_fValueMaximum = pMinMaxSupplier->getMaximumZ();
        }
        aScaleAutomatism.calculateExplicitScaleAndIncrement(
                    m_aExplicitScales[nDim], m_aExplicitIncrements[nDim] );
    }
    if(nDimensionCount<3)
        setExplicitScaleToDefault(m_aExplicitScales[2]);
}

//.............................................................................
} //namespace chart
//.............................................................................
