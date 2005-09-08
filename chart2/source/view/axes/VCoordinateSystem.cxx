/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VCoordinateSystem.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:39:24 $
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
#include "VCoordinateSystem.hxx"
#include "VCartesianCoordinateSystem.hxx"
#include "VPolarCoordinateSystem.hxx"
#include "ScaleAutomatism.hxx"
#include "VSeriesPlotter.hxx"
#include "ShapeFactory.hxx"
#include "servicenames_coosystems.hxx"

// header for define DBG_ASSERT
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

//static
VCoordinateSystem* VCoordinateSystem::createCoordinateSystem( const uno::Reference<
                                XBoundedCoordinateSystem >& xCooSysModel )
{
    rtl::OUString aViewServiceName = xCooSysModel->getViewServiceName();

    //@todo: in future the coordinatesystems should be instanciated via service factory
    VCoordinateSystem* pRet=NULL;
    if( aViewServiceName.equals( CHART2_COOSYSTEM_CARTESIAN_VIEW_SERVICE_NAME ) )
        pRet = new VCartesianCoordinateSystem(xCooSysModel);
    else if( aViewServiceName.equals( CHART2_COOSYSTEM_POLAR_VIEW_SERVICE_NAME ) )
        pRet = new VPolarCoordinateSystem(xCooSysModel);
    if(!pRet)
        pRet = new VCoordinateSystem(xCooSysModel);
    return pRet;
}

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
    , m_xLogicTargetForGrids(0)
    , m_xLogicTargetForAxes(0)
    , m_xFinalTarget(0)
    , m_xShapeFactory(0)
    , m_aMatrixSceneToScreen()
{
}
VCoordinateSystem::~VCoordinateSystem()
{
}

void SAL_CALL VCoordinateSystem::initPlottingTargets(  const uno::Reference< drawing::XShapes >& xLogicTarget
       , const uno::Reference< drawing::XShapes >& xFinalTarget
       , const uno::Reference< lang::XMultiServiceFactory >& xShapeFactory )
            throw (uno::RuntimeException)
{
    DBG_ASSERT(xLogicTarget.is()&&xFinalTarget.is()&&xShapeFactory.is(),"no proper initialization parameters");
    //is only allowed to be called once

    sal_Int32 nDimensionCount = m_xCooSysModel->getDimension();
    //create group shape for grids first thus axes are always painted above grids
    ShapeFactory aShapeFactory(xShapeFactory);
    if(nDimensionCount==2)
    {
        //create and add to target
        m_xLogicTargetForGrids = aShapeFactory.createGroup2D( xLogicTarget );
        m_xLogicTargetForAxes = aShapeFactory.createGroup2D( xLogicTarget );
    }
    else
    {
        //create and added to target
        m_xLogicTargetForGrids = aShapeFactory.createGroup3D( xLogicTarget );
        m_xLogicTargetForAxes = aShapeFactory.createGroup3D( xLogicTarget );
    }
    m_xFinalTarget  = xFinalTarget;
    m_xShapeFactory = xShapeFactory;
}

void VCoordinateSystem::setOrigin( double* fCoordinateOrigin )
{
    m_fCoordinateOrigin[0]=fCoordinateOrigin[0];
    m_fCoordinateOrigin[1]=fCoordinateOrigin[1];
    m_fCoordinateOrigin[2]=fCoordinateOrigin[2];
}

void VCoordinateSystem::setTransformationSceneToScreen(
    const drawing::HomogenMatrix& rMatrix )
{
    m_aMatrixSceneToScreen = rMatrix;
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
    rExplicitScale.Minimum = -0.5;
    rExplicitScale.Maximum = 0.5;
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
            if(pMinMaxSupplier)
            {
                aScaleAutomatism.m_fValueMinimum = pMinMaxSupplier->getMinimumX();
                aScaleAutomatism.m_fValueMaximum = pMinMaxSupplier->getMaximumX();
            }
        }
        else if(1==nDim)
        {
            if(pMinMaxSupplier)
            {
                const ExplicitScaleData& rScale = m_aExplicitScales[0];
                //@todo iterate through all xSlots which belong to coordinate system dimension in this plotter
                //and iterate through all plotter for this coordinate system dimension
                sal_Int32 nXSlotIndex = 0;
                aScaleAutomatism.m_fValueMinimum = pMinMaxSupplier->getMinimumYInRange(rScale.Minimum,rScale.Maximum);
                aScaleAutomatism.m_fValueMaximum = pMinMaxSupplier->getMaximumYInRange(rScale.Minimum,rScale.Maximum);
            }
        }
        else if(2==nDim)
        {
            if(pMinMaxSupplier)
            {
                aScaleAutomatism.m_fValueMinimum = pMinMaxSupplier->getMinimumZ();
                aScaleAutomatism.m_fValueMaximum = pMinMaxSupplier->getMaximumZ();
            }
        }
        aScaleAutomatism.calculateExplicitScaleAndIncrement(
                    m_aExplicitScales[nDim], m_aExplicitIncrements[nDim] );
    }
    if(nDimensionCount<3)
        setExplicitScaleToDefault(m_aExplicitScales[2]);
}

void VCoordinateSystem::createGridShapes()
{
}
void VCoordinateSystem::createAxesShapes( const ::com::sun::star::awt::Size& rReferenceSize, NumberFormatterWrapper* pNumberFormatterWrapper )
{
}

//.............................................................................
} //namespace chart
//.............................................................................
