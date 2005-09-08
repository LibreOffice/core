/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VPolarGrid.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:41:04 $
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
#include "VPolarGrid.hxx"
#include "VCartesianGrid.hxx"
#include "TickmarkHelper.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "chartview/ObjectIdentifier.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"

#ifndef _COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <com/sun/star/chart2/XIdentifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_LINESTYLE_HPP_
#include <com/sun/star/drawing/LineStyle.hpp>
#endif

#include <vector>
#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

VPolarGrid::VPolarGrid( const uno::Reference< XGrid >& xGrid, sal_Int32 nDimensionCount )
            : VMeterBase( uno::Reference<XMeter>::query(xGrid), nDimensionCount )
            , m_pPosHelper( new PolarPlottingPositionHelper(false) )
            , m_aIncrements()
{
    PlotterBase::m_pPosHelper = m_pPosHelper;
}

VPolarGrid::~VPolarGrid()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

void VPolarGrid::setIncrements( const uno::Sequence< ExplicitIncrementData >& rIncrements )
{
    m_aIncrements = rIncrements;
}

void VPolarGrid::getAllTickInfos( sal_Int32 nDimensionIndex, ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const
{
    TickmarkHelper aTickmarkHelper(
            m_pPosHelper->getScales()[nDimensionIndex], m_aIncrements[nDimensionIndex] );
    aTickmarkHelper.getAllTicks( rAllTickInfos );
}

//static
void VPolarGrid::createLinePointSequence_ForAngleAxis(
        drawing::PointSequenceSequence& rPoints
        , ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos
        , const ExplicitIncrementData& rIncrement
        , const ExplicitScaleData& rScale
        , PolarPlottingPositionHelper* pPosHelper
        , double fLogicRadius, double fLogicZ )
{
    uno::Reference< XScaling > xInverseScaling( NULL );
    if( rScale.Scaling.is() )
        xInverseScaling = rScale.Scaling->getInverseScaling();

    sal_Int32 nTick = 0;
    TickIter aIter( rAllTickInfos, rIncrement, 0, 0 );
    for( TickInfo* pTickInfo = aIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = aIter.nextInfo(), nTick++ )
    {
        if(nTick>=rPoints[0].getLength())
            rPoints[0].realloc(rPoints[0].getLength()+30);

        pTickInfo->updateUnscaledValue( xInverseScaling );
        double fLogicAngle = pTickInfo->fUnscaledTickValue;

        drawing::Position3D aScenePosition3D( pPosHelper->transformLogicToScene( fLogicAngle, fLogicRadius, fLogicZ ) );
        rPoints[0][nTick].X = static_cast<sal_Int32>(aScenePosition3D.PositionX);
        rPoints[0][nTick].Y = static_cast<sal_Int32>(aScenePosition3D.PositionY);
    }
    if(rPoints[0].getLength()>1)
    {
        rPoints[0].realloc(nTick+1);
        rPoints[0][nTick].X = rPoints[0][0].X;
        rPoints[0][nTick].Y = rPoints[0][0].Y;
    }
    else
        rPoints[0].realloc(0);
}

void VPolarGrid::create2DAngleGrid( const uno::Reference< drawing::XShapes >& xTarget
        , ::std::vector< ::std::vector< TickInfo > >& rRadiusTickInfos
        , ::std::vector< ::std::vector< TickInfo > >& rAngleTickInfos
        , const ::std::vector<VLineProperties>& rLinePropertiesList )
{
    const ExplicitScaleData&     rAngleScale = m_pPosHelper->getScales()[0];
    const ExplicitIncrementData& rAngleIncrement = m_aIncrements[0];
    uno::Reference< XScaling > xInverseScaling( NULL );
    if( rAngleScale.Scaling.is() )
        xInverseScaling = rAngleScale.Scaling->getInverseScaling();

    double fLogicInnerRadius = m_pPosHelper->getInnerLogicRadius();
    double fLogicOuterRadius = m_pPosHelper->getOuterLogicRadius();
    double fLogicZ      = -0.5;//as defined

    sal_Int32 nLinePropertiesCount = rLinePropertiesList.size();
    ::std::vector< ::std::vector< TickInfo > >::iterator aDepthIter             = rAngleTickInfos.begin();
    sal_Int32 nDepth=0;
    /*
    //no subgrids so far for polar angle grid (need different radii)
    const ::std::vector< ::std::vector< TickInfo > >::const_iterator aDepthEnd  = rAngleTickInfos.end();
    for( ; aDepthIter != aDepthEnd && nDepth < nLinePropertiesCount
         ; aDepthIter++, nDepth++ )
    */
    if(nLinePropertiesCount)
    {
        //create axis main lines
        drawing::PointSequenceSequence aAllPoints;
        ::std::vector< TickInfo >::iterator             aTickIter = (*aDepthIter).begin();
        const ::std::vector< TickInfo >::const_iterator aTickEnd  = (*aDepthIter).end();
        sal_Int32 nRealPointCount = 0;
        for( ; aTickIter != aTickEnd; aTickIter++ )
        {
            TickInfo& rTickInfo = *aTickIter;
            if( !rTickInfo.bPaintIt )
                continue;

            rTickInfo.updateUnscaledValue( xInverseScaling );
            double fLogicAngle = rTickInfo.fUnscaledTickValue;

            drawing::PointSequenceSequence aPoints(1);
            aPoints[0].realloc(2);
            drawing::Position3D aScenePositionStart( m_pPosHelper->transformLogicToScene( fLogicAngle, fLogicInnerRadius, fLogicZ ) );
            drawing::Position3D aScenePositionEnd(   m_pPosHelper->transformLogicToScene( fLogicAngle, fLogicOuterRadius, fLogicZ ) );
            aPoints[0][0].X = static_cast<sal_Int32>(aScenePositionStart.PositionX);
            aPoints[0][0].Y = static_cast<sal_Int32>(aScenePositionStart.PositionY);
            aPoints[0][1].X = static_cast<sal_Int32>(aScenePositionEnd.PositionX);
            aPoints[0][1].Y = static_cast<sal_Int32>(aScenePositionEnd.PositionY);
            appendPointSequence( aAllPoints, aPoints );
        }

        uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                xTarget, aAllPoints, rLinePropertiesList[nDepth] );
        //because of this name this line will be used for marking
        m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
    }
}

