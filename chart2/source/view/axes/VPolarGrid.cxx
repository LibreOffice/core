/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "VPolarGrid.hxx"
#include "VCartesianGrid.hxx"
#include "Tickmarks.hxx"
#include "PlottingPositionHelper.hxx"
#include "ShapeFactory.hxx"
#include "ObjectIdentifier.hxx"
#include "macros.hxx"
#include "CommonConverters.hxx"
#include "Tickmarks_Equidistant.hxx"
#include <com/sun/star/drawing/LineStyle.hpp>

#include <vector>
#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;
using ::com::sun::star::uno::Reference;

VPolarGrid::VPolarGrid( sal_Int32 nDimensionIndex, sal_Int32 nDimensionCount
                       , const uno::Sequence< Reference< beans::XPropertySet > > & rGridPropertiesList )
            : VAxisOrGridBase( nDimensionIndex, nDimensionCount )
            , m_aGridPropertiesList( rGridPropertiesList )
            , m_pPosHelper( new PolarPlottingPositionHelper() )
            , m_aIncrements()
{
    PlotterBase::m_pPosHelper = m_pPosHelper;
}

VPolarGrid::~VPolarGrid()
{
    delete m_pPosHelper;
    m_pPosHelper = NULL;
}

void VPolarGrid::setIncrements( const std::vector< ExplicitIncrementData >& rIncrements )
{
    m_aIncrements = rIncrements;
}

void VPolarGrid::getAllTickInfos( sal_Int32 nDimensionIndex, ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos ) const
{
    TickFactory aTickFactory(
            m_pPosHelper->getScales()[nDimensionIndex], m_aIncrements[nDimensionIndex] );
    aTickFactory.getAllTicks( rAllTickInfos );
}

void VPolarGrid::createLinePointSequence_ForAngleAxis(
        drawing::PointSequenceSequence& rPoints
        , ::std::vector< ::std::vector< TickInfo > >& rAllTickInfos
        , const ExplicitIncrementData& rIncrement
        , const ExplicitScaleData& rScale
        , PolarPlottingPositionHelper* pPosHelper
        , double fLogicRadius, double fLogicZ )
{
    Reference< XScaling > xInverseScaling( NULL );
    if( rScale.Scaling.is() )
        xInverseScaling = rScale.Scaling->getInverseScaling();

    sal_Int32 nTick = 0;
    EquidistantTickIter aIter( rAllTickInfos, rIncrement, 0, 0 );
    for( TickInfo* pTickInfo = aIter.firstInfo()
        ; pTickInfo
        ; pTickInfo = aIter.nextInfo(), nTick++ )
    {
        if(nTick>=rPoints[0].getLength())
            rPoints[0].realloc(rPoints[0].getLength()+30);

        //xxxxx pTickInfo->updateUnscaledValue( xInverseScaling );
        double fLogicAngle = pTickInfo->getUnscaledTickValue();

        drawing::Position3D aScenePosition3D( pPosHelper->transformAngleRadiusToScene( fLogicAngle, fLogicRadius, fLogicZ ) );
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
#ifdef NOTYET
void VPolarGrid::create2DAngleGrid( const Reference< drawing::XShapes >& xLogicTarget
        , ::std::vector< ::std::vector< TickInfo > >& /* rRadiusTickInfos */
        , ::std::vector< ::std::vector< TickInfo > >& rAngleTickInfos
        , const ::std::vector<VLineProperties>& rLinePropertiesList )
{
    Reference< drawing::XShapes > xMainTarget(
        this->createGroupShape( xLogicTarget, m_aCID ) );

    const ExplicitScaleData&     rAngleScale = m_pPosHelper->getScales()[0];
    Reference< XScaling > xInverseScaling( NULL );
    if( rAngleScale.Scaling.is() )
        xInverseScaling = rAngleScale.Scaling->getInverseScaling();

    double fLogicInnerRadius = m_pPosHelper->getInnerLogicRadius();
    double fLogicOuterRadius = m_pPosHelper->getOuterLogicRadius();
    double fLogicZ      = 1.0;//as defined

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
        for( ; aTickIter != aTickEnd; aTickIter++ )
        {
            TickInfo& rTickInfo = *aTickIter;
            if( !rTickInfo.bPaintIt )
                continue;

            //xxxxx rTickInfo.updateUnscaledValue( xInverseScaling );
            double fLogicAngle = rTickInfo.getUnscaledTickValue();

            drawing::PointSequenceSequence aPoints(1);
            aPoints[0].realloc(2);
            drawing::Position3D aScenePositionStart( m_pPosHelper->transformAngleRadiusToScene( fLogicAngle, fLogicInnerRadius, fLogicZ ) );
            drawing::Position3D aScenePositionEnd(   m_pPosHelper->transformAngleRadiusToScene( fLogicAngle, fLogicOuterRadius, fLogicZ ) );
            aPoints[0][0].X = static_cast<sal_Int32>(aScenePositionStart.PositionX);
            aPoints[0][0].Y = static_cast<sal_Int32>(aScenePositionStart.PositionY);
            aPoints[0][1].X = static_cast<sal_Int32>(aScenePositionEnd.PositionX);
            aPoints[0][1].Y = static_cast<sal_Int32>(aScenePositionEnd.PositionY);
            appendPointSequence( aAllPoints, aPoints );
        }

        Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                xMainTarget, aAllPoints, &rLinePropertiesList[nDepth] );
        //because of this name this line will be used for marking
        m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
    }
}
#endif

void VPolarGrid::create2DRadiusGrid( const Reference< drawing::XShapes >& xLogicTarget
        , ::std::vector< ::std::vector< TickInfo > >& rRadiusTickInfos
        , ::std::vector< ::std::vector< TickInfo > >& rAngleTickInfos
        , const ::std::vector<VLineProperties>& rLinePropertiesList )
{
    Reference< drawing::XShapes > xMainTarget(
        this->createGroupShape( xLogicTarget, m_aCID ) );

    const ExplicitScaleData&     rRadiusScale = m_pPosHelper->getScales()[1];
    const ExplicitScaleData&     rAngleScale = m_pPosHelper->getScales()[0];
    const ExplicitIncrementData& rAngleIncrement = m_aIncrements[0];
    Reference< XScaling > xInverseRadiusScaling( NULL );
    if( rRadiusScale.Scaling.is() )
        xInverseRadiusScaling = rRadiusScale.Scaling->getInverseScaling();

    sal_Int32 nLinePropertiesCount = rLinePropertiesList.size();
    ::std::vector< ::std::vector< TickInfo > >::iterator aDepthIter             = rRadiusTickInfos.begin();
    const ::std::vector< ::std::vector< TickInfo > >::const_iterator aDepthEnd  = rRadiusTickInfos.end();
    for( sal_Int32 nDepth=0
        ; aDepthIter != aDepthEnd && nDepth < nLinePropertiesCount
        ; aDepthIter++, nDepth++ )
    {
        if( !rLinePropertiesList[nDepth].isLineVisible() )
            continue;

        Reference< drawing::XShapes > xTarget( xMainTarget );
        if( nDepth > 0 )
        {
            xTarget.set( this->createGroupShape( xLogicTarget
                , ObjectIdentifier::addChildParticle( m_aCID, ObjectIdentifier::createChildParticleWithIndex( OBJECTTYPE_SUBGRID, nDepth-1 ) )
                ) );
            if(!xTarget.is())
                xTarget.set( xMainTarget );
        }

        //create axis main lines
        drawing::PointSequenceSequence aAllPoints;
        ::std::vector< TickInfo >::iterator             aTickIter = (*aDepthIter).begin();
        const ::std::vector< TickInfo >::const_iterator aTickEnd  = (*aDepthIter).end();
        for( ; aTickIter != aTickEnd; aTickIter++ )
        {
            TickInfo& rTickInfo = *aTickIter;
            if( !rTickInfo.bPaintIt )
                continue;

            //xxxxx rTickInfo.updateUnscaledValue( xInverseRadiusScaling );
            double fLogicRadius = rTickInfo.getUnscaledTickValue();
            double fLogicZ      = 1.0;//as defined

            drawing::PointSequenceSequence aPoints(1);
            VPolarGrid::createLinePointSequence_ForAngleAxis( aPoints, rAngleTickInfos
                , rAngleIncrement, rAngleScale, m_pPosHelper, fLogicRadius, fLogicZ );
            if(aPoints[0].getLength())
                appendPointSequence( aAllPoints, aPoints );
        }

        Reference< drawing::XShape > xShape = m_pShapeFactory->createLine2D(
                xTarget, aAllPoints, &rLinePropertiesList[nDepth] );
        //because of this name this line will be used for marking
        m_pShapeFactory->setShapeName( xShape, C2U("MarkHandles") );
    }
}

void VPolarGrid::createShapes()
{
    DBG_ASSERT(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is(),"Axis is not proper initialized");
    if(!(m_pShapeFactory&&m_xLogicTarget.is()&&m_xFinalTarget.is()))
        return;
    if(!m_aGridPropertiesList.getLength())
        return;

    //-----------------------------------------
    //create all scaled tickmark values
    ::std::vector< ::std::vector< TickInfo > > aAngleTickInfos;
    ::std::vector< ::std::vector< TickInfo > > aRadiusTickInfos;
    getAllTickInfos( 0, aAngleTickInfos );
    getAllTickInfos( 1, aRadiusTickInfos );

    //-----------------------------------------
    ::std::vector<VLineProperties> aLinePropertiesList;
    VCartesianGrid::fillLinePropertiesFromGridModel( aLinePropertiesList, m_aGridPropertiesList );

    //-----------------------------------------
    //create tick mark line shapes
    if(2==m_nDimension)
    {
        if(m_nDimensionIndex==1)
            this->create2DRadiusGrid( m_xLogicTarget, aRadiusTickInfos, aAngleTickInfos, aLinePropertiesList );
        //else //no Angle Grid so far as this equals exactly the y axis positions
        //    this->create2DAngleGrid( m_xLogicTarget, aRadiusTickInfos, aAngleTickInfos, aLinePropertiesList );
    }
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