void VPolarGrid::create2DRadiusGrid( const uno::Reference< drawing::XShapes >& xTarget
        , ::std::vector< ::std::vector< TickInfo > >& rRadiusTickInfos
        , ::std::vector< ::std::vector< TickInfo > >& rAngleTickInfos
        , const ::std::vector<VLineProperties>& rLinePropertiesList )
{
    const ExplicitScaleData&     rRadiusScale = m_pPosHelper->getScales()[1];
    const ExplicitScaleData&     rAngleScale = m_pPosHelper->getScales()[0];
    const ExplicitIncrementData& rAngleIncrement = m_aIncrements[0];
    uno::Reference< XScaling > xInverseRadiusScaling( NULL );
    if( rRadiusScale.Scaling.is() )
        xInverseRadiusScaling = rRadiusScale.Scaling->getInverseScaling();

    sal_Int32 nLinePropertiesCount = rLinePropertiesList.size();
    ::std::vector< ::std::vector< TickInfo > >::iterator aDepthIter             = rRadiusTickInfos.begin();
    const ::std::vector< ::std::vector< TickInfo > >::const_iterator aDepthEnd  = rRadiusTickInfos.end();
    for( sal_Int32 nDepth=0
        ; aDepthIter != aDepthEnd && nDepth < nLinePropertiesCount
        ; aDepthIter++, nDepth++ )
    {
        //create axis main lines
        drawing::PointSequenceSequence aAllPoints;
        ::std::vector< TickInfo >::iterator             aTickIter = (*aDepthIter).begin();
        const ::std::vector< TickInfo >::const_iterator aTickEnd  = (*aDepthIter).end();
        sal_Int32 nRealPointCount = 0;
        for( ; aTickIter != aTickEnd; aTickIter++ )
        {
            TickInfo& rTickInfo = *aTickIter;
            if( !rTickInfo.bPaintIt )
                continue;

            rTickInfo.updateUnscaledValue( xInverseRadiusScaling );
            double fLogicRadius = rTickInfo.fUnscaledTickValue;
            double fLogicZ      = -0.5;//as defined

            drawing::PointSequenceSequence aPoints(1);
            VPolarGrid::createLinePointSequence_ForAngleAxis( aPoints, rAngleTickInfos
                , rAngleIncrement, rAngleScale, m_pPosHelper, fLogicRadius, fLogicZ );
            if(aPoints[0].getLength())
                appendPointSequence( aAllPoints, aPoints );
        }

        uno::Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                xTarget, aAllPoints, rLinePropertiesList[nDepth] );
        //because of this name this line will be used for marking
        m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
    }
}

void SAL_CALL VPolarGrid::createShapes()
{
    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"Axis is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;
    if( !m_xMeter.is())
        return;
    uno::Reference< beans::XPropertySet > xGridProps( m_xMeter, uno::UNO_QUERY );
    if(!xGridProps.is())
        return;

    //-----------------------------------------
    //create named group shape
    uno::Reference< XIdentifiable > xIdent( m_xMeter, uno::UNO_QUERY );
    if( ! xIdent.is())
        return;

    uno::Reference< drawing::XShapes > xTarget(
        this->createGroupShape( m_xLogicTarget
        , ObjectIdentifier::createClassifiedIdentifier(
                OBJECTTYPE_GRID, xIdent->getIdentifier() )
        ) );

    //-----------------------------------------
    //create all scaled tickmark values
    ::std::vector< ::std::vector< TickInfo > > aAngleTickInfos;
    ::std::vector< ::std::vector< TickInfo > > aRadiusTickInfos;
    getAllTickInfos( 0, aAngleTickInfos );
    getAllTickInfos( 1, aRadiusTickInfos );

    //-----------------------------------------
    ::std::vector<VLineProperties> aLinePropertiesList;
    VCartesianGrid::fillLinePropertiesFromGridModel( aLinePropertiesList, xGridProps );

    //-----------------------------------------
    //create tick mark line shapes
    if(2==m_nDimension)
    {
        sal_Int32 nDimensionIndex = m_xMeter->getRepresentedDimension();
        if(nDimensionIndex==1)
            this->create2DRadiusGrid( xTarget, aRadiusTickInfos, aAngleTickInfos, aLinePropertiesList );
        else
            this->create2DAngleGrid( xTarget, aRadiusTickInfos, aAngleTickInfos, aLinePropertiesList );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
